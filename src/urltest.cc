#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "URLParser.h"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "usage: urltest <filename>" << endl;
        return 1;
    }

    ifstream ifs(argv[1]);

    string doc, line;
    while (getline(ifs, line)) {
        doc += line;
    }

    vector<url_t> urls = parse_urls(doc);

    cout << "found: " << urls.size() << endl;
    vector<url_t>::const_iterator it = urls.begin();
    while (it != urls.end()) {
        cout << "host: " << it->host << ", file: " << it->file << endl;
        it++;
    }
    
    return 0;
}
