#ifndef utils_hh
#define utils_hh

#include <fstream>
#include <string>

using namespace std;

namespace fs {
int get_file_size(char *path);
int get_file_size(int fd);
int read_proc_file(char *path, string &out);
}

namespace conv {
int atoi(const string& val);
string itoa(int val);

}

namespace math {
int min(int a, int b);
}

#endif