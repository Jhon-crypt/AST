"""
Enhanced C Parser with Complete Coverage

This module extends the C AST parser to provide both:
1. Semantic chunking of code elements (functions, structs, enums)
2. Complete coverage of the file including headers, section comments, etc.
"""

import re
import json
import hashlib
import sys
import pathlib
import argparse
from typing import Dict, List, Optional, Set, Tuple, Any, Iterator
from dataclasses import dataclass, field, asdict
from collections import defaultdict

# Import existing code
from c_ast_parser_simple import (
    _sha, _line_offsets, _byte_to_line, _find_nearest_comment,
    _parse_doxygen_comment, ASTNode, RegexBasedCParser, CChunker
)

# Regular expressions for C code structures
DOXYGEN_COMMENT = re.compile(r"/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/", re.MULTILINE)
C_COMMENT = re.compile(r"/\*(?:.|\n)*?\*/|//.*?$", re.MULTILINE)
FILE_HEADER = re.compile(r"(?:/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/|//!.*(?:\n//!.*)*)", re.MULTILINE)
SECTION_HEADER = re.compile(r"(?:^|\n)[ \t]*//[^\n]*(?:=+)[^\n]*(?:\n//[^\n]*)*", re.MULTILINE)

class EnhancedCParser(RegexBasedCParser):
    """
    Enhanced parser that captures both semantic code elements and file structure elements.
    """
    
    def __init__(self, include_comments: bool = True, max_comment_gap: int = 5, 
                 include_file_headers: bool = True, include_section_headers: bool = True):
        """
        Initialize the parser.
        
        Args:
            include_comments: Whether to include regular comments in addition to Doxygen
            max_comment_gap: Maximum number of lines between a comment and code to associate them
            include_file_headers: Whether to include file headers as separate nodes
            include_section_headers: Whether to include section headers as separate nodes
        """
        super().__init__(include_comments, max_comment_gap)
        self.include_file_headers = include_file_headers
        self.include_section_headers = include_section_headers
        
        # Enhanced patterns
        self.PATTERNS.update({
            # Enhanced typedef patterns
            "typedef_struct_named": re.compile(r"typedef\s+struct\s+(\w+)\s*\{[^}]*\}\s*\w+\s*;", re.MULTILINE | re.DOTALL),
            "typedef_enum_named": re.compile(r"typedef\s+enum\s+(\w+)\s*\{[^}]*\}\s*\w+\s*;", re.MULTILINE | re.DOTALL),
            "typedef_union_named": re.compile(r"typedef\s+union\s+(\w+)\s*\{[^}]*\}\s*\w+\s*;", re.MULTILINE | re.DOTALL),
            
            # Additional patterns for embedded C specific constructs
            "enum_definition": re.compile(r"enum\s*\{[^}]*\}\s*;", re.MULTILINE | re.DOTALL),
            "struct_definition": re.compile(r"struct\s*\{[^}]*\}\s*;", re.MULTILINE | re.DOTALL),
        })
    
    def parse_source(self, source: str, filepath: Optional[str] = None) -> List[ASTNode]:
        """Parse C source code and extract nodes."""
        nodes = super().parse_source(source, filepath)
        
        # Add file header if requested
        if self.include_file_headers:
            file_header_match = FILE_HEADER.search(source)
            if file_header_match:
                start_idx = file_header_match.start()
                end_idx = file_header_match.end()
                start_line = self._find_line_number(source, start_idx)
                end_line = self._find_line_number(source, end_idx)
                
                header_text = source[start_idx:end_idx]
                
                nodes.append(ASTNode(
                    type="file_header",
                    code=header_text,
                    start_line=start_line,
                    end_line=end_line,
                    name=filepath.split('/')[-1] if filepath else None,
                    parent=None,
                    doxygen=None,
                    comments=[]
                ))
        
        # Add section headers if requested
        if self.include_section_headers:
            for match in SECTION_HEADER.finditer(source):
                start_idx = match.start()
                end_idx = match.end()
                start_line = self._find_line_number(source, start_idx)
                end_line = self._find_line_number(source, end_idx)
                
                section_text = source[start_idx:end_idx]
                
                # Try to extract a name for the section
                section_name = None
                name_match = re.search(r"//[^\n]*(\w+)[^\n]*(?:=+)", section_text)
                if name_match:
                    section_name = name_match.group(1).strip()
                
                nodes.append(ASTNode(
                    type="section_header",
                    code=section_text,
                    start_line=start_line,
                    end_line=end_line,
                    name=section_name,
                    parent=None,
                    doxygen=None,
                    comments=[]
                ))
        
        # Sort nodes by line number
        nodes.sort(key=lambda n: n.start_line)
        return nodes

class EnhancedChunker(CChunker):
    """
    Enhanced chunker that provides both semantic chunking and complete coverage.
    """
    
    def __init__(self, max_chunk_size: int = 1600, chunk_overlap_units: int = 1,
                 one_symbol_per_chunk: bool = False, include_comments: bool = True,
                 include_file_headers: bool = True, include_section_headers: bool = True,
                 semantic_only: bool = False):
        """
        Initialize the chunker.
        
        Args:
            max_chunk_size: Maximum size of a chunk in characters
            chunk_overlap_units: Number of units to overlap between chunks
            one_symbol_per_chunk: Whether to create one chunk per symbol
            include_comments: Whether to include regular comments
            include_file_headers: Whether to include file headers
            include_section_headers: Whether to include section headers
            semantic_only: If True, only include semantic elements (functions, structs, etc.)
        """
        self.parser = EnhancedCParser(
            include_comments=include_comments,
            include_file_headers=include_file_headers,
            include_section_headers=include_section_headers
        )
        self.max_chunk_size = max_chunk_size
        self.overlap_units = chunk_overlap_units
        self.one_symbol_per_chunk = one_symbol_per_chunk
        self.semantic_only = semantic_only
        
        # Define which node types are considered "semantic"
        self.semantic_types = {
            "function_definition", "declaration", "struct_specifier", "enum_specifier",
            "union_specifier", "type_definition", "preproc_function_def", "preproc_def",
            "typedef_struct", "typedef_enum", "typedef_union", "typedef_anon_struct",
            "typedef_anon_enum", "typedef_anon_union", "typedef_struct_named",
            "typedef_enum_named", "typedef_union_named"
        }
    
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
        
        # Filter nodes if semantic_only is True
        if self.semantic_only:
            nodes = [n for n in nodes if n.type in self.semantic_types]
        
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

def main():
    """Command-line interface for the enhanced chunker."""
    ap = argparse.ArgumentParser(description="Enhanced chunker for embedded C headers")
    ap.add_argument("inputs", nargs="+", help="Header files or folders")
    ap.add_argument("--max-chars", type=int, default=1600, help="Max characters per buffered chunk")
    ap.add_argument("--overlap", type=int, default=1, help="Buffered mode: number of units to overlap")
    ap.add_argument("--one-per-symbol", action="store_true", help="Emit one chunk per AST unit")
    ap.add_argument("--no-comments", action="store_true", help="Exclude regular comments")
    ap.add_argument("--no-file-headers", action="store_true", help="Exclude file headers")
    ap.add_argument("--no-section-headers", action="store_true", help="Exclude section headers")
    ap.add_argument("--semantic-only", action="store_true", help="Only include semantic elements")
    ap.add_argument("--output", "-o", help="Output file (default: stdout)")
    args = ap.parse_args()
    
    chunker = EnhancedChunker(
        max_chunk_size=args.max_chars,
        chunk_overlap_units=args.overlap,
        one_symbol_per_chunk=args.one_per_symbol,
        include_comments=not args.no_comments,
        include_file_headers=not args.no_file_headers,
        include_section_headers=not args.no_section_headers,
        semantic_only=args.semantic_only
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

# Reuse the header file iterator from the original code
from c_ast_parser_simple import _iter_header_files

if __name__ == "__main__":
    main()
