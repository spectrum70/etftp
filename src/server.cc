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

	pkt.opcode = tools::htons(oc_opt_ack);
	strcat(data + tlen, "timeout");
	tlen += 8;
	field = conv::itoa(cc->sd->timeout);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);
	strcat(data + tlen, "tsize");
	tlen += 6;
	field = conv::itoa(cc->sd->tsize);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);
	strcat(data + tlen, "blksize");
	tlen += 8;
	field = conv::itoa(cc->sd->blksize);
	strcat(data + tlen, field.c_str());
	tlen += (field.size() + 1);

	send(cc->ss, (char*)&pkt, tlen + 2, from_ip, from_port);
}

void server::display_progress(struct send_data *sd, int block)
{
	int tag = block * progress_total_tags / sd->p.blk_total;

	if (tag > sd->p.last_tag) {
		int segments = tag - sd->p.last_tag;

		if ((segments + sd->p.last_tag) > progress_total_tags)
			segments = progress_total_tags - sd->p.last_tag;

		while (segments--)
			printf("█");

		sd->p.last_tag = tag;

		if (tag == progress_total_tags) {
			printf("]   ");
		}
	}

	fflush(stdout);
}

void server::setup_progress(struct send_data *sd)
{
	sd->p.blk_total = sd->tsize / sd->blksize;
	if (sd->tsize / sd->blksize)
		sd->p.blk_total++;

	sd->p.last_tag = 0;

	printf("\x1b[33;1m[");
	printf("\x1b[%dC\x1b[33;1m]", progress_total_tags);
	printf("\x1b[%dD", progress_total_tags + 1);
}

void server::send_err(struct ctx_client *cc, int error)
{
	struct packet_out pkt;

	pkt.opcode = tools::htons(oc_err);
	pkt.block = tools::htons(error);

	strcpy(pkt.data, "File not found");

	send(cc->ss, (char*)&pkt, 4 + strlen(pkt.data) + 1, from_ip, from_port);
}

void server::send_block(struct ctx_client *cc, int block)
{
	struct packet_out pkt;

	pkt.opcode = tools::htons(oc_data);
	pkt.block = tools::htons(block + 1);

	int size = math::min(cc->sd->blksize, cc->sd->tsize);

	read(cc->sd->fd, pkt.data, size);
	send(cc->ss, (char*)&pkt, size + 4, from_ip, from_port);

	cc->sd->tsize -= size;

	display_progress(cc->sd, block + 1);
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

void server::do_op_read(struct packet *pkt)
{
	int len;
	string file, type, field;
	char *data = pkt->data;

	struct send_data *sd = new(struct send_data);
	if (!sd)
		err << "cannot create send_data struct.\n";

	memset(sd, 0, sizeof(struct send_data));

	len = strlen(data);
	if (!len)
		return;
	file = string(data);

	data += (len + 1);
	len = strlen(data);
	if (!len)
		goto exit;
	type = string(data);
	data += (len + 1);

	for (;;) {
		len = strlen(data);
		if (!len)
			break;
		field = data;
		data += (len + 1);
		if (field == "blksize") {
			sd->blksize = atoi(data);
		} else if (field == "tsize") {
			sd->tsize = atoi(data);
		} else if (field == "timeout") {
			sd->timeout = atoi(data);
		}
	}

exit:
	inf << "requested file : " << file << "\n";

	struct ctx_client *cc = create_new_channel(sd);

	sd->fd = open((opts::get().server_path + "/" + file).c_str(), O_RDONLY);
	if (sd->fd >= 0) {
		sd->tsize = fs::get_file_size(sd->fd);

		send_opt_ack(cc);
 		setup_progress(sd);
	} else {
		inf << "file not found, closing\n";

		struct ctx_client *cc = create_new_channel(sd);

		send_err(cc, err_file_not_found);
		close_channel(cc);
	}
}

void server::close_channel(struct ctx_client *cc)
{
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
			int opcode = tools::ntohs(pkt->opcode);

			switch (opcode) {
			case oc_read:
				do_op_read(pkt);
				last = false;
				break;
			case oc_ack: {
				int block = tools::ntohs
						(*(uint16_t *)pkt->data);

				struct ctx_client *cc = mc[fd];

				send_block(cc, block);

				if (!cc->sd->tsize) {
					if (last) {
						/* Last, removing */
						close_channel(cc);
						last = false;
						inf << "transfer completed\n";
					}
					/*
					 * One more packet should be sent
					 * at the end.
					 */
					last = true;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	return 0;
}

