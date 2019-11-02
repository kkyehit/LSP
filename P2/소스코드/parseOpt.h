#include <sys/wait.h>
#define OPTION_CNT 6
char *optParseString = "jcpflr";
int optFlag[OPTION_CNT];

/*get option number*/
int getOptFlag(char a)
{
    switch (a)
    {
    case 'j': //j -- 0
        return 0;
    case 'c': //c -- 1
        return 1;
    case 'p': //p -- 2
        return 2;
    case 'f': //f -- 3
        return 3;
    case 'l': //l -- 4
        return 4;
    case 'r': //r -- 5
        return 5;
    default: // else -- -1
        return -1;
    }
}

/*option --- j */
void opt_j(char *_javaBuf)
{
    char *nextLine;
    int i = 1;
    if (_javaBuf == NULL)
        return;

    nextLine = strtok(_javaBuf, "\n"); //tokenization for new line
    if (nextLine == NULL)
        return;
    do
    {
        printf("%d %s\n", i++, nextLine);              //print line number and contents;
    } while ((nextLine = strtok(NULL, "\n")) != NULL); //tokenization for new line
}
/*option --- c */
void opt_c(char *_cFileName)
{
    char nextLine[1024];
    char writeBuffer[1024];
    char FileName[1024];
    char c;
    int line = 1; //line number
    int fd;       //file descripter
    if (_cFileName == NULL)
        return;

    memset(nextLine, 0, sizeof(nextLine));       //initialize nextLine
    memset(writeBuffer, 0, sizeof(writeBuffer)); //initialize writeBuffer
    memset(FileName, 0, sizeof(FileName));       //initialize FileName

    sprintf(FileName, "%s.c", _cFileName);   //make c file name
    if ((fd = open(FileName, O_RDONLY)) < 0) //file open error cheack
    {
        fprintf(stderr, "open errpr for %s\n", FileName);
        return;
    }
    while (read(fd, &c, 1) > 0) //read character
    {
        nextLine[strlen(nextLine)] = c; //add character at string
        if (c == '\n')
        {
            sprintf(writeBuffer, "%d %s", line++, nextLine); //make string (line number contents)
            write(1, writeBuffer, strlen(writeBuffer));      //wrtie writeBuffer at stdout
            memset(nextLine, 0, sizeof(nextLine));
        }
    }
}
/*option --- l */

void opt_l(char *FileName)
{
    int lineCnt = 0;
    int fd = open(FileName, O_RDONLY); //open
    char c;
    if (fd < 0)
    {
        fprintf(stderr, "open error for(%s)\n", FileName);
        return;
    }
    while (read(fd, &c, 1) > 0) //read onr character
    {
        if (c == '\n') //if chracter is new line
            lineCnt++; //plus linCnt 1
    }
    close(fd);                                                 //close
    printf("%s line number is %d lines\n", FileName, lineCnt); //print line cnt
}

/*option --- f */
void opt_f(char *FileName)
{
    off_t sizeOfFile = 0;
    int fd = open(FileName, O_RDONLY); //open
    char c;
    if (fd < 0)
    {
        fprintf(stderr, "open error for(%s)\n", FileName);
        return;
    }
    if ((sizeOfFile = lseek(fd, 0, SEEK_END)) < 0) //get last offset
    {
        fprintf(stderr, "lseek error for(%s)\n", FileName);
        return;
    }
    close(fd);                                                   //close
    printf("%s file size is %ld bytes\n", FileName, sizeOfFile); //print file size
}
/*option --- r */
void opt_r(char *_javaName, char *_cName, char *_javaBuf, char *_cBuf)
{
    int pid = fork(); //make children process
    int status;       //for error cheak
    if (pid == 0)     //if children process
    {
        char *nextLine;
        int i = 1;       //init line number
        system("clear"); //clear terminal
        printf("%s is converting...\n", _javaName);
        printf("----------\n%s\n----------\n", _javaName);
        nextLine = strtok(_javaBuf, "\n"); //tokenizer for new line
        if (nextLine == NULL)              //if nextLine is NULL
            exit(0);                       //error
        do
        {
            printf("%d %s\n", i++, nextLine);              //print line number and contents
        } while ((nextLine = strtok(NULL, "\n")) != NULL); //tokenizer for new line

        printf("----------\n%s.c\n----------\n", _cName);
        nextLine = strtok(_cBuf, "\n"); //tokenizer for new line
        i = 1;                          //init line number
        if (nextLine == NULL)           //if nextLine is NULL
            exit(0);                    //error
        do
        {
            printf("%d %s\n", i++, nextLine);              //print line number and contents
        } while ((nextLine = strtok(NULL, "\n")) != NULL); //tokenizer for new line
        sleep(1);                                          //sleep 1sec to saw process
        exit(0);                                           //children process exit
    }
    else if (pid > 0) //if parents process
    {
        if (wait(&status) != pid) // wait children process
        {
            /*if wait return value is not children process, error occur*/
            fprintf(stderr, "fork() error;\n");
            exit(0);
        }
    }
    else if (pid < 0)
    {
        /*if pid < 0, error occur*/
        fprintf(stderr, "fork() error :");
        exit(0);
    }
}

/*option --- p */
void opt_p(char *content)
{
    char *nextLine;
    int i = 1;
    if (content == NULL) //if content is NULL
        return;          //return

    nextLine = strtok(content, "\n"); //tokenizer for new line
    if (nextLine == NULL)             //if nextLine is NULL
        return;                       //return
    do
    {
        printf("%d %s\n", i++, nextLine);              //print line number and contents
    } while ((nextLine = strtok(NULL, "\n")) != NULL); //tokenizer for new line
}