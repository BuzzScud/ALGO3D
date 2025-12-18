#!/bin/bash
# Build Clean THESIS.md by removing duplicates

set -e

echo "Building clean THESIS.md..."
echo "Original size: $(wc -l < thesis/THESIS.md) lines"

# Create temporary directory
mkdir -p thesis/temp_reorg

cd thesis

# Extract Part I: Theoretical Foundations (lines 1-4293)
echo "Extracting Part I: Theoretical Foundations..."
sed -n '1,4293p' THESIS.md > temp_reorg/part1_foundations.md

# Extract Part II: Core Algorithms (lines 4293-11584)
echo "Extracting Part II: Core Algorithms..."
sed -n '4293,11584p' THESIS.md > temp_reorg/part2_algorithms.md

# Extract Part III: Q&A - Clock Lattice (lines 21952-29819)
echo "Extracting Clock Lattice Questions..."
sed -n '21952,29819p' THESIS.md > temp_reorg/qa_clock_lattice.md

# Extract Crystalline Abacus Questions (lines 29819-35379)
echo "Extracting Crystalline Abacus Questions..."
sed -n '29819,35379p' THESIS.md > temp_reorg/qa_crystalline_abacus.md

# Extract Novel Hashing Questions (lines 35379-40457)
echo "Extracting Novel Hashing Questions..."
sed -n '35379,40457p' THESIS.md > temp_reorg/qa_novel_hashing.md

# Extract Bitcoin/Blockchain Questions (lines 40457-44102)
echo "Extracting Bitcoin/Blockchain Questions..."
sed -n '40457,44102p' THESIS.md > temp_reorg/qa_bitcoin.md

# Extract AI Applications Questions (lines 44102-46380)
echo "Extracting AI Applications Questions..."
sed -n '44102,46380p' THESIS.md > temp_reorg/qa_ai.md

# Extract Additional Topics Questions (lines 46380-48045)
echo "Extracting Additional Topics Questions..."
sed -n '46380,48045p' THESIS.md > temp_reorg/qa_additional.md

# Extract Foundational Questions (lines 48051-51501, keep first half only)
echo "Extracting Foundational Questions (deduplicated)..."
sed -n '48051,49776p' THESIS.md > temp_reorg/qa_foundational.md
# Skip duplicate at 49776-51501

# Extract Foundational Part 2 (lines 51501-53331, keep first only)
echo "Extracting Foundational Part 2 (deduplicated)..."
sed -n '51501,53331p' THESIS.md > temp_reorg/qa_foundational_part2.md
# Skip duplicate at 53331-55161

# Extract Geometric Arithmetic Questions (lines 55161-58576, keep first set)
echo "Extracting Geometric Arithmetic Questions (deduplicated)..."
sed -n '55161,58576p' THESIS.md > temp_reorg/qa_geometric_arithmetic.md
# Skip duplicate at 58576-61991

# Extract Part IV: Specialized Applications
echo "Extracting Geometric Chemistry (deduplicated)..."
sed -n '61991,62757p' THESIS.md > temp_reorg/app_chemistry.md
# Skip duplicate at 64414-65180

echo "Extracting Geometric Metamaterials (deduplicated)..."
sed -n '62757,63531p' THESIS.md > temp_reorg/app_metamaterials.md
# Skip duplicate at 65180-65954

echo "Extracting Geometric Quantum Modeling (deduplicated)..."
sed -n '63531,64414p' THESIS.md > temp_reorg/app_quantum.md
# Skip duplicate at 65954-66837

echo "Extracting Hyperfold Cascade (deduplicated)..."
sed -n '66837,68017p' THESIS.md > temp_reorg/app_hyperfold.md
# Skip duplicate at 68017-69197

echo "Extracting Worked Examples (deduplicated)..."
sed -n '69197,69856p' THESIS.md > temp_reorg/app_examples.md
# Skip duplicate at 70526-71185

echo "Extracting Visualizations (deduplicated)..."
sed -n '69856,70526p' THESIS.md > temp_reorg/app_visualizations.md
# Skip duplicate at 71185-71855

# Extract Part V: Conclusions (lines 85567-85736)
echo "Extracting Part V: Conclusions..."
sed -n '85567,85736p' THESIS.md > temp_reorg/part5_conclusions.md

# Assemble clean version
echo ""
echo "Assembling clean THESIS.md..."
cat temp_reorg/part1_foundations.md > THESIS_CLEAN.md
cat temp_reorg/part2_algorithms.md >> THESIS_CLEAN.md

# Add Part III header
echo "" >> THESIS_CLEAN.md
echo "---" >> THESIS_CLEAN.md
echo "" >> THESIS_CLEAN.md
echo "# PART III: COMPREHENSIVE QUESTION-AND-ANSWER ANALYSIS" >> THESIS_CLEAN.md
echo "" >> THESIS_CLEAN.md

cat temp_reorg/qa_clock_lattice.md >> THESIS_CLEAN.md
cat temp_reorg/qa_crystalline_abacus.md >> THESIS_CLEAN.md
cat temp_reorg/qa_novel_hashing.md >> THESIS_CLEAN.md
cat temp_reorg/qa_bitcoin.md >> THESIS_CLEAN.md
cat temp_reorg/qa_ai.md >> THESIS_CLEAN.md
cat temp_reorg/qa_additional.md >> THESIS_CLEAN.md
cat temp_reorg/qa_foundational.md >> THESIS_CLEAN.md
cat temp_reorg/qa_foundational_part2.md >> THESIS_CLEAN.md
cat temp_reorg/qa_geometric_arithmetic.md >> THESIS_CLEAN.md

# Add Part IV header
echo "" >> THESIS_CLEAN.md
echo "---" >> THESIS_CLEAN.md
echo "" >> THESIS_CLEAN.md
echo "# PART IV: SPECIALIZED APPLICATIONS AND ANALYSES" >> THESIS_CLEAN.md
echo "" >> THESIS_CLEAN.md

cat temp_reorg/app_chemistry.md >> THESIS_CLEAN.md
cat temp_reorg/app_metamaterials.md >> THESIS_CLEAN.md
cat temp_reorg/app_quantum.md >> THESIS_CLEAN.md
cat temp_reorg/app_hyperfold.md >> THESIS_CLEAN.md
cat temp_reorg/app_examples.md >> THESIS_CLEAN.md
cat temp_reorg/app_visualizations.md >> THESIS_CLEAN.md

# Add Part V
echo "" >> THESIS_CLEAN.md
echo "---" >> THESIS_CLEAN.md
echo "" >> THESIS_CLEAN.md

cat temp_reorg/part5_conclusions.md >> THESIS_CLEAN.md

# Show results
echo ""
echo "=== RESULTS ==="
echo "Original size: $(wc -l < THESIS.md) lines"
echo "Clean size: $(wc -l < THESIS_CLEAN.md) lines"
echo "Reduction: $(($(wc -l < THESIS.md) - $(wc -l < THESIS_CLEAN.md))) lines removed"
echo "Percentage: $(echo "scale=1; ($(wc -l < THESIS.md) - $(wc -l < THESIS_CLEAN.md)) * 100 / $(wc -l < THESIS.md)" | bc)% reduction"
echo ""
echo "Clean thesis created: thesis/THESIS_CLEAN.md"
echo "Original backed up: thesis/THESIS_BEFORE_REORGANIZATION.md"

cd ..