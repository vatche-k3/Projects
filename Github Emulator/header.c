#include "header.h"
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int count = 0;
ProjectLock* headLock = NULL;
DIR* dir2;

void err_n_die(const char *fmt, ...){
    int errno_save;
    va_list ap;
    errno_save = errno;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if(errno_save != 0){
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);

    exit(1);
}

void killSignal(){
    printf("Server connection has been terminated\n");
    exit(1);
}

int argComp(char* string){
    if(strcmp(string, "create") == 0){
        return 1;
    }
    else if(strcmp(string, "destroy") == 0){
        return 2;
    }
    else if(strcmp(string, "update") == 0){
        return 3;
    }
    else if(strcmp(string, "currentversion") == 0){
        return 4;
    }
    else if(strcmp(string, "upgrade") == 0){
        return 5;
    }
    else if(strcmp(string, "checkout") == 0){
        return 6;
    }
    else if(strcmp(string, "commit") == 0){
        return 7;
    }
    else if(strcmp(string, "push") == 0){
        return 8;
    }
    else if(strcmp(string, "history") == 0){
        return 9;
    }
    else if(strcmp(string, "rollback") == 0){
        return 10;
    }
    return 0;
}

void lock_project(char* project_name, ProjectLock* headLock, int argument) {
    ProjectLock* curr = headLock;
    while(curr != NULL) {
        if(strcmp(curr->project_name, project_name) == 0) {
            int mutexCheck = pthread_mutex_lock(&(curr->lock));
            return;
        }
        curr = curr->next;
    }

    return;
}

void unlock_project(char* project_name, ProjectLock* headLock, int argument) {
    //if(argument == 5) return;
    ProjectLock* curr = headLock;
    while(curr != NULL) {
        if(strcmp(curr->project_name, project_name) == 0) {
            pthread_mutex_unlock(&(curr->lock));
            break;
        }
        curr = curr->next;
    }
    if(argument == 2){
        int err_num = pthread_mutex_destroy(&(curr->lock));
    }
}

ProjectLock* insertLock(char* project, ProjectLock* head){
    ProjectLock* ptr;
    
    if(head == NULL){
        ptr = malloc(sizeof(ProjectLock));
        ptr->project_name = malloc(strlen(project) + 1);
        memset(ptr->project_name, 0, strlen(project) + 1);
        sprintf(ptr->project_name, "%s", project);
        pthread_mutex_init(&(ptr->lock), NULL);
        head = ptr;
        return head;
    }
    ProjectLock* temp = head;

    if(strcmp(head->project_name, project) == 0){
        printf("The project already exists\n");
        return head;
    }

    while(temp->next != NULL){
        if(strcmp(temp->project_name, project) == 0){
            printf("The project already exists\n");
            return head;
        }
        temp = temp->next;
    }
    
    ptr = malloc(sizeof(ProjectLock));
    ptr->project_name = malloc(strlen(project) + 1);
    memset(ptr->project_name, 0, strlen(project) + 1);
    sprintf(ptr->project_name, "%s", project);
    pthread_mutex_init(&(ptr->lock), NULL);
    temp->next = ptr;
    return head;
}

void* readCommand(void* ptr){
//void readCommand(int connfd){
    connection_t* connfd;
    if(!ptr) pthread_exit(0);
    connfd = (connection_t *)ptr;
    int n;
    int check = 0;
    char text[100];
    int index = 0;
    int i = 0;
    int j = 0;
    char path[1000];
    int versionNumber = 1;
    char* command;
    char* project;
    char* fileName;
    char* fileSize;
    char* version;
    struct node* head = NULL;
    //char* command;
    //char* project;
    char c;
    char path2[1000];
    char path3[100];
    char* hash;
    char hashPush[32];
    DIR* dir;
    
    int lockCheck = 0;
    char project2[100];
    //char* projectLock;

    while(connfd->sock != -1){
        //n = read(connfd, &c, 1);
        //memset(project2, '\0', strlen(project2));
        n = recv(connfd->sock, &c, 1, 0);
        if(n < 1){
            printf("value of connfd 2: %d\n", connfd->sock);
            err_n_die("Recv error %d\n", n);
        }
        if(c == '$'){
            n = 0;
        }
        if((n == 0) || (isspace(c) != 0)){
            if((n == 0) && (isspace(c)!=0)){
                break;
            }
            if(check == 0){
                command = malloc(sizeof(char)*(index+1));
                for(i = 0; i <= index; i++){
                    if(i == index){
                        command[index] = '\0';
                    }
                    else{
                        command[i] = text[i];
                    }
                }
            }
            else if (check > 0){
                project = malloc(sizeof(char)*(index+1));
                for(i = 0; i <= index; i++){
                    if(i == index){
                        project[index] = '\0';
                    }
                    else{
                        project[i] = text[i];
                    }
                }
                
                if(lockCheck == 0){
                    headLock = insertLock(project, headLock);
                    lock_project(project, headLock, argComp(command));
                    
                }


                if(argComp(command) == 1){ // create
                    create(versionNumber, path, project, connfd->sock);
                    printf("Create done\n");
                    break;
                }
                else if(argComp(command) == 2){ // destroy
                    
                    
                    strcpy(path, "Repository/");
                    strcat(path, project); // concatenate project name to path
                    int r = destroy(path);
                    if(r != -1) write(connfd->sock, "$", 2);
                    unlock_project(project, headLock, argComp(command));
                    printf("Destroy done\n");
                }
                else if(argComp(command) == 3){ // update
                    lockCheck = 1;
                    if(check == 1){
                        strcpy(path, "Repository/");
                        strcat(path, project);
                        strcpy(project2, project);
                    }
                    else if(check == 2){
                        fileName = malloc(sizeof(char)*(index+1));
                        for(i = 0; i <= index; i++){
                            if(i == index){
                                fileName[index] = '\0';
                            }
                            else{
                                fileName[i] = text[i];
                            }
                        }
                    }
                    else if(check == 3){
                        
                        fileSize = malloc(sizeof(char)*(index+1));
                        for(i = 0; i <= index; i++){
                            if(i == index){
                                fileSize[index] = '\0';
                            }
                            else{
                                fileSize[i] = text[i];
                            }
                        }
                         // concatenate project name to path
                        //dir = opendir(path);
                        //if(dir){
                            // directory exists
                            //closedir(dir);
                            strcat(path, "/.Manifest");
                            head = insertValues(head, path);
                        //}
                        //else{
                            //printf("Error: Directory does not exist\n");
                            //return;
                        //}
                    }
                }
                else if(argComp(command) == 4){ // currentversion
                    strcpy(path, "Repository/");
                    strcat(path, project); // concatenate project name to path
                    //dir = opendir(path);
                    //if(dir){
                        // directory exists
                        strcat(path, "/.Manifest");
                        struct node* curHead = NULL;
                        //closedir(dir);
                        
                        curHead = insertValues(curHead, path);
                        
                        currentVersion(curHead->next, connfd->sock);
                        
                    //}
                    //else{
                        //printf("Error: Directory does not exist\n");
                        //return;
                    //}
                    printf("Current Version done\n");
                }
                else if(argComp(command) == 5){ // upgrade
                    strcpy(path, "Repository/");
                    //strcat(path, project); // concatenate project name to path
                    dir = opendir(path);
                    if(dir){
                        // directory exists
                        closedir(dir);
                        upgrade(path, connfd->sock, project); 
                    }
                    else{
                        
                        printf("Error: Directory does not exist\n");
                        return;
                    }
                    printf("Upgrade done\n");
                }
                else if(argComp(command) == 6){ // checkout
                    strcpy(path, "Repository/");
                    strcat(path, project);
                    dir = opendir(path);
                    if(dir){
                        // directory exists
                        struct node* curHead = NULL;
                        closedir(dir);
                        strcat(path, "/.Manifest");

                        write(connfd->sock, "noErr", 6);
                        curHead = insertValues(curHead, path);
                        strcpy(path, "Repository/");
                        checkout(path, connfd->sock, curHead->next, project); 
                    }
                    else{
                        write(connfd->sock, "Error", 6);
                        err_n_die("Directory does not exist\n"); // check
                    }
                    printf("Checkout done\n");
                }
                else if(argComp(command) == 7){ // commit
                    strcpy(path, "Repository/");
                    strcat(path, project);
                    //dir = opendir(path);

                    struct stat check;

                    if(stat(path, &check) == 0){
                    
                    //if(dir){
                        // directory exists
                        //closedir(dir);
                        strcat(path, "/.Manifest");
                        memset(hashPush, '\0', 32);
                        commit(path, connfd->sock, project);
                        
                    }
                    else{
                        write(connfd->sock, "Error", 6);
                        printf("Directory does not exist\n"); // check
                        return;
                    }
                    printf("Commit done\n");
                }
                else if(argComp(command) == 8){ // push
                    strcpy(path, "Repository/");
                    strcat(path, project);
                    dir = opendir(path);
                    
                    if(dir != NULL){
                        // directory exists
                        dir2 = dir;
                        //if(closedir(dir2))
                            //printf("Hello 2.\n");
                        
                        strcpy(path3, "Repository/");
                        strcat(path3, project);
                        push(path, path3, connfd->sock, project);
                    }
                    else{
                        write(connfd->sock, "Error", 6);
                        err_n_die("Directory does not exist\n"); // check
                    }
                    printf("Push done\n");
                }
                else if(argComp(command) == 9){ // history

                    strcpy(path, "Repository/");
                    strcat(path, project);
                    //dir = opendir(path);
                    //if(dir){
                        // directory exists
                        int fd = 0;
                        //closedir(dir);
                        strcat(path, "/.History");
                        fd = open(path, O_RDONLY);
                        if(fd == -1){
                            write(connfd->sock, "Error", 6);
                            close(fd);
                            err_n_die("Directory does not exist\n"); // check
                        }
                        close(fd);
                        history(path, connfd->sock);

                   // }
                    //else{
                        //write(connfd->sock, "Error", 6);
                        //err_n_die("Directory does not exist\n"); // check
                    //}
                    printf("History done\n");

                }
                else if(argComp(command) == 10){ // rollback
                    lockCheck = 1;
                    if(check == 1){
                        strcpy(project2, project);
                    }
                    else if(check == 2){
                        version = malloc(sizeof(char)*(index+1));
                        for(i = 0; i <= index; i++){
                            if(i == index){
                                version[index] = '\0';
                            }
                            else{
                                version[i] = text[i];
                            }
                        }

                        rollback(project2, version, connfd->sock);

                        unlock_project(project2, headLock, argComp(command));
                        memset(project2, '\0', strlen(project2));
                        printf("Rollback done\n");
                        return;
                    }


                }
                else{
                    err_n_die("Error: does not create.\n");
                }
            }
            index = 0;
            check++;
            if(n == 0) break;
        }
        else{
            text[index] = c;
            index++;
        }
        j++;
    }
    check = 0;

    if(argComp(command) == 3){
        int fs = atoi(fileSize);
        if(fs == 0){
            write(connfd->sock, "?", 2);
            return;
        }
        strcat(path, "2");
        int fd2 = open(path, O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 0600);
        char cTwo[fs];
        struct node* head2 = NULL;
        
        read(connfd->sock, cTwo, fs);
        write(fd2, cTwo, fs);
        close(fd2);

        int countingNode = 0;
        struct node* vvv = head;
        while(vvv != NULL){
            countingNode++;
            vvv = vvv->next;
        }

        
        head2 = insertValues(head2, path);

        int u = update(head, head2, connfd->sock, countingNode, project2);
        if(u != 1){
            remove(path);
        }

        unlock_project(project2, headLock, argComp(command));
        memset(project2, '\0', strlen(project2));

        printf("Update done\n");
        return;
        
    }

    if(n < 0){
            err_n_die("read error.");
    }
    unlock_project(project, headLock, argComp(command));
    memset(project, '\0', strlen(project));
    return;
}

void create(int versionNumber, char* path, char* project, int connfd){
    /*The server creates a project based on the project name given.*/
    struct stat st = {0};
    char buffer[3];
    int fd, sz, sender;
    struct stat fileStat;
    char fileSize[256];
    if (stat("Repository", &st) == -1){ // create repository if not created already
        mkdir("Repository", 0700);
    }
    strcpy(path, "Repository/");
    strcat(path, project); // concatenate project name to path
    strcat(path, "/");

    if (stat(path, &st) == -1){
        mkdir(path, 0700); // create project folder in repository
    }
    else{
        printf("This directory has already been created\n");
        return;
    }
    sprintf(buffer, "%d", versionNumber);
    //printf("Buffer: %s", buffer);
    
    strcat(path, ".Manifest");
    fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0600); // create manifest file in given project.
    if(fd == -1){
        printf("open error.");
        return;
    }
    sz = write(fd, buffer, strlen(buffer));
    versionNumber++;
    
    if(fstat(fd, &fileStat) < 0){
        printf("Error: file has not stats.\n");
        return;
    }
    sprintf(fileSize, "%d", fileStat.st_size);
    
    sender = send(connfd, fileSize, sizeof(fileSize), 0);
    
    if(sender < 0){
        printf("send error.\n");
        return;
    }
    close(fd);
    fd = open(path, O_RDWR | O_APPEND | O_CREAT, 0600);
    int i = 0;
    int reader = 0;
    char c;
    char ready[10];
    //while(i < 7){
    if(read(connfd, ready, 6)){
        reader = read(fd, &c, 1);
        
        sender = send(connfd, &c, sizeof(c), 0);
        
    }
    close(fd);

    strcpy(path, "Repository/");
    strcat(path, project); // concatenate project name to path
    strcat(path, "/");
    strcat(path, ".History");

    int fd2 = open(path, O_WRONLY | O_APPEND | O_CREAT, 0600);
    write(fd2, "1", 2);

    close(fd2);
}

int destroy(char* path){
    /* The server traverses through the project given and destroys it and all its contents.*/
    int pathLength = strlen(path);
    int r = -1;
    DIR *directory = opendir(path);

    if(directory){
        struct dirent *dir;
        r = 0;
        
        while(!r && (dir = readdir(directory))){
            int r2 = -1;
            char* buffer;
            int length = 0;;

            if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")){
                continue;
            }
            length = pathLength + strlen(dir->d_name) + 2;
            buffer = malloc(length);
            if(buffer){
                struct stat statbuf;
                snprintf(buffer, length, "%s/%s", path, dir->d_name);
                if(!stat(buffer, &statbuf)){
                    if(S_ISDIR(statbuf.st_mode)){
                        r2 = destroy(buffer);
                    }
                    else{
                        r2 = unlink(buffer);
                    }
                }
                //free(buffer);
            }
            r = r2;
        }
        closedir(directory);
    }
    if(!r){
        r = rmdir(path);
    }

    return r;
}

struct node* insertValues(struct node* head, char* path){
    /*First open the .Manifest in the server repository.
    - compare the server's and client's .Manifest version numbers.
        - if they are equal, return and do nothing.
        - if they are not the same, something has to change.
            - break up the file, where each node in a list is a line from the file.
    */
    //head = NULL;
    char* vNum;
    char* file;
    char* hash;
    char vNumA[100];
    char fileA[PATH_MAX+1];
    char hashA[32];
    char fileSize[100];
    int fd, reader;
    char c;
    int counter = 0;
    fd = open(path, O_RDONLY, 0600);
    int i = 0;
    int j = 0;
    int last = 0;
    struct stat file_stat;
    
    
    if (fstat(fd, &file_stat) < 0)
    {
        printf("could not get file size\n");
        return;
    }
    sprintf(fileSize, "%d", file_stat.st_size);
    
    while( (reader = read(fd, &c, 1)) != -1){
        last = last + 1;
        if(c == ' '){
            if(counter == 0){
                memset(vNumA + i,'\0',sizeof(vNumA)- i);
                
                vNum = malloc(i);
                
                for(j = 0; j < i; j++){
                    vNum[j] = vNumA[j];
                }
                vNum[i] = '\0';
                memset(vNumA, '\0', i);
                
            }
            else if(counter == 1){
                memset(fileA + i,'\0',sizeof(fileA) - i);
                i++;
                file = malloc(i);
                for(j = 0; j < i; j++){
                    file[j] = fileA[j];
                }
                file[i] = '\0';
                memset(fileA, '\0', i);
            }
            else if(counter == 2){
                i++;
                hash = malloc(i);
                for(j = 0; j < i; j++){
                    hash[j] = hashA[j];
                }
                memset(hashA, '\0', i);
            }
            counter++;
            i = 0;
        }
        else if(c == '\n' || (last == file_stat.st_size + 1)){
            if(counter == 0){
                memset(vNumA + i,'\0', sizeof(vNumA) - i);
                
                vNum = malloc(i);
                
                for(j = 0; j < i; j++){
                    vNum[j] = vNumA[j];
                }
                vNum[i] = '\0';
                memset(vNumA, '\0', i);
            }
            else if(counter == 1){
                memset(fileA + i,'\0', sizeof(fileA) - i);
                i++;
                file = malloc(i);
                for(j = 0; j < i; j++){
                    file[j] = fileA[j];
                }
                file[i] = '\0';
                memset(fileA, '\0', i);
            }
            else if(counter == 2){
                i++;
                hash = malloc(i);
                for(j = 0; j < i; j++){
                    hash[j] = hashA[j];
                }
                memset(hashA, '\0', i);
            }
            counter = 0;
            i = 0;
            head = makeList(head, vNum, file, hash);
            if(last == file_stat.st_size + 1) break;
        }
        else if(counter == 0){
            vNumA[i] = c;

            i++;
        }
        else if(counter == 1){
            fileA[i] = c;
            i++;
        }
        else if(counter == 2){
            hashA[i] = c;
            i++;
        }
    }

    memset(vNumA, '\0', sizeof(vNumA));
    memset(fileA, '\0', sizeof(fileA));
    memset(hashA, '\0', sizeof(hashA));
    count = 0;
    close(fd);
    return head;
}

struct node* makeList(struct node* head, char* vNum, char* file, char* hash){
    if (head == NULL) {
        count++;
        head = (struct node*)malloc(sizeof(struct node*));
        if(vNum != NULL){

            head->versionNumber = vNum;
        }
       
        head->next = NULL;
        return head;
    }
    struct node* pt = head;
    if(count == 1){
        pt->next = NULL;
    }
    while (pt->next != NULL) {
        pt = pt->next;
    } 
    count++;
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    newNode->versionNumber = vNum;
    newNode->name = file;
    newNode->hashcode = hash;
    newNode->next = NULL;
    pt->next = newNode;
    return head;
}

int update(struct node* head, struct node* head2, int connfd, int countingNode, char* project){
    /* Compares the server's and client's .Manifest to see if any changes have to be made.*/
    int i = 0;
    struct node* ptr = head; // this is the list of the manifest from the server.
    struct node* ptr2 = head2; // this is the list of the manifest from the client.
    char modHash[32];
    char buffer[100];
    char buffer2[100];
    //char checker[2];
    struct stat fileStatM;
    struct stat fileStatC;
    int check = 0;
    int check2 = 0;
    char path1[100];
    char path2[100];
    strcpy(path1, "Repository/");
    strcat(path1, project);
    strcat(path1, "/.Update");

    strcpy(path2, "Repository/");
    strcat(path2, project);
    strcat(path2, "/.Conflict");
    int fd = open(path1, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0600);
    int fd2 = open(path2, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0600);

    if(ptr->next!= NULL){ 
        ptr = ptr->next;
    }
    else{
        check2 = 1;
    }
    if(ptr2->next != NULL){ 
        ptr2 = ptr2->next;
    }
    else{
        check2 = 1;
    }

    if(strcmp(head->versionNumber, head2->versionNumber) == 0){
        printf("Changes do not have to be made\n");
        check = 1;
        write(connfd, "$", 2);
        return 1;
        //close(connfd);
    }
    else{
        while(ptr != NULL){ // Loop for add.
            if(countingNode == 1){
                break;
            }
            while(ptr2 != NULL){
                // if the file name from the client list is equal to the file name in the server list, stop. 
                // delete does not have to be performed.
                if(ptr2->name == NULL){
                    ptr2 = ptr2->next;
                }
                else if(strcmp(ptr2->name, ptr->name) == 0){
                    //ptr = head;
                    i = 1;
                    //memset(ptr2->name + strlen(ptr2->name), '\0', strlen(ptr2->name));
                    if((strcmp(ptr->versionNumber, ptr2->versionNumber) !=0) && (strcmp(ptr->hashcode, ptr2->hashcode) != 0)){
                        write(connfd, ptr2->name, strlen(ptr2->name));
                        read(connfd, modHash, 32);

                        if(strcmp(ptr2->hashcode, modHash) == 0){ // have to modify if the hashes are equal.
                            write(fd, "M ", 2);
                            write(fd, ptr2->name, strlen(ptr2->name));
                            write(fd, " ", 1);
                            write(fd, ptr2->hashcode, strlen(ptr2->hashcode));
                            write(fd, "\n", 1);
                        }
                        else if(strcmp(ptr2->hashcode, modHash) != 0){ // If they are not equal, there is a conflict.;
                            write(fd2, "C ", 2);
                            write(fd2, ptr2->name, strlen(ptr2->name));
                            write(fd2, " ", 1);
                            write(fd2, modHash, strlen(modHash));
                            write(fd2, "\n", 1);
                        }
                    }
                    //write(connfd, "1", 1);
                    break;
                }
                else{
                    ptr2 = ptr2->next;
                } 
            }
            if(i != 1){ // file was not found.
                write(fd, "A ", 2);
                write(fd, ptr->name, strlen(ptr->name));
                write(fd, " ", 1);
                write(fd, ptr->hashcode, strlen(ptr->hashcode));
                write(fd, "\n", 1);
            }
            else{
                i = 0;
            }
            ptr2 = head2->next;
            ptr = ptr->next;
        }

        ptr = head->next;
        ptr2 = head2->next;
        while(ptr2 != NULL){ // Loop for delete.
            while(ptr != NULL){
                // if the file name from the client list is equal to the file name in the server list, stop. 
                // Add does not have to be performed.
                if(countingNode == 1){
                    break;
                }
                else if(strcmp(ptr->name, ptr2->name) == 0){
                    //ptr = head;
                    i = 1;
                    break;
                }
                else{
                    ptr = ptr->next;
                } 
            }
            if(i != 1){ // file was not found.

                write(fd, "D ", 2);
                write(fd, ptr2->name, strlen(ptr2->name));
                write(fd, " ", 1);
                int n = write(fd, ptr2->hashcode, strlen(ptr2->hashcode));


                write(fd, "\n", 1);
            }
            else{
                i = 0;
            }
            ptr = head->next;
            ptr2 = ptr2->next;
        }
    }

    write(connfd, "1", 2);
    memset(modHash, '\0', strlen(modHash));

    close(fd);
    close(fd2);
    
    fd = open(path1, O_RDONLY, 0);
    fd2 = open(path2, O_RDONLY, 0);
    
    char ready[10];

    if(fstat(fd, &fileStatM) < 0){
        printf("could not get a file size\n");
        return;
    }
    
    if(fstat(fd2, &fileStatC) < 0){
        printf("could not get a file size\n");
        return;
    }

    if(fileStatC.st_size != 0){
        write(connfd, "#", 2);
        close(fd);
        remove(".Update");
        sprintf(buffer2, "%d", fileStatC.st_size);
        if(read(connfd, ready, 6)){
            write(connfd, buffer2, fileStatC.st_size);
        }

        char c[fileStatC.st_size];
        read(fd2, &c, fileStatC.st_size);
        if(read(connfd, ready, 6)){
            send(connfd, &c, fileStatC.st_size, 0);
        }
    }
    else{
        sprintf(buffer, "%d", fileStatM.st_size);
        write(connfd, buffer, fileStatM.st_size);

        char m[fileStatM.st_size];
        char one[1];
        read(fd, &m, fileStatM.st_size);
        if(read(connfd, one, 2)){
            send(connfd, &m, fileStatM.st_size, 0);
        }
        close(fd);
    }

    close(fd2);

    return 0;
    
    
}

void currentVersion(struct node* head, int connfd){
    /* The server simply sends the current version of the project to the client and the version numbers 
    of all the files in the .Manifest*/
    struct node* ptr = head;
    char ready[10];
    int n = 0;
    int check = 0;

    if(ptr == NULL) check = 1;

    while(ptr != NULL){
        if(read(connfd, ready, 6)) n = write(connfd, ptr->versionNumber, strlen(ptr->versionNumber));
        if(read(connfd, ready, 6)) n = write(connfd, ptr->name, strlen(ptr->name));
        ptr = ptr->next;
    }
    // Once ptr is null, send a signal to let the client know
    if(check == 1){
        write(connfd, "?", 2);
        return;
    }
    n = write(connfd, "$", 2);

    return;
}

void upgrade(char* path, int connfd, char* project){
    /* The server sends the .Update file or .Conflict file if there is one and the client applies the changes listed*/
    char buffer[100];
    char tempPath[100];
    char fileSize[100];
    char ready[10];
    int fd = 0;
    char check[2];
    struct stat fileStat;

    memset(tempPath, '\0', 100);
    memset(fileSize, '\0', 100);
    memset(buffer, '\0', 100);

    while(1){
        write(connfd, "ready", 6);
        int n = read(connfd, buffer, 100);
        if(n == 0 || n == -1){
            printf("Client had a conflict file or the update file is empty\n");
            break;
        } 
        if(strcmp(buffer, "$") == 0) break; // connection handler
        strcpy(tempPath, path);
        //strcat(tempPath, "/");
        strcat(tempPath, buffer);
        fd = open(tempPath, O_RDONLY, 0);
        if(!(fd)) break;
        if(fstat(fd, &fileStat) < 0){
            printf("Error: file has not stats.\n");
            //return;
        }
        sprintf(fileSize, "%d", fileStat.st_size);
        write(connfd, fileSize, strlen(fileSize));

        char c[fileStat.st_size];
        read(fd, &c, fileStat.st_size);
        if(read(connfd, ready, 6)){
            send(connfd, &c, fileStat.st_size, 0);
        }

        memset(tempPath, '\0', 100);
        memset(fileSize, '\0', 100);
        memset(buffer, '\0', 100);

        close(fd);
    }

    strcpy(path, "Repository/");
    strcat(path, project);
    strcat(path, "/.Manifest");

    fd = open(path, O_RDONLY, 0);
    char vs[10];
    memset(vs, '\0', 10);
    char c;
    int i = 0;

    while(read(fd, &c, 1)){
        if(c == '\n') break;
        vs[i] = c;
        i++;
    }

    //write(connfd, "$", 2);
    if(read(connfd, ready, 6)){
        write(connfd, vs, strlen(vs));
    }

    read(connfd, check, 2);
    if(strcmp(check, "$") != 0){
        printf("Unsuccessful upgrade\n");
        return;
    }

    printf("Upgrade successful\n");

    close(fd);
    return;
}

void checkout(char* path, int connfd, struct node* curHead, char* project){

    /* Send over all of the files to the client for its current version*/
    char tempPath[100];
    char fileSize[100];
    char ready[10]; // connection handler
    int fd = 0;
    int check = 0;
    struct stat fileStat;
    struct node* ptr = curHead;

    memset(tempPath, '\0', 100);
    memset(fileSize, '\0', 100);

    while(ptr != NULL){ // loops through a list of all the files in the .Manifest using a linked list
        if(read(connfd, ready, 6)){
            write(connfd, ptr->name, strlen(ptr->name));
        }
        strcpy(tempPath, path);
        strcat(tempPath, ptr->name);
        fd = open(tempPath, O_RDONLY, 0);
        if(!(fd)) break;
        if(fstat(fd, &fileStat) < 0){
            printf("Error: file has not stats.\n");
            return;
        }
        sprintf(fileSize, "%d", fileStat.st_size);
        if(read(connfd, ready, 6)){
            write(connfd, fileSize, strlen(fileSize)); // size of the file is sent to the client.
        }

        char c[fileStat.st_size];
        read(fd, &c, fileStat.st_size);
        if(read(connfd, ready, 6)){
            send(connfd, &c, fileStat.st_size, 0); // then the contents of the file are sent.
        }

        memset(tempPath, '\0', 100);
        memset(fileSize, '\0', 100);

        close(fd);
        ptr = ptr->next;
    }
    write(connfd, "$", 2);

    strcpy(tempPath, project);
    strcat(tempPath, "/.Manifest");

    if(read(connfd, ready, 6)){
        write(connfd, tempPath, strlen(tempPath));
    }

    strcpy(tempPath, path);
    strcat(tempPath, project);
    strcat(tempPath, "/.Manifest");

    fd = open(tempPath, O_RDONLY, 0);

    if(fstat(fd, &fileStat) < 0){
        printf("Error: file has not stats.\n");
        return;
    }
    sprintf(fileSize, "%d", fileStat.st_size);
    
    if(read(connfd, ready, 6)){
        write(connfd, fileSize, strlen(fileSize)); // send the size of the .Manifest itself
    }

    char c[fileStat.st_size];
    read(fd, &c, fileStat.st_size);
    if(read(connfd, ready, 6)){
        send(connfd, &c, fileStat.st_size, 0); // send the contents of the .Manifest.
    }

    memset(tempPath, '\0', 100);
    memset(fileSize, '\0', 100);
    close(fd);
    return;
}

void commit(char* path, int connfd, char* project){
    /* Client creates a commit file and sends it to the server. The server sends all the contents of its .Manifest*/
    char ready[10];
    char buffer[2];
    char buffer2[2];
    char fileSize[1000];
    char connect[13];
    char fileSize2[BUFSIZ];
    char hasher[MD5_DIGEST_LENGTH];

    struct stat fileStat;

    write(connfd, "$", 2);

    int fd = open(path, O_RDONLY, 0);

    if(fstat(fd, &fileStat) < 0){
        printf("Error: file has not stats.\n");
        return;
    }
    sprintf(fileSize, "%d", fileStat.st_size);
    if(read(connfd, ready, 6)){
        write(connfd, fileSize, strlen(fileSize));
    }

    char c[fileStat.st_size];
    read(fd, &c, fileStat.st_size);
    if(read(connfd, ready, 6)){
        send(connfd, &c, fileStat.st_size, 0);
    }

    if(read(connfd, &buffer, 2)){
        if(strcmp(buffer, "0") == 0){
            printf("Commit failed\n");
            return;
        }
        else if(strcmp(buffer, "1") == 0){
            printf("Commit successful\n");
        }
    }
    close(fd);

    read(connfd, &connect, 13);

    strcpy(path, "Repository/");
    strcat(path, project);
    strcat(path, "/.Commit");
    strcat(path, "_");
    strcat(path, connect);

    int fd2 = open(path, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0600);
    int fs = 0;

    write(connfd, "1", 2);

    if(read(connfd, fileSize2, BUFSIZ)){
        fs = atoi(fileSize2);
    }



    char commit[fs];
    bzero(commit, fs);

    //if(write(connfd, "1", 2)){
        read(connfd, commit, fs);
    //}

    memset(commit + fs, '\0', sizeof(commit) - fs);

    int n = write(fd2, &commit, fs-1);
    close(fd2);

    /********************/

    unsigned char* hashcode1 = malloc(MD5_DIGEST_LENGTH);
    
    hashcode1 = hash(project, path); // commit file from push

    int i;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++){
        sprintf(hasher+(i*2), "%02x", hashcode1[i]);
    }

    //char hasherTemp[MD5_DIGEST_LENGTH];
    char* hasherTemp = malloc(MD5_DIGEST_LENGTH);
    strcpy(hasherTemp, hasher);

    memset(hasher + 32,'\0', strlen(hasher) - 32);

    char path2[1000];
    strcpy(path2, "Repository/");
    strcat(path2, project);
    strcat(path2, "/.Commit_");
    strcat(path2, hasherTemp);

    rename(path, path2);

    /********************/

    return;
}
int checkD = 0;
void push(char* path, char* path2, int connfd, char* project){
    /*The server updates the new projects directory if the .Commit file that the client sends 
    is the same as the .Commit file that the server has in its directory.*/

    
    char fileSize[BUFSIZ];
    char connect[13];
    char hasher[MD5_DIGEST_LENGTH];
    char hasher2[MD5_DIGEST_LENGTH];
    char pathTemp[1000];
    char pathTemp2[1000];
    char pathTemp3[1000];
    char pathH[1000];
    char history[5];
    char c;
    int check = 0;
    
    write(connfd, "$", 2);

    read(connfd, &connect, 13);

    strcat(path, "/.Commit");
    strcat(path, "_");
    strcat(path, connect);
    strcat(path, "_");
    strcat(path, "push");




    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0600);
    int fs = 0;

    write(connfd, "1", 2);
    int checkFileSize = read(connfd, fileSize, BUFSIZ);
    fs = atoi(fileSize);
    
    if(fs == 0){
        printf("This is the fileSize: %d\n", fs);
        printf("could not get file size\n");
        write(connfd, "?", 2);
        close(fd);
        remove(path);
        return;
    }

    char commit[fs];
    bzero(commit, fs);
    if(write(connfd, "1", 2)){
        read(connfd, commit, fs);
    }

    memset(commit + fs, '\0', sizeof(commit) - fs);

    int n = write(fd, &commit, fs-1);

    close(fd);

    unsigned char* hashcode1 = malloc(MD5_DIGEST_LENGTH);
    
    hashcode1 = hash(project, path); // commit file from push

    int i;
    for(i = 0; i < MD5_DIGEST_LENGTH; i++){
        sprintf(hasher+(i*2), "%02x", hashcode1[i]);
    }

    char hasherTemp[MD5_DIGEST_LENGTH];
    strcpy(hasherTemp, hasher);

    memset(hasher + 32,'\0', strlen(hasher) - 32);

    /*******************************************/

    char pathHash[1000];
    strcpy(pathHash, "Repository/");
    strcat(pathHash, project);
    strcat(pathHash, "/.Commit_");
    strcat(pathHash, hasherTemp);
    strcat(pathHash, "_push");

    rename(path, pathHash);
    
    int z = checkDirDelete(project, hasherTemp);
    checkD++;

    if(z == 0){
        write(connfd, "Error", 6);
        return;
    }

    write(connfd, "$", 2);

    /*******************************************/
    //Rename the current project name.

    strcpy(pathH, "Repository/");
    strcat(pathH, project);
    strcat(pathH, "/.History");
    int fh = open(pathH, O_RDONLY, 0);
    int h = 0;
    while(read(fh, &c, 1)){
        if(c == '\n'){
            memset(history, '\0', strlen(history));
            h = 0;
        }
        else{
            history[h] = c;
            h++;
        }
    }
    close(fh);

    char delete1[1000];
    char delete2[1000];
    bzero(delete1, 1000);
    bzero(delete2, 1000);
    strcpy(delete1, "Repository/");
    strcat(delete1, history);
    strcat(delete1, "_");
    strcat(delete1, project);
    strcat(delete1, "/.Commit_");
    strcat(delete1, hasher);
    
    strcpy(delete2, "Repository/");
    strcat(delete2, history);
    strcat(delete2, "_");
    strcat(delete2, project);
    strcat(delete2, "/.Commit_");
    strcat(delete2, hasher);
    strcat(delete2, "_push");


    strcpy(pathHash, "Repository/");
    strcat(pathHash, history);
    strcat(pathHash, "_");
    strcat(pathHash, project);
    strcat(pathHash, "/.Commit_");
    strcat(pathHash, hasher);
    strcat(pathHash, "_push");

    strcpy(path, "Repository/");
    strcat(path, project);

    

    strcpy(pathTemp, "Repository/");
    strcat(pathTemp, history);
    strcat(pathTemp, "_");
    strcat(pathTemp, project);

    rename(path, pathTemp);

    memset(connect, '\0', 13);



    if(write(connfd, "ready", 6)){
        read(connfd, &connect, 13);
    }

    /******************************************/

    struct stat st = {0};

    strcpy(pathTemp2, "Repository/");
    strcat(pathTemp2, project); // concatenate project name to path
    strcat(pathTemp2, "/");
    if (stat(pathTemp2, &st) == -1){
        mkdir(pathTemp2, 0700); // create project folder in repository
    }
    strcat(pathTemp2, ".Manifest");
    int fd2 = open(pathTemp2, O_RDWR | O_APPEND | O_CREAT | O_TRUNC, 0600);


    strcpy(pathTemp3, "Repository/");
    strcat(pathTemp3, history);
    strcat(pathTemp3, "_");
    strcat(pathTemp3, project);
    strcat(pathTemp3, "/.Manifest");
    int fd3 = open(pathTemp3, O_RDONLY, 0);
    char versionNumber[10];
    
    int a = 0;

    while(read(fd3, &c, 1)){
        if(c == '\n') break;
        versionNumber[a] = c;
        a++;
    }

    int vs = atoi(versionNumber);
    vs = vs+1;
    memset(versionNumber, '\0', strlen(versionNumber));
    sprintf(versionNumber, "%d", vs);

    write(fd2, versionNumber, strlen(versionNumber));

    close(fd2);
    close(fd3);

    /*********************************/

    struct stat file_stat;
    int reader = 0;
    char token;
    int counter = 0;
    int bIndx = 0;
    int memCom = 0;
    int memFile = 0;
    int memHash = 0;
    int j = 0;
    char command[3];
    char fileName[FILENAME_MAX];
    char hash[100];
    char buffer[BUFSIZ];
    char dump[10];
    char pathPush[1000];
    int size = 0;
    char fileNameTemp[FILENAME_MAX];
    
    i = 0;

    fd = open(pathHash, O_RDONLY, 0);
    if (fd == -1) {
        remove(path);
        printf("No update file. Client must perform command update first.\n");
        return;
    }
    if (fstat(fd, &file_stat) < 0){
        remove(path);
        printf("could not get file size\n");
        return;
    }
    if (file_stat.st_size == 0){
        remove(path);
        printf("The file is empty\n");
        return;
    }
    while(i <= file_stat.st_size){
        reader = read(fd, &token, 1);
        if(reader==0||isspace(token)!=0){
            if(reader == 0 && isspace(token)!=0){
                break;
            }
            else if(counter == 0){
                for(j = 0; j < bIndx; j++){
                    command[j]=buffer[j];
                }
                memCom = bIndx;
                counter ++;
            }
            else if(counter == 1){
                for(j = 0; j < bIndx; j++){
                    fileName[j]=buffer[j];
                }
                memFile = bIndx;
                counter++;
            }
            else if(counter == 2){
                for(j = 0; j < bIndx; j++){
                    hash[j]=buffer[j];
                }
                memHash = bIndx;
                memset(command + memCom, '\0', sizeof(command)-memCom);
                memset(fileName + memFile, '\0', sizeof(fileName)-memFile);
                memset(hash + memHash, '\0', sizeof(hash)-memHash);
                //printf("bIndx: %d\n",bIndx );
                if(strcmp(command,"D")==0){

                }
                else if((strcmp(command,"A")==0) ||(strcmp(command,"M")==0)){;
                    bzero(fileNameTemp, sizeof(fileNameTemp));
                    strcpy(fileNameTemp, "Repository/");
                    strcat(fileNameTemp, fileName);
                    recv(connfd, dump, 6,0);
                    write(connfd, fileName, strlen(fileName));
                    reader = recv(connfd, buffer, BUFSIZ, 0);
                    memset(buffer + reader, '\0', BUFSIZ-reader);
                    size = atoi(buffer);
                    fd2 = open(fileNameTemp, O_WRONLY | O_TRUNC, 0600);
                    if (fd2 == -1){
                        fd3 = open(fileNameTemp, O_WRONLY | O_CREAT, 0600);
                        if(fd3==-1){
                            char* relPath = malloc(PATH_MAX);
                            bzero(pathPush, sizeof(pathPush));
                            strcpy(pathPush, fileNameTemp);
                            relPath = dirname(pathPush);
                            _mkdir(relPath);
                            int fd4 = open(fileNameTemp, O_WRONLY | O_CREAT | O_APPEND| O_TRUNC, 0600);
                            if(fd4 == -1){
                                printf("failed to create file and path\n");
                                return;
                            }
                            char wr[size];
                            bzero(wr, size);
                            write(connfd, "ready", 6);
                            read(connfd, wr , size);
                            write(fd4, wr, size);
                            add(project, fileNameTemp, fileName);
                            close(fd4);
                        }
                        else{
                            char wr[size];
                            bzero(wr, size);
                            write(connfd, "ready", 6);
                            read(connfd, wr , size);
                            write(fd3, wr, size);
                            add(project, fileNameTemp, fileName);
                            close(fd3);
                        }
                    }
                    else{
                        char wr[size];
                        bzero(wr, size);
                        write(connfd, "ready", 6);
                        read(connfd, wr , size);
                        write(fd2, wr, size);
                        add(project, fileNameTemp, fileName);
                        close(fd2);
                    }
                }
                counter++;
                counter = 0;
            }
            bzero(buffer, BUFSIZ);
            bIndx = 0;
        }
        else{
            buffer[bIndx]= token;
            bIndx++;
        }
        i++;
    }
    close(fd);

    write(connfd, "$", 2);
    
    remove(delete1);
    remove(delete2);

    //checkDirDelete(delete3, "!@#$");

    h = 0;
    char pathH2[1000];
    strcpy(pathH2, "Repository/");
    strcat(pathH2, history);
    strcat(pathH2, "_");
    strcat(pathH2, project);
    strcat(pathH2, "/.History");


    int fh2 = open(pathH2, O_RDONLY, 0);
    int fh3 = open(pathH, O_WRONLY | O_APPEND | O_CREAT, 0600);
    struct stat fileHistory;
    char fileSizeH[1000];


    if (fstat(fh2, &fileHistory) < 0){
        printf("could not get file size\n");
        return;
    }
    sprintf(fileSizeH, "%d", fileHistory.st_size);
    read(fh2, fileSizeH, fileHistory.st_size);

    h = atoi(history);
    h = h+1;
    bzero(history, 10);
    sprintf(history, "%d", h);

    write(fh3, fileSizeH, strlen(fileSizeH)); // change made here.
    write(fh3, "\n", 1);
    write(fh3, history, strlen(history));
    
    close(fh2);
    close(fh3);

    return;    
}
unsigned char* hash(char* project, char* path){
    //unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char* c = malloc(MD5_DIGEST_LENGTH);
    int i = 0;
    char file_size[256];
    struct stat file_stat;
    MD5_CTX mdContext;
    int bytes;
    char token[1000];
    int fd = open (path, O_RDONLY, 0);
    if (fd == -1) {
        printf ("%s can't be opened to hash. File does not exist in project.\n", path);
        exit(-1);
    }

    //find size of file before hash 
     if (fstat(fd, &file_stat) < 0)
        {
                printf("could not get file size\n");
                exit(EXIT_FAILURE);
        }
        
        char data[file_stat.st_size];
        MD5_Init (&mdContext);
        while ((bytes = read(fd, &data, file_stat.st_size )) != 0)
        {
            //printf("bytes : %d\n", bytes);
            //printf("string: %s\n", data);
           MD5_Update (&mdContext, data, bytes);
        }
        MD5_Final (c,&mdContext);
    
    close(fd);
    //printf("c: %s\n", c);
    return c;
    
}

void _mkdir(const char *dir) {
        char temp[256];
        char *ptr = NULL;
        size_t len;
 
        snprintf(temp, sizeof(temp),"%s",dir);
        len = strlen(temp);
        if(temp[len - 1] == '/')
                temp[len - 1] = 0;
        for(ptr = temp + 1; *ptr; ptr++)
                if(*ptr == '/') {
                        *ptr = 0;
                        mkdir(temp, S_IRWXU);
                        *ptr = '/';
                }
        mkdir(temp, S_IRWXU);
}

void add(char* project, char* file_name, char* fileNameInsert)
{

    int fd, i, writer, removed;
    int* version = NULL;
    char* sendFile;
    char* aPath = malloc(PATH_MAX);
    char path[1000];
    char path2[1000];
    char checkHash[1000];
    char file_path[1000];
    char file_size[256];
    char actualpath [PATH_MAX + 1];
    struct stat file_stat;
    unsigned char* hashcode = NULL;
    char hashdump[MD5_DIGEST_LENGTH];

    sendFile = malloc(strlen(file_name+1));
    sendFile = basename(file_name);

    bzero(path, 1000);
    strcpy(path, "Repository/");
    strcat(path, project);
    strcat(path, "/");
    strcat(path, ".Manifest");

    strcpy(aPath, file_name);
    //printf("pathname: %s\n", file_path);
    //aPath = recursive(project, file_name);
    //realpath(file_path, actualpath);
    if(aPath == NULL)
    {
       printf("file does not exist in directory cannot hash and add to manifest\n");
       exit(0);
    }
    //printf("aPath: %s\n", aPath);
    bzero(path2, 1000);
    strcpy(path2, "Repository/");
    strcat(path2, project);
    int check  = checkMan(aPath, path2);
    fd = open(path, O_WRONLY | O_APPEND, 0600 );
    if(fd == -1)
    {
        printf("Failed to open project manifest. Project may not exist\n");
        return;
    }

    if (fstat(fd, &file_stat) < 0)
        {
                printf("could not get file size\n");
                return;
        }
    if(check == 1)
    {
        hashcode = malloc(MD5_DIGEST_LENGTH);
        hashcode = hash(project, aPath);

        
        for(i = 0; i < 2 * MD5_DIGEST_LENGTH; i++)
        {
            sprintf(hashdump+(i*2), "%02x", hashcode[i]);
        } 

        strcpy(checkHash, aPath);
        strcat(checkHash, " ");
        strcat(checkHash, hashdump);

        int check  = checkMan(checkHash, project);
        if(check == 1)
        {
            printf("file name already exists with same content\n");
            close(fd);
        }
        else{
            close(fd);
            char ver[1000];
            version = remove_line(project, aPath);
            *version = *version + 1;  
            sprintf(ver , "%d",*version);

            removed = open(path, O_WRONLY | O_APPEND, 0600 );
            if(removed == -1)
            {
                printf("Failed to open Removed\n");
                return;
            }
            writer = write(removed, "\n", 1);
            writer = write(removed, ver, 1);
            writer = write(removed, " ", 1);
            writer = write(removed, fileNameInsert, strlen(fileNameInsert));
            writer = write(removed, " ", 1);
            writer = write(removed, hashdump, 2 * MD5_DIGEST_LENGTH);
        }
        close(removed);
    }
    else{
        hashcode = malloc(MD5_DIGEST_LENGTH);
        writer = write(fd, "\n", 1);
        writer = write(fd, "1 ", 2);
        writer = write(fd, fileNameInsert, strlen(fileNameInsert));
        hashcode = hash(project, aPath);
        
        for(i = 0; i < 2 * MD5_DIGEST_LENGTH; i++)
        {
            sprintf(hashdump+(i*2), "%02x", hashcode[i]);
        } 
        
        writer = write(fd, " ", 1);
        writer = write(fd, hashdump, 2 * MD5_DIGEST_LENGTH);
        close(fd);
        
    }
}

int checkMan(char* file_name, char* project)
{
    //printf("file: %s\n", file_name);
    char path[1000];
    int fd, reader;
    char buffer[1000];
    char token;
    int buf_indx = 0;

    strcpy(path, project);
    strcat(path, "/");
    strcat(path, ".Manifest");

    fd = open(path, O_RDONLY, 0 );
    if(fd == -1)
    {
        printf("Failed to open project manifest. Project may not exist\n");
        return;
    }

    while(fd != -1)
    {
        reader = read(fd, &token, 1);
        //printf("token: %c\n", token);
        if(token== '\n' || reader == 0){
            buf_indx= buf_indx + 1;
            buffer [buf_indx] = '\0';
            memset(buffer+buf_indx, '\0', sizeof(buffer)-buf_indx);
            //printf("buffer: %s\n", buffer);
            //printf("buffer: %d\n", strlen(buffer));
            if(reader == 0 &&(isspace(token)!=0)) break;
            else if(strstr(buffer,file_name) == NULL)//not found
            {
                
            }
            else//found
            {
                close(fd);
                return 1;
            }

            if(reader == 0)
            {
                break;
            }

            bzero(buffer, buf_indx);
            buf_indx = 0;
        }
        else
        {
            buffer[buf_indx] = token;
            //printf("char: %c\n", buffer[buf_indx]);
            buf_indx = buf_indx + 1;
        }
    }

    close(fd);
    return 0;

    //close(fd);
}
int checkFile(char *basePath)
{
    int nBytes, fd;
    char token;
    int entry_file, write_file;
    fd = open(basePath, O_RDONLY,0);
    //check file descriptor
    if(fd == -1){
        printf("Failed to open and read file");
        return 0;
    }
    return 1;
}

int* remove_line(char* project, char* file_name)
{
    char path[1000];
    char file_path[1000];
    int fd, temp, reader, writer;
    int *removed = malloc(sizeof(int));
    char buffer[1000];
    char token;
    char *ptr;
    int line = 0;
    int buf_indx = 0;
    int check = 0;

    strcpy(path, project);
    strcat(path, "/");
    strcat(path, ".Manifest");

    strcpy(file_path, project);
    strcat(file_path, "/");
    strcat(file_path, "tempManifest");

    fd = open(path, O_RDONLY, 0 );
    if(fd == -1)
    {
        printf("Failed to open project manifest. Project may not exist\n");
        return;
    }

    temp = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0600);
     if(temp == -1)
    {
        printf("Failed to open project manifest. Project may not exist\n");
        return;
    }
    while(fd != -1)
    {
        reader = read(fd, &token, 1);
        //printf("token %c\n", token);
        if(token== '\n' || reader == 0){
            if(reader == 0 &&(isspace(token)!=0)) break;
            else if((strstr(buffer,file_name) == NULL) || line == 0)//not present write
            {
                if(check != 0){
                    writer = write(temp, "\n", 1 );
                }
                writer = write(temp, buffer, buf_indx);
                //printf("token %s\n", buffer);
                check++;
                line ++;
            }
            else//present dont write
            {
                ptr = &buffer[0]; 
                int place = atoi(ptr);
                *removed = place;
            }

            if(reader == 0)
            {
                break;
            }

            bzero(buffer, buf_indx);
            buf_indx = 0;
        }
        else
        {
            buffer[buf_indx] = token;
            //printf("token %c\n", token);
            //printf("buffer %s\n", buffer);
            buf_indx = buf_indx + 1;
        }
    }
    close(fd);
    close(temp);
    remove(path);
    rename(file_path, path);

    if(removed == 0)
    {
        printf("did not remove\n");
        return removed;
    }
    else
    {
        //printf("value: %d\n", *removed);
        return removed;
    }

}

void history(char* path, int connfd){
    /* The server sends the .History file of the project to the client*/
    char ready[10];
    struct stat fileStat;
    char fileSize[100];

    printf("in history 0\n");

    write(connfd, "$", 2);
    printf("in history 1\n");
    int fd = open(path, O_RDONLY, 0);
    printf("in history 2\n");
    if(fstat(fd, &fileStat) < 0){
        printf("Error: file has not stats.\n");
        return;
    }
    printf("in history 3\n");
    sprintf(fileSize, "%d", fileStat.st_size);
    printf("in history 4\n");

    write(connfd, fileSize, fileStat.st_size);

    printf("in history 5\n");

    char buffer[fileStat.st_size];
    read(fd, &buffer, fileStat.st_size);
    printf("in history 6\n");
    close(fd);
    printf("in history 7\n");

    read(connfd, ready, 6);

    printf("in history 8\n");

    if(strcmp(ready, "ready") == 0){
        write(connfd, &buffer, fileStat.st_size);
    }
    else{
        printf("Error on the client side\n");
        return;
    }   

    return;
}

void rollback(char* project, char* version, int connfd){
    /* The server reverts to a version that the client provides if valid*/
    char path[1000];
    char path2[1000];
    char history[10];
    char c;
    int versionGiven = 0;
    int versionHistory = 0;
    DIR* dir;
    struct stat fileStat;
    char fileSize[10];
    struct stat check;
    strcpy(path, "Repository/");
    strcat(path, project);


    //dir = opendir(path);
    if(stat(path, &check) != 0){ // Check if the project name exists.
        write(connfd, "err1", 5);
        printf("There is an error\n");
        return;
    }
    //closedir(dir);
    

    strcat(path, "/.History");

    int fh = open(path, O_RDONLY, 0);

    if(fstat(fh, &fileStat) < 0){
        printf("Error: file has not stats.\n");
        return;
    }
    sprintf(fileSize, "%d", fileStat.st_size);


    char historyFile[fileStat.st_size];
    read(fh, historyFile, fileStat.st_size);

    close(fh);

    fh = open(path, O_RDONLY, 0);

    int h = 0;
    while(read(fh, &c, 1)){
        if(c == '\n'){
            memset(history, '\0', strlen(history));
            h = 0;
        }
        else{
            history[h] = c;
            h++;
        }
    }
    close(fh);

    versionGiven = atoi(version);
    versionHistory = atoi(history);

    if(versionGiven >= versionHistory || versionGiven == 0){
        printf("Invalid version number: %d\n", versionGiven);
        write(connfd, "err2", 5);
        return;
    }

    strcpy(path, "Repository/");
    strcat(path, project);
    destroy(path);
    versionHistory--;

    int i;
    for(i = versionHistory; i > versionGiven; i--){
        strcpy(path, "Repository/");
        memset(history, '\0', 10);
        sprintf(history, "%d", i);
        strcat(path, history);
        strcat(path, "_");
        strcat(path, project);
        destroy(path);
    }

    strcpy(path, "Repository/");
    memset(history, '\0', 10);
    sprintf(history, "%d", i);
    strcat(path, history);
    strcat(path, "_");
    strcat(path, project);


    strcpy(path2, "Repository/");
    strcat(path2, project);


    rename(path, path2);

    strcat(path2, "/.History");

    fh = open(path2, O_WRONLY | O_TRUNC, 0600);

    write(fh, historyFile, fileStat.st_size);
    write(fh, "\n", 1);
    write(fh, version, strlen(version));

    close(fh);

    write(connfd, "$", 2);

    return;
}
int checkDirDelete(char* project, char* hash){
    int z = 0;
    DIR *dir;
    struct dirent *directory;
    char path[1000];
    char delete[1000];
    char* r;
    char* s;
    bzero(path, 1000);
    strcpy(path, "Repository/");
    strcat(path, project);
    memset(path+strlen(path), '\0', sizeof(path)-strlen(path));
    if ((dir = opendir (path)) != NULL) {
        if(checkD == 0){
            dir = dir2;
        }
        while ((directory = readdir(dir)) != NULL) {
            r = strstr(directory->d_name, ".Commit");
            s = strstr(directory->d_name, hash);
            if(r != NULL){
                if(s == NULL){
                    bzero(delete, 1000);
                    strcpy(delete, "Repository/");
                    strcat(delete, project);
                    strcat(delete, "/");
                    strcat(delete, directory->d_name);
                    remove(delete);
                    memset(delete+strlen(delete), '\0', sizeof(delete)-strlen(delete));
                }
                else{
                    z = 1;
                }
            }
        }
        //closedir(dir);
    } 
    else {
        printf("Directory does not exist\n");
        return;
    }
    return z;
}