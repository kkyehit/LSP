/*commandVector를 인자로 받아 system 함수 수행*/
void process_fork_system(char *commandVector[]){
    int index = 0;//commandVector의 인자를 가리키는 변수
    int BUFFER_SIZE_FOR_SYSTEM = 0;//총 명령어의 길이저장하는 변수
    char *systemCommand;//전체 명령문을 만들기 위한 변수
    /*BUFFER_SIZE_FOR_SYSTEM  구하기*/
    for(index = 0; commandVector[index] != NULL; index++){
        BUFFER_SIZE_FOR_SYSTEM += strlen(commandVector[index]) + 1;
    }    
    /*systemCommand에 새로운 공간 할당 및 초기화*/
    systemCommand = (char *)calloc(sizeof(char), BUFFER_SIZE_FOR_SYSTEM);
    memset(systemCommand, 0, sizeof(systemCommand));
    /*systemCommand 만들기*/
    for(index = 0; commandVector[index] != NULL; index++){
        strcat(systemCommand, commandVector[index]);
        strcat(systemCommand, " ");
    }
    /*systemCommand의 마지막 공백 없애기*/
    systemCommand[strlen(systemCommand)-1] = 0;
    /*system 함수 수행*/
    system(systemCommand);
    /*할당된 공간 반환*/
    free(systemCommand);
}