#include<stdio.h>
#include<cstddef>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/mman.h>

using namespace std;

#define PAGE_SIZE 4096

class MemPool{
    public:
        MemPool();
        ~MemPool();
        int getPageCount();
        void* allocate(int);
};


// Forward declaration.
class Page;

// Points to the currently allocated page.
Page* pages;

//The total number of objects allocated.
int tot_alloc = 0;

//keep track of allocated pages;

class Page{
    private:
        // the start addr of pool.
        char* pool;
        
        //the current available memony location to allocate from.
        char* p;
        
        // The previous full page.
        Page* prev;
        int size;//size available for allocation.
        void alloc(){

            //相当于初始化这段内存，这个特殊文件提供是null字符流.
            int fd = open("/dev/zero",O_RDWR);

            //NULL-位置任意;MAP_PRIVATE-写时拷贝，更改不会改变原文件.
#ifdef linux
            pool = (char*)mmap(NULL, PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE,fd, 0);
            close(fd);
#endif
#ifdef __APPLE__
            pool = (char*)mmap(0, PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE, -1,0);//-1-匿名.共享内存
#endif
            if(pool == MAP_FAILED){
                printf("map failed: %s\n", strerror(errno));
                exit(1);
            }
            p == pool;
            size = PAGE_SIZE;
        }
        
    
public:
   Page(){prev = NULL; alloc();} 
   Page(Page* full) {prev = full; alloc();}

   //FileIndex is a class for example.
   FileIndex* allocate(int val){
        int sz = sizeof(FileIndex);
        FileIndex* fi = new(p) FileIndex(val);
        size -= sz;
        
        // If we do not have any room for next allocation.
        if(size < sz){
            pages = new Page(this);
        }
        else
            p = p + sz;
        tot_alloc++;
        return fi;
    }

//cheng
   void destroy(){
    free(); 
    tot_alloc = 0;
    }
 
        void free(){
            munmap(pool, PAGE_SIZE);
            if(prev != NULL){
                prev->destroy();
                delete prev;
            }
        }


   int getPageCount(){
        int tot = 0;
        for(Page* p = this/*cheng*/; p!= NULL; p = p->prev){
            tot++;
        }
        return tot;
    }
    
    
    int getTotalAllocated(){
        return tot_alloc;
    }
};


   MemPool::MemPool(){
        pages = new Page();
    } 
    
   MemPool::~MemPool(){
        pages->destroy();        
        delete pages;
    }
        
    
   MemPool::getTotalAllocated(){
        return pages->getTotalAllocated();
    }
   MemPool::getPageCount(){
        return pages->getPageCount();
    }

   MemPool::allocate(){
        return pages->allocate();
    }
