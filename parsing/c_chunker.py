import re, json, hashlib, sys, pathlib, argparse

# Prefer language pack (bundles grammars); fallback to tree-sitter-languages if installed
try:
    from tree_sitter_language_pack import get_parser   # pip: tree-sitter-language-pack
except ImportError:
    from tree_sitter_languages import get_parser       # pip: tree-sitter-languages

DOXY = re.compile(r"/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/", re.MULTILINE)

def _sha(*xs): return hashlib.sha1("|".join(map(str, xs)).encode()).hexdigest()[:16]

def _line_offsets(s: str):
    offs, acc = [0], 0
    for ln in s.splitlines(True):
        acc += len(ln); offs.append(acc)
    return offs

def _byte_to_line(b: int, offs):
    import bisect
    return bisect.bisect_right(offs, b) - 1

def _nearest_doxygen_above(src: str, start_line: int):
    blocks = [(m.start(), m.end(), m.group(0)) for m in DOXY.finditer(src)]
    offs = _line_offsets(src)
    best, gap = None, 10**9
    for b0, b1, raw in blocks:
        end_ln = _byte_to_line(b1, offs)
        if 0 <= (start_line - end_ln) < gap:
            best, gap = raw, start_line - end_ln
    if not best: return None
    body = re.sub(r"^/\*\*!?|\*/$", "", best.strip())
    body = "\n".join(re.sub(r"^\s*\* ?", "", ln) for ln in body.splitlines())

    tags, free = {}, []
    for ln in body.splitlines():
        m = re.match(r"@(\w+)\s*(.*)", ln)
        if m: tags.setdefault(m.group(1).lower(), []).append(m.group(2))
        else: free.append(ln)

    return {
        "raw": best,
        "brief": " ".join(tags.get("brief", [])) or None,
        "param": tags.get("param", []),
        "retval": tags.get("retval", []),
        "note": tags.get("note", []),
        "warning": tags.get("warning", []),
        "text": "\n".join(free).strip() or None
    }

class CChunker:
    """
    AST-aware chunker for C headers with Doxygen pairing.
    - Recursively walks the full tree (handles #ifdef/#ifndef wrapping).
    - Emits either one-symbol-per-chunk or buffered chunks by max chars.
    """
    TARGET = {
        "function_definition",
        "declaration",              # function prototypes & vars
        "struct_specifier",
        "enum_specifier",
        "type_definition",
        "preproc_def",
        "preproc_function_def",
    }

    def __init__(self, max_chunk_size=1600, chunk_overlap_units=1, one_symbol_per_chunk=False):
        self.parser = get_parser("c")
        self.max = max_chunk_size
        self.overlap_units = chunk_overlap_units
        self.one_symbol = one_symbol_per_chunk

    def _units(self, src: str):
        b = src.encode("utf-8")
        tree = self.parser.parse(b)
        root = tree.root_node
        offs = _line_offsets(src)

        def visit(n):
            if n.type in self.TARGET:
                code = b[n.start_byte:n.end_byte].decode("utf-8", "ignore")
                start = _byte_to_line(n.start_byte, offs)
                end   = _byte_to_line(n.end_byte, offs)
                yield {"type": n.type, "code": code, "start": start, "end": end}
            for ch in n.children:
                yield from visit(ch)

        yield from visit(root)

    def chunkify(self, code: str, filepath: str | None = None):
        if self.one_symbol:
            chunks = []
            for u in self._units(code):
                dx = _nearest_doxygen_above(code, u["start"])
                content = ((dx.get("brief")+"\n\n") if dx and dx.get("brief") else "") + (dx.get("text") or u["code"])
                chunks.append(self._emit(content, [{"u":u, "dox":dx, "content":content}], filepath))
            return chunks

        # buffered mode
        chunks, buf, units_buf = [], "", []
        for u in self._units(code):
            dx = _nearest_doxygen_above(code, u["start"])
            content = ((dx.get("brief")+"\n\n") if dx and dx.get("brief") else "") + (dx.get("text") or u["code"])

            if buf and (len(buf) + len(content) > self.max):
                chunks.append(self._emit(buf, units_buf, filepath))
                if self.overlap_units and units_buf:
                    keep = units_buf[-self.overlap_units:]
                    buf = "\n\n".join(x["content"] for x in keep); units_buf = keep
                else:
                    buf, units_buf = "", []

            units_buf.append({"u":u, "dox":dx, "content":content})
            buf = (buf+"\n\n"+content) if buf else content

        if units_buf:
            chunks.append(self._emit(buf, units_buf, filepath))
        return chunks

    def _emit(self, text, units, filepath):
        first = units[0]["u"]
        md = {
            "language": "c",
            "filepath": filepath or "",
            "chunk_units": [{"type": x["u"]["type"], "start": x["u"]["start"], "end": x["u"]["end"]} for x in units],
            "doxygen": [x["dox"] for x in units if x["dox"]],
        }
        return {
            "id": _sha(filepath, first["start"], first["end"]),
            "content": text,
            "metadata": md
        }

def _iter_header_files(paths):
    for p in paths:
        p = pathlib.Path(p)
        if p.is_file() and p.suffix.lower() in (".h", ".hpp"):
            yield p
        elif p.is_dir():
            for f in p.rglob("*"):
                if f.is_file() and f.suffix.lower() in (".h", ".hpp"):
                    yield f

def main():
    ap = argparse.ArgumentParser(description="AST-aware chunker for C headers")
    ap.add_argument("inputs", nargs="+", help="Header files or folders")
    ap.add_argument("--max-chars", type=int, default=1600, help="Max characters per buffered chunk")
    ap.add_argument("--overlap", type=int, default=1, help="Buffered mode: number of units to overlap")
    ap.add_argument("--one-per-symbol", action="store_true", help="Emit one chunk per AST unit")
    args = ap.parse_args()

    ch = CChunker(max_chunk_size=args.max_chars, chunk_overlap_units=args.overlap, one_symbol_per_chunk=args.one_per_symbol)
    for path in _iter_header_files(args.inputs):
        s = pathlib.Path(path).read_text(encoding="utf-8", errors="ignore")
        for c in ch.chunkify(s, filepath=str(path)):
            print(json.dumps(c, ensure_ascii=False))

if __name__ == "__main__":
    main()
