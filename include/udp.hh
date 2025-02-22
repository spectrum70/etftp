#ifndef __udp_hh
#define __udp_hh

#include <string>

using namespace std;

constexpr int max_udp_in_len = 1500;

class __udp_base;

class udp_common
{
public:
	udp_common();
	~udp_common();

	int send(int fd, char *data, int len,
		     const string &remote_ip, unsigned short remote_port);
	int new_chan();

protected:
	__udp_base *b;
	string local_ip;
};

class udp_client : public udp_common
{
public:
	udp_client(int use_fd);
	udp_client(unsigned short remote_port, int use_fd);
	udp_client(const string &dest_ip,
		   unsigned short remote_port, int use_fd);
private:
	string dest_ip;
	int rport;
};

class udp_server : public udp_common
{
public:
	udp_server(int listen_port);
	udp_server(const string &listen_ip, int listen_port);

	void start();
	int get_fd();
	int receive(int fd, char *data);
	void clear_from();

protected:
	string from_ip {};
	int from_port {};

private:
	int lport;
};

#endif /* __udp_hh */
