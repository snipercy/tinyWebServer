#include <string>
#include <sstream>
#include "Util.h"

using namespace std;

// Converts an integer i into s C++ string.
string int_to_string(int i) {
	stringstream out;
	out << i;
	return out.str();
}
