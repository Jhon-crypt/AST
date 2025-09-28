# Embedded C AST Parser

This project provides an AST (Abstract Syntax Tree) parser for embedded C software, designed to extract and chunk code elements from header files. It's particularly useful for preparing code for LLM+RAG (Retrieval-Augmented Generation) systems that generate automated test cases for embedded C software.

## Features

- Parse embedded C header files into semantic chunks
- Extract functions, structs, enums, typedefs, macros, and comments
- Associate Doxygen documentation with code elements
- Handle conditional compilation directives (#ifdef, #ifndef, #if)
- Generate embeddings for chunks using OpenAI models
- Upload chunks to Azure AI Search for RAG applications

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
python parsing/visualize_chunks.py parsing/sample_header.h
```

Options:
- `--max-chars 2000`: Set maximum characters per chunk
- `--one-per-symbol`: Create one chunk per symbol instead of buffering
- `--output chunks.json`: Save chunks to a JSON file
- `--plain`: Use plain text output instead of rich formatting

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

### AST Parsing

The parser uses the Tree-sitter library to generate an Abstract Syntax Tree (AST) from C code. This approach provides several advantages over regex-based parsing:

1. **Structural understanding**: The AST represents the code's structure, making it easier to identify functions, structs, etc.
2. **Context-aware**: The parser understands C syntax and can correctly handle nested structures.
3. **Robust**: Works with complex code patterns that would be difficult to parse with regex.

### Chunking Process

1. **AST Generation**: The code is parsed into an AST using Tree-sitter
2. **Node Extraction**: The parser traverses the AST to find target nodes (functions, structs, etc.)
3. **Comment Association**: Doxygen and regular comments are associated with their respective code elements
4. **Chunk Creation**: Nodes are either emitted as individual chunks or buffered together based on configuration
5. **Metadata Enrichment**: Each chunk includes metadata about the contained code elements

### Azure AI Search Integration

1. **Embedding Generation**: Each chunk is converted to a vector embedding using OpenAI models
2. **Index Creation**: A search index with vector search capabilities is created in Azure AI Search
3. **Document Upload**: Chunks with their embeddings are uploaded to the search index
4. **RAG Ready**: The indexed chunks can now be used in a RAG system for generating test cases

## Project Structure

- `parsing/c_ast_parser.py`: Core AST parser and chunker
- `parsing/azure_indexer.py`: Azure AI Search integration
- `parsing/visualize_chunks.py`: Tool to visualize parsed chunks
- `parsing/sample_header.h`: Example header file for testing

## Customization

You can customize the parser by modifying the `TARGET_TYPES` set in the `EmbeddedCParser` class to include or exclude specific AST node types. The chunking behavior can be adjusted through the `CChunker` parameters.

## License

MIT
