
#include <iostream>
#include "CacheFS.h"
#include <unistd.h> //for read

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main()
{
    char buf[4096];
    std::cout << "begin" << std::endl;
    CacheFS_init(10,LRU,0.5,0.5);
    int id1=CacheFS_open("/home/omri/Desktop/blob");
    int id2=CacheFS_open("/home/omri/Desktop/blob2");

    std::cout<<"Opened files"<<std::endl;



    int read1=CacheFS_pread(id1,(void*)buf,100,5000);
    std::cout << buf << std::endl;

    int read2=CacheFS_pread(id2,(void*)buf,100,5000);
    std::cout << buf << std::endl;

    int read3=CacheFS_pread(id2,(void*)buf,100,8000);
    std::cout << buf << std::endl;

    CacheFS_print_stat("/home/omri/Desktop/logStat");
    CacheFS_print_cache("/home/omri/Desktop/logCache");

}
