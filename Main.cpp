//
// Created by omri on 05/06/17.
//
#include <iostream>
#include "CacheFS.h"
#include "Cache2.h"



int main()
{
    std::cout << "begin" << std::endl;
    CacheBlock::initelizeBlocks(512, 100);
    std::cout << "blocks initelized" << std::endl;

    User_file_descriptor blob_ufd = CacheFile::createFile("/tmp/blob");
    std::cout << "file created" << std::endl;

    char a[100];
    CacheFile::ufd2file_map[blob_ufd]->CachFileReadFromBlock(a, 50, 0, 0);
    std::cout << "block readed" << std::endl;

    std::cout << a << std::endl;

}
