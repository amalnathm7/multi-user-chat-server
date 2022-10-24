#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

void* clientSend(void* fd) {
	char str[100];
	int client_fd = *((int*) fd);
	int wordCount = 0;
	int totalCount = 25;
	
	printf("Chat active...\n");
	
	while(1) {
		fgets(str, 100, stdin);
		str[strlen(str) - 1] = '\0';
		
		if(!strcmp(str, "stop")) {
			printf("Exiting.\n");
			exit(1);
		}
		
		if(strlen(str) != 0) {
			int count = 0;
			char* tempStr = strdup(str);
			char* token = strtok(tempStr, " ");
			
			while(token != NULL) {
				count++;
				token = strtok(NULL, " ");
			}
			
			wordCount += count;
			
			if(wordCount <= totalCount) {
				if(send(client_fd, str, (strlen(str) + 1) * sizeof(char), 0) < 0) {
					printf("Send failed!\n");
					exit(1);
				}
				
				if(wordCount == totalCount) {
					printf("Chat limit reached. Exiting.\n");
					exit(1);
				}
			} else {
				printf("Chat limit exceeded by %d words.\n", wordCount - totalCount);
				wordCount -= count;
			}
		}
	}
}

void* clientRecv(void* fd) {
	char str[120];
	int client_fd = *((int*) fd);
	
	while(1) {
		int k = recv(client_fd, str, 120 * sizeof(char), 0);
		
		if(k < 0) {
			printf("Receive failed!\n");
			exit(1);
		} else if(k == 0) {
			printf("Server down!\n");
			exit(1);
		} else {
			printf("%s\n", str);
		}
	}
}

void main() {
	int client_fd;
	struct sockaddr_in serv_addr;
	
	printf("Chat Client\n");
	
	if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket creation failed!\n");
		exit(1);
	} else {
		printf("Client socket created.\n");
	}
	
	int port;
	printf("Command: ");
	scanf("start %d", &port);
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	
	if(connect(client_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Connection failed!\n");
		exit(1);
	} else {
		printf("Connection established.\n");
	}
	
	pthread_t tid1, tid2;
	
	pthread_create(&tid1, NULL, clientSend, &client_fd);
	pthread_create(&tid2, NULL, clientRecv, &client_fd);
	
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	
	close(client_fd);
}
