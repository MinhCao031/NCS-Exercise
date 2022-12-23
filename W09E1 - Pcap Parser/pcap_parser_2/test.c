#include "lib/handler.h"
#define PCAP_FILE "TCPsample.pcap"
#define LIMIT_PACKET 2700000

extern const int DEBUGGING;
int const HASH_TABLE_SIZE = 50;


void get_packets(pcap_t *handler, FILE* fptr, FILE* fptr2);

int main(void) {
  FILE* fptr = fopen("output1.txt", "w");
  FILE* fptr2 = fopen("output2.txt", "w");

  // error buffer
  char errbuff[PCAP_ERRBUF_SIZE];

  // open file and create pcap handler
  pcap_t *const handler = pcap_open_offline(PCAP_FILE, errbuff);
  if (handler == NULL) {
    fprintf(stderr, "Error opening file: %s\n", errbuff);
    exit(EXIT_FAILURE);
  }
  get_packets(handler, stdout, stdout);
  pcap_close(handler);
  return 0;
}

void get_packets(pcap_t *handler, FILE* fptr1, FILE* fptr2) {

  // The header that pcap gives us
  struct pcap_pkthdr *header;

  // The actual packet
  u_char const *full_packet;

  int packetCount = 0;

  // create hash table
  HashTable table = create_hash_table(HASH_TABLE_SIZE);

  while (pcap_next_ex(handler, &header, &full_packet) >= 0) {

    // Show the packet number
    if (DEBUGGING) fprintf(fptr1, "Packet # %i\n", ++packetCount);

    //--------------------------------------------------------------------------
    package frame = frame_dissector(full_packet, header, fptr1);
    if (frame.is_valid == false) {
      if (DEBUGGING) fprintf(fptr1, "ERROR: Frame is not valid!\n");
      goto END;
    }

    //--------------------------------------------------------------------------
    package packet = link_dissector(frame, fptr1);
    if (packet.is_valid == false) {
      if (DEBUGGING) fprintf(fptr1, "ERROR: Packet is not valid!\n");
      goto END;
    }

    //--------------------------------------------------------------------------
    package segment = network_dissector(packet, fptr1);
    if (segment.is_valid == false) {
      if (DEBUGGING) fprintf(fptr1, "ERROR: Segment is not valid!\n");
      goto END;
    }

    //--------------------------------------------------------------------------
    package payload = transport_demux(segment, fptr1);
    if (payload.is_valid == false) {
      if (DEBUGGING) fprintf(fptr1, "ERROR: Payload is not valid!\n");
      goto END;
    }

    // insert to hash table
    parsed_packet pkt = pkt_parser(packet, segment, payload);
    insert_packet(table, pkt, fptr1);
    if (DEBUGGING)
      fprintf(fptr1, "------------------------------------"
              "------------Successfully------------\n");
    if (packetCount > LIMIT_PACKET) break;
    continue;

    END: {
      if (DEBUGGING) 
        fprintf(fptr1, "------------------------------------"
                "------------PacketFailed------------\n");
      if (packetCount > LIMIT_PACKET) break;
    }
  }

  
  print_hashtable(table, fptr2);
  // fprintf(fptr, "data length: %d\n", pop_head_payload(&search_flow(table, 3316805598312908751)->flow_up).data_len);
  // print_flow(*search_flow(table, 94129317375700));
  fprintf(fptr2, "number of flows: %d\n", count_flows(table));
  fprintf(fptr2, "Number of packets: %d ~ %d\n", count_packets(table), inserted_packets += 1);

  free_hash_table(table);
}
