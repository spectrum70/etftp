#ifndef trace_hh
#define trace_hh

#include <string>
#include <iostream>
#include <iomanip>
#include <cstdarg>

using std::cout;
using std::string;

/* vt100 stuff */

static __attribute__((unused)) const char *msg = {"\x1b[0m"};
static __attribute__((unused)) const char *imp = {"\x1b[0m"};
static __attribute__((unused)) const char *dbg = {"\x1b[1;32m"};

static __attribute__((unused)) const char *err = {"\x1b[1;31m+++err: "};
static __attribute__((unused)) const char *rst = {"\x1b[0m"};

namespace vt100 {
static inline void cursor_off()
{
	printf("\33[?25l");
}
static inline void cursor_on()
{
	printf("\33[?25h");
}
};

struct trace {
	trace () {}
	static trace & get () {
		static trace t;
		return t;
	}

	void write(const char* format, ...)
	{
		va_list argp;
		va_start(argp, format);

		cout << msg;
		vprintf(format, argp);
		cout << std::flush << rst;
		va_end(argp);
	}

	void print(const char* format, ...)
	{
		va_list argp;
		va_start(argp, format);

		printf(dbg);
		vprintf(format, argp);
		printf(rst);

		va_end(argp);
	}

	trace & operator << (const string &s)
	{
		cout << s;

		if (s[s.size() - 1] == '\n')
			cout << rst;

		return *this;
	}
	trace & operator << (int i)
	{
		cout << i;

		return *this;
	}
	trace & operator << (long int i)
	{
		cout << i;

		return *this;
	}
	trace & operator << (double i)
	{
		cout << i;

		return *this;
	}
	trace & operator << (long double i)
	{
		cout << i;

		return *this;
	}

private:
	bool verbose;
};

struct verb {
	verb () {}
	static verb & get () {
		static verb t;
		return t;
	}

	void set_verbose() { verbose = true; }


	verb & operator << (const string &s)
	{
		if (verbose) {
			cout << s;

			if (s[s.size() - 1] == '\n')
				cout << rst;
		}

		return *this;
	}
	verb & operator << (int i)
	{
		if (verbose)
			cout << i;

		return *this;
	}
	verb & operator << (long int i)
	{
		if (verbose)
			cout << i;

		return *this;
	}
	verb & operator << (double i)
	{
		if (verbose)
			cout << i;

		return *this;
	}
	verb & operator << (long double i)
	{
		if (verbose)
			cout << i;

		return *this;
	}

private:
	bool verbose;
};


struct nulldev {
	nulldev () {}
	static nulldev & get () {
		static nulldev t;
		return t;
	}
	nulldev & operator << (const string &s)
	{ return *this; }
	nulldev & operator << (int i)
	{ return *this; }
	nulldev & operator << (long int i)
	{ return *this; }
	nulldev & operator << (double i)
	{ return *this; }
	nulldev & operator << (long double i)
	{ return *this; }
};

#define msg verb::get() << msg
#define err trace::get() << err
#define inf trace::get() << imp
#ifdef DEBUG
#define dbg verb::get() << dbg
#else
#define dbg nulldev::get() << dbg
#endif

#endif /* trace_hh */
