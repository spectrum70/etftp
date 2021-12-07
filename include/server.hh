#ifndef server_hh
#define server_hh

#include <map>
#include "udp.hh"

struct packet;

using std::map;

struct send_data {
	int fd;
	int timeout;
	int tsize;
	int blksize;
};

struct ctx_client {
	int ss;
	struct send_data *sd;
};

struct server : public udp_server {
	server();
	server(int port);

	void do_op_read(struct packet *pkt, struct send_data *sd);
	void send_opt_ack(struct ctx_client *cc);
	void send_block(struct ctx_client *cc, int block);
	struct ctx_client * create_new_channel(struct send_data *sd);
	void close_channel(struct ctx_client *cc);
	int setup_fds(fd_set *rfds);
	int get_changed_fd(fd_set *rfds);

	int run();
private:
	bool sending;

	typedef map<int, struct ctx_client *> map_ctx;
	map_ctx mc;
};

#endif /* server_hh */
