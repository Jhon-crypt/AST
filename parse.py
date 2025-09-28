#!/usr/bin/env python3
"""
Main entry point for the AST parser.

This script provides a unified interface to all parsers and visualization tools.
"""

import argparse
import sys
import os

def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="AST Parser for Embedded C Software",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Simple regex-based parser (no dependencies)
  python parse.py simple parsing/samples/sample_header.h
  
  # Tree-sitter based parser
  python parse.py tree-sitter parsing/samples/sample_header.h
  
  # CAST approach (structure-aware chunking)
  python parse.py cast parsing/samples/sample_header.h
  
  # Enhanced parser (complete file coverage)
  python parse.py enhanced parsing/samples/sample_header.h
  
  # Azure AI Search integration
  python parse.py azure path/to/headers --azure-endpoint URL --azure-key KEY
"""
    )
    
    subparsers = parser.add_subparsers(dest="command", help="Parser to use")
    
    # Simple parser
    simple_parser = subparsers.add_parser("simple", help="Regex-based parser (no dependencies)")
    simple_parser.add_argument("input", nargs="+", help="Header files or folders")
    simple_parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    simple_parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    simple_parser.add_argument("--output", "-o", help="Output file (default: stdout)")
    
    # Tree-sitter parser
    ts_parser = subparsers.add_parser("tree-sitter", help="Tree-sitter based parser")
    ts_parser.add_argument("input", nargs="+", help="Header files or folders")
    ts_parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    ts_parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    ts_parser.add_argument("--output", "-o", help="Output file (default: stdout)")
    ts_parser.add_argument("--plain", action="store_true", help="Use plain text output")
    
    # CAST parser
    cast_parser = subparsers.add_parser("cast", help="CAST approach (structure-aware chunking)")
    cast_parser.add_argument("input", nargs="+", help="Header files or folders")
    cast_parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    cast_parser.add_argument("--min-chars", type=int, default=200, help="Min characters per chunk")
    cast_parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    cast_parser.add_argument("--no-hierarchy", action="store_true", help="Ignore hierarchical relationships")
    cast_parser.add_argument("--output", "-o", help="Output file (default: stdout)")
    
    # Enhanced parser
    enhanced_parser = subparsers.add_parser("enhanced", help="Enhanced parser (complete file coverage)")
    enhanced_parser.add_argument("input", nargs="+", help="Header files or folders")
    enhanced_parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    enhanced_parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    enhanced_parser.add_argument("--semantic-only", action="store_true", help="Only include semantic elements")
    enhanced_parser.add_argument("--no-file-headers", action="store_true", help="Exclude file headers")
    enhanced_parser.add_argument("--no-section-headers", action="store_true", help="Exclude section headers")
    enhanced_parser.add_argument("--output", "-o", help="Output file (default: stdout)")
    
    # Azure indexer
    azure_parser = subparsers.add_parser("azure", help="Azure AI Search integration")
    azure_parser.add_argument("input", nargs="+", help="Header files or folders")
    azure_parser.add_argument("--azure-endpoint", help="Azure AI Search endpoint URL")
    azure_parser.add_argument("--azure-key", help="Azure AI Search admin key")
    azure_parser.add_argument("--index-name", default="c-code-index", help="Index name")
    azure_parser.add_argument("--openai-key", help="OpenAI API key for embeddings")
    azure_parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    azure_parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    azure_parser.add_argument("--output", "-o", help="Save chunks to file instead of uploading")
    azure_parser.add_argument("--embedding-model", default="text-embedding-3-small", help="Embedding model")
    azure_parser.add_argument("--embedding-dim", type=int, default=1536, help="Embedding dimension")
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    # Run the appropriate parser
    if args.command == "simple":
        from parsing.visualization.visualize_simple import main as run_simple
        sys.argv = [sys.argv[0]] + args.input
        if args.max_chars:
            sys.argv += ["--max-chars", str(args.max_chars)]
        if args.one_per_symbol:
            sys.argv += ["--one-per-symbol"]
        if args.output:
            sys.argv += ["--output", args.output]
        return run_simple()
    
    elif args.command == "tree-sitter":
        from parsing.visualization.visualize_chunks import main as run_ts
        sys.argv = [sys.argv[0]] + args.input
        if args.max_chars:
            sys.argv += ["--max-chars", str(args.max_chars)]
        if args.one_per_symbol:
            sys.argv += ["--one-per-symbol"]
        if args.output:
            sys.argv += ["--output", args.output]
        if args.plain:
            sys.argv += ["--plain"]
        return run_ts()
    
    elif args.command == "cast":
        from parsing.visualization.visualize_cast import main as run_cast
        sys.argv = [sys.argv[0]] + args.input
        if args.max_chars:
            sys.argv += ["--max-chars", str(args.max_chars)]
        if args.min_chars:
            sys.argv += ["--min-chars", str(args.min_chars)]
        if args.one_per_symbol:
            sys.argv += ["--one-per-symbol"]
        if args.no_hierarchy:
            sys.argv += ["--no-hierarchy"]
        if args.output:
            sys.argv += ["--output", args.output]
        return run_cast()
    
    elif args.command == "enhanced":
        from parsing.visualization.visualize_enhanced import main as run_enhanced
        sys.argv = [sys.argv[0]] + args.input
        if args.max_chars:
            sys.argv += ["--max-chars", str(args.max_chars)]
        if args.one_per_symbol:
            sys.argv += ["--one-per-symbol"]
        if args.semantic_only:
            sys.argv += ["--semantic-only"]
        if args.no_file_headers:
            sys.argv += ["--no-file-headers"]
        if args.no_section_headers:
            sys.argv += ["--no-section-headers"]
        if args.output:
            sys.argv += ["--output", args.output]
        return run_enhanced()
    
    elif args.command == "azure":
        from parsing.integration.azure_indexer import main as run_azure
        sys.argv = [sys.argv[0]] + args.input
        if args.azure_endpoint:
            sys.argv += ["--azure-endpoint", args.azure_endpoint]
        if args.azure_key:
            sys.argv += ["--azure-key", args.azure_key]
        if args.index_name:
            sys.argv += ["--index-name", args.index_name]
        if args.openai_key:
            sys.argv += ["--openai-key", args.openai_key]
        if args.max_chars:
            sys.argv += ["--max-chars", str(args.max_chars)]
        if args.one_per_symbol:
            sys.argv += ["--one-per-symbol"]
        if args.output:
            sys.argv += ["--output", args.output]
        if args.embedding_model:
            sys.argv += ["--embedding-model", args.embedding_model]
        if args.embedding_dim:
            sys.argv += ["--embedding-dim", str(args.embedding_dim)]
        return run_azure()
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
