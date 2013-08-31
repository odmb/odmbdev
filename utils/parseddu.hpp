 #ifndef H_PARSEDDU
#define H_PARSEDDU

#include <fstream>
#include "data_packet.hpp"

using Packet::svu;

void UpdateLastFewWords(const uint16_t &, svu &);
bool FindStartOfPacket(std::ifstream &, svu &);
void GetRestOfPacket(std::ifstream &, svu &);
void GetRestOfDDUPacket(std::ifstream &, svu &);
void GetRestOfPCPacket(std::ifstream &, svu &);

#endif
