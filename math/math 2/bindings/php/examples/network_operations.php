<?php
/**
 * Network Operations Example
 * 
 * Demonstrates Bitcoin network operations using the PHP Recovery extension
 */

echo "Bitcoin Network Operations Example\n";
echo "===================================\n\n";

// Create a network client (testnet)
$rpc_url = "http://localhost:18332";
$rpc_user = "bitcoinrpc";
$rpc_pass = "your_password_here";

echo "Connecting to Bitcoin testnet...\n";
$client = recovery_network_create_client($rpc_url, $rpc_user, $rpc_pass);

if (!$client) {
    die("Failed to create network client\n");
}

echo "Connected successfully\n\n";

// Get block information
$block_hash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"; // Genesis block
echo "Getting block information...\n";
$block = recovery_network_get_block($client, $block_hash);

if ($block === false) {
    echo "Failed to get block (this is expected if not connected to a real node)\n";
} else {
    echo "Block Information:\n";
    echo "  Hash: " . $block['hash'] . "\n";
    echo "  Height: " . $block['height'] . "\n";
    echo "  Timestamp: " . date('Y-m-d H:i:s', $block['timestamp']) . "\n";
    echo "  Transactions: " . $block['tx_count'] . "\n";
}

// Get transaction information
echo "\nGetting transaction information...\n";
$tx_hash = "4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"; // First Bitcoin transaction
$tx = recovery_network_get_transaction($client, $tx_hash);

if ($tx === false) {
    echo "Failed to get transaction (this is expected if not connected to a real node)\n";
} else {
    echo "Transaction Information:\n";
    echo "  TXID: " . $tx['txid'] . "\n";
    echo "  Confirmations: " . $tx['confirmations'] . "\n";
    echo "  Size: " . $tx['size'] . " bytes\n";
}

// Get address balance
echo "\nGetting address balance...\n";
$address = "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa"; // Satoshi's address
$balance = recovery_network_get_balance($client, $address);

if ($balance === false) {
    echo "Failed to get balance (this is expected if not connected to a real node)\n";
} else {
    echo "Address Balance: " . number_format($balance, 8) . " BTC\n";
}

// Send transaction (commented out for safety)
/*
echo "\nSending transaction...\n";
$raw_tx = "0100000001..."; // Your raw transaction hex
$txid = recovery_network_send_transaction($client, $raw_tx);

if ($txid === false) {
    echo "Failed to send transaction\n";
} else {
    echo "Transaction sent: $txid\n";
}
*/

// Clean up
recovery_network_destroy_client($client);

echo "\nNote: This example requires a running Bitcoin node with RPC enabled.\n";
echo "      Update the RPC credentials to connect to a real node.\n";
?>