// Soumyadeep Das, Ansh Patel, Shifra Suthakar, Anirban Maity, Lakshman Paladugu
// CMPSC 311: Introduction to System Programming
// Final Project Option 1(Chat Application): Server Component
// 08/02/2024


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

typedef struct ClientNode {
    int client_socket; //client socket descriptor
    struct ClientNode *next; //pointer for next client in linked list
} ClientNode;

ClientNode *head = NULL; //head of linked list
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for thread synch
int client_counter = 0; //counter for client ids
int client_ids[MAX_CLIENTS] = {0}; //client id and socket array

void add_client(int client_socket) { //when a new client joins the server, function adds client to the linked list
    pthread_mutex_lock(&clients_mutex); //locks mutex for thread safetey

    ClientNode *new_node = (ClientNode *)malloc(sizeof(ClientNode)); //allocates memory and initializes new client node
    new_node->client_socket = client_socket;
    new_node->next = head;
    head = new_node;

    client_counter++;
    client_ids[client_socket] = client_counter; //assigns clients their client id

    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "User %d has joined the server.\n", client_counter);
    printf("%s", message); //prints to server

    pthread_mutex_unlock(&clients_mutex); //unlocks mutex
}

void remove_client(int client_socket) { //when a client leaves the server, function removes client from the linked list

    pthread_mutex_lock(&clients_mutex);

    ClientNode **current = &head; //pointer to head of list
    while (*current) { //go through list
        ClientNode *entry = *current; //get current node
        if (entry->client_socket == client_socket) { //check if correct client to remove
            *current = entry->next; //pointer skips current node
            free(entry); //free memory allocated for current node
            break;
        }
        current = &entry->next; //check next client node
    }

    pthread_mutex_unlock(&clients_mutex);
}


void clients_server(char *message, int sender_fd) { //sends messages to all client chats except sender
    pthread_mutex_lock(&clients_mutex);

    ClientNode *current = head; //start at head of client list
    while (current) { //go through list
        if (current->client_socket != sender_fd) { //skip sender
            if (write(current->client_socket, message, strlen(message)) < 0) { //send message to individual chats
                perror("Error writing to client");
            }
        }
        current = current->next; //check next client
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) { //handles client communication
    int client_socket = *((int *)arg); //get client socket from argument
    free(arg); //free allocated memory for argument
    char buffer[BUFFER_SIZE]; //buffer for messages
    int n;

    char welcome_message[BUFFER_SIZE];
    snprintf(welcome_message, sizeof(welcome_message), "Server Joined! You are User %d\n", client_ids[client_socket]);
    write(client_socket, welcome_message, strlen(welcome_message));

    while ((n = read(client_socket, buffer, BUFFER_SIZE)) > 0) {
        buffer[n] = '\0'; //null-terminate string
        if (strlen(buffer) > 0) {
            char message[BUFFER_SIZE + 50];
            snprintf(message, sizeof(message), "User %d: %s", client_ids[client_socket], buffer);
            printf("%s", message); //print message to server
            clients_server(message, client_socket); //print message to all clients except sender
        }
    }

    if (n == 0) { //client disconnects from server
        printf("User %d has left the server.\n", client_ids[client_socket]); //print message to server
        clients_server("A User has left the chat.\n", client_socket); //print message to all clients
    } else if (n < 0) {
        perror("Error reading from client");
    }


    close(client_socket);
    remove_client(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) //creates socket
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) //sets socket options
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    //server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) //bind socket to address
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) //listen for incoming connections
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    if (1) {printf("Server Opened\n\n");} //server opened successfully confirmation message

    while (1) { //accept and handles incoming connections
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        add_client(client_socket); //adds client to linked list

        pthread_t tid;
        int *client_sock_ptr = malloc(sizeof(int)); //allocates memory for client socket
        *client_sock_ptr = client_socket; //stores client socket
        pthread_create(&tid, NULL, handle_client, client_sock_ptr); //creates new thread to handle the client
        pthread_detach(tid); //detaches thread
    }

    close(server_fd);
    return 0;
}
