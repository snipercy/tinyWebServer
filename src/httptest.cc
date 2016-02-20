#include <iostream>
#include "HTTPLib.h"
#include "URLParser.h"

using namespace std;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "usage: httptest <url>" << endl;
        return 1;
    }
    
    string urlstr = argv[1];
    url_t  url    = parse_url_str(urlstr);

    string doc;
    http_fetch(url, doc);

    cout << doc << endl;
    
    return 0;
}
