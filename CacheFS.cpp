//
// Created by shir on 29/05/17.
//

#include "CacheFS.h"
#include "Cache.h"
#include <iostream>

//for open
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

//for close
#include <unistd.h>
#include <fstream>



static Cache *_cache;



int CacheFS_init(int blocks_num, cache_algo_t cache_algo, double f_old , double f_new){

    if(blocks_num<=0){
        return -1;
    }
    if (cache_algo==FBR &&(f_new>1 || f_new <0 || f_old>1 || f_old<0 || f_new+f_old>1)) {
        return -1;
    }

    //TODO add some try-catch. The function should fail in case that system call or library function fails (e.g. new).

    if (cache_algo==LRU){
        _cache=new Cache_LRU(blocks_num);
    }
    else if(cache_algo==LFU){
        _cache=new Cache_LFU(blocks_num);
    }
    else if(cache_algo==FBR){
        _cache=new Cache_FBR(blocks_num,f_old,f_new);
    }

//    _cache->simpletest1(20);

    return 0;

}

int CacheFS_destroy(){

    //TODO add some try-catch. The function should fail in case that system call or library function fails .
    delete _cache;

    return 0;
}


int CacheFS_open(const char *pathname){
    int id=open(pathname,O_RDONLY | O_DIRECT | O_SYNC);
    if(id<0){      //TODO we should support only files under "/tmp"!
        return -1;
    }
    _cache->addFile(pathname,id);
    return id;
}

int CacheFS_close(int file_id){
    if(close(file_id)!=0){
        return -1;
    }
    _cache->removeFile(file_id);
    return 0;
}

int CacheFS_pread(int file_id, void *buf, size_t count, off_t offset){
    return _cache->readFile(file_id,buf,count,offset);
}


int CacheFS_print_stat (const char *log_path)
{
    std::string path(log_path);

    std::ofstream log_file(path, std::ios_base::out | std::ios_base::app );
    if(log_file.fail())
    {
        return -1;
    }

    log_file <<  "Hits number: " << _cache->hitsCounter << "." << std::endl;
    if(log_file.fail())
    {
        return -1;
    }
    log_file <<  "Misses number: " << _cache->missCounter << "." << std::endl;
    if(log_file.fail())
    {
        return -1;
    }

    return 0;
}