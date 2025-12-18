# Tools Makefile for OBJECTIVE 28

CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I../include
LDFLAGS = -L../lib -lgeometric_recovery -lm -lssl -lcrypto

# All tools including new CLI tools
all: generate_ecdsa_samples generate_comprehensive_test_data run_comprehensive_tests geometric_recovery_cli train_model recover_k validate_model

# Original tools
generate_ecdsa_samples: generate_ecdsa_samples.c
	@echo "Building ECDSA sample generator..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

generate_comprehensive_test_data: generate_comprehensive_test_data.c
	@echo "Building comprehensive test data generator..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

run_comprehensive_tests: run_comprehensive_tests.c
	@echo "Building comprehensive test runner..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

geometric_recovery_cli: geometric_recovery_cli.c
	@echo "Building unified CLI tool..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

# New CLI tools (OBJECTIVE 28 completion)
train_model: train_model.c
	@echo "Building train_model CLI tool..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

recover_k: recover_k.c
	@echo "Building recover_k CLI tool..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

validate_model: validate_model.c
	@echo "Building validate_model CLI tool..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ Built successfully"

clean:
	rm -f generate_ecdsa_samples generate_comprehensive_test_data run_comprehensive_tests geometric_recovery_cli train_model recover_k validate_model
	@echo "✓ Cleaned"

.PHONY: all clean
