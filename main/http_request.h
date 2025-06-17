#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "allocator.h"
#include "esp_log.h"
#include <string.h>
#include <ctype.h>

typedef struct {
    char *method;
    char *url;
    char *body;
} http_request_t;

http_request_t* http_request_parse(allocator_t* allocator, const char *raw_request);

void free_http_request(http_request_t *req);

char* extract_form_field_to_buffer(allocator_t* allocator, const char *body, const char *key);

char* http_request_get_url_param(allocator_t* allocator, const char* url, const char* param);

#endif // HTTP_REQUEST_H