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

using Packet::DataPacket;
using Packet::svu;
using Packet::svust;
using Packet::InRange;

int main(int argc, char *argv[]){
  unsigned int words_per_line(20);
  std::string filename("");
  unsigned int entry_to_find(0);

  if(argc==2 && argv[1][0]!='-'){
    filename=argv[1];
  }else{
    char opt(' ');
    while(( opt=getopt(argc, argv, "w:f:e:") )!=-1){
      switch(opt){
      case 'w':
	words_per_line=atoi(optarg);
	break;
      case 'f':
	filename=optarg;
	break;
      case 'e':
	entry_to_find=atoi(optarg);
	break;
      default:
	std::cerr << "Error: Invalid option flag '" << opt << "'." << std::endl;
      }
    }
  }

  std::ifstream ifs(filename.c_str(),std::ifstream::in | std::ifstream::binary);
  if(ifs.is_open()){
    DataPacket data_packet;
    svu packet(0);
    unsigned int entry(0);
    if(entry_to_find!=0){
      for(entry=0; entry<entry_to_find && FindStartOfPacket(ifs, packet); ++entry){
      }
      if(entry==entry_to_find){
	GetRestOfPacket(ifs, packet);
	data_packet.SetData(packet);
	data_packet.Print(words_per_line);
      }
    }else{
      std::map<DataPacket::ErrorType, unsigned int> type_counter;
      for(entry=0; FindStartOfPacket(ifs, packet); ++entry){
	GetRestOfPacket(ifs, packet);
	data_packet.SetData(packet);
	const DataPacket::ErrorType this_type(data_packet.GetPacketType());
	if((this_type & ~DataPacket::kNoDCFEBs)!=0){
	  std::cout << "Packet " << std::dec << std::setw(8) << std::setfill(' ') << entry+1
		    << " is of type " << std::hex << std::setw(4) << std::setfill('0')
		    << this_type << "." << std::endl;
	}
	if(type_counter.find(this_type)==type_counter.end()){
	  type_counter[this_type]=1;
	}else{
	  ++type_counter[this_type];
	}
      }
      std::cout << std::dec << std::setw(8) << std::setfill(' ') << entry
		<< " total packets:" <<std::endl;
      for(std::map<DataPacket::ErrorType, unsigned int>::iterator it(type_counter.begin());
	  it!=type_counter.end(); ++it){
	std::cout << std::setw(8) << std::dec << std::setfill(' ') << it->second
		  << " packets of type " << std::setw(4) << std::setfill('0') << std::hex
		  << it->first << std::endl;
      }
    }
    ifs.close();
  }
}

void UpdateLastFewWords(const uint16_t &x, svu &buf){
  if(buf.size()>0){
    for(svust index(0); index+1<buf.size(); ++index){
      buf.at(index)=buf.at(index+1);
    }
    buf.at(buf.size()-1)=x;
  }
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
