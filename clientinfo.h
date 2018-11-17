#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H

typedef struct {
	char myfifo[200];
	char username[50];
	char password[50];

} CLIENTINFO, *CLIENTINFOPTR;



typedef struct {
	char name[50];
	char mess[200];
} MESSAGE, *MESS;

typedef struct {
	char cname[50];
	char psword[50];
} CLIENT, *CLIENTS;	
#endif