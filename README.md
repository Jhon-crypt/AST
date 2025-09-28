# Embedded C AST Parser

This project provides an AST (Abstract Syntax Tree) parser for embedded C software, designed to extract and chunk code elements from header files. It's particularly useful for preparing code for LLM+RAG (Retrieval-Augmented Generation) systems that generate automated test cases for embedded C software.

## Features

- Parse embedded C header files into semantic chunks
- Extract functions, structs, enums, typedefs, macros, and comments
- Associate Doxygen documentation with code elements
- Handle conditional compilation directives (#ifdef, #ifndef, #if)
- Generate embeddings for chunks using OpenAI models
- Upload chunks to Azure AI Search for RAG applications
- **NEW**: CAST approach for structure-aware chunking

## Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/embedded-c-ast-parser.git
cd embedded-c-ast-parser

# Install dependencies
pip install -r requirements.txt
```

## Usage

### Basic Parsing

Parse a header file and visualize the chunks:

```bash
# Using Tree-sitter based parser (requires dependencies)
python parsing/visualize_chunks.py parsing/sample_header.h

# Using regex-based parser (no dependencies)
python parsing/visualize_simple.py parsing/sample_header.h

# Using CAST approach (enhanced structure-aware chunking)
python parsing/visualize_cast.py parsing/sample_header.h
```

Options:
- `--max-chars 2000`: Set maximum characters per chunk
- `--one-per-symbol`: Create one chunk per symbol instead of buffering
- `--output chunks.json`: Save chunks to a JSON file
- `--plain`: Use plain text output instead of rich formatting (for visualize_chunks.py)

### CAST Approach (NEW)

The CAST (Chunking via Abstract Syntax Tree) approach enhances code chunking with structure-aware processing:

```bash
python parsing/visualize_cast.py parsing/sample_header.h
```

CAST-specific options:
- `--min-chars 200`: Set minimum characters per chunk
- `--no-hierarchy`: Ignore hierarchical relationships between code elements
- `--one-per-symbol`: Create one chunk per symbol instead of using split-then-merge

### Azure AI Search Integration

Process headers and upload to Azure AI Search:

```bash
python parsing/azure_indexer.py path/to/headers \
    --azure-endpoint https://your-service.search.windows.net \
    --azure-key your-admin-key \
    --index-name c-code-index \
    --openai-key your-openai-key
```

Options:
- `--max-chars 2000`: Set maximum characters per chunk
- `--one-per-symbol`: Create one chunk per symbol
- `--output chunks.json`: Save chunks to a file instead of uploading
- `--embedding-model model-name`: Specify embedding model
- `--embedding-dim 1536`: Specify embedding dimension

## How It Works

### AST Parsing Approaches

This project provides three different parsing approaches:

1. **Tree-sitter Based Parser** (`c_ast_parser.py`):
   - Uses Tree-sitter to generate an accurate AST
   - Provides robust parsing of complex C constructs
   - Requires external dependencies

2. **Regex Based Parser** (`c_ast_parser_simple.py`):
   - Uses regular expressions to extract code elements
   - No external dependencies
   - Simpler but less accurate for complex code

3. **CAST Approach** (`cast_parser.py`):
   - Enhanced structure-aware chunking
   - Implements split-then-merge algorithm
   - Preserves hierarchical relationships
   - Based on the research paper ["CAST: Enhancing Code Retrieval-Augmented Generation with Structural Chunking via Abstract Syntax Tree"](https://arxiv.org/pdf/2506.15655)

### CAST Chunking Process

The CAST approach follows these steps:

1. **Hierarchical Parsing**: Code is parsed into a hierarchical structure that preserves parent-child relationships
2. **Split Step**: Large nodes are recursively split into smaller chunks that respect syntax boundaries
3. **Merge Step**: Related nodes are merged together to form semantically coherent chunks
4. **Context Preservation**: Each chunk maintains its structural context within the codebase
5. **Metadata Enrichment**: Enhanced metadata includes hierarchical information and context

### Azure AI Search Integration

1. **Embedding Generation**: Each chunk is converted to a vector embedding using OpenAI models
2. **Index Creation**: A search index with vector search capabilities is created in Azure AI Search
3. **Document Upload**: Chunks with their embeddings are uploaded to the search index
4. **RAG Ready**: The indexed chunks can now be used in a RAG system for generating test cases

## Project Structure

- `parsing/c_ast_parser.py`: Tree-sitter based AST parser
- `parsing/c_ast_parser_simple.py`: Regex-based parser (no dependencies)
- `parsing/cast_parser.py`: Enhanced CAST implementation
- `parsing/azure_indexer.py`: Azure AI Search integration
- `parsing/visualize_chunks.py`: Tool to visualize Tree-sitter parsed chunks
- `parsing/visualize_simple.py`: Tool to visualize regex-based parsed chunks
- `parsing/visualize_cast.py`: Tool to visualize CAST chunks
- `parsing/sample_header.h`: Example header file for testing

## Customization

You can customize the parsers in several ways:

- Tree-sitter parser: Modify the `TARGET_TYPES` set in the `EmbeddedCParser` class
- Regex parser: Enhance the `PATTERNS` dictionary in the `RegexBasedCParser` class
- CAST parser: Adjust the `NODE_HIERARCHY` dictionary to change how nodes are related

## References

The CAST approach is based on research from:

- ["CAST: Enhancing Code Retrieval-Augmented Generation with Structural Chunking via Abstract Syntax Tree"](https://arxiv.org/pdf/2506.15655) (Zhang et al., 2024)

## License

MIT