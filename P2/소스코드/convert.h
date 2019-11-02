#define HEADER_FILE "header"
#define MAKE_FILE "Makefile"
#define MAX_LENGTH 1024

char *javaBuf;                           //변환 할 java 내용 저장
int javaBufSize;                         //javaBuf의 크기
char *cFileBuf;                          //변환 된 내용 저장
int cFileBufSize;                        //cFileBuf의 크기
char *opt_p_buf;                         //p옵션을 위한 버퍼
int opt_p_buf_size;                      //opt_p_buf 의 크기
char cFileList[MAX_LENGTH][MAX_LENGTH];  //변환 된 c 파일 list
int cFileListSize;                       //변환된 c file 개수
char removeList[MAX_LENGTH][MAX_LENGTH]; //지워야 할 변수 목록
int removeListSize;                      //지워야할 변수 개수
char *datatype[] = {                     //기본 데이터 타입
    "int",
    "char",
    "short",
    "long",
    "float",
    "double"};
const int datatypeSize = 6; //기본 데이터 타입 개수

int isIoExcept; //ioException이 설정 되어 있는지 확인

void init();                                                            //전역 변수 초기화
void insertToBuf(char **_buf, int *_bufsize, char *_str, int isHeader); //_buf에 _std을 이어 붙인다.
void convertFunc(char *_javaFile);                                      //_javaFile을 변환하는 함수이다.
void convertToC(int *_cfd, char *_str);                                 //한 줄을 변환하는 함수이다.
char *getNextLine(int _fd);                                             //_fd 의 다음 줄을 읽어서 반한한다.
char *removeStr(char *_str, char *_rmStr, int flag);                    //_str에서 _rmStr을 지우는 함수이다.
void setHeader(char *funcName);                                         //function에 해당하는 header를 table에서 읽어서
void make_makefile(char *_javaFile);                                    //make file을 만든다.

/*_javaFile을 변환 하는 함수이다.*/
void convertFunc(char *_javaFile)
{
    int javafd = open(_javaFile, O_RDONLY); //java 파일을 open, file descripter 할당
    int cfd = -1;                           //c 파일을 가리키는 filedescripter
    int i = 0;                              //반복문을 위한 변수
    char *next;                             //다음 문자열을 가리키는 변수

    /*java 파일 open error 시*/
    if (javafd < 0)
    {
        fprintf(stderr, "open file error %s\n", _javaFile);
        exit(1);
    }

    /*파일의 마지막 까지 한 줄 가져온다.*/
    while ((next = getNextLine(javafd)) != NULL)
    {
        /*파일의 끝이면 while 종료*/
        if (strlen(next) == 0)
            break;
        insertToBuf(&javaBuf, &javaBufSize, next, 0); //javaBuf에 읽은 내용 저장
        convertToC(&cfd, next);                       //다음 내용을 변환한다.
        /*r 옵션이 설정되어 있으면 opt_r 함수 수행*/
        if ((optFlag[getOptFlag('r')] == 1) && (cFileListSize > -1))
            opt_r(_javaFile, cFileList[cFileListSize], javaBuf, cFileBuf);
    }
    /*변환이 끝났을 때 c 파일이 open 된 상태라면*/
    if (cfd != -1)
    {
        write(cfd, cFileBuf, strlen(cFileBuf));                             //cFileBuf의 내용을 c 파일에쓰고
        printf("%s.c converting is finished!\n", cFileList[cFileListSize]); //변환이 끝났다는 msg 출력
        memset(cFileBuf, 0, sizeof(cFileBuf));
        close(cfd); //파일을 close 한다
    }
    make_makefile(_javaFile); // c 파일을 컴파일 하기 위한 makefile을 만든다.
    /*option 처리*/
    /*j 옵션*/
    if (optFlag[getOptFlag('j')] == 1)
    {
        opt_j(javaBuf);
    }

    /*c 옵션*/
    if (optFlag[getOptFlag('c')] == 1)
    {
        for (i = 0; i <= cFileListSize; i++)
        {

            opt_c(cFileList[i]);
        }
    }
    /*f 옵션*/
    if (optFlag[getOptFlag('f')] == 1)
    {
        opt_f(_javaFile);
        next = (char *)calloc(sizeof(char), MAX_LENGTH);
        for (i = 0; i <= cFileListSize; i++)
        {
            sprintf(next, "%s.c", cFileList[i]);
            opt_f(next);
        }
    }
    /*l 옵션*/
    if (optFlag[getOptFlag('l')] == 1)
    {
        opt_l(_javaFile);
        next = (char *)calloc(sizeof(char), MAX_LENGTH);
        for (i = 0; i <= cFileListSize; i++)
        {
            sprintf(next, "%s.c", cFileList[i]);
            opt_l(next);
        }
    }
    /*p 옵션*/
    if (optFlag[getOptFlag('p')] == 1)
    {
        opt_p(opt_p_buf);
    }
}
void convertToC(int *_cfd, char *_str)
{
    int i = 0, j = 0;                                          //반복문을 위한 변수
    int tabCnt = 0, squareCnt = 0;                             //tab 수와 함수의 '(' 저장
    static int sqaure = 0;                                     //'{' 수 저장
    char *cFile = (char *)calloc(sizeof(char), MAX_LENGTH);    //c 파일 이름 저장
    char *keyWordCheck;                                        //찾는 string의 시작 부분을 가리킨다.
    char *token, *token2;                                      //tokenizer를 위한 변수이다.
    char *content;                                             //함수의 parameter를 저장하는 변수이다.
    char *rmPublicStr = removeStr(_str, "public ", 0);         //_str에서 public 키워드를 지운 string이다.
    char *writeBuf = (char *)calloc(sizeof(char), MAX_LENGTH); //c 파일에 쓸 변수를 저장한다.
    char *p_tmp = (char *)calloc(sizeof(char), MAX_LENGTH);    //p옵션을 위한 string 변수이다.
    char *tmp;

    /*calloc 으로 할당 된 변수를 초기화 한다.*/
    memset(cFile, 0, sizeof(cFile));
    memset(writeBuf, 0, sizeof(writeBuf));
    memset(p_tmp, 0, sizeof(p_tmp));

    /*static final -> #defin*/
    if ((keyWordCheck = strstr(rmPublicStr, "static final ")) != NULL)
    {
        token = (char *)calloc(sizeof(char *), MAX_LENGTH); //token에 새로운 memory를 할당.
        keyWordCheck += 13;                                 //keyword를 static final 다음으로 이동한다.
        token2 = strtok(keyWordCheck, " =;");               //공백, 연산자, 세미콜론 을 기준으로 분리한다.
        /*token 은 datatype을 가리킨다.*/
        /*NULL일 경우 error, 건너 뛴다.*/
        if (token2 == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        token2 = strtok(NULL, " =;"); //공백, 연산자, 세미콜론 을 기준으로 분리한다.
        /*token 은 변수를 가리킨다.*/
        /*NULL일 경우 error, 건너 뛴다.*/
        if (token2 == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        keyWordCheck = token2;        //keyWordCheck은 변수 이름을 가리킨다.
        token2 = strtok(NULL, " =;"); //공백, 연산자, 세미콜론 을 기준으로 분리한다.
        /*token 은 할당할 상수를 가리킨다.*/
        /*NULL일 경우 error, 건너 뛴다.*/
        if (token2 == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }

        /*define 형태로 변경한다.*/
        sprintf(token, "#define %s %s", keyWordCheck, token2);
        /*rmPublicStr의 문자열을 바꾼다.*/
        rmPublicStr = token;
    }

    rmPublicStr = removeStr(rmPublicStr, "final ", 1); //final 키워드를 지운다.
    if (rmPublicStr == NULL)                           //NULL일 경우 error 가 발생.
    {
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    /*자바 파일의 tab 수를 저장한다.*/
    i = 0;
    while ((i < strlen(rmPublicStr)) && (rmPublicStr[i++] == '\t'))
        tabCnt++;

    /*대괄호의 수를 맞추기 위해 square 수를 저장한다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "{")) != NULL)
        sqaure++;
    if ((keyWordCheck = strstr(rmPublicStr, "}")) != NULL)
        sqaure--;
    /*class 키워드를 만나면 새로운 c 파일을 생성한다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "class")) != NULL)
    {
        keyWordCheck = strtok(rmPublicStr, " ");
        if (keyWordCheck == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        keyWordCheck = strtok(NULL, " ");
        if (keyWordCheck != NULL)
        {
            /*공백과 여는 대괄호를 제거한다.*/
            while (i < strlen(keyWordCheck))
            {
                if (keyWordCheck[i] == '\n' || keyWordCheck[i] == '{')
                {
                    keyWordCheck[i] = 0;
                    break;
                }
                i++;
            }

            /*c 파일을 추가한다.*/
            memset(cFileList[++cFileListSize], 0, sizeof(cFileList[cFileListSize]));
            strcat(cFileList[cFileListSize], keyWordCheck);
            strcat(cFile, keyWordCheck);
            strcat(cFile, ".c");
            /*이미 열려있는 c 파일이 존재하는 경우*/
            if ((*_cfd) > 0)
            {
                write((*_cfd), cFileBuf, strlen(cFileBuf));                             //파일에 buffer를 저장하고
                printf("%s.c converting is finished!\n", cFileList[cFileListSize - 1]); //변환 되었다는 msg를 띄운다.
                memset(cFileBuf, 0, sizeof(cFileBuf));                                  //초기화를 한다.
                sqaure = 0;
                close((*_cfd)); //파일을 종료한다.
            }
            /*새로운 파일을 open 힌다*/
            (*_cfd) = open(cFile, O_RDWR | O_CREAT | O_TRUNC, 0666);
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
    }

    /*쓸 파일이 없으면 함수를 종료한다.*/
    if ((*_cfd) < 0)
    {
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }

    token = (char *)calloc(sizeof(char *), MAX_LENGTH);
    token2 = (char *)calloc(sizeof(char *), MAX_LENGTH);
    /*지워야 할 변수가 있다면 지운다.*/
    for (i = 0; i < removeListSize; i++)
    {
        sprintf(token, "%s.", removeList[i]);
        while ((keyWordCheck = strstr(rmPublicStr, token)) != NULL)
        {
            (*keyWordCheck) = 0;
            /*token2에 변수를 지운 새로운 문자열을 만든다.*/
            sprintf(token2, "%s%s", rmPublicStr, keyWordCheck + strlen(token));
            /*token이 새로운 문자열이 된다.*/
            rmPublicStr = token2;
        }
    }

    /*main 함수를 변환 한다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "static void main")) != NULL)
    {
        insertToBuf(&cFileBuf, &cFileBufSize, "void main(int argc, char *argv[])", 0);
        /*{가 존재하면 뒤에 붙여 쓴다.*/
        if ((keyWordCheck = strstr(rmPublicStr, "{")) != NULL)
        {
            insertToBuf(&cFileBuf, &cFileBufSize, "{", 0);
        }
        insertToBuf(&cFileBuf, &cFileBufSize, "\n", 0);
        /*IOException이 설정 되어 있다면 변수를 통해 알린다.*/
        if ((keyWordCheck = strstr(rmPublicStr, "IOException")) != NULL)
            isIoExcept = 1;
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        /*main 변환 완료*/
        return;
    }
    /*출력 함수를 변환 한다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "System.out.print")) != NULL)
    {
        token = (char *)calloc(sizeof(char *), MAX_LENGTH);
        memset(token, 0, sizeof(token));
        squareCnt = 0;
        /*함수의 paramter를 구한다.*/
        for (i = 0, j = 0; i < strlen(keyWordCheck); i++)
        {
            if (keyWordCheck[i] == ')')
            {
                squareCnt--;
            }
            if (squareCnt > 0)
            {
                token[j++] = keyWordCheck[i];
            }
            if (keyWordCheck[i] == '(')
            {
                squareCnt++;
            }
        }
        /*print의 인자가 +로 연결 되어 있을 경우*/
        /*+로 토큰을 분리하여 처리한다.*/
        token2 = strtok(token, "+");
        if (token2 == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        do
        {
            for (i = 0; i < tabCnt; i++)
                strcat(writeBuf, "\t");
            if (strstr(token2, "\"") != NULL)
            {
                strcat(writeBuf, "printf(");
                strcat(writeBuf, token2);
            }
            else
            {
                strcat(writeBuf, "printf(\"%d\",");
                strcat(writeBuf, token2);
            }

            strcat(writeBuf, ");\n");
        } while ((token2 = strtok(NULL, "+")) != NULL);
        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
        /*문자열의 마지막에 '}'가 있다면*/
        /*다음 행헤 }를 추가한다.*/
        if ((keyWordCheck = strstr(rmPublicStr, "}")) != NULL)
        {
            for (i = 1; i < tabCnt; i++)
                insertToBuf(&cFileBuf, &cFileBufSize, "\t", 0);
            insertToBuf(&cFileBuf, &cFileBufSize, "}\n", 0);
        }
        /*header 추가 */
        setHeader("printf ");

        /*p 옵션이 설정되어 있다면*/
        if (optFlag[getOptFlag('p')] == 1)
        {
            /*변환된 함수 문자열을 만든다.*/
            sprintf(p_tmp, "System.out.print() -> printf()\n");
            /*이미 존재하는 문자열일 경우 넘어간다.*/
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    /*자바의 null 문자를 NULL 문자로 변환 한다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "null")) != NULL)
    {
        token = (char *)calloc(sizeof(char *), MAX_LENGTH);
        *keyWordCheck = 0;
        sprintf(token, "%sNULL%s", rmPublicStr, keyWordCheck + 4);
        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
        rmPublicStr = token;
    }

    /*입력 함수 변환*/
    if ((keyWordCheck = strstr(rmPublicStr, "nextInt")) != NULL)
    {
        int has = 0;
        if ((keyWordCheck = strstr(rmPublicStr, "}")) != NULL)
            has = 1;
        token = strtok(rmPublicStr, " \t=");

        /*token 은 입력을 받을 변수를 가리킨다.*/
        if (token == NULL)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        for (i = 0; i < tabCnt; i++)
            strcat(writeBuf, "\t");
        strcat(writeBuf, "scanf(\"%d\",&");
        strcat(writeBuf, token);
        strcat(writeBuf, ");\n");

        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);

        /*문자열의 마지막에 '}'가 있다면*/
        /*다음 행헤 }를 추가한다.*/
        if (has == 1)
        {
            for (i = 1; i < tabCnt; i++)
                insertToBuf(&cFileBuf, &cFileBufSize, "\t", 0);
            insertToBuf(&cFileBuf, &cFileBufSize, "}\n", 0);
        }
        /*header를 찾아서 설정한다.*/
        setHeader("scanf ");
        /*p 옵션이 설정되어 있다면*/
        if (optFlag[getOptFlag('p')] == 1)
        {
            /*변환된 함수 문자열을 만든다.*/
            sprintf(p_tmp, "%s.nextInt() -> scanf()\n", token);
            /*이미 존재하는 문자열일 경우 넘어간다.*/
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    token = (char *)calloc(sizeof(char *), MAX_LENGTH);
    token2 = (char *)calloc(sizeof(char *), MAX_LENGTH);
    /*[] 연산자일 경우 포인터 연산자로 변환한다.*/
    for (i = 0; i < datatypeSize; i++)
    {
        /*<datatype>[] 형태의 문자열을 찾는다.*/
        sprintf(token, "%s[]", datatype[i]);
        if ((keyWordCheck = strstr(rmPublicStr, token)) != NULL)
        {
            *(keyWordCheck + strlen(token) - 2) = '*'; //'['를 0으로 변환
            *(keyWordCheck + strlen(token) - 1) = 0;   //']'을 null로 변환
            sprintf(token2, "%s%s", rmPublicStr, keyWordCheck + strlen(token));
            rmPublicStr = token2;
        }

        /*<datatype> [] 형태의 문자열을 찾는다.*/
        sprintf(token, "%s []", datatype[i]);
        if ((keyWordCheck = strstr(rmPublicStr, token)) != NULL)
        {
            *(keyWordCheck + strlen(token) - 2) = '*'; //'['를 0으로 변환
            *(keyWordCheck + strlen(token) - 1) = 0;   //']'을 null로 변환
            sprintf(token2, "%s%s", rmPublicStr, keyWordCheck + strlen(token));
            rmPublicStr = token2;
        }
    }

    /*File 키워드를 char* 으로 변환*/
    /*char* 형은 fopen의 인자로 사용된다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "File ")) != NULL)
    {
        token = (char *)calloc(sizeof(char *), MAX_LENGTH);
        *keyWordCheck = 0;
        sprintf(token, "%schar* %s", rmPublicStr, keyWordCheck + 5);
        rmPublicStr = token;
        if (optFlag[getOptFlag('p')] == 1)
        {
            sprintf(p_tmp, "File() -> char* \n");
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
    }

    /*FileWriter를 FILE*로 변환*/
    if ((keyWordCheck = strstr(rmPublicStr, "FileWriter ")) != NULL)
    {
        token = (char *)calloc(sizeof(char *), MAX_LENGTH);
        *keyWordCheck = 0;
        sprintf(token, "%sFILE* %s", rmPublicStr, keyWordCheck + 11);
        setHeader("FILE ");
        rmPublicStr = token;
        if (optFlag[getOptFlag('p')] == 1)
        {
            sprintf(p_tmp, "FileWriter -> FILE* \n");
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
    }

    /*new 키워드 변환*/
    /*정의한 class, 데이터형, FILE 관련 함수 를 제외한 함수는 넘어간다.*/
    if ((keyWordCheck = strstr(rmPublicStr, "new ")) != NULL)
    {
        /*new datatpye[] 이라면*/
        for (i = 0; i < datatypeSize; i++)
        {
            if ((token = strstr(rmPublicStr, datatype[i])) != NULL)
            {
                /*할당할 크기를 구한다.*/
                content = strtok(rmPublicStr, "[]");
                if (content == NULL)
                    break;
                content = strtok(NULL, "[]");
                if (content == NULL)
                    break;
                (*keyWordCheck) = 0;

                /*calloc을 이용해 memory를 할당해준다.*/
                sprintf(writeBuf, "%s(%s*)calloc(sizeof(%s),%s);\n", rmPublicStr, datatype[i], datatype[i], content);
                insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);

                /*calloc에 해당하는 header를 찾아 추가한다.*/
                setHeader("calloc ");

                /*p옵션이 설정되어 있다면 P_tmp 변수에 추가한다.*/
                if (optFlag[getOptFlag('p')] == 1)
                {
                    sprintf(p_tmp, "new %s[] -> calloc()\n", datatype[i]);
                    if (strstr(opt_p_buf, p_tmp) == NULL)
                        insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
                }
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
        }

        /*new File() 라면*/
        if ((token = strstr(keyWordCheck, "File(")) != NULL)
        {
            /*parameter로 전달 된 문자열을 가리키는 char * 으로 변환한다.*/
            *keyWordCheck = 0;
            token2 = strtok(token, "()");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
            token2 = strtok(NULL, "()");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
            *token = 0;
            sprintf(writeBuf, "%s%s;", rmPublicStr, token2);
            insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        /*new FileWriter() 라면*/
        if ((token = strstr(keyWordCheck, "FileWriter(")) != NULL)
        {
            /*open 해서 쓰기 위한 파일 변수를 구한다*/
            *keyWordCheck = 0;
            token2 = strtok(token, "(,)");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
            token2 = strtok(NULL, "(,)");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
            *token = 0;

            /*p 옵션에 대한 처리를 해준다*/
            if (optFlag[getOptFlag('p')] == 1)
            {
                sprintf(p_tmp, "FileWriter() -> fopen() \n");
                if (strstr(opt_p_buf, p_tmp) == NULL)
                    insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
            }

            /*mode 를 구한다.*/
            token = strtok(NULL, "(,)");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }

            /*mode 에 따른 fopen을 호출한다.*/
            if (strstr(token, "true") != NULL)
            {
                /*파일의 뒤에 이어쓰기*/
                sprintf(writeBuf, "%sfopen(%s,\"a+\");", rmPublicStr, token2);
            }
            else
            {
                /*파일에 덮어쓰기*/
                sprintf(writeBuf, "%sfopen(%s,\"w+\");", rmPublicStr, token2);
            }

            /*header를 구한다.*/
            setHeader("FILE ");

            /*IOException이 설정되어 있다면*/
            if (isIoExcept == 1)
            {
                /*fopen 에 대한 error 처리를 해준다.*/
                token = (char *)calloc(sizeof(char), MAX_LENGTH);
                tmp =  (char *)calloc(sizeof(char), MAX_LENGTH);
                memset(token, 0, sizeof(token));
    
                for (i = 0; i < tabCnt; i++)
                    strcat(token, "\t");
                sprintf(tmp, "\n%sif(access(%s,F_OK) < 0){\n\t%sfprintf(stderr,\"open error for %%s\",%s);\n\t%sexit(1);\n\t};\n", token, token2, token, token2, token);
                insertToBuf(&cFileBuf, &cFileBufSize,tmp, 0);
                setHeader("exit ");                
                setHeader("access ");
            }
            insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
            
            
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
        /*정의한 class 를 할당하는 new 키워드 라면*/
        for (i = 0; i <= cFileListSize; i++)
        {
            if ((token = strstr(keyWordCheck, cFileList[i])) != NULL)
            {
                /*생성자만 c 파일에 쓴다.*/
                *keyWordCheck = 0;
                for (j = 0; j < tabCnt; j++)
                    strcat(writeBuf, "\t");
                strcat(writeBuf, token);
                insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);

                if ((token = strstr(rmPublicStr, cFileList[i])) != NULL)
                {
                    token2 = strtok(token, " =\t");
                    if (token2 == NULL)
                    {
                        free(cFile);
                        free(writeBuf);
                        free(p_tmp);
                        return;
                    }

                    token2 = strtok(NULL, " =\t");
                    if (token2 == NULL)
                    {
                        free(cFile);
                        free(writeBuf);
                        free(p_tmp);
                        return;
                    }
                    /*변수명으로 접근하는 코드를 수정하기 위해*/
                    /*변수명을 지워야 할 list에 추가한다.*/
                    strcat(removeList[removeListSize++], token2);
                }
            }
        }

        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }

    /*File.close()라면*/
    if ((token = strstr(rmPublicStr, ".close")) != NULL)
    {
        token2 = (char *)calloc(sizeof(char), MAX_LENGTH);
        *token = 0;

        for (i = 0; i < tabCnt; i++)
            strcat(token2, "\t");
        while ((token - 1 != NULL) && (*(token - 1) != '\t'))
        {
            token--;
        }
        /*fclose(*FILE) 로 바꾼다.*/
        sprintf(writeBuf, "%sfclose(%s);\n", token2, token);
        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
        setHeader("fclose ");
        if (optFlag[getOptFlag('p')] == 1)
        {
            sprintf(p_tmp, "%s.close() -> fclose() \n", token);
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    /*File.flush()라면*/
    if ((token = strstr(rmPublicStr, ".flush")) != NULL)
    {
        token2 = (char *)calloc(sizeof(char), MAX_LENGTH);
        *token = 0;

        for (i = 0; i < tabCnt; i++)
            strcat(token2, "\t");
        while ((token - 1 != NULL) && (*(token - 1) != '\t'))
        {
            token--;
        }
        /*fflush(*FILE)로 바꾼다.*/
        sprintf(writeBuf, "%sfflush(%s);\n", token2, token);
        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
        setHeader("fflush ");
        if (optFlag[getOptFlag('p')] == 1)
        {

            sprintf(p_tmp, "%s.flush() -> fflush() \n", token);
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    /*File.write()라면*/
    if ((keyWordCheck = strstr(rmPublicStr, ".write")) != NULL)
    {
        int fullSize = strlen(rmPublicStr);
        int isContent = 0;
        token2 = (char *)calloc(sizeof(char), MAX_LENGTH);
        token = (char *)calloc(sizeof(char), MAX_LENGTH);
        *keyWordCheck = 0;
        memset(token, 0, sizeof(token));
        memset(token2, 0, sizeof(token2));

        for (i = 0; i < tabCnt; i++)
            strcat(token2, "\t");
        for (i = strlen(rmPublicStr); i < fullSize; i++)
        {
            if (isContent == 0)
            {
                if (rmPublicStr[i] == '\"')
                {
                    isContent++;
                    continue;
                }
            }
            else if (isContent == 1)
            {
                if (rmPublicStr[i] == '\"')
                {
                    isContent--;
                    continue;
                }
                token[strlen(token)] = rmPublicStr[i];
            }
        }
        while ((keyWordCheck - 1 != NULL) && (*(keyWordCheck - 1) != '\t'))
        {
            keyWordCheck--;
        }
        /*fwrite(buf, bufsize, 1, *FILE)로 바꾼다.*/
        sprintf(writeBuf, "%sfwrite(\"%s\", %lu, 1, %s);\n", token2, token, strlen(token) - 1, keyWordCheck);
        insertToBuf(&cFileBuf, &cFileBufSize, writeBuf, 0);
        setHeader("fwrite ");

        if (optFlag[getOptFlag('p')] == 1)
        {
            sprintf(p_tmp, "%s.write() -> fwrite() \n", keyWordCheck);
            if (strstr(opt_p_buf, p_tmp) == NULL)
                insertToBuf(&opt_p_buf, &opt_p_buf_size, p_tmp, 0);
        }
        free(cFile);
        free(writeBuf);
        free(p_tmp);
        return;
    }
    /*class의 {은 무시한다.*/
    if ((strcmp(rmPublicStr, "{") == 0) || (strcmp(rmPublicStr, "{\n") == 0))
    {
        if (sqaure == 1)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
    }
    /*class의 }은 무시한다.*/
    else if ((strcmp(rmPublicStr, "}") == 0) || (strcmp(rmPublicStr, "}\n") == 0))
    {
        if (sqaure == 0)
        {
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
    }
    /*사용자가 정의한 class 변수라면*/
    for (i = 0; i <= cFileListSize; i++)
    {
        token = (char *)calloc(sizeof(char), MAX_LENGTH);
        sprintf(token, "%s ", cFileList[i]); //선언문인지 검사한다.
        if ((token = strstr(rmPublicStr, token)) != NULL)
        {
            token2 = strtok(token, " =\t;");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }

            token2 = strtok(NULL, " =\t;");
            if (token2 == NULL)
            {
                free(cFile);
                free(writeBuf);
                free(p_tmp);
                return;
            }
            /*지워야 할 변수에 추가한다.*/
            strcat(removeList[removeListSize++], token2);
            free(cFile);
            free(writeBuf);
            free(p_tmp);
            return;
        }
    }

    insertToBuf(&cFileBuf, &cFileBufSize, rmPublicStr, 0);
    free(cFile);
    free(writeBuf);
    free(p_tmp);
    return;
}
/*_str에서 _rmStr을 제거한다.*/
char *removeStr(char *_str, char *_rmStr, int flag)
{
    char *res = (char *)calloc(sizeof(char), MAX_LENGTH);
    char *strP;
    memset(res, 0, sizeof(res));

    if ((strP = strstr(_str, _rmStr)) != NULL)
    {
        /*_str에서 _rmStr 시작 위치을 NULL로 하고 
        strlen(_rmStr)떨어진 문자열을 이어붙인다.*/
        *strP = 0;
        sprintf(res, "%s%s", _str, strP + strlen(_rmStr));
        /*flag가 0일 경우 탭을 하나 제거한다.*/
        if ((flag == 0) && (res[0] == '\t'))
            return &res[1];
        return res;
    }
    else
    {
        free(res);
        /*flag가 0일 경우 탭을 하나 제거한다.*/
        if ((flag == 0) && (_str[0] == '\t'))
            return &_str[1];
        return _str;
    }
}

/*_fd가 가리키는 파일의 다음 문자열을 반환한다.*/
char *getNextLine(int _fd)
{
    char *res = (char *)calloc(MAX_LENGTH, 1);
    char c;
    int cntQuo = 0;
    int index = 0;
    memset(res, 0, sizeof(res));
    if (res == NULL)
        return NULL;
    while (read(_fd, &c, 1) > 0)
    {
        if (cntQuo == 0)
        {
            res[index++] = c;
            if (c == '\n')
            {
                break;
            }
            else if (c == '"')
            {
                cntQuo++;
            }
        }
        else
        {
            res[index++] = c;
            if (c == '"')
            {
                cntQuo--;
            }
        }
    }
    return res;
}

/*_buf에 _str을 이어 붙인다.*/
/*_buf size가 작으면 2배로 늘린다.*/
void insertToBuf(char **_buf, int *_bufsize, char *_str, int isHeader)
{
    char *tmp;

    /*size가 작으면 buf 사이즈를 두배로 늘린다*/
    while ((*_bufsize) <= strlen((*_buf)) + strlen(_str))
    {
        tmp = (*_buf);

        (*_bufsize) *= 2;
        (*_buf) = (char *)calloc(sizeof(char), (*_bufsize));
        memset((*_buf), 0, sizeof(_buf));
        strcat((*_buf), tmp);
        free(tmp);
    }
    /*header문자열이 아니라면 뒤에 이어 붙인다.*/
    if (isHeader == 0)
    {
        strcat((*_buf), _str);
        return;
    }

    /*header 라면 문자열 앞에 추가한다.*/
    tmp = (char *)calloc(sizeof(char), (*_bufsize));
    memset(tmp, 0, sizeof(tmp));
    strcat(tmp, _str);
    strcat(tmp, (*_buf));
    free((*_buf));
    (*_buf) = tmp;
}
/*Header 테이블에서 함수에 해당하는 헤더를 가져온다.*/
void setHeader(char *funcName)
{
    char *header = (char *)calloc(sizeof(char), MAX_LENGTH);
    char *nextLine, *token;
    int headerFileDes = -1;
    /*header table을 open 한다.*/
    if ((headerFileDes = open(HEADER_FILE, O_RDONLY)) < 0)
    {
        exit(0);
    }
    memset(header, 0, sizeof(header));

    /*한 줄씩 읽어서 원하는 header를 찾는다.*/
    while ((nextLine = getNextLine(headerFileDes)) != NULL)
    {
        if (strlen(nextLine) == 0)
            break;
        if (nextLine[strlen(nextLine) - 1] == '\n' ||
            nextLine[strlen(nextLine) - 1] == ' ')
            nextLine[strlen(nextLine) - 1] = 0;
        token = strtok(nextLine, "#"); // header table의 함수 이름을 얻는다.
        if (token == NULL)
            continue;
        if (strcmp(token, funcName) != 0) //함수 이름을 비교해서 같지 않으면 continue
            continue;
        /*# 문자를 기준으로 parsing 한다.*/
        while ((token = strtok(NULL, "#")) != NULL)
        {
            /*#문자르 추가해 header를 추가한다.*/
            sprintf(header, "#%s\n", token);
            /*header가 이미 추가되어 있다면 추가하지 않는다.*/
            if (strstr(cFileBuf, header) != NULL)
                continue;
            insertToBuf(&cFileBuf, &cFileBufSize, header, 1);
        }
    }
    free(header);
}

/*make 파일을 생성한다.*/
void make_makefile(char *_javaFile)
{
    char *cpJava;         //java 파일 이름을 복사한다..
    char *resName;        //java 파일의 이름
    char *contents;       //make 명령어를 저장한다.
    char *makeFileName;       //makefile 이름.
    int makeFileDes = -1; //makefile의 file descropter를 저장한다.
    int i = 0;
    cpJava = (char *)calloc(sizeof(char), MAX_LENGTH);
    contents = (char *)calloc(sizeof(char), MAX_LENGTH);
    makeFileName = (char *)calloc(sizeof(char), MAX_LENGTH);
    memset(cpJava, 0, sizeof(cpJava));
    memset(contents, 0, sizeof(contents));
    memset(makeFileName, 0, sizeof(makeFileName));
    strcat(cpJava, _javaFile);
    resName = strtok(cpJava, ".");
    sprintf(makeFileName,"%s_%s" ,resName,MAKE_FILE);
    if (resName == NULL)
    {
        fprintf(stderr, "java file error [%s]\n", _javaFile);
        exit(1);
    }
    if ((makeFileDes = open(makeFileName, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0)
    {
        fprintf(stderr, "make file open error [%s]\n", makeFileName);
        exit(1);
    }

    sprintf(contents, "%s :", resName); // <resName> :
    write(makeFileDes, contents, strlen(contents));
    for (i = 0; i <= cFileListSize; i++)
    {
        sprintf(contents, " %s.o", cFileList[i]); // <resName> : cFileList[0].o ...
        write(makeFileDes, contents, strlen(contents));
    }
    write(makeFileDes, "\n", 1);

    sprintf(contents, "\tgcc -o %s", resName); // gcc -o <resName>
    write(makeFileDes, contents, strlen(contents));
    for (i = 0; i <= cFileListSize; i++)
    {
        sprintf(contents, " %s.o", cFileList[i]); // gcc -o <resName> cFileList[0].o ...
        write(makeFileDes, contents, strlen(contents));
    }
    write(makeFileDes, "\n", 1);

    for (i = 0; i <= cFileListSize; i++)
    {
        sprintf(contents, "%s.o :\n", cFileList[i]); //cFileList[0].o :
        write(makeFileDes, contents, strlen(contents));
        sprintf(contents, "\tgcc -c -o %s.o %s.c\n", cFileList[i], cFileList[i]); //gcc -c -o cFileList[0].o cFileList[0].c
        write(makeFileDes, contents, strlen(contents));
    }
    close(makeFileDes);

    free(cpJava);
    free(contents);
    free(makeFileName);
}

/*전역 변수를 초기화 한다.*/
void init()
{
    int i = 0;
    isIoExcept = 0;
    memset(optFlag, 0, sizeof(optFlag));
    javaBuf = (char *)calloc(sizeof(char), 1);
    javaBufSize = 1;
    cFileBuf = (char *)calloc(sizeof(char), 1);
    cFileBufSize = 1;
    opt_p_buf = (char *)calloc(sizeof(char), 1);
    opt_p_buf_size = 1;

    cFileListSize = -1;
    removeListSize = 0;

    memset(javaBuf, 0, sizeof(javaBuf));
    memset(cFileBuf, 0, sizeof(cFileBuf));
    memset(cFileList, 0, sizeof(cFileList));
    memset(removeList, 0, sizeof(removeList));
}