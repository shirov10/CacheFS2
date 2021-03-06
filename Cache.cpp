//
// Created by shir on 29/05/17.
//

#include "Cache.h"
#include <sys/stat.h>


#include <cstdlib> //for realpath

#include <string.h> //for memcpy
#include <algorithm> ///for min
#include <sys/time.h>
#include <assert.h>
#include <iostream>

#include <unistd.h> //for read

#include <fstream>
#include <stdlib.h>
#include <malloc.h>
#include <limits.h>

// region Block

Block::Block(int blockSize) {
    content=(char*)aligned_alloc(blockSize,blockSize);
    //content=new char[blockSize];
    isEmpty=true;
}

//endregion



//region Cache

Cache::Cache(int blocks_num) {
    blocksNum = blocks_num;
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
        //free((char*)blocks[i]->realPath); //frees the memory that was allocated with malloc
        free(blocks[i]->content);
        delete blocks[i];
    }
}

void Cache::addFile(std::shared_ptr<std::string> filePath, int id) {
    fileIDs[id]=filePath;
}

void Cache::removeFile(int id) {
    fileIDs.erase(id);

}

std::shared_ptr<std::string> Cache::getRealPath(int file_id) {
    char buffer[PATH_MAX];
    realpath(fileIDs[file_id]->c_str(), buffer);
    std::shared_ptr<std::string> shared_path = std::make_shared<std::string>(buffer);
    //free(buffer);
    return shared_path;
    // please notice that realpath is alocated using maloc, and must be freed!
}

/**
 * finds the block number blockNumInFile int the cache. If it's not there- returns -1
 */
int Cache::findBlock(std::shared_ptr<std::string> path, int blockNumInFile) {
    for (int i=0; i<(int)blocks.size();++i ){
        if (!blocks[i]->isEmpty && (*blocks[i]->realPath == *path) &&
                blockNumInFile==blocks[i]->blockNumInFile){
            return i;
        }
    }
    return -1;
}

Block* Cache::cacheBlock(int fd, std::shared_ptr<std::string> path, int blockNumInFile) {
    int b=blockNumToUse();
    Block * blockPtr=blocks[b];

    int read_bytes=(int)pread(fd,(void*)blockPtr->content,(size_t)blockSize,(off_t)(blockNumInFile*blockSize));

    if (read_bytes==-1){
        return nullptr;
    }

    blockPtr->isEmpty=false;
    blockPtr->realPath=path;
    blockPtr->length=read_bytes;
    blockPtr->blockNumInFile=blockNumInFile;


    updateAfterReplaceMent(b);
    updateAfterAccess(b);

    return blockPtr;
}


long filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return (long)in.tellg();
}


int Cache::readFile(int file_id, void *buf, size_t count, off_t offset) {

    if(fileIDs.count(file_id)==0){//meaning the file is not open
        return -1;
    }

    if((int)count==0||(int)offset<0){
        return 0;
    }

    //calculate the wanted blocks
    int firstBlock, lastBlock;
    firstBlock=(int)offset/blockSize;
    lastBlock=((int)offset+(int)count)/blockSize-1*(((int)offset+(int)count)%blockSize==0); //if it is a multiple of blocksize- minus 1

    //Get the real path of the given file
    std::shared_ptr<std::string> realPath = getRealPath(file_id);

    int alreadyCopied=0, blockNumInCache,bytesToCopy, offsetInBlock;
    Block* block_ptr;

    //search for the blocks in the cache
    for (int i = firstBlock; i <= lastBlock; ++i) {
        if((double)filesize(realPath.get()->c_str())/blockSize<i){
            break;
        }
        blockNumInCache=findBlock(realPath, i);
        if(blockNumInCache==-1){ //meaning the block is not in the cache

            block_ptr=cacheBlock(file_id,realPath,i);
            if(block_ptr== nullptr){
                return -1;
            }
            missCounter++;
        }
        else{ //meaning the block was already in the cache

            block_ptr=blocks[blockNumInCache];

            updateAfterAccess(blockNumInCache);

            hitsCounter++;
        }

        offsetInBlock=(i==firstBlock)? (int)offset%blockSize:0;

        bytesToCopy = std::max( std::min((int)count-alreadyCopied,block_ptr->length-offsetInBlock), 0); //How much bytes to copy


        //copy memory from the cache to the buffer
        memcpy((char*)buf+alreadyCopied,block_ptr->content+offsetInBlock,(size_t)bytesToCopy);
        alreadyCopied+=bytesToCopy;
    }
    return alreadyCopied;

}

int Cache::blockNumToUse()
{
    auto it = blocks.begin();
    for(; it != blocks.end(); it++)
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


int Cache::printCacheWithComperator(bool(*compareFunc)(Block* , Block* ), const char* log_path)
{
    std::vector<Block*> sorted_cpy_of_blocks_vector = blocks;
    std::sort(sorted_cpy_of_blocks_vector.begin(), sorted_cpy_of_blocks_vector.end(), compareFunc);

    std::string path(log_path);
    std::ofstream log_file(path, std::ios_base::out | std::ios_base::app );
    if(log_file.fail())
    {
        return -1;
    }

    for (auto it = sorted_cpy_of_blocks_vector.begin(); it != sorted_cpy_of_blocks_vector.end(); it++)
    {
        Block* block = *it;
        if (!block->isEmpty)
        {
            log_file << *block->realPath << " "<<block->blockNumInFile << std::endl;
        }
    }
    return 0;
}

void Cache::updateAfterAccess(int blockNum)
{
    blocks[blockNum]->refCount++;
    timeval tv;
    gettimeofday(&tv, NULL);
    blocks[blockNum]->lastAccessTime=(long long)(tv.tv_sec) * 1000000 + (long long)(tv.tv_usec); //in microsecs
}

void Cache::updateAfterReplaceMent(int blockNum)
{
    blocks[blockNum]->refCount = 0;
}
void Cache::updateAfterDelete(int) { }
//endregion


//region Cache_LRU

Cache_LRU::Cache_LRU(int blocks_num):Cache(blocks_num)
{
};

Cache_LRU::~Cache_LRU() {
}

int Cache_LRU::blockNumToUseAlogo()
{
    auto compareFunc = [](Block* a, Block* b) { return a->lastAccessTime < b->lastAccessTime; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    int index = (int)std::distance( this->blocks.begin(), it );
    return index;
}


//void Cache_LRU::updateAfterAccess(int blockNum)
//{
//    blocks[blockNum]->refCount++;
//}
//void Cache_LRU::updateAfterReplaceMent(int blockNum)
//{
//    blocks[blockNum]->refCount = 0;
//}
//void Cache_LRU::updateAfterDelete(int blockNum){}

int Cache_LRU::printCache(const char *log_path)
{
    auto compareFunc = [](Block* a, Block* b) { return a->lastAccessTime >= b->lastAccessTime; };
    return this->printCacheWithComperator(compareFunc, log_path);
}



//endregion

//region Cache_LFU

Cache_LFU::Cache_LFU(int blocks_num):Cache(blocks_num){
};

Cache_LFU::~Cache_LFU() {
}

int Cache_LFU::blockNumToUseAlogo()
{
    auto compareFunc = [](Block* a, Block* b) { return a->refCount < b->refCount; };
    auto it = std::min_element(this->blocks.begin(), this->blocks.end(), compareFunc);
    int index = (int)std::distance( this->blocks.begin(), it );
    return index;
}
// TODO implement
//void Cache_LFU::updateAfterAccess(int blockNum)
//{
////    blocks[blockNum]->refCount++;
//}
//void Cache_LFU::updateAfterReplaceMent(int blockNum)
//{
////    blocks[blockNum]->refCount = 0;
//}
//void Cache_LFU::updateAfterDelete(int blockNum){}

int Cache_LFU::printCache(const char *log_path)
{
    auto compareFunc = [](Block* a, Block* b) { return a->refCount >= b->refCount; };
    return this->printCacheWithComperator(compareFunc, log_path);
}
//endregion


//region Cache_FBR

Cache_FBR::Cache_FBR(int blocks_num, double f_old,double f_new):Cache(blocks_num)
{
    newPartitionSize =  (int)(f_new * blocks_num);
    oldPartitionSize =  (int)(f_old * blocks_num);

};

Cache_FBR::~Cache_FBR() {
}
int Cache_FBR::blockNumToUseAlogo()
{

    assert((int)blocks_info.size() == blocksNum);
    assert(blocks_info.size() == blocks.size());

    auto end_it = blocks_info.rbegin();
    auto old_partition_begin_iterator = blocks_info.rbegin();
    for (int i = 0; i < oldPartitionSize; i++)
    {
        end_it++;
    }

    auto compareFunc = [](std::shared_ptr<FBR_MetaData> a, std::shared_ptr<FBR_MetaData> b)
    {
        return a->refCount < b->refCount;
    };
    auto it = std::min_element(old_partition_begin_iterator, end_it, compareFunc);
    std::shared_ptr<FBR_MetaData> tmp = (*it);
    return tmp->_blockIndex;
}

void Cache_FBR::updateAfterAccess(int blockNum)
{
    Block* accessedBlock = blocks[blockNum];
    std::shared_ptr<FBR_MetaData> metaData = std::static_pointer_cast<FBR_MetaData> (accessedBlock->metaData);

    assert(metaData != nullptr);

    auto meta_data_iterator = std::find(blocks_info.begin(), blocks_info.end(), metaData);
    int placeInQueue = (int) std::distance(blocks_info.begin(), meta_data_iterator);

    if (placeInQueue >= newPartitionSize)
    {
        metaData->refCount++;
    }


    // ---------------------------
    blocks_info.remove(metaData);
    blocks_info.push_front(metaData);
}
void Cache_FBR::updateAfterReplaceMent(int blockNum)
{

    Block* accessedBlock = blocks[blockNum];
    std::shared_ptr<FBR_MetaData> metaData = std::static_pointer_cast<FBR_MetaData> (accessedBlock->metaData);



    if (metaData == nullptr)
    {
        metaData = std::make_shared<FBR_MetaData>(accessedBlock, blockNum);
        accessedBlock->metaData = metaData;
    }
    else
    {
        blocks_info.remove(metaData);
    }
    blocks_info.push_front(metaData);
}
void Cache_FBR::updateAfterDelete(int blockNum)
{
    Block* accessedBlock = blocks[blockNum];
    std::shared_ptr<FBR_MetaData> metaData = std::static_pointer_cast<FBR_MetaData> (accessedBlock->metaData);

    assert(metaData != nullptr);

    blocks_info.remove(metaData);
    accessedBlock->metaData = nullptr;
}

int Cache_FBR::printCache(const char *log_path)
{
    std::string path(log_path);
    std::ofstream log_file(path, std::ios_base::out | std::ios_base::app );
    if(log_file.fail())
    {
        return -1;
    }
//    std::cout << "number of blocks info " << blocks_info.size() << std::endl;
    for (auto it = blocks_info.begin(); it != blocks_info.end(); it++)
    {
        std::shared_ptr<FBR_MetaData> metadata = *it;
        const Block* block = metadata->_block;
        log_file << *block->realPath << " " << block->blockNumInFile << std::endl;
    }

    return 0;
}

MetaData::MetaData(Block *block, int blockIndex):_block(block), _blockIndex(blockIndex) { }
FBR_MetaData::FBR_MetaData(Block * block, int blockIndex):MetaData(block, blockIndex)  { }
//endregion

