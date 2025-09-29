"""
CAST: Chunking via Abstract Syntax Tree for Embedded C Code

This module implements the CAST approach described in the paper:
"CAST: Enhancing Code Retrieval-Augmented Generation with Structural Chunking via Abstract Syntax Tree"
https://arxiv.org/pdf/2506.15655

The implementation is based on the existing RegexBasedCParser but enhances it with:
1. Improved structure-aware chunking
2. Split-then-merge algorithm for better semantic units
3. Hierarchical node relationships
4. Improved handling of nested structures
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
from parsing.parsers.c_ast_parser_simple import (
    _sha, _line_offsets, _byte_to_line, _find_nearest_comment,
    _parse_doxygen_comment, ASTNode, RegexBasedCParser
)

# Regular expressions for C code structures
DOXYGEN_COMMENT = re.compile(r"/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/", re.MULTILINE)
C_COMMENT = re.compile(r"/\*(?:.|\n)*?\*/|//.*?$", re.MULTILINE)

# Node types ordered by hierarchy level (used for merging related nodes)
NODE_HIERARCHY = {
    # Preprocessor directives are top-level containers
    "preproc_ifdef": 0,
    "preproc_ifndef": 0,
    "preproc_if": 0,
    "preproc_else": 0,
    "preproc_elif": 0,
    "preproc_endif": 0,
    # Type definitions and specifiers
    "type_definition": 1,  # Typedef is highest in type hierarchy
    "struct_specifier": 2,
    "enum_specifier": 2,
    "union_specifier": 2,
    # Function definitions
    "function_definition": 3,
    # Variable declarations
    "declaration": 4,
    # Preprocessor macros
    "preproc_function_def": 5,
    "preproc_def": 5,
}

@dataclass
class CASTNode(ASTNode):
    """
    Enhanced AST node with additional properties for CAST approach.
    """
    children: List["CASTNode"] = field(default_factory=list)
    parent_node: Optional["CASTNode"] = None
    depth: int = 0
    
    def __hash__(self):
        """Make the node hashable based on its position and type."""
        return hash((self.start_line, self.end_line, self.type, self.name))
    
    def __eq__(self, other):
        """Equality check for hashing."""
        if not isinstance(other, CASTNode):
            return False
        return (self.start_line == other.start_line and 
                self.end_line == other.end_line and 
                self.type == other.type and 
                self.name == other.name)
    
    def add_child(self, child: "CASTNode") -> None:
        """Add a child node."""
        self.children.append(child)
        child.parent_node = self
        child.depth = self.depth + 1
    
    def get_full_context(self) -> str:
        """Get the full context including parent nodes."""
        context = []
        if self.parent_node:
            context.append(self.parent_node.get_full_context())
        if self.name:
            context.append(f"{self.type}:{self.name}")
        else:
            context.append(self.type)
        return "/".join(filter(None, context))
    
    def to_dict(self) -> Dict:
        """Convert to dictionary representation with enhanced metadata."""
        result = super().to_dict()
        result["depth"] = self.depth
        result["context"] = self.get_full_context()
        return result

class CASTParser(RegexBasedCParser):
    """
    Enhanced parser that builds a hierarchical AST and implements the CAST approach.
    """
    
    def __init__(self, include_comments: bool = True, max_comment_gap: int = 5):
        super().__init__(include_comments, max_comment_gap)
        
        # Enhanced patterns for better structure detection
        self.PATTERNS.update({
            "include_directive": re.compile(r'#include\s+[<"]([^>"]+)[>"]', re.MULTILINE),
            "typedef_struct": re.compile(r'typedef\s+struct\s+(\w+)(?:\s*\{[^}]*\}|\s*;)', re.MULTILINE | re.DOTALL),
            "typedef_enum": re.compile(r'typedef\s+enum\s+(\w+)(?:\s*\{[^}]*\}|\s*;)', re.MULTILINE | re.DOTALL),
            "typedef_union": re.compile(r'typedef\s+union\s+(\w+)(?:\s*\{[^}]*\}|\s*;)', re.MULTILINE | re.DOTALL),
            "struct_field": re.compile(r'(\w+(?:\s*\*+)?)\s+(\w+)(?:\[[^\]]*\])?\s*;', re.MULTILINE),
        })
    
    def _build_hierarchy(self, nodes: List[CASTNode]) -> List[CASTNode]:
        """
        Build a hierarchical structure of nodes based on their position and scope.
        
        This implements the core of the CAST approach by establishing parent-child
        relationships between nodes based on their structural relationships.
        """
        # First, identify preprocessor directive blocks (ifdef/ifndef/if -> endif)
        preproc_stack = []
        preproc_blocks = {}
        
        # First pass: identify preprocessor blocks
        for node in sorted(nodes, key=lambda n: n.start_line):
            if node.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if"}:
                preproc_stack.append(node)
            elif node.type == "preproc_endif" and preproc_stack:
                start_node = preproc_stack.pop()
                preproc_blocks[start_node] = (start_node.start_line, node.end_line)
        
        # Second pass: identify struct/enum hierarchy
        # For example, a typedef struct might contain nested structs
        struct_hierarchy = {}
        for node in nodes:
            if node.type in {"type_definition", "struct_specifier", "enum_specifier", "union_specifier"}:
                for potential_child in nodes:
                    if potential_child != node and \
                       potential_child.start_line > node.start_line and \
                       potential_child.end_line < node.end_line and \
                       potential_child.type in {"struct_specifier", "enum_specifier", "union_specifier"}:
                        struct_hierarchy[potential_child] = node
        
        # Sort nodes by start line and then by hierarchy level (containers first)
        # We need to process preprocessor directives first as they can contain other elements
        nodes.sort(key=lambda n: (n.start_line, NODE_HIERARCHY.get(n.type, 99)))
        
        # Find potential parent-child relationships
        root_nodes = []
        node_map = {(n.start_line, n.end_line): n for n in nodes}
        
        for node in nodes:
            # Find potential parent nodes (nodes that fully contain this node)
            parent_found = False
            
            for parent in nodes:
                if parent == node:
                    continue
                    
                # Check if parent contains this node
                parent_contains_node = False
                
                # Special handling for preprocessor directives
                if parent.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if"} and parent in preproc_blocks:
                    start_line, end_line = preproc_blocks[parent]
                    if start_line < node.start_line and end_line >= node.end_line:
                        parent_contains_node = True
                # Special handling for struct hierarchy
                elif node in struct_hierarchy and struct_hierarchy[node] == parent:
                    parent_contains_node = True
                # Normal containment check
                elif parent.start_line < node.start_line and parent.end_line >= node.end_line:
                    parent_contains_node = True
                    
                if parent_contains_node:
                    
                    # Find the closest parent
                    if not parent_found or (
                        node.parent_node and
                        parent.start_line > node.parent_node.start_line and
                        parent.end_line <= node.parent_node.end_line
                    ):
                        parent.add_child(node)
                        parent_found = True
            
            if not parent_found:
                root_nodes.append(node)
        
        return root_nodes
    
    def parse_source(self, source: str, filepath: Optional[str] = None) -> List[CASTNode]:
        """
        Parse C source code and extract a hierarchical AST.
        """
        # Use the base parser to get initial nodes
        base_nodes = super().parse_source(source, filepath)
        
        # Convert to CASTNodes
        cast_nodes = []
        for node in base_nodes:
            cast_node = CASTNode(
                type=node.type,
                code=node.code,
                start_line=node.start_line,
                end_line=node.end_line,
                name=node.name,
                parent=node.parent,
                doxygen=node.doxygen,
                comments=node.comments
            )
            cast_nodes.append(cast_node)
        
        # Build hierarchy
        root_nodes = self._build_hierarchy(cast_nodes)
        
        # Update depths based on hierarchy
        def _update_depths(node, depth=0):
            node.depth = depth
            for child in node.children:
                _update_depths(child, depth + 1)
        
        for root in root_nodes:
            _update_depths(root)
        
        # Flatten back to a list for compatibility with existing code
        flattened = []
        
        def _flatten(nodes):
            for node in nodes:
                flattened.append(node)
                _flatten(node.children)
        
        _flatten(root_nodes)
        return flattened

class CASTChunker:
    """
    Enhanced chunker that implements the CAST approach.
    
    Features:
    - Structure-preserving chunks that respect syntax boundaries
    - Split-then-merge algorithm for better semantic units
    - Hierarchical node relationships
    - Improved handling of nested structures
    """
    
    def __init__(
        self, 
        max_chunk_size: int = 1600, 
        min_chunk_size: int = 200,
        chunk_overlap_units: int = 1, 
        one_symbol_per_chunk: bool = False, 
        include_comments: bool = True,
        respect_hierarchy: bool = True
    ):
        """
        Initialize the chunker.
        
        Args:
            max_chunk_size: Maximum size of a chunk in characters
            min_chunk_size: Minimum size of a chunk in characters
            chunk_overlap_units: Number of units to overlap between chunks
            one_symbol_per_chunk: Whether to create one chunk per symbol
            include_comments: Whether to include regular comments
            respect_hierarchy: Whether to respect hierarchical relationships
        """
        self.parser = CASTParser(include_comments=include_comments)
        self.max_chunk_size = max_chunk_size
        self.min_chunk_size = min_chunk_size
        self.overlap_units = chunk_overlap_units
        self.one_symbol_per_chunk = one_symbol_per_chunk
        self.respect_hierarchy = respect_hierarchy
    
    def _get_node_size(self, node: CASTNode) -> int:
        """Get the size of a node including its documentation."""
        content = self._format_node_content(node)
        return len(content)
    
    def _should_split_node(self, node: CASTNode) -> bool:
        """Determine if a node should be split based on its size and type."""
        node_size = self._get_node_size(node)
        
        # Don't split small nodes
        if node_size <= self.max_chunk_size:
            return False
            
        # Don't split certain types of nodes even if they're large
        if node.type in {"preproc_def", "preproc_function_def"}:
            return False
            
        return True
    
    def _group_related_nodes(self, nodes: List[CASTNode]) -> List[List[CASTNode]]:
        """
        Group related nodes together based on their relationships.
        
        This implements the "merge" part of the split-then-merge algorithm.
        """
        if not nodes:
            return []
            
        # Group by parent conditional directive
        parent_groups = defaultdict(list)
        for node in nodes:
            parent_key = node.parent or "root"
            parent_groups[parent_key].append(node)
        
        # Group by type and proximity
        result_groups = []
        for parent_key, group in parent_groups.items():
            # Sort by line number
            group.sort(key=lambda n: n.start_line)
            
            current_group = []
            current_size = 0
            
            for node in group:
                node_size = self._get_node_size(node)
                
                # If adding this node would exceed max size, start a new group
                if current_group and current_size + node_size > self.max_chunk_size:
                    result_groups.append(current_group)
                    current_group = []
                    current_size = 0
                
                current_group.append(node)
                current_size += node_size
            
            # Add the last group
            if current_group:
                result_groups.append(current_group)
        
        return result_groups
    
    def _split_large_node(self, node: CASTNode) -> List[CASTNode]:
        """
        Split a large node into smaller chunks.
        
        This implements the "split" part of the split-then-merge algorithm.
        """
        # If the node has children, use them for splitting
        if node.children:
            return [node]  # The hierarchy will be handled in chunkify
            
        # For large nodes without children, we need to split the content
        # This is a simplified approach - in a real implementation, you'd want
        # to split more intelligently based on the node type
        
        lines = node.code.splitlines()
        chunks = []
        current_chunk = []
        current_size = 0
        
        for line in lines:
            line_size = len(line) + 1  # +1 for newline
            
            if current_chunk and current_size + line_size > self.max_chunk_size:
                # Create a new node for this chunk
                chunk_code = "\n".join(current_chunk)
                chunk_node = CASTNode(
                    type=f"{node.type}_part",
                    code=chunk_code,
                    start_line=node.start_line,
                    end_line=node.start_line + len(current_chunk),
                    name=node.name,
                    parent=node.parent,
                    doxygen=node.doxygen if not chunks else None,  # Only include doxygen in first chunk
                    comments=node.comments if not chunks else []
                )
                chunks.append(chunk_node)
                
                # Start a new chunk
                current_chunk = [line]
                current_size = line_size
            else:
                current_chunk.append(line)
                current_size += line_size
        
        # Add the last chunk
        if current_chunk:
            chunk_code = "\n".join(current_chunk)
            chunk_node = CASTNode(
                type=f"{node.type}_part",
                code=chunk_code,
                start_line=node.start_line + len(chunks),
                end_line=node.end_line,
                name=node.name,
                parent=node.parent,
                doxygen=node.doxygen if not chunks else None,
                comments=node.comments if not chunks else []
            )
            chunks.append(chunk_node)
        
        return chunks
    
    def _format_node_content(self, node: CASTNode) -> str:
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
        
        # Add context information if this is a nested node
        if self.respect_hierarchy and node.depth > 0:
            context = node.get_full_context()
            content.append(f"Context: {context}")
            content.append("")
        
        # Add the code
        content.append(node.code)
        
        return "\n".join(content)
    
    def _emit_chunk(self, nodes: List[CASTNode], content: str, filepath: Optional[str] = None) -> Dict:
        """Create a chunk from the given nodes and content."""
        first_node = nodes[0]
        
        # Get the maximum depth of any node in this chunk
        max_depth = max((n.depth for n in nodes), default=0)
        
        metadata = {
            "language": "c",
            "filepath": filepath or "",
            "depth": max_depth,  # Add overall depth to the metadata
            "chunk_units": [
                {
                    "type": n.type,
                    "start": n.start_line,
                    "end": n.end_line,
                    "name": n.name,
                    "depth": n.depth,
                    "context": n.get_full_context() if self.respect_hierarchy else None
                } for n in nodes
            ],
            "conditional_context": [n.parent for n in nodes if n.parent],
        }
        
        return {
            "id": _sha(filepath, first_node.start_line, first_node.end_line),
            "content": content,
            "metadata": metadata
        }
    
    def chunkify(self, code: str, filepath: Optional[str] = None) -> List[Dict]:
        """
        Chunk the given code using the CAST approach.
        
        Args:
            code: The source code to chunk
            filepath: Optional filepath for metadata
            
        Returns:
            List of chunks, each containing content and metadata
        """
        # Parse the source into nodes
        nodes = self.parser.parse_source(code, filepath)
        
        # If one symbol per chunk is requested, just return individual nodes
        if self.one_symbol_per_chunk:
            chunks = []
            for node in nodes:
                # Check if we need to split this node
                if self._should_split_node(node):
                    split_nodes = self._split_large_node(node)
                    for split_node in split_nodes:
                        content = self._format_node_content(split_node)
                        chunks.append(self._emit_chunk([split_node], content, filepath))
                else:
                    content = self._format_node_content(node)
                    chunks.append(self._emit_chunk([node], content, filepath))
            return chunks
        
        # For buffered mode, we need to implement the split-then-merge algorithm
        
        # Step 1: Split large nodes
        processed_nodes = []
        for node in nodes:
            if self._should_split_node(node):
                processed_nodes.extend(self._split_large_node(node))
            else:
                processed_nodes.append(node)
        
        # Step 2: Group related nodes
        node_groups = self._group_related_nodes(processed_nodes)
        
        # Step 3: Create chunks from groups
        chunks = []
        for group in node_groups:
            # Format content for the entire group
            contents = [self._format_node_content(node) for node in group]
            combined_content = "\n\n".join(contents)
            
            # Create a chunk
            chunks.append(self._emit_chunk(group, combined_content, filepath))
            
            # Handle overlap if needed
            if self.overlap_units and len(group) > self.overlap_units:
                overlap_group = group[-self.overlap_units:]
                overlap_contents = [self._format_node_content(node) for node in overlap_group]
                overlap_content = "\n\n".join(overlap_contents)
                chunks.append(self._emit_chunk(overlap_group, overlap_content, filepath))
        
        return chunks

def main():
    """Command-line interface for the CAST chunker."""
    ap = argparse.ArgumentParser(description="CAST: Chunking via Abstract Syntax Tree for Embedded C")
    ap.add_argument("inputs", nargs="+", help="Header files or folders")
    ap.add_argument("--max-chars", type=int, default=1600, help="Max characters per buffered chunk")
    ap.add_argument("--min-chars", type=int, default=200, help="Min characters per chunk")
    ap.add_argument("--overlap", type=int, default=1, help="Buffered mode: number of units to overlap")
    ap.add_argument("--one-per-symbol", action="store_true", help="Emit one chunk per AST unit")
    ap.add_argument("--no-comments", action="store_true", help="Exclude regular comments")
    ap.add_argument("--no-hierarchy", action="store_true", help="Ignore hierarchical relationships")
    ap.add_argument("--output", "-o", help="Output file (default: stdout)")
    args = ap.parse_args()
    
    chunker = CASTChunker(
        max_chunk_size=args.max_chars,
        min_chunk_size=args.min_chars,
        chunk_overlap_units=args.overlap,
        one_symbol_per_chunk=args.one_per_symbol,
        include_comments=not args.no_comments,
        respect_hierarchy=not args.no_hierarchy
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
from parsing.parsers.c_ast_parser_simple import _iter_header_files

if __name__ == "__main__":
    main()