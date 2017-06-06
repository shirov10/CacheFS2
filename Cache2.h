//
// Created by omri on 05/06/17.
//

#ifndef CACHEFS_CACHE2_H
#define CACHEFS_CACHE2_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>


typedef int Os_file_descriptor;
typedef int User_file_descriptor;

class CacheFile;

class CacheBlock
{
    // ------- statics - memory management --------------
    /**
 * number of blocks
 */
    static int _blocks_num;
    /**
     * block size
     */
public:
    static int _block_size;
private:
    /**
    * current number of full blocks
    */
    static int _number_of_blocks_in_use;
    /**
     * list of empty blocks indecies.
     */
    static std::list<int> _empty_blocks_list;
    /**
     * vector of all blocks - for memory management
     */
    //static std::vector<CacheBlock> _all_blocks_vector;
    static CacheBlock* _all_blocks_vector;

public:
    static int initelizeBlocks(int blockSize, int blockNum);
    static void destroyBlocks();
    static CacheBlock* giveMeEmptyBlockToUse();
    static void freeAllThosBlocks(std::list<int> blocksToSetFree);


    // ------------- not statics - block internal info -------------

//private:
    /**
     * the block number in the file, start from 0.
     * also the index in the blocks vector in the file.
     */
    int _index_in_file;
    /**
     * lengh - can be block-size or less, if this is the last block in the file
     */
    int _lengh;
    /**
     * the file associated with the block.
     */
    CacheFile * _file;
    /**
     * the content of the block
     */
    char * _content;

    // ------------ methods ------------

    CacheBlock();
    ~CacheBlock();

    // ------------ metadata for algorithems ---------
    /**
     * used by algorithems.
     */
    long lastAccessTime;
    unsigned int refCount;
};

class CacheFile
{
private:
    /**
     * the real file descriptor of the file
     */
    Os_file_descriptor _fd;
    /**
     * absolute (real) path to the file
     */
    std::string _path;

    /**
     * lengh in bytes
     */
    off_t _lengh_bytes;
    /**
     * lengh in blocks
     */
    int _lengh_blocks;
    /**
     * vector of pointers to all the blocks in the file.
     * if certain block is not cached, the pointer will be null.
     */
    std::vector<CacheBlock*> _blocks;

    // ------------- statics ----------------------
public:
    /**
    * map: user fd to file (shared ptrs)
    */
    static std::map<User_file_descriptor, std::shared_ptr<CacheFile> > ufd2file_map;


    /**
     * check if the file exists,
     *  if not - create it and return shared pointer to it
     *  if exists - return the shared pointer to it.
     */
    static User_file_descriptor createFile(const char* path);
    static int readFromFile(User_file_descriptor ufd, char * buf, size_t count, off_t offset);

    // ------------- methods ----------------------
    /**
 * destruct CacheFile object
 * the only method that use close()
 */
    ~CacheFile();

    /**
 * create new CacheFile object
 * the only method that use open()
 */
    CacheFile(const char* path);
private:
    /**
     * read from file to buffer.
     * read new blocks if needed.
     */
    int CachFileRead(char * buf, size_t count, off_t offset);
    int CachFileReadFromBlock(char * buf, size_t count, off_t offset, unsigned int blockNum);
    /**
     * to signal this file that block number 'index' was taken
     */
    void deleteBlockFromFile(int index);
};



class Cache2 {


};


#endif //CACHEFS_CACHE2_H
