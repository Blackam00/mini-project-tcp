#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Status Codes ตามโจทย์
#define STATUS_OK 0
#define STATUS_FAIL 1
#define STATUS_INVALID_ARGUMENTS 200
#define STATUS_BOARD_NOT_FOUND 201
#define STATUS_TASK_NOT_FOUND 203
#define STATUS_SAVE_BOARD_FAIL 500
#define STATUS_SAVE_TASK_FAIL 502

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0; // ลบ newline ที่ติดมา
        printf("[Server Received]: %s\n", buffer);

        char command[32] = {0};
        char arg1[256] = {0};
        char arg2[256] = {0};
        
        // แยกคำสั่งและอาร์กิวเมนต์คั่นด้วยเครื่องหมายคอมมา (,)
        sscanf(buffer, "%[^,], %[^,], %[^\n]", command, arg1, arg2);

        char response[BUFFER_SIZE] = {0};

        // 1. LIST_BOARD
        if (strcmp(command, "LIST_BOARD") == 0) {
            FILE *fp = fopen("boards.txt", "r");
            if (fp) {
                char line[256];
                int count = 1;
                char list_content[BUFFER_SIZE] = "";
                while (fgets(line, sizeof(line), fp)) {
                    line[strcspn(line, "\r\n")] = 0;
                    char temp[300];
                    snprintf(temp, sizeof(temp), "%d. %s\n", count++, line);
                    strcat(list_content, temp);
                }
                fclose(fp);
                snprintf(response, sizeof(response), "%s", list_content);
            } else {
                snprintf(response, sizeof(response), "");
            }
        }
        // 2. ADD_BOARD
        else if (strcmp(command, "ADD_BOARD") == 0) {
            FILE *fp = fopen("boards.txt", "a");
            if (fp) {
                fprintf(fp, "%s\n", arg1);
                fclose(fp);
                snprintf(response, sizeof(response), "ADD_BOARD, %d", STATUS_OK);
            } else {
                snprintf(response, sizeof(response), "ADD_BOARD, %d", STATUS_SAVE_BOARD_FAIL);
            }
        }
        // 3. LIST_TASK (ตาม Board ID ที่เลือก)
        else if (strcmp(command, "LIST_TASK") == 0) {
            FILE *fp = fopen("tasks.txt", "r");
            char list_content[BUFFER_SIZE] = "";
            if (fp) {
                char line[256];
                int count = 1;
                while (fgets(line, sizeof(line), fp)) {
                    // รูปแบบในไฟล์ tasks.txt เก็บเป็น: BoardID|TaskTitle
                    char b_id[32], t_title[220];
                    if (sscanf(line, "%[^|]|%[^\n]", b_id, t_title) == 2) {
                        if (strcmp(b_id, arg1) == 0) {
                            char temp[300];
                            snprintf(temp, sizeof(temp), "%d. %s\n", count++, t_title);
                            strcat(list_content, temp);
                        }
                    }
                }
                fclose(fp);
            }
            snprintf(response, sizeof(response), "%s", list_content);
        }
        // 4. ADD_TASK
        else if (strcmp(command, "ADD_TASK") == 0) {
            FILE *fp = fopen("tasks.txt", "a");
            if (fp) {
                // บันทึกรูปแบบ BoardID|TaskTitle
                fprintf(fp, "%s|%s\n", arg1, arg2);
                fclose(fp);
                snprintf(response, sizeof(response), "ADD_TASK, %d", STATUS_OK);
            } else {
                snprintf(response, sizeof(response), "ADD_TASK, %d", STATUS_SAVE_TASK_FAIL);
            }
        }
        else {
            snprintf(response, sizeof(response), "UNKNOWN_COMMAND, %d", STATUS_FAIL);
        }

        send(client_socket, response, strlen(response), 0);
    }

    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket >= 0) {
            printf("Client connected!\n");
            handle_client(new_socket);
        }
    }

    close(server_fd);
    return 0;
}