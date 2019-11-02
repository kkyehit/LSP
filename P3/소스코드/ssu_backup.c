#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX_COMMMAND_VECTOR 20 //command의 인자로 받을 수 있는 최대 개수
#define MAX_BUFFER 1024        //버퍼의 크기
#ifndef MAX_FILE_NAME          //파일 이름의 최대 길이 정의
#define MAX_FILE_NAME 255
#endif           // !MAX_FILE_NAME
#ifndef DIR_MODE //디렉토리 권한
#define DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif
#define LOG_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //로그 파일 모드

#define LOG_FILE_NAME "log.txt" //로그 파일 이름

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#include "ssu_backup_list.h"    //링크드 리스트 구조체 및 함수 정의, list 명령 기능 정의
#include "ssu_backup_process.h" //백업 thread 정의
#include "ssu_backup_add.h"     //add 명령어 기능 정의
#include "ssu_backup_remove.h"  //remove 명령어 기능 정의
#include "ssu_backup_system.h"  //ls, vi, vim 명령 기능 정의
#include "ssu_backup_compare.h" //compare 명령어 기능 정의
#include "ssu_backup_recover.h" //recover 명령어 기능 정의

/*인자로 받을 수 있는 명령어 종류 및 개수*/
const char *COMMAND_LIST[] = {"add", "remove", "compare", "recover", "list", "ls", "vi",
                              "vim", "exit"};
const int COMMAND_LIST_LENGHT = 9;

/*받은 명령어에 따른 알맞을 기능을 호출 하는 함수*/
int process_command_func(char *commandVector[], int logfd, char *directory);

int main(int argc, char *argv[])
{
    char *directory, *logfile_path;               //백업 drectory이름 및 log 파일 경로 이름
    char *command;                                //명령어 전체를 가리는 포인터
    char *tmp;                                    //임시 포인터
    char *commandVector[MAX_COMMMAND_VECTOR + 1]; //명령어를 공백 단위로 구분한 배열
    char character;                               //입력된 문자열을 하나 씩 읽기 위한 변수
    int index = 0;                                //commandVector에 저장하기 위해 사용되는 변수
    int logfd = -1;                               //로그 파일 디스트립터
    struct stat statbuf;                          //stat 구조체

    /*너무 많은 인자가 입력된 경우*/
    if (argc > 2)
    {
        fprintf(stderr, "usage : %s <directory>\n", argv[0]);
        exit(1);
    }
    /*디렉토리 경로가 입력되지 않은 경우*/
    if (argc == 1)
    {
        /*디렉토리의 경로는 현제 작업 디렉토리 밑에 생성한다.*/
        directory = "./backup";
        /*디렉토리 생성.*/
        mkdir(directory, DIR_MODE);
    }
    /*디렉토리 경로가 입력된 경우*/
    else
    {
        /*디렉토리 이름에 새로운 공간 할당 마지막 문자 처리*/
        directory = (char *)calloc(sizeof(char), MAX_BUFFER); //새로운 공간 할당
        memset(directory, 0, sizeof(directory));              //0으로 초기화
        strcpy(directory, argv[1]);                           //디렉토리 목사
        if (directory[strlen(directory) - 1] != '/')          //마지막 문자가 /가 아니라면 추가
            strcat(directory, "/");                           // '/' 추가
        strcat(directory, "backup");                          //생성할 backup 디렉토리 이름 이어붙이기
        mkdir(directory, DIR_MODE);                           //디렉토리가 없을 경우를 대비하여 디렉토리 생성
    }
    if (directory[strlen(directory) - 1] == '/') //마지막 문자가 / 라면 지우기
    {
        directory[strlen(directory) - 1] = 0;
    }

    /*디렉토리가 접근 가능하고 존재하는지 확인 및 디렉토리 파일인지 확인*/
    if (lstat(directory, &statbuf) < 0) //stat 구조체에 파일 정보 저장
    {
        fprintf(stderr, "lstat error for %s\n", directory);
        exit(1);
    }
    if (access(directory, F_OK | X_OK) < 0 || !S_ISDIR(statbuf.st_mode)) //존재, 접근, 디렉토리 확인
    {
        fprintf(stderr, "Usage : %s <directory>\n", argv[0]);
        exit(1);
    }
    logfile_path = (char *)calloc(sizeof(char), MAX_BUFFER + 10); //로그 파일에 새 공간 할당.
    command = (char *)calloc(sizeof(char), MAX_BUFFER);           //command에 새 공간 할당

    /*로그 파일 경로 만들기*/
    strcpy(logfile_path, directory); //디렉토리 파일 뒤에
    if (logfile_path[strlen(directory) - 1] != '/')
    {
        logfile_path[strlen(directory)] = 0;
        strcat(logfile_path, "/");
    }
    strcat(logfile_path, LOG_FILE_NAME); //log 파일 이름 이어붙이기

    /*log 파일 open*/
    if ((logfd = open(logfile_path, O_RDWR | O_CREAT | O_APPEND, LOG_MODE)) < 0)
    {
        fprintf(stderr, "open error for %s\n", logfile_path);
        exit(1);
    }

    /*명령 입력 받기*/
    while (1)
    {
        /*command 명령어 초기화*/
        memset(command, 0, sizeof(command));
        /*stdout에 출력*/
        write(1, "20142300>", 10);
        fflush(stdin);
        /*한 문자씩 읽기*/
        while ((character = fgetc(stdin)) > 0)
        {
            /*개행을 만나면 종료*/
            if (character == '\n')
            {
                break;
            }
            /*command의 마지막에 문자 추가*/
            command[strlen(command) + 1] = 0;
            command[strlen(command)] = character;
            /*command의 버퍼 크기를 넘어서는 인자인 경우*/
            if (strlen(command) == MAX_BUFFER - 1)
            {
                fprintf(stderr, "21042300>too long\n");
                continue;
            }
        }
        /*character가 음수인 경우*/
        if (character <= 0)
        {
            write(1, "\n", 1); //개행 출력 후 while문 종료
            break;
        }
        /*command의 첫번 째 인자가 0 또는 개행인 경우 그대로 진행*/
        if (command[0] == 0 || command[0] == '\n')
        {
            continue;
        }
        index = 0;
        /*command를 공백 기준으로 토큰 분리*/
        tmp = strtok(command, " ");
        /*토큰 분리에 실패한 경우*/
        if (tmp == NULL)
        {
            fprintf(stderr, "21042300>command error\n");
            continue;
        }
        /*토큰 분리 과정*/
        do
        {
            commandVector[index++] = tmp;     //commandVector에 다음 토큰 추가
            if (index >= MAX_COMMMAND_VECTOR) //MAX_COMMMAND_VECTOR보다 많아지면 종료
                break;
        } while ((tmp = strtok(NULL, " ")) != NULL);
        commandVector[index] = NULL; //commnadvector의 마지막 인자는 NULL
        
        /*입력 처리 하는 함수의 반환값이 0이면 while 종료*/
        if (process_command_func(commandVector, logfd, directory) == -1)
            break;
    }
    printf("20142300>backup program exit.\n");
    /*할당된 공간 및 사용한 자원 반환*/
    free(command);
    close(logfd);
    pthread_mutex_destroy(&mutex);
}
/*입력에 대한 처리, exit인 경우 -1, 알 수 없는 명령어의 경우 0, 성공한 경우 1*/
int process_command_func(char *commandVector[], int logfd, char *directory)
{
    int i = 0;
    /*exit인 경우*/
    if (strcmp(commandVector[0], COMMAND_LIST[COMMAND_LIST_LENGHT - 1]) == 0)
    {
        char *commandRemove[3]; //작업중인 모든 thread 종료하는 msg 만들기
        commandRemove[0] = "remove";
        commandRemove[1] = "-a";
        commandRemove[2] = NULL;
        process_remove(commandRemove, logfd, directory); //기존에 backup중이던 thread 종료
        return -1;                                       //-1 리턴함으로써 종료 알림
    }
    /*exit가 아닌경우 경우*/
    for (i = 0; i < COMMAND_LIST_LENGHT - 1; i++)
    {
        if (strcmp(commandVector[0], COMMAND_LIST[i]) == 0)
        {
            if (i == 0) //add
            {
                process_add(commandVector, logfd, directory);
            }
            else if (i == 1) //remove
            {
                process_remove(commandVector, logfd, directory);
            }
            else if (i == 2) //compare
            {
                process_compare(commandVector);
            }
            else if (i == 3) //recover
            {
                process_recover(commandVector, logfd, directory);
            }
            else if (i == 4) //list
            {
                process_list(logfd);
            }
            else if (i == 5 || i == 6 || i == 7) //vi, vim, ls
            {
                process_fork_system(commandVector);
            }
            return 1;
        }
    }

    /*COMMAND_LIST에 포함된 명령어가 아닌 경우*/
    fprintf(stderr, "20142300>unkown command [%s]\n",commandVector[0]);
    return 0;
}