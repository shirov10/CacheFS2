
#include <iostream>
#include "CacheFS.h"



int main()
{
    char* buf[1000];
    std::cout << "begin" << std::endl;
    CacheFS_init(5,LFU,0,0);
    int id1=CacheFS_open("/home/shir/Desktop/f1/la");
    int id2=CacheFS_open("/home/shir/Desktop/f1/lala.file");

    std::cout<<"Opened files"<<std::endl;

    CacheFS_pread(id1,buf,100,100);

}
