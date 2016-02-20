#include <stdio.h>
#include <string>
#include "URLParser.h"
#include "HTTPLib.h"
#include "csapp.h"

using namespace std;

int is_text(char buf[], rio_t* rio) {
    while (Rio_readlineb(rio, buf, MAXLINE) != 0) {
        // Check for end of HTTP header:
        if (buf[0] == '\r' && buf[1] == '\n')
            return false;
        // Check for HTTP Content-Type field:
        char* ct = strcasestr(buf, "Content-Type:");
        if (ct != NULL) {
            ct = strcasestr(buf, "text/");
            if (ct != NULL) {
                return true;
            }
        }
    }
    return false;
}

// This function fetches the http resource specified by the
// provided 'url' into the string 'doc'.  Returns -1 if it
// fails; 0 otherwise.
int http_fetch(url_t url, string& doc) {
    int   fd  = Open_clientfd((char*)url.host.c_str(), 80);

    if (fd < 0)
        return fd;
    
    char buf[MAXLINE];
    sprintf(buf, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n",
            url.file.c_str(), url.host.c_str());
    Rio_writen(fd, buf, strlen(buf));
    
    rio_t rio;
    Rio_readinitb(&rio, fd);

    if (is_text(buf, &rio)) {
        while (Rio_readlineb(&rio, buf, MAXLINE) != 0) {
            doc += buf;
        }
        return 0;
    }

    Close(fd);
    return -1;
}
