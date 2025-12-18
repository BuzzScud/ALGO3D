#!/usr/bin/env python3
"""
Statistical Analysis of Platonic Prime Clustering

This script performs rigorous statistical tests to determine if the clustering
of primes near p_s^d targets is statistically significant or could occur by chance.
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
from scipy.stats import chi2, kstest, poisson
import sys

def load_prime_data(filename='prime_analysis_data.csv'):
    """Load prime analysis data from CSV"""
    try:
        df = pd.read_csv(filename)
        print(f"✓ Loaded {len(df)} primes from {filename}")
        return df
    except FileNotFoundError:
        print(f"Error: {filename} not found. Run analyze_million_primes first.")
        sys.exit(1)

def chi_square_clustering_test(df, targets, window=50):
    """
    Chi-square test for clustering near targets
    
    H0: Primes are uniformly distributed
    H1: Primes cluster near targets
    """
    print("\n" + "="*60)
    print("CHI-SQUARE TEST FOR DIMENSIONAL CLUSTERING")
    print("="*60)
    
    # Count primes in windows around targets vs. elsewhere
    in_window = 0
    out_window = 0
    
    max_prime = df['prime'].max()
    
    for _, row in df.iterrows():
        prime = row['prime']
        near_target = False
        
        for target in targets:
            if abs(prime - target) <= window:
                near_target = True
                break
        
        if near_target:
            in_window += 1
        else:
            out_window += 1
    
    # Calculate expected counts assuming uniform distribution
    total_primes = len(df)
    total_range = max_prime - 2
    
    # Total window space
    window_space = sum(2 * window for target in targets if target <= max_prime)
    non_window_space = total_range - window_space
    
    expected_in_window = total_primes * (window_space / total_range)
    expected_out_window = total_primes * (non_window_space / total_range)
    
    # Chi-square statistic
    chi_square = ((in_window - expected_in_window)**2 / expected_in_window +
                  (out_window - expected_out_window)**2 / expected_out_window)
    
    # Degrees of freedom = categories - 1
    df_chi = 1
    p_value = 1 - chi2.cdf(chi_square, df_chi)
    
    print(f"\nWindow size: ±{window} around each target")
    print(f"Number of targets: {len([t for t in targets if t <= max_prime])}")
    print(f"\nObserved:")
    print(f"  Primes in windows: {in_window}")
    print(f"  Primes outside windows: {out_window}")
    print(f"\nExpected (uniform distribution):")
    print(f"  Primes in windows: {expected_in_window:.1f}")
    print(f"  Primes outside windows: {expected_out_window:.1f}")
    print(f"\nChi-square statistic: {chi_square:.4f}")
    print(f"Degrees of freedom: {df_chi}")
    print(f"P-value: {p_value:.6f}")
    
    if p_value < 0.05:
        print(f"\n✓ SIGNIFICANT: Clustering is statistically significant (p < 0.05)")
        print(f"  We reject the null hypothesis of uniform distribution.")
    else:
        print(f"\n✗ NOT SIGNIFICANT: Cannot reject uniform distribution (p >= 0.05)")
    
    return chi_square, p_value

def kolmogorov_smirnov_test(df, targets, window=50):
    """
    Kolmogorov-Smirnov test comparing observed vs. expected distribution
    """
    print("\n" + "="*60)
    print("KOLMOGOROV-SMIRNOV TEST")
    print("="*60)
    
    # Calculate distances to nearest target for each prime
    distances = []
    for _, row in df.iterrows():
        prime = row['prime']
        min_dist = min(abs(prime - target) for target in targets if target <= prime * 2)
        distances.append(min_dist)
    
    # Expected distribution (uniform)
    max_dist = max(distances)
    uniform_dist = np.random.uniform(0, max_dist, len(distances))
    
    # K-S test
    statistic, p_value = kstest(distances, uniform_dist)
    
    print(f"\nComparing distance distribution to uniform distribution")
    print(f"K-S statistic: {statistic:.6f}")
    print(f"P-value: {p_value:.6f}")
    
    if p_value < 0.05:
        print(f"\n✓ SIGNIFICANT: Distribution differs from uniform (p < 0.05)")
    else:
        print(f"\n✗ NOT SIGNIFICANT: Cannot reject uniform distribution (p >= 0.05)")
    
    return statistic, p_value

def poisson_clustering_test(df, targets, window=10):
    """
    Test if clustering follows Poisson distribution
    """
    print("\n" + "="*60)
    print("POISSON DISTRIBUTION TEST")
    print("="*60)
    
    # Count primes in each window
    counts = []
    for target in targets:
        if target > df['prime'].max():
            continue
        count = len(df[(df['prime'] >= target - window) & 
                      (df['prime'] <= target + window)])
        counts.append(count)
    
    # Calculate lambda (mean count)
    lambda_param = np.mean(counts)
    
    # Expected Poisson distribution
    observed_freq = np.bincount(counts)
    max_count = len(observed_freq)
    
    expected_freq = [len(counts) * poisson.pmf(k, lambda_param) 
                     for k in range(max_count)]
    
    # Chi-square test for goodness of fit
    chi_square = sum((obs - exp)**2 / exp 
                     for obs, exp in zip(observed_freq, expected_freq) if exp > 0)
    df_chi = max_count - 2  # -1 for constraint, -1 for estimated parameter
    p_value = 1 - chi2.cdf(chi_square, df_chi)
    
    print(f"\nWindow size: ±{window} around each target")
    print(f"Mean primes per window: {lambda_param:.2f}")
    print(f"Standard deviation: {np.std(counts):.2f}")
    print(f"\nObserved distribution: {observed_freq}")
    print(f"Expected (Poisson): {[f'{e:.1f}' for e in expected_freq]}")
    print(f"\nChi-square statistic: {chi_square:.4f}")
    print(f"P-value: {p_value:.6f}")
    
    if p_value >= 0.05:
        print(f"\n✓ FITS POISSON: Clustering follows Poisson distribution (p >= 0.05)")
        print(f"  This suggests random clustering around targets.")
    else:
        print(f"\n✗ DOESN'T FIT POISSON: Clustering differs from Poisson (p < 0.05)")
        print(f"  This suggests non-random clustering pattern.")
    
    return chi_square, p_value

def monte_carlo_simulation(df, targets, window=50, n_trials=10000):
    """
    Monte Carlo simulation: Generate random "primes" and compare clustering
    """
    print("\n" + "="*60)
    print("MONTE CARLO SIMULATION")
    print("="*60)
    print(f"\nRunning {n_trials} simulations...")
    
    # Real clustering score
    real_score = 0
    for _, row in df.iterrows():
        prime = row['prime']
        for target in targets:
            if abs(prime - target) <= window:
                real_score += 1
                break
    
    # Simulate random distributions
    max_prime = df['prime'].max()
    num_primes = len(df)
    
    random_scores = []
    for trial in range(n_trials):
        # Generate random "primes"
        random_primes = np.random.randint(2, max_prime, num_primes)
        
        # Calculate clustering score
        score = 0
        for prime in random_primes:
            for target in targets:
                if abs(prime - target) <= window:
                    score += 1
                    break
        
        random_scores.append(score)
        
        if (trial + 1) % 1000 == 0:
            print(f"  Completed {trial + 1}/{n_trials} trials", end='\r')
    
    print(f"  Completed {n_trials}/{n_trials} trials")
    
    # Calculate percentile
    percentile = stats.percentileofscore(random_scores, real_score)
    p_value = (100 - percentile) / 100
    
    print(f"\nReal clustering score: {real_score}")
    print(f"Mean random score: {np.mean(random_scores):.1f}")
    print(f"Std random score: {np.std(random_scores):.1f}")
    print(f"Percentile of real score: {percentile:.2f}%")
    print(f"P-value: {p_value:.6f}")
    
    if percentile > 95:
        print(f"\n✓ HIGHLY SIGNIFICANT: Real clustering in top 5% (p < 0.05)")
        print(f"  Real primes cluster significantly more than random.")
    else:
        print(f"\n✗ NOT SIGNIFICANT: Real clustering not exceptional (p >= 0.05)")
    
    # Plot histogram
    plt.figure(figsize=(10, 6))
    plt.hist(random_scores, bins=50, alpha=0.7, edgecolor='black')
    plt.axvline(real_score, color='red', linestyle='--', linewidth=2, 
                label=f'Real Score: {real_score}')
    plt.axvline(np.mean(random_scores), color='blue', linestyle='--', linewidth=2,
                label=f'Mean Random: {np.mean(random_scores):.1f}')
    plt.xlabel('Clustering Score')
    plt.ylabel('Frequency')
    plt.title(f'Monte Carlo Simulation: Real vs. Random Clustering (n={n_trials})')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.savefig('monte_carlo_clustering.png', dpi=300, bbox_inches='tight')
    print(f"\n✓ Saved plot to monte_carlo_clustering.png")
    
    return percentile, p_value

def generate_targets(max_value=1000000):
    """Generate all p_s^d targets up to max_value"""
    targets = []
    base_primes = [2, 3, 5, 7, 11, 13]
    
    for p_s in base_primes:
        d = 1
        while True:
            target = p_s ** d
            if target > max_value:
                break
            if target >= 2:
                targets.append(target)
            d += 1
    
    return sorted(set(targets))

def main():
    print("\n" + "="*60)
    print("STATISTICAL ANALYSIS OF PLATONIC PRIME CLUSTERING")
    print("="*60)
    
    # Load data
    df = load_prime_data()
    
    # Generate targets
    targets = generate_targets(df['prime'].max())
    print(f"✓ Generated {len(targets)} dimensional targets")
    
    # Run all tests
    results = {}
    
    # Test 1: Chi-square
    chi_stat, chi_p = chi_square_clustering_test(df, targets, window=50)
    results['chi_square'] = {'statistic': chi_stat, 'p_value': chi_p}
    
    # Test 2: Kolmogorov-Smirnov
    ks_stat, ks_p = kolmogorov_smirnov_test(df, targets, window=50)
    results['ks_test'] = {'statistic': ks_stat, 'p_value': ks_p}
    
    # Test 3: Poisson
    poisson_stat, poisson_p = poisson_clustering_test(df, targets, window=10)
    results['poisson'] = {'statistic': poisson_stat, 'p_value': poisson_p}
    
    # Test 4: Monte Carlo
    mc_percentile, mc_p = monte_carlo_simulation(df, targets, window=50, n_trials=10000)
    results['monte_carlo'] = {'percentile': mc_percentile, 'p_value': mc_p}
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY OF STATISTICAL TESTS")
    print("="*60)
    print("\nTest                    Statistic    P-value    Significant?")
    print("-" * 60)
    print(f"Chi-square              {results['chi_square']['statistic']:10.4f}  {results['chi_square']['p_value']:9.6f}  {'YES' if results['chi_square']['p_value'] < 0.05 else 'NO'}")
    print(f"Kolmogorov-Smirnov      {results['ks_test']['statistic']:10.6f}  {results['ks_test']['p_value']:9.6f}  {'YES' if results['ks_test']['p_value'] < 0.05 else 'NO'}")
    print(f"Poisson fit             {results['poisson']['statistic']:10.4f}  {results['poisson']['p_value']:9.6f}  {'YES' if results['poisson']['p_value'] < 0.05 else 'NO'}")
    print(f"Monte Carlo (top 5%)    {results['monte_carlo']['percentile']:10.2f}% {results['monte_carlo']['p_value']:9.6f}  {'YES' if results['monte_carlo']['p_value'] < 0.05 else 'NO'}")
    
    # Overall conclusion
    significant_count = sum(1 for test in results.values() 
                           if test['p_value'] < 0.05)
    
    print("\n" + "="*60)
    print("OVERALL CONCLUSION")
    print("="*60)
    
    if significant_count >= 3:
        print("\n✓ STRONG EVIDENCE: Dimensional clustering is statistically significant")
        print(f"  {significant_count}/4 tests show significance (p < 0.05)")
        print("  Primes cluster near p_s^d targets more than expected by chance.")
    elif significant_count >= 2:
        print("\n⚠ MODERATE EVIDENCE: Some evidence for dimensional clustering")
        print(f"  {significant_count}/4 tests show significance (p < 0.05)")
        print("  Results suggest clustering but not conclusive.")
    else:
        print("\n✗ WEAK EVIDENCE: Clustering may be due to chance")
        print(f"  Only {significant_count}/4 tests show significance (p < 0.05)")
        print("  Cannot conclusively prove non-random clustering.")
    
    print("\n" + "="*60)

if __name__ == "__main__":
    main()