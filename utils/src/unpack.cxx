/*
  DCFEB unpacking script
  Parses CSC raw files, extracts DCFEB data (ignoring CFEBs and any improperly parsed DCFEBs),
  and produces diagnostic plots and information.

  Command line options:
  -f: Sets input file.
  -s: Sets the first event to parse and process.
  -e: Sets the last event to parse and process.
  -p: Make plot to visualize muons

  Author: Adam Dishaw (ald77@physics.ucsb.edu)
  Last modified: 2014-05-29
*/

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>
#include <map>
#include <stdint.h>
#include <unistd.h>
#include "TFile.h"
#include "TTree.h"
#include "unpack.hpp"
#include "data_packet.hpp"
#include "unpacker.hpp"

using Packet::DataPacket;
using Packet::svu;
using Packet::InRange;

int main(int argc, char *argv[]){
  std::string filename("");
  unsigned start_entry(0), end_entry(0);
  bool plot_muons(false);

  std::stringstream ss("");

  if(argc==2 && argv[1][0]!='-'){
    filename=argv[1];
  }else{
    char opt(' ');
    while(( opt=getopt(argc, argv, "f:s:e:p") )!=-1){
      switch(opt){
      case 'f':
        filename=optarg;
        break;
      case 's':
        start_entry=GetNumber(optarg);
        break;
      case 'e':
        end_entry=GetNumber(optarg);
        break;
      case 'p':
        plot_muons=true;
        break;
      default:
        std::cerr << "Error: Invalid option flag '" << opt << "'." << std::endl;
      }
    }
  }

  if(start_entry!=0 || end_entry!=0){
    if(end_entry==0) end_entry=start_entry;
    if(start_entry==0) start_entry=end_entry;
    if(start_entry>end_entry){
      const unsigned temp(start_entry);
      start_entry=end_entry;
      end_entry=temp;
    }
  }else{
    start_entry=1;
    end_entry=std::numeric_limits<unsigned>::max();
  }

  std::ifstream ifs(filename.c_str(),std::ifstream::in | std::ifstream::binary);
  if(ifs.is_open()){
    DataPacket data_packet;
    svu packet(0);
    unsigned entry(1);
    
    for(entry=1; entry<start_entry && FindStartOfNextPacket(ifs, packet); ++entry){
    }
    Packet::Unpacker unpacker;
    TFile file((filename+".root").c_str(),"recreate");
    file.cd();
    TTree tree("unpack","unpack");
    uint16_t l1a(0), dcfeb(0), cutoff(0);
    float muon_peak(0.0), muon_average(0.0), pedestal_average(0.0), muon_pedestal_ratio(0.0), average_layer(0.0), average_strip(0.0), average_time_bin(0.0);
    tree.Branch("l1a", &l1a);
    tree.Branch("dcfeb", &dcfeb);
    tree.Branch("cutoff", &cutoff);
    tree.Branch("muon_peak", &muon_peak);
    tree.Branch("muon_average", &muon_average);
    tree.Branch("pedestal_average", &pedestal_average);
    tree.Branch("muon_pedestal_ratio", &muon_pedestal_ratio);
    tree.Branch("average_layer", &average_layer);
    tree.Branch("average_strip", &average_strip);
    tree.Branch("average_time_bin", &average_time_bin);
    for(; entry<=end_entry && FindStartOfNextPacket(ifs, packet); ++entry){
      GetRestOfPacket(ifs, packet);
      data_packet.SetData(packet);
      const std::vector<std::pair<unsigned, std::vector<Packet::dcfeb_data> > > raw_dcfeb_data(data_packet.GetValidDCFEBData());
      for(unsigned dcfeb_num(0); dcfeb_num<raw_dcfeb_data.size(); ++dcfeb_num){
        unpacker.SetData(raw_dcfeb_data.at(dcfeb_num).second, data_packet.GetL1A(), raw_dcfeb_data.at(dcfeb_num).first);
        l1a=unpacker.l1a();
        dcfeb=unpacker.dcfeb();
        cutoff=unpacker.GetCutoff();
        muon_peak=unpacker.GetMuonPeak();
        muon_average=unpacker.GetMuonAverage();
        pedestal_average=unpacker.GetPedestalAverage();
        muon_pedestal_ratio=unpacker.GetMuonPedestalRatio();
        average_layer=unpacker.GetAverageLayer();
        average_strip=unpacker.GetAverageStrip();
        average_time_bin=unpacker.GetAverageTimeBin();
        tree.Fill();
        /*std::cout << ' ' << std::setw(8) << unpacker.l1a()
          << ' ' << std::setw(8) << unpacker.dcfeb()
          << ' ' << std::setw(8) << unpacker.GetCutoff()
          << ' ' << std::setw(8) << unpacker.GetMuonAverage()
          << ' ' << std::setw(8) << unpacker.GetPedestalAverage()
          << ' ' << std::setw(8) << unpacker.GetMuonPedestalRatio()
          << ' ' << std::setw(8) << unpacker.GetAverageLayer()
          << ' ' << std::setw(8) << unpacker.GetAverageStrip()
          << ' ' << std::setw(8) << unpacker.GetAverageTimeBin() << std::endl;*/
        std::ostringstream oss("");
        oss << "testing_" << unpacker.l1a() << '_' << unpacker.dcfeb() << ".pdf";
        //unpacker.PrintData();
        if(plot_muons) unpacker.MakePlot(oss.str());
      }
    } 
    file.cd();
    tree.Write();
    file.Close();
    ifs.close();
  }else{
    std::cerr << "Error: could not open " << filename << std::endl;
  }
}

void UpdateLastFewWords(const uint16_t x, svu& buf){
  if(buf.size()>0){
    for(unsigned index(0); index+1<buf.size(); ++index){
      buf.at(index)=buf.at(index+1);
    }
    buf.at(buf.size()-1)=x;
  }
}

bool FindStartOfPacket(std::ifstream& ifs, svu& header){
  header.clear();
  header.resize(8,0);
  uint16_t word(0);
  while(ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    UpdateLastFewWords(word, header);
    if(header.at(5)==0x8000 && header.at(6)==0x0001 && header.at(7)==0x8000){
      for(unsigned word_count(8);
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
       && InRange(header.at(6), 0xA000, 0xAFFF)) return true;
  }
  return false;
}

bool FindStartOfNextPacket(std::ifstream&ifs, svu& header){
  GetRestOfPacket(ifs, header);
  header.clear();
  return FindStartOfPacket(ifs, header);
}

void GetRestOfDDUPacket(std::ifstream& ifs, svu& packet){
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

void GetRestOfPCPacket(std::ifstream& ifs, svu& packet){
  uint16_t word(0);
  while(!(InRange(packet.at(packet.size()-8), 0xF000, 0xFFFF)
          && InRange(packet.at(packet.size()-7), 0xF000, 0xFFFF)
          && InRange(packet.at(packet.size()-6), 0xF000, 0xFFFF)
          && InRange(packet.at(packet.size()-5), 0xF000, 0xFFFF)
          && InRange(packet.at(packet.size()-4), 0xE000, 0xEFFF)
          && InRange(packet.at(packet.size()-3), 0xE000, 0xEFFF)
          && InRange(packet.at(packet.size()-2), 0xE000, 0xEFFF))
        && ifs.read(reinterpret_cast<char*>(&word), sizeof(word))){
    packet.push_back(word);
  }
}

void GetRestOfPacket(std::ifstream& ifs, svu& packet){
  if(packet.size()<8) return;
  if(packet.at(5)==0x8000 && packet.at(6)==0x0001 && packet.at(7)==0x8000){
    GetRestOfDDUPacket(ifs, packet);
  }else{
    GetRestOfPCPacket(ifs, packet);
  }
}

uint_fast64_t GetNumber(const std::string& input){
  return strtol(input.c_str(), NULL, 0);
}
