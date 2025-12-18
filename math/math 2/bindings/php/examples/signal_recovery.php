<?php
/**
 * Signal Recovery Example
 * 
 * Demonstrates audio and signal processing using the PHP Recovery extension
 */

echo "Signal Recovery Example\n";
echo "=======================\n\n";

// Generate a simple sine wave (clean signal)
$sample_rate = 44100;
$duration = 1.0; // 1 second
$frequency = 440.0; // A4 note
$sample_count = (int)($sample_rate * $duration);

$clean_signal = '';
for ($i = 0; $i < $sample_count; $i++) {
    $t = $i / $sample_rate;
    $sample = sin(2 * M_PI * $frequency * $t);
    $clean_signal .= pack('f', $sample);
}

echo "Generated clean sine wave: $frequency Hz, $sample_rate Hz sample rate\n";

// Add noise to simulate corruption
$noisy_signal = '';
for ($i = 0; $i < $sample_count; $i++) {
    $clean_sample = unpack('f', substr($clean_signal, $i * 4, 4))[1];
    $noise = (mt_rand() / mt_getrandmax() - 0.5) * 0.2; // 20% noise
    $noisy_sample = $clean_sample + $noise;
    $noisy_signal .= pack('f', $noisy_sample);
}

echo "Added noise to signal\n";

// Calculate SNR before recovery
$snr_before = recovery_signal_get_snr($clean_signal, $noisy_signal);
echo "SNR before recovery: " . number_format($snr_before, 2) . " dB\n";

// Recover the audio signal
echo "\nRecovering audio signal...\n";
$recovered_signal = recovery_signal_recover_audio($noisy_signal, $sample_rate, 1); // mono

if ($recovered_signal === false) {
    die("Failed to recover audio signal\n");
}

echo "Audio recovery complete\n";

// Calculate SNR after recovery
$snr_after = recovery_signal_get_snr($clean_signal, $recovered_signal);
echo "SNR after recovery: " . number_format($snr_after, 2) . " dB\n";
echo "Improvement: " . number_format($snr_after - $snr_before, 2) . " dB\n";

// Denoise the signal
echo "\nDenoising signal...\n";
$denoised_signal = recovery_signal_denoise($noisy_signal, 0.1);

if ($denoised_signal === false) {
    die("Failed to denoise signal\n");
}

$snr_denoised = recovery_signal_get_snr($clean_signal, $denoised_signal);
echo "SNR after denoising: " . number_format($snr_denoised, 2) . " dB\n";

// Resample the signal
echo "\nResampling signal from 44100 Hz to 48000 Hz...\n";
$resampled_signal = recovery_signal_resample($clean_signal, 44100, 48000);

if ($resampled_signal === false) {
    die("Failed to resample signal\n");
}

$new_sample_count = strlen($resampled_signal) / 4;
echo "Resampled signal: $new_sample_count samples at 48000 Hz\n";

echo "\nSignal processing complete!\n";
?>