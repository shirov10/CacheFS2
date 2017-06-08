//
// Created by omri on 05/06/17.
//

#include <assert.h>
#include <iostream>
#include <limits.h>
//#include <bits>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "Cache2.h"


std::map<User_file_descriptor, std::shared_ptr<CacheFile> > CacheFile::ufd2file_map = std::map<User_file_descriptor, std::shared_ptr<CacheFile> >();


Cache2::Cache2(size_t blockSize, int blockNum){
    CacheBlock::_block_size = blockSize;

    _block_size = blockSize;
    _blocks_num = blockNum;
    _number_of_blocks_in_use = 0;

    //_all_blocks_vector = std::vector<CacheBlock>(blockNum);
    _all_blocks_vector = new CacheBlock[blockNum];

    for (int i = 0; i < blockNum; i++)
    {
        _empty_blocks_list.push_back(i);
    }
}

Cache2::~Cache2()
{
    delete[] _all_blocks_vector;
}

CacheBlock *Cache2::giveMeEmptyBlockToUse() {
    assert(_blocks_num - _number_of_blocks_in_use == _empty_blocks_list.size());
    if (!_empty_blocks_list.empty())
    {
        std::cout << "next block index is " << _empty_blocks_list.front() << std::endl;
        CacheBlock* retVal = _all_blocks_vector+_empty_blocks_list.front();
        _empty_blocks_list.pop_front();
        _number_of_blocks_in_use++;
        return retVal;
    }
    else
    {
        std::cout << "problem!! please implement block free machanizem!!!" << std::endl;
    }
}

CacheBlock::CacheBlock() {
    _index_in_file = -1;
    _lengh = -1;
    _file = nullptr;
    _content = (char*)aligned_alloc( _block_size, _block_size );
}

CacheBlock::~CacheBlock() {
    delete(_content);
}

// ---------------------------------------------------------------------------------


User_file_descriptor CacheFile::createFile(const char *path) {

    std::shared_ptr<CacheFile> retVal;

    // find absolute path
    char c_new_path[PATH_MAX+1];
    realpath(path, c_new_path);
    std::string str_new_path(c_new_path);

    // check if file exists
    Os_file_descriptor os_fd = -1;
    for (auto it = ufd2file_map.begin(); it != ufd2file_map.end(); it++)
    {
        if (it->second->_path == str_new_path)
        {
            os_fd = it->second->_fd;
            retVal = it->second;
            assert(os_fd != -1);
        }
    }

    if (os_fd == -1)
    {
        // need to open file
        // file do not exist in system - create it!!!
        retVal = std::make_shared<CacheFile>(path);
    }

    // find next available user file descriptor
    User_file_descriptor user_fd = 0;
    for (auto it = ufd2file_map.begin(); it != ufd2file_map.end(); it++)
    {
        if (user_fd != it->first)
        {
            break;
        }
        else
        {
            user_fd++;
        }
    }

    ufd2file_map[user_fd] = retVal;

    return user_fd;
}

int CacheFile::readFromFile(User_file_descriptor ufd, char * buf, size_t count, off_t offset)
{
    std::shared_ptr<CacheFile> file = ufd2file_map[ufd];
    file->CachFileRead(buf, count, offset);
    return 0; // TODO
}

CacheFile::CacheFile(const char *path)
{
    _fd = open(path,O_RDONLY);// | O_DIRECT | O_SYNC); TODO
    if(_fd<0){      //TODO we should support only files under "/tmp"!
        // TODO be aware of possibility of errors here! what to do in sch a case???
    }
    // find absolute path
    char c_new_path[PATH_MAX+1];
    realpath(path, c_new_path);
    std::string str_new_path(c_new_path);
    _path = str_new_path;

    // find file lengh
    off_t fsize;
    fsize = lseek(_fd, 0, SEEK_END);
    _lengh_bytes = fsize;

    // fine file lengh - blocks
    if (_lengh_bytes % CacheBlock::_block_size == 0)
    {
        _lengh_blocks = _lengh_bytes / CacheBlock::_block_size;
    }
    else
    {
        _lengh_blocks = _lengh_bytes / CacheBlock::_block_size + 1;
    }

    // initelize blocks vector
    _blocks = std::vector<CacheBlock*>(_lengh_blocks);

}

CacheFile::~CacheFile() {

}


int CacheFile::CachFileRead(char *buf, size_t count, off_t offset) {
    if(offset >= this->_lengh_bytes)
    {
        return 0;
    }

    // if file is not long enogh
    count = std::min((unsigned int)count, (unsigned int)(_lengh_bytes-offset));

    //calculate the wanted blocks
    int firstBlock, lastBlock;
    firstBlock = (int)offset/CacheBlock::_block_size;
    lastBlock = ((int)offset+(int)count)/CacheBlock::_block_size;

    int readed_count = 0;
    int blockToRead = firstBlock;
    off_t offset_in_block = offset % CacheBlock::_block_size;
    while (readed_count < count)
    {
        int last_index_to_read_in_block = std::min((int)CacheBlock::_block_size, (int)count - readed_count);

        size_t tmp_count = last_index_to_read_in_block - offset_in_block;

        CachFileReadFromBlock(buf+readed_count, tmp_count, offset_in_block, blockToRead);

        offset_in_block = 0;
        blockToRead++;
        readed_count += tmp_count;
    }

    return 0;
}
/**
 * ask for block and read it, if needed.
 * return number of readed bytes.
 * asume
 */
int CacheFile::CachFileReadFromBlock(char * buf, size_t count, off_t offset, unsigned int blockNum)
{
    if (_blocks[blockNum] == nullptr)
    {
        // read block to memory

        // ask a free block to read into it
        _blocks[blockNum] = CacheBlock::giveMeEmptyBlockToUse();

        // find block offset in file
        off_t block_offset_in_file = blockNum * CacheBlock::_block_size;
        assert(block_offset_in_file < _lengh_bytes);

        // seek the right place in the file
        if(block_offset_in_file != lseek(_fd, block_offset_in_file, SEEK_SET))
        {
            assert(false);
            return -1;// TODO check this and find better ways
        }
        // read into the block
        off_t to_read = CacheBlock::_block_size;
        while (to_read > 0)
        {
            int readed = read(_fd, _blocks[blockNum]->_content, to_read);
            std::cout << readed << std::endl;
            to_read = to_read - readed;
        };
    }

    memcpy(buf, _blocks[blockNum]+offset, count);

    return 0; // todo
}


void CacheFile::deleteBlockFromFile(int index) {
    _blocks[index] = nullptr;
}

