//
// Created by shir on 29/05/17.
//

#include "Cache.h"
#include <sys/stat.h>


#include <cstdlib> //for realpath

#include <string.h> //for memcpy
#include <algorithm> ///for min
#include <ctime>

#include <unistd.h> //for read

#include <iostream> //TODO delete

// region Block

Block::Block(int blockSize) {
    content=(char*)aligned_alloc(blockSize,blockSize);
    //content=new char[blockSize];
    isEmpty=true;
}

//endregion



//region Cache

Cache::Cache(int blocks_num) {
    //get the block size
    struct stat fi;
    stat("/tmp", &fi);
    blockSize = (int)fi.st_blksize;
    blocks= std::vector<Block*>(blocks_num);
    for (int i = 0; i <(int)blocks.size(); ++i) {
        blocks[i]=new Block(blockSize);
    }
}

Cache::~Cache() {

    for(int i = 0; i <(int)blocks.size(); ++i){
        free((char*)blocks[i]->realPath); //frees the memory that was allocated with malloc
        free (blocks[i]);
    }
}

void Cache::addFile(const char *filePath, int id) {
    fileIDs[id]=filePath;
}

void Cache::removeFile(int id) {
    fileIDs.erase(id);
}

const char* Cache::getRealPath(int file_id) {
    return realpath(fileIDs[file_id], NULL);
    // please notice that realpath is alocated using maloc, and must be freed!
}

/**
 * finds the block number blockNumInFile int the cache. If it's not there- returns -1
 */
int Cache::findBlock(const char *path, int blockNumInFile) {
    for (int i=0; i<(int)blocks.size();++i ){
        if (!blocks[i]->isEmpty && !strcmp(blocks[i]->realPath,path) && blockNumInFile==blocks[i]->blockNumInFile){
            return i;
        }
    }
    return -1;
}

Block* Cache::cacheBlock(int fd, const char *path, int blockNumInFile) {
    int b=blockNumToUse();
    Block * blockPtr=blocks[b];

    int read_bytes=(int)pread(fd,(void*)blockPtr->content,(size_t)blockSize,(off_t)(blockNumInFile*blockSize));

    if (read_bytes==-1){
        return nullptr;
    }

    blockPtr->isEmpty=false;
    blockPtr->realPath=path;
    blockPtr->refCount=1;
    blockPtr->lastAccessTime=std::time(nullptr);
    blockPtr->length=read_bytes;
    blockPtr->blockNumInFile=blockNumInFile;
    return blockPtr;
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

    int alreadyCopied=0, blockNumInCache,bytesToCopy, offsetInBlock;
    Block* block_ptr;

    //search for the blocks in the cache
    for (int i = firstBlock; i <= lastBlock; ++i) {
        blockNumInCache=findBlock(realPath,i);
        if(blockNumInCache==-1){ //meaning the block is not in the cache
            std::cout<<"Read from disk. File: "<<file_id<<" Block: "<<i<<std::endl; //TODO delete
            block_ptr=cacheBlock(file_id,realPath,i);
            if(block_ptr== nullptr){
                return -1;
            }
        }
        else{ //meaning the block was already in the cache
            std::cout<<"Read from cache. File: "<<file_id<<" Block: "<<i<<std::endl; //TODO delete
            block_ptr=blocks[blockNumInCache];
            block_ptr->lastAccessTime = std::time(nullptr);
            block_ptr->refCount++;
        }

        bytesToCopy=std::min((int)count-alreadyCopied,block_ptr->length); //How much bytes to copy

        offsetInBlock=(i==firstBlock)? (int)offset%blockSize:0;

        //copy memory from the cache to the buffer
        memcpy((char*)buf+alreadyCopied,block_ptr->content+offsetInBlock,(size_t)bytesToCopy);
        alreadyCopied+=bytesToCopy;
    }
    return alreadyCopied;

}

int Cache::blockNumToUse()
{
    auto it = blocks.begin();
    for(it = blocks.begin(); it != blocks.end(); it++)
    {
        Block* block = *it;
        if (block->isEmpty)
        {
            break;
        }
    }
    if (it != blocks.end())
    {
        int index = (int)std::distance( this->blocks.begin(), it );
        return index;
    }
    else
    {
        return this->blockNumToUseAlogo();
    }


}



//endregion


//region Cache_LRU

Cache_LRU::Cache_LRU(int blocks_num):Cache(blocks_num)
{
};

Cache_LRU::~Cache_LRU() {
}

int Cache_LRU::blockNumToUseAlogo()
{
    auto compareFunc = [](Block* a, Block* b) { return a->lastAccessTime > b->lastAccessTime; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    int index = (int)std::distance( this->blocks.begin(), it );
    return index;
}


//endregion

//region Cache_LFU

Cache_LFU::Cache_LFU(int blocks_num):Cache(blocks_num){
};

Cache_LFU::~Cache_LFU() {
}

int Cache_LFU::blockNumToUseAlogo()
{
    auto compareFunc = [](Block* a, Block* b) { return a->refCount > b->refCount; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    int index = std::distance( this->blocks.begin(), it );
    return index;
}



//endregion

//region Cache_FBR

Cache_FBR::Cache_FBR(int blocks_num, double f_old,double f_new):Cache(blocks_num){
    _f_new=f_new;
    _f_old=f_old;
};

Cache_FBR::~Cache_FBR() {
}
int Cache_FBR::blockNumToUseAlogo()
{
}


//endregion