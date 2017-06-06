//
// Created by shir on 29/05/17.
//

#include "Cache.h"
#include <sys/stat.h>


#include <cstdlib> //for realpath

#include <string.h> //for memcpy
#include <algorithm> ///for min
#include <ctime>    // for the time of LRU
#include <limits.h>
#include <string>
#include <bits/fcntl-linux.h>
#include <fcntl.h>

// region Block

Block::Block(int blockSize) {
    content=new char[blockSize];
}


//endregion



//region Cache

Cache::Cache(int blocks_num) {
    //get the block size
    struct stat fi;
    stat("/tmp", &fi);
    blocks= std::vector<Block*>(blocks_num);
    for (int i = 0; i <(int)blocks.size(); ++i) {
        blocks[i]=new Block(blockSize);
    }



}

Cache::~Cache() {

    for(int i = 0; i <(int)blocks.size(); ++i){
        free((char*)blocks[i]->realPath); //frees the memory that was allocated with malloc
        delete blocks[i];
    }
}

UFD Cache::cach_open(const char *path)
{
    OsFD fd = checkExist(path);
    if(fd == -1)
    {
        // file do not exist in system - open it!!!
        fd = open(path,O_RDONLY | O_DIRECT | O_SYNC);
        if(fd<0){      //TODO we should support only files under "/tmp"!
            return -1;
        }
        // update the new abs path in the path map
        char c_new_path[PATH_MAX+1];
        realpath(path, c_new_path);
        string str_new_path(c_new_path);
        osFD_2_abs_path[fd] = str_new_path;

        //osFD_ref_count[fd] = 1;
    }
    else
    {
        //osFD_ref_count[fd]++;
    }
    // create new user-fd and register it in Osfd-ufd map
    UFD ufd = findAvailableUFD();
    user2os_FD_map[ufd] = fd;

    return ufd;
}

OsFD Cache::checkExist(const char* path)
{
    char c_new_path[PATH_MAX+1];
    realpath(path, c_new_path);
    string str_new_path(c_new_path);
    // check if file is open
    for(auto it = osFD_2_abs_path.begin(); it != osFD_ref_count.end(); it++)
    {
        if(str_new_path == it->second)
        {
            // the 2 files are the same!
            return it->first;
        }
    }
    return -1;
}
UFD Cache::findAvailableUFD()
{
    return last_fd++;
    // TODO this is not the right way, just temporary...
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

Block* Cache::findBlockByOsFD(OsFD fd, int blockNumInFile)
{
    for(auto it = blocks.begin(); it != blocks.end(); it++)
    {
        Block* block = *it;
        if ( (block->fd == fd) && (blockNumInFile == block->blockNumInFile) )
        {
            return block;
        }
    }
    return nullptr;
}

/**
 * finds the block number blockNumInFile int the cache. If it's not there- returns -1
 */
int Cache::findBlock(const char *path, int blockNumInFile) {
    for (int i=0; i<(int)blocks.size();++i ){
        if (strcmp(blocks[i]->realPath,path) && blockNumInFile==blocks[i]->blockNumInFile){
            return i;
        }
    }
    return -1;
}

// you realy write all this peace of code without execuing it once?!?!?!?!?!?
int Cache::readFile(UFD ufd, void *buf, size_t count, off_t offset) {

    if(user2os_FD_map.count(ufd)==0){//meaning the file is not open
        return -1;
    }

    OsFD file_id = user2os_FD_map[ufd];

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
            //TODO cache it

            blockNumInCache=findBlock(realPath,i);
        }
        block_ptr=blocks[blockNumInCache];
        bytesToCopy=std::min((int)count-alreadyCopied,block_ptr->length); //How much bytes to copy

        offsetInBlock=(i==firstBlock)? (int)offset%blockSize:0;

        //copy memory from the cache to the buffer
        memcpy((char*)buf+alreadyCopied,blocks[blockNumInCache]->content+offsetInBlock,bytesToCopy);
        alreadyCopied+=bytesToCopy;
    }

}



//endregion

//region Cache_LRU

Cache_LRU::Cache_LRU(int blocks_num):Cache(blocks_num)
{
};

Cache_LRU::~Cache_LRU() {
}

std::vector<Block*>::iterator Cache_LRU::blockNumToRemove()
{
    auto compareFunc = [](Block* a, Block* b) { return a->lastAccessTime > b->lastAccessTime; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    return it;
}
void Cache_LRU::updateAlgoDataAfterAccess(Block* accessed)
{
    accessed->lastAccessTime = std::time(nullptr);
}
void Cache_LRU::updateAlgoAfterReplacement(Block* newBlock)
{
    newBlock->lastAccessTime = std::time(nullptr);
}

//endregion

//region Cache_LFU

Cache_LFU::Cache_LFU(int blocks_num):Cache(blocks_num){
};

Cache_LFU::~Cache_LFU() {
}

std::vector<Block*>::iterator Cache_LFU::blockNumToRemove()
{
    auto compareFunc = [](Block* a, Block* b) { return a->refCount > b->refCount; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    return it;
}
void Cache_LFU::updateAlgoDataAfterAccess(Block* accessed)
{
    accessed->refCount++;
}
void Cache_LFU::updateAlgoAfterReplacement(Block* newBlock)
{
    newBlock->refCount = 0;
}



//endregion

//region Cache_FBR

Cache_FBR::Cache_FBR(int blocks_num, double f_old,double f_new):Cache(blocks_num){
    _f_new=f_new;
    _f_old=f_old;
};

Cache_FBR::~Cache_FBR() {
}
std::vector<Block*>::iterator Cache_FBR::blockNumToRemove()
{
}
void Cache_FBR::updateAlgoDataAfterAccess(Block* accessed)
{
}
void Cache_FBR::updateAlgoAfterReplacement(Block* newBlock)
{
}


//endregion