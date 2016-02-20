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
#include <algorithm>
#include <functional>

using namespace std;

class FileIndex{
    private:
        int index;    
        FileIndex* next;

        /*forbid copy assignment or copy construct*/
        FileIndex& operator = (const FileIndex& rhs);
        FileIndex(const FileIndex& );
    public:
        FileIndex(int);        
        int getIndex();
        FileIndex* getNext();
        void setNext(FileIndex*);
        //void* operator new(size_t sz, void* v);
};
//end class
int FileIndex::getIndex(){
    return index;
}

FileIndex* FileIndex::getNext(){
    return next;
}

FileIndex::FileIndex(int val){
    index = val;
    next = NULL;
}

void FileIndex::setNext(FileIndex* n){
    next = n; 
}

//void* operator new(size_t sz, void* v){
//   return v;
// }

class InvertedIndexGen{
    private:
        // A map from word to a list of file index objects.
        map< string, FileIndex* > idx;
        // A map from file index to url.
        map< int, string > urlmap;

        
        int loadIndexFile(const string& argv_1, vector<string>&);
        int indexFiles(const vector<string>&);
        int readWords(const string& filename, vector<string>& word_vec);
        void insert(string& word_vec_i, int );

    public:
        ~InvertedIndexGen();
        int build(const string&);
        
        //returns a vec of URLs given a word.
        vector<string> lookup(const string&);
        
        // Returns a set of URLs given a search string.
        // A search string is a list of words delimited by spaces.
        set<string> search(const string& word);
        
        int size();
        void printURLMap(ostream&);
        void printWordMap(ostream&);
};
//end class 

/*Splits*/
vector<string> &split_delim(const string& s, char delim, vector<string>& elems){
    stringstream ss(s); 
    string item;
    while(getline(ss, item, delim)){
       item.erase(remove(item.begin(), item.end(), ' '), item.end()); 
        elems.push_back(item);
    }
    return elems;
}

vector<string> split_delim(const string &s, char delim){
    vector<string> elems;
    return split_delim(s, delim, elems );
}

InvertedIndexGen::~InvertedIndexGen(){
    idx.clear(); 
    urlmap.clear();
}

/*build*/
int InvertedIndexGen::build(const string& file){
    vector<string> files;
    if(loadIndexFile(file, files) == -1)
        return -1;
    if(indexFiles(files) == -1)
        return -1;
    return 0;
}

/**********************loadIndexfiles************************
 * 将文本文件中每一行存入vector中
 *
//input-1:idxfile: web/test_index.txt from argv[1]
//input-2:vec<str> s: to store result:
//        s[i] is a ith line in test_index.txt: 
//        s[i]="web/test_idxfiles/1.txt # http://hao123.com"
*************************************************************/
int InvertedIndexGen::loadIndexFile( const string& idxfile,vector<string>& files){
    ifstream infile(idxfile.c_str());
    if(infile){
        string line;
        int lineno = 1;
        while(getline(infile, line)){
            if("" == line)//cheng
                cerr << "[" << lineno 
                     << "] found blank line in input file. skipping." 
                     << endl;
            else
                files.push_back(line);
            lineno++;
        }//while
    }//fi
    else{
        cerr << "can't open file " << idxfile << endl;
        return -1;
    }
    return 0;
}

/**************indexFiles**************
//input:vec<string>files, it return from loadIndexFiles(...)
//files[0] = web/test_idxfiles/1.txt # http://hao123.com
//store words in one file into grobel var 
*************************************/
int InvertedIndexGen::indexFiles(const vector<string>& files){
    set<string> seen;// already indexed;
    vector<string> words;//
    int fcnt = 0;
    //cheng     
    for(vector<string>::const_iterator citer = files.begin(); citer != files.end(); ++citer){
        string file = *citer;
        vector<string> file_line = split_delim(file, '#');
        
        file = file_line[0];
        string url = file_line[1];

        
        // Check to see if we have indexed this file already.
        if(seen.find(file) == seen.end()){
            seen.insert(file);
            if(readWords(file, words) == -1)
               return -1; 
            //cheng
            
            for(vector<string>::iterator iter = words.begin(); 
                     iter != words.end(); ++iter){
                insert(*iter,fcnt); //insert in idx
            }
            //不能直接调用成员函数
    //        for_each(words.begin(), words.end(),mem_fun( &InvertedIndexGen::insert) );
            words.clear();
            urlmap[fcnt] = url;
            fcnt++;
            cout<<"fcnt: \n"<<fcnt<<endl;
        }
        else { 
            cerr << "duplicate input file: " << file 
                 << ". Skipping." << endl;
        }
    }
    return 0;
}

bool alpha(char c) {
	return isalpha(c);
}

bool not_alpha(char c) {
	return !isalpha(c);
}



/*split*/
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



/*readWord*/
int InvertedIndexGen::readWords(const string& file, vector<string>& words){
    ifstream infile(file.c_str());
    if(infile){
        string line;
        while(getline(infile, line)){
            vector<string> tmp = split(line);
            words.insert(words.end(), tmp.begin(), tmp.end());
        }
        return 0;
    }
    else {
        cerr << "con't open file '" << file << "'" << endl;
        return -1;
    }
}




/******insert*****cheng***/
void InvertedIndexGen::insert(string& word, int fcnt){
    map<string, FileIndex*>::iterator iter = idx.find(word);
    if(iter == idx.end()){ // Create the new file index object.
        FileIndex* fi = new FileIndex(fcnt);
        idx[word] = fi;
    }
    else{
        FileIndex* cur = iter->second;
        FileIndex* pre = cur;
        
        while(cur != NULL){
            if(cur->getIndex() == fcnt)
                return;
            else{
                pre = cur;
                cur = cur->getNext();
            }
        }
        // Create the new file index object.
        FileIndex* fi = new FileIndex(fcnt);
        
        // And install it.
        cur->setNext(fi);
    }
}


/*to_set()*/

/*printURLMap*/
void InvertedIndexGen::printURLMap(ostream& ios){
    for(map<int , string>::const_iterator i = urlmap.begin(); i != urlmap.end(); ++i){
        int idx = i->first;
        string url = i->second;
        ios << idx << " -> " << url << endl;
    }
}

void InvertedIndexGen::printWordMap(ostream& ios){
    for(map<string, FileIndex*>::iterator i = idx.begin(); i != idx.end(); ++i){
        string word = i->first;
        FileIndex* fidx = i->second;
        ios << word << ": ";
        while(fidx != NULL ){
            ios << fidx->getIndex();
            fidx = fidx->getNext();
            if (fidx != NULL) 
                ios << ", ";
            else 
                ios << ". ";
        }
    }
}

int InvertedIndexGen::size(){
   return urlmap.size();
}

/*lookup     cheng*/
//vectoc<string> InvertedIndexGen::lookup(const string& word){
//}

/*search*/
set<string> InvertedIndexGen::search(const string& words) {
        set<string>  urls;
        vector<string> search_words;
        split_delim(words, ' ',search_words);
        for(vector<string>::const_iterator w= search_words.begin();
            w != search_words.end(); ++w){
            
            map<string, FileIndex*>::const_iterator e = idx.find(*w);
            if(e != idx.end()){

                set<string> nurls;
                FileIndex* fidx = e->second;
                while(fidx != NULL){
                    int idx = fidx->getIndex();
                    string url = urlmap[idx];
                    nurls.insert(url);
                    fidx = fidx->getNext();
                }
                
                if(urls.empty()){
                    set<string>::const_iterator i = nurls.begin();
                    while(i != nurls.end()){
                        urls.insert(*i);
                        ++i;
                    }
                }

                else{
                    set<string> xx;
                    set<string>::iterator it = xx.begin();
                    set_intersection(urls.begin(), urls.end(),
                                    nurls.begin(), nurls.end(),
                                    inserter(xx, xx.begin()));
                    urls.clear();
                    urls.insert(xx.begin(), xx.end());
                }//else
            }//if
        }
        return urls;
}



int main(int argc, char* argv[]) {
  if (argc != 2) {
    cerr << "usage: idxr file" << endl;
    return 1;
  }

  InvertedIndexGen ivgen;
  ivgen.build(argv[1]);
  cout << "indexed " << ivgen.size() << " documents." << endl;
  
  char line[80];
  cout << "> ";
  cin.getline(line, 80);
  while (cin.good()) {
      string word(line);

      if (word == "\\all") {
          ivgen.printWordMap(cout);
      }
      else {
          set<string> urls = ivgen.search(word);
          for (set<string>::const_iterator it = urls.begin();
               it != urls.end();
               it++) {
              cout << *it << endl;
          }
          cout << urls.size() << " results." << endl;
          cout << " results." << endl;
          cout << "> ";
          cin.getline(line, 80);
      }
  }
  cout << endl;
  
  return 0;
}


