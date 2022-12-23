#include "handler.h"
#include <assert.h>
#include <string.h>

int inserted_packets = 0;
const int DEBUGGING = 1;
Node* head_wait_list;

uint64_t get_flow_key(uint64_t ip1, uint64_t ip2, uint64_t p1, uint64_t p2) {
  uint64_t half64 = 2147483648;
  uint64_t ans = ip1 + ip2 + (p1*p1 - p1*p2 + p2*p2) * half64;
  return ans;
}

// classify and insert a new packet into hash table
void insert_packet(HashTable table, parsed_packet pkt, FILE* fptr) {

  if (DEBUGGING) fprintf(fptr, "Try inserting packet...\n");

  uint64_t flow_key;

  if (pkt.protocol == IPPROTO_TCP) {
    flow_key = get_flow_key(pkt.src_ip.s_addr, pkt.dst_ip.s_addr, pkt.tcp.source, pkt.tcp.dest);
    if (DEBUGGING) fprintf(fptr, "IP Source: %u\n", pkt.src_ip.s_addr);/**/
    if (DEBUGGING) fprintf(fptr, "IP Destination: %u\n", pkt.dst_ip.s_addr);/**/
    if (DEBUGGING) fprintf(fptr, "Port Source: %u\n", pkt.tcp.source);/**/
    if (DEBUGGING) fprintf(fptr, "Port Destination: %u\n", pkt.tcp.dest);/**/
    if (DEBUGGING) fprintf(fptr, "TCP seq: %u\n", pkt.tcp.seq);/**/
    if (DEBUGGING) fprintf(fptr, "Try inserting TCP...\n");/**/
    insert_tcp_pkt(table, flow_key, pkt, fptr);
    if (DEBUGGING) fprintf(fptr, "Done inserting TCP\n");/**/
  } else if (pkt.protocol == IPPROTO_UDP) {
    flow_key = get_flow_key(pkt.src_ip.s_addr, pkt.dst_ip.s_addr, pkt.udp.source, pkt.udp.dest);
    if (DEBUGGING) fprintf(fptr, "IP Source: %u\n", pkt.src_ip.s_addr);/**/
    if (DEBUGGING) fprintf(fptr, "IP Destination: %u\n", pkt.dst_ip.s_addr);/**/
    if (DEBUGGING) fprintf(fptr, "Port Source: %u\n", pkt.udp.source);/**/
    if (DEBUGGING) fprintf(fptr, "Port Destination: %u\n", pkt.udp.dest);/**/
    if (DEBUGGING) fprintf(fptr, "UDP seq: %u\n", pkt.tcp.seq);/**/
    if (DEBUGGING) fprintf(fptr, "Try inserting UDP...\n");/**/
    insert_udp_pkt(table, flow_key, pkt, fptr);
    if (DEBUGGING) fprintf(fptr, "Done inserting UDP\n");/**/
  } else {
    if (DEBUGGING) fprintf(fptr, "Try inserting packet: Nothing done (not TCP or UDP)\n");/**/
  }
}

// insert tcp packet to flow
void insert_tcp_pkt(HashTable table, uint64_t flow_key, parsed_packet pkt, FILE* fptr) {
  if (DEBUGGING) fprintf(fptr, "Finding flowkey = %ld...\n", flow_key);/**/
  flow_base_t *flow = search_flow(table, flow_key, fptr);
  if (DEBUGGING) fprintf(fptr, "Found flowkey\n");/**/
  if (DEBUGGING) fprintf(fptr, "Flag = %x\n", pkt.tcp.th_flags);/**/ 

  if (flow == NULL) {
    if (DEBUGGING) fprintf(fptr, "TCP flow not found, creating new one if it is SYN...\n");/**/

    if (pkt.tcp.th_flags == TH_SYN) { // create new flow if it is SYN
      flow_base_t new_flow = create_flow(pkt, fptr);
      new_flow.exp_seq_up = pkt.tcp.seq + 1;
      insert_new_flow(table, create_flow_node(flow_key, new_flow, fptr));
      if (DEBUGGING) fprintf(fptr, "New TCP flow-up created\n");/**/
    } else {
      if (DEBUGGING) fprintf(fptr, "Packet is not SYN, ignoring...\n");/**/
    }
  } else if (pkt.tcp.th_flags == 0x12) { // Insert to flow-down when it's a SYN/ACK
    if (DEBUGGING) fprintf(fptr, "SYN/ACK detected\n");
    Node *new_pkt_node = create_payload_node(pkt, fptr);
    flow->exp_seq_down = pkt.tcp.seq + 1;
    insert_to_flow(new_pkt_node, DESC, &(flow->flow_down), fptr);
  } else if (pkt.tcp.th_flags == 0x18) {
    Node *new_pkt_node = create_payload_node(pkt, fptr);
    if (DEBUGGING) fprintf(fptr, "Flow found, inserting TCP to flow...\n");/**/
    if (get_flow_direction(flow, pkt, fptr) == &(flow->flow_up)) {
      if (DEBUGGING) fprintf(fptr, "UP...\n");
      if (DEBUGGING) fprintf(fptr, "Checking up seq = %u...\n", flow->exp_seq_up);
      if (DEBUGGING) fprintf(fptr, "Checking real seq = %u...\n", pkt.tcp.seq);
      if (flow->exp_seq_up == pkt.tcp.seq) {
        if (DEBUGGING) fprintf(fptr, "CHECK: EQUAL\n");
        flow->exp_seq_up += ((parsed_payload *)new_pkt_node->value)->data_len;
        insert_to_flow(new_pkt_node, DESC, &(flow->flow_up), fptr);
        if (DEBUGGING) fprintf(fptr, "Flow found, done inserting TCP\n");/**/
        if (DEBUGGING) fprintf(fptr, "Expected Seq UP/DOWN = %u, %u\n", flow->exp_seq_up, flow->exp_seq_down);/**/
        inserted_packets += 1;

      } else {
        if (DEBUGGING) fprintf(fptr, "CHECK: NOT EQUAL\n");
        insert_node_asc(&head_wait_list, new_pkt_node);
      }
    } else if (get_flow_direction(flow, pkt, fptr) == &(flow->flow_down)) {
      if (DEBUGGING) fprintf(fptr, "DOWN...\n");
      if (DEBUGGING) fprintf(fptr, "Checking down seq = %u...\n", flow->exp_seq_down);
      if (DEBUGGING) fprintf(fptr, "Checking real seq = %u...\n", pkt.tcp.seq);
      if (1 || flow->exp_seq_down == pkt.tcp.seq) {
        if (DEBUGGING) fprintf(fptr, "CHECK: EQUAL\n");
        do {
          flow->exp_seq_down += ((parsed_payload *)new_pkt_node->value)->data_len;
          insert_to_flow(new_pkt_node, DESC, &(flow->flow_down), fptr);
          if (DEBUGGING) fprintf(fptr, "Inserted 1 TCP packet\n");/**/
          delete_node(&head_wait_list, flow->exp_seq_down, fptr);
          // flow->exp_seq_down += ((parsed_payload *)new_pkt_node->value)-> data_len;
          new_pkt_node = search_node(head_wait_list, flow->exp_seq_down);
        } while (new_pkt_node != NULL);        
        insert_to_flow(new_pkt_node, DESC, &(flow->flow_down), fptr);
        if (DEBUGGING) fprintf(fptr, "Flow found, done inserting TCP\n");/**/
        if (DEBUGGING) fprintf(fptr, "Expected Seq UP/DOWN = %u, %u\n", flow->exp_seq_up, flow->exp_seq_down);/**/
        inserted_packets += 1;


      } else {
        if (DEBUGGING) fprintf(fptr, "CHECK: NOT EQUAL\n");
        insert_node_asc(&head_wait_list, new_pkt_node);
      }        
    } else {
      if (DEBUGGING) fprintf(fptr, "UNEXPECTED!!!");
      insert_node_asc(&head_wait_list, new_pkt_node);
    }
  } else {
    Node *new_pkt_node = create_payload_node(pkt, fptr);
    if (DEBUGGING) fprintf(fptr, "Nothing done, maybe this is unnecessary processing packets\n");/**/
    insert_node_asc(&head_wait_list, new_pkt_node);
  }
}

// insert udp packet to flow
void insert_udp_pkt(HashTable table, uint64_t flow_key, parsed_packet pkt, FILE* fptr) {
  if (DEBUGGING) fprintf(fptr, "Finding flowkey = %ld...\n", flow_key);/**/
  flow_base_t *flow = search_flow(table, flow_key, fptr);
  if (DEBUGGING) fprintf(fptr, "Found flowkey\n");/**/

  Node *new_pkt_node = create_payload_node(pkt, fptr);

  if (flow == NULL) {
    if (DEBUGGING) fprintf(fptr, "UDP flow not found, creating new one\n");/**/
    flow_base_t new_flow = create_flow(pkt, fptr);

    if (DEBUGGING) fprintf(fptr, "Try inserting UDP to flow...\n");/**/
    insert_to_flow(new_pkt_node, FIRST, get_flow_direction(&new_flow, pkt, fptr), fptr);

    if (DEBUGGING) fprintf(fptr, "Try inserting UDP flow to table...\n");/**/
    insert_new_flow(table, create_flow_node(flow_key, new_flow, fptr));

    if (DEBUGGING) fprintf(fptr, "New UDP flow created, done inserting UDP\n");/**/
    inserted_packets += 1;
  } else {
    if (DEBUGGING) fprintf(fptr, "Flow found, inserting UDP to flow...\n");/**/
    insert_to_flow(new_pkt_node, FIRST, get_flow_direction(flow, pkt, fptr), fptr);

    if (DEBUGGING) fprintf(fptr, "Flow found, done inserting UDP\n");/**/
    inserted_packets += 1;
  }
}

// print the hash table
void print_hashtable(HashTable const table, FILE* fptr) {

  if (DEBUGGING) fprintf(fptr, "**********HASH TABLE**********\n");
  for (uint i = 0; i < table.size; i++) {
    Node *head = table.lists[i];
    if (DEBUGGING) fprintf(fptr, "Id [%d]: \n", i);
    print_flows(head, fptr);
    if (DEBUGGING) fprintf(fptr, "\n");
  }
}

void print_flows(Node const *const head, FILE* fptr) {

  const Node *scaner = head;

  while (scaner != NULL) {
    if (DEBUGGING) fprintf(fptr, "Key: %lu:\n", scaner->key);
    print_flow(*(flow_base_t *)scaner->value, fptr);
    scaner = scaner->next;
  }
}

// print flow info like src ip, dst ip, src port, dst port, protocol and payload
void print_flow(flow_base_t flow, FILE* fptr) {
  // print ip addresses
  if (DEBUGGING) fprintf(fptr, "\t|ip: %s", inet_ntoa(flow.sip));
  if (DEBUGGING) fprintf(fptr, " <=> %s, ", inet_ntoa(flow.dip));

  // print port
  if (DEBUGGING) fprintf(fptr, "port: %d", flow.sp);
  if (DEBUGGING) fprintf(fptr, " <=> %d\n", flow.dp);

  if (flow.ip_proto == IPPROTO_TCP) {
    if (DEBUGGING) fprintf(fptr, "\t|Protocol: TCP\n");

    // print expected sequence number
    if (DEBUGGING) fprintf(fptr, "\t|exp seq DOWN: %u, ", flow.exp_seq_down);
    if (DEBUGGING) fprintf(fptr, "exp seq UP: %u\n", flow.exp_seq_up);
  } else {
    if (DEBUGGING) fprintf(fptr, "\t|Protocol: UDP\n");
  }

  // print list of packets in the flow
  print_flow_direction(flow.flow_up, true, fptr);
  print_flow_direction(flow.flow_down, false, fptr);
}

// print payload in a flow direction
void print_flow_direction(Node const *head, bool is_up, FILE* fptr) {

  Node const *temp = head;
  char const *direction = is_up ? "UP" : "DOWN";

  while (temp != NULL) {

    if (DEBUGGING) fprintf(fptr, "\t\t[%s] ", direction);
    if (DEBUGGING) fprintf(fptr, "Seq: %ld, data size: %d\n", temp->key,
           ((parsed_payload *)temp->value)->data_len);

	////print_payload(((parsed_payload *)temp->value)->data, ((parsed_payload *)temp->value)->data_len);
	////fprintf(fptr, "\t\t-----------------------------------------------------------------------\n");

    temp = temp->next;
  }
}


// create new packet node
Node *create_payload_node(parsed_packet pkt, FILE* fptr) {

  Node *const node = malloc(sizeof(Node));
  assert(node != NULL);

  // allocate memory for value
  parsed_payload *value = malloc(sizeof(parsed_payload));
  assert(value != NULL);

  // allocate memory for payload
  u_char *const payload = malloc(pkt.payload.data_len);
  memcpy(payload, pkt.payload.data, pkt.payload.data_len);

  // copy payload to value
  *value = (parsed_payload){.data = payload, .data_len = pkt.payload.data_len};

  // move packet data to node
  node->value = value;
  node->key = pkt.protocol == IPPROTO_TCP ? pkt.tcp.seq : 0;
  node->next = NULL;

  return node;
}

Node *create_flow_node(uint64_t key, flow_base_t flow, FILE* fptr) {

  Node *const node = malloc(sizeof(Node));
  assert(node != NULL);

  // allocate memory for value
  node->value = malloc(sizeof(flow_base_t));
  assert(node->value != NULL);

  // copy value to the new node
  memcpy(node->value, &flow, sizeof(flow_base_t));

  node->key = key;
  node->next = NULL;
  return node;
}

// create new flow from packet info and initialize flow direction
flow_base_t create_flow(parsed_packet pkt, FILE* fptr) {

  return pkt.protocol == IPPROTO_TCP
			 ? (flow_base_t){
				   .sip = pkt.src_ip,
				   .dip = pkt.dst_ip,
				   .sp= pkt.tcp.source,
				   .dp= pkt.tcp.dest,
				   .ip_proto = pkt.protocol,
				   .flow_up = NULL,
				   .flow_down = NULL,
			   }
			 : (flow_base_t){
				   .sip = pkt.src_ip,
				   .dip = pkt.dst_ip,
				   .sp= pkt.udp.source,
				   .dp= pkt.udp.dest,
				   .ip_proto = pkt.protocol,
				   .flow_up = NULL,
				   .flow_down = NULL,
			   };
}

// get flow direction by compare src ip of the packet with the flow
Node **get_flow_direction(flow_base_t const *flow, parsed_packet pkt, FILE* fptr) {
  if (DEBUGGING) fprintf(fptr, "***%u vs %u***\n", pkt.src_ip.s_addr, flow->sip.s_addr);
  return pkt.src_ip.s_addr == flow->sip.s_addr 
    ? (Node **)(&flow->flow_up)
    : (Node **)(&flow->flow_down);
}

/*
 * print package payload data (avoid printing binary data)
 */
void print_payload(u_char const *payload, uint payload_size, FILE* fptr) {

  /** if (payload_size > 0) { */
  /**   if (DEBUGGING) fprintf(fptr, "\t\tpayload size: %u bytes\n", payload_size); */
  /** } else { */
  /**   if (DEBUGGING) fprintf(fptr, "\t\tpayload size: 0 bytes\n"); */
  /**   return; */
  /** } */

  if (DEBUGGING) fprintf(fptr, "\n");

  int len = payload_size;
  int len_rem = payload_size;
  int line_width = 11; /* number of bytes per line */
  int line_len;
  int offset = 0; /* zero-based offset counter */
  u_char const *ch = payload;

  if (len <= 0)
    return;

  /* data fits on one line */
  if (len <= line_width) {
    print_hex_ascii_line(ch, len, offset, fptr);
    return;
  }

  /* data spans multiple lines */
  for (;;) {
    /* compute current line length */
    line_len = line_width % len_rem;
    /* print line */
    print_hex_ascii_line(ch, line_len, offset, fptr);
    /* compute total remaining */
    len_rem = len_rem - line_len;
    /* shift pointer to remaining bytes to print */
    ch = ch + line_len;
    /* add offset */
    offset = offset + line_width;
    /* check if we have line width chars or less */
    if (len_rem <= line_width) {
      /* print last line and get out */
      print_hex_ascii_line(ch, len_rem, offset, fptr);
      break;
    }
  }

  return;
}

void print_hex_ascii_line(u_char const *const payload, int len, int offset, FILE* fptr) {

  int gap;
  u_char const *ch;

  /* offset */
  if (DEBUGGING) fprintf(fptr, "\t\t%05d   ", offset);

  /* hex */
  ch = payload;
  for (int i = 0; i < len; i++) {
    if (DEBUGGING) fprintf(fptr, "%02x ", *ch);
    ch++;
    /* print extra space after 8th byte for visual aid */
    if (i == 7)
      if (DEBUGGING) fprintf(fptr, " ");
  }
  /* print space to handle line less than 8 bytes */
  if (len < 8)
    if (DEBUGGING) fprintf(fptr, " ");

  /* fill hex gap with spaces if not full line */
  if (len < 16) {
    gap = 16 - len;
    for (int i = 0; i < gap; i++) {
      if (DEBUGGING) fprintf(fptr, "   ");
    }
  }
  if (DEBUGGING) fprintf(fptr, "   ");

  /* ascii (if printable) */
  ch = payload;
  for (int i = 0; i < len; i++) {
    if (isprint(*ch)) {
      if (DEBUGGING) fprintf(fptr, "%c", *ch);
    } else {
      if (DEBUGGING) fprintf(fptr, ".");
    }
    ch++;
  }

  if (DEBUGGING) fprintf(fptr, "\n");

  return;
}
