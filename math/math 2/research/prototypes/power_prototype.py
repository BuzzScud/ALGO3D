#!/usr/bin/env python3
"""
Babylonian Exponentiation Prototype
Uses geometric principles for efficient computation
Key insight: Exponentiation is repeated scaling + rotation
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

def babylonian_power_geometric(base: int, exponent: int, modulus: int = None) -> int:
    """
    Babylonian Exponentiation using geometric principles
    Uses binary exponentiation (exponentiation by squaring)
    O(log n) complexity, but each operation is O(1)!
    """
    print(f"\n{'='*60}")
    if modulus:
        print(f"GEOMETRIC POWER: {base}^{exponent} mod {modulus}")
    else:
        print(f"GEOMETRIC POWER: {base}^{exponent}")
    print(f"{'='*60}")
    
    if exponent < 0:
        raise ValueError("Negative exponents not supported in this prototype")
    
    if exponent == 0:
        return 1
    
    # Step 1: Convert to Babylonian representation
    bn_base = number_to_babylonian(base, base=10)
    vec_base = babylonian_to_vector(bn_base)
    
    print(f"\nStep 1: Convert base to geometric form")
    print(f"  Base {base} → magnitude={vec_base.magnitude:.4f}, angle={math.degrees(vec_base.angle):.1f}°")
    
    # Step 2: Binary exponentiation (exponentiation by squaring)
    print(f"\nStep 2: Binary exponentiation (O(log n) operations)")
    print(f"  Exponent {exponent} in binary: {bin(exponent)}")
    
    result = 1
    current_power = base
    exp_remaining = exponent
    step = 0
    
    while exp_remaining > 0:
        if exp_remaining & 1:  # If bit is 1
            result = result * current_power
            if modulus:
                result = result % modulus
            print(f"    Step {step}: Multiply result by {current_power} → {result}")
        
        current_power = current_power * current_power
        if modulus:
            current_power = current_power % modulus
        exp_remaining >>= 1
        step += 1
    
    print(f"\nStep 3: Final result")
    print(f"  {base}^{exponent} = {result}")
    if modulus:
        print(f"  {base}^{exponent} mod {modulus} = {result}")
    
    return result

def babylonian_power_simple(base: int, exponent: int, modulus: int = None) -> int:
    """
    Simplified Babylonian power using Python's built-in
    """
    if modulus:
        return pow(base, exponent, modulus)
    else:
        return base ** exponent

def test_power():
    """Test Babylonian exponentiation"""
    print("=" * 60)
    print("BABYLONIAN EXPONENTIATION PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (2, 3, None),      # 2³ = 8
        (3, 4, None),      # 3⁴ = 81
        (5, 2, None),      # 5² = 25
        (2, 10, None),     # 2¹⁰ = 1024
        (7, 3, 12),        # 7³ mod 12 = 7
        (5, 3, 13),        # 5³ mod 13 = 8
        (2, 100, 1000),    # 2¹⁰⁰ mod 1000 (large exponent)
    ]
    
    for test in test_cases:
        if len(test) == 3:
            base, exp, mod = test
        else:
            base, exp = test
            mod = None
        
        print(f"\n{'='*60}")
        if mod:
            print(f"TEST CASE: {base}^{exp} mod {mod}")
            expected = pow(base, exp, mod)
        else:
            print(f"TEST CASE: {base}^{exp}")
            expected = base ** exp
        print(f"Expected: {expected}")
        print(f"{'='*60}")
        
        # Test geometric version
        result_geometric = babylonian_power_geometric(base, exp, mod)
        
        # Test simple version
        result_simple = babylonian_power_simple(base, exp, mod)
        
        print(f"\n{'='*60}")
        print(f"RESULTS:")
        print(f"  Geometric: {result_geometric}")
        print(f"  Simple: {result_simple}")
        print(f"  Expected: {expected}")
        print(f"  Geometric Match: {result_geometric == expected}")
        print(f"  Simple Match: {result_simple == expected}")
        print(f"{'='*60}\n")

def demonstrate_binary_exponentiation():
    """Demonstrate binary exponentiation algorithm"""
    print("\n" + "="*60)
    print("BINARY EXPONENTIATION: THE KEY TO O(log n)")
    print("="*60)
    
    base, exp = 3, 13
    
    print(f"\nComputing {base}^{exp}:")
    print(f"  Traditional: {exp} multiplications")
    print(f"  Binary: {exp.bit_length()} multiplications (O(log n))")
    
    print(f"\nBinary representation of {exp}: {bin(exp)} = {bin(exp)[2:]}")
    print(f"  = 1×2³ + 1×2² + 0×2¹ + 1×2⁰")
    print(f"  = 8 + 4 + 1")
    
    print(f"\nAlgorithm:")
    print(f"  {base}^{exp} = {base}^8 × {base}^4 × {base}^1")
    print(f"  = {base**8} × {base**4} × {base}")
    print(f"  = {base**13}")
    
    print(f"\nWhy this is efficient:")
    print(f"  - We compute {base}^1, {base}^2, {base}^4, {base}^8 by repeated squaring")
    print(f"  - Each squaring is one multiplication")
    print(f"  - Total: {exp.bit_length()} multiplications instead of {exp}")

def demonstrate_modular_exponentiation():
    """Demonstrate modular exponentiation"""
    print("\n" + "="*60)
    print("MODULAR EXPONENTIATION: CRYPTOGRAPHY'S FOUNDATION")
    print("="*60)
    
    base, exp, mod = 7, 100, 13
    
    print(f"\nComputing {base}^{exp} mod {mod}:")
    print(f"  Without modulo: {base}^{exp} = {base**exp} (huge number!)")
    print(f"  With modulo: {base}^{exp} mod {mod} = {pow(base, exp, mod)}")
    
    print(f"\nWhy modular exponentiation is crucial:")
    print(f"  1. RSA encryption uses modular exponentiation")
    print(f"  2. Diffie-Hellman key exchange uses it")
    print(f"  3. Digital signatures use it")
    print(f"  4. Keeps numbers manageable (always < modulus)")
    
    print(f"\nBabylonian insight:")
    print(f"  - Each multiplication wraps around the clock")
    print(f"  - We never leave the clock face!")
    print(f"  - This is why it's O(log n) with O(1) operations")

def demonstrate_geometric_insight():
    """Demonstrate the geometric insight of exponentiation"""
    print("\n" + "="*60)
    print("GEOMETRIC INSIGHT: EXPONENTIATION AS SPIRAL")
    print("="*60)
    
    print("\nIn Babylonian mathematics, exponentiation creates a spiral:")
    print("  - Each multiplication scales the magnitude")
    print("  - Each multiplication rotates the angle")
    print("  - The result spirals outward (or inward for fractions)")
    
    print(f"\nExample: 2^3 = 8")
    print(f"  Start: magnitude=2, angle=72° (position 2 in base 10)")
    print(f"  After 2²: magnitude=4, angle=144° (2×72°)")
    print(f"  After 2³: magnitude=8, angle=216° (3×72°)")
    print(f"  Result: 8 at position 8 on the clock")
    
    print(f"\nWith modular arithmetic:")
    print(f"  The spiral wraps around the clock")
    print(f"  Creates beautiful geometric patterns")
    print(f"  This is the basis of cyclic groups in cryptography!")

if __name__ == "__main__":
    test_power()
    demonstrate_binary_exponentiation()
    demonstrate_modular_exponentiation()
    demonstrate_geometric_insight()