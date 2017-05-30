//
// Created by shir on 29/05/17.
//

#include "Cache.h"
#include <sys/stat.h>

#include <cstdlib> //for realpath

#include <string.h> //for memcpy


//region Cache

Cache::Cache(int blocks_num) {
    //get the block size
    struct stat fi;
    stat("/tmp", &fi);
    blockSize = fi.st_blksize;

    buffer= new char[blocks_num*blockSize];

}

Cache::~Cache() {
    delete buffer;
//    for(auto it=fileIDs.begin();it!=fileIDs.end();++it){ //frees the memory that was allocated with malloc
//        free((char*)it->first);
//    }
}

void Cache::addFile(const char *filePath, int id) {
    fileIDs[id]=filePath;
}

void Cache::removeFile(int id) {
    fileIDs.erase(id);

}

const char* Cache::getRealPath(int file_id) {
    return realpath(fileIDs[file_id], NULL);
}

int Cache::readFile(int file_id, void *buf, size_t count, off_t offset) {

    if(fileIDs.count(file_id)==0){//meaning the file is not open
        return -1;
    }

    //calculate the wanted blocks
    int firstBlock, lastBlock;
    firstBlock=(int)offset/blockSize;
    lastBlock=((int)offset+(int)count)/blockSize;

    //Get the real path of the given file
    const char * realPath=getRealPath(file_id);

    int alreadyCopied=0, blockNumInCache,bytesToCopy;

    //search for the blocks in the cache
    for (int i = firstBlock; i <= lastBlock; ++i) {
        auto it=filesInfo.find(realPath);
        if(it!=filesInfo.end() && it->second.count(i)!=0){ //meaning the block i is in the cache

            blockNumInCache=it->second.find(i)->second; //The block number in the cache buffer

            bytesToCopy=(i!=lastBlock)? blockSize:((int)count)%blockSize; //How much bytes to copy TODO take care of end of file!

            //copy memory from the cache to the buffer
            memcpy(buf+alreadyCopied,&buffer[blockNumInCache],bytesToCopy);
            alreadyCopied+=bytesToCopy;
        }
        else{ //The block is not in the cache

        }
    }

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