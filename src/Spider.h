#ifndef __SPIDER_H
#define __SPIDER_H

#include <string>

// This function crawls starting at a root
// URL, with a given number of worker threads,
// to a given depth.
 //extern "C" void crawl(std::string&, int, int);
 void crawl(std::string&, int, int);
 extern "C" int crawl_c(char*, int, int);

#endif
