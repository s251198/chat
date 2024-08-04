// Soumyadeep Das, Ansh Patel, Shifra Suthakar, Anirban Maity, Lakshman Paladugu
// CMPSC 311: Introduction to System Programming
// Final Project Option 1(Chat Application): Client Component
// 08/02/2024


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *receive_messages(void *sock) { //receives messages from server
    int client_socket = *((int *)sock); //client socket from argument
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = read(client_socket, buffer, BUFFER_SIZE)) > 0) { //reads messages from server
        buffer[n] = '\0'; //null-terminates string
        printf("%s\n", buffer);
    }

    return NULL;
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr; //server address
    char buffer[BUFFER_SIZE];
    pthread_t recv_thread;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //creates socket
        printf("\n Socket creation error \n");
        return -1;
    }

    //server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { //IP to binary
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { //connects to server
        printf("\nConnection Failed \n");
        return -1;
    }

    pthread_create(&recv_thread, NULL, receive_messages, &sock); //creates thread to receive messages from server

    while (1) { //reads user input and sends messages to server
        fgets(buffer, BUFFER_SIZE, stdin);
        if (send(sock, buffer, strlen(buffer), 0) < 0) { //sends messages to server
            printf("Send failed\n");
            break;
        }
    }

    close(sock);
    return 0;
}
