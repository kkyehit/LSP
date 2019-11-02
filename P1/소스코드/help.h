/*help 메세지*/
const char *h_msg[3] = {"Usage : ", " <STUDENTDIR> <TRUEDIR> [OPTION]",
                        "Option :\n-e <DIRNAME>       print error on \'DIRNAME/ID/qname_error.txt\' file\n-t <QNAMES>        complie QNAME.C woth -lpthread option\n-h                 print usage\n-p                 print student\'s score ans total average\n-c <IDS>           print ID`s score\n"};

/*help 메세지를 출력하는 함수*/
void print_h_msg(char *_filename)
{
   write(1, h_msg[0], strlen(h_msg[0]));
   write(1, _filename, strlen(_filename));
   write(1, h_msg[1], strlen(h_msg[1]));
   write(1, "\n", 1);
   write(1, h_msg[2], strlen(h_msg[2]));
}