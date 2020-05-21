#ifndef _HEADER_H_
#define _HEADER_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include <ftw.h>
#include <linux/limits.h>
#include <openssl/md5.h>
#include <libgen.h>
#define MAXLINE 4096
typedef struct sockaddr_in SA_IN;

typedef struct{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

struct node{
	char* versionNumber;
	char* name;
	char* hashcode;
	struct node * next;
};

typedef struct _locks {
	char* project_name;
	pthread_mutex_t lock;
	struct _locks* next;
} ProjectLock;

void err_n_die(const char *fmt, ...);
void killSignal();
int argComp(char* string);
void* readCommand(void* p_connfd);
void create(int versionNumber, char* path, char* project, int listenfd);
int destroy(char* path);
struct node* insertValues(struct node* head, char* path);
struct node* makeList(struct node* head, char* vNum, char* file, char* hash);
int update(struct node* head, struct node* head2, int connfd, int countingNode, char* project);
void currentVersion(struct node* head, int connfd);
void upgrade(char* path, int connfd, char* project);
void checkout(char* path, int connfd, struct node* curHead, char* project);
void commit(char* path, int connfd, char* project);
void push(char* path, char* path2, int connfd, char* project);
unsigned char* hash(char* project, char* path);
void _mkdir(const char *dir);
void add(char* project, char* file_name, char* fileNameInsert);
int checkMan(char* file_name, char* project);
char *recursive(char *basePath, char *fileName);
int checkFile(char *basePath);
int* remove_line(char* project, char* file_name);
void history(char* path, int connfd);
void rollback(char* project, char* version, int connfd);
int checkDirDelete(char* project, char* hash);
void lock_project(char* project_name, ProjectLock* headLock, int argument);
void unlock_project(char* project, ProjectLock* headLock, int argument);
ProjectLock* insertLock(char* project, ProjectLock* head);

#endif