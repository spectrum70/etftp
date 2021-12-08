#ifndef server_hh
#define server_hh

#include <map>
#include "udp.hh"

struct packet;

using std::map;

struct progress {
	int last_tag;
	int blk_total;
	int blk_tags;
};

struct send_data {
	int fd;
	int timeout;
	int tsize;
	int blksize;
	struct progress p;
};

struct ctx_client {
	int ss;
	struct send_data *sd;
};

struct server : public udp_server {
	server();
	server(int port);

	void do_op_read(struct packet *pkt);
	void send_opt_ack(struct ctx_client *cc);
	void send_block(struct ctx_client *cc, int block);
	void send_err(struct ctx_client *cc, int error);
	struct ctx_client * create_new_channel(struct send_data *sd);
	void close_channel(struct ctx_client *cc);
	int setup_fds(fd_set *rfds);
	int get_changed_fd(fd_set *rfds);
	void setup_progress(struct send_data *sd);
	void display_progress(struct send_data *sd, int block);

	int run();
private:
	bool sending;
	int total_blocks;
	string server_path;

	typedef map<int, struct ctx_client *> map_ctx;
	map_ctx mc;
};

#endif /* server_hh */
