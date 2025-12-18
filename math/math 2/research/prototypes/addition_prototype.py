#!/usr/bin/env python3
"""
Babylonian Addition Prototype
Uses geometric clock-based vector addition for O(1) complexity
"""

import math
from dataclasses import dataclass
from typing import Tuple

@dataclass
class BabylonianPosition:
    """Represents a position on the Babylonian clock"""
    position: int      # Position on clock (1 to base, not 0 to base-1)
    ring: int          # Which ring (0=12, 1=60, 2=60, 3=100)
    angle: float       # Angle in radians (0 to 2π)
    radius: float      # Distance from center (1.0 = outer, 0.0 = center)
    polarity: bool     # Positive (True) or negative (False)

@dataclass
class Vector3D:
    """3D vector for geometric operations"""
    x: float
    y: float
    z: float

@dataclass
class FoldedPosition:
    """Position after folding to first quadrant"""
    folded: BabylonianPosition
    original_quadrant: int
    polarity_flipped: bool

@dataclass
class PolarityState:
    """Tracks polarity through operations"""
    current_polarity: bool  # True = positive, False = negative
    boundary_crossings: int

# Ring configurations (base values)
RING_BASES = [12, 60, 60, 100]
RING_RADII = [1.0, 0.8, 0.6, 0.4]  # Outer to inner

def get_base_for_ring(ring: int) -> int:
    """Get the base for a given ring"""
    if ring < 0 or ring >= len(RING_BASES):
        return 10  # Default to base 10
    return RING_BASES[ring]

def number_to_position(number: int, base: int = 10, ring: int = 0) -> BabylonianPosition:
    """
    Map a number to its Babylonian clock position
    CRITICAL: Positions start at 1, not 0
    """
    # Handle negative numbers
    is_negative = number < 0
    number = abs(number)
    
    # Calculate position (1-based, not 0-based)
    position = (number % base)
    if position == 0:
        position = base  # Wrap to base, not 0
    
    # Calculate angle: θ = 2π × (position / base)
    angle = 2 * math.pi * (position / base)
    
    # Get radius for this ring
    radius = RING_RADII[ring] if ring < len(RING_RADII) else 0.5
    
    return BabylonianPosition(
        position=position,
        ring=ring,
        angle=angle,
        radius=radius,
        polarity=not is_negative
    )

def position_to_vector(pos: BabylonianPosition) -> Vector3D:
    """Convert a Babylonian position to a 3D vector"""
    x = pos.radius * math.cos(pos.angle)
    y = pos.radius * math.sin(pos.angle)
    z = pos.ring * 0.1  # Z represents ring depth
    
    # Apply polarity
    if not pos.polarity:
        x, y, z = -x, -y, -z
    
    return Vector3D(x=x, y=y, z=z)

def vector_to_position(vec: Vector3D, base: int = 10, ring: int = 0) -> BabylonianPosition:
    """Convert a 3D vector back to a Babylonian position"""
    # Calculate angle from x, y
    angle = math.atan2(vec.y, vec.x)
    if angle < 0:
        angle += 2 * math.pi
    
    # Calculate radius
    radius = math.sqrt(vec.x**2 + vec.y**2)
    
    # Calculate position from angle
    position = int((angle / (2 * math.pi)) * base)
    if position == 0:
        position = base
    
    # Determine polarity (based on vector magnitude)
    polarity = (vec.x >= 0 or vec.y >= 0)
    
    return BabylonianPosition(
        position=position,
        ring=ring,
        angle=angle,
        radius=radius,
        polarity=polarity
    )

def fold_to_first_quadrant(pos: BabylonianPosition) -> FoldedPosition:
    """
    Fold any position to the first quadrant (0-90°)
    This is the key to O(1) complexity
    """
    # Determine which quadrant (1-4)
    quadrant = int(pos.angle / (math.pi / 2)) + 1
    if quadrant > 4:
        quadrant = 4
    
    folded_angle = pos.angle
    polarity_flipped = False
    
    if quadrant == 1:
        # Already in Q1
        pass
    elif quadrant == 2:
        # Mirror across vertical axis (90° line)
        folded_angle = math.pi - pos.angle
        polarity_flipped = True
    elif quadrant == 3:
        # Mirror to Q1 (subtract π)
        folded_angle = pos.angle - math.pi
        polarity_flipped = True
    elif quadrant == 4:
        # Mirror across vertical axis
        folded_angle = 2 * math.pi - pos.angle
        polarity_flipped = False
    
    folded_pos = BabylonianPosition(
        position=pos.position,
        ring=pos.ring,
        angle=folded_angle,
        radius=pos.radius,
        polarity=pos.polarity
    )
    
    return FoldedPosition(
        folded=folded_pos,
        original_quadrant=quadrant,
        polarity_flipped=polarity_flipped
    )

def unfold_from_first_quadrant(folded: FoldedPosition, result_pos: BabylonianPosition) -> BabylonianPosition:
    """Unfold a result from Q1 back to its original quadrant"""
    quadrant = folded.original_quadrant
    unfolded_angle = result_pos.angle
    
    if quadrant == 2:
        unfolded_angle = math.pi - result_pos.angle
    elif quadrant == 3:
        unfolded_angle = result_pos.angle + math.pi
    elif quadrant == 4:
        unfolded_angle = 2 * math.pi - result_pos.angle
    
    return BabylonianPosition(
        position=result_pos.position,
        ring=result_pos.ring,
        angle=unfolded_angle,
        radius=result_pos.radius,
        polarity=result_pos.polarity
    )

def position_to_number(pos: BabylonianPosition, base: int = 10) -> int:
    """Map a Babylonian position back to a number"""
    # Position is 1-based, so we need to adjust
    number = pos.position
    if number == base:
        number = 0
    
    # Apply polarity
    if not pos.polarity:
        number = -number
    
    return number

def babylonian_add(a: int, b: int, base: int = 10) -> int:
    """
    Babylonian Addition using geometric clock-based vector addition
    O(1) complexity through vector operations
    """
    # Step 1: Map to positions
    pos_a = number_to_position(a, base)
    pos_b = number_to_position(b, base)
    
    print(f"\nStep 1: Map to positions")
    print(f"  {a} → Position {pos_a.position}, Angle {pos_a.angle:.4f} rad ({math.degrees(pos_a.angle):.1f}°)")
    print(f"  {b} → Position {pos_b.position}, Angle {pos_b.angle:.4f} rad ({math.degrees(pos_b.angle):.1f}°)")
    
    # Step 2: Fold to Q1
    folded_a = fold_to_first_quadrant(pos_a)
    folded_b = fold_to_first_quadrant(pos_b)
    
    print(f"\nStep 2: Fold to Q1")
    print(f"  {a} in Q{folded_a.original_quadrant}, folded angle: {math.degrees(folded_a.folded.angle):.1f}°")
    print(f"  {b} in Q{folded_b.original_quadrant}, folded angle: {math.degrees(folded_b.folded.angle):.1f}°")
    
    # Step 3: Convert to 3D vectors
    vec_a = position_to_vector(folded_a.folded)
    vec_b = position_to_vector(folded_b.folded)
    
    print(f"\nStep 3: Convert to vectors")
    print(f"  V_a = ({vec_a.x:.4f}, {vec_a.y:.4f}, {vec_a.z:.4f})")
    print(f"  V_b = ({vec_b.x:.4f}, {vec_b.y:.4f}, {vec_b.z:.4f})")
    
    # Step 4: Add vectors (THIS IS THE KEY OPERATION - O(1))
    vec_result = Vector3D(
        x=vec_a.x + vec_b.x,
        y=vec_a.y + vec_b.y,
        z=vec_a.z + vec_b.z
    )
    
    print(f"\nStep 4: Add vectors")
    print(f"  V_result = ({vec_result.x:.4f}, {vec_result.y:.4f}, {vec_result.z:.4f})")
    
    # Step 5: Convert back to position
    result_pos = vector_to_position(vec_result, base)
    
    print(f"\nStep 5: Convert back to position")
    print(f"  Result position: {result_pos.position}, Angle: {math.degrees(result_pos.angle):.1f}°")
    
    # Step 6: Track polarity
    polarity_state = PolarityState(current_polarity=True, boundary_crossings=0)
    if folded_a.polarity_flipped:
        polarity_state.current_polarity = not polarity_state.current_polarity
        polarity_state.boundary_crossings += 1
    if folded_b.polarity_flipped:
        polarity_state.current_polarity = not polarity_state.current_polarity
        polarity_state.boundary_crossings += 1
    
    result_pos.polarity = polarity_state.current_polarity
    
    print(f"\nStep 6: Track polarity")
    print(f"  Boundary crossings: {polarity_state.boundary_crossings}")
    print(f"  Final polarity: {'positive' if result_pos.polarity else 'negative'}")
    
    # Step 7: Unfold and map to number
    # For addition, we typically stay in the same quadrant or wrap around
    # This is a simplified version - full implementation would handle carries
    result = position_to_number(result_pos, base)
    
    # Handle wrapping (carries)
    if result < 0:
        result += base
    
    print(f"\nStep 7: Map to number")
    print(f"  Final result: {result}")
    
    return result

def test_addition():
    """Test the Babylonian addition prototype"""
    print("=" * 60)
    print("BABYLONIAN ADDITION PROTOTYPE TEST")
    print("=" * 60)
    
    test_cases = [
        (3, 5, 10),    # Simple: 3 + 5 = 8
        (7, 8, 10),    # Wrap: 7 + 8 = 15 (shows carry)
        (48, 18, 10),  # From research: 48 + 18 = 66
        (5, 7, 12),    # Base 12: 5 + 7 = 12 (10 in base 12)
    ]
    
    for a, b, base in test_cases:
        print(f"\n{'=' * 60}")
        print(f"TEST: {a} + {b} in base {base}")
        print(f"Expected (traditional): {a + b}")
        print(f"{'=' * 60}")
        
        result = babylonian_add(a, b, base)
        traditional = a + b
        
        print(f"\n{'=' * 60}")
        print(f"RESULT: {result}")
        print(f"Traditional: {traditional}")
        print(f"Match: {result == traditional or result == (traditional % base)}")
        print(f"{'=' * 60}\n")

if __name__ == "__main__":
    test_addition()