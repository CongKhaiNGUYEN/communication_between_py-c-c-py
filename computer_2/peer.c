#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define C_TO_PYTHON "CTOPYTHON"
#define PYTHON_TO_C "PYTHONTOC"


char name[1024];
int PORT;
int check = 1;

int sending_message();
void receiving(int server_fd);
void *receive_thread(void *server_fd);
void send_to_python(char* message);
char* recv_from_python();

int main(int argc, char const *argv[])
{

    strcpy(name, "joueur2");

    PORT = 4444;

    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int k = 0;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    // we can change the ip address here. For connecting multicomputers
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT);

    // display the server socket addr and port
    printf("IP address is: %s\n", inet_ntoa(address.sin_addr));
    printf("port is: %d\n", (int)ntohs(address.sin_port));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    int ch;
    pthread_t tid;
    pthread_create(&tid, NULL, &receive_thread, &server_fd);
    ch = 1;
    do
    {
        switch (ch)
        {
        case 1:
            ch = sending_message();
            break;
        case 0:
            printf("\nLeaving\n");
            break;
        default:
            printf("\nWrong choice\n");
        }
    } while (ch);

    close(server_fd);

    return 0;
}

// Send messages to port
int sending_message()
{

    char buffer[2000] = {0};
    int PORT_server = 9999;
    if (check == 1){
        printf("\npress enter\n");
        getchar();
        check = 0;
    }
    // printf("Enter the port to send message:");
    // scanf("%d", &PORT_server);

    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char hello[1024] = {0};
   
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY defaut is always 0.0.0.0
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT_server);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return 0;
    }

    char dummy;
    
    strcpy(buffer, recv_from_python());
    send(sock, buffer, 100, 0);
    printf("\nMessage sent\n");
    return 1;
}

// Calling receiving every 2 seconds
void *receive_thread(void *server_fd)
{
    int s_fd = *((int *)server_fd);
    while (1)
    {
        sleep(2);
        receiving(s_fd);
    }
}

void receiving(int server_fd)
{
    struct sockaddr_in address;
    int valread;
    char buffer[2000] = {0};
    int addrlen = sizeof(address);
    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_fd, &current_sockets);

    int k = 0;
    while (1)
    {
        k++;
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("Error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &ready_sockets))
            {

                if (i == server_fd)
                {
                    int client_socket;

                    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                                (socklen_t *)&addrlen)) < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(client_socket, &current_sockets);
                }
                else
                {
                    valread = recv(i, buffer, sizeof(buffer), 0);
                    buffer[sizeof(buffer)+1] = '\n';
                    buffer[sizeof(buffer) + 1] = '\0';
                    send_to_python(buffer);
                    FD_CLR(i, &current_sockets);
                }
            }
        }

        if (k == (FD_SETSIZE * 2))
            break;
    }
}

void send_to_python(char* message){
    int fd, check=0;
    mknod(C_TO_PYTHON, __S_IFIFO | 0640, 0);
    fd = open(C_TO_PYTHON, O_WRONLY);
    write(fd, message, 100);
    // if (strncmp(message, "end", 3)==0){
    //     printf("\nBYE BYE\n");
        
    // }
    // else if (strncmp(message, "exit",4)==0){
    //     printf("\nBYE BYE\n");
        
    // }
    // else if (strncmp(message, "END",3)==0){
    //     printf("\nBYE BYE\n");
        
    // }
    // else if (strncmp(message, "EXIT",4)==0){
    //     printf("\nBYE BYE\n");
        
    // }
    // if (check==1){
    //     unlink(C_TO_PYTHON);
    //     unlink(PYTHON_TO_C);
    // }
    close(fd);
}
char* recv_from_python(){
    int fd, readbytes;
    char* readbuf;
    readbuf = (char*)malloc(80);
    mknod(PYTHON_TO_C, __S_IFIFO | 0640, 0);
    fd = open(PYTHON_TO_C, O_RDONLY);
    readbytes = read(fd, readbuf, 100);
    return readbuf;
}

