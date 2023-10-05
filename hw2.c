#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>

struct sockaddr_in server_addr;

// function to get and bind socket
int get_socket(int port) {
    int socket_FD = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_port = htons(port);
    if (bind(socket_FD, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Unable to bind socket");
        exit(EXIT_FAILURE);
    }
    return socket_FD;
}

int main(int argc, char** argv) {
    // take in and parse input
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Program Start\n");
    if (argc != 5) {
        fprintf(stderr, "Usage: %s [seed] [port] [dictionary_file] [longest_word_length]\n", argv[0]);
        exit(1);
    }

    int seed = atoi(argv[1]);
    int port= atoi(argv[2]);
    int longest = atoi(argv[4]);
    srand(seed);

    // Error check input
    if(longest < 0 || longest > 1024){
        printf("Invalid longest word length\n");
        exit(1);
    }

    // Read in dictionary file
    FILE * fp = fopen(*(argv+3), "r");

    if(!fp){
        printf("%s\n", *(argv+3));
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: hw4.out <listener-port> <seed> <word-filename> <num-words>");
        return EXIT_FAILURE;
    }
    int file_size;
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read in words
    char** dict = malloc(file_size * sizeof(char*));
    for(int i = 0; i < file_size; i++){
        dict[i] = malloc(longest * sizeof(char));
    }

    char *x = malloc(longest * sizeof(char));
    int y = 0;
    while (fscanf(fp, "%s\n", x) == 1) {
        memcpy(dict[y], x, longest);
    }

    for(int i = 0; i < file_size; i++){
        int j = 0;
        while(dict[i][j]){
            j++;
        }
        dict[i][j] = '\0';
    }

    // Set up server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    int num_connect = 0;
    fd_set readfd, ready;
    FD_ZERO(&readfd);
    FD_SET(STDIN_FILENO, &readfd);

    int client_sockets[5]; 
    char * client_users[5]; 

    // start word game
    while (1) {
        ready = readfd;
        select(FD_SETSIZE, &ready, NULL, NULL, NULL);

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready)) {
                // get new connection
                if (i == STDIN_FILENO) {
                    int newport = 0;
                    scanf("%d", &newport);
                    if (num_connect < 5) {
                        int socket_FD = get_socket(port);
                        server_addr.sin_port = htons(newport);
                        if (connect(socket_FD, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
                            perror("Unable to connect");
                            exit(EXIT_FAILURE);
                        }
                        FD_SET(socket_FD, &readfd);
                        client_sockets[num_connect] = socket_FD;
                        num_connect++;
                    }
                } else { // handle guess
                    char message[512];
                    memset(message, 0, 512);
                    int amount = read(i, message, 512);
                    struct sockaddr_in server_addr2;
                    socklen_t len;
                    getpeername(i, (struct sockaddr*)&server_addr2, &len);
                    if (amount == 0) {
                        printf(">Server on %d closed\n", ntohs(server_addr2.sin_port));
                        FD_CLR(i, &readfd);
                        close(i);
                    } else {
                        printf(">%d %s\n", ntohs(server_addr.sin_port), message);
                    }
                }
            }
        }
    }

}