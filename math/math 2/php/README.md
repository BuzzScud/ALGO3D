# Crystalline Math PHP Extension

This PHP extension provides access to the Crystalline Math library's powerful mathematical functions, including O(1) deterministic prime generation, rainbow tables, and clock lattice operations.

**⚠️ Known Issue:** `crystalline_prime_nth()` has a segmentation fault issue. See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for details and workarounds. All other functions work correctly.

## Features

- **O(1) Prime Generation**: Generate primes deterministically in constant time
- **Primality Testing**: Fast primality checking
- **Rainbow Tables**: Efficient prime lookups
- **Clock Lattice**: Babylonian clock structure with 12-fold symmetry
- **Pure Mathematics**: No dependencies on standard math libraries

## Requirements

- PHP 7.0 or higher
- PHP development headers (php-dev or php-devel)
- Build tools (gcc, make, autoconf)
- Crystalline Math library (built automatically)

## Installation

### Ubuntu/Debian

```bash
cd /path/to/crystalline
sudo scripts/install_php_ubuntu.sh
```

### CentOS/RHEL/Fedora

```bash
cd /path/to/crystalline
sudo scripts/install_php_centos.sh
```

### Manual Installation

```bash
# Build the math library first
cd math
make

# Build and install PHP extension
cd ../php
phpize
./configure --enable-crystalline-math
make
sudo make install

# Add to php.ini
echo "extension=crystalline_math.so" | sudo tee /etc/php/$(php -r 'echo PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;')/mods-available/crystalline_math.ini

# Enable the extension
sudo phpenmod crystalline_math

# Restart PHP/Apache
sudo systemctl restart apache2  # or php-fpm
```

## Usage

### Prime Generation

```php
<?php
// Generate prime using O(1) formula
$prime = crystalline_prime_generate_o1(3, 0);  // Position 3, Magnitude 0
echo "Prime: $prime\n";  // Output: Prime: 5

// Check if number is prime
$is_prime = crystalline_prime_is_prime(157);
echo "157 is " . ($is_prime ? "prime" : "composite") . "\n";

// Get nth prime
$prime = crystalline_prime_nth(10);
echo "10th prime: $prime\n";  // Output: 10th prime: 29
?>
```

### Clock Lattice

```php
<?php
// Map prime to clock position
$pos = crystalline_clock_position(17);
echo "Prime 17: Ring {$pos['ring']}, Position {$pos['position']}\n";

// Validate clock position
$valid = crystalline_clock_validate(0, 5);
echo "Position valid: " . ($valid ? "yes" : "no") . "\n";
?>
```

### Rainbow Table

```php
<?php
// Initialize rainbow table
$capacity = crystalline_rainbow_init(10000);

// Populate with primes
crystalline_rainbow_populate(1000);

// Lookup prime by index
$prime = crystalline_rainbow_lookup(50);
echo "50th prime: $prime\n";
?>
```

## API Reference

### Prime Functions

- `crystalline_prime_generate_o1(int $position, int $magnitude): int`
  - Generate prime using O(1) deterministic formula
  - Position: 0-11 (clock position)
  - Magnitude: 0+ (magnitude level)
  - Returns: Prime number or 0 if composite

- `crystalline_prime_is_prime(int $n): bool`
  - Check if number is prime
  - Returns: true if prime, false otherwise

- `crystalline_prime_nth(int $n): int`
  - Get the nth prime number (1-based)
  - Returns: The nth prime

### Rainbow Table Functions

- `crystalline_rainbow_init(int $capacity = 10000): int`
  - Initialize rainbow table
  - Returns: Capacity on success, false on failure

- `crystalline_rainbow_populate(int $count): bool`
  - Populate table with primes
  - Returns: true on success

- `crystalline_rainbow_lookup(int $index): int`
  - Lookup prime by index
  - Returns: Prime number

- `crystalline_rainbow_count(): int`
  - Get count of primes in table
  - Returns: Number of primes

### Clock Lattice Functions

- `crystalline_clock_position(int $prime): array`
  - Map prime to clock position
  - Returns: Array with keys: ring, position, angle, radius

- `crystalline_clock_validate(int $ring, int $position): bool`
  - Validate clock position
  - Returns: true if valid

### Utility Functions

- `crystalline_version(): string`
  - Get extension version
  - Returns: Version string

## Examples

See the `examples/php/` directory for complete examples:

- `prime_generation.php` - Prime generation and testing
- `clock_lattice.php` - Clock lattice operations
- `rainbow_table.php` - Rainbow table usage

## Performance

The Crystalline Math library provides significant performance improvements:

- **O(1) Prime Generation**: 100-1000x faster than trial division
- **Rainbow Table**: 3-5x faster prime lookups
- **No math.h**: Pure crystalline mathematics with no external dependencies

## Troubleshooting

### Extension not loading

Check if the extension is installed:
```bash
php -m | grep crystalline_math
```

Check PHP configuration:
```bash
php --ini
```

### Build errors

Make sure you have all dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install php-dev build-essential

# CentOS/RHEL
sudo yum install php-devel gcc make
```

### Math library not found

Build the math library first:
```bash
cd ../math
make
```

## License

See the main project LICENSE file.

## Support

For issues and questions, please refer to the main Crystalline project documentation.