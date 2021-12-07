#ifndef getopts_hh
#define getopts_hh

#include <string>
#include <vector>

using std::string;
using std::vector;

struct opts {
	bool verbose;
	string dest_ip;
	vector<string> nonopts {};
};

struct getopts {
	getopts(int argc, char **argv);
	opts & get_options() { return options; }
	void defaults();
	void info();
	void usage();
private:
	opts options {};
};

#endif /* getopts_hh */
