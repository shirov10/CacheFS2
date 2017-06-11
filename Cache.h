//
// Created by shir on 29/05/17.
//

#ifndef CACHE_H
#define CACHE_H


#include <map>
#include <list>
#include <vector>
#include <memory>

struct Block;

class MetaData
{
public:
    const Block* _block;
    const int _blockIndex;

    MetaData(Block* block, int blockIndex);
};

struct Block{
//    const char* realPath;
    std::shared_ptr<std::string> realPath;
    int blockNumInFile;
    int length;
    char * content;
    bool isEmpty;
    long long lastAccessTime;
    unsigned int refCount;
    std::shared_ptr<MetaData> metaData = nullptr;

    Block(int blockSize);


};


class Cache {


protected:
    int blockSize; //TODO add const maybe?
    int blocksNum;
    std::vector<Block*> blocks;
    std::map<int, std::shared_ptr<std::string>> fileIDs;
    //std::map<const char*,std::map<int,int> > filesInfo; // a map of pairs <path, map>, such that this is a map of pairs <block number in file, block number in cache>

    std::shared_ptr<std::string> getRealPath(int file_id);
    Block* cacheBlock(int fd, std::shared_ptr<std::string> path, int blockNumInFile);
    virtual int blockNumToUseAlogo() = 0;


    int findBlock(std::shared_ptr<std::string> path, int blockNumInFile);

    /*
     * thos methods intends to update the metadata about the block in the cach after any change in it.
     * implementation specific to algorithem
     */
    virtual void updateAfterAccess(int blockNum);
    virtual void updateAfterReplaceMent(int blockNum);
    virtual void updateAfterDelete(int blockNum);


public:
    Cache(int blocks_num);
    virtual ~Cache();
    void addFile(std::shared_ptr<std::string>, int id);
    void removeFile(int id);
    int readFile(int file_id, void *buf, size_t count, off_t offset);
    int blockNumToUse();
    virtual int printCache(const char *log_path) = 0;
    int printCacheWithComperator(bool(*f)(Block* a, Block* b), const char* path);


    int missCounter = 0;
    int hitsCounter = 0;

};


class Cache_LRU : public Cache{
private:

public:
    Cache_LRU(int blocks_num);
    ~Cache_LRU();
    int blockNumToUseAlogo() override ;

//    virtual void updateAfterAccess(int blockNum) override ;
//    virtual void updateAfterReplaceMent(int blockNem) override ;
//    virtual void updateAfterDelete(int blockNum) override ;

    virtual int printCache(const char *log_path) override ;

//    virtual void simpletest1(int iterations);


};


class Cache_LFU : public Cache{
private:

public:
    Cache_LFU(int blocks_num);
    ~Cache_LFU();
    int blockNumToUseAlogo() override ;

//    virtual void updateAfterAccess(int blockNum) override ;
//    virtual void updateAfterReplaceMent(int blockNem) override ;
//    virtual void updateAfterDelete(int blockNum) override ;

    virtual int printCache(const char *log_path) override ;

};

class FBR_MetaData : public MetaData
{
public:
    int refCount;
    FBR_MetaData(Block* block, int blockIndex);;
};

class Cache_FBR : public Cache{
private:

    int newPartitionSize;
    //int middlePartitionSize;
    int oldPartitionSize;

    std::list<std::shared_ptr<FBR_MetaData>> blocks_info;
    virtual void updateAfterAccess(int blockNum) override ;
    virtual void updateAfterReplaceMent(int blockNem) override ;
    virtual void updateAfterDelete(int blockNum) override ;

    virtual int printCache(const char *log_path) override ;

public:
    Cache_FBR(int blocks_num, double f_old,double f_new);
    ~Cache_FBR();
    int blockNumToUseAlogo() override ;


};


#endif //CACHE_H
