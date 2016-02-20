#ifndef _FILE_INDEX_H
#define _FILE_INDEX_H
#include <cstddef>

class FileIndex {
 public:
  FileIndex(int);
  FileIndex* getNext();
  int getIndex();
  void setNext(FileIndex*);
  void* operator new(size_t sz, void* v);

 private:
  int index;
  FileIndex* next;
};

#endif
