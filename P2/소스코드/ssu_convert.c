#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "parseOpt.h"
#include "convert.h"
#define SECOND_TO_MICRO 1000000

void ssu_routin(struct timeval *begin_t, struct timeval *end_t)
{
    end_t->tv_sec -= begin_t->tv_sec; //초 차이 계산
    if (end_t->tv_usec < begin_t->tv_usec)
    { //시작과 끝의 micro sec 차가 음수가 되지 않게 만들기 위한 조건문,
        end_t->tv_sec--;
        end_t->tv_usec += SECOND_TO_MICRO;
    }
    end_t->tv_usec -= begin_t->tv_usec; //micro sec 차이 계산
    printf("Run time : %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}

int main(int argc, char *argv[])
{
    char a;                        //option
    char javaFile[1024];           //java file name
    int optnum;                    //option number for array
    struct timeval begin_t, end_t; //variable for time

    gettimeofday(&begin_t, NULL); //time at start

    init(); //initialize globla variation

    if (argc < 2)
    {
        fprintf(stderr, "usage : %s <FILENAME> [OPTION]\n", argv[0]);
        exit(1);
    }
    memset(javaFile, 0, sizeof(javaFile)); //initialize javaFile
    strcat(javaFile, argv[1]);             //copy java file name

    while ((a = getopt(argc, argv, optParseString)) != -1) //option parsing
    {
        optnum = getOptFlag(a);  //get option number
        if (optnum != -1)        //if unkwon option is -1
            optFlag[optnum] = 1; //set option flag
        if (optnum == -1)        //error exception
        {
            fprintf(stderr, "option error for %c\n", a);
            exit(0);
        }
    }

    convertFunc(javaFile); //java file convert to c file

    gettimeofday(&end_t, NULL);   //time at end
    ssu_routin(&begin_t, &end_t); //print runtime
}
