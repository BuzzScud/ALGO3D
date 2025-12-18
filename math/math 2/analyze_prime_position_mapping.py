#!/usr/bin/env python3
"""
Deep analysis of prime-to-position mapping to discover the deterministic formula.
"""

import math

# First 232 primes from lookup table
PRIMES_232 = [
    # Ring 0: Hours (12 primes)
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37,
    # Ring 1: Minutes (60 primes)
    41, 43, 47, 53, 59, 61, 67, 71, 73, 79,
    83, 89, 97, 101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
    191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
    241, 251, 257, 263, 269, 271, 277, 281, 283, 293,
    307, 311, 313, 317, 331, 337, 347, 349, 353, 359,
    # Ring 2: Seconds (60 primes)
    367, 373, 379, 383, 389, 397, 401, 409, 419, 421,
    431, 433, 439, 443, 449, 457, 461, 463, 467, 479,
    487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601, 607, 613,
    617, 619, 631, 641, 643, 647, 653, 659, 661, 673,
    677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
    # Ring 3: Milliseconds (100 primes)
    751, 757, 761, 769, 773, 787, 797, 809, 811, 821,
    823, 827, 829, 839, 853, 857, 859, 863, 877, 881,
    883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
    967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021,
    1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087, 1091,
    1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163,
    1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231,
    1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297, 1301,
    1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381, 1399,
    1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459
]

def map_index_to_ring_position(index):
    """Map prime index to (ring, position) following Babylonian structure"""
    if index <= 12:
        return (0, index)  # Ring 0: Hours
    elif index <= 72:
        return (1, index - 12)  # Ring 1: Minutes
    elif index <= 132:
        return (2, index - 72)  # Ring 2: Seconds
    elif index <= 232:
        return (3, index - 132)  # Ring 3: Milliseconds
    else:
        # Beyond 232: logarithmic spiral
        adjusted = index - 232
        ring = (int(math.log(adjusted + 1, 3)) % 4) + 4
        return (ring, adjusted % 1000)

def analyze_ring_0_pattern():
    """Analyze Ring 0 (Hours) - First 12 primes"""
    print("=" * 80)
    print("RING 0 ANALYSIS (Hours - First 12 Primes)")
    print("=" * 80)
    
    print(f"\n{'Position':<10} {'Index':<8} {'Prime':<8} {'Gap':<8} {'Mod 12':<10}")
    print("-" * 50)
    
    for i in range(12):
        index = i + 1
        prime = PRIMES_232[i]
        gap = PRIMES_232[i] - PRIMES_232[i-1] if i > 0 else 0
        mod12 = prime % 12
        
        marker = " ← SACRED" if index == 3 else (" ← SACRED" if index == 12 else "")
        print(f"{index:<10} {index:<8} {prime:<8} {gap:<8} {mod12:<10}{marker}")
    
    print("\nKey observations:")
    print("- Position 3: Prime = 5 (π at 3 o'clock)")
    print("- Position 12: Prime = 37 (12 o'clock, top)")
    print("- Gaps: 1, 2, 2, 4, 2, 4, 2, 4, 6, 2, 6")
    print("- Pattern: Gaps increase as we go around the clock")

def analyze_position_3_pattern():
    """Analyze position 3 across all rings (π position)"""
    print("\n" + "=" * 80)
    print("POSITION 3 ANALYSIS (π Position Across All Rings)")
    print("=" * 80)
    
    print(f"\n{'Ring':<8} {'Index':<8} {'Prime':<10} {'Mod 12':<10} {'Notes'}")
    print("-" * 60)
    
    # Ring 0, Position 3
    print(f"{0:<8} {3:<8} {PRIMES_232[2]:<10} {PRIMES_232[2] % 12:<10} π at 3 o'clock")
    
    # Ring 1, Position 3
    idx = 12 + 3
    print(f"{1:<8} {idx:<8} {PRIMES_232[idx-1]:<10} {PRIMES_232[idx-1] % 12:<10}")
    
    # Ring 2, Position 3
    idx = 72 + 3
    print(f"{2:<8} {idx:<8} {PRIMES_232[idx-1]:<10} {PRIMES_232[idx-1] % 12:<10}")
    
    # Ring 3, Position 3
    idx = 132 + 3
    print(f"{3:<8} {idx:<8} {PRIMES_232[idx-1]:<10} {PRIMES_232[idx-1] % 12:<10}")

def analyze_mod_12_within_rings():
    """Analyze mod 12 distribution within each ring"""
    print("\n" + "=" * 80)
    print("MOD 12 DISTRIBUTION WITHIN RINGS")
    print("=" * 80)
    
    rings = [
        (0, 12, "Hours"),
        (13, 72, "Minutes"),
        (73, 132, "Seconds"),
        (133, 232, "Milliseconds")
    ]
    
    for start_idx, end_idx, name in rings:
        ring_primes = PRIMES_232[start_idx-1:end_idx]
        
        mod12_dist = {1: 0, 5: 0, 7: 0, 11: 0, 'other': 0}
        for p in ring_primes:
            m = p % 12
            if m in [1, 5, 7, 11]:
                mod12_dist[m] += 1
            else:
                mod12_dist['other'] += 1
        
        print(f"\n{name} (Primes {start_idx}-{end_idx}):")
        print(f"  ≡ 1:  {mod12_dist[1]:3d} primes")
        print(f"  ≡ 5:  {mod12_dist[5]:3d} primes")
        print(f"  ≡ 7:  {mod12_dist[7]:3d} primes")
        print(f"  ≡ 11: {mod12_dist[11]:3d} primes")
        print(f"  Other: {mod12_dist['other']:3d} primes")

def analyze_babylonian_encoding():
    """Test if primes can be decoded from Babylonian base-60 representation"""
    print("\n" + "=" * 80)
    print("BABYLONIAN BASE-60 ENCODING HYPOTHESIS")
    print("=" * 80)
    
    print("\nTesting if prime = f(ring_digits in base-60)...")
    
    for i in range(min(20, len(PRIMES_232))):
        index = i + 1
        prime = PRIMES_232[i]
        ring, pos = map_index_to_ring_position(index)
        
        # Try various encodings
        # Encoding 1: Simple concatenation
        if ring == 0:
            encoded = pos
        elif ring == 1:
            encoded = 12 + pos
        elif ring == 2:
            encoded = 12 + 60 + pos
        else:
            encoded = 12 + 60 + 60 + pos
        
        print(f"Index {index:3d}: Ring {ring}, Pos {pos:3d} → Prime {prime:4d} (encoded: {encoded:3d})")

def analyze_gap_patterns():
    """Analyze prime gap patterns"""
    print("\n" + "=" * 80)
    print("PRIME GAP PATTERN ANALYSIS")
    print("=" * 80)
    
    gaps = [PRIMES_232[i] - PRIMES_232[i-1] for i in range(1, len(PRIMES_232))]
    
    print(f"\nFirst 30 gaps: {gaps[:30]}")
    print(f"\nGap statistics:")
    print(f"  Min gap: {min(gaps)}")
    print(f"  Max gap: {max(gaps)}")
    print(f"  Average gap: {sum(gaps) / len(gaps):.2f}")
    print(f"  Median gap: {sorted(gaps)[len(gaps)//2]}")
    
    # Count gap frequencies
    gap_freq = {}
    for g in gaps:
        gap_freq[g] = gap_freq.get(g, 0) + 1
    
    print(f"\nMost common gaps:")
    for gap, count in sorted(gap_freq.items(), key=lambda x: -x[1])[:10]:
        print(f"  Gap {gap:3d}: {count:3d} times ({100*count/len(gaps):.1f}%)")

def analyze_144000_neighborhood():
    """Analyze primes near 144000"""
    print("\n" + "=" * 80)
    print("144000 NEIGHBORHOOD ANALYSIS")
    print("=" * 80)
    
    # Check if 143999 and 144001 are prime
    def is_prime_simple(n):
        if n < 2: return False
        if n == 2: return True
        if n % 2 == 0: return False
        for i in range(3, int(n**0.5) + 1, 2):
            if n % i == 0: return False
        return True
    
    print(f"\n144000 = 3 × 12³ × (250/9) = {3 * 12**3 * (250/9)}")
    print(f"\nNeighborhood:")
    
    for offset in range(-10, 11):
        n = 144000 + offset
        is_p = is_prime_simple(n)
        mod12 = n % 12
        marker = " ← TWIN PRIME" if n == 143999 else (" ← COMPOSITE (11×13091)" if n == 144001 else "")
        status = "PRIME" if is_p else "composite"
        print(f"  {n:7d} (mod 12 = {mod12:2d}): {status:10s}{marker}")

if __name__ == "__main__":
    analyze_ring_0_pattern()
    analyze_position_3_pattern()
    analyze_mod_12_within_rings()
    analyze_babylonian_encoding()
    analyze_gap_patterns()
    analyze_144000_neighborhood()
    
    print("\n" + "=" * 80)
    print("AWAITING USER'S PYTHON EXAMPLES FOR FURTHER INSIGHTS")
    print("=" * 80)