import re
import json
import hashlib
import sys
import pathlib
import argparse
from typing import Dict, List, Optional, Set, Tuple, Any, Iterator
from dataclasses import dataclass, field, asdict

# Prefer language pack (bundles grammars); fallback to tree-sitter-languages if installed
try:
    from tree_sitter_language_pack import get_parser   # pip: tree-sitter-language-pack
except ImportError:
    from tree_sitter_languages import get_parser       # pip: tree-sitter-languages

# Regex for Doxygen comments (both /*! */ and /** */ styles)
DOXYGEN_COMMENT = re.compile(r"/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/", re.MULTILINE)
# Regular C comments (both block and line)
C_COMMENT = re.compile(r"/\*(?:.|\n)*?\*/|//.*?$", re.MULTILINE)

def _sha(*xs) -> str:
    """Generate a short SHA hash from the inputs."""
    return hashlib.sha1("|".join(map(str, xs)).encode()).hexdigest()[:16]

def _line_offsets(s: str) -> List[int]:
    """Calculate byte offsets for each line in the source."""
    offs, acc = [0], 0
    for ln in s.splitlines(True):
        acc += len(ln)
        offs.append(acc)
    return offs

def _byte_to_line(b: int, offs: List[int]) -> int:
    """Convert byte offset to line number."""
    import bisect
    return bisect.bisect_right(offs, b) - 1

def _find_nearest_comment(src: str, start_line: int, regex, max_gap: int = 5) -> Optional[Dict]:
    """Find the nearest comment above the given line."""
    blocks = [(m.start(), m.end(), m.group(0)) for m in regex.finditer(src)]
    offs = _line_offsets(src)
    best, gap = None, max_gap + 1
    
    for b0, b1, raw in blocks:
        end_ln = _byte_to_line(b1, offs)
        if 0 <= (start_line - end_ln) < gap:
            best, gap = raw, start_line - end_ln
            
    if not best:
        return None
        
    # Clean up the comment
    body = re.sub(r"^/\*\*!?|\*/$|^//", "", best.strip())
    body = "\n".join(re.sub(r"^\s*\* ?", "", ln) for ln in body.splitlines())
    
    return {
        "raw": best,
        "text": body.strip() or None,
        "line_gap": gap
    }

def _parse_doxygen_comment(comment: Dict) -> Dict:
    """Parse a Doxygen comment into structured data."""
    if not comment or not comment.get("text"):
        return comment
        
    body = comment["text"]
    tags, free = {}, []
    
    for ln in body.splitlines():
        m = re.match(r"@(\w+)\s*(.*)", ln)
        if m:
            tags.setdefault(m.group(1).lower(), []).append(m.group(2))
        else:
            free.append(ln)
    
    return {
        "raw": comment["raw"],
        "brief": " ".join(tags.get("brief", [])) or None,
        "param": tags.get("param", []),
        "retval": tags.get("retval", []),
        "return": tags.get("return", []),
        "note": tags.get("note", []),
        "warning": tags.get("warning", []),
        "text": "\n".join(free).strip() or None,
        "line_gap": comment.get("line_gap")
    }

@dataclass
class ASTNode:
    """Represents a node in the AST."""
    type: str
    code: str
    start_line: int
    end_line: int
    name: Optional[str] = None
    parent: Optional[str] = None
    doxygen: Optional[Dict] = None
    comments: List[Dict] = field(default_factory=list)
    
    def to_dict(self) -> Dict:
        """Convert to dictionary representation."""
        result = {
            "type": self.type,
            "code": self.code,
            "start_line": self.start_line,
            "end_line": self.end_line
        }
        
        if self.name:
            result["name"] = self.name
        if self.parent:
            result["parent"] = self.parent
        if self.doxygen:
            result["doxygen"] = self.doxygen
        if self.comments:
            result["comments"] = self.comments
            
        return result

class EmbeddedCParser:
    """
    AST-aware parser for embedded C headers with comprehensive comment handling.
    
    Features:
    - Extracts functions, structs, enums, typedefs, macros, and variable declarations
    - Associates Doxygen and regular comments with their respective code elements
    - Handles conditional compilation (#ifdef/#ifndef/#if)
    - Supports nested structures (e.g., structs within structs)
    - Extracts names of declarations where possible
    """
    
    # Target node types to extract from the AST
    TARGET_TYPES = {
        "function_definition",       # Function implementations
        "declaration",               # Function prototypes & variable declarations
        "struct_specifier",          # struct definitions
        "enum_specifier",            # enum definitions
        "union_specifier",           # union definitions
        "type_definition",           # typedef statements
        "preproc_def",               # #define macros
        "preproc_function_def",      # Function-like macros
        "preproc_ifdef",             # #ifdef blocks
        "preproc_ifndef",            # #ifndef blocks
        "preproc_if",                # #if blocks
    }
    
    def __init__(self, include_comments: bool = True, max_comment_gap: int = 5):
        """
        Initialize the parser.
        
        Args:
            include_comments: Whether to include regular comments in addition to Doxygen
            max_comment_gap: Maximum number of lines between a comment and code to associate them
        """
        self.parser = get_parser("c")
        self.include_comments = include_comments
        self.max_comment_gap = max_comment_gap
        
    def _extract_name(self, node_type: str, code: str) -> Optional[str]:
        """Extract the name of a declaration if possible."""
        if node_type == "function_definition":
            # Match function name: return_type name(params)
            match = re.search(r'(?:\w+\s+)+(\w+)\s*\(', code)
            if match:
                return match.group(1)
        
        elif node_type == "declaration" and "(" in code:
            # Function prototype: return_type name(params);
            match = re.search(r'(?:\w+\s+)+(\w+)\s*\(', code)
            if match:
                return match.group(1)
        
        elif node_type == "struct_specifier":
            # struct name { ... } or struct name;
            match = re.search(r'struct\s+(\w+)', code)
            if match:
                return match.group(1)
        
        elif node_type == "enum_specifier":
            # enum name { ... } or enum name;
            match = re.search(r'enum\s+(\w+)', code)
            if match:
                return match.group(1)
        
        elif node_type == "union_specifier":
            # union name { ... } or union name;
            match = re.search(r'union\s+(\w+)', code)
            if match:
                return match.group(1)
        
        elif node_type == "type_definition":
            # typedef ... name;
            match = re.search(r'typedef\s+(?:.*?)\s+(\w+)\s*;', code)
            if match:
                return match.group(1)
        
        elif node_type == "preproc_def":
            # #define NAME value
            match = re.search(r'#define\s+(\w+)', code)
            if match:
                return match.group(1)
        
        elif node_type == "preproc_function_def":
            # #define NAME(params) value
            match = re.search(r'#define\s+(\w+)\s*\(', code)
            if match:
                return match.group(1)
        
        return None
    
    def _find_parent_directive(self, node, source_bytes) -> Optional[str]:
        """Find the parent conditional directive if any."""
        parent = node.parent
        while parent:
            if parent.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if"}:
                # Extract the condition from the directive
                condition_node = parent.child_by_field_name("condition")
                if condition_node:
                    return source_bytes[condition_node.start_byte:condition_node.end_byte].decode("utf-8", "ignore")
            parent = parent.parent
        return None
    
    def parse_file(self, filepath: str) -> List[ASTNode]:
        """Parse a C header file and extract AST nodes."""
        path = pathlib.Path(filepath)
        if not path.exists():
            raise FileNotFoundError(f"File not found: {filepath}")
        
        source = path.read_text(encoding="utf-8", errors="ignore")
        return self.parse_source(source, filepath)
    
    def parse_source(self, source: str, filepath: Optional[str] = None) -> List[ASTNode]:
        """Parse C source code and extract AST nodes."""
        source_bytes = source.encode("utf-8")
        tree = self.parser.parse(source_bytes)
        root = tree.root_node
        line_offsets = _line_offsets(source)
        
        nodes = []
        
        def visit(node, parent_directive=None):
            if node.type in self.TARGET_TYPES:
                code = source_bytes[node.start_byte:node.end_byte].decode("utf-8", "ignore")
                start_line = _byte_to_line(node.start_byte, line_offsets)
                end_line = _byte_to_line(node.end_byte, line_offsets)
                
                # Skip empty or very short nodes
                if len(code.strip()) < 3:
                    return
                
                # Find the nearest Doxygen comment
                doxygen = _find_nearest_comment(source, start_line, DOXYGEN_COMMENT, self.max_comment_gap)
                if doxygen:
                    doxygen = _parse_doxygen_comment(doxygen)
                
                # Find regular comments if enabled
                comments = []
                if self.include_comments:
                    comment = _find_nearest_comment(source, start_line, C_COMMENT, self.max_comment_gap)
                    if comment and (not doxygen or comment["raw"] != doxygen.get("raw")):
                        comments.append(comment)
                
                # Extract name if possible
                name = self._extract_name(node.type, code)
                
                # Find parent directive if any
                parent = self._find_parent_directive(node, source_bytes) or parent_directive
                
                ast_node = ASTNode(
                    type=node.type,
                    code=code,
                    start_line=start_line,
                    end_line=end_line,
                    name=name,
                    parent=parent,
                    doxygen=doxygen,
                    comments=comments
                )
                
                nodes.append(ast_node)
                
                # For conditional directives, pass the condition to children
                if node.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if"}:
                    condition_node = node.child_by_field_name("condition")
                    if condition_node:
                        parent_directive = source_bytes[condition_node.start_byte:condition_node.end_byte].decode("utf-8", "ignore")
            
            # Visit children
            for child in node.children:
                visit(child, parent_directive)
        
        visit(root)
        return nodes

class CChunker:
    """
    Chunker for C code based on AST parsing.
    
    Features:
    - Creates chunks based on AST nodes (functions, structs, etc.)
    - Can create one chunk per symbol or buffer multiple symbols together
    - Includes associated comments and documentation
    - Supports overlapping chunks for better context
    """
    
    def __init__(self, max_chunk_size: int = 1600, chunk_overlap_units: int = 1, 
                 one_symbol_per_chunk: bool = False, include_comments: bool = True):
        """
        Initialize the chunker.
        
        Args:
            max_chunk_size: Maximum size of a chunk in characters
            chunk_overlap_units: Number of units to overlap between chunks
            one_symbol_per_chunk: Whether to create one chunk per symbol
            include_comments: Whether to include regular comments
        """
        self.parser = EmbeddedCParser(include_comments=include_comments)
        self.max_chunk_size = max_chunk_size
        self.overlap_units = chunk_overlap_units
        self.one_symbol_per_chunk = one_symbol_per_chunk
    
    def chunkify(self, code: str, filepath: Optional[str] = None) -> List[Dict]:
        """
        Chunk the given code into semantic units.
        
        Args:
            code: The source code to chunk
            filepath: Optional filepath for metadata
            
        Returns:
            List of chunks, each containing content and metadata
        """
        nodes = self.parser.parse_source(code, filepath)
        
        # Sort nodes by line number
        nodes.sort(key=lambda n: n.start_line)
        
        if self.one_symbol_per_chunk:
            # One chunk per symbol
            chunks = []
            for node in nodes:
                content = self._format_node_content(node)
                chunks.append(self._emit_chunk([node], content, filepath))
            return chunks
        
        # Buffered mode - group nodes into chunks
        chunks, buffer, node_buffer = [], "", []
        
        for node in nodes:
            node_content = self._format_node_content(node)
            
            # If adding this node would exceed max size, emit the current buffer
            if buffer and (len(buffer) + len(node_content) > self.max_chunk_size):
                chunks.append(self._emit_chunk(node_buffer, buffer, filepath))
                
                # Handle overlap if needed
                if self.overlap_units and node_buffer:
                    keep = node_buffer[-self.overlap_units:]
                    buffer = "\n\n".join(self._format_node_content(n) for n in keep)
                    node_buffer = keep
                else:
                    buffer, node_buffer = "", []
            
            # Add the node to the buffer
            node_buffer.append(node)
            buffer = (buffer + "\n\n" + node_content) if buffer else node_content
        
        # Emit any remaining buffer
        if node_buffer:
            chunks.append(self._emit_chunk(node_buffer, buffer, filepath))
            
        return chunks
    
    def _format_node_content(self, node: ASTNode) -> str:
        """Format a node's content including documentation."""
        content = []
        
        # Add Doxygen comment if available
        if node.doxygen:
            if node.doxygen.get("brief"):
                content.append(f"Brief: {node.doxygen['brief']}")
            
            if node.doxygen.get("text"):
                content.append(node.doxygen["text"])
            
            # Add parameters documentation
            if node.doxygen.get("param"):
                content.append("Parameters:")
                for param in node.doxygen["param"]:
                    content.append(f"- {param}")
            
            # Add return value documentation
            if node.doxygen.get("return") or node.doxygen.get("retval"):
                content.append("Returns:")
                for ret in node.doxygen.get("return", []) + node.doxygen.get("retval", []):
                    content.append(f"- {ret}")
            
            if content:
                content.append("")  # Add a blank line after documentation
        
        # Add regular comments if available
        if node.comments:
            for comment in node.comments:
                if comment.get("text"):
                    content.append(f"Comment: {comment['text']}")
            if content:
                content.append("")  # Add a blank line after comments
        
        # Add the code
        content.append(node.code)
        
        return "\n".join(content)
    
    def _emit_chunk(self, nodes: List[ASTNode], content: str, filepath: Optional[str] = None) -> Dict:
        """Create a chunk from the given nodes and content."""
        first_node = nodes[0]
        
        metadata = {
            "language": "c",
            "filepath": filepath or "",
            "chunk_units": [
                {
                    "type": n.type,
                    "start": n.start_line,
                    "end": n.end_line,
                    "name": n.name
                } for n in nodes
            ],
            "conditional_context": [n.parent for n in nodes if n.parent],
        }
        
        return {
            "id": _sha(filepath, first_node.start_line, first_node.end_line),
            "content": content,
            "metadata": metadata
        }

def _iter_header_files(paths):
    """Iterate over all header files in the given paths."""
    for p in paths:
        p = pathlib.Path(p)
        if p.is_file() and p.suffix.lower() in (".h", ".hpp"):
            yield p
        elif p.is_dir():
            for f in p.rglob("*"):
                if f.is_file() and f.suffix.lower() in (".h", ".hpp"):
                    yield f

def main():
    """Command-line interface for the chunker."""
    ap = argparse.ArgumentParser(description="AST-aware chunker for embedded C headers")
    ap.add_argument("inputs", nargs="+", help="Header files or folders")
    ap.add_argument("--max-chars", type=int, default=1600, help="Max characters per buffered chunk")
    ap.add_argument("--overlap", type=int, default=1, help="Buffered mode: number of units to overlap")
    ap.add_argument("--one-per-symbol", action="store_true", help="Emit one chunk per AST unit")
    ap.add_argument("--no-comments", action="store_true", help="Exclude regular comments")
    ap.add_argument("--output", "-o", help="Output file (default: stdout)")
    args = ap.parse_args()
    
    chunker = CChunker(
        max_chunk_size=args.max_chars,
        chunk_overlap_units=args.overlap,
        one_symbol_per_chunk=args.one_per_symbol,
        include_comments=not args.no_comments
    )
    
    chunks = []
    for path in _iter_header_files(args.inputs):
        try:
            source = pathlib.Path(path).read_text(encoding="utf-8", errors="ignore")
            file_chunks = chunker.chunkify(source, filepath=str(path))
            chunks.extend(file_chunks)
        except Exception as e:
            print(f"Error processing {path}: {e}", file=sys.stderr)
    
    # Output the chunks
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            for chunk in chunks:
                f.write(json.dumps(chunk, ensure_ascii=False) + "\n")
    else:
        for chunk in chunks:
            print(json.dumps(chunk, ensure_ascii=False))

if __name__ == "__main__":
    main()



