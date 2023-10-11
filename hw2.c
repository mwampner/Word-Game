#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

struct sockaddr_in server_addr;

typedef struct{
    int c_sock;
    char * c_user;
}clients;

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

// handle guess
char * handle_guess(char * g, char * a){
    // declare variables
    char * response = malloc(2 * sizeof(int));
    char * answer = malloc(strlen(a));
    char * guess = malloc(strlen(g));
    memcpy(answer, a, strlen(a));
    memcpy(guess, g, strlen(g));
    int correct = 0;
    int wrong_place = 0;

    // Validate guess word has correct length
    if(strlen(guess) != strlen(answer)){
        response[0] = 'x';
    }
    else{ // Check letters
        // check for correct letter and placement
        for(int i = 0; i < strlen(answer); i++){
            if(guess[i] == answer[i]){
                correct++;
            }
        }

        // check for correct letter and wrong placement
        for(int i = 0; i < strlen(guess); i++){
            if(guess[i] != '0'){
                for(int j = 0; j < strlen(answer); j++){
                    if(answer[j] != '0'){
                        if(guess[i] == answer[j]){
                            answer[j] = '0';
                            wrong_place++;
                        }
                    }
                }
            }
        }

        // build response 
        memcpy(response, &correct, sizeof(int));
        memcpy(response+4, &wrong_place, sizeof(int));
    }

    return response;
}

// handle username
bool handle_user(char * new_user, char ** user_list, int num_users, int len){
    // Check if username is being used
    for(int i = 0; i < num_users; i ++){
        if(len == strlen(user_list[i])){
            for(int j = 0; j < len; j++){
                if(tolower(new_user[j]) != tolower(user_list[i][j])){
                    j = len;
                }
                if(j = len-1){
                    return false;
                }
            }
        }
    }

    // indicates valid username
    return true;
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
    srand(seed);

    // Set up server
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket error");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 1) == -1) {
        perror("failed");
        close(server_socket);
        exit(1);
    }
    fd_set readfd, ready;
    FD_ZERO(&readfd);
    FD_SET(STDIN_FILENO, &readfd);

    //int client_sockets[5]; 
    //char * client_users[5]; 
    clients * client = malloc(5 * sizeof(clients));
    char ** user_list = malloc(5*sizeof(char*));
    for(int i = 0; i < 5; i++){
        client[i].c_sock = -1;
    }

    // get random word
    int t = rand() % file_size;
    char * word = malloc(longest * sizeof(char));
    memcpy(word, dict[t], strlen(dict[t]));
    word[strlen(dict[t])] = '\0';
    int num_connect = 0; 

    // start word game
    while (1) {
        ready = readfd;
        
        // Check for disconnection
        if(num_connect != 0){
            for(int i = 0; i < 5; i++){
                
            }
        }

        // Ready for guesses
        select(FD_SETSIZE, &ready, NULL, NULL, NULL);
        int newsd;
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready)) {
                // get new connection
                if (i == STDIN_FILENO) {
                    int newport = 0;
                    scanf("%d", &newport);
                    if (num_connect < 5) {

                        // Accept client connection
                        struct sockaddr_in remote_client;
                        int addrlen = sizeof( remote_client );
                        newsd = accept(server_socket, (struct sockaddr *)&remote_client,
                        (socklen_t *)&addrlen );
                        if ( newsd == -1 ) { break; }
                        /*int socket_FD = get_socket(port);
                        server_addr.sin_port = htons(newport);
                        if (connect(socket_FD, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
                            perror("Unable to connect");
                            exit(EXIT_FAILURE);
                        }*/
                        FD_SET(newsd, &readfd);
                        //find empty spot in clients struct
                        for(int i = 0; i < 5; i++){
                            if(client[i].c_sock == -1){
                                client[i].c_sock = newsd;
                                num_connect++;
                            }
                        }
                    }

                    // get username
                    char * user = malloc(512);
                    int user_len;
                    int n = send( newsd, "Welcome to Guess the Word, please enter your username.\0", 55, 0 );
                    if ( n == -1 ) { perror( "send() failed" ); }

                    // receive potential username
                    bool unused;
                    n = recv( newsd, user, 512, 0 );
                    if(n == 0 || n == -1){// disconnect client
                        for(int i = 0; i < 5; i++){
                            if(newsd == client[i].c_sock){
                                client[i].c_sock = -1;
                                num_connect--;
                            }
                        }
                    }
                    else{
                        user_len = n;
                        user[user_len] = '\0';
                        // check for valid username
                        unused = handle_user(user, user_list, num_connect, user_len);
                    }
                    // prompt for username until valid
                    while(!unused){
                        // build new packet
                        char * packet = malloc(62 + strlen(user));
                        memcpy(packet, "Username ", 9);
                        memcpy(packet+9, user, strlen(user));
                        // prompt for new user
                        memcpy(packet+9+strlen(user), " is already taken, please enter a different username\0", 53);
                        n = send( newsd, packet,  62+strlen(user), 0 );
                        free(user);
                        user = malloc(512);
                        // receive new user
                        n = recv( newsd, user, 512, 0 );
                        if(n == 0 || n == -1){// disconnect client
                            for(int i = 0; i < 5; i++){
                                if(newsd == client[i].c_sock){
                                    client[i].c_sock = -1;
                                    num_connect--;
                                }
                            }
                            break;
                        }
                        else{
                            user_len = n;
                            user[user_len] = '\0';
                            // check for valid username
                            unused = handle_user(user, user_list, num_connect, user_len);
                        }
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