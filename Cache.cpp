//
// Created by shir on 29/05/17.
//

#include "Cache.h"
#include <sys/stat.h>

#include <cstdlib> //for realpath


//region Cache

Cache::Cache(int blocks_num) {
    //get the block size
    struct stat fi;
    stat("/tmp", &fi);
    int blksize = fi.st_blksize;

    buffer= new char[blocks_num*blksize];

}

Cache::~Cache() {
    delete buffer;
//    for(auto it=fileIDs.begin();it!=fileIDs.end();++it){ //frees the memory that was allocated with malloc
//        free((char*)it->first);
//    }
}

void Cache::addFile(const char *filePath, int id) {
    fileIDs[filePath]=id;
}

const char * Cache::getFilePath(int fileID) {
    for (auto it=fileIDs.begin();it!=fileIDs.end();++it){
        if (it->second==fileID)
            return it->first;
    }
    return nullptr;
}



//endregion

//region Cache_LRU

Cache_LRU::Cache_LRU(int blocks_num):Cache(blocks_num){
};

Cache_LRU::~Cache_LRU() {
}



//endregion

//region Cache_LFU

Cache_LFU::Cache_LFU(int blocks_num):Cache(blocks_num){
};

Cache_LFU::~Cache_LFU() {
}



//endregion

//region Cache_FBR

Cache_FBR::Cache_FBR(int blocks_num, double f_old,double f_new):Cache(blocks_num){
    _f_new=f_new;
    _f_old=f_old;
};

Cache_FBR::~Cache_FBR() {
}



//endregion