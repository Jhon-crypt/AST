"""
Simple visualization script for C code chunks.
"""

import json
import argparse
import pathlib
import sys

# Import our parser
from parsing.parsers.c_ast_parser_simple import CChunker

def process_file(filepath: str, max_chars: int = 1600, one_per_symbol: bool = False) -> list:
    """Process a single file and return chunks."""
    chunker = CChunker(
        max_chunk_size=max_chars,
        chunk_overlap_units=1,
        one_symbol_per_chunk=one_per_symbol
    )
    
    try:
        source = pathlib.Path(filepath).read_text(encoding="utf-8", errors="ignore")
        return chunker.chunkify(source, filepath=filepath)
    except Exception as e:
        print(f"Error processing {filepath}: {e}", file=sys.stderr)
        return []

def visualize_chunks(chunks: list) -> None:
    """Visualize chunks using plain text."""
    for i, chunk in enumerate(chunks):
        print(f"Chunk {i+1}/{len(chunks)} (ID: {chunk['id']})")
        print("-" * 80)
        
        # Print metadata
        metadata = chunk.get("metadata", {})
        print(f"File: {metadata.get('filepath', 'N/A')}")
        
        # Print chunk units
        print("Units:")
        for unit in metadata.get("chunk_units", []):
            unit_type = unit.get("type", "unknown")
            name = unit.get("name", "")
            lines = f"{unit.get('start', '?')}-{unit.get('end', '?')}"
            print(f"  - {unit_type} {name} (lines {lines})")
        
        # Print conditional context
        if metadata.get("conditional_context"):
            print("Conditional context:")
            for ctx in metadata.get("conditional_context"):
                print(f"  - {ctx}")
        
        # Print content
        print("\nContent:")
        print(chunk.get("content", ""))
        print("=" * 80)
        print()

def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="Visualize C code chunks")
    parser.add_argument("input", help="Header file to process")
    parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    parser.add_argument("--output", "-o", help="Save chunks to JSON file")
    args = parser.parse_args()
    
    # Process the file
    chunks = process_file(args.input, args.max_chars, args.one_per_symbol)
    
    # Save to file if requested
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            for chunk in chunks:
                f.write(json.dumps(chunk, ensure_ascii=False) + "\n")
        print(f"Saved {len(chunks)} chunks to {args.output}")
    
    # Visualize chunks
    visualize_chunks(chunks)

if __name__ == "__main__":
    main()

