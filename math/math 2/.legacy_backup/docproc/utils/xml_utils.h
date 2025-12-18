#ifndef XML_UTILS_H
#define XML_UTILS_H

#include <stddef.h>

/**
 * XML Utilities for Document Processing
 * 
 * Wrapper around libxml2 for parsing XML documents.
 */

/**
 * Extract text from XML document
 * 
 * @param xml_content XML content as string
 * @param output Output buffer for extracted text
 * @param output_size Size of output buffer
 * @return 0 on success, -1 on error
 */
int xml_extract_text(const char* xml_content, char* output, size_t output_size);

/**
 * Extract text from specific XML elements
 * 
 * @param xml_content XML content as string
 * @param element_name Name of elements to extract (e.g., "w:t" for DOCX)
 * @param output Output buffer for extracted text
 * @param output_size Size of output buffer
 * @return 0 on success, -1 on error
 */
int xml_extract_elements(const char* xml_content, const char* element_name,
                        char* output, size_t output_size);

/**
 * Extract attribute value from XML
 * 
 * @param xml_content XML content as string
 * @param element_name Element name
 * @param attr_name Attribute name
 * @param output Output buffer for attribute value
 * @param output_size Size of output buffer
 * @return 0 on success, -1 on error
 */
int xml_extract_attribute(const char* xml_content, const char* element_name,
                         const char* attr_name, char* output, size_t output_size);

#endif // XML_UTILS_H