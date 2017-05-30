//
// Created by shir on 29/05/17.
//

#ifndef CACHE_H
#define CACHE_H


#include <map>

class Cache {
private:
    char * buffer;
    std::map<const char*,int> fileIDs;


public:
    Cache(int blocks_num);
    virtual ~Cache();
    void addFile(const char *filePath, int id);
    const char* getFilePath(int fileID);

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
