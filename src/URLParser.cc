#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "URLParser.h"

using namespace std;

// A typedef to shorten things a bit:
typedef string::const_iterator iter;

// A function that returns true if the character c is not
// a URL character:
bool not_url_char(char c) {
    // characters, in addition to alphanumberics, that can appear in URL
    static const string url_ch = "~;/?:@=&$-_.+!*'(),";

    // see whether c can appear in a URL and return the negative:
    return !(isalnum(c) ||
             find(url_ch.begin(), url_ch.end(), c) != url_ch.end());
}

// Finds the end of a URL:
iter url_end(iter b, iter e) {
    return find_if(b, e, not_url_char);
}

// Finds the beginning of a URL:
iter url_beg(iter b, iter e) {
    // The protocol separator:
    static const string sep = "://";

    iter i = b;

    while ((i = search(i, e, sep.begin(), sep.end())) != e) {
        // Make sure the separator isn't at the beginning or end of the line:
        if (i != b && i + sep.size() != e) {
            
            // beg marks the beginning of the protocol name:
            iter beg = i;
            while (beg != b && isalpha(beg[-1]))
                --beg;

            // is there at least one appropriate character before and after
            // the separator?
            if (beg != i && !not_url_char(i[sep.size()])) {
                string proto = string(beg, i);
                if (proto == "http" || proto == "https")
                    return beg;
            }
        }

        // The separator we found wasn't part of a URL; advance i
        // past this separator:
        i += sep.size();
    }

    return e;
}

// Parses out host and file parts of the URL:
url_t parse_url(iter b, iter e) {
    iter p = b;
    iter f;

    // Scan past protocol part:
    while (p != e) {
        if (*p == '/') {
            if ((p+1) != e && *(p+1) == '/') {
                p = p+1;
                break;
            }
        }
        p++;
    }

    // Scan host part:
    iter h = p+1;
    while (h != e) {
        if (*h == '/')
            break;
        h++;
    }

    // Extract the host and file of the URL:
    string host = string(p+1, h);
    string file = string(h, e);
    url_t url;
    url.host = host;
    url.file = file;
    
    return url;
}

url_t parse_url_str(const string urlstr) {
    return parse_url(urlstr.begin(), urlstr.end());
}

vector<url_t> parse_urls(const string& doc) {
    // A vector to hold urls:
    vector<url_t> urlvec;

    // The beginning and end of the document:
    iter b = doc.begin(), e = doc.end();

    // Scan the entire document:
    while (b != e) {
        // Look for one or more letters followed by ://
        b = url_beg(b, e);

        // If we found it:
        if (b != e) {
            // get the rest of the URL:
            iter after = url_end(b, e);

            // remember the URL:
            string surl = string(b, after);
            url_t  url  = parse_url(surl.begin(), surl.end());

            // A dirty filter to eliminate potential bad URLs:
            if (url.host.size() > 5)            
                urlvec.push_back(url);

            // advance b and check for more URLs on this line:
            b = after;
        }
    }

    return urlvec;
}
