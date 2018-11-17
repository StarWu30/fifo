#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "clientinfo.h"

#define FIFO_NAME1 "/home/wuxingshu_2016150122/code/server_fifo/fifo_1"
#define FIFO_NAME2 "/home/wuxingshu_2016150122/code/server_fifo/fifo_2"
#define FIFO_NAME3 "/home/wuxingshu_2016150122/code/server_fifo/fifo_3"

#define BLUE         "\033[0;32;34m" 
#define NONE         "\033[m" 
#define BUFF_SZ 300

char mypipename[BUFF_SZ];

int main() {
	int res;
	int op;
	int fifo_fd1, fifo_fd2, fifo_fd3;
	int my_fifo;
	CLIENTINFO info;
	MESSAGE message;
	char buffer[BUFF_SZ];
	char username[50];
	char password[50];
	if (access(FIFO_NAME1, F_OK) == -1) {
		perror("Could not open FIFO1\n");
		exit(EXIT_FAILURE);
	}
	
	if (access(FIFO_NAME2, F_OK) == -1) {
                perror("Could not open FIFO2\n");
                exit(EXIT_FAILURE);
        }
	if (access(FIFO_NAME3, F_OK) == -1) {
                perror("Could not open FIFO3\n");
                exit(EXIT_FAILURE);
        }
	
	fifo_fd1 = open(FIFO_NAME1, O_WRONLY);
	if (fifo_fd1 == -1) {
		perror("Could not open FIFO1 for write access\n");
		exit(EXIT_FAILURE);
	}	
	fifo_fd2 = open(FIFO_NAME2, O_WRONLY);
        if (fifo_fd2 == -1) {
                perror("Could not open FIFO1 for write access\n");
                exit(EXIT_FAILURE);
        } 
	fifo_fd3 = open(FIFO_NAME3, O_WRONLY);
        if (fifo_fd3 == -1) {
                perror("Could not open FIFO3 for write access\n");
                exit(EXIT_FAILURE);
        } 
	printf("\n");
	printf(BLUE"****************************************\n");
	printf("* Welcome to group-chatting!           *\n");
	printf("* 1. sign up                           *\n");
	printf("* 2. sign in                           *\n");
	printf("* 3. quit                              *\n");
	printf("****************************************\n"NONE);
	do{
		printf("Please select your operation:");
		scanf("%d", &op);	
	}while(op !=1 && op != 2 && op != 3);
	switch(op){
	case 1:
		// user input new username
		do {
			printf("Please input new username:");
			scanf("%s", username);
			sprintf(mypipename,"/home/wuxingshu_2016150122/code/server_fifo/%s",username);
			res = mkfifo(mypipename, 0777);
			if (res != 0) {
				printf("This username have already existed, please input another name\n");
			}
		}while(res);
		
		my_fifo = open(mypipename, O_RDONLY | O_NONBLOCK);
		if (my_fifo == -1) {
			perror("Client could not open own FIFO for read only access");
			exit(EXIT_FAILURE);
		}

		printf("Please input new password:");
		scanf("%s", password);
		strcpy(info.myfifo, mypipename);
		strcpy(info.username, username);
		strcpy(info.password, password);
		//printf("%s\n%s\n%s\n", info.myfifo, info.username, info.password);
		int b = write(fifo_fd1, &info, sizeof(CLIENTINFO));
		close(fifo_fd1);
		//printf("%d\n", b);
		memset(buffer, '\0', BUFF_SZ);
		while(1) {
			res = read(my_fifo, buffer, BUFF_SZ);
			if (res > 0) {
				printf("%s\n", buffer);
				break;
			}
		}
		close(my_fifo);
		break;
	case 2:
		do {
			printf("Please input your username:");
			scanf("%s", username);
			sprintf(mypipename,"/home/wuxingshu_2016150122/code/server_fifo/%s",username);
			my_fifo = open(mypipename, O_RDONLY | O_NONBLOCK);
			if (my_fifo == -1) {
				printf("The username does not exist.\n");
			}
		} while(my_fifo == -1);
		
		printf("Please input your password:");
		scanf("%s", password);
		strcpy(info.myfifo, mypipename);
		strcpy(info.username, username);
		strcpy(info.password, password);
		
		write(fifo_fd2, &info, sizeof(CLIENTINFO));
		//close(fifo_fd2);
		//printf("%s\n", mypipename);
		memset(buffer, '\0', BUFF_SZ);
		//("send buffer\n");
		while(1) {
			res = read(my_fifo, buffer, BUFF_SZ);
			//printf("read byte from server is %d\n", res);
			if (res > 0) {
				if(strcmp(buffer, "yes") == 0) {
					printf("Hello, %s!\n", username);
					break;
				}else {
					printf("Sorry, your password is wrong!\n");
					exit(EXIT_FAILURE);
				}
			}
		}
		pid_t pid = fork();
		while(1){
			if (pid > 0) {
				memset(buffer, '\0', BUFF_SZ);
				//printf("message:");
				fgets(buffer, BUFF_SZ, stdin);
				if (strcmp(buffer, "exit") == 0) {
					break;
				}
				strcpy(message.mess, buffer);
				strcpy(message.name, username);
				write(fifo_fd3, &message, sizeof(MESSAGE));	
			} else {
				memset(buffer, '\0', BUFF_SZ);
				while(1) {
					res = read(my_fifo, buffer, strlen(buffer)+1);
					if (res > 0) {
						printf("%s",buffer);
						break;
					}
				}
			}
		}
		close(my_fifo);
	default:
		printf("Have a good day and welcome here next time!\n");
		// close(fifo_fd1);
		// close(fifo_fd2);
		// close(fifo_fd3);
		break;
	}
}
