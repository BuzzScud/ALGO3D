# Build and Test Guide - 88D Integration

This guide provides step-by-step instructions for building the C library, testing the PHP API, testing from JavaScript, and verifying 88D threading.

## Prerequisites

- GCC compiler
- PHP 7.4+ with FFI extension (optional, for direct C calls)
- Make
- pthread library
- SQLite3 development libraries

## Step 1: Build the C Library

### 1.1 Build Dependencies

First, build the math library (foundation):

```bash
cd "math/math 2/math"
make clean
make -j4
```

If you encounter compilation errors (like unused variables), you may need to adjust the Makefile to remove `-Werror` temporarily:

```bash
# Edit math/Makefile and change:
# CFLAGS = -Wall -Wextra -Werror -O2 ...
# to:
# CFLAGS = -Wall -Wextra -O2 ...
```

### 1.2 Build Algorithms Library

```bash
cd "math/math 2/algorithms"
make clean
make -j4
```

### 1.3 Build CLLM Library

```bash
cd "math/math 2/cllm"
make clean
make -j4
```

This will create:
- `libcllm.a` (static library)
- `libcllm.so` (shared library)

### 1.4 Verify Build

```bash
ls -lh "math/math 2/cllm/libcllm.so"
```

You should see the shared library file.

## Step 2: Test PHP API Directly

### 2.1 Run PHP Test Script

```bash
cd /Users/christiantavarez/ALGO3D
php test_price_projection.php
```

Expected output:
```
=== Price Projection API Test ===

Test Configuration:
  - Historical prices: 6 points
  - Depth prime: 127
  - Base price: 105.0
  - Steps: 20
  - Projection count: 3
  - Omega Hz: 432.0

Using fallback method (PHP math)...

=== Test Results ===
Method used: fallback
✓ SUCCESS

Projection Lines: 3
Steps per line: 20

Line 1 (first 5 points): 105.00, 105.01, 105.02, ...
```

**Note:** If FFI or exec methods are available, they will be used instead of fallback.

### 2.2 Test via HTTP (if web server is running)

```bash
curl -X POST http://localhost:8080/api/price_projection.php \
  -H "Content-Type: application/json" \
  -d '{
    "historical_prices": [100, 101, 102, 103, 104, 105],
    "depth_prime": 127,
    "base": 105.0,
    "steps": 20,
    "projection_count": 3,
    "omega_hz": 432.0
  }'
```

## Step 3: Test from JavaScript in Browser

### 3.1 Start Web Server

```bash
cd /Users/christiantavarez/ALGO3D
php -S localhost:8080
```

### 3.2 Open Test Page

Open in browser:
```
http://localhost:8080/test_price_projection.html
```

### 3.3 Test in Main Application

1. Navigate to the Price Projection tab in the main application
2. Select a symbol (e.g., SPY, QQQ)
3. Click "Generate Projections"
4. Check browser console for logs showing:
   - "Calling PHP backend for price projections (CLLM with 88D threading)..."
   - "✓ Projections computed via [method]"

## Step 4: Verify 88D Threading is Active

### 4.1 Compile Threading Test

```bash
cd /Users/christiantavarez/ALGO3D
gcc -o test_88d_threading test_88d_threading.c \
  -I"math/math 2/cllm/include" \
  -L"math/math 2/cllm" \
  -lcllm \
  -L"math/math 2/algorithms" \
  -lalgorithms \
  -L"math/math 2/math/lib" \
  -lcrystallinemath \
  -lpthread -lm
```

### 4.2 Run Threading Test

```bash
export LD_LIBRARY_PATH="math/math 2/cllm:math/math 2/algorithms:math/math 2/math/lib:$LD_LIBRARY_PATH"
./test_88d_threading
```

Expected output:
```
=== 88D Threading Verification Test ===

Test 1: Creating CLLM model...
  ✓ CLLM model created

Test 2: Verifying 88D threading...
  ✓ Thread pool exists

Test 3: Verifying thread count...
  ✓ Thread count correct: 96 threads (8 layers × 12 threads)

Test 4: Verifying thread structure...
  ✓ All 96 threads are valid

Test 5: Verifying token assignments...
  ✓ Token assignments exist
  ✓ Vocab size: 1000

Test 6: Testing price projection with 88D threading...
  ✓ Price projection successful
  ✓ Generated 3 projection lines
  ✓ Steps per line: 20

  Sample projection (first line, first 5 points):
    Step 0: 105.00
    Step 1: 105.01
    ...

=== All Tests Passed ===
✓ 88D threading is active and working correctly
```

## Troubleshooting

### Build Issues

1. **Unused variable errors:**
   - Remove `-Werror` from Makefiles temporarily
   - Or fix the unused variables in source code

2. **Missing dependencies:**
   - Install development packages: `sudo apt-get install build-essential libpthread-stubs0-dev`

3. **Library not found:**
   - Set `LD_LIBRARY_PATH` to include library directories
   - Or install libraries to system paths

### PHP API Issues

1. **FFI not available:**
   - Install PHP FFI extension: `sudo apt-get install php-ffi`
   - Or use exec/fallback methods

2. **Executable not found:**
   - Build the C executable (if using exec method)
   - Or use fallback method

### JavaScript Issues

1. **CORS errors:**
   - Ensure PHP headers are set correctly in `api/price_projection.php`
   - Check browser console for specific errors

2. **API not responding:**
   - Check PHP error logs
   - Verify web server is running
   - Test PHP API directly first

## Verification Checklist

- [ ] C library builds successfully (`libcllm.so` exists)
- [ ] PHP test script runs and returns results
- [ ] JavaScript test page works in browser
- [ ] Main application Price Projection tab works
- [ ] 88D threading test passes
- [ ] Browser console shows "CLLM" or "exec" method (not just "fallback")

## Next Steps

Once all tests pass:

1. **Optimize C implementation:**
   - Use actual CLLM forward pass for projections
   - Implement proper CrystallineAbacus integration
   - Add more parallel computation

2. **Remove fallbacks:**
   - Once C implementation is stable, remove PHP/JS fallbacks
   - Make CLLM with 88D threading the only method

3. **Performance tuning:**
   - Profile the C code
   - Optimize thread distribution
   - Add caching for repeated projections

## Support

For issues or questions:
- Check build logs for specific errors
- Review PHP error logs
- Check browser console for JavaScript errors
- Verify all dependencies are installed

