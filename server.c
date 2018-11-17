#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "clientinfo.h"
#define SIZE 300
#define FIFO_NAME1 "/home/wuxingshu_2016150122/code/server_fifo/fifo_1"
#define FIFO_NAME2 "/home/wuxingshu_2016150122/code/server_fifo/fifo_2"
#define FIFO_NAME3 "/home/wuxingshu_2016150122/code/server_fifo/fifo_3"


CLIENT client[50];
int num = 0;

int daemon(int nochdir, int noclose) {
	pid_t pid;
	pid = fork();
	/* if create process fail */
	if (pid < 0) {
		perror("fork");
		return -1;
	}
	/* parent process exit */
	if (pid != 0)
		exit(0);
	pid = setsid();
	if (pid < -1) {
		perror("setsid");
	 	return -1;
	}
	/* change working directory to root directory  */
	if (!nochdir)
		chdir("/");
	/* redirect std input and output to empty device */
	if (!noclose) {
		int fd;
		fd = open("/dev/null", O_RDWR, 0);
		if (fd != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if (fd > 2)
				close(fd);
		}
	}
	umask(0000);
	return 0;
}

int get_clientindex(CLIENTINFO info) {
	for (int i = 0; i < num; i++) {
		if (strcmp(info.username,client[i].cname) == 0) {
			return i;
		}
	}
	return -1;
}

void handler(int sig) {
	unlink(FIFO_NAME1);
	unlink(FIFO_NAME2);
	unlink(FIFO_NAME3);
	exit(1);
}

int main() {
	char buffer[SIZE];
	char fifoname[SIZE];
	int res;
	int index;
	int fdl;
	int maxfd;
	int sig;
	int fifo_fd1, fifo_fd2, fifo_fd3;
	CLIENTINFO info;
	
	MESSAGE message;
	fd_set my_read;
	struct timeval time;
	time.tv_sec = 5;
	time.tv_usec = 0;
	
	signal(SIGKILL, handler);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);
	
	//daemon(0,0);

	/* create sign up FIFO */
	if (access(FIFO_NAME1, F_OK) == -1) {
		res = mkfifo(FIFO_NAME1, 0777);
		if (res != 0) {
			perror("Sign up FIFO was not created\n");
			exit(EXIT_FAILURE);
		}
	}	

	/* create sign in FIFO */
	if (access(FIFO_NAME2, F_OK) == -1) {
		res = mkfifo(FIFO_NAME2, 0777);
		if (res != 0) {
			perror("Sign in FIFO was not created\n");
			exit(EXIT_FAILURE);
		}
	}

	/* create group-chatting FIFO */
	if (access(FIFO_NAME3, F_OK) == -1) {
		res = mkfifo(FIFO_NAME3, 0777);
		if (res != 0) {
			perror("Group-chatting FIFO was not created\n");
			exit(EXIT_FAILURE);
		}
	}
	
	/* open sign up FIFO for reading */
	fifo_fd1 = open(FIFO_NAME1, O_RDONLY);
	if (fifo_fd1 == -1) {
		perror("Server could not open sign up FIFO for read only access\n");
		exit(EXIT_FAILURE);
	}

	/* open sign in FIFO for reading */
	fifo_fd2 = open(FIFO_NAME2, O_RDONLY);
	if (fifo_fd2 == -1) {
		perror("Server could not open sign in FIFO for read only access\n");
		exit(EXIT_FAILURE);
	}
	
	/* open group-chatting FIFO for reading */
	fifo_fd3 = open(FIFO_NAME3, O_RDONLY);
	if (fifo_fd3 == -1) {
		perror("Server could not open group-chatting FIFO for read only access\n");
		exit(EXIT_FAILURE);
	}

	while(1) {
		FD_ZERO(&my_read);
		FD_SET(fifo_fd1, &my_read);
		FD_SET(fifo_fd2, &my_read);
		FD_SET(fifo_fd3, &my_read);
		maxfd = fifo_fd1 >= fifo_fd2? fifo_fd1 : fifo_fd2;
		maxfd = maxfd >= fifo_fd3 ? maxfd : fifo_fd3;
		//printf("maxfd is %d, fd1 is %d, fd2 is %d, fd3 is %d\n", maxfd, fifo_fd1, fifo_fd2, fifo_fd3);
		sig = select(maxfd+1, &my_read, NULL, NULL, &time);
		if(sig == -1) {
			perror("Select funtion is wrong");
			exit(-1);
		}
		// if(res == 0) {
		// 	printf("res is %d\n", res);
		// 	continue;
		// }

		/* sign up fifo */
		if(FD_ISSET(fifo_fd1, &my_read)) {
			// read client infomation
			res = read(fifo_fd1, &info, sizeof(CLIENTINFO));
			printf("fifo1 %d\n", res);
			if (res != 0) {
				// open client fifo
				fdl = open(info.myfifo, O_WRONLY | O_NONBLOCK);
				if (fdl == -1){
					// if fail, print error reason
					perror("Server could not open this FIFO");
					
				}
				// send succuss sign to client
				memset(buffer, '\0', SIZE);
				sprintf(buffer, "%s sign up successfully!\n", info.username);
				write(fdl, buffer, strlen(buffer)+1);
				// add client
				strcpy(client[num].cname, info.username);
				strcpy(client[num].psword, info.password);
				printf("%s %s\n", client[num].cname, client[num].psword);
				num++;
				close(fdl);
			}
		}

		/* sign in fifo */
		if(FD_ISSET(fifo_fd2, &my_read)) {
			// read client information
			res = read(fifo_fd2, &info, sizeof(CLIENTINFO));
			printf("fifo2 %d\n", res);
			if (res != 0) {
				printf("client arrived\n");
				// open client fifo
				fdl = open(info.myfifo, O_RDWR | O_NONBLOCK);
				if (fdl == -1) {
					printf("%s fail\n", info.myfifo);
					perror("Server could not open this FIFO");
					exit(EXIT_FAILURE);
				}
				printf("%s OK\n", info.myfifo);
				memset(buffer, '\0', SIZE);
				index = get_clientindex(info);
				if (index == -1) {
					sprintf(buffer, "%s does not exist", info.username);
					printf("%s\n", buffer);
					write(fdl, buffer, strlen(buffer)+1);
				} else {
					// check password
					if (strcmp(info.password, client[index].psword) == 0) {
						strcpy(buffer, "yes");
						printf("%s\n", buffer);
						write(fdl, buffer, strlen(buffer)+1);
					} else {
						printf("%s\n", buffer);
						write(fdl, buffer, strlen(buffer)+1);
					}
				}
				close(fdl);
			} 
		}
		/* group-chatting fifo */
		if(FD_ISSET(fifo_fd3, &my_read)) {
			// read client information
			res = read(fifo_fd3, &message, sizeof(MESSAGE));
			printf("fifo3 %d\n", res);
			if (res != 0) {
				memset(buffer, '\0', SIZE);
				// merge username and his message
				sprintf(buffer, "%s:%s",message.name, message.mess);
				printf("num is %d\n", num);
				for(int i = 0; i < num; i++) {
					// send to every client
					memset(fifoname, '\0', SIZE);
					sprintf(fifoname, "/home/wuxingshu_2016150122/code/server_fifo/%s",client[i].cname);
					if (access(fifoname, F_OK) == -1) {
						printf("%s", client[i].cname);
						perror(" could not be accessed");
						continue;
					}
					// open client fifo
					fdl = open(fifoname, O_RDWR | O_NONBLOCK);
					if (fdl == -1) {
						printf("%s", client[i].cname);
						perror(" have not existed yet");
						continue;
					}
					printf("%s", buffer);
					write(fdl, buffer, sizeof(buffer)+1);
					close(fdl);
				}
			}
		}
	}
	return 0;
}	
