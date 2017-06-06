//
// Created by shir on 29/05/17.
//

#ifndef CACHE_H
#define CACHE_H


#include <map>
#include <list>
#include <vector>
#include <stdlib.h>
#include <string>

// user file descriptor
typedef int UFD;
// os file descriptor
typedef int OsFD;
// counter...
typedef int Counter;

struct Block{
    // no reason to save real path when you can save FD
    const char* realPath;

    OsFD fd;
    bool isLastBlockInFile = false;

    int blockNumInFile;
    int length;
    char * content;

    long lastAccessTime;
    unsigned int refCount;

    Block(int blockSize);
};


class Cache {

protected:
    int blockSize; //TODO add const maybe?
    std::vector<Block*> blocks;

    std::map<int,const char*> fileIDs;
    // map user fd to native, os fd (may be many user fd'd pointing to the same native fd)
    std::map<UFD, OsFD> user2os_FD_map;
    //std::map<OsFD, Counter> osFD_ref_count;
    std::map<OsFD ,string> osFD_2_abs_path;

    UFD last_fd = 0;
    //std::map<const char*,std::map<int,int> > filesInfo; // a map of pairs <path, map>, such that this is a map of pairs <block number in file, block number in cache>

    const char* getRealPath(int file_id);
    void cacheBlock(const char* path, int blockNumInFile);
    virtual std::vector<Block*>::iterator blockNumToRemove() = 0;
    virtual void updateAlgoDataAfterAccess(Block* accessed) = 0;
    virtual void updateAlgoAfterReplacement(Block* newBlock) = 0;
    int findBlock(const char* path, int blockNumInFile);
    /**
     * search for the block by it's native file descriptor and the index in the file.
     *
     */
    Block* findBlockByOsFD(OsFD fd, int blockNumInFile);
    // if the file is opened, return the OS file descriptor for it.
    // else, return -1
    OsFD checkExist(const char* path);
    UFD findAvailableUFD();


public:
    UFD cach_open(const char* path);
    Cache(int blocks_num);
    virtual ~Cache();
    void addFile(const char *filePath, int id);
    void removeFile(int id);
    int readFile(UFD ufd, void *buf, size_t count, off_t offset);

};


class Cache_LRU : public Cache{
private:
    std::vector<Block*>::iterator blockNumToRemove() override;
    void updateAlgoDataAfterAccess(Block*) override ;
    void updateAlgoAfterReplacement(Block*) override ;
public:
    Cache_LRU(int blocks_num);
    ~Cache_LRU();
};


class Cache_LFU : public Cache{
private:
    std::vector<Block*>::iterator blockNumToRemove() override;
    void updateAlgoDataAfterAccess(Block*) override ;
    void updateAlgoAfterReplacement(Block*) override ;
public:
    Cache_LFU(int blocks_num);
    ~Cache_LFU();
};

class Cache_FBR : public Cache{
private:
    std::vector<Block*>::iterator blockNumToRemove() override;
    void updateAlgoDataAfterAccess(Block*) override ;
    void updateAlgoAfterReplacement(Block*) override ;
    double _f_old;
    double _f_new;

public:
    Cache_FBR(int blocks_num, double f_old,double f_new);
    ~Cache_FBR();

};


#endif //CACHE_H
