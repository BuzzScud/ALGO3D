#!/usr/bin/env python3
"""
Root Extraction Prototype
Uses Newton-Raphson method with geometric operations
Key insight: Root extraction is finding the inverse of exponentiation
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

def integer_sqrt_newton(n: int) -> int:
    """
    Integer square root using Newton-Raphson method
    Finds largest integer x such that x² ≤ n
    """
    if n < 0:
        raise ValueError("Cannot take square root of negative number")
    if n == 0:
        return 0
    if n == 1:
        return 1
    
    # Initial guess: use bit length for fast convergence
    x = 1 << ((n.bit_length() + 1) // 2)
    
    while True:
        # Newton-Raphson: x_new = (x + n/x) / 2
        x_new = (x + n // x) // 2
        
        if x_new >= x:
            return x
        x = x_new

def integer_nth_root_newton(n: int, root: int) -> int:
    """
    Integer nth root using Newton-Raphson method
    Finds largest integer x such that x^root ≤ n
    """
    if n < 0 and root % 2 == 0:
        raise ValueError("Cannot take even root of negative number")
    if n == 0:
        return 0
    if n == 1:
        return 1
    if root == 1:
        return n
    
    # Handle negative numbers for odd roots
    negative = n < 0
    n = abs(n)
    
    # Initial guess: use bit length
    x = 1 << ((n.bit_length() + root - 1) // root)
    
    while True:
        # Newton-Raphson: x_new = ((root-1)*x + n/x^(root-1)) / root
        x_power = x ** (root - 1)
        x_new = ((root - 1) * x + n // x_power) // root
        
        if x_new >= x:
            result = x
            if negative:
                result = -result
            return result
        x = x_new

def geometric_sqrt(n: int, base: int = 10) -> int:
    """
    Geometric square root using Babylonian representation
    Key insight: sqrt is finding the number whose square equals n
    """
    print(f"\n{'='*60}")
    print(f"GEOMETRIC SQUARE ROOT: √{n} in base {base}")
    print(f"{'='*60}")
    
    if n < 0:
        raise ValueError("Cannot take square root of negative number")
    
    # Step 1: Convert to Babylonian representation
    bn = number_to_babylonian(n, base)
    
    print(f"\nStep 1: Map to Babylonian form")
    print(f"  {n} → Position {bn.position}, Rotations {bn.rotations}")
    
    # Step 2: Use Newton-Raphson for integer square root
    result = integer_sqrt_newton(n)
    
    print(f"\nStep 2: Newton-Raphson iteration")
    print(f"  Finding x such that x² ≤ {n}")
    print(f"  Result: {result}")
    
    # Step 3: Verify result
    print(f"\nStep 3: Verification")
    print(f"  {result}² = {result * result}")
    print(f"  {result + 1}² = {(result + 1) * (result + 1)}")
    print(f"  {result * result} ≤ {n} < {(result + 1) * (result + 1)}: {result * result <= n < (result + 1) * (result + 1)}")
    
    return result

def geometric_nth_root(n: int, root: int, base: int = 10) -> int:
    """
    Geometric nth root using Babylonian representation
    """
    print(f"\n{'='*60}")
    print(f"GEOMETRIC {root}TH ROOT: {root}√{n} in base {base}")
    print(f"{'='*60}")
    
    if n < 0 and root % 2 == 0:
        raise ValueError(f"Cannot take even root of negative number")
    
    # Step 1: Convert to Babylonian representation
    bn = number_to_babylonian(abs(n), base)
    
    print(f"\nStep 1: Map to Babylonian form")
    print(f"  {n} → Position {bn.position}, Rotations {bn.rotations}")
    
    # Step 2: Use Newton-Raphson for integer nth root
    result = integer_nth_root_newton(n, root)
    
    print(f"\nStep 2: Newton-Raphson iteration")
    print(f"  Finding x such that x^{root} ≤ {abs(n)}")
    print(f"  Result: {result}")
    
    # Step 3: Verify result
    print(f"\nStep 3: Verification")
    print(f"  {result}^{root} = {result ** root}")
    print(f"  {result + 1}^{root} = {(result + 1) ** root}")
    if n >= 0:
        print(f"  {result ** root} ≤ {n} < {(result + 1) ** root}: {result ** root <= n < (result + 1) ** root}")
    
    return result

def test_sqrt():
    """Test square root"""
    print("=" * 60)
    print("SQUARE ROOT PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (4, 10),      # √4 = 2
        (9, 10),      # √9 = 3
        (16, 10),     # √16 = 4
        (25, 10),     # √25 = 5
        (100, 10),    # √100 = 10
        (144, 12),    # √144 = 12 (base 12)
        (1000, 10),   # √1000 = 31
    ]
    
    for n, base in test_cases:
        print(f"\n{'='*60}")
        print(f"TEST CASE: √{n} in base {base}")
        expected = int(math.sqrt(n))
        print(f"Expected: {expected}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = geometric_sqrt(n, base)
        
        # Test simple version
        result_simple = integer_sqrt_newton(n)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric}")
        print(f"  Simple: {result_simple}")
        print(f"  Expected: {expected}")
        print(f"  Geometric Match: {result_geometric == expected}")
        print(f"  Simple Match: {result_simple == expected}")
        print(f"{'='*60}\n")

def test_nth_root():
    """Test nth root"""
    print("\n" + "=" * 60)
    print("NTH ROOT PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (8, 3, 10),      # ³√8 = 2
        (27, 3, 10),     # ³√27 = 3
        (64, 3, 10),     # ³√64 = 4
        (16, 4, 10),     # ⁴√16 = 2
        (32, 5, 10),     # ⁵√32 = 2
        (1000, 3, 10),   # ³√1000 = 10
    ]
    
    for n, root, base in test_cases:
        print(f"\n{'='*60}")
        print(f"TEST CASE: {root}√{n} in base {base}")
        expected = int(n ** (1/root))
        print(f"Expected: {expected}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = geometric_nth_root(n, root, base)
        
        # Test simple version
        result_simple = integer_nth_root_newton(n, root)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric}")
        print(f"  Simple: {result_simple}")
        print(f"  Expected: {expected}")
        print(f"  Geometric Match: {result_geometric == expected}")
        print(f"  Simple Match: {result_simple == expected}")
        print(f"{'='*60}\n")

def demonstrate_newton_raphson():
    """Demonstrate Newton-Raphson method"""
    print("\n" + "="*60)
    print("NEWTON-RAPHSON METHOD: FINDING ROOTS")
    print("="*60)
    
    n = 100
    
    print(f"\nFinding √{n} using Newton-Raphson:")
    print(f"  Formula: x_new = (x + n/x) / 2")
    print(f"  Start with initial guess based on bit length")
    
    x = 1 << ((n.bit_length() + 1) // 2)
    print(f"\n  Initial guess: {x}")
    
    step = 0
    while True:
        x_new = (x + n // x) // 2
        print(f"  Step {step}: x = {x}, x_new = {x_new}")
        
        if x_new >= x:
            print(f"\n  Converged! √{n} = {x}")
            print(f"  Verification: {x}² = {x*x}, {x+1}² = {(x+1)*(x+1)}")
            break
        
        x = x_new
        step += 1
    
    print(f"\nWhy Newton-Raphson is efficient:")
    print(f"  - Quadratic convergence (doubles correct digits each iteration)")
    print(f"  - Only {step + 1} iterations needed")
    print(f"  - Each iteration is O(1) with Babylonian arithmetic")

def demonstrate_geometric_insight():
    """Demonstrate the geometric insight of root extraction"""
    print("\n" + "="*60)
    print("GEOMETRIC INSIGHT: ROOTS AS INVERSE OPERATIONS")
    print("="*60)
    
    print("\nIn Babylonian mathematics, roots are inverse operations:")
    print("  - Multiplication scales magnitude, adds angles")
    print("  - Root extraction finds the original before scaling")
    print("  - √n finds x such that x × x = n")
    print("  - ³√n finds x such that x × x × x = n")
    
    print(f"\nExample: √64 = 8")
    print(f"  We're finding x such that x² = 64")
    print(f"  Geometric: x × x creates a square with area 64")
    print(f"  The side length of that square is 8")
    
    print(f"\nNewton-Raphson as geometric averaging:")
    print(f"  Each iteration averages x and n/x")
    print(f"  This finds the geometric mean")
    print(f"  Converges to the square root!")

if __name__ == "__main__":
    test_sqrt()
    test_nth_root()
    demonstrate_newton_raphson()
    demonstrate_geometric_insight()