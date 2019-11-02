
#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 255
#endif
void process_recover(char *commandVector[], int logfd, char *directory)
{
    int opt = 0;                                                      //-n 옵션을 위한 변수
    int index = 1;                                                    //cammandVector를 위한 변수
    int listIndex = 0;                                                //list의 크기
    int inputIndex = 0;                                               //입력받은 index
    int i, j;                                                         //반복문을 위한 변수
    int selectfd;                                                     //selectfd : 사용자가 선택한 파일의 디스크립터
    int newfilefd;                                                    //newfilefd : 백업 할 파일의 디스크립터
    char *filename = NULL;                                            //파일의 이름을 가리킨다.
    char *backupfilename = NULL;                                      //pathname에서 파일의 이름을 가리킨다.
    char *findfilename = (char *)calloc(sizeof(char), MAX_FILE_NAME); //찾을 파일의 pathname
    char findfilelist[MAX_FILE_NAME][MAX_FILE_NAME];                  //찾을 파일의 목록을 저장한다. 최대 255개
    char tmp[MAX_FILE_NAME];                                          //정렬을 위한 임시 변수
    char character;                                                   //문자를 읽기 위한 변수
    char *newfilename = NULL;                                         //-n옵션이 새로운 파일의 이름을 가리키기 위한 변수
    char log_msg[MAX_LOG_MSG];                                        //log에 쓸 메세지를 저장할 변수

    struct backup_struct *node = NULL; //현제 백업 list에 있는지 확인 하기 위한 변수

    struct stat statbuf; //파일의 상태를 얻어 mode를 알아내기 위한 변수

    DIR *dirp;             //DIR 스트림
    struct dirent *dirent; //dir entry

    struct tm *tm_p; //시간을 구하기 위한 구조체 변수
    time_t now;

    memset(findfilelist, 0, sizeof(findfilelist));

    /*commandVector에서 원하는 정보를 얻는 과정*/
    for (index = 1; commandVector[index] != NULL; index++)
    {
        /*-n옵션인지 확인*/
        if (strcmp(commandVector[index], "-n") == 0)
        {
            /*옵션이 설정 되어 있음을 표시*/
            opt = 1;
            continue;
        }
        /*-n <new file>*/
        if ((opt == 1) && (newfilename == NULL))
        {
            newfilename = commandVector[index];
            continue;
        }
        /*filename이 NULL이면 할당해 준다.*/
        if (filename == NULL)
        {
            /*realpath() : 상대경로를 정대 경로로 바꾸어 주는 함수*/
            /*get_real_PATH() : ~ 경로를 알 수 있는 경로로 변환*/
            if ((filename = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
            {
                /*파일이 존재하지 않는 경우 error, -1 리턴*/
                fprintf(stderr, "recover>[error] realpath error for %s\n", get_real_PATH(commandVector[index]));
                fprintf(stderr, "recover>[error] maybe %s is not exist\n", get_real_PATH(commandVector[index]));
                fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
                return;
            }
            continue;
        }
        /*그 외의 입력이 주어지면 error*/
        fprintf(stderr, "usage : recover <FILENAME> [OPTION]\n");
        return;
    }
    /*commandVector 확인 후 filename이 주어지지 않았다면 error*/
    if (filename == NULL)
    {
        fprintf(stderr, "usage : recover <FILENAME> [OPTION]\n");
        return;
    }
    /*commandVector 확인 후 옵션이 설정 되어 있지만 newfile 이름이 주어지지 않는 경우 error*/
    if ((opt == 1) && (newfilename == NULL))
    {
        fprintf(stderr, "usage : recover <FILENAME> [OPTION]\n");
        return;
    }
    /*backup list에 존재하는지 확인*/
    node = findFileName(filename);
    /*존재 한다면 list에서 지운다.*/
    if (node != NULL)
    {
        char *removeCommand[3];
        /*list에서 제거 된다는 msg 출력*/
        fprintf(stderr, "recover>remove back list %s\n", filename);
        /*remove filename 형식에 맞게 removeCommand 구성*/
        removeCommand[0] = "remove";
        removeCommand[1] = filename;
        removeCommand[2] = NULL;
        /*remove 수행*/
        process_remove(removeCommand, logfd, directory);
    }
    /*backup 파일 상의 파일 경로를 만든다.*/
    sprintf(findfilename, "%s%s_", directory, filename);
    for (index = strlen(findfilename) - 1; index >= 0; index--)
    {
        if (findfilename[index] == '/')
        {
            /*backup파일 경로 상에서 이름만을 가리키는 포인터 할당*/
            backupfilename = findfilename + index + 1;
            /*directory 경로와 파일 이름을 분리한다.*/
            findfilename[index] = 0;
            break;
        }
    }
    /*directory 오픈*/
    if ((dirp = opendir(findfilename)) == NULL)
    {
        fprintf(stderr, "recover>[error]open error for %s\n", findfilename);
        return;
    }
    /*directory entry 확인.*/
    while ((dirent = readdir(dirp)) != NULL)
    {
        /*파일이름을 포함하는 파일을 리스트에 추가*/
        if (strstr(dirent->d_name, backupfilename) == dirent->d_name)
        {
            /*경로와 함께 list에 추가*/
            sprintf(findfilelist[listIndex], "%s/%s", findfilename, dirent->d_name);
            listIndex++;
            /*list의 크기가 255가 넘어가면 멈춘다.*/
            if (listIndex == MAX_FILE_NAME)
                break;
        }
    }

    /*버블 정렬을 이용한 정렬*/
    for (i = 0; i < listIndex; i++)
    {
        for (j = i + 1; j < listIndex; j++)
        {
            if (strcmp(findfilelist[i], findfilelist[j]) > 0)
            {
                memcpy(tmp, findfilelist[i], MAX_FILE_NAME);
                memcpy(findfilelist[i], findfilelist[j], MAX_FILE_NAME);
                memcpy(findfilelist[j], tmp, MAX_FILE_NAME);
            }
        }
    }

    /*list를 출력*/
    for (index = 0; index < listIndex; index++)
    {
        /*size를 알아내기 위해 stat구조체 사용*/
        if (lstat(findfilelist[index], &statbuf) < 0)
            continue;
        /*size와 함께 파일 이름 출력*/
        printf("recover>%d.\t%-70s\t%ld\n", index, findfilelist[index], statbuf.st_size);
    }
    /*list가 없으면 error*/
    if (index == 0)
    {
        fprintf(stderr, "recover>%s not founded at %s\n", filename, directory);
        return;
    }
    /*마지막은 선택하지 않음*/
    printf("recover>%d. \tnot selet\n", index);

    /*번호 입력 받기*/
    printf("recover>input index : ");
    scanf("%d", &inputIndex);
    getchar();

    /*선택하지 않는 경우 종료*/
    if (inputIndex == index)
    {
        printf("recover>not select\n");
        fflush(stdout);
        free(findfilename);
        return;
    }
    /*입력 받은 숫자가 너무 큰 경우 error */
    if (inputIndex > index)
    {
        fprintf(stderr, "recover>[error]range over\n");
        free(findfilename);
        return;
    }

    /*선택한 백업 파일 open*/
    if ((selectfd = open(findfilelist[inputIndex], O_RDONLY)) < 0)
    {
        fprintf(stderr, "recover>[error]open error for %s\n", findfilelist[inputIndex]);
        return;
    }

    /*option이 설정되지 않는 경우 원본 파일에 덮어쓰기*/
    if (opt == 0)
    {
        /*입력 받은  파일에 backup*/
        if ((newfilefd = open(filename, O_WRONLY | O_TRUNC)) < 0)
        {
            fprintf(stderr, "recover>[error]open error for %s\n", findfilelist[inputIndex]);
            return;
        }
    }
    /*option이 설정되어있는 경우 새로운 파일에 작성*/
    else
    {
        /*새로 만들 파일의 mode를 결정하기 위해 stat구조체 사용*/
        if (lstat(findfilelist[inputIndex], &statbuf) < 0)
        {
            fprintf(stderr, "recover>[error]lstat error for %s\n", findfilelist[inputIndex]);
            return;
        }
        /*파일 생성 및 오픈*/
        if ((newfilefd = open(newfilename, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, statbuf.st_mode)) < 0)
        {
            /*파일이 이미 존재할 경우 error*/
            fprintf(stderr, "recover>[error]open or creat error for %s\n", newfilename);
            fprintf(stderr, "recover>[error]maybe already exitst file name\n");
            return;
        }
    }
    write(1, "recover>backup :\n", 18);

    /*backup 파일에서 한 문자를 읽어서*/
    while ((read(selectfd, &character, 1)) > 0)
    {
        /*표준 출력에 출력*/
        write(1, &character, 1);
        /*backup을 복구할 파일에 출력*/
        write(newfilefd, &character, 1);
    }

    pthread_mutex_lock(&mutex);//mutex를 이용한 동기화
    time(&now);             //시간을 구한다
    tm_p = localtime(&now); //struct tm 구조체로 만든다.

    /*log msg 만들기*/
    memset(log_msg, 0, MAX_LOG_MSG);
    sprintf(log_msg, "[%02d%02d%02d %02d%02d%02d] %s recover\n", ((tm_p->tm_year + 1900) % 100), tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec, filename);
    /*log파일에 출력*/
    write(logfd, log_msg, strlen(log_msg));

    pthread_mutex_unlock(&mutex);//mutex잠금 해제

    /*할당된 공간 반환*/
    free(findfilename);
}