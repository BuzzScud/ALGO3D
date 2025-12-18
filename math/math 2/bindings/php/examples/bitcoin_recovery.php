<?php
/**
 * Bitcoin Key Recovery Example
 * 
 * Demonstrates Bitcoin private key recovery from partial data
 */

// Partial Bitcoin private key (some bytes corrupted)
$partial_key = hex2bin("5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf");

echo "Bitcoin Key Recovery Example\n";
echo "=============================\n\n";

// Try to recover the key for mainnet
echo "Attempting to recover Bitcoin mainnet key...\n";
$recovered_key = recovery_crypto_recover_bitcoin_key($partial_key, 0); // 0 = MAINNET

if ($recovered_key === false) {
    echo "Failed to recover mainnet key\n";
} else {
    echo "Recovered Key: $recovered_key\n";
}

// Try testnet
echo "\nAttempting to recover Bitcoin testnet key...\n";
$recovered_key = recovery_crypto_recover_bitcoin_key($partial_key, 1); // 1 = TESTNET

if ($recovered_key === false) {
    echo "Failed to recover testnet key\n";
} else {
    echo "Recovered Key: $recovered_key\n";
}

echo "\nNote: This is a demonstration. Real key recovery requires\n";
echo "      actual partial key data and multiple samples.\n";
?>