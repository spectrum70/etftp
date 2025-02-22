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

#include "server.hh"
#include "trace.hh"
#include "utils.hh"
#include "getopts.hh"
#include "protocol.hh"

#include <sys/select.h>
#include <sys/socket.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstring>

constexpr int udp_port = 69;
constexpr int progress_total_tags = 32;

server::server() : udp_server(udp_port)
{
}

server::server(int port) : udp_server(port)
{
}

void server::send_opt_ack(struct ctx_client *cc)
{
	struct packet pkt;
	char *data = pkt.data;
	int tlen = 0;
	string field;

	memset(&pkt, 0, sizeof(struct packet));

	pkt.opcode = tools::h2ns(oc_opt_ack);
	strcat(data + tlen, "timeout");
	tlen += 8;
	field = conv::itoa(cc->sd->timeout);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);
	strcat(data + tlen, "tsize");
	tlen += 6;
	field = conv::itoa(cc->sd->total_size);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);
	strcat(data + tlen, "blksize");
	tlen += 8;
	field = conv::itoa(cc->sd->blk_size);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);

	send(cc->ss, (char*)&pkt, tlen + 2, from_ip, from_port);
}

void server::display_progress(struct send_data *sd, int block)
{
	int tag = block * progress_total_tags / sd->p.blk_total;
	int percent;

	if (tag > sd->p.last_tag) {
		int segments = tag - sd->p.last_tag;

		if ((segments + sd->p.last_tag) > progress_total_tags)
			segments = progress_total_tags - sd->p.last_tag;

		while (segments--)
			printf("\x1b[31;1m▇");

		sd->p.last_tag = tag;

		if (tag == progress_total_tags) {
			printf("\x1b[33;1m]   \x1b[?25h");
			return;
		}
	}

	/* Print percentage */
	percent = block * 100 / sd->p.blk_total;
	printf("\x1b""7");
	printf("\x1b[%dC \x1b[0m%2d%%", (progress_total_tags - tag + 1), percent);
	printf("\x1b""8");

	fflush(stdout);
}

void server::setup_progress(struct send_data *sd)
{
	sd->p.blk_total = sd->total_size / sd->blk_size;
	if (sd->total_size / sd->blk_size)
		sd->p.blk_total++;

	sd->p.last_tag = 0;

	printf("\x1b[33;1m[");
	for (int i = 0; i < progress_total_tags; ++i)
		printf("-");
	printf("\x1b[33;1m]");
	printf("\x1b[%dD", progress_total_tags + 1);
	printf("\x1b[?25l");
}

void server::send_err(struct ctx_client *cc, int error)
{
	struct packet_out pkt;

	pkt.opcode = tools::h2ns(oc_err);
	pkt.block = tools::h2ns(error);

	strcpy(pkt.data, "File not found");

	send(cc->ss, (char*)&pkt, 4 + strlen(pkt.data) + 1, from_ip, from_port);
}

void server::send_block(struct ctx_client *cc, int block)
{
	struct packet_out pkt;

	pkt.opcode = tools::h2ns(oc_data);
	pkt.block = tools::h2ns(block + 1);

	int size = math::min(cc->sd->blk_size, cc->sd->total_size);

	/*
	 * Resend, so read back one block.
	 * TODO: evaluate if reading from memory can be faster, and possible.
	 */
	if (block == last_block)
		lseek(cc->sd->fd, -size, SEEK_CUR);

	read(cc->sd->fd, pkt.data, size);
	send(cc->ss, (char*)&pkt, size + 4, from_ip, from_port);

	/* Don't update progress on resend */
	if (block != last_block) {
		cc->sd->total_size -= size;
		display_progress(cc->sd, block + 1);
	}

	last_block = block;
}

struct ctx_client * server::create_new_channel(struct send_data *sd)
{
	struct ctx_client *cc = new(struct ctx_client);

	if (!cc)
		err << "cannot create new channel\n";

	memset(cc, 0, sizeof(struct ctx_client));

	cc->ss = new_chan();
	cc->sd = sd;
	mc[cc->ss] = cc;

	return cc;
}

bool server::fetch_option(char **str, string &data)
{
	int len = strlen(*str);

	if (!len)
		return false;

	data = string(*str);
	*str += (len + 1);

	return true;
}

/*
 * There are 2 tftp revisions,
 * Supposing eveyone uses rev 2. (RFC1350)
 * | Opcode | Filename | 0 | Mode | 0 |
 * Plus eventually extension to know file size (RFC 2347)
 * |  opc   | filename | 0 | mode | 0 |  opt1  | 0 | value1 | 0 | ...
 */
int server::do_op_read(struct packet *pkt)
{
	string file, mode, field;
	char *data = pkt->data;

	struct send_data *sd = new(struct send_data);
	if (!sd) {
		err << "cannot create send_data struct.\n";
		return -1;
	}

	memset(sd, 0, sizeof(struct send_data));

	if (!fetch_option(&data, file))
		return -1;
	if (!fetch_option(&data, mode))
		return -1;

	if (mode != "octet") {
		inf << "not octet mode, exiting.\n";
		return -1;
	}

	for (;;) {
		if (!fetch_option(&data, field))
			break;
		if (field == "blksize") {
			sd->blk_size = atoi(data);
		} else if (field == "tsize") {
			sd->total_size = atoi(data);
		} else if (field == "timeout") {
			sd->timeout = atoi(data);
		}
	}

	struct ctx_client *cc = create_new_channel(sd);

	sd->fd = open((opts::get().server_path + "/" + file).c_str(), O_RDONLY);
	if (sd->fd >= 0) {
		sd->total_size = fs::get_file_size(sd->fd);
		send_opt_ack(cc);

	} else {
		err << "file <" << file << "> not found, closing\n";

		struct ctx_client *cc = create_new_channel(sd);

		send_err(cc, err_file_not_found);
		close_channel(cc);

		return -1;
	}

	inf << "requested file: " << file
	    << ", size: " << sd->total_size << " bytes\n";

	inf << "from ip: " << from_ip << "\n";

	if (sd->blk_size) {
		inf << "requested block size: " << sd->blk_size << "\n";
	} else {
		inf << "blocksize not specified, defaulting to 1468\n";
		sd->blk_size = 1468;
	}

	setup_progress(sd);

	return 0;

}

void server::close_channel(struct ctx_client *cc)
{
	clear_from();

	mc.erase(cc->ss);

	shutdown(cc->ss, SHUT_WR);
	close(cc->sd->fd);

	delete(cc->sd);
	delete(cc);
}

inline int server::setup_fds(fd_set *rfds)
{
	map_ctx::iterator i;
	int max = -1;

	FD_ZERO(rfds);
	for (i = mc.begin(); i != mc.end(); ++i) {
		if (i->first > max)
			max = i->first;
		FD_SET(i->first, rfds);
	}

	return max;
}

inline int server::get_changed_fd(fd_set *rfds)
{
	map_ctx::iterator i;

	for (i = mc.begin(); i != mc.end(); ++i) {
		if (FD_ISSET(i->first, rfds))
			return i->first;
	}

	return -1;
}

int server::run()
{
	int rval, fd, fdmax;
	fd_set rfds;
	struct timeval tv;
	char buff[max_udp_in_len];
	bool last = false;

	if (!fs::dir_exist(opts::get().server_path)) {
		err << "can't access server path\n";
		exit(1);
	}

	inf << "server path : " << opts::get().server_path << "\n";
	inf << "listening for requests ...\n";
	fd = get_fd();
	mc[fd] = 0;

	for (;;) {
		fdmax = setup_fds(&rfds);

		tv.tv_sec = 0;
		tv.tv_usec = 10000;

		rval = select(fdmax + 1, &rfds, NULL, NULL, &tv);
		if (rval <= 0)
			continue;

		fd = get_changed_fd(&rfds);
		if (fd == -1)
			continue;

		if (receive(fd, buff)) {
			struct packet *pkt = (struct packet *)buff;
			int opcode = tools::n2hs(pkt->opcode);

			switch (opcode) {
			case oc_read:
				if (do_op_read(pkt) != 0)
					return -1;
				last = false;
				break;
			case oc_ack: {
				int block = tools::n2hs
						(*(uint16_t *)pkt->data);

				struct ctx_client *cc = mc[fd];

				if (last) {
					/* Last, removing */
					close_channel(cc);
					last = false;
					inf << "transfer completed\n";
					break;
				}

				send_block(cc, block);
				last_block = block;

				if (!cc->sd->total_size) {
					/* Exit on ack at the end. */
					last = true;
				}
				break;
			}
			case oc_write:
				printf("oc_write!\n");
				break;
			case oc_data:
				printf("data!\n");
				break;
			case oc_err:
				printf("err +++\n");
				break;
			default:
				printf("oc not recognized %d\n", opcode);
				break;
			}
		}
	}

	return 0;
}

