#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

typedef struct {
    char method[8];
    char url[256];
} http_request_t;

http_request_t http_request_parse(const char* request) {
    http_request_t req;
    sscanf(request, "%s %s", req.method, req.url);
    return req;
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