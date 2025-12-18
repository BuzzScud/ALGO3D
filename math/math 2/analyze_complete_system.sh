#!/bin/bash

# Complete System Analysis Script
# Analyzes entire repository to identify gaps and create action plan

echo "=== CRYSTALLINE COMPLETE SYSTEM ANALYSIS ==="
echo ""

# 1. Count lines of code
echo "1. CODE STATISTICS"
echo "=================="
echo "Math Library:"
find math -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
echo ""
echo "Algorithms Library:"
find algorithms -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
echo ""
echo "Prototypes:"
find prototypes -name "*.c" -o -name "*.h" | xargs wc -l | tail -1
echo ""
echo "Thesis:"
wc -l thesis/THESIS_COMPLETE.md
echo ""

# 2. Check for key implementations
echo "2. KEY IMPLEMENTATIONS CHECK"
echo "============================"
echo "88D Architecture:"
find . -name "*88d*" -o -name "*architecture_88d*" | grep -v ".git" | grep -v "thesis" | grep -v "DEEP_ANALYSIS"
echo ""
echo "Duality:"
find . -name "*duality*" | grep -v ".git" | grep -v "thesis" | grep -v "DEEP_ANALYSIS"
echo ""
echo "Sphere Boundary:"
find . -name "*sphere_boundary*" -o -name "*boundary*" | grep -v ".git" | grep -v "thesis"
echo ""
echo "Frequency:"
find . -name "*frequency*" -o -name "*cymatic*" | grep -v ".git" | grep -v "thesis"
echo ""

# 3. Check for Babylonian operations
echo "3. BABYLONIAN OPERATIONS CHECK"
echo "=============================="
echo "Quadrant folding:"
grep -r "quadrant" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""
echo "Polarity tracking:"
grep -r "polarity" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""
echo "Clock triangle:"
grep -r "clock_triangle" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""

# 4. Check for dead code
echo "4. POTENTIAL DEAD CODE"
echo "======================"
echo "Legacy files:"
find . -name "*legacy*" -o -name "*old*" -o -name "*backup*" | grep -v ".git" | wc -l
echo ""
echo "Unused test files:"
find . -name "test_*.c" -type f | wc -l
echo ""

# 5. Check constants
echo "5. CONSTANTS CHECK"
echo "=================="
echo "PI definitions:"
grep -r "define.*PI" math/ algorithms/ --include="*.h" | wc -l
echo ""
echo "PHI definitions:"
grep -r "define.*PHI" math/ algorithms/ --include="*.h" | wc -l
echo ""

# 6. Check for thesis concepts in code
echo "6. THESIS CONCEPTS IN CODE"
echo "=========================="
echo "Kissing spheres:"
grep -r "kissing" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""
echo "Platonic solids:"
grep -r "platonic" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""
echo "NTT:"
grep -r "ntt" math/ algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""
echo "Blind recovery:"
grep -r "blind_recovery" algorithms/ --include="*.c" --include="*.h" | wc -l
echo ""

echo ""
echo "=== ANALYSIS COMPLETE ==="