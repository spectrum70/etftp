/*
 * etftp - embedded tftp
 *
 * Angelo Dureghello (C) 2021 <angelo@kernel-space.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along https://gibiru.com/with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */

#include "utils.hh"
#include "trace.hh"

#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace fs {

int get_file_size(char *path)
{
	std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);

	return in.tellg();
}

int get_file_size(int fd)
{
	int off;

	off = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	return off;
}

int read_proc_file(char *path, string &out)
{
	string buff;
	ifstream f(path);

	if (!f.is_open())
		return -1;

	for (;;) {
		if (!f.good())
			break;

		getline(f, buff);
		out += buff;
		out += "\n";
	}

	return 0;
}
}

namespace conv
{
int atoi(const string& val)
{
	stringstream ss;
	int rval;

	ss << val;
	ss >> rval;

	return rval;
}

string itoa(int val)
{
	stringstream ss;

	ss << val;

	return ss.str();
}
}

namespace math {
int min(int a, int b)
{
	if (a <= b)
		return a;

	return b;
}

}
