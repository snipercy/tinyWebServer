#include "FileIndex.h"
#include <cstddef>
#include <string>

using namespace std;

FileIndex::FileIndex(int i) {
  index = i;
  next  = NULL;
}

FileIndex* FileIndex::getNext() {
  return next;
}

void FileIndex::setNext(FileIndex* n) {
  next = n;
}

int FileIndex::getIndex() {
  return index;
}

void* FileIndex::operator new (size_t sz, void* v) {
  return v;
}
