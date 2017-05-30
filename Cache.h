//
// Created by shir on 29/05/17.
//

#ifndef CACHE_H
#define CACHE_H


#include <map>
#include <list>


class Cache {

//protected:
//    struct cacheFileInfo{int blockNumInFile; int offsetInCache;};

private:
    int blockSize; //TODO add const maybe?
    char * buffer;
    std::map<int,const char*> fileIDs;
    std::map<const char*,std::map<int,int>> filesInfo; // a map of pairs <path, map>, such that this is a map of pairs <block number in file, block number in cache>

    const char* getRealPath(int file_id);




public:
    Cache(int blocks_num);
    virtual ~Cache();
    void addFile(const char *filePath, int id);
    void removeFile(int id);
    int readFile(int file_id, void *buf, size_t count, off_t offset);

};


class Cache_LRU : public Cache{
private:

public:
    Cache_LRU(int blocks_num);
    ~Cache_LRU();
};


class Cache_LFU : public Cache{
private:

public:
    Cache_LFU(int blocks_num);
    ~Cache_LFU();
};

class Cache_FBR : public Cache{
private:
    double _f_old;
    double _f_new;

public:
    Cache_FBR(int blocks_num, double f_old,double f_new);
    ~Cache_FBR();

};


#endif //CACHE_H
