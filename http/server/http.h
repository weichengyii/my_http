//
// Created by wcy on 3/3/23.
//

#ifndef CASUAL_HTTP_H
#define CASUAL_HTTP_H

#define PORT 8080
#define BUFFER_SIZE 1024



typedef enum {
    HTTP_GET,
    HTTP_POST,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_HEAD,
    HTTP_OPTIONS,
    HTTP_PATCH,
    HTTP_TRACE,
    HTTP_CONNECT,
    HTTP_UNKNOWN
} http_method;

typedef enum {
    HTTP_text_html,
    HTTP_text_css,
    HTTP_text_javascript,
} http_content_type;

void http_server_start();

void http_server();

void *http_request(void *arg);

void http_response_file(int client_socket, char *path, http_content_type file_type);

void find_file_name(char *file_dir);

#endif //CASUAL_HTTP_H
