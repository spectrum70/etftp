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

#include <iostream>
#include <getopt.h>

#include "getopts.hh"
#include "server.hh"
#include "version.hh"

using namespace std;

void getopts::info()
{
	cout << appname << " v." << version << "-g" << GIT_VERSION << "\n";
}

void getopts::usage()
{
	info();
	cout << "(C) 2021, kernel-space.org\n"
	     << "Usage: " << appname << " [OPTION]\n"
	     << "Example: ./" << appname << " -v\n"
	     << "Options:\n"
	     << "  -h,  --help        this help\n"
	     << "  -p,  --path        server root path (def. /srv/tftp)\n"
	     << "  -V,  --version     program version\n"
	     << "  -v                 verbose\n"
	     << "\n";
}

void getopts::defaults()
{
	options.dest_ip = "localhost";
}

getopts::getopts(int argc, char **argv)
{
	int c;

	defaults();

	for (;;) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'V'},
			{"dest-ip", required_argument, 0, 'd'},
			{"", no_argument, 0, 'v'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "hvVd:",
				long_options, &option_index);

		if (c == -1) {
			if (optind < argc) {
				while (optind < argc)
					options.nonopts.
						push_back(argv[optind++]);
				break;
			}
			return;
		}

		switch (c) {
		case 'h':
			usage();
			exit(-1);
			break;
		case 'v':
			options.verbose = true;
			break;
		case 'V':
			info();
			exit(-1);
			break;
		case 'd':
			options.dest_ip = optarg;
			break;
		default:
			exit(-2);
		}
	}
}
