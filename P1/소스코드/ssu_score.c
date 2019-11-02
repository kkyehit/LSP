#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include "help.h"
#include "blankCmp2.h"
#include "coption.h"

#define _exe ".exe"                    //.exe 파일 확장자
#define _out ".stdout"                 //.stdout 파일 확장자
#define _score_table "score_table.csv" //문제별 점수 정보 저장 할 파일 이름
#define _RESULT_TABLE_NAME "score.csv" //점수 경과 저장할 파일 이름
#define _opt_string "e:pt:hc:"         //옵션
#define _opt_t "-lpthread"             //t 옵션이 있을 시 컴파일에 주어질 옵션
#define _error 0                       //error 시 점수
#define _warning -0.1                  //warning 당 점수
#define _time_limit 5                  //학생 문제 시간 제한
#define _buf_size 1024 * 10            //문자열을 저장항 버퍼 크기
#define _file_length 100               // 최대 파일 이름 길이
#define _MAX_STUDENT 110               //최대 학생수 및 최대 문제 수
#define _MAX_PARAM 5                   //옵션의 최대 인자 수
#define _SECOND_TO_MICRO 1000000       //1sec == 1000000

char stdDir[_file_length], ansDir[_file_length], errDir[_file_length]; //directoy 경로
char filesName[_MAX_STUDENT][_file_length];                            //정답 파일 이름
struct dirent *files[_MAX_STUDENT];                                    // 정답파일
char answer[_MAX_STUDENT][_MAX_STUDENT][_buf_size];                    //정답
int problemCnt;                                                        //문제 수
char optionFlag[_MAX_PARAM];                                           // e : 0 , p : 1, t : 2, h : 3, c : 4,
double score[_MAX_STUDENT][_MAX_STUDENT];                              //문제별 스코어 ex) 1-1.txt -> score[1][1] 2.txt -> score[2][0]
int t_list[_MAX_PARAM];                                                //t 옵션 뒤에 오는 인자들
char c_list[_MAX_PARAM][_file_length];                                 //c 옵션 뒤에 오는 인자들

char studentList[_MAX_STUDENT][_file_length];                  //학생 디렉토리 목록,
double studentScore[_MAX_STUDENT][_MAX_STUDENT][_MAX_STUDENT]; // 학생의 문제 별 점수[학생인덱스][문제번호][가지문제 번호]
int studentCount;                                              //학생 수
                                                               //학생 수

/*option별 인덱스 반환*/
int indefOfOpt(char a);

/*파일 관련*/
char *dtoa(double _a);                                //double 형을 string 으로 변환, 변환된 string 리턴
char *itoa(int _a);                                   //int 형을 String 으로 변환, 변환된 string 리턴
int getProblemSet(char *filename, int num[2]);        // 문제 번호와 확장자 얻기,
void getProblemNum(const char *filename, int num[2]); // 문제 번호 얻기
int getExt(const char *filename);                     //확장자 얻기
char *getName(const char *filename);                  //파일 이름 얻기
int cmpProblemNum(char *_file1, char *_file2);        //file1의 문제번호가 작으면 0, 아니면 1 return;

/*점수 테이블 관련*/
void makeScoreTable(int _scoreFileDes); //점수 테이블 만들기
void getScore();                        //점수 가져오기
int isNumber(char *_str);               //string이 숫자인지 확인, 맏으면 1 틀리면 -

/*파일 로딩, 컴파일 함수*/
void loadFileTo(char *_filename, char *_to, char *_dir); //파일 내용 로드하는 함수
void removeBlankAndLower(char *_at, char *_to);          //공백과 대문자 지우는 함수

void complieAndLoadFile(char *_filename, char *_dir); //컴파일 하고 저장 및 로드 하는 함수

/*초기화*/
int init(int _argc, char *_argv[]);             //memset과 getopt 파싱
void loadAns();                                 //점수 파일 로딩
void cToExe(char *_filename, char *_dir);       //c 파일을 exe 파일로 만들기
void exeToFile(char *_exeFileName, char *_dir); //exe 파일을 실행시켜 stdout 파일에 출력

/*학생 관련 함수*/
void loadStd();                                                                  //학생 답안 파일 불러오기
void complieAndLoadFileStd(char *_filename, char *_dir, char *_res, int _index); //학생의 답안파일을 불러오기 위한 함수
int simpleCmp(char *_str1, char *_str2);                                         //단순이 정답과 학생 답안 비교하는 함수 맞으면 1 틀리면 0 return;
double findWarning(const char *pathname);                                        //warning 이 있다면 감점될 점수 return 없으면 0
int findError(const char *pathname);                                             //error 가 있다면 1, 없다면 0 return
int complexCmp(char *_str1, char *_str2);                                        //연산자가 포함된 답안 체점, 가능한 답이면 1 아니면 0 return;

/*시간 측정을 위한 함수*/
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        print_h_msg(argv[0]);
        exit(0);
    }

    struct timeval begin_t, end_t;
    gettimeofday(&begin_t, NULL);

    init(argc, argv); // 초기화

    /*정답 파일 목록 로딩*/
    write(1, "loading ans..\n", 14);
    loadAns();

    /*점수 테이블 생성 또는 로딩*/
    getScore();

    /*학생 정보 로딩*/
    write(1, "loading std..\n", 14);
    loadStd();

    /*프로그램 마지막에 c 옵션 처리*/
    if (optionFlag[indefOfOpt('c')])
    {
        opt_c(_RESULT_TABLE_NAME, c_list);
    }

    /*걸린 시간 출력*/
    gettimeofday(&end_t, NULL);
    ssu_runtime(&begin_t, &end_t);
    printf("Runtime : %ld:%06ld(sec:usec)\n", end_t.tv_sec, end_t.tv_usec);
}
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{
    end_t->tv_sec -= begin_t->tv_sec;

    if (end_t->tv_usec < begin_t->tv_usec)
    {
        end_t->tv_sec--;
        end_t->tv_usec += _SECOND_TO_MICRO;
    }
    end_t->tv_usec -= begin_t->tv_usec;
}

int indefOfOpt(char a)
{
    if (a == 'e') //optionFlag의 e 위치는 0
        return 0;
    if (a == 't') //optionFlag의 t 위치는 0
        return 1;
    if (a == 'c') //optionFlag의 c 위치는 0
        return 2;
    if (a == 'p') //optionFlag의 p 위치는 0
        return 3;
    if (a == 'h') //optionFlag의 h 위치는 0
        return 4;
    return -1; // 그 외의 문자는 잘못된 옵션
}

int init(int _argc, char *_argv[])
{
    int a;                    //option을 파싱하기 위한 변수
    DIR *checkValue;          //<ERR_DIR> 폴더가 이미 존재하는지 확인하기 위한 변수
    char rmComand[_buf_size]; //<ERR_DIR> 폴더가 이미 존재하면 지우기 위한 command 변수

    /*memset() 을 이용한 초기화*/
    memset(rmComand, 0, sizeof(rmComand));

    memset(studentList, 0, sizeof(studentList));
    memset(studentScore, 0, sizeof(studentScore));

    memset(filesName, 0, sizeof(filesName));
    memset(answer, 0, sizeof(answer));
    memset(optionFlag, 0, sizeof(optionFlag));

    memset(score, 0, sizeof(score));

    memset(stdDir, 0, sizeof(stdDir));
    memset(ansDir, 0, sizeof(ansDir));
    memset(errDir, 0, sizeof(errDir));

    memset(t_list, 0, sizeof(t_list));
    memset(c_list, 0, sizeof(c_list));

    if (_argv[1][0] != '.' && _argv[1][0] != '/' && _argv[1][0] != '~')
        strcat(stdDir, "./"); //학생 디렉토리, 폴더명만 주어지면 하위폴더,
    if (_argv[2][0] != '.' && _argv[2][0] != '/' && _argv[1][0] != '~')
        strcat(ansDir, "./");   //정답 디렉토리,
    strcat(stdDir, _argv[1]);   //학생 디렉토리,
    strcat(ansDir, _argv[2]);   //정답 디렉토리,
    strcat(rmComand, "rm -r "); //디렉토리 삭제 명령어

    for (int i = 0; i < 4; i++)
        optionFlag[i] = 0;

    /*option 파싱*/
    while ((a = getopt(_argc, _argv, _opt_string)) != -1)
    {
        if (a == 'e')
        {
            optionFlag[0] = 1; //e 옵션 플래그설정
            if (optarg[0] != '.' && optarg[0] != '/' && optarg[0] != '~')
                strcat(errDir, "./"); //err directory 설정
            strcat(errDir, optarg);
            if ((checkValue = opendir(errDir)) != NULL)
            { //errdir이 이미 존재하는지 확인하기 위함
                strcat(rmComand, errDir);
                system(rmComand);
                closedir(checkValue);
            }
            mkdir(errDir, 0777);
        }
        else if (a == 't')
        {
            optionFlag[1] = 1;                                   //t 옵션 플래그설정
            for (int i = optind - 1, k = 0; i < _argc; i++, k++) //인자 5개를 t_list에 저장
            {
                if (_argv[i][0] == '-') //옵션을 만나면 for문 종료
                    break;
                if (k >= 5) //인자가 5개 보다 많이 들어오게 되면 error msg 출력
                {
                    printf("Maximum Number of Argument Exceeded. :: %s\n", _argv[i]);
                    continue;
                }
                t_list[k] = atoi(_argv[i]);
            }
        }
        else if (a == 'c')
        {
            optionFlag[2] = 1;                                   //c 옵션 플래그설정
            for (int i = optind - 1, k = 0; i < _argc; i++, k++) //인자 5개를 c_list에 저장

            {
                if (_argv[i][0] == '-') //옵션을 만나면 for문 종료
                    break;
                if (k >= 5) //인자가 5개 보다 많이 들어오게 되면 error msg 출력
                {
                    printf("Maximum Number of Argument Exceeded. :: %s\n", _argv[i]);
                    continue;
                }
                memcpy(c_list[k], _argv[i], sizeof(_argv[i]));
            }
        }
        else if (a == 'p')
            optionFlag[3] = 1; //p 옵션 플래그설정
        else if (a == 'h')
        {
            optionFlag[4] = 1;     //h 옵션 플래그설정
            print_h_msg(_argv[0]); //help 메세지 출력 후
            exit(0);               //프로그램 종료
        }
    }
}
void complieAndLoadFile(char *_filename, char *_dir)
{
    int index = -1;
    int problemNum[2];          //문제 번호를 위한 배열, 1-1 의 경우 {1,1} 1 의 경우 {1,0}
    char outFile[_file_length]; //출력할 파일 이름
    char *tmp;
    char name[_buf_size];                                           //파일의 확장자를 제외한 이름
    int stderrtmpDes = open("stderrtmp", O_WRONLY | O_CREAT, 0766); //stderr 임시저장 할 파일
    int stdouttmpDes = open("stdouttmp", O_WRONLY | O_CREAT, 0766); //stdout 임시저장
    int errtmpDes = open("errtmp", O_WRONLY | O_CREAT, 0766);       //err출력할 임시파일
    dup2(2, stderrtmpDes);                                          //stderr 을 임시저장
    dup2(1, stdouttmpDes);                                          //stdout 을 임시저장
    dup2(errtmpDes, 2);                                             //err을 출력할 임시 파일,

    getProblemNum(_filename, problemNum); //문제번호를 파싱하여 배열에 저장
    if (getExt(_filename) == 0)           //확장자를 확인한다. 0이면 확장자는 .c 이다.
    {
        tmp = getName(_filename);
        memset(name, 0, sizeof(name));
        memcpy(name, tmp, strlen(tmp));
        cToExe(_filename, _dir);    //점수 파일 컴파일
        exeToFile(_filename, _dir); //컴파일된 파일 파일출력,

        memset(outFile, 0, sizeof(outFile));
        memcpy(outFile, name, sizeof(name));
        loadFileTo(strcat(outFile, _out), answer[problemNum[0]][problemNum[1]], _dir);                   //출력 파일을 읽어 내용을 가져오는 함수
        removeBlankAndLower(answer[problemNum[0]][problemNum[1]], answer[problemNum[0]][problemNum[1]]); //공백을 지우고 소문자로 통일하는 함수
    }
    else
    {
        loadFileTo(_filename, answer[problemNum[0]][problemNum[1]], _dir); //.txt 파일을 읽어 내용을 가져오는 함수
    }
    /*standard 복원,*/
    dup2(stderrtmpDes, 2);
    dup2(stdouttmpDes, 1);
    /*파일 디스크립터 close()*/
    close(stderrtmpDes);
    close(stdouttmpDes);
    close(errtmpDes);
    /*임시 파일 삭제*/
    remove("stderrtmp");
    remove("stdouttmp");
    remove("errtmp");
}

void loadAns()
{
    struct dirent *folder, *file; //folder : <AND_DIR>의 정보를 저장하는 구조체, file : <ANS_DIR>의 하위 디렉토리의 정보를 저장하는 구조체
    char tmp[_buf_size];
    int index = 0;     //문제의 수를 저장할 변수
    DIR *f_dir = NULL; //<ANS_DIR>을 열기 위한 변수
    DIR *p_dir = NULL; //<ANS_DIR>의 하위 디렉토리를 열기 위한 변수

    memset(filesName, 0, sizeof(filesName));
    if ((f_dir = opendir(ansDir)) == NULL) //폴더가 존재하지 않는 경우
    {
        if (optionFlag[indefOfOpt('c')]) //만약 옵션c가 설정되어 있다면
        {
            opt_c(_RESULT_TABLE_NAME, c_list); //c_list의 항목을 파일에서 찾는 함수 호출
            exit(1);                           //프로그램 종료
        }
        write(1, "can not read ", 13); //에러 메세지 출력후
        write(1, stdDir, strlen(ansDir));
        write(1, "\n", 1);
        exit(0); // 프로그램 종료
    }

    while ((folder = readdir(f_dir)) != NULL)
    {
        if (folder->d_name[0] != '.') //. 및 .. 디렉토리 필터링
        {
            memset(tmp, 0, strlen(tmp));
            strcat(tmp, ansDir);
            strcat(tmp, "/");
            strcat(tmp, folder->d_name);
            if ((p_dir = opendir(tmp)) != NULL) //하위 디렉토리 오픈
            {
                while ((file = readdir(p_dir)) != NULL)
                {
                    if ((file->d_name[0] != '.') && (getExt(file->d_name) != -1)) //. 및 .. 디렉토리 필터링, 확장자가 c, txt일 파일만 if문 수행
                    {

                        strcat(filesName[index++], file->d_name); //정답 파일 이름
                        /*점수 파일 컴파일 및 */
                        /*정답 파일 내용 불러오기*/
                        complieAndLoadFile(file->d_name, tmp);
                    }
                }
            }
            closedir(p_dir);
        }
    }

    problemCnt = index; //문제의 수를 나타내는 전역 변수에 index 할당

    closedir(f_dir);
    /*문제를 정렬하는 버블 정렬*/
    for (int i = 0; i < index; i++)
    {
        for (int j = i + 1; j < index; j++)
        {
            if (cmpProblemNum(filesName[i], filesName[j]))
            {
                memset(tmp, 0, strlen(tmp));
                memcpy(tmp, filesName[i], strlen(filesName[i]));
                memset(filesName[i], 0, strlen(filesName[i]));
                memcpy(filesName[i], filesName[j], strlen(filesName[j]));
                memset(filesName[j], 0, strlen(filesName[j]));
                memcpy(filesName[j], tmp, strlen(tmp));
            }
        }
    }
}

int cmpProblemNum(char *_file1, char *_file2) //두 문제의 숫자를 비교하는 함수
{
    int num1[2], num2[2], comNum1, comNum2;
    getProblemNum(_file1, num1);
    getProblemNum(_file2, num2);
    comNum1 = num1[0] * 10 + num1[1];
    comNum2 = num2[0] * 10 + num2[1];
    return (comNum1 < comNum2) ? 0 : 1;
}

void getScore() // 점수를 가져와 저장하는 함수
{
    char scoreFile[_file_length] = "./"; //scoreFile : ./
    int scoreFileDes;                    //score_table 파일 디스크립터
    int problemNum[2];                   //문제의 번호 추출하기 위한 변수
    int index = 0;                       //p_buf의 index를 가리키기 위한 변수
    char buf[_buf_size];                 //파일에서 문제와 점수를 읽기 위한 변수
    char *p_buf[_MAX_STUDENT * 2];       //파싱된 문자열을 가리키는 변수
    char *tmp;

    strcat(scoreFile, ansDir);       //scoreFile : ./<ANS_DIR>
    strcat(scoreFile, "/");          //scoreFile : ./<ANS_DIR>/
    strcat(scoreFile, _score_table); //scoreFile : ./<ANS_DIR>/score_table.csv

    scoreFileDes = open(scoreFile, O_RDWR | O_CREAT | O_EXCL, 0777); //점수 파일 생성, 이미 존재한다면 -1 리턴

    memset(buf, 0, _buf_size);
    if (scoreFileDes != -1) //이미 존재하지 않으면
    {
        makeScoreTable(scoreFileDes); //점수 테이블을 만드는 함수 호출
    }
    else //빈 파일이 아니면 읽어 들인다.
    {
        if ((scoreFileDes = open(scoreFile, O_RDWR, 0777)) == -1) //다시 점수 테이블을 open()
        {
            fprintf(stderr, "error open score _ table \n");
            exit(1);
        }
        read(scoreFileDes, buf, _buf_size); //점수 테이블의 내용을 읽는다.

        tmp = strtok(buf, "\n,"); // 공백과 , 으로 파싱한다.
        p_buf[index++] = tmp;
        while (tmp = strtok(NULL, "\n,")) // 공백과 , 으로 파싱한다.
        {
            p_buf[index++] = tmp; //p_buf는 파싱된 각 문자열을 가지킨다.
        }
        index = 0;
        while (p_buf[index] != NULL) //p_buf는 문제번호 점수 문제번호 ... 점수 라는 규칙을 가지고 있다.
        {
            getProblemNum(p_buf[index], problemNum); //문제 번호 추출
            index++;                                 //점수로 이동
            if (p_buf[index] == NULL)                //점수가 없으면 테이블에 문제가 있다.
            {
                fprintf(stderr, "error on score table\n");
                exit(1);
            }
            score[problemNum[0]][problemNum[1]] = strtod(p_buf[index], &p_buf[index]); //strtod를 통해 문자열을 double 형으로 변경
            index++;                                                                   //다음 문제로 이동
        }
    }
    close(scoreFileDes);
}

void makeScoreTable(int _scoreFileDes) //테이블 생성 함수
{
    int scoreOpt = 0;
    write(1, "1. input blank question and program question`s score. ex) 0.5 , 1\n", 67);
    write(1, "2. input all question`s score. ex) input value of 1-1 : 0.5\n", 62);
    write(1, "select type >> ", 15);
    scanf("%d", &scoreOpt);
    if (scoreOpt == 1) //파일의 종류에 따른 점수입력
    {
        double blankScore, progScore;
        write(1, "input value blank question : ", 29);
        scanf("%lf", &blankScore);
        write(1, "input value program question : ", 31);
        scanf("%lf", &progScore);

        int n = 0;
        while (n < problemCnt)
        {
            int problemNum[2], flag;
            char *tmp;

            write(_scoreFileDes, filesName[n], strlen(filesName[n]));

            write(_scoreFileDes, " , ", 3);
            flag = getProblemSet(filesName[n], problemNum); //문제번호와 확장자 유형 얻기
            if (flag == 0)                                  //c 파일이라면
            {
                score[problemNum[0]][problemNum[1]] = progScore; //프로그램 점수 할당
                tmp = dtoa(progScore);
                write(_scoreFileDes, tmp, strlen(tmp));
            }
            if (flag == 1) // txt 파일이라면
            {
                score[problemNum[0]][problemNum[1]] = blankScore; //빈칸문제 점수 할당
                tmp = dtoa(blankScore);
                write(_scoreFileDes, tmp, strlen(tmp));
            }
            write(_scoreFileDes, "\n", 1);
            n++;
        }
    }
    else if (scoreOpt == 2) // 파일 각각에 대한 점수 입력
    {
        int index = -1;
        while (++index < problemCnt) //모든 문제에 대한 점수 직접 입력
        {
            int problemNum[2];
            char *strScore;
            getProblemNum(filesName[index], problemNum);
            printf("Input of %s :", filesName[index]);          //filesName의 점수 입력받기
            scanf("%lf", &score[problemNum[0]][problemNum[1]]); //filesName의 점수를 입력 받아 문제번호에 해당하는 전역변수에 저장

            strScore = dtoa(score[problemNum[0]][problemNum[1]]); //double형을 string으로 변환

            write(_scoreFileDes, filesName[index], strlen(filesName[index])); //문제 파일 이름을 출력
            write(_scoreFileDes, " , ", 3);
            write(_scoreFileDes, strScore, strlen(strScore)); //문제 점수를 출력
            write(_scoreFileDes, "\n", 1);
        }
    }
    else // 잘못된 옵션 입력시 다시 시도
    {
        system("clear");
        write(1, "input 1 or 2\n", 13);
        makeScoreTable(_scoreFileDes);
    }
}

int getProblemSet(char *filename, int num[2]) //문제 번호와 확장자 추출
{
    getProblemNum(filename, num); //문제 번호 num에 저장
    return getExt(filename);      // 확장자 유형 리턴
}
void getProblemNum(const char *filename, int num[2]) //문제 번호 추출
{
    char tmpName[_file_length];
    char *ext;
    memset(tmpName, 0, sizeof(tmpName));
    memcpy(tmpName, filename, strlen(filename));
    ext = strtok(tmpName, "-."); //-과 .을 기준으로 파싱하여 숫자를 추출한다.
    num[0] = atoi(ext);          //앞 숫자를 num[0]에 할당
    ext = strtok(NULL, "-.");
    if (isNumber(ext))      //숫자라면
        num[1] = atoi(ext); //num[1]에 해당 번호 할당
    else                    //하위 문제가 없는 문제,
        num[1] = 0;         //num[1]에 0 할당
}
int getExt(const char *filename)
{
    char tmpName[_file_length];
    char *ext;
    memset(tmpName, 0, sizeof(tmpName));
    memcpy(tmpName, filename, strlen(filename));
    ext = strtok(tmpName, "."); //. 기준으로 파싱한다,
    if (ext == NULL)
        return -1;
    ext = strtok(NULL, "."); //strtok()을 두번 호출해야 확장자를 가리킨다.
    if (ext == NULL)
        return -1;
    if (strcmp(ext, "c") == 0) //확장자가 c 면 0
        return 0;
    if (strcmp(ext, "txt") == 0) //확장자가 txt 면 1
        return 1;
    return -1; // 그 외의 확장자는 -1을 리턴한다.
}
char *getName(const char *filename) //파일의 확장자를 제거한 이름을 얻는다.
{
    char tmpName[_file_length];
    char *ext;
    memset(tmpName, 0, sizeof(tmpName));
    memcpy(tmpName, filename, strlen(filename));
    ext = strtok(tmpName, "."); //.을 한번 호출하면 확장자를 제외한 이름을 추출할 수 있다.
    return ext;
}
char *dtoa(double _a) //double to String
{
    char *res = (char *)malloc(sizeof(char) * 100);
    sprintf(res, "%0.2lf", _a);
    return res;
}
char *itoa(int _a) //int to String
{
    char *res = (char *)malloc(sizeof(char) * 100);
    sprintf(res, "%d", _a);
    return res;
}
int isNumber(char *_str) //str이 숫자인지 확인
{
    if (('0' <= _str[0]) && (_str[0] <= '9'))
        return 1;
    return 0;
}
void cToExe(char *_filename, char *_dir) // c파일을 exe 파일로 만들기,
{
    char output[_file_length];
    int isLptread = 0;

    if (optionFlag[indefOfOpt('t')])
        for (int i = 0; i < 5; i++)
            if (atoi(getName(_filename)) == t_list[i])
            {
                isLptread = 1;
                break;
            }

    memset(output, 0, sizeof(output));
    strcat(output, "gcc ");             //output = "gcc "
    strcat(output, _dir);               //output = "gcc  <_dir>"
    strcat(output, "/");                //output = "gcc  <_dir>/"
    strcat(output, _filename);          //output = "gcc  <_dir>/<_filename>"
    strcat(output, " -o ");             //output = "gcc  <_dir>/<_filename> -o "
    strcat(output, _dir);               //output = "gcc  <_dir>/<_filename> -o <_dir>"
    strcat(output, "/");                //output = "gcc  <_dir>/<_filename> -o <_dir>/"
    strcat(output, getName(_filename)); //output = "gcc  <_dir>/<_filename> -o <_dir>/<NAME>" <NAME> <- 파일의 확장자를 제외한 이름
    strcat(output, _exe);               //output = "gcc  <_dir>/<_filename> -o <_dir>/<NAME>.exe"
    if (isLptread)                      //-t 옵션이 적용된 문제라면
        strcat(output, " -lpthread ");  //output = "gcc  <_dir>/<_filename> -o <_dir>/<NAME>.exe -lpthread"
    system(output);
}
void exeToFile(char *_exeFileName, char *_dir) // exe파일을 파일에 출력 만들기,
{
    struct timeval begin_t, end_t;
    char toFileC[_file_length];
    char toFile[_file_length];
    char command[_file_length];
    char path[_file_length];
    int fileDes;

    memset(path, 0, sizeof(path));
    memset(command, 0, sizeof(command));
    memset(toFile, 0, sizeof(toFile));
    memset(toFileC, 0, sizeof(toFileC));

    strcat(command, "timeout "); //command = "timeout "
    strcat(command, itoa(5));    //5초 이상 수행될 시 종료
    strcat(command, "s ");       //command = "timeout 5s"
    strcat(path, _dir);          //path = "<_dir>"
    strcat(path, "/");           //path = "<_dir>/"

    strcat(path, getName(_exeFileName)); //path = "<_dir>/<NAME>"

    memcpy(toFile, path, strlen(path));
    memcpy(toFileC, path, strlen(path));
    strcat(toFile, _out);  //toFile = "<_dir>/<NAME>.stdout"
    strcat(toFileC, _exe); //toFileC = "<_dir>/<NAME>.exe"

    if ((fileDes = open(toFile, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1) //.stdout 파일 오픈
    {
        fprintf(stderr, "open file error at exe to file()"); //오픈 error 여도 메세지만 출력
    }
    else
    {

        dup2(fileDes, 1);             //표준 출력을 파일로 변경
        strcat(command, toFileC);     //command = "timeout 5s <_dir>/<NAME>.exe"
        gettimeofday(&begin_t, NULL); // 학생의 프로그램 시작 시간 기록.
        system(command);
        gettimeofday(&end_t, NULL);      // 학생 프로그램 종료 시간 기록
        ssu_runtime(&begin_t, &end_t);   //두 시간의 차를 ent_t에 저장
        if (end_t.tv_sec >= _time_limit) //시간차가 _time_limit이상이면 timeout error 메세지 출력한다.
        {
            fprintf(stderr, "time out error : %ld\n", end_t.tv_sec);
        }
        fsync(fileDes); //파일에 대한 출력을 마무리 시킨다.
        close(fileDes);
    }
}
void loadFileTo(char *_filename, char *_to, char *_dir) //파일을 읽어서 _to에 저장한다.
{
    int des, cnt = 0, i;     //des는 파일 디스크립터, cnt는 읽은 문자열의 총 길이,
    char c;                  //문자를 읽기 위한 charactor 형 변수
    char path[_file_length]; //파일의 경로 저장

    memset(path, 0, sizeof(path));
    strcat(path, _dir);
    strcat(path, "/");
    strcat(path, _filename);

    if ((des = open(path, O_RDONLY)) == -1)
    {
        fprintf(stderr, "open file error %s\n", path);
    }
    else
    {
        lseek(des, 0, SEEK_SET); //파일의 시작으로 이동
        while (read(des, &c, 1) > 0)
        { //파일에서 하나씩 읽어 버퍼에 저장한다.
            _to[cnt++] = c;
            if (cnt == _buf_size) //버퍼가 꽉차면 종료한다.
                break;
        }
        _to[cnt] = 0; //문자열의 마지막을 0으로 설정한다.
        close(des);
    }

    if (cnt > 0)
    {
        if ((_to[cnt - 1] == ' ') || (_to[cnt - 1] == '\n')) //마지막 문자열이 공백이거나 개행미녀 지운다.
            _to[cnt - 1] = 0;
    }
}
void removeBlankAndLower(char *_at, char *_to)
{
    int index1 = 0, index2 = 0; //_to -> index1, _at -> index2;
    int dif = 'A' - 'a';        //dif에 'A'의 아스키 코드 에서 'a'의 아스키 코드 값을 뺀 값을 저장한다.

    while (_at[index2] != 0) //문자열의 끝에 도다랄 때 까지 반복
    {
        if (_at[index2] == ' ') //공백이면 _at의 index만 증가시킨다.
        {
            index2++;
            continue;
        }
        if (('A' <= _at[index2]) && (_at[index2] <= 'Z')) //문자가 재문자라면 dif를 뺴준다.
        {
            _at[index2] -= dif;
        }
        _to[index1++] = _at[index2++]; //_to에 _at을 복사한다.
    }
    while (index1 < _buf_size) //남은 부분을 0으로 채운다.
    {
        _to[index1++] = 0;
    }
}

void loadStd()
{
    char path[_buf_size];
    struct dirent *folder, *file;  //디렉토리, 파일을 가리키는 dirent 구조체
    DIR *f_dir;                    //<STD_DIR> 을 나타내는 DIR 구조체포인터
    DIR *p_dir;                    //<STD_DIR> 의 하위 폴더를 나타내기 위한 DIR 구조체 폴더
    int index = 0;                 //학생의 수를 나타내기 위한 변수
    int scorefd;                   //학생의 점수 정보를 저장하는 파일의 디스크립터
    double ever = 0;               //p 옵션이 설정되어 있을 때 평균을 구하기 위한 변수
    double sum = 0;                //학생의 점수를 모두 더하기 위한 변수
    char stdAns[_buf_size];        // 학생이 제출한 답
    char resultPath[_file_length]; //경과 파일을 저장할 변수
    int problemNum[2];             //문제의 번호를 위한 변수
    char *scoreStr;
    char tmp[_buf_size];

    memset(resultPath, 0, sizeof(resultPath));

    strcat(resultPath, "./");
    strcat(resultPath, _RESULT_TABLE_NAME);
    /* score 파일 오픈 error 처리*/
    if ((scorefd = open(resultPath, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1)
    {
        fprintf(stderr, "error open score file\n");
        exit(1);
    }
    /*문제 파일 이름을 score 파일에 출력, */
    for (int i = 0; i < problemCnt; i++)
    {
        write(scorefd, ",", 1);
        write(scorefd, filesName[i], strlen(filesName[i]));
    }
    write(scorefd, ",", 1);
    write(scorefd, "sum", 3); //점수를 모두 더한 값도 파일에 저장
    write(scorefd, "\n", 1);

    /*<STR_DIR> 폴더 열기*/
    if ((f_dir = opendir(stdDir)) == NULL) //폴더 open에 error가 있다면
    {
        if (optionFlag[indefOfOpt('c')]) //옵션 c가 설정되어 있다면
        {
            opt_c(_RESULT_TABLE_NAME, c_list); //score 파일에서 c_list를 찾아 출력하는 함수 호출
            exit(1);                           //프로그램 종료
        }
        write(1, "can not read ", 13); //error msg 출력
        write(1, ansDir, strlen(ansDir));
        write(1, "\n", 1);
        exit(0); //프로그램 종료
    }
    while ((folder = readdir(f_dir)) != NULL) //<STD_DIR> 폴더를 openDir() 함수로 얻은 DIR 구조체를 이용해 readdir();
    {
        if (folder->d_name[0] != '.') //.과 .. 디렉토리 필터링
        {
            memset(path, 0, sizeof(path));
            strcat(path, stdDir);         //path : "<STD_DIR>"
            strcat(path, "/");            //path : "<STD_DIR>/"
            strcat(path, folder->d_name); //path : "<STD_DIR>/<FOLDER_NAME>"
            memset(studentList[index], 0, sizeof(studentList[index]));
            if ((p_dir = opendir(path)) != NULL) //<STD_DIR> 폴더의 하위 폴더를 openDir();
            {
                strcat(studentList[index++], folder->d_name); //에러가 나지 안으면 <STD_DIR> 하위 폴더의 이름을 studentList에 저장
            }
        }
    }
    studentCount = index; //studentCount 에 학생 수 저장
    closedir(f_dir);      //openDir한 directory close,
    closedir(p_dir);      //openDir한 directory close,
    /*학번을 정렬*/
    for (int i = 0; i < index; i++)
    {
        for (int j = i + 1; j < index; j++)
        {
            if (strcmp(studentList[i], studentList[j]) > 0)
            {
                memset(tmp, 0, strlen(tmp));
                strcat(tmp, studentList[i]);
                memset(studentList[i], 0, strlen(studentList[i]));
                strcat(studentList[i], studentList[j]);
                memset(studentList[j], 0, strlen(studentList[j]));
                strcat(studentList[j], tmp);
            }
        }
    }
    /*학번 디렉토리를 순회하며 정답 채점*/
    for (int i = 0; i < index; i++)
    {
        memset(path, 0, sizeof(path));
        strcat(path, stdDir);                                   //path ; "<STD_DIR>"
        strcat(path, "/");                                      //path ; "<STD_DIR>/"
        strcat(path, studentList[i]);                           //path ; "<STD_DIR>/<STUDNET_NAME>"
        write(1, studentList[i], strlen(studentList[i]));       //화면에 학번 출력
        write(scorefd, studentList[i], strlen(studentList[i])); //파일에 학변 출력
        write(scorefd, ",", 1);

        if ((p_dir = opendir(path)) != NULL) //폴더를 opendir();
        {
            while ((file = readdir(p_dir)) != NULL) //폴더의 항목을 읽기,
            {
                if ((file->d_name[0] != '.') && (getExt(file->d_name) != -1)) //. 과 .. 이 아니고 확장자가 c 와 txt 라면
                {
                    memset(stdAns, 0, sizeof(stdAns));
                    complieAndLoadFileStd(file->d_name, path, stdAns, i); //컴파일 및 파일읽기, 그리고 문자열을 비교하는 채점 함수 호출
                }
            }
        }
        /*한 학생의 채점이 끝나면*/
        sum = 0;
        for (int j = 0; j < problemCnt; j++) //문제 리스트에 해당되는 학생의 점수를 파일에 출력
        {
            getProblemNum(filesName[j], problemNum);
            scoreStr = dtoa(studentScore[i][problemNum[0]][problemNum[1]]); //double형을 string으로 변환하여 scoreStr에 저장
            sum += studentScore[i][problemNum[0]][problemNum[1]];           //합계 계산
            write(scorefd, scoreStr, strlen(scoreStr));                     //파일에 합계출력
            write(scorefd, ",", 1);
        }
        write(1, " complete", 9);        //완료되었다는 msg 화면에 출력
        scoreStr = dtoa(sum);            //double형을 string으로 변환하여 scoreStr에 저장
        if (optionFlag[indefOfOpt('p')]) //p 옵션이 설정되어 있으면
        {
            ever += sum; //평균 계산을 위해 sum을 더하고
            write(1, " ... ", 5);
            write(1, scoreStr, strlen(scoreStr)); //현제 학생의 점수를 화면에 출력
        }
        write(scorefd, scoreStr, strlen(scoreStr)); //합계를 파일에 출력
        write(scorefd, "\n", 1);
        write(1, "\n", 1);
    }
    if (optionFlag[indefOfOpt('p')]) //재첨에 모두 끝난 후 p 옵션이 설정되어 있다면
    {
        ever = ever / studentCount;               //평균을 계산하고
        printf("Total average : %0.2lf\n", ever); // 평균을 출력
        fflush(stdout);
    }
    close(scorefd);
}

void complieAndLoadFileStd(char *_filename, char *_dir, char *_res, int _index)
{
    int ext = getExt(_filename);                                              //파일의 확장자에 해당되는 번호 얻기
    int problemNum[2];                                                        //문제 번호를 알기위한 위한 배열
    int errorfd;                                                              //error파일을 가리키는 파일 디스크립터
    off_t errorFileSize;                                                      // error 파일의 size 를 알기 위한 변수
    char outFile[_file_length];                                               //출력된 파일이름
    char name[_file_length];                                                  //문제의 이름을 저장하는 변수
    char errorFile[_file_length];                                             //error 파일의 경로
    int stderrtmpDes = open("stderrtmp", O_WRONLY | O_CREAT | O_TRUNC, 0766); //stderr 임시저장 할 파일
    int stdouttmpDes = open("stdouttmp", O_WRONLY | O_CREAT | O_TRUNC, 0766); //stdout 임시저장
    DIR *checkValue;                                                          //<ERR_DIR>에 _index에 해당하는 학번 폴더가 있는지 검사하기 위한 변수

    memset(name, 0, sizeof(name));
    strcat(name, getName(_filename));
    getProblemNum(_filename, problemNum);

    if (optionFlag[indefOfOpt('e')]) //e 옵션이 설정되어 있다면
    {
        memset(errorFile, 0, sizeof(errorFile));
        strcat(errorFile, errDir);              //errorFile : <ERR_DIR>
        strcat(errorFile, "/");                 //errorFile : <ERR_DIR>/
        strcat(errorFile, studentList[_index]); //errorFile : <ERR_DIR>/<STUDENT_NAME>

        if ((checkValue = opendir(errorFile)) == NULL) //<ERR_DIR>/<STUDENT_NAME> 가 존재하지않으면
            mkdir(errorFile, 0777);                    //디렉토리 생성
        else
            closedir(checkValue); //closedIr()

        strcat(errorFile, "/");          //errorFile : <ERR_DIR>/<STUDENT_NAME>/
        strcat(errorFile, name);         //errorFile : <ERR_DIR>/<STUDENT_NAME>/<PROBLEM_NAME>
        strcat(errorFile, "_error.txt"); //errorFile : <ERR_DIR>/<STUDENT_NAME>/<PROBLEM_NAME>_error.txt
    }
    else //e옵션이 설정되어 있지 않으면
    {
        memset(errorFile, 0, sizeof(errorFile));
        strcat(errorFile, "errtmp"); //errorFile : errtmp
    }
    if ((errorfd = open(errorFile, O_RDWR | O_CREAT | O_TRUNC, 0777)) == -1) //error 를 출력할 파일 open
    {
        fprintf(stderr, "open error file %s\n", errorFile); //에러를 출력할 파일의 open()에 실패하면
        exit(0);                                            //프로그램을 종료한다.
    }

    dup2(2, stderrtmpDes); //strerr을 임시저장
    dup2(1, stdouttmpDes); //strout을 임시저장
    dup2(errorfd, 2);      //strerr을 errorfd로 대체
    memset(outFile, 0, sizeof(outFile));
    memcpy(outFile, name, sizeof(name));

    if (ext == 0) //확장자가 c 라면
    {
        cToExe(_filename, _dir);    //점수 파일 컴파일
        exeToFile(_filename, _dir); //컴파일된 파일 파일출력,

        loadFileTo(strcat(outFile, _out), _res, _dir); //출력 파일의 내용을 _res에 저장한다.
        removeBlankAndLower(_res, _res);               //대문자를 소문자로 바꾸고 공백을 제거한다.
        if (findError(errorFile) == 1)                 //만약 error 파일에 error 메세지가 존재한다면
        {
            studentScore[_index][problemNum[0]][problemNum[1]] = _error; //현제문제의 점수를 _error 로 설정
        }
        else if (strlen(_res) == 0) //읽어들인 파일의 길이가 0 이라면
        {
            studentScore[_index][problemNum[0]][problemNum[1]] = 0; //점수는 0점이다.
        }
        else
        {
            if (simpleCmp(_res, answer[problemNum[0]][problemNum[1]])) //정답과 학생의 파일 내용을 비교한다.
            {
                studentScore[_index][problemNum[0]][problemNum[1]] = score[problemNum[0]][problemNum[1]]; //정답과 같다면 studentScore[학생번호][문제번호1][문제번호2] 에 점수 저장
                studentScore[_index][problemNum[0]][problemNum[1]] += findWarning(errorFile);             //warning이 발생한 만큼 감점
            }
            if (studentScore[_index][problemNum[0]][problemNum[1]] < 0) //점수가 0보다 작아진다면
                studentScore[_index][problemNum[0]][problemNum[1]] = 0; //0점으로 저장
        }
    }
    else if (ext == 1) //확장자가 txt 라면
    {
        loadFileTo(_filename, _res, _dir);                          // 파일의 내용을 _res에 저장한다.
        if (complexCmp(answer[problemNum[0]][problemNum[1]], _res)) //연산자에 따른 다수의 정답이 가능하기 떄문에 복잡한 비교를 한다.
        {
            studentScore[_index][problemNum[0]][problemNum[1]] = score[problemNum[0]][problemNum[1]]; //정답이면 점수 저장
        }
        else
        {
            studentScore[_index][problemNum[0]][problemNum[1]] = 0; //아니면 0점
        }
    }

    /*standard 복원*/
    dup2(stderrtmpDes, 2);
    dup2(stdouttmpDes, 1);

    if (optionFlag[indefOfOpt('e')]) //e 옵션이 설정되어 있을 떄
    {
        if ((errorFileSize = lseek(errorfd, 0, SEEK_END)) == -1) //lseek으로 파일의 길이를 구한다
        {
            fprintf(stderr, "error lseek\n");
            exit(1);
        }
        close(errorfd);
        if (errorFileSize == 0) //파일의 길이가 0 이라면
        {
            unlink(errorFile); //파일을 지운다
        }
    }
    else //e 옴셥이 없다면
    {
        close(errorfd);
        remove(errorFile); //임시 error 파일을 지운다.
    }

    /*파일 디스크립터 종료*/
    close(stderrtmpDes);
    close(stdouttmpDes);
    /*임시파일들 삭제*/
    remove("stderrtmp");
    remove("stdouttmp");
    remove("errtmp");
}

int simpleCmp(char *_str1, char *_str2) //두 string을 단순 비교
{
    if ((_str1[0] == 0) || (_str2[0] == 0))
        return 0;
    if (strcmp(_str1, _str2) == 0)
        return 1;
    return 0;
}

int findError(const char *pathname) //Error 파일에 error메세지가 있으면 0점 처리
{
    const char errmsg[] = "error";           //error 파일에서 찾을 msg
    const int fd = open(pathname, O_RDONLY); //인자로 받은 error 파일 open()
    char c;                                  //error 파일에서 문자열을 하나식 찾기 위한 변수
    int index = 0;                           //errmsg의 하나의 문자를 가리키기 위한 변수

    if (fd < 0)                 //error파일의 open()이 실패하면
        return 0;               //실패를 리턴
    while (read(fd, &c, 1) > 0) //파일에서 문자를 하나씩 읽는다.
    {
        if (errmsg[index] == c) //errmsg의 index번 문자와 윌치하면
        {
            index++; //index 증가
        }
        else
        {
            index = 0; //일치하지 않으면 index는 0으로 돌아가 처믕부터 다시 비교한다.
        }
        if (index == 5) //원하는 msg를 찾은 경우
        {
            close(fd); //파일을 닫고
            return 1;  //성공을 리턴한다.
        }
    }
    close(fd); //찾이 못했다면 파일을 닫고
    return 0;  //실패를 리턴한다.
}

double findWarning(const char *pathname) // error 파일에서 Warning이 있으면 감점되는 점수 return
{
    const char errmsg[] = "warning";         //찾을 msg
    const int fd = open(pathname, O_RDONLY); //인자로 받은 error 파일 open()
    char c;                                  //파일에서 하나의 문자을 얻기 위한 변수
    int index = 0;                           //errmsg를 하나씩 빅교하기 위한 변수이다.
    double res = 0;                          //리턴할 점수

    if (fd < 0)                 //error 파일 open()가 실패하면
        return 0;               //함수종료
    while (read(fd, &c, 1) > 0) //error 폴더에서 문자열을 하나씩 입력받아 warning이 있는지 검사한다.
    {
        if (errmsg[index] == c) //index는 지금까지 일치한 문자의 개수이다.
        {
            index++; //index의 문자가 일치하면 index++
        }
        else
        {
            index = 0; //index의 문자가 일치하지 않으면 index 는 0으로 돌아가게 된다.
        }
        if (index == 7) //errmsg가 모두 일치하면
        {
            res += _warning; //감점을 위해 warning 점수를 res에 추가한다.
            index = 0;       //index는 0으로 돌아간다.
        }
    }
    close(fd);
    return res; //warning 점수의 함을 리턴한다, warninrg 개수에 따라 다르다.
}

int complexCmp(char *_str1, char *_str2) // _str1 정답 , _str2 학생, 빈칸 채우기 채점하는 함수
{
    int strCnt = 0;            //token으로 분리된 문자열의 개수를 저장 할 변수
    char cmpstr1[_buf_size];   //정답 파일의 문자열을 복사 할 변수, strtok()을 수행하면 문자열이 변형 되기 때문에 문자열을 복사하여 strtok() 수행
    char *paramStr1[1024];     //expCmp의 첫번째 인자로 전달할 문자열, strtok()으로 분리된 문자열 배열이다.
    char paramStr2[_buf_size]; //_str2문자열을 복사할 변수
    char *tmp;
    memset(cmpstr1, 0, sizeof(cmpstr1));
    memset(paramStr1, 0, sizeof(paramStr1));
    memset(paramStr2, 0, sizeof(paramStr2));

    if ((_str1 == NULL) || (_str2 == NULL)) //문자열이 존재하지 않으면 실패를 리턴
        return 0;
    if ((_str1[0] == 0) || (_str2[0] == 0)) //문자열이 길이가 0 이면 실패를 리턴,
        return 0;

    strcat(cmpstr1, _str1);   //정답파일의 문자열을 cmpstr1에 복사
    strcat(paramStr2, _str2); //학생이 제출한 답인 _str2를 paramStr2에 저장

    tmp = strtok(cmpstr1, ":"); //cmpstr1을 : 기준으로 파싱
    paramStr1[strCnt++] = tmp;  //파싱된 문자열을 가리키는 paramStr1[];

    while ((tmp = strtok(NULL, ":")) != NULL) //cmpstr1을 : 기준으로 파싱
    {
        paramStr1[strCnt++] = tmp; //파싱된 문자열을 가리키는 paramStr1[];
    }
    for (int i = 0; i < strCnt; i++) //파싱된 문자열을 순회하며 학생의 답과 비교
    {
        if (expCmp(paramStr1[i], paramStr2) == 1) //두 문자열을 비교해서 같으면 1을 리턴
        {
            return 1;
        }
    }

    return 0; //모든 경우를 고려해도 정답이 아니라면 0 리턴
}
