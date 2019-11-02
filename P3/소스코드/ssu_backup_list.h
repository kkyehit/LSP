#define PATH_LENGHT 255       //최대 파일 경로 길이
#define LIST_DEF 1            //list가 정의 되어 있음을 알리기 위함
#define MAX_LOG_MSG_list 1024 //로그 메세지의 길이

extern char **environ;

char *get_real_PATH(char *filename);

struct backup_struct
{
    struct backup_struct *next, *prev; //이중 링크드 리스트
    struct stat statbuf;

    int backup_period;              //백업 주기
    int backup_opt;                 //백업 옵션, 비트마스크 형태
    int lifetime;                   //백업 폴더 내에서 존재하는 시간, -1이면 설정 안된것,
    int number;                     // -n 옵션의 인자
    char ori_filename[PATH_LENGHT]; //원래 파일 경로
    pthread_t tid;                  //thread id

    int logfd;                      //로그 파일 디스크립터
    char *directory;                //백업 경로
};

struct backup_struct *root = NULL; //링크드 리스트의 시작
struct backup_struct *tail = NULL; //링크드 리스트의 끝
int listcnt = 0;

/*링크드 리스트에 노드 추가하는 함수*/
struct backup_struct *addList(int period, int backup_opt, int lifetime, int number, char ori_filename[PATH_LENGHT])
{
    /*새로운 노드를 할당 받는다.*/
    struct backup_struct *node = (struct backup_struct *)calloc(sizeof(struct backup_struct), 1);
    /*변수 초기화 및 할당*/
    memset(node, 0, sizeof(node));            //메모리 초기화
    node->backup_period = period;             //백업 주기 할당
    node->backup_opt = backup_opt;            //option 할당
    node->lifetime = lifetime;                //백업 파일이 존재할 시간 할당
    node->number = number;                    //백업 파일의 최대 개수 할당
    strcpy(node->ori_filename, ori_filename); //원본 파일 이름 저장
    node->next = NULL;                        //포인터 초기화
    node->prev = NULL;                        //포인터 초기화

    /*node의 statbuf에 원본 파일의 정보 저장*/
    if (lstat(node->ori_filename, &node->statbuf) < 0)
    {
        printf("addList>lstat [%s]\n", node->ori_filename);
        free(node);
        return NULL;
    }

    printf("addList>[%s]\n", node->ori_filename);
    /*list가 비어있다면*/
    if (tail == NULL)
    {
        tail = root = node;//root = tail
    }
    /*그렇지 않다면*/
    else
    {
        tail->next = node;
        node->prev = tail;
        tail = tail->next;
    }
    listcnt++;//list 길이 추가
    return node;//node 반환
}

/*링크드 리트에서 특정 노드를 지우는 함수,*/
/*노드를 지우고 다음 노드를 반환한다.*/
struct backup_struct *removeNode(struct backup_struct *node)
{
    /*node가 NULL인 경우 NULL 리턴*/
    if (node == NULL)
        return NULL;
    
    /*노드가 하나인 경우*/
    if (root == tail)
    {
        free(node);//node의 할당된 공간 반환
        root = tail = NULL;//root와 tail을 NULL초 초기화
        return NULL;//NULL 리턴
    }

    /*list 재배열*/
    struct backup_struct *prev = node->prev;
    struct backup_struct *next = node->next;

    if (prev != NULL)
        prev->next = next;
    if (next != NULL)
        next->prev = prev;
    if (node == root)
        root = next;

    free(node);//node의 할당된 공간 반환
    listcnt--;//list 길이 감소
    return next;//다음 노드 반환
}

/*인자로 들어온 파일의 이름을 리스트에서 검색하는 함수*/
struct backup_struct *findFileName(char *ori_filename)
{
    /*root 부터 탐색한다.*/
    struct backup_struct *node = root;
    /*node가 더이상 없을 때까지 탐색*/
    while (node != NULL)
    {   
        /*파일의 이름을 찾은 경우*/
        if (strcmp(node->ori_filename, ori_filename) == 0)
        {
            /*해당 노드 반환*/
            return node;
        }
        /*아니면 계속 진행*/
        node = node->next;
    }
    return NULL; // 찾지 못함
}

/*리스트의 목록을 출력한다.*/
void process_list(int logfd)
{
    struct backup_struct *node = root;
    char mode[5] = {0};
    /*목록 출력*/
    printf("PATH                                                                  PERIOD    OPTION\n");
    /*한 줄 단위로 노드에 대한 정보 출력*/
    while (node != NULL)
    {
        memset(mode, 0, sizeof(mode));
        /*각각의 옵션에 대해 설정 되어 있다면 해당 옵션,*/
        /*그렇지 않다면 . 출력*/
        if (node->backup_opt & (1 << 0))//m
            strcat(mode, "m");
        else
            strcat(mode, ".");
        if (node->backup_opt & (1 << 1))//n
            strcat(mode, "n");
        else
            strcat(mode, ".");
        if (node->backup_opt & (1 << 2))//t
            strcat(mode, "t");
        else
            strcat(mode, ".");
        if (node->backup_opt & (1 << 3))//d
            strcat(mode, "d");
        else
            strcat(mode, ".");
        /*원본 파일 이름, 백업 기간, mode 출력*/
        printf("%-70s%-10d%-10s\n", node->ori_filename, node->backup_period, mode);
        node = node->next;
    }
    /*버퍼 비우기*/
    fflush(stdout);
}

/*~에 대한 절대 경로 반환하는 함수*/
char *get_real_PATH(char *filename)
{
    char *res = (char *)calloc(sizeof(char), PATH_LENGHT);//새로운 공간 할당
    memset(res, 0, sizeof(res));//초기화
    /*경로의 앞 문자가 ~라면 HOME 디렉토리로 변환*/
    if (filename[0] == '~')
    {
        strcat(res, getenv("HOME"));//HOME 디렉토리 경로 얻기
        if (*(filename + 1) != '/')// '~'인 경우 '~/' 로 변환
            strcat(res, "/");
        strcat(res, filename + 1);//새로운 경로 생성 완료
        return res;//새로운 경로 리턴
    }
    /*경로의 앞문자가 '~'가 아니면*/
    else
    {
        return filename;//변환 없이 반환
    }
};