//
// Created by shir on 29/05/17.
//

#ifndef CACHE_H
#define CACHE_H


#include <map>
#include <list>
#include <vector>


struct Block{
    const char* realPath;
    int blockNumInFile;
    int length;
    char * content;
    bool isEmpty;
    long lastAccessTime;
    unsigned int refCount;

    Block(int blockSize);


};


class Cache {

protected:
    int blockSize; //TODO add const maybe?
    std::vector<Block*> blocks;
    std::map<int,const char*> fileIDs;
    //std::map<const char*,std::map<int,int> > filesInfo; // a map of pairs <path, map>, such that this is a map of pairs <block number in file, block number in cache>

    const char* getRealPath(int file_id);
    Block* cacheBlock(int fd, const char *path, int blockNumInFile);
    virtual int blockNumToUseAlogo() = 0;


    int findBlock(const char* path, int blockNumInFile);




public:
    Cache(int blocks_num);
    virtual ~Cache();
    void addFile(const char *filePath, int id);
    void removeFile(int id);
    int readFile(int file_id, void *buf, size_t count, off_t offset);
    int blockNumToUse();


};


class Cache_LRU : public Cache{
private:

public:
    Cache_LRU(int blocks_num);
    ~Cache_LRU();
    int blockNumToUseAlogo();

};


class Cache_LFU : public Cache{
private:

public:
    Cache_LFU(int blocks_num);
    ~Cache_LFU();
    int blockNumToUseAlogo();

};

class Cache_FBR : public Cache{
private:
    double _f_old;
    double _f_new;

public:
    Cache_FBR(int blocks_num, double f_old,double f_new);
    ~Cache_FBR();
    int blockNumToUseAlogo();


};


#endif //CACHE_H
