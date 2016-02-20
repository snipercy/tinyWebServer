#include <map>
#include <set>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cstring>
#include "Util.h"
#include "FileIndex.h"
#include "InvertedIndexGen.h"
#include "MemPool.h"
#include "tiny.h"

using namespace std;

// Splits a string on delimiter.
vector<string> &split_delim(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        // Strip whitespace:
        item.erase (remove (item.begin(), item.end(), ' '), item.end());
        // Add to elems:
        elems.push_back(item);
    }
    return elems;
}

// Splits a string on delimiter.
vector<string> split_delim(const string &s, char delim) {
    vector<string> elems;
    return split_delim(s, delim, elems);
}

InvertedIndexGen::InvertedIndexGen() {
    // Default constructor.
}

InvertedIndexGen::~InvertedIndexGen() {
    idx.clear();
    urlmap.clear();
}

int InvertedIndexGen::build(const string& file) {
    vector<string> files;
    if (loadIndexFile(files, file) == -1)
        return -1;
    if (indexFiles(files) == -1)
        return -1;
    return 0;
}

vector<string> InvertedIndexGen::lookup(const string& word) {
    vector<string> urls;
    map<string, FileIndex*>::const_iterator e = idx.find(word);
    if (e != idx.end()) {
        FileIndex* fidx = e->second;
        while (fidx != NULL) {
            int    idx = fidx->getIndex();
            string url = urlmap[idx];
            urls.push_back(url);
            fidx = fidx->getNext();
        }
    }
    return urls;
}

set<string> InvertedIndexGen::search(const string& words) {
    set<string>    urls;
    vector<string> search_words;
    split_delim(words, ' ', search_words);
    for (vector<string>::const_iterator w = search_words.begin();
         w != search_words.end();
         w++) {
        map<string, FileIndex*>::const_iterator e = idx.find(*w);
        if (e != idx.end()) {
            set<string> nurls;
            FileIndex* fidx = e->second;
            while (fidx != NULL) {
                int    idx = fidx->getIndex();
                string url = urlmap[idx];
                nurls.insert(url);
                fidx = fidx->getNext();
            }

            if (urls.empty()) {
                set<string>::const_iterator i = nurls.begin();
                while (i != nurls.end()) {
                    urls.insert(*i);
                    i++;
                }
            }
            else {
                set<string> xx;
                set<string>::iterator it = xx.begin();
                set_intersection(urls.begin(), urls.end(),
                                 nurls.begin(), nurls.end(),
                                 inserter(xx,xx.begin()));
                urls.clear();
                urls.insert(xx.begin(), xx.end());
            }
        }
    }
    return urls;
}

// Utility routines.
int InvertedIndexGen::loadIndexFile(vector<string>& files, const string& idxfile) {
	ifstream infile(idxfile.c_str());
	if (infile) {
		string line;
		int lineno = 1;
		while (getline(infile, line)) {
			if (line == "")
				cerr << "[" << int_to_string(lineno)
				     << "] found blank line in input file. skipping." << endl;
			else
				files.push_back(line);
			lineno++;
		}
		return 0;
	}
	else {
		cerr << "can't open file " << idxfile << endl;
		return -1;
	}
}

int InvertedIndexGen::indexFiles(const vector<string>& files) {
    vector<string> words; // Words in a file.
    set<string>    seen;  // Files we have "seen" (already indexed).
    int            fcnt;  // The file we are indexing.

    fcnt = 0; // First file is 0.

    // Iterate over files in index file.
    for (vector<string>::const_iterator f = files.begin();
         f != files.end(); ++f) {	//files: a line of web/test_index.txt (web/test_idxfiles/1.txt # http://nytimes.com)
        string file = *f;			//file: str before '#' (web/test_idxfiles/1.txt)

        // Split the line based on index file structure:
        // file # url  
        vector<string> file_line = split_delim(file, '#');
        file = file_line[0];
        string url = file_line[1];	// str after '#' (http://nytimes.com)
        
        // Check to see if we have indexed this file already.
        set<string>::iterator s = seen.find(file);
        if (s == seen.end()) {
            seen.insert(file);
            if (readWords(file, words) == -1)	//open file(web/test_index.txt) and split each word store in words (vec<str>)
                return -1;
            // Index each of the words.
            for (vector<string>::iterator w = words.begin();
                 w != words.end(); ++w) {
                // Insert the word/file into our inverted index.
                insert(*w, fcnt);
            }
            words.clear();

            // Add a mapping from file index to url:
            urlmap[fcnt] = url;
            
            fcnt++;
        }
        else {
            cerr << "duplicate input file: " << file << ".  Skipping." << endl;
        }
    }

    return 0;
}

void InvertedIndexGen::insert(const string& word, int fcnt) {
    map<string, FileIndex*>::iterator e = idx.find(word);
    if (e == idx.end()) {
        // Create the new file index object.
        FileIndex* fi = mp.allocate(fcnt);
        idx[word] = fi;
    }
    else {
        FileIndex* p = e->second;  // The current file index.
        FileIndex* l = p;          // The last file index we saw.

        // Search list to see if we already have it.
        while (p != NULL) {
            if (p->getIndex() == fcnt) {
                return;
            }
            else {
                l = p;
                p = p->getNext();
            }
        }

        // Create the new file index object.
        FileIndex* fi = mp.allocate(fcnt);

        // And install it.
        l->setNext(fi);
    }
}

bool alpha(char c) {
	return isalpha(c);
}

bool not_alpha(char c) {
	return !isalpha(c);
}

vector<string> split(const string& str) {
	vector<string> ret;
	string::const_iterator i = str.begin();
	while (i != str.end()) {
		// Ignore unrecognized characters (non-alpha):
		i = find_if(i, str.end(), alpha);

		// Find the next alpha word:
		string::const_iterator j = find_if(i, str.end(), not_alpha);

		// Copy the characters in [i, j)
		if (i != str.end())
			ret.push_back(string(i, j));

		i = j;
	}
	return ret;
}

int InvertedIndexGen::readWords(const string& file, vector<string>& v) {
	ifstream infile(file.c_str());
	if (infile) {
		string line;
		while (getline(infile, line)) {
			vector<string> words = split(line);
			v.insert(v.end(), words.begin(), words.end());
		}
		return 0;
	}
	else {
    cerr << "can't open file '" << file << "'" << endl;
    return -1;
	}
}

void to_set(set<int>& s, FileIndex* fi) {
    FileIndex* p = fi;
    while (p != NULL) {
        s.insert(p->getIndex());
        p = p->getNext();
    }
}

void InvertedIndexGen::printURLMap(ostream& ios) {
    typedef map<int, string>::const_iterator iter;
    for (iter i = urlmap.begin(); i != urlmap.end(); i++) {
        int    idx = i->first;
        string url = i->second;
        ios << idx << " -> " << url << endl;
    }
}

void InvertedIndexGen::printWordMap(ostream& ios) {
    typedef map<string, FileIndex*>::const_iterator iter;
    for (iter i = idx.begin(); i != idx.end(); i++) {
        string     word = i->first;
        FileIndex* fidx = i->second;
        ios << word << ": ";
        while (fidx != NULL) {
            ios << fidx->getIndex();
            fidx = fidx->getNext();
            if (fidx != NULL)
                ios << ", ";
            else
                ios << endl;
        }
    }    
}

int InvertedIndexGen::size() {
    return urlmap.size();
}

// C Support Routines for Tiny Web Server
InvertedIndexGen idx;
void build_index(char* index_file) {
    cout << "Building index on " << index_file
         << "..." << endl;
    idx.build(index_file);
    cout << "Indexed " << idx.size() << " documents."
         << endl;
}

url_list* search_index(char* words) {
    string w(words);
    set<string> urls = idx.search(w);
    url_list* list = NULL;
    for (set<string>::const_iterator i = urls.begin();
         i != urls.end();
         i++) {
        url_list* x = new url_list;
        x->url  = new char [(*i).size()+1];
        strcpy(x->url, (*i).c_str());
        x->next = list;
        list = x;
    }
    return list;
}

void delete_list(url_list* list) {
    if (list == NULL)
        return;
    else {
        delete_list(list->next);
        delete list->url;
        delete list;
    }
}

int list_length(url_list* list) {
    int len = 0;
    while (list != NULL) {
        len++;
        list = list->next;
    }
    return len;
}
