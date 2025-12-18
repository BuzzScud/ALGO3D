/**
 * URL Database Implementation
 * 
 * SQLite-based URL management with:
 * - GET parameter preservation
 * - Timestamp tracking
 * - Priority calculation
 * - Status management
 */

#include "url_database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

// Database structure
struct URLDatabase {
    sqlite3* db;
    char db_path[1024];
};

// SQL schema
static const char* CREATE_TABLE_SQL = 
    "CREATE TABLE IF NOT EXISTS urls ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  url TEXT UNIQUE NOT NULL,"
    "  domain TEXT NOT NULL,"
    "  path TEXT,"
    "  query_string TEXT,"
    "  file_type TEXT,"
    "  first_seen INTEGER NOT NULL,"
    "  last_crawled INTEGER,"
    "  crawl_count INTEGER DEFAULT 0,"
    "  priority INTEGER DEFAULT 0,"
    "  status TEXT DEFAULT 'pending',"
    "  source_url TEXT,"
    "  blocked INTEGER DEFAULT 0"
    ");";

static const char* CREATE_INDEXES_SQL[] = {
    "CREATE INDEX IF NOT EXISTS idx_url ON urls(url);",
    "CREATE INDEX IF NOT EXISTS idx_domain ON urls(domain);",
    "CREATE INDEX IF NOT EXISTS idx_status ON urls(status);",
    "CREATE INDEX IF NOT EXISTS idx_priority ON urls(priority DESC);",
    "CREATE INDEX IF NOT EXISTS idx_last_crawled ON urls(last_crawled);",
    "CREATE INDEX IF NOT EXISTS idx_blocked ON urls(blocked);",
    NULL
};

/**
 * Parse URL to extract components
 */
static void parse_url(const char* url, char* domain, char* path, char* query_string, char* file_type) {
    if (!url) return;
    
    // Initialize outputs
    if (domain) domain[0] = '\0';
    if (path) path[0] = '\0';
    if (query_string) query_string[0] = '\0';
    if (file_type) strcpy(file_type, "html");  // Default
    
    // Find protocol end
    const char* proto_end = strstr(url, "://");
    if (!proto_end) return;
    
    const char* start = proto_end + 3;
    
    // Extract domain
    const char* path_start = strchr(start, '/');
    if (path_start) {
        size_t domain_len = path_start - start;
        if (domain && domain_len < 255) {
            strncpy(domain, start, domain_len);
            domain[domain_len] = '\0';
        }
        
        // Extract path and query string
        const char* query_start = strchr(path_start, '?');
        if (query_start) {
            // Path before query
            size_t path_len = query_start - path_start;
            if (path && path_len < 2047) {
                strncpy(path, path_start, path_len);
                path[path_len] = '\0';
            }
            
            // Query string (PRESERVE IT!)
            if (query_string) {
                strncpy(query_string, query_start + 1, 1023);
                query_string[1023] = '\0';
            }
        } else {
            // No query string, just path
            if (path) {
                strncpy(path, path_start, 2047);
                path[2047] = '\0';
            }
        }
        
        // Determine file type from path
        if (path && file_type) {
            const char* ext = strrchr(path, '.');
            if (ext) {
                ext++;  // Skip the dot
                if (strcasecmp(ext, "pdf") == 0) strcpy(file_type, "pdf");
                else if (strcasecmp(ext, "txt") == 0) strcpy(file_type, "txt");
                else if (strcasecmp(ext, "doc") == 0 || strcasecmp(ext, "docx") == 0) strcpy(file_type, "doc");
                else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 || 
                         strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0) strcpy(file_type, "image");
                else if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 || 
                         strcasecmp(ext, "mkv") == 0) strcpy(file_type, "video");
                else if (strcasecmp(ext, "mp3") == 0 || strcasecmp(ext, "wav") == 0 || 
                         strcasecmp(ext, "flac") == 0) strcpy(file_type, "audio");
                else if (strcasecmp(ext, "zip") == 0 || strcasecmp(ext, "tar") == 0 || 
                         strcasecmp(ext, "gz") == 0) strcpy(file_type, "archive");
            }
        }
    } else {
        // No path, just domain
        if (domain) {
            strncpy(domain, start, 255);
            domain[255] = '\0';
        }
    }
}

/**
 * Create/open database
 */
URLDatabase* url_db_open(const char* db_path) {
    if (!db_path) return NULL;
    
    URLDatabase* db = (URLDatabase*)calloc(1, sizeof(URLDatabase));
    if (!db) return NULL;
    
    strncpy(db->db_path, db_path, sizeof(db->db_path) - 1);
    
    // Open SQLite database
    int rc = sqlite3_open(db_path, &db->db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db->db));
        free(db);
        return NULL;
    }
    
    // Create table
    char* err_msg = NULL;
    rc = sqlite3_exec(db->db, CREATE_TABLE_SQL, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db->db);
        free(db);
        return NULL;
    }
    
    // Create indexes
    for (int i = 0; CREATE_INDEXES_SQL[i] != NULL; i++) {
        rc = sqlite3_exec(db->db, CREATE_INDEXES_SQL[i], NULL, NULL, &err_msg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
    
    return db;
}

/**
 * Close database
 */
void url_db_close(URLDatabase* db) {
    if (!db) return;
    
    if (db->db) {
        sqlite3_close(db->db);
    }
    
    free(db);
}

/**
 * Add URL to database
 */
int url_db_add(URLDatabase* db, const char* url, const char* source_url) {
    if (!db || !url) return -1;
    
    // Parse URL
    char domain[256];
    char path[2048];
    char query_string[1024];
    char file_type[32];
    
    parse_url(url, domain, path, query_string, file_type);
    
    // Prepare SQL
    const char* sql = "INSERT OR IGNORE INTO urls "
                     "(url, domain, path, query_string, file_type, first_seen, source_url) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?);";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }
    
    // Bind parameters
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, domain, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, query_string, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, file_type, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 6, (int64_t)time(NULL));
    sqlite3_bind_text(stmt, 7, source_url ? source_url : "", -1, SQLITE_STATIC);
    
    // Execute
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db->db));
        return -1;
    }
    
    return 0;
}

/**
 * Remove URL from database
 */
int url_db_remove(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "DELETE FROM urls WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Block URL
 */
int url_db_block(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "UPDATE urls SET blocked = 1, status = 'blocked' WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Unblock URL
 */
int url_db_unblock(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "UPDATE urls SET blocked = 0, status = 'pending' WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Mark URL as crawled
 */
int url_db_mark_crawled(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "UPDATE urls SET status = 'crawled', last_crawled = ?, crawl_count = crawl_count + 1 WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, (int64_t)time(NULL));
    sqlite3_bind_int64(stmt, 2, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Mark URL as failed
 */
int url_db_mark_failed(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "UPDATE urls SET status = 'failed' WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Mark URL as currently being crawled
 */
int url_db_mark_crawling(URLDatabase* db, uint64_t id) {
    if (!db) return -1;
    
    const char* sql = "UPDATE urls SET status = 'crawling' WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE) ? 0 : -1;
}

/**
 * Get next URL to crawl
 */
URLEntry* url_db_get_next(URLDatabase* db) {
    if (!db) return NULL;
    
    // Get highest priority uncrawled URL
    const char* sql = "SELECT * FROM urls WHERE status = 'pending' AND blocked = 0 ORDER BY priority DESC, first_seen ASC LIMIT 1;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return NULL;
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    // Create entry
    URLEntry* entry = (URLEntry*)calloc(1, sizeof(URLEntry));
    if (!entry) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    entry->id = sqlite3_column_int64(stmt, 0);
    strncpy(entry->url, (const char*)sqlite3_column_text(stmt, 1), sizeof(entry->url) - 1);
    strncpy(entry->domain, (const char*)sqlite3_column_text(stmt, 2), sizeof(entry->domain) - 1);
    strncpy(entry->path, (const char*)sqlite3_column_text(stmt, 3), sizeof(entry->path) - 1);
    strncpy(entry->query_string, (const char*)sqlite3_column_text(stmt, 4), sizeof(entry->query_string) - 1);
    strncpy(entry->file_type, (const char*)sqlite3_column_text(stmt, 5), sizeof(entry->file_type) - 1);
    entry->first_seen = sqlite3_column_int64(stmt, 6);
    entry->last_crawled = sqlite3_column_int64(stmt, 7);
    entry->crawl_count = sqlite3_column_int(stmt, 8);
    entry->priority = sqlite3_column_int(stmt, 9);
    strncpy(entry->status, (const char*)sqlite3_column_text(stmt, 10), sizeof(entry->status) - 1);
    strncpy(entry->source_url, (const char*)sqlite3_column_text(stmt, 11), sizeof(entry->source_url) - 1);
    entry->blocked = sqlite3_column_int(stmt, 12) != 0;
    
    sqlite3_finalize(stmt);
    return entry;
}

/**
 * Query URLs
 */
URLEntry** url_db_query(URLDatabase* db, const char* filter, int* count) {
    if (!db || !count) return NULL;
    
    *count = 0;
    
    // Build SQL
    char sql[4096];
    if (filter && filter[0] != '\0') {
        snprintf(sql, sizeof(sql), "SELECT * FROM urls WHERE %s ORDER BY priority DESC, first_seen ASC;", filter);
    } else {
        snprintf(sql, sizeof(sql), "SELECT * FROM urls ORDER BY priority DESC, first_seen ASC;");
    }
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return NULL;
    
    // Count results first
    int capacity = 100;
    URLEntry** entries = (URLEntry**)calloc(capacity, sizeof(URLEntry*));
    if (!entries) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    // Fetch results
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Expand array if needed
        if (*count >= capacity) {
            capacity *= 2;
            URLEntry** new_entries = (URLEntry**)realloc(entries, capacity * sizeof(URLEntry*));
            if (!new_entries) {
                // Free existing entries
                for (int i = 0; i < *count; i++) {
                    free(entries[i]);
                }
                free(entries);
                sqlite3_finalize(stmt);
                return NULL;
            }
            entries = new_entries;
        }
        
        // Create entry
        URLEntry* entry = (URLEntry*)calloc(1, sizeof(URLEntry));
        if (!entry) continue;
        
        entry->id = sqlite3_column_int64(stmt, 0);
        strncpy(entry->url, (const char*)sqlite3_column_text(stmt, 1), sizeof(entry->url) - 1);
        strncpy(entry->domain, (const char*)sqlite3_column_text(stmt, 2), sizeof(entry->domain) - 1);
        strncpy(entry->path, (const char*)sqlite3_column_text(stmt, 3), sizeof(entry->path) - 1);
        strncpy(entry->query_string, (const char*)sqlite3_column_text(stmt, 4), sizeof(entry->query_string) - 1);
        strncpy(entry->file_type, (const char*)sqlite3_column_text(stmt, 5), sizeof(entry->file_type) - 1);
        entry->first_seen = sqlite3_column_int64(stmt, 6);
        entry->last_crawled = sqlite3_column_int64(stmt, 7);
        entry->crawl_count = sqlite3_column_int(stmt, 8);
        entry->priority = sqlite3_column_int(stmt, 9);
        strncpy(entry->status, (const char*)sqlite3_column_text(stmt, 10), sizeof(entry->status) - 1);
        strncpy(entry->source_url, (const char*)sqlite3_column_text(stmt, 11), sizeof(entry->source_url) - 1);
        entry->blocked = sqlite3_column_int(stmt, 12) != 0;
        
        entries[*count] = entry;
        (*count)++;
    }
    
    sqlite3_finalize(stmt);
    return entries;
}

/**
 * Get URL by ID
 */
URLEntry* url_db_get_by_id(URLDatabase* db, uint64_t id) {
    if (!db) return NULL;
    
    const char* sql = "SELECT * FROM urls WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return NULL;
    
    sqlite3_bind_int64(stmt, 1, id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    // Create entry
    URLEntry* entry = (URLEntry*)calloc(1, sizeof(URLEntry));
    if (!entry) {
        sqlite3_finalize(stmt);
        return NULL;
    }
    
    entry->id = sqlite3_column_int64(stmt, 0);
    strncpy(entry->url, (const char*)sqlite3_column_text(stmt, 1), sizeof(entry->url) - 1);
    strncpy(entry->domain, (const char*)sqlite3_column_text(stmt, 2), sizeof(entry->domain) - 1);
    strncpy(entry->path, (const char*)sqlite3_column_text(stmt, 3), sizeof(entry->path) - 1);
    strncpy(entry->query_string, (const char*)sqlite3_column_text(stmt, 4), sizeof(entry->query_string) - 1);
    strncpy(entry->file_type, (const char*)sqlite3_column_text(stmt, 5), sizeof(entry->file_type) - 1);
    entry->first_seen = sqlite3_column_int64(stmt, 6);
    entry->last_crawled = sqlite3_column_int64(stmt, 7);
    entry->crawl_count = sqlite3_column_int(stmt, 8);
    entry->priority = sqlite3_column_int(stmt, 9);
    strncpy(entry->status, (const char*)sqlite3_column_text(stmt, 10), sizeof(entry->status) - 1);
    strncpy(entry->source_url, (const char*)sqlite3_column_text(stmt, 11), sizeof(entry->source_url) - 1);
    entry->blocked = sqlite3_column_int(stmt, 12) != 0;
    
    sqlite3_finalize(stmt);
    return entry;
}

/**
 * Check if URL exists
 */
bool url_db_exists(URLDatabase* db, const char* url) {
    if (!db || !url) return false;
    
    const char* sql = "SELECT COUNT(*) FROM urls WHERE url = ?;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count > 0;
}

/**
 * Get total URL count
 */
int url_db_count_total(URLDatabase* db) {
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM urls;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

/**
 * Get pending URL count
 */
int url_db_count_pending(URLDatabase* db) {
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM urls WHERE status = 'pending' AND blocked = 0;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

/**
 * Get crawled URL count
 */
int url_db_count_crawled(URLDatabase* db) {
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM urls WHERE status = 'crawled';";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

/**
 * Get blocked URL count
 */
int url_db_count_blocked(URLDatabase* db) {
    if (!db) return 0;
    
    const char* sql = "SELECT COUNT(*) FROM urls WHERE blocked = 1;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

/**
 * Export URLs to file
 */
int url_db_export(URLDatabase* db, const char* file_path) {
    if (!db || !file_path) return -1;
    
    FILE* fp = fopen(file_path, "w");
    if (!fp) return -1;
    
    int count = 0;
    URLEntry** entries = url_db_query(db, NULL, &count);
    if (!entries) {
        fclose(fp);
        return -1;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s\n", entries[i]->url);
    }
    
    url_db_free_entries(entries, count);
    fclose(fp);
    
    return 0;
}

/**
 * Import URLs from file
 */
int url_db_import(URLDatabase* db, const char* file_path) {
    if (!db || !file_path) return -1;
    
    FILE* fp = fopen(file_path, "r");
    if (!fp) return -1;
    
    char line[4096];
    int imported = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Skip empty lines
        if (line[0] == '\0') continue;
        
        // Add URL
        if (url_db_add(db, line, NULL) == 0) {
            imported++;
        }
    }
    
    fclose(fp);
    return imported;
}

/**
 * Free URL entry
 */
void url_db_free_entry(URLEntry* entry) {
    if (entry) {
        free(entry);
    }
}

/**
 * Free URL entry array
 */
void url_db_free_entries(URLEntry** entries, int count) {
    if (!entries) return;
    
    for (int i = 0; i < count; i++) {
        if (entries[i]) {
            free(entries[i]);
        }
    }
    
    free(entries);
}

/**
 * Reset all URLs to pending status (for recrawling)
 */
int url_db_reset_all_to_pending(URLDatabase* db) {
    if (!db || !db->db) return -1;
    
    const char* sql = "UPDATE urls SET status = 'pending', last_crawled = NULL, crawl_count = 0 WHERE status != 'pending';";
    
    char* err_msg = NULL;
    int rc = sqlite3_exec(db->db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "ERROR: Failed to reset URLs: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    int changes = sqlite3_changes(db->db);
    printf("Reset %d URLs to pending status\n", changes);
    
    return changes;
}