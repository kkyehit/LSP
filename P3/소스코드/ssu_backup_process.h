#ifndef LIST_DEF
#include "ssu_backup_list.h"
#endif
#ifndef DIR_MODE
#define DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif

#include <errno.h>

#define MAX_BACKUP_FILE 1024 //백업 파일의 경로 문자열의 최대 길이
#define MAX_LOG_MSG 1024     //로그 파일에 저장될 문자열의 최대 길이
#define MAX_OF_DIR 255       //디렉토리 경로의 최댓값

void process_opt_t(struct backup_struct *node, struct tm *tm_p); //option t가 적용 되었을 경우 수행될 함수
void process_opt_n(struct backup_struct *node);                  //option n이 적용 되었을 경우 실행 될 함수

/*thread가 수행할 함수*/
void *process_update_thread(void *arg)
{
    struct backup_struct *node = (struct backup_struct *)arg;
    int ori_fd, backup_fd;             //원본 파일 디스크립터, 백업 파일 디스크립터
    char backup_file[MAX_BACKUP_FILE]; //백업 파일 경로 저장
    char log_msg[MAX_LOG_MSG];         //로그 파일에 저장될 문자열 자정

    char character;       //파일에서 한 문자를 읽이 위한 변수
    char cwd[MAX_OF_DIR]; //현제 작업 디렉토리 저장

    struct tm *tm_p; //파일에 시간을 쓰기 위한 구초제
    time_t now;      //파일에 시간을 쓰기 위해 사용되는 변수

    struct stat statbuf; //파일의 속정을 저장 할 변수

    memset(backup_file, 0, MAX_BACKUP_FILE); //backup_file 초기화
    /*mutex를 이용한 동기화를 이용해 log 파일에 msg 저장*/
    pthread_mutex_lock(&mutex); //mutex lock
    time(&now);                 //현제 시간 구하기
    tm_p = localtime(&now);     //현제시간을 tm 구조체로 변환

    memset(log_msg, 0, MAX_LOG_MSG); //los_msg 초기화
    sprintf(log_msg, "[%02d%02d%02d %02d%02d%02d] %s added\n", ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec, node->ori_filename);
    write(node->logfd, log_msg, strlen(log_msg)); //log 파일에 msg 저장

    pthread_mutex_unlock(&mutex); //mutex unlock

    while (1)
    {
        time(&now);             //현제 시간 구하기
        tm_p = localtime(&now); //현제시간을 tm 구조체로 변환
        /*파일의 경로 만들기*/
        sprintf(backup_file, "%s%s_%02d%02d%02d%02d%02d%02d", node->directory, node->ori_filename, ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
        backup_file[strlen(backup_file)] = 0;

        /*원본 파일 open*/
        if ((ori_fd = open(node->ori_filename, O_RDONLY)) < 0)
        {
            fprintf(stderr, "process >[ERROR]open error for %s\n", node->ori_filename);
            removeNode(node); //error시 node 지우기
            return NULL;
        }
        /*원본 파일의 정보 얻기 */
        if (lstat(node->ori_filename, &statbuf) < 0)
        {
            fprintf(stderr, "process >[ERROR]lstat error for %s\n", node->ori_filename);
            removeNode(node); //error시 node 지우기
            return NULL;
        }
        /*-m 옵션 확인, 설정 돠어 있다면*/
        if (node->backup_opt & (1 << 0))
        {
            /*원본 파일의 mtiem과 node에 저장되어 있는 statbuf의 mtime 비교*/
            if (node->statbuf.st_mtime == statbuf.st_mtime)
            {
                /*-t 옵션이 설정되어 있다면*/
                if (node->backup_opt & (1 << 2))
                {
                    process_opt_t(node, tm_p);
                }
                /*mtime이 같다면 주어진 시간 만큼 대기 한 후 다시 진행*/
                close(ori_fd);
                sleep(node->backup_period);
                continue;
            }
            else
            {
                /*다르다면 mtime 업데이트 후 backup 수행*/
                node->statbuf.st_mtime = statbuf.st_mtime;
            }
        }
        if ((backup_fd = open(backup_file, O_RDWR | O_CREAT | O_TRUNC, statbuf.st_mode)) < 0)
        {
            fprintf(stderr, "process >[ERROR]open error for %s\n", backup_file);
            printf("ERROR : %s\n", strerror(errno));
            removeNode(node); //error시 node 지우기
            return NULL;
        }

        while (read(ori_fd, &character, 1) > 0)
        {
            write(backup_fd, &character, 1);
        }

        /*mutex를 이용한 동기화를 이용해 log 파일에 msg 저장*/
        pthread_mutex_lock(&mutex);      //mutex lock
        time(&now);                      //현제 시간 구하기
        tm_p = localtime(&now);          //현제시간을 tm 구조체로 변환
        memset(log_msg, 0, MAX_LOG_MSG); //los_msg 초기화
        sprintf(log_msg, "[%02d%02d%02d %02d%02d%02d] %s generated\n", ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec, backup_file);
        write(node->logfd, log_msg, strlen(log_msg)); //log 파일에 msg 저장
        pthread_mutex_unlock(&mutex);                 //mutex unlock

        /*file close*/
        close(backup_fd);
        close(ori_fd);

        /*-t 옵션이 설정되어 있다면*/
        if (node->backup_opt & (1 << 2))
        {
            process_opt_t(node, tm_p);
        }
        /*-n 옵션이 설정되어 있다면*/
        if (node->backup_opt & (1 << 1))
        {
            process_opt_n(node);
        }

        sleep(node->backup_period);
    }
    /*thread 끝나면 node remove*/
    removeNode(node);
}

void process_opt_t(struct backup_struct *node, struct tm *tm_p)
{
    char pathname[MAX_BACKUP_FILE];        //파일의 전체 경로를 저장할 변수
    char pathname_d_name[MAX_BACKUP_FILE]; //파일의 전체 경로를 저장할 변수
    char copy_d_name[MAX_BACKUP_FILE];     //파일의 이름을 저장할 변수
    char timestr[13];                      //시간을 문자열로 저장할 함수
    char *filename_p;                      //파일의 이름을 가리키는 포인터

    DIR *dir_p;              //DIR 포인터
    struct dirent *dirent_p; //dirent 포인터

    int i = 0;
    int filetime = 0, nowtime = 0; //파일이 저장된 시간, 현제 시간

    /*파일 경로 만들기*/
    sprintf(pathname, "%s%s_", node->directory, node->ori_filename);

    /*현제 시간 문자열로 만들고 int 형으로 변환하기*/
    sprintf(timestr, "%02d%02d%02d", tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
    nowtime = atoi(timestr);

    /*파일 이름과 디렉토리 경로 분리*/
    for (i = strlen(pathname) - 1; i >= 0; i--)
    {
        if (pathname[i] == '/')
        {
            pathname[i] = 0;
            filename_p = &pathname[i + 1];
            break;
        }
    }

    /*디렉토리 open*/
    if ((dir_p = opendir(pathname)) == NULL)
    {
        fprintf(stderr, "process >[ERROR] -t opt, opendir error for %s\n", pathname);
        return;
    }

    /*디렉토리 읽기*/
    while ((dirent_p = readdir(dir_p)) != NULL)
    {

        /*filename에 해당되는 backup파일 이라면*/
        if (strstr(dirent_p->d_name, filename_p) == dirent_p->d_name)
        {
            /*파일 이름을 로컬 변수에 복사*/
            memset(copy_d_name, 0, sizeof(copy_d_name));
            strcpy(copy_d_name, dirent_p->d_name);

            /*파일의 backup 시간 구하기*/
            for (i = strlen(copy_d_name) - 1; i >= 0; i--)
            {
                if (copy_d_name[i] == '_')
                {
                    filetime = atoi(&copy_d_name[i + 1 + 6]);
                    break;
                }
            }

            /*파일의 시간이 0이라면 error, 다음으로 진행*/
            if (filetime == 0)
                continue;

            /*backup 파일의 시간과 현제 파일의 시간차를 구해서 파일의 시간을 확인*/
            if (nowtime - filetime >= node->lifetime)
            {
                /*backup 폴더에 존재할 시간이 지났다면 파일 삭제*/

                sprintf(pathname_d_name, "%s/%s", pathname, copy_d_name);
                if (unlink(pathname_d_name) < 0)
                {
                    fprintf(stderr, "process >[ERROR] -t opt, unlink error for %s\n", pathname_d_name);
                    continue;
                }
            }
        }
    }
    closedir(dir_p);
}
void process_opt_n(struct backup_struct *node)
{
    char pathname[MAX_BACKUP_FILE];                  //파일의 전체 경로를 저장할 변수
    char pathname_d_name[MAX_BACKUP_FILE];           //파일의 전체 경로를 저장할 변수
    char tmp[MAX_BACKUP_FILE];                       //정렬하기 위한 임시 변수
    char filelist[MAX_BACKUP_FILE][MAX_BACKUP_FILE]; //파일의 목록을 저장할 변수
    char *filename_p;                                //파일의 이름을 가리키는 포인터

    DIR *dir_p;
    struct dirent *dirent_p;

    int filecnt = 0, i, j; //파일의 개수를 저장할 변수

    /*파일 경로 만들기*/
    sprintf(pathname, "%s%s_", node->directory, node->ori_filename);
    /*파일 이름과 디렉토리 경로 분리*/
    for (i = strlen(pathname) - 1; i >= 0; i--)
    {
        if (pathname[i] == '/')
        {
            pathname[i] = 0;
            filename_p = &pathname[i + 1];
            break;
        }
    }
    /*디렉토리 open*/
    if ((dir_p = opendir(pathname)) == NULL)
    {
        fprintf(stderr, "process >[ERROR] -n opt, opendir error for %s\n", pathname);
        return;
    }
    /*디렉토리 읽기*/
    while ((dirent_p = readdir(dir_p)) != NULL)
    {

        /*filename에 해당되는 backup파일 이라면*/
        if (strstr(dirent_p->d_name, filename_p) == dirent_p->d_name)
        {
            /*파일 이름을 list에 추가*/
            memset(filelist[filecnt], 0, sizeof(filelist[filecnt]));
            strcpy(filelist[filecnt++], dirent_p->d_name);
        }
        if (filecnt >= MAX_BACKUP_FILE)
            break;
    }
    /*버블 정렬을 이용한 내림차순 정렬(최근의 정보가 맨 위로)*/
    for (i = 0; i < filecnt; i++)
    {
        for (j = i + 1; j < filecnt; j++)
        {
            if (strcmp(filelist[i], filelist[j]) < 0)
            {
                memcpy(tmp, filelist[i], MAX_BACKUP_FILE);
                memcpy(filelist[i], filelist[j], MAX_BACKUP_FILE);
                memcpy(filelist[j], tmp, MAX_BACKUP_FILE);
            }
        }
    }
    /*가장 최근의 number개 제외한 나머지 파일 삭제*/
    for (i = node->number; i < filecnt; i++)
    {
        sprintf(pathname_d_name, "%s/%s", pathname, filelist[i]);
        unlink(pathname_d_name);
    }
    closedir(dir_p);
}
