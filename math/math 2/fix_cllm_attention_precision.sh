#!/bin/bash
# Script to fix float-to-double precision issues in cllm_attention.c

FILE="src/ai/cllm_attention.c"
BACKUP="${FILE}.backup"

echo "Creating backup: ${BACKUP}"
cp "${FILE}" "${BACKUP}"

echo "Fixing precision issues in ${FILE}..."

# 1. Fix float literals (0.0f -> 0.0, 1.0f -> 1.0, etc.)
sed -i 's/\([0-9]\+\)\.\([0-9]*\)f\>/\1.\2/g' "${FILE}"

# 2. Fix float type declarations to double
# Be careful not to change function names or comments
sed -i 's/\bfloat\b\([^a-zA-Z_]\)/double\1/g' "${FILE}"

# 3. Fix specific patterns
sed -i 's/static const double CYMATIC_FREQS/static const double CYMATIC_FREQS/g' "${FILE}"

echo "Done! Changes applied to ${FILE}"
echo "Backup saved as ${BACKUP}"
echo ""
echo "Please review the changes with: diff ${BACKUP} ${FILE}"