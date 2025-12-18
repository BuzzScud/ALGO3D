<?php
echo "PHP Version: " . phpversion() . "\n";
echo "Loaded php.ini: " . php_ini_loaded_file() . "\n";
echo "crystalline_math loaded: " . (extension_loaded('crystalline_math') ? 'YES' : 'NO') . "\n";
echo "algorithms loaded: " . (extension_loaded('algorithms') ? 'YES' : 'NO') . "\n";
echo "\nAvailable functions:\n";
echo "prime_nth exists: " . (function_exists('prime_nth') ? 'YES' : 'NO') . "\n";
echo "prime_is_prime exists: " . (function_exists('prime_is_prime') ? 'YES' : 'NO') . "\n";
echo "is_prime exists: " . (function_exists('is_prime') ? 'YES' : 'NO') . "\n";
?>