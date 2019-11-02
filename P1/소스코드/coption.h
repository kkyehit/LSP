#define _CLASS_NUM 80
#define _SCORE_LENGTH 80

/*opttion c를 처리하는 함수*/
void opt_c(char *filepath, char list[5][100])
{
    int cnt = 0;                 //c 리스트의 개수
    int p_count = 0;             //문제 수
    int fd = 0;                  //score.csv 파일의 문제 번호
    int i = 0, readcnt;          //readCnt : 읽은 바이트 수
    int index = 0, commaCnt = 0; //한 열의 comma 개수
    char c;                      //파일에서 문자를 읽기 위한 변수
    char resbuf[_SCORE_LENGTH];  //점수를 저장 할 변수
    char classbuf[_CLASS_NUM];   //학번을 저장할 변수

    while ((cnt < 5) && (list[cnt] != NULL) && (list[cnt][0] != 0))
    { //c_list의 개수 얻기
        cnt++;
    }

    if ((fd = open(filepath, O_RDONLY)) < 0) //filepath 를 open(), 에러시 함수 종료
    {
        fprintf(stderr, "open error for %s\n", filepath);
        return;
    }
    while ((readcnt = read(fd, &c, 1)) > 0)
    {
        if (c == '\n') //한 열을 탐색
            break;
        if (c == ',') //comma 개수 저장 -> 문제 수가 된다.
            p_count++;
    }
    if (readcnt < 0) //read() 함수에 error 가있다면 함수 종료
    {
        fprintf(stderr, "%s file read error\n", filepath);
        return;
    }

    for (i = 0; i < cnt; i++)
    {
        lseek(fd, 0, SEEK_SET); //처음으로 커서 이동.
        while (1)
        {
            memset(resbuf, 0, sizeof(resbuf));
            memset(classbuf, 0, sizeof(classbuf));
            index = 0;
            commaCnt = 0;
            while ((readcnt = read(fd, &c, 1)) > 0)
            {
                if (c == ',')
                    break;
                classbuf[index++] = c; //학번을 저장
            }
            commaCnt++;
            while ((readcnt = read(fd, &c, 1)) > 0) //comma 수만큼 이동하여 마지막에 존재하는 sum에 접근
            {
                if (c == ',')
                    commaCnt++;
                if (p_count == commaCnt)
                    break;
            }
            index = 0;
            while ((readcnt = read(fd, &c, 1)) > 0) //점수 문자열을 resbuf에 저장
            {
                if (c == '\n')
                    break;
                resbuf[index++] = c;
            }
            if (strcmp(list[i], classbuf) == 0) //만약 읽어들인 학번이 c_list에 존재한다면
            {
                printf("%s's socre : %s\n", classbuf, resbuf); //학번과 점수 출력
                break;
            }
            else if (readcnt == 0) //만약 파일 끝에 도달했다면
            {
                printf("%s not found\n", list[i]); //찾지 못했다는 msg 출력
                break;
            }
        }
    }
}