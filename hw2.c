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

// struct to store client info
typedef struct{
    int c_sock;
    char * c_user;
    int c_len;
}clients;

// handle guess
char * handle_guess(char * g, char * a, int a_len){
    // declare variables
    char * response = malloc(2 * sizeof(int));
    char * answer = malloc(strlen(a));
    char * guess = malloc(strlen(g));
    memcpy(answer, a, strlen(a));
    memcpy(guess, g, strlen(g));
    int correct = 0;
    int wrong_place = 0;

     // Check letters
        // check for correct letter and placement
        for(int i = 0; i < a_len; i++){
            if(guess[i] == answer[i]){
                correct++;
            }
        }

        // check for correct letter and wrong placement
        for(int i = 0; i < a_len; i++){
            if(guess[i] != '0'){
                for(int j = 0; j < a_len; j++){
                    if(answer[j] != '0'){
                        if(guess[i] == answer[j]){
                            answer[j] = '0';
                            wrong_place++;
                        }
                    }
                }
            }

        // build response 
        memcpy(response, &wrong_place, sizeof(int));
        memcpy(response+4, &correct, sizeof(int));
    }

    return response;
}

// handle username
bool handle_user(char * new_user, int new_len, clients * c_list, int num_users){
    // Check if username is being used
    for(int i = 0; i < 5; i ++){
        if(c_list[i].c_sock != -1){
            if(new_len == c_list[i].c_len){ // check length
                for(int j = 0; j < new_len; j++){
                    if(tolower(new_user[j]) != tolower((c_list[i].c_user)[j])){ // commpare letters
                        j = new_len;
                    }
                    if(j == new_len-1){
                        return false;
                    }
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
    FILE * fp;
    fp = fopen(argv[3], "r");

    if(!fp){
        fprintf(stderr, "ERROR: File %s not found\n", argv[3]);
        return EXIT_FAILURE;
    }

    // Get number of words
    char *x = malloc(longest * sizeof(char));
    int file_size = 0;
    while (fscanf(fp, "%s\n", x) == 1) {
        file_size++;
    }
    fseek(fp, 0, SEEK_SET);
    // Read in words
    char** dict = malloc(file_size * sizeof(char*));
    int y = 0;
    while (fscanf(fp, "%s\n", x) == 1) {
        dict[y] = malloc(strlen(x)+1);
        memcpy(dict[y], x, strlen(x)+1);
        dict[y][strlen(x)] = '\0';
        y++;
    }

    // Seed random number
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
    for(int i = 0; i < 5; i++){
        client[i].c_sock = -1;
    }

    // get random word
    int t = rand() % file_size;
    printf("Secret Word: %s\n", dict[t]);
    char * word = malloc(longest * sizeof(char));
    memcpy(word, dict[t], strlen(dict[t]));
    int num_connect = 0;
    word[strlen(dict[t])] = '\0';
    int word_len = strlen(dict[t]);

    // start word game
    while (1) {
        ready = readfd;
        
        // Ready for guesses
        FD_SET(server_socket, &ready);
        for (int i = 0; i < 5; i++) {
            if (client[i].c_sock != -1) {
                FD_SET(client[i].c_sock, &ready);
            }
        }
        if(num_connect == 0){
            printf("Waiting for connection\n");
        }
        else{
            printf("Waiting for guess\n");
        }
        select(FD_SETSIZE, &ready, NULL, NULL, NULL);
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready)) {
             // Check for new connection
                if (i == server_socket) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int new_client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
                    if (new_client_socket == -1) {
                        perror("accept failed");
                        continue;
                    }

                    // Add the new client socket to the set of file descriptors to monitor
                    int client_index = -1;
                    
                    //find empty spot in clients struct
                    for(int j = 0; j < 5; j++){
                        if(client[j].c_sock == -1){
                            client_index = j;
                            break;
                        }
                    }
                    if (client_index != -1) {
                        //add client
                        num_connect++;
                        client[client_index].c_sock = new_client_socket;
                        client[client_index].c_len = 0;

                        // prompt for username
                        char username[512];
                       memset(username, 0, sizeof(username));
                        write(new_client_socket, "Welcome to Guess the Word, please enter your username.", 56);
                        read(new_client_socket, username, sizeof(username) - 1);
                        username[strlen(username) - 1] = '\0';

                        // check for valid username
                        while(client[client_index].c_len == 0){
                            if (num_connect > 1 && !handle_user(username, strlen(username), client, 5)) {//invalid username reprompt client
                                write(new_client_socket, "Username is already taken, please enter a different username", 61);
                                read(new_client_socket, username, sizeof(username) - 1);
                            } else { // valid username
                                client[client_index].c_user = strdup(username);
                                client[client_index].c_len = strlen(username);
                                char welcome_message[512];
                                snprintf(welcome_message, sizeof(welcome_message), "Let's start playing, %s\n", username);
                                write(new_client_socket, welcome_message, strlen(welcome_message));
                                //add user to game
                                printf("User %s added to game\n", username);
                                char join_msg[512];
                                snprintf(join_msg, sizeof(join_msg), "There are %d player(s) playing. The secret word is %d letter(s).\n", num_connect, word_len);
                                write(new_client_socket, join_msg, strlen(join_msg));
                            }
                        }
                    } else {// reject if 5 players
                        write(new_client_socket, "Server is busy. Please try again later.\n", 40);
                        close(new_client_socket);
                    }
                }   
                else { // handle guess
                    char message[512];
                    memset(message, 0, 512);
                    int amount = read(i, message, 512);
                    struct sockaddr_in server_addr2;
                    socklen_t len;
                    // get client information
                    getpeername(i, (struct sockaddr*)&server_addr2, &len);
                    // find user
                    int index;
                    for(int j = 0; j < 5; j++){
                        if(client[j].c_sock == i){
                            index = j;
                            break;
                        }
                    }
                    // set up guess packet
                    printf("User %s made a guess\n", client[index].c_user);
                    char * guess_pck = malloc(2*sizeof(int));
                    message[strlen(message)-1]='\0';
                    
                    char guess_msg [512];
                    bool no_msg = false;
                    bool correct = false;
                    if(amount == 0 || amount == -1){ // check for disconnect
                        printf("User %s disconnected\n", client[index].c_user);
                        // close connection
                        FD_CLR(i, &readfd);
                        close(i);
                        //remove from clients
                        num_connect--;
                        client[index].c_sock = -1;
                        client[index].c_user = NULL;
                        client[index].c_len = 0;
                        no_msg = true;
                    }
                    else if(strlen(message) != word_len){// Check for valid guess
                        no_msg = true;
                        printf("Invalid guess length. The secret word is %ld letter(s).\n", strlen(word));
                        snprintf(guess_msg, sizeof(guess_msg), "Invalid guess length. The secret word is %ld letter(s).\n", strlen(word));
                        write(client[index].c_sock, guess_msg, strlen(guess_msg));
                    }
                    else{ // valid guess length
                        guess_pck = handle_guess(message, word, word_len);
                        if(guess_pck[4] == strlen(word)){ // correct guess
                            printf("%s has correctly guessed the word %s\n", client[index].c_user, word);
                            snprintf(guess_msg, sizeof(guess_msg), "%s has correctly guessed the word %s\n", client[index].c_user, word);
                            // pick new word and restart game
                            correct = true;
                            free(word);
                            t = rand() % file_size;
                            printf("Secret Word: %s\n", dict[t]);
                            word = malloc(longest * sizeof(char));
                            word_len = strlen(dict[t]);
                            memcpy(word, dict[t], strlen(dict[t]));
                            word[strlen(dict[t])] = '\0';
                        }
                        else{ // incorrect guess
                            printf("%s guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", client[index].c_user, message, *guess_pck, (guess_pck[4]));
                            snprintf(guess_msg, sizeof(guess_msg), "%s guessed %s: %d letter(s) were correct and %d letter(s) were correctly placed.\n", client[index].c_user, message, *guess_pck, (guess_pck[4]));
                        }
                    }    

                    //send guess message to all players if applicable
                    if(!no_msg){
                        for(int j = 0; j < 5; j++){
                            if(client[j].c_sock != -1){
                                write(client[j].c_sock, guess_msg, strlen(guess_msg));
                                if(correct){
                                    FD_CLR(client[j].c_sock, &readfd);
                                    close(client[j].c_sock);
                                    client[j].c_sock = -1;
                                    client[j].c_user = NULL;
                                    client[j].c_len = 0;
                                    num_connect--;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}