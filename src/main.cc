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

#include "version.hh"
#include "getopts.hh"
#include "server.hh"
#include "trace.hh"

void intro()
{
	inf << "Welcome to "
	    << "\x1b[33;1me\x1b[34;1mt\x1b[33;1mf\x1b[34;1mt\x1b[33;1mp"
	    << "\x1b[0m" <<  " v." << version
	    << "-g" << GIT_VERSION <<" server\n";
}

int main(int argc, char **argv)
{
	getopts get_opts(argc, argv);

	intro();

	if (opts::get().verbose) {
		verb::get().set_verbose();

		msg << "verbosity enabled ...";
	}

	server s;

	return s.run();
}
