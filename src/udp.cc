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

#include "udp.hh"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <cstdlib>
#include <cstring>

/*
 * private implementation (pimpl) stuff,
 * --- user should not and cannot access this classes ---
 */
class __udp_base {
public:
	__udp_base(int fd = 0)
	{
		if (!fd) {
			if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror((string("__udp_base::") +
					__func__).c_str());
				exit(1);
			}
		}
		else s = fd;
	}
	~__udp_base() {
		close(s);
	}

	int fd() { return s; }
	void set_fd(int fds) { s = fds; }
private:
	int s;
};

udp_client::udp_client(int use_fd) : dest_ip(""), rport(0) {
	b = new __udp_base(use_fd);
}
udp_client::udp_client(unsigned short remote_port, int use_fd) :
	dest_ip(""), rport(remote_port) {
	b = new __udp_base(use_fd);
}
udp_client::udp_client(const string &dest_ip,
		       unsigned short remote_port, int use_fd) :
	dest_ip(dest_ip), rport(remote_port) {
	b = new __udp_base(use_fd);
}

udp_common::udp_common()
{

}

udp_common::~udp_common()
{
}

int udp_common::send(int fd, char *data, int len,
		     const string &remote_ip, unsigned short remote_port)
{
	static struct sockaddr_in to;

	if (remote_ip == "" || !remote_port) return 0;

	memset((char*)&to, 0, sizeof(struct sockaddr_in));

	to.sin_family = AF_INET;
	to.sin_port = htons(remote_port);
	inet_aton(remote_ip.c_str(), &to.sin_addr);

	return (int)sendto(fd, data, len, 0, (struct sockaddr*)&to, sizeof(to));
}

udp_server::udp_server(int listen_port) : lport(listen_port)
{
	local_ip = "0.0.0.0";
	b = new __udp_base();
	start();
}

udp_server::udp_server(const string &listen_ip, int listen_port) :
	lport(listen_port)
{
	local_ip = listen_ip;
	b = new __udp_base();
	start();
}

void udp_server::start()
{
	sockaddr_in server;

	memset((char*)&server, 0, sizeof(struct sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_port = htons(lport);

	if (inet_aton(local_ip.c_str(), &server.sin_addr) == 0) {
		perror((string("udp_server::") + __func__ + ":inet_aton ")
			.c_str());
		exit(1);
	}
	if (bind(b->fd(), (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror((string("udp_server::") + __func__ + ":bind ")
			.c_str());
		exit(1);
	}
}

int udp_common::new_chan()
{
	sockaddr_in server;

	int nfd = socket(AF_INET, SOCK_DGRAM, 0);

	if (nfd < 0) {
		perror((string("udp_common::") + __func__).c_str());
		exit(1);
	}

	memset((char*)&server, 0, sizeof(struct sockaddr_in));

	server.sin_family = AF_INET;
	server.sin_port = htons(0);

	if (inet_aton(local_ip.c_str(), &server.sin_addr) == 0) {
		perror((string("udp_server::") + __func__ + ":inet_aton ")
			.c_str());
		exit(1);
	}
	if (bind(nfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror((string("udp_server::") + __func__ + ":bind ")
			.c_str());
		exit(1);
	}

	return nfd;
}

int udp_server::get_fd()
{
	return b->fd();
}

int udp_server::receive(int fd, char *data)
{
	struct sockaddr_in srcaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int rval = recvfrom(fd, data, max_udp_in_len, 0,
		 (struct sockaddr *)&srcaddr, &addrlen);

	from_ip = inet_ntoa(srcaddr.sin_addr);
	from_port = ntohs(srcaddr.sin_port);

	return rval;
}
