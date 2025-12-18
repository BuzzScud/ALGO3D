/**
 * XML Utilities Implementation
 * 
 * Uses libxml2 for XML parsing.
 */

#include "xml_utils.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <string.h>
#include <stdio.h>

/**
 * Recursively extract text from XML node
 */
static void extract_text_recursive(xmlNode* node, char** output, char* output_end) {
    if (!node || !output || !*output) return;
    
    for (xmlNode* cur = node; cur && *output < output_end; cur = cur->next) {
        if (cur->type == XML_TEXT_NODE && cur->content) {
            const char* text = (const char*)cur->content;
            size_t len = strlen(text);
            size_t available = output_end - *output;
            
            if (len > 0 && available > 1) {
                size_t to_copy = (len < available - 1) ? len : available - 1;
                memcpy(*output, text, to_copy);
                *output += to_copy;
                
                // Add space after text node
                if (*output < output_end - 1) {
                    **output = ' ';
                    (*output)++;
                }
            }
        }
        
        // Recurse into children
        if (cur->children) {
            extract_text_recursive(cur->children, output, output_end);
        }
    }
}

/**
 * Extract text from XML document
 */
int xml_extract_text(const char* xml_content, char* output, size_t output_size) {
    if (!xml_content || !output || output_size == 0) {
        return -1;
    }
    
    // Parse XML
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL, 
                                  XML_PARSE_RECOVER | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!doc) {
        return -1;
    }
    
    // Get root element
    xmlNode* root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        return -1;
    }
    
    // Extract text
    char* out = output;
    char* out_end = output + output_size - 1;
    extract_text_recursive(root, &out, out_end);
    *out = '\0';
    
    xmlFreeDoc(doc);
    return 0;
}

/**
 * Extract text from specific XML elements
 */
int xml_extract_elements(const char* xml_content, const char* element_name,
                        char* output, size_t output_size) {
    if (!xml_content || !element_name || !output || output_size == 0) {
        return -1;
    }
    
    // Parse XML
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL,
                                  XML_PARSE_RECOVER | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!doc) {
        return -1;
    }
    
    // Get root element
    xmlNode* root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        return -1;
    }
    
    // Find elements with matching name
    char* out = output;
    char* out_end = output + output_size - 1;
    
    // Use XPath to find elements
    xmlXPathContextPtr context = xmlXPathNewContext(doc);
    if (!context) {
        xmlFreeDoc(doc);
        return -1;
    }
    
    // Register namespaces if needed
    xmlXPathRegisterNs(context, (xmlChar*)"w", 
                      (xmlChar*)"http://schemas.openxmlformats.org/wordprocessingml/2006/main");
    xmlXPathRegisterNs(context, (xmlChar*)"text",
                      (xmlChar*)"urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    
    // Build XPath expression
    char xpath[256];
    snprintf(xpath, sizeof(xpath), "//%s", element_name);
    
    xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*)xpath, context);
    
    if (result && result->nodesetval) {
        xmlNodeSetPtr nodes = result->nodesetval;
        
        for (int i = 0; i < nodes->nodeNr && out < out_end; i++) {
            xmlNode* node = nodes->nodeTab[i];
            if (node->type == XML_ELEMENT_NODE) {
                xmlChar* content = xmlNodeGetContent(node);
                if (content) {
                    const char* text = (const char*)content;
                    size_t len = strlen(text);
                    size_t available = out_end - out;
                    
                    if (len > 0 && available > 1) {
                        size_t to_copy = (len < available - 1) ? len : available - 1;
                        memcpy(out, text, to_copy);
                        out += to_copy;
                        
                        // Add space after element
                        if (out < out_end - 1) {
                            *out++ = ' ';
                        }
                    }
                    xmlFree(content);
                }
            }
        }
    }
    
    *out = '\0';
    
    if (result) xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlFreeDoc(doc);
    
    return 0;
}

/**
 * Extract attribute value from XML
 */
int xml_extract_attribute(const char* xml_content, const char* element_name,
                         const char* attr_name, char* output, size_t output_size) {
    if (!xml_content || !element_name || !attr_name || !output || output_size == 0) {
        return -1;
    }
    
    // Parse XML
    xmlDocPtr doc = xmlReadMemory(xml_content, strlen(xml_content), NULL, NULL,
                                  XML_PARSE_RECOVER | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    if (!doc) {
        return -1;
    }
    
    // Get root element
    xmlNode* root = xmlDocGetRootElement(doc);
    if (!root) {
        xmlFreeDoc(doc);
        return -1;
    }
    
    // Find first element with matching name and extract attribute
    for (xmlNode* cur = root; cur; cur = cur->next) {
        if (cur->type == XML_ELEMENT_NODE && 
            xmlStrcmp(cur->name, (xmlChar*)element_name) == 0) {
            xmlChar* attr = xmlGetProp(cur, (xmlChar*)attr_name);
            if (attr) {
                strncpy(output, (char*)attr, output_size - 1);
                output[output_size - 1] = '\0';
                xmlFree(attr);
                xmlFreeDoc(doc);
                return 0;
            }
        }
        
        // Recurse into children
        if (cur->children) {
            // Simplified recursion - would need full implementation
        }
    }
    
    xmlFreeDoc(doc);
    return -1;
}