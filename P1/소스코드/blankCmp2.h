#define _BUF_SIZE_CMP 1024    //비교에 사용되는 bufer의 크기
#define _HEADER 99           //HEADER의 priority
#define _iS_BRACKET -2        //9() 의 priority
#define _iS_SQUARE_BRACKET -3 //[]의 priority
#define _IS_POINTER_OPER -4   //포인터의 priority
#define _MAX_CMP_TIME 3       //비교에 사용 되는 시간이 너무 길면 안된다.

struct postNode //후위 표현식을 위한 구조체
{
    char *name;                    //노드가 가리키는 string
    int priority;                  //name의 우선수위
    struct postNode *next, *prev;  //이전 노드와 다음 노드 가리킴,
    struct postNode *right, *left; //연산자의 왼쪽 변수와 오른쪽 변수의 범위를 나타내기 위해 사용
} typedef postNode;

/*char형 연산자 리스트*/
const char operList[] = {'+', '-', '*', '/', '%', '=', '!', '|', '&', '~', '>', '<', '^', '(', ')', '{', '}', '[', ']', ','};
const int operSize = 20;

/*양 옆이 바뀌어도 답이 되는 연산자*/
const char *canchOperList[] = {"+", "*", "&", "&&", "|", "||", "^", "==", "!=", "<", "<=", ">", ">="};
const int canchOperSize = 13;

/*단항 연산자*/
const char *oneOperList[] = {"++", "--", "!", "~"};
const int oneOperSize = 4;

/*양옆이 바뀌면 자신도 바뀌어야 하는 비교 연산자*/
char *operStrSet[] = {"<", ">", "<=", ">="};

/*데이터 형*/
const char *dataList[] = {"bool", "short", "int", "long", "float", "double", "char", "void"};
const int dataListSize = 8;

struct timeval cmpStart_t, cmpNow_t; //연산자 비교시 너무 시간이 오래걸리면 pass

char *makeSpace(char *str);                                                                 //연산자 단위로 공백 만들기
char *removeSpace(char *str);                                                               //필요없는 공백 지우기
char *getStr(postNode *_head);                                                              //후위 표현식으로 구현된 리스트를 string 으로 변환
int isChOperend(char *a);                                                                   //양옆이 바뀌어도 되는 연산자 인가 맞으면 1 틀리면 0
int isOperend(char a);                                                                      //연산자인가 맞으면 1 틀리면 0
int isOneOperend(char *a);                                                                  //단항연산자인가 맞으면 1 틀리면 0
int getPriority(char *str);                                                                 //연산자의 우선순위 return
int expCmp(char *_str1, char *_str2);                                                       //표현식 두개 비교 같으면 1, 다르면 -1
int getList(postNode *_ansNode, postNode *_stdNode);                                        //두 스택의 노드들을 바꾸면서 가능한 답인지 비교 같으면 1, 다르면 -1
int cmpList(postNode *_tail, char *_str);                                                   //_tail로 만들 수 있는 표현식과 _str 비교 같으면 1, 다르면 -1
int checkInclude(postNode *_head);                                                          //Include 문장이 포함되어 잇는 문자열인지 확인
int isPointer(postNode *_node);                                                             //_node가 data형인지 검사 맞으면 1 아니면 0 리턴
void setPointer(postNode *_head);                                                           // pointer에 대해 priority를 설정하기 위한 함수
void setPointer2(postNode *_head);                                                          //
void combinStr(char *str);                                                                  //한 묶음으로 처리해야 하는 연산자 처리,
void setRightANdLeft(postNode *_head);                                                      //노드가 연산자라면 양 옆의 변수 범위 설정
void changeRightLeft(postNode *_node);                                                      //양 옆 노드 위치 바꾸기
void setOperStr(postNode *_node);                                                           // < 와  > 를 바꾸고 >= 과 <=를 바꾸는 함수
void rmDupNotOper(postNode *_node);                                                         //연속인 부정연산자 제거
int changeNode(postNode *_a_start, postNode *_a_end, postNode *_b_start, postNode *_b_end); //양 옆 변수가 연산자라면 옆 연산자의 변수와 바꿀 때 사용

int changeNode2(postNode *_ansNode, postNode *_stdNode, postNode *_tmp); //동인한 priority의 동일한 연산자에 대해 처리하는 함수 불가능시 0 리턴
int changeNode3(postNode *_tail, postNode *_tmp, postNode *_stdNode);    //동일한 priority의 다른 연산자에 대해 처리하는 함수 불가능시 0 리턴

postNode *setPriority(postNode *_head);               //노드마다 우선순위 설정, 성공시 null 반환 , 실패시 해당 노드 반환
postNode *getClose(postNode *_head);                  //이후에 ) 또는 ] 가 나오는 지 찾는 함수, 해당 노드 return 실패시 NULL
postNode *getPreClose(postNode *_head);               //이전에 ( 또는 [ 가 나왔는지 찾는 함수, 해당 노드 return 실패시 NULL
postNode *makePostStack(char *_str, postNode *_tail); //후위 표현식 리스트 생성, 남은 연산자 리스트 return;
postNode *getHeader(postNode *_tail);                 //리스트의 Header 반환
postNode *getTail(postNode *_tail);                   //리스트의 tail 반하환

int expCmp(char *_str1, char *_str2) // 두 연산식이 같은지 비교,
{
    postNode *str1Head = (postNode *)malloc(sizeof(postNode));
    postNode *str1Tail = str1Head;
    postNode *str2Head = (postNode *)malloc(sizeof(postNode));
    postNode *str2Tail = str2Head;
    char *afstr1, *afstr2;
    char *afafstr1, *afafstr2;

    str1Head->priority = _HEADER;
    str2Head->priority = _HEADER;

    if (_str1 == NULL || _str1 == NULL) //문자열 중 하나가 NULL 이라면
        return 0;                       //실패 리턴

    if ((strlen(_str1) == 0) || (strlen(_str2) == 0)) //문자열이 길이가 0 인 문자가 있다면
        return 0;                                     //실패 리턴

    /*문자열 끝의 공백과 개행 제거*/
    if (_str1[strlen(_str1) - 1] == ' ' || _str1[strlen(_str1) - 1] == '\n')
        _str1[strlen(_str1) - 1] = 0;
    if (_str2[strlen(_str2) - 1] == ' ' || _str2[strlen(_str1) - 1] == '\n')
        _str2[strlen(_str2) - 1] = 0;

    /*두 문자열을 단순 비교*/
    if (strcmp(_str1, _str2) == 0)
        return 1;

    afstr1 = makeSpace(_str1); //연산자 단위로 공백을 만들고
    combinStr(afstr1);         //하나의 연산자는 다시 하나로 합친다
    removeSpace(afstr1);       //포인터와 -는 연산되는 문자열에 붙인다.

    afstr2 = makeSpace(_str2); //연산자 단위로 공백을 만들고
    combinStr(afstr2);         //하나의 연산자는 다시 하나로 합친다
    removeSpace(afstr2);       //포인터와 -는 연산되는 문자열에 붙인다.

    /*NULL문자열이 있다면 실패를 리턴*/
    if (afstr1 == NULL || afstr2 == NULL)
        return 0;
    /*두 문자열이 같으면 성공을 리턴(컴파일시 에러가 없는 공백 무시 하는 기능)*/
    if (strcmp(afstr1, afstr2) == 0)
        return 1;
    makePostStack(afstr1, str1Tail); //후위 연산자 스택 이중 링크드 리스트를 만든다.
    setPriority(str1Head);           //각 노드별로 priority를 설정한다.

    setPointer(str1Head); //포인터 노드의 priority를 설정한다

    setRightANdLeft(str1Head); //각 노드마다 왼쪽항과 오른쪽 항의 위치를 구한다
    setPointer2(str1Head);     //연관된 포인터 노드를 구해본다.ex. (char *) &buf
    setRightANdLeft(str1Head); //다시 오른쪽 항과 왼쪽 항을 구한다.

    makePostStack(afstr2, str2Tail); //후위 연산자 스택 이중 링크드 리스트를 만든다.
    setPriority(str2Head);           // _str1과 같은 알고리즘을 수행한다.

    setPointer(str2Head);

    setRightANdLeft(str2Head);
    setPointer2(str2Head);
    setRightANdLeft(str2Head);

    afafstr1 = getStr(str1Head); //이중 링키드 리스트를 string 형으로 바꾼다.
    afafstr2 = getStr(str2Head);
    /*둘중 히나가 NULL 이라면 실패를 리턴*/
    if (afafstr1 == NULL || afafstr2 == NULL)
        return 0;
    /*두 문자열의 길이가 다르다면 같을 수 없으므로 실패를 리턴*/
    if (strlen(afafstr1) != strlen(afafstr2))
        return 0;
    /*두 문자열이 같다면 성공을 리턴*/
    if (strcmp(afafstr1, afafstr2) == 0)
        return 1;
    /*만약 include를 위한 문장이라면 위치를 바꿀 수 없다.*/
    if (checkInclude(str1Head) == 0)
        return 0;

    str1Tail = getTail(str1Head);
    str2Tail = getTail(str2Head);
    gettimeofday(&cmpStart_t, NULL);    //재귀호출 시작 시간 기록,
    return getList(str1Tail, str2Tail); //getList() 를 통해 후위 연산식의 위치를 바꾸어 가며 비교
}

int checkInclude(postNode *_head)
{
    _head = getHeader(_head);
    _head = _head->next; //첫 문장이
    if (_head != NULL)   //NULL 이거나
        return 0;
    if (_head->name == NULL) //가리키는 문자열이 없거나
        return 0;
    if ((strcmp(_head->name, "include") == 0) || (strcmp(_head->name, "#include") == 0)) //include 문장 이라면
    {
        return 0; // 실패를 리턴
    }
    return 1; //조건에 충족하면 1을 리턴
}

int isPointer(postNode *_node) //데이터형 다음에 *또는 &가 오면 pointer 연산자이다.
{
    int i = 0;
    if (_node == NULL || _node->name == NULL)
        return 0;

    for (i = 0; i < dataListSize; i++)
    {
        if (strcmp(_node->name, dataList[i]) == 0) //미리 선언해둔 data형 list와 비교
            return 1;                              //pointer 라면 1 리턴
    }

    return 0; //아니면 0 리턴
}
void setPointer(postNode *_head) //데이터 형에 따른 pointer 연산자 설정
{
    if (_head == NULL)
        return;

    postNode *head = _head;
    head = head->next;
    while (head != NULL && head->name != NULL)
    {
        if ((strcmp(head->name, "*") == 0) || (strcmp(head->name, "&") == 0)) //문자열이 * 또는 & 일때 수행
        {
            if (head->prev != NULL)
            {
                if (isPointer(head->prev)) //만약 앞의 문자열이 데이터 형 이면 포인터다 ex. (char &)
                {
                    head->priority = _IS_POINTER_OPER; //포인터 우선순위 설정
                }
                else if (head->prev->prev != NULL) //int *a -> int a * 기 되므로 이에대한 처리
                {
                    if (isPointer(head->prev->prev)) //선언문이라면
                    {
                        head->priority = _IS_POINTER_OPER; //포인터 설정
                    }
                }
            }
        }
        head = head->next;
    }
}
void setPointer2(postNode *_head) //오른쪽 항과 왼쪽 항의 위치가 정해 지고 수행
{                                 /*포인터 선언문, 캐스팅과 연관된 연산자 처리*/
    if (_head == NULL)
        return;

    postNode *head = _head;
    head = head->next;
    while (head != NULL && head->name != NULL)
    {
        if ((strcmp(head->name, "*") == 0) || (strcmp(head->name, "&") == 0))
        {
            if (head->prev != NULL)
            {
                if (head->prev->left == head->prev->right)        //포인터라면 가리키는 노드는 하나
                    if (head->prev->priority == _IS_POINTER_OPER) //만약 포인터라면
                    {
                        head->priority = _IS_POINTER_OPER; //이 노드도 포인터 이다.
                    }
            }
            if (head->right != NULL)
            {
                if (head->right->prev != NULL)
                {
                    if (head->right->prev->left == head->right->prev->right) //포인터라면 가리키는 노드는 하나
                    {
                        if (head->right->prev->priority == _IS_POINTER_OPER) //만약 포인터라면
                        {
                            head->priority = _IS_POINTER_OPER; //이 노드도 포인터이다.
                        }
                    }
                    if (isPointer(head->right->prev))
                    {
                        head->priority = _IS_POINTER_OPER;
                    }
                }
            }
        }
        head = head->next;
    }
}
char *getStr(postNode *_head) //postNode 를 string으로 변환하는 함수
{
    postNode *tail = getHeader(_head); //_head를 얻고
    char *res = (char *)malloc(sizeof(char) * _BUF_SIZE_CMP);
    tail = tail->next; //다음으로 진헹
    memset(res, 0, sizeof(res));

    while (tail != NULL) //NULL이 아닐 때 까지
    {
        strcat(res, tail->name); //문자열 및 연산자 사이에
        strcat(res, " ");        //공백을 너어서 분류
        tail = tail->next;
    }
    return res;
}
int cmpList(postNode *_tail, char *_str) //두 list를 비교한다.
{
    gettimeofday(&cmpNow_t, NULL);
    if (cmpNow_t.tv_sec - cmpStart_t.tv_sec > _MAX_CMP_TIME)
        return 0;
    postNode *tmp;
    int flag = 0;
    char *res;

    if (_tail->priority == _HEADER) //_tail이 header 라면 실패
    {
        return 0;
    }

    res = getStr(_tail); //문자열로 변형하여
    if (res == NULL)     //NULL 이면 실패 반환
        return 0;
    if (strcmp(res, _str) == 0) //문자열을 비교해서 같으면
        return 1;               // 성공을 반환
    return 0;                   //정답이 될 수 없다면 0을 반환
}
int getList(postNode *_ansNode, postNode *_stdNode) //연산식을 바꾸어 가며 비교하는 함수
{
    gettimeofday(&cmpNow_t, NULL);
    if (cmpNow_t.tv_sec - cmpStart_t.tv_sec > _MAX_CMP_TIME) //비교하는 시간이 너무 길어지면 종료
        return 0;
    postNode *tmp;
    int flag = 0;
    char *res; //문자열을 가리키는 변수
    if (_ansNode == NULL)
    {
        return 0;
    }
    if (_ansNode->priority == _HEADER)
    {
        return 0;
    }
    if (isChOperend(_ansNode->name)) //앞뒤가 바뀌어도 되는 연산자 라면
    {
        changeRightLeft(_ansNode);                  //오른쪽과 왼쪽의 항을 바꾼다.
        setRightANdLeft(_ansNode);                  //오른쪽 항과 왼쪽 항의 위치를 갱신한다.
        res = getStr(_ansNode);                     //문자열로 변환한다
        if (cmpList(_stdNode, res) == 1)            //문자열과 학생의 후위연산식을 비교한다
            return 1;                               //같으면 1 리턴
        if (getList(_ansNode->prev, _stdNode) == 1) //같지않다면 getList를 재귀 호출한다.
            return 1;                               //재귀 호출한 결과 1이 리턴 되었다면 1리턴
        if ((_ansNode->left != NULL) && (_ansNode->right != NULL))
        {
            if (changeNode2(_ansNode, _stdNode, _ansNode)) //a+b+c와 같이 연속되는 동일한 우선순위의 동일한 연산식에 대해 바꾸어 본다
                return 1;                                  //같은 식이 될 수 있다면 1을 리턴한다
        }
        changeRightLeft(_ansNode); //오른쪽 항과 왼쪽 항을 다시 바꾸어 원래 연산식을 변경한다.
        setRightANdLeft(_ansNode); //오른쪽 항과 왼쪽항의 위치를 갱신한다.

        if ((_ansNode->left != NULL) && (_ansNode->right != NULL))
        {
            if (changeNode2(_ansNode, _stdNode, _ansNode)) //다시 연속되는 동일한 연산식에 대해 처리한다.
                return 1;                                  //같은 식이 될 수 있다면 1을 리턴한다.
        }
    }
    if (_ansNode->priority > 0) //+- 와 같이 동일안 우선순위의 다른 연산자에 대한 처리를 위한 알고리즘
    {
        if (_ansNode->right != NULL)
        {
            tmp = _ansNode->right->prev;
            while ((tmp != NULL) && (tmp->priority > 0) && (tmp->name != NULL))
            {
                if (strcmp(_ansNode->name, "+") == 0)
                {
                    if (strcmp(tmp->name, "-") == 0)
                    {
                        if (changeNode3(_ansNode, tmp, _stdNode) == 1) //연산자와 오른쪽항을 바꾸는 함수
                            return 1;                                  //changeNode3()는 위치를 바꾸고 getList를 호출하여 동일한 연산식인지 확인한다.
                    }
                }
                else if (strcmp(_ansNode->name, "-") == 0)
                {
                    if ((strcmp(tmp->name, "-") == 0) || (strcmp(tmp->name, "+") == 0))
                    {
                        if (changeNode3(_ansNode, tmp, _stdNode) == 1) //연산자와 오른쪽항을 바꾸는 함수
                            return 1;
                    }
                }
                else if (strcmp(_ansNode->name, "*") == 0)
                {
                    if (strcmp(tmp->name, "/") == 0)
                    {

                        if (changeNode3(_ansNode, tmp, _stdNode) == 1) //연산자와 오른쪽항을 바꾸는 함수
                            return 1;
                    }
                }
                else if (strcmp(_ansNode->name, "/") == 0)
                {
                    if ((strcmp(tmp->name, "*") == 0) || (strcmp(tmp->name, "/") == 0))
                    {

                        if (changeNode3(_ansNode, tmp, _stdNode) == 1) //연산자와 오른쪽항을 바꾸는 함수
                            return 1;
                    }
                }
                if (tmp->right != NULL)
                    tmp = tmp->right->prev; //다음 항으로 이동
                else
                    tmp = NULL; //없다면 NULL, while문 종료
            }
        }
    }
    return getList(_ansNode->prev, _stdNode); //getList를 재귀호출한다.
}
void changeRightLeft(postNode *_node) //양쪽의 항을 바꾸는 함수
{
    postNode *leftend = _node->left;
    if (_node->right == NULL || leftend == NULL)
        return;
    postNode *leftstart = _node->right->prev;                     //왼쪽항의 시작
    postNode *rightend = _node->right, *rightstart = _node->prev; //오른쪽항의 시작 및 끝
    postNode *start = leftend->prev, *end = _node;                //두 항의 끝 node 와 연결되는 mode
    if (leftend == NULL || leftstart == NULL || rightend == NULL || rightstart == NULL ||
        start == NULL || end == NULL) //NULL을 가리키는 포인터가 있다면 error 가 있다.
        return;                       //함수 종료

    /*오른쪽과 왼쪽항의 위치를 바꾸는 알고리즘*/
    rightend->prev = start;
    rightstart->next = leftend;

    leftend->prev = rightstart;
    leftstart->next = end;

    start->next = rightend;
    end->prev = leftstart;
    setOperStr(_node);
    _node = getHeader(_node);
    /*각 항마다 오른쪽 왼쪽을 갱신한다.*/
    setRightANdLeft(_node);
}
int changeNode(postNode *_a_start, postNode *_a_end, postNode *_b_start, postNode *_b_end) //두 node list의 위치를 바꾸는 함수
{
    postNode *tmp;
    if (_a_end == NULL || _a_start == NULL || _b_end == NULL || _b_start == NULL)
        return 0;                                              //NULL을 인자로 받으면 실패를 리턴
    postNode *a_prev = _a_end->prev, *a_next = _a_start->next; //a list의 전 노드, 다음 노드를 가각 가리킨다.
    postNode *b_prev = _b_end->prev, *b_next = _b_start->next; //b list의 전 노드, 다음 노드를 가각 가리킨다.

    if (a_prev != _b_start) //두 리스트 사이에 다른 node가 존재하면
    {
        /*양 끝, 중앙을 기준으로 노드 위치 바꾸기*/
        _a_end->prev = b_prev;
        if (b_prev != NULL)
            b_prev->next = _a_end;

        _a_start->next = b_next;
        if (b_next != NULL)
            b_next->prev = _a_start;

        _b_end->prev = a_prev;
        if (a_prev != NULL)
            a_prev->next = _b_end;

        _b_start->next = a_next;
        if (a_next != NULL)
            a_next->prev = _b_start;
    }
    else
    {
        /*양 끝을 기준으로 노드 바꾸기,*/
        /*두 list는 연속되어 있다.*/
        _a_end->prev = b_prev;
        if (b_prev != NULL)
            b_prev->next = _a_end;

        _a_start->next = _b_end;
        _b_end->prev = _a_start;

        _b_start->next = a_next;
        if (a_next != NULL)
            a_next->prev = _b_start;
    }
    a_prev = getHeader(a_prev);
    return 1;
}

int changeNode2(postNode *_ansNode, postNode *_stdNode, postNode *_tmp)
{
    if (_ansNode == NULL || _tmp == NULL)
        return 0;
    postNode *tmp = _tmp;
    tmp = tmp->right->prev; //이전 연산식 가리키기는 tmp 변수
    char *res;
    if ((tmp != NULL) && (strcmp(_ansNode->name, tmp->name) == 0))
    {
        setRightANdLeft(getHeader(_ansNode));                 //항을 갱신한다.
        if ((tmp->left != NULL) && (tmp->left->prev != NULL)) //
        {
            if (changeNode(_ansNode->prev, _ansNode->right, tmp->prev, tmp->right)) //두 노드를 바꿀수 있다면 바꾸고 if문 수행
            {

                res = getStr(_ansNode);          //string으로 변환
                if (cmpList(_stdNode, res) == 1) //두 연산식 비교
                    return 1;

                setRightANdLeft(getHeader(_ansNode));       //_ansNode의 header부터 왼쪽 오른쪽 항 위치 설정
                if (getList(_ansNode->prev, _stdNode) == 1) //두 노드가 바뀐채로 getList호출
                    return 1;
                changeRightLeft(_ansNode); //오른쪽항과 왼쪽 항을 바꾸고
                setRightANdLeft(_ansNode); //항을 갱신 하고
                res = getStr(_ansNode);
                if (cmpList(_stdNode, res) == 1) //비교 후
                    return 1;
                if (getList(_ansNode->prev, _stdNode) == 1) // 다시 getList 호출
                    return 1;
                changeRightLeft(_ansNode); //오른쪽 항과 왼쪽 항을 복구
                setRightANdLeft(_ansNode); //항을 갱신

                changeNode(_ansNode->prev, _ansNode->right, tmp->prev, tmp->right); //두 노드를 갱신한다.
                setRightANdLeft(getHeader(_ansNode));
            }
        }
        setRightANdLeft(getHeader(_ansNode)); //항을 갱신한다
        if ((tmp->right != NULL) && (tmp->right->prev != NULL))
        {
            if (changeNode(_ansNode->prev, _ansNode->right, tmp->right->prev, tmp->left)) //두 노드를 바꿀수 있다면 바꾸고 if문 수행
            {

                res = getStr(_ansNode);
                if (cmpList(_stdNode, res) == 1)
                    return 1;

                setRightANdLeft(getHeader(_ansNode));
                if (getList(_ansNode->prev, _stdNode) == 1)
                    return 1;
                changeRightLeft(_ansNode);
                setRightANdLeft(_ansNode);
                res = getStr(_ansNode);
                if (cmpList(_stdNode, res) == 1)
                    return 1;
                if (getList(_ansNode->prev, _stdNode) == 1)
                    return 1;
                changeRightLeft(_ansNode);
                setRightANdLeft(_ansNode);
                changeNode(_ansNode->prev, _ansNode->right, tmp->right->prev, tmp->left); //바꾼 두 노드를 다시 복구한다.
                setRightANdLeft(getHeader(_ansNode));
            }
        }
        if (changeNode2(_ansNode, _stdNode, tmp)) //재쥐호출 함으로써 모든 경우를 탐색하게 된다.
            return 1;                             //같은 연산식이 확인 되면 1 리턴
        else
            tmp = NULL;
    }
    return 0;
}
int changeNode3(postNode *_tail, postNode *_tmp, postNode *_stdNode) //연산자와 함께 오른쪽 항도 바뀐다.
{
    char *res;
    changeNode(_tail, _tail->right, _tmp, _tmp->right); //항의 위치를 바꾸는 함수
    setRightANdLeft(getHeader(_tmp));                   //항의 위치를 갱신

    res = getStr(_tmp);
    if (cmpList(_stdNode, res) == 1) //두 후위 연산식 비교
        return 1;
    if (getList(_tmp->prev, _stdNode) == 1) //계속해서 비교하는 함수 호출
        return 1;

    changeNode(_tmp, _tmp->right, _tail, _tail->right); //다시 복구한다.
    setRightANdLeft(getHeader(_tail));
    return 0;
}
postNode *getHeader(postNode *_tail) //해당 리스트의 header를 return
{
    int a = 0;
    while (_tail->priority != _HEADER)
    {
        _tail = _tail->prev;
    }
    return _tail;
}
postNode *getTail(postNode *_tail) //해당 리스트의 tail을 return
{
    while (_tail->next != NULL)
    {
        _tail = _tail->next;
    }
    return _tail;
}

void setRightANdLeft(postNode *_head) //두 항위 위치를 구하는 함수
{
    postNode *tmp;
    while (_head != NULL)
    {
        if (_head->priority == _HEADER) //header라면 다음으로 진행
        {
            _head = _head->next;
            continue;
        }
        if (_head->priority == -1) //일반 문자열이라면
        {
            _head->left = _head; //오른쪽항과 왼쪽항은 자기 자신.
            _head->right = _head;
        }
        else if (_head->priority == _iS_BRACKET) //()일때
        {
            if (strcmp(_head->name, "(") == 0) //여는 괄호라면
            {
                _head->left = _head; //오른쪽항과 왼쪽항은 자기 자신.
                _head->right = _head;
            }
            else //닫는 괄호라면
            {
                tmp = getPreClose(_head);
                if (tmp != NULL)
                    _head->left = tmp->prev; //오른쪽항과 왼쪽항은 여는 괄호의 앞부분, 함수의 괄호만 후위 연산식에 포함된다.
                else
                    _head->left = NULL; //여는 괄호가 없다..
                _head->right = _head->left;
            }
        }
        else if (_head->priority == _iS_SQUARE_BRACKET) //대활호 라면
        {
            if (strcmp(_head->name, "[") == 0) //여는 대괄호라면
            {
                _head->left = _head;
                _head->right = _head;
            }
            else //닫는 대괄호라면
            {
                tmp = getPreClose(_head);
                if (tmp != NULL)
                    _head->left = tmp->prev; //오른쪽항과 왼쪽항은 여는 괄호의 앞부분, []를 가지는 변수
                else
                    _head->left = NULL; //여는 대괄호가 없다..
                _head->right = _head->left;
            }
        }
        else if (isOneOperend(_head->name)) //단항연산자라면
        {
            if (_head->prev != NULL)
                _head->right = _head->prev->left;
            else
                _head->right = NULL;    //피연사가 없다..
            _head->left = _head->right; //하나의 피연사자만을 가리킨다.
        }
        else if (_head->priority == _IS_POINTER_OPER) //포인터라면
        {
            if (_head->prev != NULL)
                _head->right = _head->prev->left; //오른쪽항 설정.
            else
                _head->right = NULL; // 오른쪽항에 문제가 있다.
            if (_head->prev != NULL)
            {
                if (_head->prev->prev != NULL)       //(char *) &buf, 캐스팅을 위한 처리
                    _head->left = _head->prev->left; //캐스팅을 후위 연산식으로 변환하면 char * buf & 가된다
                else
                    _head->left = _head->right; //두단계 전에 node가 없다면 하나의 변수에 대한 포인터
            }
            else
                _head->left = _head->right; //앞 단계에 노드가 없다면 하나의 변수에 대한 포인터
        }
        else //일반 연산자 라면
        {
            if (_head->prev != NULL)
                _head->right = _head->prev->left; //오른쪽 항은 이전항의 왼쪽항까지
            else
                _head->right = NULL; //이전항이 없으면 NULL
            tmp = _head->right;      //오른쪽항의 시작으로 이동
            if (tmp != NULL)
            { //오른쪽 항의 시작부분의 이전노드, 그 노드의 왼쪽항의 시작부분이 _head의 왼쪽항의 시작부분이 된다.
                if (tmp->prev != NULL)
                    _head->left = tmp->prev->left;
                else
                    _head->left = NULL;
            }
            else
                _head->left = NULL;
        }
        _head = _head->next; //다음 노드로 이동하며 반복 수행
    }
}

postNode *postPush(postNode *_tail, char *_str) //후위 연산자 리스트에 노드 추가하는 함수
{
    postNode *tmp = _tail; //_tail 임시 저장

    _tail = (postNode *)malloc(sizeof(postNode)); //새로운 노드 생성
    _tail->name = _str;                           //가리키는 문자열 할당
    _tail->priority = getPriority(_str);          //우선순위 할당

    if (tmp != NULL) //기존의 _tail과 새로운 _tail 연결
        tmp->next = _tail;
    if (_tail != NULL)
        _tail->prev = tmp;

    return _tail;
}
postNode *postPop(postNode *_tail) //후위 연산자 리스트에 노드 제거하는 함수
{
    postNode *tmp = NULL;
    if (_tail != NULL)
        tmp = _tail->prev; //_tail의 이전 노드 가리킨다.
    if (tmp != NULL)
        tmp->next = NULL; //_tail 제거
    free(_tail);          //할당된 공간 반남
    return tmp;           //_tail->prev가 tail이 된다.
}

postNode *makePostStack(char *_str, postNode *_tail)
{
    char *tmp;
    int open1 = 0;
    int open2 = 0;
    postNode *head, *tail, *strhead, *strTail;

    head = (postNode *)malloc(sizeof(postNode)); //연산자를 가리키는 임시 리스트
    head->priority = _HEADER;
    tail = head;

    strhead = (postNode *)malloc(sizeof(postNode)); //string 을 가리키는 리스트
    strhead->priority = _HEADER;
    strTail = strhead;

    tmp = strtok(_str, " ");
    if (tmp == NULL)
        return NULL;
    strTail = postPush(strTail, tmp); //토큰으로 분리된 string을 가리키는 연산자 리스트
    while ((tmp = strtok(NULL, " ")) != NULL)
    {
        strTail = postPush(strTail, tmp); //NULL이 아닐때까지 노드를 추가한다.
    }
    setPriority(strhead); //괄호에 대한 우선순위 설정한다.

    strTail = strhead->next;

    while (strTail != NULL)
    {
        if (strTail->priority == -1)
        {
            _tail = postPush(_tail, strTail->name);
        }

        else
        {
            if (strTail->priority == _iS_BRACKET) //연산자 괄호가 아니고
            {
                if (strcmp(strTail->name, "(") == 0)        //여는 괄호라면
                    _tail = postPush(_tail, strTail->name); //후위 연산자에 추가
            }
            else if (strTail->priority == _iS_SQUARE_BRACKET) //연산자 괄호고
            {
                if (strcmp(strTail->name, "[") == 0)        //여는 괄호라면
                    _tail = postPush(_tail, strTail->name); //후위 연산자에 추가
            }                                               // 닫는 괄호일 경우 괄호안의 연산자에 대해 먼저 처리를 해주어야 하기 때문에 추가하지 않는다.

            if (strcmp(strTail->name, ")") == 0) //닫는 괄호라면
            {
                while ((tail->name != NULL) && (strcmp(tail->name, "("))) //여는 괄호가 나올때까지
                {                                                         //후위 연산식에 임시 연산자를 후입선출의 방식으로 꺼내 추가한다.
                    _tail = postPush(_tail, tail->name);
                    tail = postPop(tail);
                }
                if ((tail->name != NULL))
                { //연산자 리스트에서 여는 괄호를 pop 해준다.
                    tail = postPop(tail);
                }
                else
                {
                    _tail = postPush(_tail, strTail->name);
                }
                if (strTail->priority == _iS_BRACKET)       //연산자 괄호가 아니라면
                    _tail = postPush(_tail, strTail->name); //후위 연산자에 추가
                strTail = strTail->next;                    //다음 스트링 리스트로 이동해서
                continue;                                   //계속진행
            }

            if (strcmp(strTail->name, "]") == 0)
            {
                while ((tail->name != NULL) && (strcmp(tail->name, "[")))
                { //후위 연산식에 임시 연산자를 후입선출의 방식으로 꺼내 추가한다.
                    _tail = postPush(_tail, tail->name);
                    tail = postPop(tail);
                }
                if ((tail->name != NULL))
                { //연산자 리스트에서 여는 괄호를 pop 해준다.
                    tail = postPop(tail);
                }
                else
                {
                    _tail = postPush(_tail, strTail->name);
                }
                if (strTail->priority == _iS_SQUARE_BRACKET) //연산자 괄호가 아니라면
                    _tail = postPush(_tail, strTail->name);  //후위 연산자에 추가
                strTail = strTail->next;                     //다음 스트링 리스트로 이동해서
                continue;                                    //계속진행
            }

            if (tail->name != NULL)
            { //부정연산자가 두번 연속 나오게 되면
                if ((strcmp(tail->name, "!") == 0) && (strcmp(strTail->name, "!") == 0))
                { //후위 연산식에서 pop 하고 string 리스트를 다음으로 이동해 부정연산자 무시
                    tail = postPop(tail);
                    strTail = strTail->next;
                }
                else if ((strcmp(tail->name, "~") == 0) && (strcmp(strTail->name, "~") == 0))
                {
                    tail = postPop(tail);
                    strTail = strTail->next;
                }
            }

            while ((strTail != NULL) && (strTail->priority >= tail->priority))
            { //높은 우선순위는 후위 연산식에 추가
                if ((tail->name == NULL) ||
                    (strcmp(tail->name, "(") == 0) ||
                    (strcmp(tail->name, "[") == 0))
                    break;                           //여는 괄호 만나기 전까지 반복
                _tail = postPush(_tail, tail->name); //임시 리스트에서 꺼내서 후위 연산자에 추가
                tail = postPop(tail);                //임시 리스트의 마지막 노드 제거
            }
            if (strTail != NULL)
                tail = postPush(tail, strTail->name); //낮은 우선순위는 임시 리스트에 저장
        }
        if (strTail != NULL)
            strTail = strTail->next; //다음 문자열로 이동
    }

    while ((tail != NULL) && (tail->name != NULL))
    { //남은 연산식을 후위 연산식에 추가한다
        if (((strcmp(tail->name, "(") != 0) && (strcmp(tail->name, "[") != 0)))
        { //괄호는 무시한다.
            _tail = postPush(_tail, tail->name);
        }

        tail = postPop(tail); //이시 연산자 리스트의 마지막 노드를 제거한다.
    }

    return head;
}
void combinStr(char *str) //원래 하나의 연산자는 하나로 묶는다. ex -> ==
{
    int index = -1, resIndex = -1;
    while (str[++index] != 0)
    {
        str[++resIndex] = str[index];
        if (isOperend(str[index]))
        {
            if (isOperend(str[index + 2]))
            {
                if (str[index] == '+')
                {
                    if (str[index + 2] == '+' || str[index + 2] == '=') // ++, +=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '-')
                {
                    if (str[index + 2] == '-' || str[index + 2] == '=' || str[index + 2] == '>') // --, -= ,->
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '*')
                {
                    if (str[index + 2] == '=' || str[index + 2] == '*') // *=, **
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '/')
                {
                    if (str[index + 2] == '=') // /=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '%')
                {
                    if (str[index + 2] == '=') // %=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '=')
                {
                    if (str[index + 2] == '=') // ==
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '!')
                {
                    if (str[index + 2] == '=') // !=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '|')
                {
                    if (str[index + 2] == '=' || str[index + 2] == '|') // |= ||
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '&')
                {
                    if (str[index + 2] == '=' || str[index + 2] == '&') // &= &&
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '<')
                {
                    if (str[index + 2] == '=') // <=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                    if (str[index + 2] == '<') //<<
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                        if (str[index + 2] == '=') //<<=
                        {
                            index += 2;
                            str[++resIndex] = str[index];
                        }
                    }
                }
                else if (str[index] == '>')
                {
                    if (str[index + 2] == '=') //>=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                    if (str[index + 2] == '>') //>>
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                        if (str[index + 2] == '=') //>>=
                        {
                            index += 2;
                            str[++resIndex] = str[index];
                        }
                    }
                }
                else if (str[index] == '^')
                {
                    if (str[index + 2] == '=') // ^=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
                else if (str[index] == '~')
                {
                    if (str[index + 2] == '=') // ~=
                    {
                        index += 2;
                        str[++resIndex] = str[index];
                    }
                }
            }
        }
    }
    str[++resIndex] = 0;
}

int getPriority(char *str) //연산자 우선순위에 따른 우선순위 얻는 함수
{
    if (strcmp(str, " ") == 0)
        return -1;
    if (strcmp(str, "(") == 0 || strcmp(str, ")") == 0 ||
        strcmp(str, "[") == 0 || strcmp(str, "]") == 0)
        return 1;
    if (strcmp(str, "++") == 0 || strcmp(str, "--") == 0 ||
        strcmp(str, "~") == 0 || strcmp(str, "!") == 0)
        return 2;
    if (strcmp(str, "*") == 0 || strcmp(str, "%") == 0 ||
        strcmp(str, "/") == 0)
        return 3;
    if (strcmp(str, "+") == 0 || strcmp(str, "-") == 0)
        return 4;
    if (strcmp(str, "<<") == 0 || strcmp(str, ">>") == 0)
        return 5;
    if (strcmp(str, "<") == 0 || strcmp(str, "<=") == 0 ||
        strcmp(str, ">") == 0 || strcmp(str, ">=") == 0)
        return 6;
    if (strcmp(str, "==") == 0 || strcmp(str, "!=") == 0)
        return 7;
    if (strcmp(str, "&") == 0)
        return 8;
    if (strcmp(str, "^") == 0)
        return 9;
    if (strcmp(str, "|") == 0)
        return 10;
    if (strcmp(str, "&&") == 0)
        return 11;
    if (strcmp(str, "||") == 0)
        return 12;
    if (strcmp(str, "?") == 0)
        return 13;
    if (strcmp(str, "=") == 0 || strcmp(str, "*=") == 0 ||
        strcmp(str, "+=") == 0 || strcmp(str, "/=") == 0 ||
        strcmp(str, "%=") == 0 || strcmp(str, "&=") == 0 ||
        strcmp(str, "|=") == 0 || strcmp(str, "<<=") == 0 ||
        strcmp(str, ">>=") == 0)
        return 14;
    if (strcmp(str, ",") == 0)
        return 13;
    return -1;
}
postNode *getClose(postNode *_head) //닫는 괄호 위치 탐색
{
    char *p_name = _head->name; //현제 노드의 문자열
    char *res;                  //찾아야 되는 괄호 가리킨다
    int stack = 0;              //stack을 이용해 여는 괄호과 닫는 괄호의 쌍이 맞게되는 node 를 리턴
    if (strcmp(p_name, "(") == 0)
        res = ")";
    else if (strcmp(p_name, "[") == 0)
        res = "]";
    while (_head != NULL)
    {
        if (strcmp(_head->name, p_name) == 0)   //현제 node와 같은 문자열을 만나면
            stack++;                            //stack 변수 증가
        else if (strcmp(_head->name, res) == 0) //찾아야 되는 문자열 만나면
            stack--;                            //stack 감소
        if (stack == 0)                         // stack이 0이 되는 node 리턴
            return _head;
        _head = _head->next;
    }
    return _head;
}
postNode *getPreClose(postNode *_head) //여는 괄호 위치 탐색
{
    char *p_name = _head->name; //현제 노드의 문자열
    char *res;                  //찾아야 되는 괄호 가리킨다
    int stack = 0;              //stack을 이용해 여는 괄호과 닫는 괄호의 쌍이 맞게되는 node 를 리턴
    if (strcmp(p_name, ")") == 0)
        res = "(";
    else if (strcmp(p_name, "]") == 0)
        res = "[";
    while (_head->priority != _HEADER)
    {
        if (strcmp(_head->name, p_name) == 0)   //현제 node와 같은 문자열을 만나면
            stack++;                            //stack 변수 증가
        else if (strcmp(_head->name, res) == 0) //찾아야 되는 문자열 만나면
            stack--;                            //stack 감소
        if (stack == 0)                         // stack이 0이 되는 node 리턴
            return _head;
        _head = _head->prev;
    }
    return _head;
}
postNode *setPriority(postNode *_head) //괄호의 우선순위 설정하는 함수
{
    postNode *tmp = _head;
    while (_head != NULL)
    {
        if (_head->priority == _HEADER)
        {
            _head = _head->next;
            continue;
        }
        if (_head->priority == -1)
        {
            if (_head->next != NULL)
            {
                tmp = _head->next;
                if (strcmp(tmp->name, "(") == 0)
                {
                    tmp->priority = _iS_BRACKET; //해당 노드도 우선순위 설정한다
                    tmp = getClose(tmp);         //닫는 괄호 찾아서
                    if (tmp != NULL)
                        tmp->priority = _iS_BRACKET; //해당 노드도 우선순위 설정한다
                }
                else if (strcmp(tmp->name, "[") == 0)
                {
                    tmp->priority = _iS_SQUARE_BRACKET; //해당 노드도 우선순위 설정한다
                    tmp = getClose(tmp);                //닫는 괄호 찾아서
                    if (tmp != NULL)
                        tmp->priority = _iS_SQUARE_BRACKET; //해당노드의 우선순위 찾는다
                }
            }
        }
        else if (strcmp(_head->name, "]") == 0) //[][]에 대한 처리
        {
            if (_head->next != NULL)
            {
                tmp = _head->next;
                if (strcmp(tmp->name, "[") == 0)
                {
                    tmp->priority = _iS_SQUARE_BRACKET; //해당 노드도 우선순위 설정한다
                    tmp = getClose(tmp);                //닫는 괄호 찾아서
                    if (tmp != NULL)
                        tmp->priority = _iS_SQUARE_BRACKET; //해당노드의 우선순위 찾는다
                }
            }
        }
        _head = _head->next;
    }
    return _head;
}
char *makeSpace(char *str) //연산자 공백으로 구분하는 함수, 여러번 반복되는 공백 제거
{
    int index = -1, resIndex = -1;
    char *res = (char *)malloc(sizeof(char) * _BUF_SIZE_CMP);
    memset(res, 0, sizeof(res));
    while (str[++index] != 0)
    {
        if ((index == 0) && (str[index] == ' ')) //문자열 앞부분의 공백 제거
            continue;
        if ((resIndex >= 0) && (res[resIndex] == ' ') && (str[index] == ' ')) //반복되는 공백 제거
            continue;
        res[++resIndex] = str[index];
        if ((resIndex >= 0))
        {
            if (isOperend(str[index])) //연산자라면
            {
                if (res[resIndex - 1] == ' ') //앞과
                {
                    res[++resIndex] = ' '; //뒤에 공백 추가
                }
                else
                {
                    res[resIndex] = ' '; //문자열을 공백으로 구분
                    res[++resIndex] = str[index];
                    res[++resIndex] = ' ';
                }
            }
        }
    }
    return res;
}
int isOperend(char a) //연산자인지 확인
{
    if (a == 0)
        return 0;
    for (int i = 0; i < operSize; i++)
        if (a == operList[i])
            return 1;
    return 0;
}

int isChOperend(char *a) //양 옆이 바뀡도 가능한 연산자인지 확인
{
    for (int i = 0; i < canchOperSize; i++)
        if (strcmp(a, canchOperList[i]) == 0)
            return 1;
    return 0;
}
int isOneOperend(char *a) //단항 연산자 인지 확인
{
    for (int i = 0; i < oneOperSize; i++)
        if (strcmp(a, oneOperList[i]) == 0)
            return 1;
    return 0;
}
void setOperStr(postNode *_node)
{                                                     //비교 연산자 바꾸는 함수
    if (strcmp(_node->name, operStrSet[0]) == 0)      // < 일겅우
        _node->name = operStrSet[1];                  // > 로 바꾼다
    else if (strcmp(_node->name, operStrSet[1]) == 0) // > 일겅우
        _node->name = operStrSet[0];                  // < 로 바꾼다
    else if (strcmp(_node->name, operStrSet[2]) == 0) // <= 일겅우
        _node->name = operStrSet[3];                  // >= 로 바꾼다
    else if (strcmp(_node->name, operStrSet[3]) == 0) // >= 일겅우
        _node->name = operStrSet[2];                  // >= 로 바꾼다
}
char *removeSpace(char *str) //-와 포인터 연산에 대한 피연산자와의 공백을 없앤다
{

    int in1 = 0, in2 = 0;
    while (str[in1] != 0)
    {
        str[in2++] = str[in1];
        if (str[in1] == '-' || str[in1] == '*' || str[in1] == '&')
        {
            if (str[in1 + 1] == ' ' && (in1 == 0 || str[in1 - 1] == ' ')) //양 옆에 공백으로 구분되어 하나의 연사식이라면
            {
                if (0 <= in1 - 2 && isOperend(str[in1 - 2])) // 이전의 문자열이 연산식이라면
                {
                    if ((str[in1 - 2] == ')') || (str[in1 - 2] == ']')) // 닫는 괄호를 제외한 다른 연산자라면
                    {
                        in1++;
                        continue;
                    }
                    in1 += 2; //공백을 제거한다
                    continue;
                }
                else if (in1 - 2 < 0) // 앞에 문자가 없으면
                {
                    in1 += 2; //공백을 제거한다
                    continue;
                }
            }
        }
        in1++;
    }
    str[in2] = 0;
}

void rmDupNotOper(postNode *_node) //부정연산자 중복 사용 제거
{
    if (_node == NULL)
        return;

    postNode *node = getHeader(_node); //header부터 시작한다.
    if (node == NULL)
        return;

    node = node->next;
    if (node == NULL)
        return;

    while (node->next != NULL) //노드를 따라가며 탐색
    {
        if (node->name != NULL && node->next->name != NULL)
        {
            if ((strcmp(node->name, "!") == 0) && (strcmp(node->name, node->next->name) == 0))
            {                                        //자신과 자신의 다음 노드가 부전연산자 라면
                node->prev->next = node->next->next; //전 노드와 다음 다음 노드 연결
                if (node->next->next != NULL)
                {
                    node->next->next->prev = node->prev; //다음노드와 전 노드 연결
                    node = node->next->next;             // 다음 노드를 건너뛰고 진행
                    continue;
                }
            }
            else if ((strcmp(node->name, "~") == 0) && (strcmp(node->name, node->next->name) == 0))
            {                                        //자신과 자신의 다음 노드가 부전연산자 라면
                node->prev->next = node->next->next; //전 노드와 다음 다음 노드 연결
                if (node->next->next != NULL)
                {
                    node->next->next->prev = node->prev; //다음노드와 전 노드 연결
                    node = node->next->next;             // 다음 노드를 건너뛰고 진행
                    continue;
                }
            }
        }
        if (node != NULL)
            node = node->next;
        else
            node = NULL;
    }
}
