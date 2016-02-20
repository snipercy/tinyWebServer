#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "MemPool.h"
#include "FileIndex.h"

using namespace std;

#define PAGE_SIZE 4096

// Forward declaration.
class Page;

// Points to the currently allocated page.
Page* pages;

// The total number of objects allocated.
int tot_alloc = 0;

// Keeps track of allocated pages.
class Page {
public:
  Page() { 
    prev = NULL;
    alloc();
  }

  Page(Page* full) {
    prev = full;
    alloc();
  }

  FileIndex* allocate(int val) {
    int        sz = sizeof(FileIndex);
    FileIndex* fi = new (p) FileIndex(val);

    // Adjust availabe size in pool.
    size -= sz;

    // If we do not have any room for next allocation.
    if (size < sz) {
      // Allocate the next page.
      pages = new Page(this);
    }
    else {
      // Increment the allocation pointer.
      p = p + sz;
    }
    
    tot_alloc++;

    return fi;
  }

  void destroy() {
    free();
    tot_alloc = 0;
  }

  int getPageCount() {
    int tot = 0;
    for (Page* p = this; p != NULL; p = p->prev) {
      tot++;
    }
    return tot;
  }

  int getTotalAllocated() {
    return tot_alloc;
  }

private:
  // The start address of pool.
  char* pool;

  // The current available memory location to allocate from.
  // The "allocation pointer".
  char* p;

  // The previous full page.
  Page* prev;

  // The size available for allocation.
  int size;

  // Allocate virtual memory.
  void alloc() {
      // We conditionally compile code if we are on Linux or Mac OS X
#ifdef linux
    int fd = open("/dev/zero", O_RDWR);
    pool = (char*) mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
#endif
#ifdef __APPLE__
    pool = (char*) mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE,
                        MAP_ANON | MAP_PRIVATE, -1, 0);
#endif
    if (pool == MAP_FAILED) {
      printf("map failed: %s\n", strerror(errno));
      exit(1);
    }
    p = pool;
    size = PAGE_SIZE;
  }

  void free() {
    munmap(pool, PAGE_SIZE);
    if (prev != NULL) {
      prev->destroy();
      delete prev;
    }
  }
};


MemPool::MemPool() {
  pages = new Page();
}

MemPool::~MemPool() {
  pages->destroy();
  delete pages;
}

int MemPool::getPageCount() {
  return pages->getPageCount();
}

int MemPool::getTotalAllocated() {
  return pages->getTotalAllocated();
}

FileIndex* MemPool::allocate(int val) {
  return pages->allocate(val);
}

// Defines the memory pool object.
MemPool mp;
