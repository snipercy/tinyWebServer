#include <iostream>
#include <vector>
#include "InvertedIndexGen.h"

using namespace std;

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
          cout << "> ";
          cin.getline(line, 80);
      }
  }
  cout << endl;
  
  return 0;
}
