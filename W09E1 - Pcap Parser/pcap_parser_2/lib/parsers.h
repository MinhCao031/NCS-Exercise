#ifndef PARSER_H
#define PARSER_H

#include "dissection.h"

typedef struct {
  u_char const *data;
  uint data_len;
} parsed_payload;

typedef struct {

  // currently only used for IPv4
  struct in_addr src_ip;
  struct in_addr dst_ip;

  // protocol
  uint16_t protocol;

  union {
    struct tcphdr tcp;
    struct udphdr udp;
  };

  parsed_payload payload;

} parsed_packet;

parsed_packet pkt_parser(package packet, package segment, package payload);

void tcp_parser(parsed_packet *pkt, package segment, package payload);
void udp_parser(parsed_packet *pkt, package segment, package payload);

#endif
