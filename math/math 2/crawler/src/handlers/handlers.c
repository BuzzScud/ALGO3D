#include "handlers.h"
#include <stdio.h>

/**
 * Register all built-in site handlers
 */
void register_all_handlers(void) {
    printf("Registering built-in site handlers...\n");
    
    // Initialize registry
    site_handlers_init();
    
    // Register Twitter handler
    const SiteHandler* twitter = get_twitter_handler();
    if (twitter) {
        site_handlers_register(twitter);
    }
    
    // Register Britannica handler
    const SiteHandler* britannica = get_britannica_handler();
    if (britannica) {
        site_handlers_register(britannica);
    }
    
    // Register Etymonline handler
    const SiteHandler* etymonline = get_etymonline_handler();
    if (etymonline) {
        site_handlers_register(etymonline);
    }
    
    // Register Wikipedia handler
    const SiteHandler* wikipedia = get_wikipedia_handler();
    if (wikipedia) {
        site_handlers_register(wikipedia);
    }
    
    // Register Reddit handler
    const SiteHandler* reddit = get_reddit_handler();
    if (reddit) {
        site_handlers_register(reddit);
    }
    
    // Register Stack Overflow handler
    const SiteHandler* stackoverflow = get_stackoverflow_handler();
    if (stackoverflow) {
        site_handlers_register(stackoverflow);
    }
    
    // Register News handler
    const SiteHandler* news = get_news_handler();
    if (news) {
        site_handlers_register(news);
    }
    
    // Register Archive.org handler
    const SiteHandler* archive = get_archive_handler();
    if (archive) {
        site_handlers_register(archive);
    }
    
    printf("Site handlers registration complete (8 handlers registered)\n");
}
