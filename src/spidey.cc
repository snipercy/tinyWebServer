#include "Spider.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cout << "usage: spidey <root url> <#workers> <depth>" << endl;
        return 1;
    }

    string root_url = argv[1];
    int    nworkers = atoi(argv[2]);
    int    depth    = atoi(argv[3]);
    crawl(root_url, nworkers, depth);
    
    return 0;
}
