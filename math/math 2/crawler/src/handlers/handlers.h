#ifndef HANDLERS_H
#define HANDLERS_H

#include "../site_handlers.h"

/**
 * Get handler instances
 */
const SiteHandler* get_twitter_handler(void);
const SiteHandler* get_britannica_handler(void);
const SiteHandler* get_etymonline_handler(void);
const SiteHandler* get_wikipedia_handler(void);
const SiteHandler* get_reddit_handler(void);
const SiteHandler* get_stackoverflow_handler(void);
const SiteHandler* get_news_handler(void);
const SiteHandler* get_archive_handler(void);

/**
 * Register all built-in handlers
 */
void register_all_handlers(void);

#endif // HANDLERS_H
