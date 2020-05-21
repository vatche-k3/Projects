// Christopher Nguyen ccn38 and Vatche Kafafian vak37
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<ctype.h>
extern int errno;
 
 
int checkWrite = 0;
int checkWrite2 = 0; // check for creating huffman codebook text file.
int numArg = 0;
int counter = 0; // count the number of nodes to insert into the minheap. 
int size = 0; // keep track of size in insert function.
struct node {
    int freq;
    char* token;
    struct node* next;
    struct node* left;
    struct node* right;
};
 
struct press{
    char bytes[1000];
    char token[1000];
    struct press* next;
};
int compressCompare(int sz, int fd3, struct press* ptr, char* string, int check){
    while(ptr != NULL){
        if(check == 1){
            //printf("Token: %s\n", ptr->token);
            //printf("Bytes: %s\n", ptr->bytes);
            //printf("Word: %s\n", string);
            //printf("********\n");
        }
        else if(check == 0){
            //printf("Token: %s\n", ptr->bytes);
            //printf("Bytes: %s\n", ptr->bytes);
            //printf("Word: %s\n", string);
            //printf("********\n");
        }
        //printf("Is it equal: %d\n", test);
        if(check == 1){ // coming from compress
            if(strcmp(string, ptr->token) == 0){
                sz = write(fd3, ptr->bytes, strlen(ptr->bytes));
                return 1;
                break;
            }
        }
        else if(check == 0){ // coming from decompress
            if(strcmp(string, ptr->bytes) == 0){
                sz = write(fd3, ptr->token, strlen(ptr->token));
                return 1;
                break;
            }
        }
        ptr = ptr->next;
    }
    return 0;
}
 
void decompress(char* fileH){
    int check= 0;  
    int fd1 = 0;
    int fd2 = 0;
    int fd3 = 0;
    int sz = 0;
    int result = 0;
    char file[100];
 
    int fileCheck = strlen(fileH); // put this into decompress
    if(fileH[fileCheck-1] != 'z'){
        //printf("%s is not an .hcz file, cannot decompress\n", fileH);
        return;
    }
    fd2 = open(fileH, O_RDWR);
    if(fd2 == -1)
    {
        printf("Failed to open and read file\n");
        return;
    }
    strcpy(file, fileH);
    int copy = 0;
    int length = strlen(file);
    for(copy = strlen(file); copy > length-5; copy--){
        //printf("Copy: %d\n", copy);
        file[copy] = '\0';
    }
    //printf("This is the file that you're looking for: %s\n", file);
    fd3 = open(file, O_RDWR | O_CREAT, 00700);
    memset(file, '0', sizeof(file));
    char a;
    int i = 0; // for bytes
    int j = 0; // for strings
    int k = 0; // for second file fd2
    char bitstring[1000];
    struct press* head = NULL;
    struct press* ptr = NULL;
    head = malloc(sizeof(struct press));
    ptr = head;
    if ((fd1 = open("HuffmanCodebook.txt", O_RDWR)) >= 0){
        while (read(fd1, &a, 1) == 1){
            //putchar(a);
            if(isdigit(a)){
                ptr->bytes[i] = a;
                i++;
            }
            else if(isalpha(a) || a == '<' || a == '>'){
                ptr->token[j] = a;
                j++;
            }
            else if(a == '\n'){
                memset(ptr->bytes+i, '\0', 900);
                i = 0;
                j = 0;
                if(strcmp("<SPC>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = ' ';
                }
                else if(strcmp("<TAB>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = '\t';
                }
                else if(strcmp("<LF>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = '\n';
                }
                ptr->next = malloc(sizeof(struct press));
                ptr = ptr->next;
            }
        }
    }
    /*if(fd1==-1){
        printf("errno: %s\n", strerror(errno));
    }*/
    if (fd2 >= 0){
        memset(bitstring, '\0', sizeof(bitstring));
        while (read(fd2, &a, 1) == 1){
            ptr = head;
            bitstring[k] = a;
            k++;
            result = compressCompare(sz, fd3, ptr, bitstring, check);
            if(result == 1){ // the byte was found.
                k = 0;
                memset(bitstring, '\0', sizeof(bitstring));
            }
        }
    }
    ptr = head;
    compressCompare(sz, fd3, ptr, bitstring, check);
    memset(file, '0', sizeof(file));
    free(head);
    head = NULL;
    close(fd1);
    close(fd2);
    close(fd3);
}
 
void compress(char* file){
    int check = 1;  
    int fd1 = 0;
    int fd2 = 0;
    int fd3 = 0;
    int sz = 0;
    char fileH[100];
    
    int fileCheck = strlen(file); // put this into compress
    if(file[fileCheck-1] == 'z'){
        //printf("%s is not an .txt file, cannot compress\n", file);
        return;
    }
    fd2 = open(file, O_RDWR);
    if(fd2 == -1)
    {
        printf("Failed to open and read file\n");
        return;
    }
    strcpy(fileH, file);
    strcat(fileH, ".hcz");
    //printf("test.txt.hcz: %s\n", fileH);
    fd3 = open(fileH, O_RDWR | O_CREAT, 00700);
    memset(fileH, '\0', sizeof(fileH));
    char a;
    int i = 0; // for bytes
    int j = 0; // for strings
    int k = 0; // for second file fd2
    char word[1000];
    int result = 0;
    struct press* head = NULL;
    struct press* ptr = NULL;
    head = malloc(sizeof(struct press));
    ptr = head;
    if ((fd1 = open("HuffmanCodebook.txt", O_RDWR)) >= 0){
        while (read(fd1, &a, 1) == 1){
            //putchar(a);
            if(isdigit(a)){
                ptr->bytes[i] = a;
                i++;
            }
            else if(isalpha(a) || a == '<' || a == '>'){
                ptr->token[j] = a;
                j++;
            }
            else if(a == '\n'){
                 memset(ptr->bytes+i, '\0', 900);
                i = 0;
                j = 0;
                if(strcmp("<SPC>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = ' ';
                }
                else if(strcmp("<TAB>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = '\t';
                    //printf("Value of this string: %d\n", ptr->token[0]);
                }
                else if(strcmp("<LF>", ptr->token) == 0){
                    memset(ptr->token, '\0', sizeof(ptr->token));
                    ptr->token[0] = '\n';
                }
                ptr->next = malloc(sizeof(struct press));
                ptr = ptr->next;
            }
        }
    }
    if (fd2 >= 0){
        memset(word, '\0', sizeof(word));
        while (read(fd2, &a, 1) == 1){
            ptr = head;
            word[k] = a;
            k++;
            result = compressCompare(sz, fd3, ptr, word, check);
            if(result == 1){ // the byte was found.
                k = 0;
                memset(word, '\0', sizeof(word));
            }
        }
    }
    ptr = head;
    compressCompare(sz, fd3, ptr, word, check);
    memset(fileH, '\0', sizeof(fileH));
    free(head);
    close(fd1);
    close(fd2);
    close(fd3);
}
 
void checkFile(char *basePath, int flag1, int flag2)
{
    int nBytes, fd;
    char token;
    int entry_file, write_file;
    fd = open(basePath, O_RDONLY,0);
    //check file descriptor
    if(fd == -1){
        printf("Failed to open and read file");
        return;
    }
    if(flag1 == 4 || flag2 == 4)//recursive decompress
    {
        decompress(basePath);
    }
    else if(flag1 == 3 || flag2 == 3)//recursive compress
    {
        compress(basePath);
    }
    else{
        int fileCheck = strlen(basePath); // put this into decompress
        if(basePath[fileCheck-1] == 'z'){
            //printf("%s is an .hcz file, cannot write\n", basePath);
            return;
        }
        if(checkWrite == 0)
        {
            write_file = open("writeFile.txt", O_CREAT | O_WRONLY | O_APPEND | O_TRUNC, 0600);
            checkWrite ++;
        }
        else
        {
            write_file = open("writeFile.txt", O_CREAT | O_WRONLY | O_APPEND, 0600);
        }
        if(write_file == -1){
            printf("Failed to create and write to write_file");
            return;
        }
        while(fd != -1)
        {
            nBytes = read(fd, &token, 1);
            if(nBytes == 0){
                token = '\n';
                write(write_file, &token, 1);
                break;
            }
            //printf("%c", token);
            write(write_file, &token, 1);
        }
    }
    close(write_file);
    close(fd);
}
void recursive(char *basePath, int checkFlag1, int checkFlag2)
{
    char path[1000];
    struct dirent* input_file;
    DIR* FD = opendir(basePath);
    if (NULL == (FD = opendir (basePath))) //open input directory
    {
       // fprintf(stderr, "Error : Not a dir - %s\n", strerror(errno));
        checkFile(basePath, checkFlag1, checkFlag2);
        return;
    }
    while ((input_file = readdir(FD)) != NULL)
    {
        if (strcmp(input_file->d_name, ".") != 0 && strcmp(input_file->d_name, "..") != 0)
        {
            //printf("file: %s\n", input_file->d_name);
            // Construct new path from our base path
            strcpy(path, basePath);
            strcat(path, "/");
            strcat(path, input_file->d_name);
            recursive(path, checkFlag1, checkFlag2);
        }
    }
    closedir(FD);
}
struct node* newNode(struct node* head, char* var) {
    if (head == NULL) {
        head = (struct node*)malloc(sizeof(struct node*));
        head->token = var;
        head->freq = 1;
        head->next = NULL;
        counter = counter + 1;
        return head;
    }
    struct node* pt = head;
    while (pt->next != NULL) {
        pt = pt->next;
    }
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    newNode->token = var;
    newNode->freq = 1;
    newNode->next = NULL;
    pt->next = newNode;
    counter = counter+1;
    return head;
}
void frequency(struct node *head) 
{ 
    struct node *curr, *ptr, *duplicate; 
    curr = head; 
  
    /* Pick elements one by one */
    while (curr != NULL && curr->next != NULL) 
    { 
        ptr = curr; 
        /* Compare the curr element with rest of the elements */
        while (ptr->next != NULL) 
        { 
            /* If duplicate then delete it */
            if (strcmp(curr->token, ptr->next->token)==0) 
            { 
                curr->freq= curr->freq + 1;
                duplicate = ptr->next; 
                ptr->next = ptr->next->next; 
                counter = counter - 1;
                free(duplicate); 
            } 
            else
            { 
                ptr = ptr->next;
            } 
        } 
        curr = curr->next; 
    } 
} 
int delimeters(char token){
     char* spaceArr = "<SPC>";
    if(token == ' '){
        return 1;
    }
    else if(token == '\n'){
        return 2;
    }
    else if(token == '\t'){
        return 3;
    }
    return;
}
int checkFlag(char* flag)
{
    if(strcmp(flag,"-b")==0)
    {
        return 1;
    }
    else if(strcmp(flag,"-r")==0)
    {
        return 2;
    }
    else if(strcmp(flag,"-c")==0)
    {
        return 3;
    }
    else if(strcmp(flag,"-d")==0)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}
struct node* buildList(int flagb)
{
    int fd, num_Bytes, i, loadIndx, delim;
    loadIndx = 0;
    char token;
    char* newArr;
    char* newDelim;
    char* file;
    struct node* listHead = NULL;
    char loadArr[1000];
    char* spaceArr = "<SPC>";
    char* newline = "<NEWLINE>";
     char* tab = "<TAB>";
    fd = open("writeFile.txt", O_RDONLY,0);//building file
    //check file descriptor
    if(fd == -1){
        printf("Failed to open and read file");
        exit(-1);
    }
     while(fd != -1)
    {
        num_Bytes = read(fd, &token, 1);
         if((num_Bytes == 0) || (isspace(token) != 0))
        {
            if((num_Bytes == 0) && (isspace(token)!=0)){
                break;
            }
            newArr = malloc(sizeof(char)*(loadIndx+1));
            /*if(loadArr[0] == '0'){
                loadArr[0] = ' ';
                loadIndx = loadIndx + 1;
            }*/
            if(isspace(token)!=0){
                newDelim = malloc(sizeof(char)*(6));
                delim = delimeters(token);
                if(delim == 1){
                    newDelim = spaceArr;
                }
                else if(delim == 2){
                    newDelim = newline;
                }
                else if(delim == 3){
                    newDelim = tab;
                }
            }
            for(i = 0; i < loadIndx; i++)
            {
                newArr[i] = loadArr[i];
            }
            newArr[loadIndx] = '\0';
            //create new node here set = to newArr
            if(newArr[0]!='\0'){
                listHead = newNode(listHead, newArr);
            }
            if(newDelim[0]!='\0' || newDelim != NULL){
                listHead = newNode(listHead, newDelim);
            }
            loadIndx = 0; //reset next word
            //stop at end of file
             if(num_Bytes == 0){
                break;
            }
        }
        else
        {
            loadArr[loadIndx] = token;
            loadIndx ++;
        }
    }
 
 
    frequency(listHead);
    //check list
   /* while(listHead!=NULL){
        printf("this is the list: %s\n", listHead->token);
        printf("this is frquency: %d\n", listHead->freq);
        listHead = listHead->next;
    }*/
    close(fd);
    return listHead;
}
void swap(struct node* a, struct node* b){
    struct node temp = *a;
    *a = *b;
    *b = temp;
}
void printCodeArray(struct node* head, int* codes, int hi){
    if(head->left){
        codes[hi] = 0;
        printCodeArray(head->left, codes, hi + 1);
    }
    if(head->right){
        codes[hi] = 1;
        printCodeArray(head->right, codes, hi + 1);
    }
 
 
    if(!(head->left) && !(head->right)){
        int sz;
        int i;
        char* code;
        int index = 0;
        int fd = 0;
        if(checkWrite2 == 0){
            fd = open("HuffmanCodebook.txt", O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0600);
            checkWrite2++;
        }
        else{
            fd = open("HuffmanCodebook.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
            checkWrite2++;
        }
        if(fd < 0){
            perror("r1");
            exit(1);
        }
        code = malloc(hi*sizeof(char));
        for(i = 0; i < hi; i++){
            //codes = codes[i];
            index += sprintf(&code[index], "%d", codes[i]);
            //sz = write(fd, &codes[i], sizeof(codes[i]));
        }
            /*if(checkWrite2 == 1){
                sz = write(fd, "\\", 1);
                sz = write(fd, "\n", 1);
            }*/
            sz = write(fd, code, strlen(code));
            sz = write(fd, "    ", 4);
            sz = write(fd, head->token, strlen(head->token));
            sz = write(fd, "\n", 1);
            //printf("%s\n", code);
            close(fd);
    }
}
 
 
void insertNewNode(struct node** array, struct node*hi, int capacity){
    capacity++;
    int ptr = capacity - 1;
 
 
    while(ptr && hi->freq < array[(ptr - 1) / 2]->freq){
        array[ptr] = array[(ptr - 1) / 2];
        ptr = (ptr - 1) / 2;
    }
    array[ptr] = hi;
}
 
 
struct node* mallocNewNode(int frequency){
    struct node* temp = malloc(sizeof(struct node));
    temp->left = NULL;
    temp->right = NULL;
    temp->freq = frequency;
 
 
    return temp;
}

void heapify(struct node** array, int index, int capacity){ // O(nlogn)
    int lo = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
 
 
    if(left < capacity && array[left]->freq < array[lo]->freq){
        lo = left;
    }
    if(right < capacity && array[right]->freq < array[lo]->freq){
        lo = right;
    }
    if(lo != index){
        swap(array[lo], array[index]);
        heapify(array, lo, capacity);
    }
}
 
 
struct node* heapMin(struct node** array, int capacity){
    struct node* temp = array[0];
    array[0] = array[capacity - 1];
    --capacity;
    heapify(array, 0, capacity);
 
 
    return temp;
 
 
}
 
 
struct node* huffmanHelper(struct node** array, int capacity){ 
    struct node *left, *right, *hi = NULL;
    
    while(capacity != 1){
        left = heapMin(array, capacity);
        capacity--;
        right = heapMin(array, capacity);
        capacity--;
 
 
        hi = mallocNewNode(left->freq + right->freq);
 
 
        hi->left = left;
        hi->right = right;
 
 
        insertNewNode(array, hi, capacity);
        capacity++;
    }
}
 
 
void huffmanTreeBuilder(struct node** array, int capacity){
    struct node* head = huffmanHelper(array, capacity);
    int* codes = malloc(capacity*(sizeof(int)));
    int hi = 0;
    printCodeArray(head, codes, hi);
}
 
 
void insert(struct node* head, struct node** array, int capacity){ // O(log n)
    struct node* ptr = NULL;
    struct node* temp = NULL;
    ptr = (struct node*) malloc(sizeof(struct node*)* 1000);
    if(size == capacity){
        return;
    }
    size++;
    int insert = size-1;
    
    ptr->freq = head->freq;
    ptr->token = head->token;
    array[insert] = ptr;
    
    while(insert != 0 && array[(insert-1)/2]->freq > array[insert]->freq){
        swap(array[insert], array[(insert-1)/2]);
        insert = (insert-1)/2;
    }
}
 
int main(int argc, char* argv[]){
    char* flag1;
    char* flag2;
    int i;
    struct node* listHead = NULL;
    if(argc < 3)//checl amount of Arguments
    {
        printf("Fatal Error: expected at least three arguments, had one\n");
        return EXIT_FAILURE;
    }
    else //check args
    {
        if(argc == 3)// just build single file only 1 flag build
        {
            numArg = 3;
            flag1 = malloc(sizeof(char)*(100));
            flag1 = argv[1];
            if(checkFlag(flag1) != 1){
                printf("incorrect input\n");
                return EXIT_FAILURE;
            }
            else{
                recursive(argv[2], 0, 0);
            }
        }
        else if(argc == 4)
        {
            numArg = 4;
            flag1 = malloc(sizeof(char)*(3));
            flag1 = argv[1];
            flag2 = malloc(sizeof(char)*(3));
            flag2 = argv[2];
            if(checkFlag(flag1)==0)
            {
                 printf("incorrect input\n");
                return EXIT_FAILURE;
            }
            else if(checkFlag(flag2)!=0)//flag not file
            {
                if((checkFlag(flag1) == 2 && checkFlag(flag2) == 1) || (checkFlag(flag1) == 1 && checkFlag(flag2) == 2)){
                 //recursive build
                 recursive(argv[3], 0, 0);
                }
                else
                {
                    printf("incorrect input\n");
                    return EXIT_FAILURE;
                }
            }
            else if(checkFlag(flag2) == 0)
            {
                if(checkFlag(flag1)== 4) //decompress single file
                {
                    //printf("decompress\n");
                    decompress(argv[2]);
                } 
                else if(checkFlag(flag1) == 3) //compress single file
                {
                    //printf("compress\n");
                    compress(argv[2]);
                } 
                else
                {
                    printf("incorrect input\n");
                    return EXIT_FAILURE;
                }
            }
            
        }
        else if(argc == 5)//can only recursively compress
        {
            flag1 = malloc(sizeof(char)*(100));
            flag1 = argv[1];
            flag2 = malloc(sizeof(char)*(100));
            flag2 = argv[2];
            numArg = 5; 
            if((checkFlag(flag1) == 2 && checkFlag(flag2) == 3) || (checkFlag(flag1) == 3 && checkFlag(flag2) == 2))
            {
                int checkFlag1 = checkFlag(flag1);
                int checkFlag2 = checkFlag(flag2);
                recursive(argv[3], checkFlag1, checkFlag2);
            }
            else if((checkFlag(flag1) == 2 && checkFlag(flag2) == 4) || (checkFlag(flag1) == 4 && checkFlag(flag2) == 2))
            {
                int checkFlag1 = checkFlag(flag1);
                int checkFlag2 = checkFlag(flag2);
                recursive(argv[3], checkFlag1, checkFlag2);
            }
            else
            {
                printf("can only recursively decompress with five correct args\n");
                return EXIT_FAILURE;
            }
        }
 
 
    }
//build list
    struct node** array = malloc(10000*(sizeof(struct node*))); // the minheap array,
    if(numArg == 3 || (numArg == 4 && (checkFlag(flag1)==1|| checkFlag(flag2)==1)))
    {
        listHead = buildList(1);
        /*while(listHead!=NULL){
            printf("this is the list: %s\n", listHead->token);
            printf("this is frquency: %d\n", listHead->freq);
            listHead = listHead->next;
        }
        */
        //struct node* ptr = listHead; // this is where the minheap is made.
        while(listHead != NULL){
            insert(listHead, array, counter);
            //printf("token: %s\n", listHead->token);
            listHead = listHead->next;
        }
 
 
        // this for loop prints each token and freqency in the minheap.
        /*for(i = 0; i < counter; i++){
            printf("%s: ", array[i]->token);
            printf("%d\n", array[i]->freq);
        }*/
        for(i = 0; i < counter; i++){
        array[i]->left = NULL;
        array[i]->right = NULL;
        array[i]->next = NULL;
    }
        huffmanTreeBuilder(array, counter);
        //printf("success\n");
    } 
    free(listHead);
    for(i = 0; i < counter; i++){
        free(array[i]);
    }
    free(array);
}
 

