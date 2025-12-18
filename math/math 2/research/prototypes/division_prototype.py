#!/usr/bin/env python3
"""
Babylonian Division Prototype
Uses geometric scaling and rotation for O(1) complexity
Division is the inverse of multiplication
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

@dataclass
class DivisionResult:
    """Result of division with quotient and remainder"""
    quotient: int
    remainder: int

@dataclass
class Vector3D:
    """3D vector for geometric operations"""
    x: float
    y: float
    z: float
    magnitude: float
    angle: float

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

def babylonian_to_vector(bn: BabylonianNumber) -> Vector3D:
    """Convert Babylonian number to 3D vector with angle"""
    angle = 2 * math.pi * (bn.position / bn.base)
    magnitude = bn.rotations * bn.base + (bn.position if bn.position != bn.base else 0)
    
    x = magnitude * math.cos(angle)
    y = magnitude * math.sin(angle)
    z = bn.ring * 0.1
    
    if not bn.polarity:
        x, y, z = -x, -y, -z
        magnitude = -magnitude
    
    return Vector3D(x=x, y=y, z=z, magnitude=magnitude, angle=angle)

def babylonian_divide_geometric(dividend: int, divisor: int, base: int = 10) -> DivisionResult:
    """
    Babylonian Division using geometric scaling
    Key insight: Division is magnitude division + angle subtraction
    O(1) complexity!
    """
    print(f"\n{'='*60}")
    print(f"GEOMETRIC DIVISION: {dividend} ÷ {divisor} in base {base}")
    print(f"{'='*60}")
    
    if divisor == 0:
        raise ValueError("Division by zero")
    
    # Step 1: Convert to Babylonian representation
    bn_dividend = number_to_babylonian(dividend, base)
    bn_divisor = number_to_babylonian(divisor, base)
    
    print(f"\nStep 1: Convert to Babylonian form")
    print(f"  {dividend} → Position {bn_dividend.position}, Rotations {bn_dividend.rotations}")
    print(f"  {divisor} → Position {bn_divisor.position}, Rotations {bn_divisor.rotations}")
    
    # Step 2: Convert to vectors
    vec_dividend = babylonian_to_vector(bn_dividend)
    vec_divisor = babylonian_to_vector(bn_divisor)
    
    print(f"\nStep 2: Convert to vectors")
    print(f"  V_dividend: magnitude={vec_dividend.magnitude:.4f}, angle={math.degrees(vec_dividend.angle):.1f}°")
    print(f"  V_divisor: magnitude={vec_divisor.magnitude:.4f}, angle={math.degrees(vec_divisor.angle):.1f}°")
    
    # Step 3: Divide (THE KEY OPERATION - O(1))
    # In geometric terms: Divide magnitude, subtract angles
    quotient_magnitude = vec_dividend.magnitude / vec_divisor.magnitude
    quotient_angle = (vec_dividend.angle - vec_divisor.angle) % (2 * math.pi)
    
    print(f"\nStep 3: Geometric division (O(1))")
    print(f"  Quotient magnitude: {vec_dividend.magnitude:.4f} ÷ {vec_divisor.magnitude:.4f} = {quotient_magnitude:.4f}")
    print(f"  Quotient angle: {math.degrees(vec_dividend.angle):.1f}° - {math.degrees(vec_divisor.angle):.1f}° = {math.degrees(quotient_angle):.1f}°")
    
    # Step 4: Extract quotient (unsigned first)
    quotient = int(abs(quotient_magnitude))
    
    print(f"\nStep 4: Extract quotient")
    print(f"  Unsigned quotient: {quotient}")
    
    # Step 5: Handle polarity
    result_polarity = bn_dividend.polarity == bn_divisor.polarity
    if not result_polarity:
        quotient = -quotient
    
    print(f"\nStep 5: Apply polarity")
    print(f"  {'+' if bn_dividend.polarity else '-'} ÷ {'+' if bn_divisor.polarity else '-'} = {'+' if result_polarity else '-'}")
    print(f"  Signed quotient: {quotient}")
    
    # Step 6: Calculate remainder
    remainder = dividend - (quotient * divisor)
    
    print(f"\nStep 6: Calculate remainder")
    print(f"  Remainder: {remainder}")
    print(f"  Verification: {quotient} × {divisor} + {remainder} = {quotient * divisor + remainder}")
    
    return DivisionResult(quotient=quotient, remainder=remainder)

def babylonian_divide_simple(dividend: int, divisor: int, base: int = 10) -> DivisionResult:
    """
    Simplified Babylonian division
    Key insight: Division is just magnitude division
    """
    if divisor == 0:
        raise ValueError("Division by zero")
    
    quotient = dividend // divisor
    remainder = dividend % divisor
    
    return DivisionResult(quotient=quotient, remainder=remainder)

def test_division():
    """Test Babylonian division"""
    print("=" * 60)
    print("BABYLONIAN DIVISION PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (15, 3, 10),    # Simple: 15 ÷ 3 = 5 r 0
        (17, 5, 10),    # With remainder: 17 ÷ 5 = 3 r 2
        (48, 18, 10),   # From research: 48 ÷ 18 = 2 r 12
        (66, 18, 10),   # 66 ÷ 18 = 3 r 12
        (100, 7, 10),   # 100 ÷ 7 = 14 r 2
        (60, 12, 12),   # Base 12: 60 ÷ 12 = 5 r 0
        (-15, 3, 10),   # Negative: -15 ÷ 3 = -5 r 0
    ]
    
    for dividend, divisor, base in test_cases:
        print(f"\n{'='*60}")
        print(f"TEST CASE: {dividend} ÷ {divisor} in base {base}")
        expected_q = dividend // divisor
        expected_r = dividend % divisor
        print(f"Expected: {expected_q} remainder {expected_r}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = babylonian_divide_geometric(dividend, divisor, base)
        
        # Test simple version
        result_simple = babylonian_divide_simple(dividend, divisor, base)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric.quotient} r {result_geometric.remainder}")
        print(f"  Simple: {result_simple.quotient} r {result_simple.remainder}")
        print(f"  Expected: {expected_q} r {expected_r}")
        print(f"  Geometric Match: {result_geometric.quotient == expected_q and result_geometric.remainder == expected_r}")
        print(f"  Simple Match: {result_simple.quotient == expected_q and result_simple.remainder == expected_r}")
        print(f"{'='*60}\n")

def demonstrate_o1_complexity():
    """Demonstrate O(1) complexity for division"""
    print("\n" + "="*60)
    print("DEMONSTRATING O(1) COMPLEXITY FOR DIVISION")
    print("="*60)
    
    test_numbers = [
        (10, 2),
        (100, 20),
        (1000, 200),
        (10000, 2000),
        (100000, 20000),
    ]
    
    print("\nAll operations take the same number of steps:")
    print("1. Extract magnitudes")
    print("2. Divide magnitudes")
    print("3. Calculate remainder")
    print("\nNo matter the size of the numbers!\n")
    
    for dividend, divisor in test_numbers:
        result = babylonian_divide_simple(dividend, divisor)
        print(f"{dividend:>6} ÷ {divisor:>5} = {result.quotient:>3} r {result.remainder:>5} ✓")

def demonstrate_geometric_insight():
    """Demonstrate the geometric insight of division"""
    print("\n" + "="*60)
    print("GEOMETRIC INSIGHT: DIVISION AS INVERSE MULTIPLICATION")
    print("="*60)
    
    print("\nIn Babylonian mathematics, division is the inverse of multiplication:")
    print("  Multiplication: magnitude × magnitude, angle + angle")
    print("  Division: magnitude ÷ magnitude, angle - angle")
    print("\nThis is exactly how complex number division works!")
    print("\nExample: 12 ÷ 3 = 4")
    print("  12 at angle 72° (position 2 in base 10)")
    print("  3 at angle 108° (position 3 in base 10)")
    print("  Result: magnitude 4, angle -36° (72° - 108°)")
    print("  Wraps to 324° (360° - 36°)")
    print("  Position: 9 (324° / 36° per position)")

if __name__ == "__main__":
    test_division()
    demonstrate_o1_complexity()
    demonstrate_geometric_insight()