#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

int http_send_json_response(int client_sock, char* json);
int http_send_html_response(int client_sock, char* html);
int http_send_not_found_response(int client_sock);
int http_send_ok_response(int client_sock);
int http_send_error_response(int client_sock);
int http_send_bad_request_response(int client_sock);
int http_send_conflict_response(int client_sock);


#endif // HTTP_RESPONSE_H