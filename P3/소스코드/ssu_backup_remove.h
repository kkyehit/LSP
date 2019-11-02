#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 255
#endif

/*remove를 하기 위한 함수*/
void process_remove(char *commandVector[], int logfd, char *directory)
{
    char *filename = NULL;      //인자로 주어진 파일 이름을 가리킬 포인터
    int opt = 0;                //-a 옵션을 처리하기 위해 필요한 변수
    int index = 0;              //commandVectoer를 탐색하기 위한 변수
    int status = 0;             //thread의 상태 값을 저장할 변수
    struct backup_struct *node; //리스트의 한 노드를 가리키는 변수
    struct tm *tm_p;            //시간을 구하기 위한 구조체 변수
    time_t now;                 //시간을 저장할 변수

    char log_msg[MAX_LOG_MSG]; //logfile에 저장할 msg를 저장하는 함수

    time(&now);             //시간을 구한다
    tm_p = localtime(&now); //struct tm 구조체로 만든다.

    for (index = 1; commandVector[index] != NULL; index++)
    {
        /*옵션에 대한 처리, list의 모든 노드 지우기*/
        if (strcmp(commandVector[index], "-a") == 0)
        {
            opt = 1;
            continue;
        }
        /*절대 경로 구하기*/
        if ((filename = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
        {
            fprintf(stderr, "remove>[error] realpath error\n");
            return;
        }
    }
    /*-a 옵션일 경우*/
    if (opt == 1)
    {
        /*파일 이름이 인자로 들어오면 error*/
        if (filename != NULL)
        {
            fprintf(stderr, "remove -a \t\t\t -> remove all, do not input filename\n");
            return;
        }
        /*list의 시작부터 처리한다.*/
        node = root;
        /*더이상 node가 없을 때 까지 반복*/
        while (node != NULL)
        {
            /*노드의 파일에 작업을 하는 thread 종료 명령*/
            if (pthread_cancel(node->tid) != 0)
            {
                fprintf(stderr, "remove>[error] pthread_cancel error for %ld\n", node->tid);
                return;
            }

            /*쓰래드의 종료를 기다림*/
            pthread_join(node->tid, (void *)&status);

            /*logfile에 작성*/
            pthread_mutex_lock(&mutex);      //mutex를 이용해 동기화 수행
            time(&now);                      //시간을 구한다
            tm_p = localtime(&now);          //struct tm 구조체로 만든다.
            memset(log_msg, 0, MAX_LOG_MSG); //log_msg 변수 초기화
            sprintf(log_msg, "[%02d%02d%02d %02d%02d%02d] %s removed\n", ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec, node->ori_filename);
            write(node->logfd, log_msg, strlen(log_msg)); //log 파일에 쓰기
            pthread_mutex_unlock(&mutex);                 //mutex잠금 해제

            /*리스트에서 노드 제거, 다음 노드를 반환한다.*/
            node = removeNode(node);
        }
    }
    /*-a 옵션이 아닌 경우*/
    else
    {
        /*파일의 이름이 인자로 주어지지 않았다면 에러*/
        if (filename == NULL)
        {
            fprintf(stderr, "remove <FILENAME> [OPTION]\n");
            return;
        }
        /*리스트에서 해당 파일 이름 검색*/
        node = findFileName(filename);
        /*해당 파일 이름을 백업 하고 있는node가 없는 경우*/
        if (node == NULL)
        {
            fprintf(stderr, "remove>[error] find error for %s\n", filename);
            return;
        }
        /*노드의 파일에 작업을 하는 thread 종료 명령*/
        if (pthread_cancel(node->tid) != 0)
        {
            fprintf(stderr, "remove>[error] pthread_cancel error for %ld\n", node->tid);
            return;
        }
        /*쓰래드의 종료를 기다림*/
        pthread_join(node->tid, (void *)&status);

        /*logfile에 작성*/

        pthread_mutex_lock(&mutex);      //mutex를 이용해 동기화 수행
        time(&now);                      //시간을 구한다
        tm_p = localtime(&now);          //struct tm 구조체로 만든다.
        memset(log_msg, 0, MAX_LOG_MSG); //log_msg 변수 초기화
        sprintf(log_msg, "[%02d%02d%02d %02d%02d%02d] %s removed\n", ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec, node->ori_filename);
        write(node->logfd, log_msg, strlen(log_msg)); //log 파일에 쓰기

        pthread_mutex_unlock(&mutex); //mutex잠금 해제

        /*리스트에서 노드 제거*/
        removeNode(node);
    }
    /*할당된 공간 해제*/
    free(filename);
}