//
// Created by wcy on 3/3/23.
//

#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include "http.h"

static const char html_file_dir[] = "/home/wcy/Program/C/Casual/http/html";
#define MAX_FILE_LIST_SIZE 1024
static char *file_list[MAX_FILE_LIST_SIZE];
int file_list_size = 0;

void http_server_start() {
    find_file_name(html_file_dir);
    http_server();
}

void http_server() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t address_size = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int));
    if (server_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);


    while (true) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_address, &address_size);
        if (client_socket == -1) {
            perror("accept");
            break;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, http_request, &client_socket) != 0) {
            perror("pthread_create");
            break;
        }
    }

    close(server_socket);
}

void *http_request(void *arg) {
    int client_socket = *(int *) arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read <= 0) {
        return NULL;
    }

//    printf("buffer: %s\n", buffer);
    char *method, *path;
    method = strtok(buffer, " ");
    path = strtok(NULL, " ");

    char file_name[100];
    if (strcmp(method, "GET") == 0) {



        if (path[1] != '\0') {
            for (int i = 0; i < file_list_size; ++i) {
                if (strcmp(&path[1], file_list[i]) == 0) {
                    strcpy(file_name, file_list[i]);
                }
            }
        }


        if (strcmp(path, "/") == 0) {
            memset(file_name, 0, sizeof(file_name));
            strcpy(file_name, "index.html");
        } else if (strcmp(path, "/login") == 0) {
            memset(file_name, 0, sizeof(file_name));
            strcpy(file_name, "login.html");
        }

        if (file_name[0] == '\0') {
            strcpy(file_name, "404.html");
        }

        if (strstr(file_name, ".js") != NULL) {
            http_response_file(client_socket, file_name, HTTP_text_javascript);
        } else if (strstr(file_name, ".css") != NULL) {
            http_response_file(client_socket, file_name, HTTP_text_css);
        } else {
            http_response_file(client_socket, file_name, HTTP_text_html);
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(path, "/login") == 0) {
            http_response_file(client_socket, "index.html", HTTP_text_html);
        } else {
            http_response_file(client_socket, "index.html", HTTP_text_html);
        }
    }


    close(client_socket);
    return NULL;
}

void http_response_file(int client_socket, char *path, http_content_type file_type) {
    char file_path[100];
    strcpy(file_path, html_file_dir);
    if (path[0] != '/') {
        strcat(file_path, "/");
    }

    strcat(file_path, path);
    FILE *fp = fopen(file_path, "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char response_header[100];
    char response_body[BUFFER_SIZE];
    switch (file_type) {
        case HTTP_text_html:
            strcpy(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
            break;
        case HTTP_text_css:
            strcpy(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\n\r\n");
            break;
        case HTTP_text_javascript:
            strcpy(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/javascript\r\n\r\n");
            break;
        default:
            strcpy(response_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
            break;
    }

    send(client_socket, response_header, strlen(response_header), 0);
    while (1) {
        int n = fread(response_body, 1, BUFFER_SIZE, fp);

        send(client_socket, response_body, n, 0);
        if (n < BUFFER_SIZE) {
            break;
        }
    }
}

void find_file_name(char *file_dir) {
    if (file_dir == NULL) {
        return;
    }
    char file_dir_[256];
    strcpy(file_dir_, file_dir);
    if (file_dir_[strlen(file_dir_) - 1] == '/') {
        file_dir_[strlen(file_dir_) - 1] = '\0';
    }

    DIR *dir;
    struct dirent *ent;
    char stack[64][256];
    int i = 0, j = 1;

    int len = strlen(file_dir_);

    strcpy(stack[i], file_dir_);

    while (i < j) {
        dir = opendir(stack[i]);
        if (dir == NULL) {
            perror("opendir");
        }
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR) {
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                    continue;
                } else {
                    strcpy(stack[j], stack[i]);
                    strcat(stack[j], "/");
                    strcat(stack[j], ent->d_name);
                    j++;
                }
            } else if (ent->d_type == DT_REG) {
                file_list[file_list_size] = (char *) malloc(sizeof(char) * 256);
                if (strlen(stack[i]) == len) {
                    strcpy(file_list[file_list_size], ent->d_name);
                } else {
                    strcpy(file_list[file_list_size], &stack[i][len + 1]);
                    strcat(file_list[file_list_size], "/");
                    strcat(file_list[file_list_size], ent->d_name);
                }
                file_list_size++;
            }
        }
        closedir(dir);
        i++;
    }
}


