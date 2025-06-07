#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "allocator.h"

typedef struct {
    char *method;
    char *url;
    char *body;
} http_request_t;

http_request_t* http_request_parse(allocator_t* allocator, const char *raw_request) {
    printf("Remaining memory on entry: %zu\n", allocator_available(allocator));
    http_request_t* req = allocator_alloc_type(allocator, http_request_t, 1);
    if (!req) return NULL;

    const char *method_end = strchr(raw_request, ' ');
    if (!method_end) {
        ESP_LOGI("HTTPRequest", "Cannot find method end in request");
        allocator_free(allocator);
        return NULL;
    }

    size_t method_len = method_end - raw_request;
    req->method = allocator_strndup(allocator, raw_request, method_len);
    if (!req->method){
        ESP_LOGI("HTTPRequest", "Cannot allocate memory for method");
        allocator_free(allocator);
        return NULL;
    } 

    const char *url_start = method_end + 1;
    const char *url_end = strchr(url_start, ' ');
    if (!url_end){
        ESP_LOGI("HTTPRequest", "Cannot find URL end in request");
        allocator_free_n(allocator, 2);
        return NULL;
    }

    size_t url_len = url_end - url_start;
    req->url = allocator_strndup(allocator, url_start, url_len);
    if (!req->url) {
        ESP_LOGI("HTTPRequest", "Cannot allocate memory for URL");
        allocator_free_n(allocator, 2);
        req->method = NULL;
        return req;
    }

    // Find start of headers
    const char *headers_start = url_end + 1;
    const char *body_start = strstr(headers_start, "\r\n\r\n");
    int content_length = 0;

    if (body_start) {
        body_start += 4;  // skip "\r\n\r\n"

        // Look for Content-Length header
        const char *cl = strcasestr(headers_start, "Content-Length:");
        if (cl) {
            cl += strlen("Content-Length:");
            while (*cl && isspace((unsigned char)*cl)) cl++;
            content_length = atoi(cl);
        }

        if (content_length > 0) {
            req->body = allocator_strndup(allocator, body_start, content_length);
            if (!req->body) {
                ESP_LOGI("HTTPRequest", "Cannot allocate memory for body");
                allocator_free_n(allocator, 3);
                return NULL;
            }
        }
    }

    return req;
}

void free_http_request(http_request_t *req) {
    if(req == NULL) {
        return;
    }
    if (req->method != NULL) {
        free(req->method);
        req->method = NULL;
    }
    if (req->url != NULL) {
        free(req->url);
        req->url = NULL;
    }
    if (req->body != NULL) {
        free(req->body);
        req->body = NULL;
    }
}

char* extract_form_field_to_buffer(allocator_t* allocator, const char *body, const char *key) {
    if (!body || !key || !allocator) return NULL;

    size_t key_len = strlen(key);
    const char *pos = body;

    while ((pos = strstr(pos, key)) != NULL) {
        if ((pos == body || *(pos - 1) == '&') && pos[key_len] == '=') {
            pos += key_len + 1;  // Skip past "key="

            const char *end = strchr(pos, '&');
            size_t value_len = end ? (size_t)(end - pos) : strlen(pos);

            return allocator_strndup(allocator, pos, value_len);
        }

        pos += key_len;
    }

    return NULL;  // Key not found
}

char* http_request_get_url_param(allocator_t* allocator, const char* url, const char* param) {
    const char* start = strstr(url, param); // find start of the parameter
    if (!start) {
        return NULL;
    }
    start += strlen(param) + 1; // move past "param="
    if (*start == '\0') {
        return NULL; // param found but it has no value
    }
    const char* end = strchr(start, '&');
    if (!end) {
        end = url + strlen(url);
    }
    size_t len = end - start;
    return allocator_strndup(allocator, start, len);
}

#endif // HTTP_REQUEST_H