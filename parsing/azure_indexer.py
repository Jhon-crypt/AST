"""
Azure AI Search indexer for C code chunks.

This script demonstrates how to:
1. Parse C header files into semantic chunks
2. Generate embeddings for each chunk
3. Upload the chunks to Azure AI Search
"""

import os
import json
import argparse
import pathlib
from typing import List, Dict, Any

# Import our AST parser
from c_ast_parser import CChunker

# Azure AI Search client
from azure.search.documents import SearchClient
from azure.search.documents.indexes import SearchIndexClient
from azure.search.documents.indexes.models import (
    SearchIndex,
    SearchField,
    SearchFieldDataType,
    SimpleField,
    SearchableField,
    VectorSearch,
    VectorSearchProfile,
    HnswAlgorithmConfiguration,
    VectorSearchAlgorithmKind,
    VectorSearchAlgorithmMetric,
)
from azure.core.credentials import AzureKeyCredential

# OpenAI for embeddings
import openai

class AzureSearchIndexer:
    """
    Class for indexing C code chunks in Azure AI Search.
    """
    
    def __init__(
        self, 
        search_service_endpoint: str,
        search_admin_key: str,
        index_name: str,
        embedding_model: str = "text-embedding-ada-002",
        embedding_dimension: int = 1536,
        openai_api_key: str = None,
        openai_api_base: str = None
    ):
        """
        Initialize the indexer.
        
        Args:
            search_service_endpoint: Azure Search service endpoint URL
            search_admin_key: Azure Search admin key
            index_name: Name of the search index
            embedding_model: OpenAI embedding model name
            embedding_dimension: Dimension of the embedding vectors
            openai_api_key: OpenAI API key (if using OpenAI directly)
            openai_api_base: OpenAI API base URL (if using Azure OpenAI)
        """
        self.search_credential = AzureKeyCredential(search_admin_key)
        self.index_client = SearchIndexClient(
            endpoint=search_service_endpoint,
            credential=self.search_credential
        )
        self.search_client = SearchClient(
            endpoint=search_service_endpoint,
            index_name=index_name,
            credential=self.search_credential
        )
        self.index_name = index_name
        self.embedding_model = embedding_model
        self.embedding_dimension = embedding_dimension
        
        # Configure OpenAI client
        if openai_api_key:
            openai.api_key = openai_api_key
        if openai_api_base:
            openai.api_base = openai_api_base
    
    def create_search_index(self) -> None:
        """Create the search index if it doesn't exist."""
        if self.index_name not in [index.name for index in self.index_client.list_indexes()]:
            print(f"Creating index: {self.index_name}")
            
            # Define fields for the index
            fields = [
                SimpleField(name="id", type=SearchFieldDataType.String, key=True),
                SearchableField(name="content", type=SearchFieldDataType.String, 
                                analyzer_name="en.microsoft"),
                SimpleField(name="filepath", type=SearchFieldDataType.String, filterable=True, 
                            sortable=True),
                SimpleField(name="language", type=SearchFieldDataType.String, filterable=True),
                SimpleField(name="chunk_type", type=SearchFieldDataType.Collection(SearchFieldDataType.String), 
                            filterable=True),
                SimpleField(name="name", type=SearchFieldDataType.Collection(SearchFieldDataType.String), 
                            filterable=True, searchable=True),
                SimpleField(name="conditional_context", type=SearchFieldDataType.Collection(SearchFieldDataType.String), 
                            filterable=True),
                SearchField(name="embedding", type=SearchFieldDataType.Collection(SearchFieldDataType.Single),
                            vector_search_dimensions=self.embedding_dimension, 
                            vector_search_profile_name="embedding_profile"),
            ]
            
            # Configure vector search
            vector_search = VectorSearch(
                profiles=[
                    VectorSearchProfile(
                        name="embedding_profile",
                        algorithm_configuration_name="embedding_config",
                    )
                ],
                algorithms=[
                    HnswAlgorithmConfiguration(
                        name="embedding_config",
                        kind=VectorSearchAlgorithmKind.HNSW,
                        parameters={
                            "m": 4,
                            "efConstruction": 400,
                            "efSearch": 500,
                            "metric": VectorSearchAlgorithmMetric.COSINE
                        }
                    )
                ]
            )
            
            # Create the index
            index = SearchIndex(name=self.index_name, fields=fields, vector_search=vector_search)
            self.index_client.create_index(index)
            print(f"Index {self.index_name} created successfully")
        else:
            print(f"Index {self.index_name} already exists")
    
    def generate_embedding(self, text: str) -> List[float]:
        """Generate an embedding for the given text."""
        try:
            response = openai.Embedding.create(
                input=text,
                model=self.embedding_model
            )
            return response["data"][0]["embedding"]
        except Exception as e:
            print(f"Error generating embedding: {e}")
            # Return a zero vector as fallback
            return [0.0] * self.embedding_dimension
    
    def upload_chunks(self, chunks: List[Dict]) -> None:
        """Upload chunks to the search index."""
        if not chunks:
            print("No chunks to upload")
            return
        
        # Prepare documents for indexing
        documents = []
        for chunk in chunks:
            # Extract metadata
            metadata = chunk.get("metadata", {})
            filepath = metadata.get("filepath", "")
            
            # Extract chunk types and names
            chunk_types = []
            names = []
            for unit in metadata.get("chunk_units", []):
                chunk_types.append(unit.get("type", ""))
                if unit.get("name"):
                    names.append(unit.get("name"))
            
            # Generate embedding
            embedding = self.generate_embedding(chunk["content"])
            
            # Create document
            document = {
                "id": chunk["id"],
                "content": chunk["content"],
                "filepath": filepath,
                "language": metadata.get("language", "c"),
                "chunk_type": chunk_types,
                "name": names,
                "conditional_context": metadata.get("conditional_context", []),
                "embedding": embedding
            }
            
            documents.append(document)
        
        # Upload in batches of 1000 (Azure Search limit)
        batch_size = 1000
        for i in range(0, len(documents), batch_size):
            batch = documents[i:i+batch_size]
            try:
                self.search_client.upload_documents(batch)
                print(f"Uploaded batch {i//batch_size + 1}/{(len(documents)-1)//batch_size + 1}")
            except Exception as e:
                print(f"Error uploading batch: {e}")

def process_headers(
    input_paths: List[str],
    max_chunk_size: int = 1600,
    chunk_overlap: int = 1,
    one_per_symbol: bool = False
) -> List[Dict]:
    """
    Process header files and generate chunks.
    
    Args:
        input_paths: List of files or directories to process
        max_chunk_size: Maximum size of a chunk in characters
        chunk_overlap: Number of units to overlap between chunks
        one_per_symbol: Whether to create one chunk per symbol
        
    Returns:
        List of chunks
    """
    chunker = CChunker(
        max_chunk_size=max_chunk_size,
        chunk_overlap_units=chunk_overlap,
        one_symbol_per_chunk=one_per_symbol,
        include_comments=True
    )
    
    chunks = []
    for path in input_paths:
        path = pathlib.Path(path)
        if path.is_file() and path.suffix.lower() in (".h", ".hpp"):
            try:
                print(f"Processing {path}")
                source = path.read_text(encoding="utf-8", errors="ignore")
                file_chunks = chunker.chunkify(source, filepath=str(path))
                chunks.extend(file_chunks)
            except Exception as e:
                print(f"Error processing {path}: {e}")
        elif path.is_dir():
            for file_path in path.rglob("*.h") or path.rglob("*.hpp"):
                try:
                    print(f"Processing {file_path}")
                    source = file_path.read_text(encoding="utf-8", errors="ignore")
                    file_chunks = chunker.chunkify(source, filepath=str(file_path))
                    chunks.extend(file_chunks)
                except Exception as e:
                    print(f"Error processing {file_path}: {e}")
    
    return chunks

def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(description="Index C header files in Azure AI Search")
    parser.add_argument("inputs", nargs="+", help="Header files or directories to process")
    parser.add_argument("--max-chars", type=int, default=1600, help="Max characters per chunk")
    parser.add_argument("--overlap", type=int, default=1, help="Number of units to overlap")
    parser.add_argument("--one-per-symbol", action="store_true", help="One chunk per symbol")
    parser.add_argument("--output", "-o", help="Save chunks to file instead of uploading")
    parser.add_argument("--azure-endpoint", help="Azure Search endpoint")
    parser.add_argument("--azure-key", help="Azure Search admin key")
    parser.add_argument("--index-name", default="c-code-index", help="Azure Search index name")
    parser.add_argument("--openai-key", help="OpenAI API key")
    parser.add_argument("--openai-base", help="OpenAI API base URL (for Azure OpenAI)")
    parser.add_argument("--embedding-model", default="text-embedding-ada-002", help="Embedding model name")
    parser.add_argument("--embedding-dim", type=int, default=1536, help="Embedding dimension")
    args = parser.parse_args()
    
    # Process header files
    chunks = process_headers(
        args.inputs,
        max_chunk_size=args.max_chars,
        chunk_overlap=args.overlap,
        one_per_symbol=args.one_per_symbol
    )
    
    print(f"Generated {len(chunks)} chunks")
    
    # Save to file if requested
    if args.output:
        with open(args.output, "w", encoding="utf-8") as f:
            for chunk in chunks:
                f.write(json.dumps(chunk, ensure_ascii=False) + "\n")
        print(f"Saved chunks to {args.output}")
        return
    
    # Upload to Azure AI Search if credentials provided
    if args.azure_endpoint and args.azure_key:
        indexer = AzureSearchIndexer(
            search_service_endpoint=args.azure_endpoint,
            search_admin_key=args.azure_key,
            index_name=args.index_name,
            embedding_model=args.embedding_model,
            embedding_dimension=args.embedding_dim,
            openai_api_key=args.openai_key,
            openai_api_base=args.openai_base
        )
        
        # Create index if it doesn't exist
        indexer.create_search_index()
        
        # Upload chunks
        indexer.upload_chunks(chunks)
        print("Chunks uploaded to Azure AI Search")
    else:
        print("Azure Search credentials not provided. Use --output to save chunks to file.")

if __name__ == "__main__":
    main()
