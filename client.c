#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void show_board_menu() {
    printf("\nBoard\n");
    printf("1. List Board\n");
    printf("2. Create Board\n");
    printf("3. Update Board\n");
    printf("4. Delete Board\n");
    printf("Please select option: ");
}

void show_task_menu() {
    printf("\nWhat do you want to do?\n");
    printf("1. Create Task\n");
    printf("2. Edit Task\n");
    printf("3. Delete Task\n");
    printf("You select option: ");
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    while (1) {
        show_board_menu();
        int option;
        if (scanf("%d", &option) != 1) break;
        getchar(); // clear newline

        if (option == 1) { // List Board
            char message[] = "LIST_BOARD";
            send(sock, message, strlen(message), 0);

            memset(buffer, 0, BUFFER_SIZE);
            recv(sock, buffer, BUFFER_SIZE, 0);

            printf("\nTask\n");
            printf("List Boards:\n");
            printf("%s", buffer);

            int board_no;
            printf("Please select Board no: ");
            if (scanf("%d", &board_no) != 1) break;
            getchar();

            // หลังจากเลือก Board แล้ว เข้าสู่หน้า List/Manage Task
            while (1) {
                char task_msg[BUFFER_SIZE];
                snprintf(task_msg, sizeof(task_msg), "LIST_TASK, %d", board_no);
                send(sock, task_msg, strlen(task_msg), 0);

                memset(buffer, 0, BUFFER_SIZE);
                recv(sock, buffer, BUFFER_SIZE, 0);

                printf("\nList Task in Board no. %d\n", board_no);
                if (strlen(buffer) > 0) {
                    printf("%s", buffer);
                } else {
                    printf("(No tasks yet)\n");
                }

                show_task_menu();
                int task_option;
                if (scanf("%d", &task_option) != 1) break;
                getchar();

                if (task_option == 1) { // Create Task
                    char task_title[256];
                    printf("Please input task title: ");
                    fgets(task_title, sizeof(task_title), stdin);
                    task_title[strcspn(task_title, "\n")] = 0;

                    char add_task_msg[BUFFER_SIZE];
                    snprintf(add_task_msg, sizeof(add_task_msg), "ADD_TASK, %d, %s", board_no, task_title);
                    send(sock, add_task_msg, strlen(add_task_msg), 0);

                    memset(buffer, 0, BUFFER_SIZE);
                    recv(sock, buffer, BUFFER_SIZE, 0);
                    printf("Response: %s\n", buffer);
                } else {
                    // สำหรับ Edit / Delete Task สามารถเพิ่มเงื่อนไขต่อได้ที่นี่
                    break;
                }
            }

        } 
        else if (option == 2) { // Create Board
            char board_name[256];
            printf("Please input board title: ");
            fgets(board_name, sizeof(board_name), stdin);
            board_name[strcspn(board_name, "\n")] = 0;

            char message[BUFFER_SIZE];
            snprintf(message, sizeof(message), "ADD_BOARD, %s", board_name);
            send(sock, message, strlen(message), 0);

            memset(buffer, 0, BUFFER_SIZE);
            recv(sock, buffer, BUFFER_SIZE, 0);
            printf("Response: %s\n", buffer);
        }
        else {
            break;
        }
    }

    close(sock);
    return 0;
}