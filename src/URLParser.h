#ifndef _URL_PARSER_H
#define _URL_PARSER_H

#include <string>
#include <vector>

// A structure for URLs:
struct url_t {
    std::string host;
    std::string file;
};

// Parses a single URL string.
url_t parse_url_str(const std::string);

// Parses a string and returns all the urls
// found in that string.  Note that this is
// passed by reference.
std::vector<url_t> parse_urls(const std::string&);

#endif
