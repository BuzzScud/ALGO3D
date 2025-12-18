# Tools Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../cllm/include/ai -I../algorithms/include -I../math/include
LDFLAGS = -L.. -L../algorithms -lcrystalline -lcllm -lalgorithms -lm -Wl,-rpath,'$$ORIGIN/..'

TOOLS = web_scraper train_model cllm_crawler

all: $(TOOLS)

web_scraper: web_scraper.c
	@echo "Building web_scraper..."
	$(CC) $(CFLAGS) -o $@ $< -L.. -lcrawler -lcllm -lcrystalline -lcurl -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ web_scraper built"

train_model: train_model.c
	@echo "Building train_model..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ train_model built"

cllm_crawler: cllm_crawler.c
	@echo "Building cllm_crawler..."
	$(CC) $(CFLAGS) -o $@ $< -L.. -lcrawler -lcllm -lcrystalline -lcurl -lpthread -Wl,-rpath,'$$ORIGIN/..'
	@echo "✓ cllm_crawler built"

clean:
	@echo "Cleaning tools..."
	rm -f $(TOOLS)
	@echo "✓ Tools cleaned"

.PHONY: all clean
test_train_tiny: test_train_tiny.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)


# Bitcoin recovery tool
bitcoin-recovery: bitcoin-recovery.c
	$(CC) $(CFLAGS) -I../lib/recovery_core/include -I../lib/recovery_crypto/include -I../lib/recovery_network/include \
		-o bitcoin-recovery bitcoin-recovery.c \
		-L../lib/recovery_core -L../lib/recovery_crypto -L../lib/recovery_network \
		-lrecovery_core -lrecovery_crypto -lrecovery_network -lssl -lcrypto -lcurl -lm

.PHONY: bitcoin-recovery

# Bitcoin recovery tool
bitcoin-recovery: bitcoin-recovery.c
	$(CC) $(CFLAGS) -I../lib/recovery_core/include -I../lib/recovery_crypto/include -I../lib/recovery_network/include \
		-o bitcoin-recovery bitcoin-recovery.c \
		-L../lib/recovery_core -L../lib/recovery_crypto -L../lib/recovery_network \
		-lrecovery_core -lrecovery_crypto -lrecovery_network -lssl -lcrypto -lcurl -lm

.PHONY: bitcoin-recovery
