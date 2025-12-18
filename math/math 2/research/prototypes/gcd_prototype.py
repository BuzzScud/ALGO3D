#!/usr/bin/env python3
"""
Babylonian GCD Prototype
Uses Euclidean algorithm with geometric clock-based operations
Key insight: GCD finds the largest clock that both numbers fit on!
"""

import math
from dataclasses import dataclass

@dataclass
class BabylonianNumber:
    """Represents a number in Babylonian geometric form"""
    position: int
    rotations: int
    ring: int
    base: int
    polarity: bool
    
    def to_int(self) -> int:
        """Convert back to integer"""
        value = self.rotations * self.base + (self.position if self.position != self.base else 0)
        return value if self.polarity else -value

def number_to_babylonian(number: int, base: int = 10, ring: int = 0) -> BabylonianNumber:
    """Map a number to Babylonian representation"""
    is_negative = number < 0
    number = abs(number)
    
    rotations = number // base
    position = number % base
    if position == 0:
        position = base
    
    return BabylonianNumber(
        position=position,
        rotations=rotations,
        ring=ring,
        base=base,
        polarity=not is_negative
    )

def babylonian_gcd_geometric(a: int, b: int) -> int:
    """
    Babylonian GCD using Euclidean algorithm
    Key insight: GCD is the largest "clock size" that both numbers fit on
    O(log min(a,b)) complexity with O(1) operations per step
    """
    print(f"\n{'='*60}")
    print(f"GEOMETRIC GCD: gcd({a}, {b})")
    print(f"{'='*60}")
    
    # Handle edge cases
    a, b = abs(a), abs(b)
    if b == 0:
        return a
    if a == 0:
        return b
    
    print(f"\nGeometric Interpretation:")
    print(f"  Finding the largest clock that both {a} and {b} fit on")
    print(f"  This clock size is the GCD")
    
    # Euclidean algorithm
    print(f"\nEuclidean Algorithm:")
    step = 0
    original_a, original_b = a, b
    
    while b != 0:
        # Map to Babylonian representation
        bn_a = number_to_babylonian(a, base=b)
        remainder = a % b
        
        print(f"  Step {step}: {a} = {a // b} × {b} + {remainder}")
        print(f"    → {a} on a {b}-clock stops at position {bn_a.position}")
        
        a, b = b, remainder
        step += 1
    
    print(f"\nResult: gcd({original_a}, {original_b}) = {a}")
    print(f"  This is the largest clock both numbers fit on!")
    
    return a

def babylonian_gcd_simple(a: int, b: int) -> int:
    """Simplified GCD using Python's built-in"""
    return math.gcd(abs(a), abs(b))

def babylonian_lcm_geometric(a: int, b: int) -> int:
    """
    Babylonian LCM using the formula: lcm(a,b) = (a×b) / gcd(a,b)
    O(log min(a,b)) complexity (dominated by GCD)
    """
    print(f"\n{'='*60}")
    print(f"GEOMETRIC LCM: lcm({a}, {b})")
    print(f"{'='*60}")
    
    a, b = abs(a), abs(b)
    
    if a == 0 or b == 0:
        return 0
    
    # Step 1: Compute GCD
    gcd_val = babylonian_gcd_simple(a, b)
    print(f"\nStep 1: Compute GCD")
    print(f"  gcd({a}, {b}) = {gcd_val}")
    
    # Step 2: Use formula lcm(a,b) = (a×b) / gcd(a,b)
    lcm_val = (a * b) // gcd_val
    
    print(f"\nStep 2: Apply formula")
    print(f"  lcm({a}, {b}) = ({a} × {b}) / {gcd_val}")
    print(f"  = {a * b} / {gcd_val}")
    print(f"  = {lcm_val}")
    
    print(f"\nGeometric Interpretation:")
    print(f"  LCM is the smallest clock that both numbers complete")
    print(f"  {a} completes {lcm_val // a} rotations")
    print(f"  {b} completes {lcm_val // b} rotations")
    print(f"  They meet at position 0 (start) after {lcm_val} steps")
    
    return lcm_val

def babylonian_lcm_simple(a: int, b: int) -> int:
    """Simplified LCM using Python's built-in"""
    a, b = abs(a), abs(b)
    if a == 0 or b == 0:
        return 0
    return (a * b) // math.gcd(a, b)

def test_gcd():
    """Test Babylonian GCD"""
    print("=" * 60)
    print("BABYLONIAN GCD PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (48, 18),    # gcd(48, 18) = 6
        (100, 35),   # gcd(100, 35) = 5
        (17, 19),    # gcd(17, 19) = 1 (coprime)
        (24, 36),    # gcd(24, 36) = 12
        (1071, 462), # gcd(1071, 462) = 21 (Euclid's example)
        (270, 192),  # gcd(270, 192) = 6
    ]
    
    for a, b in test_cases:
        print(f"\n{'='*60}")
        print(f"TEST CASE: gcd({a}, {b})")
        expected = math.gcd(a, b)
        print(f"Expected: {expected}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = babylonian_gcd_geometric(a, b)
        
        # Test simple version
        result_simple = babylonian_gcd_simple(a, b)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric}")
        print(f"  Simple: {result_simple}")
        print(f"  Expected: {expected}")
        print(f"  Geometric Match: {result_geometric == expected}")
        print(f"  Simple Match: {result_simple == expected}")
        print(f"{'='*60}\n")

def test_lcm():
    """Test Babylonian LCM"""
    print("\n" + "=" * 60)
    print("BABYLONIAN LCM PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (12, 18),    # lcm(12, 18) = 36
        (4, 6),      # lcm(4, 6) = 12
        (21, 6),     # lcm(21, 6) = 42
        (15, 25),    # lcm(15, 25) = 75
        (7, 13),     # lcm(7, 13) = 91 (coprime)
    ]
    
    for a, b in test_cases:
        print(f"\n{'='*60}")
        print(f"TEST CASE: lcm({a}, {b})")
        expected = (a * b) // math.gcd(a, b)
        print(f"Expected: {expected}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = babylonian_lcm_geometric(a, b)
        
        # Test simple version
        result_simple = babylonian_lcm_simple(a, b)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric}")
        print(f"  Simple: {result_simple}")
        print(f"  Expected: {expected}")
        print(f"  Geometric Match: {result_geometric == expected}")
        print(f"  Simple Match: {result_simple == expected}")
        print(f"{'='*60}\n")

def demonstrate_euclidean_algorithm():
    """Demonstrate the Euclidean algorithm"""
    print("\n" + "="*60)
    print("EUCLIDEAN ALGORITHM: ANCIENT WISDOM")
    print("="*60)
    
    a, b = 1071, 462
    
    print(f"\nFinding gcd({a}, {b}) - Euclid's original example!")
    print(f"\nThe algorithm:")
    print(f"  1. Divide {a} by {b}, get remainder")
    print(f"  2. Replace {a} with {b}, {b} with remainder")
    print(f"  3. Repeat until remainder is 0")
    print(f"  4. The last non-zero remainder is the GCD")
    
    print(f"\nStep-by-step:")
    step = 0
    while b != 0:
        q, r = divmod(a, b)
        print(f"  Step {step}: {a} = {q} × {b} + {r}")
        a, b = b, r
        step += 1
    
    print(f"\nGCD = {a}")
    print(f"\nWhy this works:")
    print(f"  - Any common divisor of a and b also divides (a mod b)")
    print(f"  - So gcd(a, b) = gcd(b, a mod b)")
    print(f"  - We keep reducing until we find the GCD!")

def demonstrate_geometric_insight():
    """Demonstrate the geometric insight of GCD"""
    print("\n" + "="*60)
    print("GEOMETRIC INSIGHT: GCD AS CLOCK SIZE")
    print("="*60)
    
    a, b = 48, 18
    gcd_val = math.gcd(a, b)
    
    print(f"\nFinding gcd({a}, {b}) = {gcd_val}")
    print(f"\nGeometric interpretation:")
    print(f"  - Imagine clocks of different sizes")
    print(f"  - {a} fits perfectly on a {gcd_val}-clock: {a // gcd_val} complete rotations")
    print(f"  - {b} fits perfectly on a {gcd_val}-clock: {b // gcd_val} complete rotations")
    print(f"  - {gcd_val} is the LARGEST clock both fit on!")
    
    print(f"\nWhy this matters:")
    print(f"  - GCD finds the fundamental period")
    print(f"  - It's the basic unit both numbers are built from")
    print(f"  - In music: GCD finds the fundamental frequency")
    print(f"  - In geometry: GCD finds the basic tile size")

def demonstrate_lcm_insight():
    """Demonstrate the geometric insight of LCM"""
    print("\n" + "="*60)
    print("GEOMETRIC INSIGHT: LCM AS MEETING POINT")
    print("="*60)
    
    a, b = 12, 18
    lcm_val = (a * b) // math.gcd(a, b)
    
    print(f"\nFinding lcm({a}, {b}) = {lcm_val}")
    print(f"\nGeometric interpretation:")
    print(f"  - Two runners on a circular track")
    print(f"  - Runner A completes a lap every {a} seconds")
    print(f"  - Runner B completes a lap every {b} seconds")
    print(f"  - They meet at the start after {lcm_val} seconds!")
    
    print(f"\nVerification:")
    print(f"  - Runner A: {lcm_val // a} laps in {lcm_val} seconds")
    print(f"  - Runner B: {lcm_val // b} laps in {lcm_val} seconds")
    print(f"  - Both back at start position!")
    
    print(f"\nWhy this matters:")
    print(f"  - LCM finds synchronization points")
    print(f"  - In music: LCM finds when rhythms align")
    print(f"  - In astronomy: LCM finds planetary alignments")
    print(f"  - In scheduling: LCM finds when events coincide")

if __name__ == "__main__":
    test_gcd()
    test_lcm()
    demonstrate_euclidean_algorithm()
    demonstrate_geometric_insight()
    demonstrate_lcm_insight()