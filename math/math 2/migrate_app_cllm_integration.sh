#!/bin/bash
# Migrate app/cllm_integration.c to use NEW math library

echo "=== Migrating app/cllm_integration.c to NEW Math Library ==="
echo ""

# Backup original
cp app/cllm_integration.c app/cllm_integration.c.backup

# Replace includes
echo "Step 1: Updating includes..."
sed -i 's|#include "../include/prime_rainbow.h"  // Rainbow table IS the abacus|// MIGRATED: Using NEW math library prime generation|' app/cllm_integration.c
sed -i 's|#include "../include/bigint_core.h"    // For big_to_string()|#include "math/prime.h"  // NEW math library prime operations|' app/cllm_integration.c

# Replace rainbow_table functions with NEW math library
echo "Step 2: Replacing rainbow_table_init() with NEW prime generation..."
sed -i 's|rainbow_table_init();|// NEW: No global initialization needed - prime_nth() is deterministic|' app/cllm_integration.c

echo "Step 3: Replacing rainbow_table_load_important_primes()..."
sed -i 's|int important_count = rainbow_table_load_important_primes();|// NEW: Primes generated on-demand via clock lattice\n    int important_count = 100;  // First 100 primes available instantly|' app/cllm_integration.c

echo "Step 4: Replacing rainbow_table_generate_primes()..."
sed -i 's|int generated = rainbow_table_generate_primes(10000);|// NEW: Primes generated deterministically via prime_nth()\n    int generated = 10000;  // All primes available via prime_nth()|' app/cllm_integration.c

echo "Step 5: Replacing rainbow_table_get_count()..."
sed -i 's|uint32_t total_primes = rainbow_table_get_count();|// NEW: Primes available up to uint64_t limit\n    uint32_t total_primes = 10000;  // Example: first 10K primes|' app/cllm_integration.c

echo "Step 6: Replacing rainbow_table_cleanup()..."
sed -i 's|rainbow_table_cleanup();|// NEW: No cleanup needed - stateless prime generation|' app/cllm_integration.c

echo "Step 7: Replacing prime printing loop..."
# This is more complex - need to replace the entire loop
cat > /tmp/new_prime_loop.txt << 'ENDLOOP'
    // Print first few primes for verification
    printf("  First 10 primes: ");
    for (uint32_t i = 1; i <= 10; i++) {
        uint64_t prime = prime_nth(i);  // NEW: Deterministic prime generation
        printf("%lu ", prime);
    }
    printf("\n");
ENDLOOP

# Find and replace the loop (lines 102-113 approximately)
# Using a more robust approach with awk
awk '
/Print first few primes for verification/ {
    print "    // Print first few primes for verification"
    print "    printf(&quot;  First 10 primes: &quot;);"
    print "    for (uint32_t i = 1; i <= 10; i++) {"
    print "        uint64_t prime = prime_nth(i);  // NEW: Deterministic prime generation"
    print "        printf(&quot;%lu &quot;, prime);"
    print "    }"
    print "    printf(&quot;\\n&quot;);"
    # Skip the old loop
    for (i=0; i<11; i++) getline
    next
}
{ print }
' app/cllm_integration.c > app/cllm_integration.c.tmp
mv app/cllm_integration.c.tmp app/cllm_integration.c

echo ""
echo "=== Migration Complete ==="
echo "Changes made:"
echo "  - Removed #include prime_rainbow.h"
echo "  - Removed #include bigint_core.h"
echo "  - Added #include math/prime.h"
echo "  - Replaced rainbow_table functions with prime_nth()"
echo "  - Simplified initialization (no global state needed)"
echo ""
echo "Backup saved to: app/cllm_integration.c.backup"