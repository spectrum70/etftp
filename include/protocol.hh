#ifndef __protocol_hh
#define __protocol_hh

#include <cstdint>

enum opcode {
	oc_read = 1,
	oc_write,
	oc_data,
	oc_ack,
	oc_err,
	oc_opt_ack,
};

enum tftp_errors {
	err_file_not_found = 1,
};

struct packet {
	uint16_t opcode;
	char data[1024];
} __attribute__((packed));

struct packet_out {
	uint16_t opcode;
	uint16_t block;
	char data[1024 * 32];
} __attribute__((packed));

namespace tools {
uint16_t ntohs(uint16_t val);
uint16_t htons(uint16_t val);
}

struct protocol {

};

#endif /* __protocol_hh */



