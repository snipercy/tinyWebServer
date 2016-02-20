#ifndef _MEM_POOL_H
#define _MEM_POOL_H
#include <cstddef>
#include <stdio.h>
#include "FileIndex.h"

class MemPool {
 public:
  MemPool();
  ~MemPool();
  int getPageCount();
  int getTotalAllocated();
  FileIndex* allocate(int);
};

extern MemPool mp;

#endif
