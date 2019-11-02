#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 255
#endif

#ifndef DIR_MODE
#define DIR_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)//디렉토리 생성시 설정할 권한
#endif

/*-d 옵션시 수행할 함수*/
void process_add_directory(char *cwd, int logfd, int period, int opt, int lifetime, int number, char *directory);

int process_add(char *commandVector[], int logfd, char *directory)
{
    char *filename = NULL; //add하기 위한 파일 이름
    int opt = 0;           //option을 저장할 변수, 비트를 이용한다.
    int index = 0;         //commandVector배열의 인자를 가리키는 변수
    int period = 0;        //백업 기간을 저장할 변수
    int lifetime = -1;     //-t 옵션 입력시 디렉토리내 보관 기간
    int number = 0;        //-n 옵션 입력 시 디렉토리 내 최개 개수 저장
    int min = 0, sec = 0;  //lifetime에 60진수로 저장

    char copy_ori_file[MAX_FILE_NAME]; //원본 파일 이름을 복사할 배열
    char cwd[MAX_FILE_NAME];           //현제 파일 경로를 저장할 변수
    char *name_p1, *name_p2;           //파일 이름을 파싱 하기 위한 포인터

    struct backup_struct *node; //새로운 노드를 가리킬 변수
    struct stat statbuf;        //원본 파일의 정보를 저장할 변수

    for (index = 1; commandVector[index] != NULL; index++)
    {
        /*m 옵션, period 후 파일의 mtile이 수정 되었을 경우 백업*/
        if (strcmp(commandVector[index], "-m") == 0)
        {
            opt |= (1 << 0);
            continue;
        }
        /*n 옵션, 가장 최근의 number개의 백업 파일 외 나머지 삭제*/
        if (strcmp(commandVector[index], "-n") == 0)
        {
            opt |= (1 << 1);
            index++;
            /*number가 입력되지 않은 경우*/
            if (commandVector[index] == NULL)
            {

                fprintf(stderr, "usage : -n [NUMBER] \n");
                return -1;
            }
            /*정수형이 아닌 경우*/
            if (strstr(commandVector[index], ".") != NULL)
            {
                fprintf(stderr, "add >[error] NUMBER have to integer\n");
                return -1;
            }
            number = atoi(commandVector[index]);
            /*입력 범위를 벗어난 경우*/
            if((number<1) || (number > 100)){
                fprintf(stderr, "add >[error] range : 1<= NUMBER <= 100\n");
                return -1;
            }
            continue;
        }
        /*-t 옵션 디렉토리내 보관 기간, lifetime에 저장*/
        if (strcmp(commandVector[index], "-t") == 0)
        {
            opt |= (1 << 2);
            index++;
            /*시간이 입력되지 않은 경우*/
            if (commandVector[index] == NULL)
            {
                fprintf(stderr, "usage : -t [TIME] \n");
                return -1;
            }
            /*정수형이 아닌 경우*/
            if (strstr(commandVector[index], ".") != NULL)
            {
                fprintf(stderr, "add >[error] TIME have to integer\n");
                return -1;
            }
            lifetime = atoi(commandVector[index]);
            /*입력 범위를 벗어난 경우*/
            if((lifetime<60) || (lifetime> 1200)){
                fprintf(stderr, "add >[error] range : 60<= TIME <= 1200\n");
                return -1;
            }
            /*lifetime에서 min과 sec 구하기*/
            min = lifetime / 60;
            sec = lifetime % 60;
            /*lifetime를 min sec 형태로 바꾸기*/
            lifetime = min * 100 + sec;
            continue;
        }
        /*지정한 디렉토리 내의 모든 파일 백업 리스트에 저장, 서브 디랙토리 내 파일까지 저장*/
        if (strcmp(commandVector[index], "-d") == 0)
        {
            opt |= (1 << 3);
            index++;
            /*directory가 입력되지 않은 경우*/
            if (commandVector[index] == NULL)
                break;
            /*directory의 절대 경로로 변환*/
            if ((filename = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
            {
                fprintf(stderr, "add >[error] realpath error for %s\n", get_real_PATH(commandVector[index]));
                fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
                return -1;
            }
            continue;
        }
        /*잘 못 입력한 옵션에 대한 에러처리*/
        if (commandVector[index][0] == '-')
        {
            fprintf(stderr, "add >[error] unkown option\n");
            return -1;
        }
        /*파일 이름 입력 받기*/
        if (filename == NULL)
        {
            /*절대 경로로 변환 한 파일 이름 저장*/
            if ((filename = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
            {
                fprintf(stderr, "add >[error] realpath error for %s\n", get_real_PATH(commandVector[index]));
                fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
                return -1;
            }
            continue;
        }
        /*period 입력 받기*/
        else if (period == 0)
        {
            /*정수형이 아니라면 error*/
            if (strstr(commandVector[index], ".") != NULL)
            {
                fprintf(stderr, "add >[error] period have to integer\n");
                return -1;
            }
            /*period가 0인 경우 error*/
            if ((period = atoi(commandVector[index])) == 0)
            {
                fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
                return -1;
            }

            /*입력 범위를 벗어난 경우*/
            if((period<5) || (period> 10)){
                fprintf(stderr, "add >[error] range : 5<= period <= 10\n");
                return -1;
            }
            continue;
        }
        fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
        return -1;
    }
    /*필수 요건이 충족 되지 않았다면 error */
    /*파일 이름이너무 큰 겨ㅛㅇ우 error*/
    if (strlen(filename) >= MAX_FILE_NAME)
    {
        fprintf(stderr, "add >[error] filename is too long [%s]\n", filename);
        return -1;
    }
    /*-t 옵션의 인자가 적절하지 않는 경우*/
    /*-t [TIME]*/
    if ((opt & (1 << 2)) && (lifetime == 0))
    {
        fprintf(stderr, "usage : -t [TIME] \n");
        return -1;
    }
    /*-n 옵션의 인자가 적절하지 않는 경우*/
    /*-n [NUMBER]*/
    if ((opt & (1 << 1)) && (number == 0))
    {
        fprintf(stderr, "usage : -n [NUMBER] \n");
        return -1;
    }
    /*기간이 주어지지 않았거나 filename이 주어지지 않았다면 error*/
    if (period == 0 || filename == NULL)
    {
        fprintf(stderr, "usage : add <FILENAME> [PERIOD] [OPTION]\n");
        return -1;
    }
    /*접근 가능한지 확인*/
    if (access(filename, F_OK) < 0)
    {
        fprintf(stderr, "add : [error] cannot access for %s\n", filename);
        return -1;
    }
    /*파일의 상태 정보 stat구조체에 저장*/
    if (lstat(filename, &statbuf) < 0)
    {
        fprintf(stderr, "add : [error] lstat error for %s\n", filename);
        return -1;
    }
    /*-d option이 아닌 경우*/
    if ((opt & (1 << 3)) == 0)
    {
        /*regular 파일이 아닌 경우 error*/
        if (!S_ISREG(statbuf.st_mode))
        {
            fprintf(stderr, "add : [error] not regular file [%s]\n", filename);
            return -1;
        }
        /*파일이 이미 존재 할 경우 error*/
        if (findFileName(filename) != NULL)
        {
            fprintf(stderr, "add : [error] already add [%s]\n", filename);
            return -1;
        }
        /*node를 list에 추가*/
        node = addList(period, opt, lifetime, number, filename);
        if (node == NULL)
        {
            fprintf(stderr, "add : [error] add to list error [%s]\n", filename);
            return -1;
        }
        /*node에 로그파일 디스크립터와 백업 디렉토리 저장*/
        node->logfd = logfd;
        node->directory = directory;

        /*원래 경로를 파싱 하기 위해 복사*/
        memset(copy_ori_file, 0, MAX_FILE_NAME);
        strcpy(copy_ori_file, node->ori_filename);

        /*현제 작업 경로 저장*/
        getcwd(cwd, MAX_FILE_NAME);

        /*백업 경로로 이동*/
        chdir(node->directory);
        /*슬레쉬 기준으로 파싱하며 백업 디렉토리에 폴더 생성*/
        name_p1 = strtok(copy_ori_file, " /");
        /*파싱 실패시 에러 처리*/
        if (name_p1 == NULL)
        {
            fprintf(stderr, "tokenizer error for %s\n", node->ori_filename);
            removeNode(node);
            return -1;
        }
        while ((name_p2 = strtok(NULL, " /")) != NULL)
        {
            mkdir(name_p1, DIR_MODE); //디렉토리 생성
            chdir(name_p1);           //디렉토리 변경
            name_p1 = name_p2;        //가장 마지막 인자 찾기 -> 파일의 이름
        }
        /*기존의 작업 디렉토리로 이동*/
        chdir(cwd);

        /*backup process 생성*/
        if (pthread_create(&node->tid, NULL, process_update_thread, (void *)node) < 0)
        {
            fprintf(stderr, "add : [error] pthread_creat error for %s \n", filename);
            return -1;
        }
        /*할당된 공간 해제*/
        free(filename);

        return 1;
    }
    /*-d 옵션이 설정된 경우*/
    else
    {
        /*directory 파일이 아닌 경우 error*/
        if (!S_ISDIR(statbuf.st_mode))
        {
            fprintf(stderr, "add : [error] not directory file [%s]\n", filename);
            return -1;
        }

        /*directory내용을 add하기위한 함수 호출*/
        process_add_directory(filename, logfd, period, opt, lifetime, number, directory);

        /*할당된 공간 해제*/
        free(filename);

        return 1;
    }
    return -1;
}

/*directory내용을 add하기위한 함수*/
void process_add_directory(char *curdir, int logfd, int period, int opt, int lifetime, int number, char *directory)
{
    DIR *dirp;                             //DIR 포인터
    struct dirent *direntp;                //dirent 포인터
    char backupFile[MAX_FILE_NAME * 3];    //backupFile경로
    char oriPath[MAX_FILE_NAME * 2];       //원본 파일 경로
    char dname[MAX_FILE_NAME];             //dname 복사 하기 위한 배열
    char copy_ori_file[MAX_FILE_NAME * 2]; //원본 파일의 경로를 복사하기 위한 변수
    char cwd[MAX_FILE_NAME];               //현제 작업 디렉토리 저장할 변수
    char *name_p1, *name_p2;               //파일의 이름을 가리키기 위한 변수
    struct stat statbuf;                   //파일의 속성을 저장할 변수
    struct backup_struct *node;            //새로운 node를 가리키기 위한 변수

    /*초기화*/
    memset(backupFile, 0, sizeof(backupFile));
    memset(oriPath, 0, sizeof(oriPath));
    /*directory open*/
    if ((dirp = opendir(curdir)) == NULL)
    {
        fprintf(stderr, "add : [error] opendir error for [%s]\n", cwd);
        return;
    }
    /*read directory entry*/
    while ((direntp = readdir(dirp)) != NULL)
    {
        /*. , .. 디렉토리 제외*/
        if (direntp->d_name[0] == '.')
            continue;
        memset(dname, 0, sizeof(dname));                              //dname 초기화
        strcat(dname, direntp->d_name);                               //d_name 복사
        sprintf(backupFile, "%s%s/%s", directory, backupFile, dname); //backup 파일 경로 만들기
        sprintf(oriPath, "%s/%s", curdir, dname);                     //원본 파일 경로 만들기

        /*파일의 상태 정보 stat구조체에 저장*/
        if (lstat(oriPath, &statbuf) < 0)
        {
            fprintf(stderr, "add : [error] lstat error for %s\n", oriPath);
            continue;
        }
        /*regular 파일이 아닌 경우*/
        if (!S_ISREG(statbuf.st_mode))
        {
            /*directory 파일이 아닌 경우 error*/
            if (!S_ISDIR(statbuf.st_mode))
            {
                fprintf(stderr, "add : [error] not directory file or regular file [%s]\n", oriPath);
                continue;
            }
            /*directory 파일인 경우*/
            else
            {
                /*함수 재귀 호출*/
                process_add_directory(oriPath, logfd, period, opt, lifetime, number, directory);
            }
            continue;
        }
        /*파일이 이미 존재 할 경우 error*/
        if (findFileName(oriPath) != NULL)
        {
            fprintf(stderr, "add : [error] already add [%s]\n", oriPath);
            continue;
        }
        /*node를 list에 추가*/
        node = addList(period, opt, lifetime, number, oriPath);
        if (node == NULL)
        {
            fprintf(stderr, "add : [error] add to list error [%s]\n", oriPath);
            continue;
        }
        /*node에 log 파일의 디스크립터와 directory 할당*/
        node->logfd = logfd;
        node->directory = directory;

        /*원래 경로를 파싱 하기 위해 복사*/
        memset(copy_ori_file, 0, sizeof(copy_ori_file));
        strcpy(copy_ori_file, node->ori_filename);

        /*현제 작업 경로 저장*/
        getcwd(cwd, MAX_FILE_NAME);

        /*백업 경로로 이동*/
        chdir(node->directory);
        /*슬레쉬 기준으로 파싱하며 백업 디렉토리에 폴더 생성*/
        name_p1 = strtok(copy_ori_file, " /");
        /*파싱 실패시 에러 처리*/
        if (name_p1 == NULL)
        {
            fprintf(stderr, "tokenizer error for %s\n", node->ori_filename);
            removeNode(node);
            return;
        }
        while ((name_p2 = strtok(NULL, " /")) != NULL)
        {
            mkdir(name_p1, DIR_MODE); //디렉토리 생성
            chdir(name_p1);           //디렉토리 변경
            name_p1 = name_p2;        //가장 마지막 인자 찾기 -> 파일의 이름
        }
        /*기존 경로로 이동*/
        chdir(cwd);

        /*backup process 생성*/
        if (pthread_create(&node->tid, NULL, process_update_thread, (void *)node) < 0)
        {
            fprintf(stderr, "add : [error] pthread_creat error for %s \n", oriPath);
            continue;
        }
    }
    closedir(dirp);
}