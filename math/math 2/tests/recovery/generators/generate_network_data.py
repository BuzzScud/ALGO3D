#!/usr/bin/env python3
"""
Generate clean network test data for all subcategories.

Generates:
1. Social networks (friend graphs, communities)
2. Computer networks (topology, routing)
3. Biological networks (protein interactions, pathways)
4. Transportation networks (roads, rail, routes)
5. Abstract graphs (generic structures)

All data is deterministic (seeded) for reproducibility.
"""

import json
import random
import math
from pathlib import Path

# Set seed for reproducibility
SEED = 42
random.seed(SEED)

# Output directory
OUTPUT_DIR = Path("../data/network")
OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

def save_json(nodes, edges, metadata, filename):
    """Save network in JSON format."""
    data = {
        "nodes": nodes,
        "edges": edges,
        "metadata": metadata
    }
    
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

def save_gml(nodes, edges, metadata, filename):
    """Save network in GML format."""
    with open(filename, 'w') as f:
        f.write("graph [\n")
        f.write(f"  comment &quot;{metadata['name']}&quot;\n")
        f.write(f"  directed {1 if metadata.get('directed', False) else 0}\n")
        
        # Write nodes
        for node in nodes:
            f.write(f"  node [\n")
            f.write(f"    id {node['id']}\n")
            if 'label' in node:
                f.write(f"    label &quot;{node['label']}&quot;\n")
            f.write(f"  ]\n")
        
        # Write edges
        for edge in edges:
            f.write(f"  edge [\n")
            f.write(f"    source {edge['source']}\n")
            f.write(f"    target {edge['target']}\n")
            if 'weight' in edge:
                f.write(f"    weight {edge['weight']}\n")
            f.write(f"  ]\n")
        
        f.write("]\n")

def generate_social_network():
    """Generate a small social network (friend graph)."""
    num_nodes = 20
    
    # Create nodes (people)
    nodes = [{"id": i, "label": f"Person_{i}"} for i in range(num_nodes)]
    
    # Create edges (friendships) - random connections
    edges = []
    for i in range(num_nodes):
        # Each person has 2-5 friends
        num_friends = random.randint(2, 5)
        friends = random.sample([j for j in range(num_nodes) if j != i], num_friends)
        for friend in friends:
            # Avoid duplicate edges
            if not any(e['source'] == min(i, friend) and e['target'] == max(i, friend) for e in edges):
                edges.append({
                    "source": min(i, friend),
                    "target": max(i, friend)
                })
    
    metadata = {
        "name": "Social Network",
        "type": "Social",
        "num_nodes": num_nodes,
        "num_edges": len(edges),
        "directed": False,
        "description": "Friend graph with 20 people"
    }
    
    return nodes, edges, metadata

def generate_computer_network():
    """Generate a computer network topology (star topology)."""
    num_nodes = 10
    
    # Node 0 is the central router
    nodes = [{"id": 0, "label": "Router"}]
    nodes.extend([{"id": i, "label": f"Computer_{i}"} for i in range(1, num_nodes)])
    
    # Star topology - all computers connect to router
    edges = [{"source": 0, "target": i, "weight": 1.0} for i in range(1, num_nodes)]
    
    metadata = {
        "name": "Computer Network",
        "type": "Computer",
        "num_nodes": num_nodes,
        "num_edges": len(edges),
        "directed": False,
        "topology": "star",
        "description": "Star topology with central router"
    }
    
    return nodes, edges, metadata

def generate_biological_network():
    """Generate a biological network (protein interactions)."""
    num_proteins = 15
    
    # Create nodes (proteins)
    nodes = [{"id": i, "label": f"Protein_{chr(65+i)}"} for i in range(num_proteins)]
    
    # Create edges (interactions) - scale-free network
    edges = []
    # Hub proteins (highly connected)
    hubs = [0, 1, 2]
    
    for hub in hubs:
        # Hub connects to many proteins
        targets = random.sample([i for i in range(num_proteins) if i != hub], 5)
        for target in targets:
            edges.append({
                "source": min(hub, target),
                "target": max(hub, target),
                "weight": random.uniform(0.5, 1.0)
            })
    
    # Other proteins have fewer connections
    for i in range(3, num_proteins):
        num_connections = random.randint(1, 3)
        targets = random.sample([j for j in range(num_proteins) if j != i], num_connections)
        for target in targets:
            if not any(e['source'] == min(i, target) and e['target'] == max(i, target) for e in edges):
                edges.append({
                    "source": min(i, target),
                    "target": max(i, target),
                    "weight": random.uniform(0.3, 0.9)
                })
    
    metadata = {
        "name": "Protein Interaction Network",
        "type": "Biological",
        "num_nodes": num_proteins,
        "num_edges": len(edges),
        "directed": False,
        "description": "Protein-protein interaction network"
    }
    
    return nodes, edges, metadata

def generate_transportation_network():
    """Generate a transportation network (road network)."""
    num_cities = 12
    
    # Create nodes (cities)
    nodes = [{"id": i, "label": f"City_{i}"} for i in range(num_cities)]
    
    # Create edges (roads) - grid-like structure
    edges = []
    grid_size = int(math.sqrt(num_cities))
    
    for i in range(num_cities):
        row = i // grid_size
        col = i % grid_size
        
        # Connect to right neighbor
        if col < grid_size - 1:
            edges.append({
                "source": i,
                "target": i + 1,
                "weight": random.uniform(10, 50)  # distance in km
            })
        
        # Connect to bottom neighbor
        if row < grid_size - 1:
            edges.append({
                "source": i,
                "target": i + grid_size,
                "weight": random.uniform(10, 50)
            })
    
    metadata = {
        "name": "Transportation Network",
        "type": "Transportation",
        "num_nodes": num_cities,
        "num_edges": len(edges),
        "directed": False,
        "description": "Road network connecting cities in a grid"
    }
    
    return nodes, edges, metadata

def generate_abstract_graph():
    """Generate an abstract graph (random graph)."""
    num_nodes = 15
    edge_probability = 0.3
    
    # Create nodes
    nodes = [{"id": i, "label": f"Node_{i}"} for i in range(num_nodes)]
    
    # Create edges (Erdős–Rényi random graph)
    edges = []
    for i in range(num_nodes):
        for j in range(i + 1, num_nodes):
            if random.random() < edge_probability:
                edges.append({
                    "source": i,
                    "target": j,
                    "weight": random.uniform(0.1, 1.0)
                })
    
    metadata = {
        "name": "Abstract Graph",
        "type": "Abstract",
        "num_nodes": num_nodes,
        "num_edges": len(edges),
        "directed": False,
        "edge_probability": edge_probability,
        "description": "Erdős–Rényi random graph"
    }
    
    return nodes, edges, metadata

def main():
    """Generate all network test data."""
    print("Generating network test data...")
    print(f"Output directory: {OUTPUT_DIR}")
    print(f"Random seed: {SEED}\n")
    
    generators = [
        ("social_network", generate_social_network),
        ("computer_network", generate_computer_network),
        ("biological_network", generate_biological_network),
        ("transportation_network", generate_transportation_network),
        ("abstract_graph", generate_abstract_graph)
    ]
    
    for name, generator in generators:
        nodes, edges, metadata = generator()
        
        # Save as JSON
        json_file = OUTPUT_DIR / f"{name}.json"
        save_json(nodes, edges, metadata, json_file)
        
        # Save as GML
        gml_file = OUTPUT_DIR / f"{name}.gml"
        save_gml(nodes, edges, metadata, gml_file)
        
        print(f"  ✓ Generated {name}: {metadata['num_nodes']} nodes, {metadata['num_edges']} edges")
    
    total_files = len(list(OUTPUT_DIR.glob('*')))
    print(f"\n✅ Generated {total_files} network test files")
    print(f"   Total files: {total_files} files in {OUTPUT_DIR}")

if __name__ == "__main__":
    main()