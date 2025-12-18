#!/usr/bin/env python3
"""
Visualization Tools for Platonic Prime Resonance

Creates comprehensive visualizations of prime resonance patterns,
dimensional clustering, and geometric structures.
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from mpl_toolkits.mplot3d import Axes3D
import sys

# Set style
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

def load_data():
    """Load prime analysis data"""
    try:
        df = pd.read_csv('prime_analysis_data.csv')
        print(f"✓ Loaded {len(df)} primes")
        return df
    except FileNotFoundError:
        print("Error: prime_analysis_data.csv not found")
        print("Run ./tools/analyze_million_primes first")
        sys.exit(1)

def plot_resonance_landscape(df, max_primes=10000):
    """Plot resonance values for all primes"""
    print("\nCreating resonance landscape plot...")
    
    df_subset = df.head(max_primes)
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 10))
    
    # Full landscape
    scatter = ax1.scatter(df_subset['prime'], df_subset['resonance'], 
                         c=df_subset['resonance'], cmap='viridis', 
                         alpha=0.6, s=20)
    ax1.set_xlabel('Prime Number', fontsize=12)
    ax1.set_ylabel('Geometric Resonance', fontsize=12)
    ax1.set_title(f'Prime Resonance Landscape (First {max_primes} primes)', 
                  fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    plt.colorbar(scatter, ax=ax1, label='Resonance')
    
    # Highlight top resonance primes
    top_resonance = df_subset.nlargest(20, 'resonance')
    ax1.scatter(top_resonance['prime'], top_resonance['resonance'], 
               color='red', s=100, marker='*', edgecolors='black', 
               linewidths=1, label='Top 20 Resonance', zorder=5)
    ax1.legend()
    
    # Zoomed view on high resonance region
    high_res = df_subset[df_subset['resonance'] > 0.5]
    ax2.scatter(high_res['prime'], high_res['resonance'], 
               c=high_res['resonance'], cmap='plasma', alpha=0.7, s=30)
    ax2.set_xlabel('Prime Number', fontsize=12)
    ax2.set_ylabel('Geometric Resonance', fontsize=12)
    ax2.set_title('High Resonance Region (resonance > 0.5)', 
                  fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    
    # Annotate Platonic primes
    platonic_primes = [5, 23, 29, 127, 241]
    for prime in platonic_primes:
        if prime in high_res['prime'].values:
            row = high_res[high_res['prime'] == prime].iloc[0]
            ax2.annotate(f'{prime}', xy=(prime, row['resonance']),
                        xytext=(10, 10), textcoords='offset points',
                        bbox=dict(boxstyle='round,pad=0.5', fc='yellow', alpha=0.7),
                        arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))
    
    plt.tight_layout()
    plt.savefig('resonance_landscape.png', dpi=300, bbox_inches='tight')
    print("✓ Saved resonance_landscape.png")
    plt.close()

def plot_dimensional_clustering(df):
    """Heatmap showing prime density near p_s^d targets"""
    print("\nCreating dimensional clustering heatmap...")
    
    # Generate targets
    base_primes = [2, 3, 5, 7, 11]
    max_d = 10
    
    # Create matrix of prime counts
    matrix = np.zeros((len(base_primes), max_d))
    
    for i, p_s in enumerate(base_primes):
        for d in range(1, max_d + 1):
            target = p_s ** d
            if target > df['prime'].max():
                continue
            
            # Count primes within ±50 of target
            count = len(df[(df['prime'] >= target - 50) & 
                          (df['prime'] <= target + 50)])
            matrix[i, d-1] = count
    
    # Create heatmap
    fig, ax = plt.subplots(figsize=(12, 6))
    sns.heatmap(matrix, annot=True, fmt='.0f', cmap='YlOrRd', 
                xticklabels=range(1, max_d + 1),
                yticklabels=[f'p_s={p}' for p in base_primes],
                cbar_kws={'label': 'Number of Primes (±50)'})
    ax.set_xlabel('Dimension (d)', fontsize=12)
    ax.set_ylabel('Base Prime (p_s)', fontsize=12)
    ax.set_title('Prime Clustering Near p_s^d Targets (±50 window)', 
                 fontsize=14, fontweight='bold')
    
    plt.tight_layout()
    plt.savefig('dimensional_clustering_heatmap.png', dpi=300, bbox_inches='tight')
    print("✓ Saved dimensional_clustering_heatmap.png")
    plt.close()

def plot_mod12_distribution(df):
    """Bar chart of prime distribution by mod 12"""
    print("\nCreating mod 12 distribution plot...")
    
    mod12_counts = df['mod12'].value_counts().sort_index()
    
    fig, ax = plt.subplots(figsize=(12, 6))
    
    colors = ['red' if i not in [1, 5, 7, 11] else 'green' 
              for i in range(12)]
    
    bars = ax.bar(range(12), [mod12_counts.get(i, 0) for i in range(12)], 
                  color=colors, alpha=0.7, edgecolor='black')
    
    ax.set_xlabel('Residue Class (mod 12)', fontsize=12)
    ax.set_ylabel('Number of Primes', fontsize=12)
    ax.set_title('Prime Distribution by mod 12 Residue Class', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(range(12))
    ax.grid(True, alpha=0.3, axis='y')
    
    # Add legend
    from matplotlib.patches import Patch
    legend_elements = [Patch(facecolor='green', alpha=0.7, label='Valid for primes > 3'),
                      Patch(facecolor='red', alpha=0.7, label='Invalid (divisible by 2 or 3)')]
    ax.legend(handles=legend_elements, loc='upper right')
    
    # Annotate Platonic primes
    platonic_mod12 = {5: 5, 23: 11, 29: 5, 127: 7, 241: 1}
    for prime, mod in platonic_mod12.items():
        ax.annotate(f'Prime {prime}', xy=(mod, mod12_counts.get(mod, 0)),
                   xytext=(0, 20), textcoords='offset points',
                   ha='center', fontsize=8,
                   bbox=dict(boxstyle='round,pad=0.3', fc='yellow', alpha=0.7),
                   arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))
    
    plt.tight_layout()
    plt.savefig('mod12_distribution.png', dpi=300, bbox_inches='tight')
    print("✓ Saved mod12_distribution.png")
    plt.close()

def plot_3d_resonance_surface(df, max_primes=1000):
    """3D surface plot of resonance vs. prime and dimension"""
    print("\nCreating 3D resonance surface...")
    
    df_subset = df.head(max_primes)
    
    fig = plt.figure(figsize=(14, 10))
    ax = fig.add_subplot(111, projection='3d')
    
    # Estimate dimension for each prime based on nearest target
    dimensions = []
    for _, row in df_subset.iterrows():
        # Simple heuristic: log scale
        dim = int(np.log2(row['prime'])) if row['prime'] > 1 else 1
        dimensions.append(min(dim, 20))
    
    scatter = ax.scatter(df_subset['prime'], dimensions, df_subset['resonance'],
                        c=df_subset['resonance'], cmap='plasma', 
                        s=30, alpha=0.6)
    
    ax.set_xlabel('Prime Number', fontsize=11)
    ax.set_ylabel('Estimated Dimension', fontsize=11)
    ax.set_zlabel('Geometric Resonance', fontsize=11)
    ax.set_title('3D Resonance Surface: Prime × Dimension × Resonance', 
                 fontsize=14, fontweight='bold')
    
    plt.colorbar(scatter, ax=ax, label='Resonance', shrink=0.5)
    
    plt.tight_layout()
    plt.savefig('3d_resonance_surface.png', dpi=300, bbox_inches='tight')
    print("✓ Saved 3d_resonance_surface.png")
    plt.close()

def plot_gap_analysis(df, max_primes=10000):
    """Analyze and plot prime gaps"""
    print("\nCreating prime gap analysis...")
    
    df_subset = df.head(max_primes)
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(15, 10))
    
    # Gap vs. prime
    ax1.scatter(df_subset['prime'], df_subset['gap'], alpha=0.5, s=10)
    ax1.set_xlabel('Prime Number', fontsize=12)
    ax1.set_ylabel('Gap to Next Prime', fontsize=12)
    ax1.set_title('Prime Gaps vs. Prime Number', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    
    # Gap distribution
    ax2.hist(df_subset['gap'], bins=50, edgecolor='black', alpha=0.7)
    ax2.set_xlabel('Gap Size', fontsize=12)
    ax2.set_ylabel('Frequency', fontsize=12)
    ax2.set_title('Distribution of Prime Gaps', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3, axis='y')
    
    # Add statistics
    mean_gap = df_subset['gap'].mean()
    median_gap = df_subset['gap'].median()
    max_gap = df_subset['gap'].max()
    
    stats_text = f'Mean: {mean_gap:.2f}\nMedian: {median_gap:.0f}\nMax: {max_gap:.0f}'
    ax2.text(0.95, 0.95, stats_text, transform=ax2.transAxes,
            verticalalignment='top', horizontalalignment='right',
            bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.8),
            fontsize=10)
    
    plt.tight_layout()
    plt.savefig('prime_gap_analysis.png', dpi=300, bbox_inches='tight')
    print("✓ Saved prime_gap_analysis.png")
    plt.close()

def plot_resonance_vs_distance(df, max_primes=10000):
    """Plot resonance vs. distance to nearest target"""
    print("\nCreating resonance vs. distance plot...")
    
    df_subset = df.head(max_primes)
    
    fig, ax = plt.subplots(figsize=(12, 8))
    
    scatter = ax.scatter(df_subset['distance'], df_subset['resonance'],
                        c=df_subset['resonance'], cmap='viridis',
                        alpha=0.6, s=30)
    
    ax.set_xlabel('Distance to Nearest Target', fontsize=12)
    ax.set_ylabel('Geometric Resonance', fontsize=12)
    ax.set_title('Resonance vs. Distance to Nearest p_s^d Target', 
                 fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    plt.colorbar(scatter, ax=ax, label='Resonance')
    
    # Add exponential decay curve
    x = np.linspace(0, df_subset['distance'].max(), 100)
    y = np.exp(-x**2 / 100)  # sigma = 100
    ax.plot(x, y, 'r--', linewidth=2, label='Expected decay (σ=100)')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig('resonance_vs_distance.png', dpi=300, bbox_inches='tight')
    print("✓ Saved resonance_vs_distance.png")
    plt.close()

def create_summary_dashboard(df):
    """Create comprehensive summary dashboard"""
    print("\nCreating summary dashboard...")
    
    fig = plt.figure(figsize=(20, 12))
    gs = fig.add_gridspec(3, 3, hspace=0.3, wspace=0.3)
    
    # 1. Resonance landscape (top left, spans 2 columns)
    ax1 = fig.add_subplot(gs[0, :2])
    df_subset = df.head(5000)
    scatter = ax1.scatter(df_subset['prime'], df_subset['resonance'],
                         c=df_subset['resonance'], cmap='viridis',
                         alpha=0.6, s=10)
    ax1.set_xlabel('Prime Number')
    ax1.set_ylabel('Resonance')
    ax1.set_title('Resonance Landscape', fontweight='bold')
    ax1.grid(True, alpha=0.3)
    plt.colorbar(scatter, ax=ax1)
    
    # 2. Mod 12 distribution (top right)
    ax2 = fig.add_subplot(gs[0, 2])
    mod12_counts = df['mod12'].value_counts().sort_index()
    colors = ['red' if i not in [1, 5, 7, 11] else 'green' for i in range(12)]
    ax2.bar(range(12), [mod12_counts.get(i, 0) for i in range(12)],
           color=colors, alpha=0.7)
    ax2.set_xlabel('mod 12')
    ax2.set_ylabel('Count')
    ax2.set_title('Mod 12 Distribution', fontweight='bold')
    ax2.grid(True, alpha=0.3, axis='y')
    
    # 3. Top resonance primes (middle left)
    ax3 = fig.add_subplot(gs[1, 0])
    top_20 = df.nlargest(20, 'resonance')
    ax3.barh(range(20), top_20['resonance'].values)
    ax3.set_yticks(range(20))
    ax3.set_yticklabels(top_20['prime'].values, fontsize=8)
    ax3.set_xlabel('Resonance')
    ax3.set_title('Top 20 Resonance Primes', fontweight='bold')
    ax3.invert_yaxis()
    ax3.grid(True, alpha=0.3, axis='x')
    
    # 4. Clustering heatmap (middle center)
    ax4 = fig.add_subplot(gs[1, 1])
    base_primes = [2, 3, 5, 7]
    max_d = 8
    matrix = np.zeros((len(base_primes), max_d))
    for i, p_s in enumerate(base_primes):
        for d in range(1, max_d + 1):
            target = p_s ** d
            if target > df['prime'].max():
                continue
            count = len(df[(df['prime'] >= target - 50) & 
                          (df['prime'] <= target + 50)])
            matrix[i, d-1] = count
    sns.heatmap(matrix, annot=True, fmt='.0f', cmap='YlOrRd', ax=ax4,
                xticklabels=range(1, max_d + 1),
                yticklabels=[f'{p}' for p in base_primes],
                cbar_kws={'label': 'Count'})
    ax4.set_xlabel('Dimension')
    ax4.set_ylabel('Base Prime')
    ax4.set_title('Dimensional Clustering', fontweight='bold')
    
    # 5. Gap distribution (middle right)
    ax5 = fig.add_subplot(gs[1, 2])
    ax5.hist(df.head(10000)['gap'], bins=30, edgecolor='black', alpha=0.7)
    ax5.set_xlabel('Gap Size')
    ax5.set_ylabel('Frequency')
    ax5.set_title('Prime Gap Distribution', fontweight='bold')
    ax5.grid(True, alpha=0.3, axis='y')
    
    # 6. Resonance vs distance (bottom, spans all columns)
    ax6 = fig.add_subplot(gs[2, :])
    df_subset = df.head(5000)
    scatter = ax6.scatter(df_subset['distance'], df_subset['resonance'],
                         c=df_subset['mod12'], cmap='tab10',
                         alpha=0.6, s=20)
    ax6.set_xlabel('Distance to Nearest Target')
    ax6.set_ylabel('Resonance')
    ax6.set_title('Resonance vs. Distance (colored by mod 12)', fontweight='bold')
    ax6.grid(True, alpha=0.3)
    plt.colorbar(scatter, ax=ax6, label='mod 12')
    
    # Add title
    fig.suptitle('Platonic Prime Resonance - Comprehensive Dashboard', 
                 fontsize=16, fontweight='bold', y=0.995)
    
    plt.savefig('summary_dashboard.png', dpi=300, bbox_inches='tight')
    print("✓ Saved summary_dashboard.png")
    plt.close()

def main():
    print("\n" + "="*60)
    print("PLATONIC PRIME RESONANCE VISUALIZATION")
    print("="*60)
    
    # Load data
    df = load_data()
    
    # Create all visualizations
    plot_resonance_landscape(df, max_primes=10000)
    plot_dimensional_clustering(df)
    plot_mod12_distribution(df)
    plot_3d_resonance_surface(df, max_primes=1000)
    plot_gap_analysis(df, max_primes=10000)
    plot_resonance_vs_distance(df, max_primes=10000)
    create_summary_dashboard(df)
    
    print("\n" + "="*60)
    print("VISUALIZATION COMPLETE")
    print("="*60)
    print("\nGenerated files:")
    print("  - resonance_landscape.png")
    print("  - dimensional_clustering_heatmap.png")
    print("  - mod12_distribution.png")
    print("  - 3d_resonance_surface.png")
    print("  - prime_gap_analysis.png")
    print("  - resonance_vs_distance.png")
    print("  - summary_dashboard.png")
    print("\n")

if __name__ == "__main__":
    main()