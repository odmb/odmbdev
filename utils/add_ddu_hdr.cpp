/*
DDU parsing script
Parses CSC raw files for either single event or whole file diagnostic report. The diagnostic
report produces a 16 bit error-code for each event containing, from MSB to LSB, a 1 for the
errors ((no ODMB trailer, no ODMB header, no OTMB data, no ALCT data), (missing words at end of OTMB data, extra words at end of OTMB data, missing words at start of OTMB data, extra words at start of OTMB data), (missing words at end of ALCT data, extra words at end of ALCT data, missing words at start of ALCT data, extra words at start of ALCT data), (unparsed words, no DDU trailer, no DDU header, no DCFEB data)).

Command line options:
-f: Sets input file.
-e: Sets event number to parse and process. Setting this to zero or leaving it unset produces a     diagnostic report for all events in the file.
-w: Sets the number of words to print per line. Default is 20.

If only one comman line option is given (without a "-"), it is used as a file name and a diagnostic report is produced.

Author: Adam Dishaw (ald77@physics.ucsb.edu)
Last modified: 2013-09-19
*/

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <unistd.h>
#include "parseddu.hpp"
#include "data_packet.hpp"

#define MaxWords 10000
using namespace std;

void UpdateLastFewWords(const uint16_t &, Packet::svu &);
bool FindStartOfPacket(std::ifstream &, Packet::svu &);
void GetRestOfPacket(std::ifstream &, Packet::svu &);
void GetRestOfDDUPacket(std::ifstream &, Packet::svu &);
void GetRestOfPCPacket(std::ifstream &, Packet::svu &);
int calcDDUcrc(unsigned short uipacket[], unsigned int entries);
using Packet::DataPacket;
using Packet::svu;
using Packet::svust;
using Packet::InRange;


int main(int argc, char *argv[]){
  std::string filename("");

  if(argc==2 && argv[1][0]!='-'){
    filename=argv[1];
  }else{
    cout<<"USAGE: ./add_ddu_hdr.exe FileName"<<endl;
    return 0;
  }

  unsigned short dduheader[] = {0xF860, 0x0002, 0x0000, 0x5000, 0x0000, 0x8000, 
				0x0001, 0x8000, 0x2FC1, 0x0001, 0x3030, 0x0001};
  unsigned short ddutrailer[] = {0x8000, 0x8000, 0xFFFF, 0x8000, 0x0001, 0x0005, 
				 0xC2DB, 0x8040, 0xC2C0, 0x4918, 0x000E, 0xA000};
  string outname("ddu_" + filename); 
  FILE *outfile;
  outfile = fopen(outname.c_str(),"wb");

  std::ifstream ifs(filename.c_str(),std::ifstream::in | std::ifstream::binary);
  if(ifs.is_open()){
    DataPacket data_packet;
    svu packet(0);
    unsigned short uipacket[MaxWords];
    unsigned int entry=0;

    // Loop over all ODMB packets
    for(entry=0; FindStartOfPacket(ifs, packet); ++entry){
      GetRestOfPacket(ifs, packet);
      if(packet.size()>=MaxWords){cout<<"Event "<<entry<<" is too long"<<endl; continue;}
      for(unsigned int ind=0; ind<packet.size(); ind++){
	uipacket[ind] = packet[ind];
      }

      // Parsing ODMB header
      unsigned short l1a_lsb = 0x0FFF & uipacket[0];
      unsigned short l1a_msb = 0x0FFF & uipacket[1];
      unsigned short bxcnt = 0x0FFF & uipacket[3];
      dduheader[1] = (bxcnt<<4) | (dduheader[1] & 0x000F);
      dduheader[2] = (l1a_msb<<12) | l1a_lsb;
      dduheader[3] = (l1a_msb>>4) | (dduheader[3] & 0xFF00);
      int CRC = calcDDUcrc(uipacket, packet.size());
      int nWords = 6 + packet.size()/4;
      ddutrailer[9] = 0xFFFF & CRC;
      ddutrailer[10] = 0xFFFF & nWords;
      ddutrailer[11] = (0x00FF & nWords>>16) | (ddutrailer[11] & 0xFF00);

      // Write packet to file
      fwrite(dduheader, sizeof(dduheader[0]), 12, outfile);
      fwrite(uipacket, sizeof(uipacket[0]), packet.size(), outfile);
      fwrite(ddutrailer, sizeof(ddutrailer[0]), 12, outfile);
    }
    ifs.close();
    if(outfile) fclose(outfile);
    cout<<outname<<" printed with "<<entry<<" events"<<endl;
  } else cout << filename << " does not exist"<<endl;

  return 1;
}

bool FindStartOfPacket(std::ifstream &ifs, svu &header){
  header.clear();
  header.resize(8,0);
  uint16_t word(0);
  while(ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    UpdateLastFewWords(word, header);
    if(header.at(5)==0x8000 && header.at(6)==0x0001 && header.at(7)==0x8000){
      for(unsigned int word_count(8);
	  word_count<20 && ifs.read(reinterpret_cast<char*>(&word), sizeof(word));
	  ++word_count){
	header.push_back(word);
      }
      return true;
    }
    if(InRange(header.at(0), 0x9000, 0x9FFF)
       && InRange(header.at(1), 0x9000, 0x9FFF)
       && InRange(header.at(2), 0x9000, 0x9FFF)
       && InRange(header.at(3), 0x9000, 0x9FFF)
       && InRange(header.at(4), 0xA000, 0xAFFF)
       && InRange(header.at(5), 0xA000, 0xAFFF)
       && InRange(header.at(6), 0xA000, 0xAFFF)
       && InRange(header.at(7), 0xA000, 0xAFFF)) return true;
  }
  return false;
}

void UpdateLastFewWords(const uint16_t &x, svu &buf){
  if(buf.size()>0){
    for(svust index(0); index+1<buf.size(); ++index){
      buf.at(index)=buf.at(index+1);
    }
    buf.at(buf.size()-1)=x;
  }
}

void GetRestOfDDUPacket(std::ifstream &ifs, svu &packet){
  uint16_t word(0);
  while(packet.size()<12 && ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    packet.push_back(word);
  }
  while(!(packet.at(packet.size()-12)==0x8000
	  && packet.at(packet.size()-11)==0x8000
	  && packet.at(packet.size()-10)==0xFFFF
	  && packet.at(packet.size()-9)==0x8000)
	&& ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    packet.push_back(word);
  }
}

void GetRestOfPCPacket(std::ifstream &ifs, svu &packet){
  uint16_t word(0);
  while(!(InRange(packet.at(packet.size()-8), 0xF000, 0xFFFF)
	  && InRange(packet.at(packet.size()-7), 0xF000, 0xFFFF)
	  && InRange(packet.at(packet.size()-6), 0xF000, 0xFFFF)
	  && InRange(packet.at(packet.size()-5), 0xF000, 0xFFFF)
	  && InRange(packet.at(packet.size()-4), 0xE000, 0xEFFF)
	  && InRange(packet.at(packet.size()-3), 0xE000, 0xEFFF)
	  && InRange(packet.at(packet.size()-2), 0xE000, 0xEFFF)
	  && InRange(packet.at(packet.size()-1), 0xE000, 0xEFFF))
	&& ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    packet.push_back(word);
  }
}

void GetRestOfPacket(std::ifstream &ifs, svu &packet){
  if(packet.size()<8) return;
  if(packet.at(5)==0x8000 && packet.at(6)==0x0001 && packet.at(7)==0x8000){
    GetRestOfDDUPacket(ifs, packet);
  }else{
    GetRestOfPCPacket(ifs, packet);
  }
}

int calcDDUcrc(unsigned short vec[], unsigned int entries){
  int CRC=0;
  for (uint16_t j=0, w=0; j<entries; j++ ){
    w = vec[j] & 0xffff;
    for (uint32_t i=15, t=0, ncrc=0; i<16; i--){
      t = ((w >> i) & 1) ^ ((CRC >> 15) & 1);
      ncrc = (CRC << 1) & 0xfffc;
      ncrc |= (t ^ (CRC & 1)) << 1;
      ncrc |= t;
      CRC = ncrc;
    }
  }
  
  return CRC;
}
