#ifndef _INVERTED_INDEX_GEN_H
#define _INVERTED_INDEX_GEN_H

#include <map>
#include <vector>
#include <set>
#include <string>
#include <iostream>
#include "FileIndex.h"

class InvertedIndexGen {
 public:
    InvertedIndexGen();
    ~InvertedIndexGen();
    int build(const std::string&);

    // Returns a vector of URLs given a word.
    std::vector<std::string> lookup(const std::string&);
    // Returns a set of URLs given a search string.
    // A search string is a list of words delimited by spaces.
    std::set<std::string> search(const std::string&);

    void printURLMap(std::ostream&);
    void printWordMap(std::ostream&);

    int size();
 private:
    // A map from word to a list of file index objects.
    std::map<std::string, FileIndex*> idx;

    // A map from file index to url.
    std::map<int, std::string>        urlmap;
    
    // utility routines
    int loadIndexFile(std::vector<std::string>&, const std::string&);
    int indexFiles(const std::vector<std::string>&);
    int readWords(const std::string&, std::vector<std::string>&);
    void insert(const std::string&, int);
};

#endif
