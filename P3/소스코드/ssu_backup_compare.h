void process_compare(char *commandVector[])
{
    char *file1 = NULL, *file2 = NULL;
    struct stat stat1, stat2;
    int index = 1;

    /*commandVector를 이용해 두 파일의 경로 얻기*/
    for (index = 1; commandVector[index] != NULL; index++)
    {
        /*file1 경로 얻기*/
        if (file1 == NULL)
        {
            /*절대 경로 얻기*/
            if ((file1 = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
            {
                fprintf(stderr, "compare>[error] realpath error for %s\n", commandVector[index]);
                fprintf(stderr, "compare <FILENAME1> <FILENAME2>\n");
                return;
            }
        }
        /*file2 경로 얻기*/
        else if (file2 == NULL)
        {
            /*절대 경로 얻기*/
            if ((file2 = realpath(get_real_PATH(commandVector[index]), NULL)) == NULL) // 원본 파일의 절대 경로
            {
                fprintf(stderr, "compare>[error] realpath error for %s\n", commandVector[index]);
                fprintf(stderr, "compare <FILENAME1> <FILENAME2>\n");
                return;
            }
        }
        /*file1 file2 외 다른 정보가 입력 되었으면 error*/
        else
        {
            fprintf(stderr, "compare <FILENAME1> <FILENAME2>\n");
            return;
        }
    }

    /*file1 경로 또는 file2 경로가 입력되지 않은 경우*/
    if (file1 == NULL || file2 == NULL)
    {
        fprintf(stderr, "compare <FILENAME1> <FILENAME2>\n");
        return;
    }

    /*file1이 존재하지 않는 경우*/
    if (access(file1, F_OK) < 0)
    {
        fprintf(stderr, "compare>[error] %s not exit\n", file1);
        return;
    }
    /*file2가 존재하지 않는 경우*/
    if (access(file2, F_OK) < 0)
    {
        fprintf(stderr, "compare>[error] %s not exit\n", file2);
        return;
    }

    /*file1정보 얻어오기*/
    if (lstat(file1, &stat1) < 0)
    {
        fprintf(stderr, "compare>[error] lstat error for %s\n", file1);
        return;
    }
    /*file2정보 얻어오기*/
    if (lstat(file2, &stat2) < 0)
    {
        fprintf(stderr, "compare>[error] lstat error for %s\n", file2);
        return;
    }

    /*file1이 regular파일인지 확인*/
    if (!S_ISREG(stat1.st_mode))
    {
        fprintf(stderr, "compare>[error] %s not regular\n", file1);
        return;
    }
    /*file2가 regular파일인지 확인*/
    if (!S_ISREG(stat2.st_mode))
    {
        fprintf(stderr, "compare>[error] %s not regular\n", file2);
        return;
    }
    printf("compare : \n");
    /*두 파일의 mtime과 크기가 같은 경우 같은 파일*/
    if ((stat1.st_mtime == stat2.st_mtime) &&
        (stat1.st_size == stat2.st_size))
    {
        printf("SAME\n");
    }
    /*두 파일의 mtime 또는 크기가 같은 경우 다른파일*/
    else
    {
        printf("NAME\t\t\t\t\t\t\t\t       MTIME\t\t    SIZE\n");
        printf("%-70s %-20ld %-10ld\n", file1, stat1.st_mtime, stat1.st_size); //file1의 정보 출력
        printf("%-70s %-20ld %-10ld\n", file2, stat2.st_mtime, stat2.st_size); //file2의 정보 출력
        printf("NOT SAME\n");
    }
    /*할당된 공간 해제*/
    free(file1);
    free(file2);
}