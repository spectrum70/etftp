#ifndef getopts_hh
#define getopts_hh

#include <string>
#include <vector>

using std::string;
using std::vector;

struct opts {
	static opts &get() {
		static opts o;
		return o;
	}
	bool verbose;
	string server_path;
	vector<string> nonopts {};
};

struct getopts {
	getopts(int argc, char **argv);
private:
	void defaults();
	void info();
	void usage();
};

#endif /* getopts_hh */
