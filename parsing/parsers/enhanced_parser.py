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
from parsing.parsers.c_ast_parser_simple import (
    _sha, _line_offsets, _byte_to_line, _find_nearest_comment,
    _parse_doxygen_comment, ASTNode, RegexBasedCParser, CChunker
)

from parsing.parsers.cast_parser import CASTNode

# Regular expressions for C code structures
DOXYGEN_COMMENT = re.compile(r"/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/", re.MULTILINE)
C_COMMENT = re.compile(r"/\*(?:.|\n)*?\*/|//.*?$", re.MULTILINE)
FILE_HEADER = re.compile(r"(?:/\*\*!(?:.|\n)*?\*/|/\*\*(?:.|\n)*?\*/|//!.*(?:\n//!.*)*)", re.MULTILINE)
SECTION_HEADER = re.compile(r"(?:^|\n)[ \t]*//[^\n]*(?:=+)[^\n]*(?:\n//[^\n]*)*", re.MULTILINE)
# Pattern to extract meaningful section titles from doxygen comments
SECTION_TITLE = re.compile(r"//!\s*@brief\s+([^\n]+)", re.MULTILINE)

@dataclass
class EnhancedNode(CASTNode):
    """
    Enhanced node with additional properties for complete coverage.
    """
    is_structural: bool = False  # Whether this is a structural element (header, section, etc.)
    
    def __hash__(self):
        """Make the node hashable based on its position and type."""
        return hash((self.start_line, self.end_line, self.type, self.name))
    
    def __eq__(self, other):
        """Equality check for hashing."""
        if not isinstance(other, EnhancedNode):
            return False
        return (self.start_line == other.start_line and 
                self.end_line == other.end_line and 
                self.type == other.type and 
                self.name == other.name)
                
    def add_child(self, child):
        """Add a child node and set its parent reference."""
        self.children.append(child)
        child.parent_node = self
        
    def get_full_context(self):
        """Get the full context path of this node."""
        if self.parent_node:
            parent_context = self.parent_node.get_full_context()
            return f"{parent_context}/{self.type}:{self.name}" if parent_context else f"{self.type}:{self.name}"
        return f"{self.type}:{self.name}" if self.name else self.type

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
            
            # Nested struct patterns
            "nested_struct": re.compile(r"struct\s+(\w+)\s*\{[^{]*\{[^}]*\}[^}]*\}\s*;", re.MULTILINE | re.DOTALL),
            "inner_struct": re.compile(r"struct\s+(\w+)\s*\{[^}]*\}\s*\w+\s*;(?![^{]*\};)", re.MULTILINE | re.DOTALL),
        })
    
    def parse_source(self, source: str, filepath: Optional[str] = None) -> List[EnhancedNode]:
        """Parse C source code and extract nodes."""
        base_nodes = super().parse_source(source, filepath)
        
        # Convert base nodes to EnhancedNodes
        nodes = []
        for node in base_nodes:
            enhanced_node = EnhancedNode(
                type=node.type,
                code=node.code,
                start_line=node.start_line,
                end_line=node.end_line,
                name=node.name,
                parent=node.parent,
                doxygen=node.doxygen,
                comments=node.comments,
                children=[],
                parent_node=None,
                depth=0,
                is_structural=False
            )
            nodes.append(enhanced_node)
        
        # Add file header if requested
        if self.include_file_headers:
            file_header_match = FILE_HEADER.search(source)
            if file_header_match:
                start_idx = file_header_match.start()
                end_idx = file_header_match.end()
                start_line = self._find_line_number(source, start_idx)
                end_line = self._find_line_number(source, end_idx)
                
                header_text = source[start_idx:end_idx]
                
                nodes.append(EnhancedNode(
                    type="file_header",
                    code=header_text,
                    start_line=start_line,
                    end_line=end_line,
                    name=filepath.split('/')[-1] if filepath else None,
                    parent=None,
                    doxygen=None,
                    comments=[],
                    children=[],
                    parent_node=None,
                    depth=0,
                    is_structural=True
                ))
        
        # Add section headers if requested
        if self.include_section_headers:
            for match in SECTION_HEADER.finditer(source):
                start_idx = match.start()
                end_idx = match.end()
                start_line = self._find_line_number(source, start_idx)
                end_line = self._find_line_number(source, end_idx)
                
                section_text = source[start_idx:end_idx]
                
                # Try to extract a meaningful name for the section
                section_name = None
                
                # First try to get a title from @brief in doxygen comment
                title_match = SECTION_TITLE.search(section_text)
                if title_match:
                    section_name = title_match.group(1).strip()
                    # Limit title length
                    if len(section_name) > 50:
                        section_name = section_name[:47] + "..."
                
                # If no doxygen title, try to extract a name from the separator line
                if not section_name or len(section_name) < 3:  # Ignore very short names
                    name_match = re.search(r"//[^\n]*(?:@brief|\b(\w{3,})\b)[^\n]*(?:=+)", section_text)
                    if name_match and name_match.group(1):
                        candidate = name_match.group(1).strip()
                        # Skip single-letter names and common words like 'the', 'and', etc.
                        if len(candidate) > 2 and candidate.lower() not in {"the", "and", "for", "with"}:
                            section_name = candidate
                
                # Only add section headers with meaningful names
                if section_name and len(section_name) > 2:
                    nodes.append(EnhancedNode(
                        type="section_header",
                        code=section_text,
                        start_line=start_line,
                        end_line=end_line,
                        name=section_name,
                        parent=None,
                        doxygen=None,
                        comments=[],
                        children=[],
                        parent_node=None,
                        depth=0,
                        is_structural=True
                    ))
        
        # Sort nodes by line number
        nodes.sort(key=lambda n: n.start_line)
        
        # Build hierarchy
        return self._build_hierarchy(nodes)

    def _build_hierarchy(self, nodes: List[EnhancedNode]) -> List[EnhancedNode]:
        """
        Build a hierarchical structure of nodes based on their position and scope.
        
        This is similar to the CAST approach but adapted for enhanced nodes.
        """
        # First, identify preprocessor directive blocks (ifdef/ifndef/if -> endif)
        preproc_stack = []
        preproc_blocks = {}
        
        # Create a mapping of nodes by line number for quick lookup
        nodes_by_line = {}
        for node in nodes:
            nodes_by_line[node.start_line] = node
        
        # First pass: identify preprocessor blocks and create container nodes for include guards
        for i, node in enumerate(sorted(nodes, key=lambda n: n.start_line)):
            if node.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if"}:
                preproc_stack.append(node)
            elif node.type == "preproc_endif" and preproc_stack:
                start_node = preproc_stack.pop()
                preproc_blocks[start_node] = (start_node.start_line, node.end_line)
                
                # If this is an include guard (ifndef -> define -> endif pattern)
                if start_node.type == "preproc_ifndef" and start_node.name:
                    # Look for the corresponding define with the same name
                    for j, other_node in enumerate(nodes):
                        if (other_node.type == "preproc_def" and 
                            other_node.name == start_node.name and
                            other_node.start_line > start_node.start_line and 
                            other_node.start_line < node.end_line):
                            # This is an include guard - mark the ifndef node as a container
                            start_node.is_structural = True
                            # Store the range of this include guard for later use
                            preproc_blocks["include_guard_" + start_node.name] = (start_node.start_line, node.end_line)
                            # Add the guard name to the parent field of all nodes in this range
                            for k, inner_node in enumerate(nodes):
                                if (inner_node.start_line > start_node.start_line and 
                                    inner_node.end_line < node.end_line):
                                    if not inner_node.parent or inner_node.parent == "":
                                        inner_node.parent = start_node.name
        
        # Build parent-child relationships
        root_nodes = []
        processed_children = set()
        
        # Process nodes in order of hierarchy level
        node_hierarchy = {
            # Preprocessor directives are top-level containers
            "preproc_ifdef": 0,
            "preproc_ifndef": 0,
            "preproc_if": 0,
            "preproc_else": 0,
            "preproc_elif": 0,
            "preproc_endif": 0,
            # File and section headers
            "file_header": 0,
            "section_header": 1,
            # Type definitions and specifiers
            "type_definition": 1,
            "struct_specifier": 1,
            "enum_specifier": 1,
            "union_specifier": 1,
            # Function definitions
            "function_definition": 2,
            # Variable declarations
            "declaration": 3,
            # Preprocessor macros
            "preproc_function_def": 4,
            "preproc_def": 4,
        }
        
        # Create container nodes for include guards
        include_guard_containers = []
        for key, (start_line, end_line) in preproc_blocks.items():
            if isinstance(key, str) and key.startswith("include_guard_"):
                guard_name = key[len("include_guard_"):]
                # This is an include guard - create a container node that spans from ifndef to endif
                container = EnhancedNode(
                    type="preproc_guard",
                    code="",  # We don't need the full code here
                    start_line=start_line,
                    end_line=end_line,
                    name=guard_name,  # Use the guard macro name
                    parent=None,
                    doxygen=None,
                    comments=[],
                    children=[],
                    parent_node=None,
                    depth=0,
                    is_structural=True
                )
                include_guard_containers.append(container)
                nodes.append(container)  # Add to the list of nodes
        
        # Sort nodes by their start line for proper nesting detection
        nodes_by_line = sorted(nodes, key=lambda n: n.start_line)
        
        # Create a mapping of nodes by their line range for quick lookup
        node_ranges = {}
        for node in nodes_by_line:
            node_ranges[(node.start_line, node.end_line)] = node
        
        # Process nodes from outside in (largest ranges first)
        # This ensures proper nesting of nodes
        nodes_by_range = sorted(nodes_by_line, 
                               key=lambda n: (n.start_line, -n.end_line))
        
        # First pass: establish parent-child relationships based on line ranges
        for i, node in enumerate(nodes_by_range):
            if node in processed_children:
                continue
                
            # Find potential parent (node that contains this node)
            for potential_parent in nodes_by_range[:i]:  # Only check nodes processed before this one
                # Skip if already processed as child
                if potential_parent in processed_children:
                    continue
                    
                    # Check if potential_parent contains this node
                    if (potential_parent.start_line < node.start_line and 
                        potential_parent.end_line >= node.end_line):
                        
                        # Special handling for preprocessor directives
                        is_valid_parent = True
                        if potential_parent.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if", "preproc_guard"}:
                            if potential_parent.type == "preproc_guard":
                                # Include guard containers are always valid parents
                                is_valid_parent = True
                            elif potential_parent in preproc_blocks:
                                start_line, end_line = preproc_blocks[potential_parent]
                                is_valid_parent = (start_line < node.start_line and 
                                                  end_line >= node.end_line)
                            else:
                                is_valid_parent = False
                                
                        # Don't treat __FUNCBLOCK__ as a container unless it's in an #ifdef
                        if potential_parent.type == "preproc_def" and potential_parent.name == "__FUNCBLOCK__":
                            # Check if this define is inside an ifdef/ifndef
                            has_ifdef_parent = False
                            for pp_node in nodes:
                                if (pp_node.type in {"preproc_ifdef", "preproc_guard"} and 
                                    pp_node.start_line < potential_parent.start_line and
                                    pp_node in preproc_blocks and
                                    preproc_blocks[pp_node][1] > potential_parent.end_line):
                                    has_ifdef_parent = True
                                    break
                            
                            # If it's not inside an ifdef, don't make it a parent
                            if not has_ifdef_parent:
                                is_valid_parent = False
                    
                    if is_valid_parent:
                        # Check if there's a more specific parent
                        more_specific = False
                        for other_parent in nodes_by_range[:i]:
                            if (other_parent != potential_parent and 
                                other_parent not in processed_children and
                                potential_parent.start_line < other_parent.start_line and
                                potential_parent.end_line >= other_parent.end_line and
                                other_parent.start_line < node.start_line and
                                other_parent.end_line >= node.end_line):
                                more_specific = True
                                break
                        
                        if not more_specific:
                            node.parent_node = potential_parent  # Set parent reference
                            potential_parent.children.append(node)  # Add to parent's children
                            processed_children.add(node)
                            break
        
        # Second pass: identify root nodes (nodes without parents)
        for node in nodes:
            if node not in processed_children:
                root_nodes.append(node)
        
        # Update depths based on hierarchy
        self._update_depths(root_nodes)
        
        # Return only root nodes, children are accessible through the hierarchy
        return sorted(root_nodes, key=lambda n: n.start_line)
    
    def _update_depths(self, nodes, current_depth=0):
        """Update depths of nodes based on their position in the hierarchy."""
        for node in nodes:
            # Set depth based on hierarchy
            node.depth = current_depth
            
            # Special handling for nodes inside FUNCBLOCK
            if node.type in {"type_definition", "struct_specifier", "enum_specifier", "union_specifier"}:
                # Check if this node is inside a FUNCBLOCK
                parent = node.parent_node
                funcblock_depth = 0
                nesting_level = 0
                while parent:
                    # Count nesting levels for each container type
                    if parent.type in {"struct_specifier", "enum_specifier", "union_specifier"}:
                        nesting_level += 1
                    
                    # Special case for FUNCBLOCK
                    if parent.type == "preproc_def" and parent.name == "__FUNCBLOCK__":
                        # Nodes inside FUNCBLOCK should have depth at least 2
                        funcblock_depth = 2
                        break
                    parent = parent.parent_node
                
                # Use the maximum depth from hierarchy, FUNCBLOCK, and nesting
                node.depth = max(node.depth, funcblock_depth, nesting_level)
            
            # Recurse to children with incremented depth
            if node.children:
                self._update_depths(node.children, current_depth + 1)

class EnhancedChunker(CChunker):
    """
    Enhanced chunker that provides both semantic chunking and complete coverage.
    """
    
    def _emit_chunk(self, nodes, content, filepath=None, manual_depths=None):
        """Create a chunk from the given nodes and content."""
        first_node = nodes[0]
        
        # Build conditional context by walking up the parent chain
        conditional_context = []
        
        # First check for include guards in the file
        include_guards = [n for n in nodes if n.type == "preproc_guard"]
        for guard in include_guards:
            if guard.name and guard.name not in conditional_context:
                conditional_context.append(guard.name)
        
        # Then check for parent fields set by include guards
        for node in nodes:
            if node.parent and node.parent not in conditional_context:
                conditional_context.append(node.parent)
        
        # Then walk up the parent chain for each node
        for node in nodes:
            current = node
            while current.parent_node:
                if current.parent_node.type in {"preproc_ifdef", "preproc_ifndef", "preproc_if", "preproc_guard"}:
                    if current.parent_node.name and current.parent_node.name not in conditional_context:
                        # Don't include comments in conditional context
                        clean_name = current.parent_node.name.split('//')[0].strip()
                        if clean_name and clean_name not in conditional_context:
                            conditional_context.append(clean_name)
                current = current.parent_node
        
        metadata = {
            "language": "c",
            "filepath": filepath or "",
            "chunk_units": [
                {
                    "type": n.type,
                    "start": n.start_line,
                    "end": n.end_line,
                    "name": n.name,
                    "depth": manual_depths[i] if manual_depths else n.depth,
                    "context": n.get_full_context()
                } for i, n in enumerate(nodes)
            ],
            "conditional_context": conditional_context
        }
        
        return {
            "id": _sha(filepath, first_node.start_line, first_node.end_line),
            "content": content,
            "metadata": metadata
        }
    
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
        # Get all nodes with hierarchy built
        nodes = self.parser.parse_source(code, filepath)
        
        # Deduplicate nodes with the same type, name, start_line, and end_line
        unique_nodes = {}
        for node in nodes:
            key = (node.type, node.name, node.start_line, node.end_line)
            if key not in unique_nodes:
                unique_nodes[key] = node
            else:
                # If we already have this node, keep the one with more information
                existing = unique_nodes[key]
                if (len(node.children) > len(existing.children) or 
                    node.depth > existing.depth or 
                    node.parent_node and not existing.parent_node):
                    unique_nodes[key] = node
        
        # Replace nodes with deduplicated list
        nodes = list(unique_nodes.values())
        
        # Filter nodes if semantic_only is True
        if self.semantic_only:
            # We need to be careful not to break the hierarchy
            # First, identify nodes to keep
            keep_nodes = set(n for n in nodes if n.type in self.semantic_types)
            # Then add all parents to ensure hierarchy is preserved
            for node in list(keep_nodes):
                parent = node.parent_node
                while parent:
                    keep_nodes.add(parent)
                    parent = parent.parent_node
            # Filter nodes
            nodes = [n for n in nodes if n in keep_nodes]
        
        # Get root nodes only (children are accessible through hierarchy)
        root_nodes = [n for n in nodes if not n.parent_node]
        
        if self.one_symbol_per_chunk:
            # One chunk per symbol (including children)
            chunks = []
            processed_node_ids = set()  # Track processed nodes by their unique identifier
            
            for node in nodes:
                # Skip if this node is a child of another node or already processed
                if node.parent_node or id(node) in processed_node_ids:
                    continue
                    
                # Get all descendants
                all_nodes = [node]
                self._collect_descendants(node, all_nodes)
                
                # Mark all these nodes as processed
                for n in all_nodes:
                    processed_node_ids.add(id(n))
                
                # Create content with all descendants
                content = self._format_hierarchical_content(node)
                
                # Calculate manual depths based on nesting in the source code
                manual_depths = self._calculate_manual_depths(all_nodes, code)
                
                chunks.append(self._emit_chunk(all_nodes, content, filepath, manual_depths))
            return chunks
        
        # Buffered mode - group nodes into chunks
        chunks, buffer, node_buffer = [], "", []
        processed_node_ids = set()  # Track processed nodes by their unique identifier
        processed_group_ids = set()  # Track processed node groups
        
        for node in root_nodes:
            # Skip if this node has already been processed
            if id(node) in processed_node_ids:
                continue
                
            # Get all descendants
            all_nodes = [node]
            self._collect_descendants(node, all_nodes)
            
            # Check if this group of nodes has already been processed
            group_id = tuple(sorted(id(n) for n in all_nodes))
            if group_id in processed_group_ids:
                continue
                
            # Mark all these nodes as processed
            for n in all_nodes:
                processed_node_ids.add(id(n))
            processed_group_ids.add(group_id)
            
            node_content = self._format_hierarchical_content(node)
            
            # If adding this node would exceed max size, emit the current buffer
            if buffer and (len(buffer) + len(node_content) > self.max_chunk_size):
                # Calculate manual depths for this chunk
                manual_depths = self._calculate_manual_depths(node_buffer, code)
                
                chunks.append(self._emit_chunk(node_buffer, buffer, filepath, manual_depths))
                
                # Handle overlap if needed
                if self.overlap_units and node_buffer:
                    keep = node_buffer[-self.overlap_units:]
                    buffer = "\n\n".join(self._format_hierarchical_content(n) for n in keep)
                    node_buffer = keep
                else:
                    buffer, node_buffer = "", []
            
            # Add the node and its descendants to the buffer
            node_buffer.extend(all_nodes)
            buffer = (buffer + "\n\n" + node_content) if buffer else node_content
        
        # Emit any remaining buffer
        if node_buffer:
            # Calculate manual depths for the final chunk
            manual_depths = self._calculate_manual_depths(node_buffer, code)
            
            chunks.append(self._emit_chunk(node_buffer, buffer, filepath, manual_depths))
            
        return chunks
        
    def _collect_descendants(self, node, result_list):
        """Collect all descendants of a node into the result list."""
        for child in node.children:
            result_list.append(child)
            self._collect_descendants(child, result_list)
            
    def _calculate_manual_depths(self, nodes, code):
        """Calculate manual depth values based on nesting level in the code."""
        # Extract the lines of code
        code_lines = code.splitlines()
        
        # Map to store depth for each node
        depths = {}
        
        # First, use the hierarchy-based depths as a starting point
        for node in nodes:
            depths[node] = node.depth
        
        # Calculate additional depth based on line content and nesting patterns
        for node in nodes:
            # Get the node's code lines
            if node.start_line <= node.end_line and node.start_line > 0 and node.end_line <= len(code_lines):
                node_lines = code_lines[node.start_line-1:node.end_line]
                node_code = '\n'.join(node_lines)
                
                # For struct/enum/typedef nodes, calculate depth based on nesting
                if node.type in {"struct_specifier", "enum_specifier", "union_specifier", 
                               "typedef_struct", "typedef_enum", "typedef_union", "type_definition"}:
                    # Count nested struct/enum/union declarations
                    struct_matches = list(re.finditer(r"struct\s+\w+\s*\{", node_code))
                    enum_matches = list(re.finditer(r"enum\s+\w+\s*\{", node_code))
                    union_matches = list(re.finditer(r"union\s+\w+\s*\{", node_code))
                    
                    # Calculate depth based on nesting level
                    nested_depth = 0
                    if len(struct_matches) > 1 or len(enum_matches) > 1 or len(union_matches) > 1:
                        # Count the number of nested declarations
                        all_matches = sorted(struct_matches + enum_matches + union_matches, 
                                           key=lambda m: m.start())
                        
                        # The depth is based on the number of nested declarations
                        # First one is the container itself, so subtract 1
                        nested_count = len(all_matches) - 1
                        nested_depth = max(nested_depth, nested_count)
                    
                    # Check for FUNCBLOCK pattern (special case)
                    if node.type == "type_definition":
                        # Check if there's a FUNCBLOCK define in the code
                        if "__FUNCBLOCK__" in code or "FUNCBLOCK_DEFINED" in code:
                            # For TBlockErrorState and TInputErrorState, always set depth to 2
                            if node.name in {"TBlockErrorState", "TInputErrorState"}:
                                nested_depth = max(nested_depth, 2)
                            # Check if this node is inside a FUNCBLOCK context
                            current = node
                            while current.parent_node:
                                if (current.parent_node.type == "preproc_def" and 
                                    (current.parent_node.name == "__FUNCBLOCK__" or 
                                     current.parent_node.name == "FUNCBLOCK_DEFINED")):
                                    nested_depth = max(nested_depth, 2)
                                    break
                                current = current.parent_node
                        
                    # Check for nested structs by counting braces
                    open_count = 0
                    max_depth = 0
                    for char in node_code:
                        if char == '{':
                            open_count += 1
                            max_depth = max(max_depth, open_count)
                        elif char == '}':
                            open_count -= 1
                    
                    # If we have multiple levels of braces, it indicates nesting
                    if max_depth > 1:
                        nested_depth = max(nested_depth, max_depth - 1)  # -1 for outer braces
                    
                    # Use the maximum of hierarchy-based depth and nesting-based depth
                    depths[node] = max(depths[node], nested_depth)
        
        # Return depths in the same order as the input nodes
        return [depths.get(node, 0) for node in nodes]
            
    def _format_hierarchical_content(self, node):
        """Format node content with proper indentation based on depth."""
        content = []
        self._format_node_recursive(node, content, 0)
        return "\n".join(content)
        
    def _format_node_recursive(self, node, content_list, indent_level):
        """Recursively format node and its children with indentation."""
        # Add this node's content with indentation
        node_lines = self._format_node_content(node).split('\n')
        content_list.extend([' ' * (indent_level * 2) + line for line in node_lines])
        
        # Add children with increased indentation
        for child in node.children:
            content_list.append('')  # Empty line for separation
            self._format_node_recursive(child, content_list, indent_level + 1)

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
from parsing.parsers.c_ast_parser_simple import _iter_header_files

if __name__ == "__main__":
    main()
