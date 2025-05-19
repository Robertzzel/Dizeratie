#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H


typedef struct {
    char *method;
    char *url;
    char *body;
} http_request_t;

http_request_t http_request_parse(const char *raw_request) {
    http_request_t req = {0};

    const char *method_end = strchr(raw_request, ' ');
    if (!method_end) return req;

    size_t method_len = method_end - raw_request;
    req.method = strndup(raw_request, method_len);

    const char *url_start = method_end + 1;
    const char *url_end = strchr(url_start, ' ');
    if (!url_end) return req;

    size_t url_len = url_end - url_start;
    req.url = strndup(url_start, url_len);

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
            req.body = strndup(body_start, content_length);
        }
    }

    return req;
}

void free_http_request(http_request_t *req) {
    free(req->method);
    free(req->url);
    free(req->body);
    req->method = req->url = req->body = NULL;
}

int extract_form_field_to_buffer(const char *body, const char *key, char *buffer, size_t buffer_size) {
    if (!body || !key || !buffer || buffer_size == 0) return 0;

    size_t key_len = strlen(key);
    const char *pos = body;

    while ((pos = strstr(pos, key)) != NULL) {
        if ((pos == body || *(pos - 1) == '&') && pos[key_len] == '=') {
            pos += key_len + 1;  // Skip past "key="

            const char *end = strchr(pos, '&');
            size_t value_len = end ? (size_t)(end - pos) : strlen(pos);

            if (value_len >= buffer_size) {
                // Not enough space in buffer (leave it unmodified)
                return 0;
            }

            memcpy(buffer, pos, value_len);
            buffer[value_len] = '\0';
            return 1;
        }

        pos += key_len;
    }

    return 0;  // Key not found
}

int http_request_get_url_param(const char* url, const char* param, size_t value_max_size, char* value) {
    const char* start = strstr(url, param);
    if (!start) {
        return -1;
    }
    start += strlen(param) + 1;
    const char* end = strchr(start, '&');
    if (!end) {
        end = url + strlen(url);
    }
    size_t len = end - start;
    if (len >= value_max_size) {
        return -1;
    }
    strncpy(value, start, len);
    value[len] = '\0';
    return 0;
}

#endif // HTTP_REQUEST_H