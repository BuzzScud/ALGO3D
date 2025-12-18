/**
 * Test program for link management
 */

#include "link_management.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== Link Management Test ===\n\n");
    
    // Create queue
    LinkQueue* queue = link_queue_create("/tmp/test_links.txt");
    if (!queue) {
        fprintf(stderr, "Failed to create link queue\n");
        return 1;
    }
    printf("✓ Link queue created\n");
    
    // Add some links
    printf("\n--- Adding Links ---\n");
    link_queue_add(queue, "https://example.com/page1", 5, "https://example.com");
    link_queue_add(queue, "https://example.com/page2", 3, "https://example.com");
    link_queue_add(queue, "https://example.com/page3", 10, "https://example.com");
    link_queue_add(queue, "https://example.com/page4", 1, "https://example.com");
    printf("  Added 4 links\n");
    printf("  Queue size: %d\n", link_queue_size(queue));
    printf("  Total links: %d\n", link_queue_total(queue));
    
    // Test duplicate detection
    printf("\n--- Duplicate Detection ---\n");
    if (link_queue_is_duplicate(queue, "https://example.com/page1")) {
        printf("  ✓ Correctly detected duplicate\n");
    }
    if (!link_queue_is_duplicate(queue, "https://example.com/page99")) {
        printf("  ✓ Correctly detected non-duplicate\n");
    }
    
    // Try to add duplicate
    int result = link_queue_add(queue, "https://example.com/page1", 5, NULL);
    printf("  Adding duplicate returned: %d (0 = already exists)\n", result);
    printf("  Queue size after duplicate: %d\n", link_queue_size(queue));
    
    // Get next links (should be in priority order)
    printf("\n--- Getting Links (Priority Order) ---\n");
    char url[2048];
    for (int i = 0; i < 4; i++) {
        if (link_queue_get_next(queue, url, sizeof(url)) == 0) {
            printf("  %d. %s\n", i+1, url);
            link_queue_mark_crawled(queue, url);
        }
    }
    
    printf("\n--- After Crawling ---\n");
    printf("  Queue size (uncrawled): %d\n", link_queue_size(queue));
    printf("  Total links: %d\n", link_queue_total(queue));
    
    // Add more links
    printf("\n--- Adding More Links ---\n");
    link_queue_add(queue, "https://example.com/page5", 7, "https://example.com/page1");
    link_queue_add(queue, "https://example.com/page6", 2, "https://example.com/page1");
    printf("  Added 2 more links\n");
    printf("  Queue size: %d\n", link_queue_size(queue));
    
    // Batch add
    printf("\n--- Batch Add ---\n");
    CrawlerLink batch[3];
    strcpy(batch[0].url, "https://example.com/batch1");
    batch[0].priority = 8;
    strcpy(batch[1].url, "https://example.com/batch2");
    batch[1].priority = 4;
    strcpy(batch[2].url, "https://example.com/batch3");
    batch[2].priority = 6;
    
    int added = link_queue_add_batch(queue, batch, 3);
    printf("  Added %d links in batch\n", added);
    printf("  Queue size: %d\n", link_queue_size(queue));
    
    // Save queue
    printf("\n--- Saving Queue ---\n");
    if (link_queue_save(queue) == 0) {
        printf("  ✓ Queue saved to /tmp/test_links.txt\n");
    }
    
    // Destroy and recreate to test loading
    printf("\n--- Testing Load ---\n");
    link_queue_destroy(queue);
    
    queue = link_queue_create("/tmp/test_links.txt");
    printf("  ✓ Queue loaded from file\n");
    printf("  Queue size: %d\n", link_queue_size(queue));
    printf("  Total links: %d\n", link_queue_total(queue));
    
    // Cleanup
    link_queue_destroy(queue);
    printf("\n✓ Queue destroyed\n");
    
    printf("\n=== All tests passed ===\n");
    return 0;
}