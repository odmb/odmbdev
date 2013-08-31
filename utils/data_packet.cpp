#include "data_packet.hpp"
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdint.h>

namespace Packet{
  bool AllInRange(const svu &vec, const svust &start, const svust &end,
		  const uint16_t &low, const uint16_t &high){
    bool all_in_range(true);
    for(svust index(start); index<vec.size() && index<end && all_in_range; ++index){
      if(!InRange(vec.at(index), low, high)) all_in_range=false;
    }
    return all_in_range;
  }

  void PrintBuffer(const svu &buffer, const unsigned int &words_per_line){
    for(unsigned int index(0); index<buffer.size(); ++index){
      if(index && !(index%words_per_line)) std::cout << std::endl;
      std::cout << std::hex << std::setw(4) << std::setfill('0') << buffer.at(index) << " ";
    }
    std::cout << std::endl;
  }

  void PutInRange(svust &a, svust &b, const svust &min, const svust &max){
    if(max<=min){
      a=max;
      b=max;
    }else{
      if(a<min) a=min;
      if(a>max) a=max;
      if(b<a) b=a;
      if(b>max) b=max;
    }
  }

  DataPacket::DataPacket():
    full_packet(0),
    ddu_header_start(-1), ddu_header_end(-1),
    odmb_header_start(-1), odmb_header_end(-1),
    alct_start(-1), alct_end(-1),
    otmb_start(-1), otmb_end(-1),
    dcfeb_start(0), dcfeb_end(0),
    odmb_trailer_start(-1), odmb_trailer_end(-1),
    ddu_trailer_start(-1), ddu_trailer_end(-1),
    parsed(false){
  }

  DataPacket::DataPacket(const svu &packet_in):
    full_packet(packet_in),
    ddu_header_start(-1), ddu_header_end(-1),
    odmb_header_start(-1), odmb_header_end(-1),
    alct_start(-1), alct_end(-1),
    otmb_start(-1), otmb_end(-1),
    dcfeb_start(0), dcfeb_end(0),
    odmb_trailer_start(-1), odmb_trailer_end(-1),
    ddu_trailer_start(-1), ddu_trailer_end(-1),
    parsed(false){
  }

  void DataPacket::SetData(const svu &packet_in){
    parsed=false;
    full_packet=packet_in;
  }

  svu DataPacket::GetData() const{
    return full_packet;
  }

  svu DataPacket::GetDDUHeader() const{
    if(!parsed) Parse();
    return GetComponent(0, ddu_header_end);
  }

  svu DataPacket::GetODMBHeader() const{
    if(!parsed) Parse();
    return GetComponent(odmb_header_start, odmb_header_end);
  }

  svu DataPacket::GetALCTData() const{
    if(!parsed) Parse();
    return GetComponent(alct_start, alct_end);
  }

  svu DataPacket::GetOTMBData() const{
    if(!parsed) Parse();
    return GetComponent(otmb_start, otmb_end);
  }

  std::vector<svu> DataPacket::GetDCFEBData() const{
    if(!parsed) Parse();
    std::vector<svu> data(0);
    for(svsvustst dcfeb(0); dcfeb<dcfeb_start.size(); ++dcfeb){
      data.push_back(GetComponent(dcfeb_start.at(dcfeb), dcfeb_end.at(dcfeb)));
    }
    return data;
  }

  svu DataPacket::GetODMBTrailer() const{
    if(!parsed) Parse();
    return GetComponent(odmb_trailer_start, odmb_trailer_end);
  }

  svu DataPacket::GetDDUTrailer() const{
    if(!parsed) Parse();
    return GetComponent(ddu_trailer_start, ddu_trailer_end);
  }

  svu DataPacket::GetComponent(const svust &start, const svust &end) const{
    if(start<=end && end<=full_packet.size()){
      return svu(full_packet.begin()+start, full_packet.begin()+end);
    }else{
      return svu(0);
    }
  }

  DataPacket::ErrorType DataPacket::GetPacketType() const{
    if(!parsed) Parse();
    return static_cast<ErrorType>((HasNoDCFEBs()?no_dcfebs:good)
				  | (HasNoDDUHeader()?no_ddu_header:good)
				  | (HasNoDDUTrailer()?no_ddu_trailer:good)
				  | (HasExtraALCTStartWords()?extra_alct_start:good)
				  | (HasMissingALCTStartWords()?missing_alct_start:good)
				  | (HasExtraALCTEndWords()?extra_alct_end:good)
				  | (HasMissingALCTEndWords()?missing_alct_end:good)
				  | (HasExtraOTMBStartWords()?extra_otmb_start:good)
				  | (HasMissingOTMBStartWords()?missing_otmb_start:good)
				  | (HasExtraOTMBEndWords()?extra_otmb_end:good)
				  | (HasMissingOTMBEndWords()?missing_otmb_end:good)
				  | (HasNoALCT()?no_alct:good)
				  | (HasNoOTMB()?no_otmb:good)
				  | (HasUnusedWords()?unused_words:good)
				  | (HasNoODMBHeader()?no_odmb_header:good)
				  | (HasNoODMBTrailer()?no_odmb_trailer:good));
  }
  
  void DataPacket::Parse() const{
    FindDDUHeader();
    FindODMBHeader();
    FindALCTandOTMBData();
    FindDCFEBData();
    FindODMBTrailer();
    FindDDUTrailer();
    parsed=true;
  }

  void DataPacket::FindDDUHeader() const{
    ddu_header_start=-1;
    ddu_header_end=-1;
    for(svust index(0); index+11<full_packet.size(); ++index){
      if(full_packet.at(index+5)==0x8000
	 && full_packet.at(index+6)==0x0001
	 && full_packet.at(index+7)==0x8000){
	ddu_header_start=index;
	ddu_header_end=index+12;
	return;
      }
    }
  }

  void DataPacket::FindODMBHeader() const{
    odmb_header_start=-1;
    odmb_header_end=-1;
    for(svust index(0); index+7<full_packet.size(); ++index){
      if(InRange(full_packet.at(index), 0x9000, 0x9FFF)
	 && InRange(full_packet.at(index+1), 0x9000, 0x9FFF)
	 && InRange(full_packet.at(index+2), 0x9000, 0x9FFF)
	 && InRange(full_packet.at(index+3), 0x9000, 0x9FFF)
	 && InRange(full_packet.at(index+4), 0xA000, 0xAFFF)
	 && InRange(full_packet.at(index+5), 0xA000, 0xAFFF)
	 && InRange(full_packet.at(index+6), 0xA000, 0xAFFF)
	 && InRange(full_packet.at(index+7), 0xA000, 0xAFFF)){
	odmb_header_start=index;
	odmb_header_end=index+8;
	return;
      }
    }
  }

  void DataPacket::FindALCTandOTMBData() const{
    const unsigned int d_run_threshhold(3);
    const svust bad_index(-1);
    alct_start=bad_index;
    alct_end=bad_index;
    otmb_start=bad_index;
    otmb_end=bad_index;
    svust d_run_start_1(bad_index), d_run_end_1(bad_index);
    svust d_run_start_2(bad_index), d_run_end_2(bad_index);
    svust d_run_start_3(bad_index), d_run_end_3(bad_index);
    FindRunInRange(d_run_start_1, d_run_end_1, 0, d_run_threshhold, 0xD000, 0xDFFF);
    FindRunInRange(d_run_start_2, d_run_end_2, d_run_end_1, d_run_threshhold, 0xD000, 0xDFFF);
    FindRunInRange(d_run_start_3, d_run_end_3, d_run_end_2, d_run_threshhold, 0xD000, 0xDFFF);
    if(d_run_start_3==bad_index){
      if(d_run_start_2==bad_index){
	if(d_run_start_1!=bad_index){
	  alct_start=d_run_start_1;
	  alct_end=d_run_end_1;
	}
      }else{
	alct_start=d_run_start_1;
	alct_end=d_run_end_2;
      }
    }else{
      alct_start=d_run_start_1;
      otmb_start=SplitALCTandOTMB(d_run_start_2, d_run_end_2);
      alct_end=otmb_start;
      otmb_end=d_run_end_3;
    }
  }

  void DataPacket::FindDCFEBData() const{
    dcfeb_start.clear();
    dcfeb_end.clear();
    std::vector<svust> temp_dcfeb_end(0);
    for(svust index(99); index<full_packet.size(); ++index){
      if(full_packet.at(index)==0x7FFF &&
	 (index==99 || full_packet.at(index-100)==0x7FFF
	  || InRange(full_packet.at(index-100), 0xD000, 0xDFFF))){
	temp_dcfeb_end.push_back(index+1);
      }
    }
    for(svsvustst dcfeb(7); dcfeb<temp_dcfeb_end.size(); dcfeb+=8){
      dcfeb_start.push_back(temp_dcfeb_end.at(dcfeb-7)-100);
      dcfeb_end.push_back(temp_dcfeb_end.at(dcfeb));
    }
  }

  void DataPacket::FindODMBTrailer() const{
    odmb_trailer_start=-1;
    odmb_trailer_end=-1;
    for(svust index(0); index+7<full_packet.size(); ++index){
      if(InRange(full_packet.at(index), 0xF000, 0xFFFF)
	 && InRange(full_packet.at(index+1), 0xF000, 0xFFFF)
	 && InRange(full_packet.at(index+2), 0xF000, 0xFFFF)
	 && InRange(full_packet.at(index+3), 0xF000, 0xFFFF)
	 && InRange(full_packet.at(index+4), 0xE000, 0xEFFF)
	 && InRange(full_packet.at(index+5), 0xE000, 0xEFFF)
	 && InRange(full_packet.at(index+6), 0xE000, 0xEFFF)
	 && InRange(full_packet.at(index+7), 0xE000, 0xEFFF)){
	odmb_trailer_start=index;
	odmb_trailer_end=index+8;
      }
    }
  }

  void DataPacket::FindDDUTrailer() const{
    ddu_trailer_start=-1;
    ddu_trailer_end=-1;
    for(svust index(0); index+11<full_packet.size(); ++index){
      if(full_packet.at(index)==0x8000
	 && full_packet.at(index+1)==0x8000
	 && full_packet.at(index+2)==0xFFFF
	 && full_packet.at(index+3)==0x8000){
	ddu_trailer_start=index;
	ddu_trailer_end=index+12;
      }
    }
  }

  void DataPacket::FindRunInRange(svust &start, svust &end, const svust &left,
				  const svust &min_length, const uint16_t &low,
				  const uint16_t &high) const{
    start=-1;
    end=-1;
    svust index(left);
    for(; index+min_length-1<full_packet.size(); ++index){
      if(AllInRange(full_packet, index, index+min_length, low, high)){
	start=index;
	end=full_packet.size();
	break;
      }
    }
    for(index=index+min_length; index<full_packet.size(); ++index){
      if(!InRange(full_packet.at(index), low, high)){
	end=index;
	break;
      }
    }
  }

  svust DataPacket::SplitALCTandOTMB(const svust &start, const svust &end) const{
    if(start<end){
      for(svust index(start); index<end && index<full_packet.size(); ++index){
	if(full_packet.at(index)==0xDB0C) return index;
      }
      for(svust index(start); index<end && index<full_packet.size(); ++index){
	if(full_packet.at(index)==0xDB0A) return index;
      }
      return static_cast<svust>(ceil(start+0.5*(end-start)));
    }else{
      return end;
    }
  }

  void DataPacket::Print(const unsigned int &words_per_line) const{
    if(!parsed) Parse();
    svust ddu_header_start_temp(ddu_header_start), ddu_header_end_temp(ddu_header_end);
    svust odmb_header_start_temp(odmb_header_start), odmb_header_end_temp(odmb_header_end);
    svust alct_start_temp(alct_start), alct_end_temp(alct_end);
    svust otmb_start_temp(otmb_start), otmb_end_temp(otmb_end);
    std::vector<svust> dcfeb_start_temp(dcfeb_start), dcfeb_end_temp(dcfeb_end);
    svust odmb_trailer_start_temp(odmb_trailer_start), odmb_trailer_end_temp(odmb_trailer_end);
    svust ddu_trailer_start_temp(ddu_trailer_start), ddu_trailer_end_temp(ddu_trailer_end);

    const svsvustst num_dcfebs(dcfeb_start.size());
    const std::string uncat("Uncategorized");

    PutInRange(ddu_header_start_temp, ddu_header_end_temp, 0, full_packet.size());
    PutInRange(odmb_header_start_temp, odmb_header_end_temp,
	       ddu_header_end_temp, full_packet.size());
    PutInRange(alct_start_temp, alct_end_temp, odmb_header_end_temp, full_packet.size());
    PutInRange(otmb_start_temp, otmb_end_temp, alct_end_temp, full_packet.size());
    if(num_dcfebs>0){
      PutInRange(dcfeb_start_temp.at(0), dcfeb_end_temp.at(0),
		 otmb_end_temp, full_packet.size());
      for(svsvustst dcfeb(1); dcfeb<num_dcfebs; ++dcfeb){
	PutInRange(dcfeb_start_temp.at(dcfeb), dcfeb_end_temp.at(dcfeb),
		   dcfeb_start_temp.at(dcfeb-1), full_packet.size());
      }
      PutInRange(odmb_trailer_start_temp, odmb_trailer_end_temp,
		 dcfeb_end_temp.at(num_dcfebs-1), full_packet.size());
    }else{
      PutInRange(odmb_trailer_start_temp, odmb_trailer_end_temp,
		 otmb_end_temp, full_packet.size());
    }
    PutInRange(ddu_trailer_start_temp, ddu_trailer_end_temp,
	       odmb_trailer_end_temp, full_packet.size());

    PrintComponent(uncat, 0, ddu_header_start_temp, words_per_line);
    PrintComponent("DDU Header", ddu_header_start_temp, ddu_header_end_temp, words_per_line);
    PrintComponent(uncat, ddu_header_end_temp, odmb_header_start_temp, words_per_line);
    PrintComponent("ODMB Header", odmb_header_start_temp,
		   odmb_header_end_temp, words_per_line);
    PrintComponent(uncat, odmb_header_end_temp, alct_start_temp, words_per_line);
    PrintComponent("ALCT", alct_start_temp, alct_end_temp, words_per_line);
    PrintComponent(uncat, alct_end_temp, otmb_start_temp, words_per_line);
    PrintComponent("OTMB", otmb_start_temp, otmb_end_temp, words_per_line);
    if(num_dcfebs>0){
      PrintComponent(uncat, otmb_end_temp, dcfeb_start_temp.at(0), words_per_line);
      for(svsvustst dcfeb(0); dcfeb+1<num_dcfebs; ++dcfeb){
	std::ostringstream oss("");
	oss << "DCFEB " << dcfeb+1;
	PrintComponent(oss.str(), dcfeb_start_temp.at(dcfeb),
		       dcfeb_end_temp.at(dcfeb), words_per_line);
	PrintComponent(uncat, dcfeb_end_temp.at(dcfeb),
		       dcfeb_start_temp.at(dcfeb+1), words_per_line);
      }
      std::ostringstream oss("");
      oss << "DCFEB " << num_dcfebs;
      PrintComponent(oss.str(), dcfeb_start_temp.at(num_dcfebs-1),
		     dcfeb_end_temp.at(num_dcfebs-1), words_per_line);
      PrintComponent(uncat, dcfeb_end_temp.at(num_dcfebs-1),
		     odmb_trailer_start_temp, words_per_line);
    }
    PrintComponent("ODMB Trailer", odmb_trailer_start_temp,
		     odmb_trailer_end_temp, words_per_line);
    PrintComponent(uncat, odmb_trailer_end_temp, ddu_trailer_start_temp, words_per_line);
    PrintComponent("DDU Trailer", ddu_trailer_start_temp,
		   ddu_trailer_end_temp, words_per_line);
    PrintComponent(uncat, ddu_trailer_end_temp, full_packet.size(), words_per_line);
  }


  void DataPacket::PrintComponent(const std::string &str, const svust &start,
				  const svust &end, const unsigned int &words_per_line) const{
    if(start<end){
      std::cout << str << std::endl;
      PrintBuffer(GetComponent(start, end), words_per_line);
      std::cout << std::endl;
    }
  }

  bool DataPacket::HasUnusedWords() const{
    if(!parsed) Parse();
    if(ddu_header_end<odmb_header_start) return true;
    if(odmb_header_end<alct_start) return true;
    if(alct_end<otmb_start) return true;
    if(dcfeb_start.size()>0){
      if(otmb_end<dcfeb_start.at(0)) return true;
      for(svsvustst dcfeb(0); dcfeb+1<dcfeb_start.size(); ++dcfeb){
	if(dcfeb_end.at(dcfeb)<dcfeb_start.at(dcfeb+1)) return true;
      }
      if(dcfeb_end.at(dcfeb_end.size()-1)<odmb_trailer_start) return true;     
    }else{
      if(otmb_end<odmb_trailer_start) return true;
    }
    if(odmb_trailer_end<ddu_trailer_start) return true;
    if(ddu_trailer_end<full_packet.size()) return true;
    return false;
  }

  unsigned short DataPacket::GetContainingRanges(const svust &word) const{
    unsigned short num_ranges(0);
    if(InRange(word, ddu_header_start, ddu_header_end-1)) ++num_ranges;
    if(InRange(word, odmb_header_start, odmb_header_end-1)) ++num_ranges;
    if(InRange(word, alct_start, alct_end-1)) ++num_ranges;
    if(InRange(word, otmb_start, otmb_end-1)) ++num_ranges;
    if(InRange(word, odmb_trailer_start, odmb_trailer_end-1)) ++num_ranges;
    if(InRange(word, ddu_trailer_start, ddu_trailer_end-1)) ++num_ranges;
    for(svsvustst dcfeb(0); dcfeb<dcfeb_start.size(); ++dcfeb){
      if(InRange(word, dcfeb_start.at(dcfeb), dcfeb_end.at(dcfeb)-1)) ++num_ranges;
    }
    return num_ranges;
  }

  bool DataPacket::HasNoDDUHeader() const{
    if(!parsed) Parse();
    return ddu_header_end==ddu_header_start;
  }

  bool DataPacket::HasNoODMBHeader() const{
    if(!parsed) Parse();
    return odmb_header_end==odmb_header_start;
  }

  bool DataPacket::HasNoALCT() const{
    if(!parsed) Parse();
    return alct_end==alct_start;
  }

  bool DataPacket::HasNoOTMB() const{
    if(!parsed) Parse();
    return otmb_end==otmb_start;
  }

  bool DataPacket::HasNoDCFEBs() const{
    if(!parsed) Parse();
    return dcfeb_start.size()==0;
  }

  bool DataPacket::HasNoODMBTrailer() const{
    if(!parsed) Parse();
    return odmb_trailer_end==odmb_trailer_start;
  }

  bool DataPacket::HasNoDDUTrailer() const{
    if(!parsed) Parse();
    return ddu_trailer_end==ddu_trailer_start;
  }

  bool DataPacket::HasExtraALCTStartWords() const{
    if(!parsed) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<5) return false;
    return InRange(alct.at(4), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingALCTStartWords() const{
    if(!parsed) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<4) return true;
    return !InRange(alct.at(0), 0xD000, 0xDFFF)
      || !InRange(alct.at(1), 0xD000, 0xDFFF)
      || !InRange(alct.at(2), 0xD000, 0xDFFF)
      || !InRange(alct.at(3), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasExtraALCTEndWords() const{
    if(!parsed) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<5) return false;
    return InRange(alct.at(alct.size()-5), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingALCTEndWords() const{
    if(!parsed) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<4) return true;
    return !InRange(alct.at(alct.size()-4), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-3), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-2), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-1), 0xD000, 0xDFFF);
  }
  bool DataPacket::HasExtraOTMBStartWords() const{
    if(!parsed) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<5) return false;
    return InRange(otmb.at(4), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingOTMBStartWords() const{
    if(!parsed) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<4) return true;
    return !InRange(otmb.at(0), 0xD000, 0xDFFF)
      || !InRange(otmb.at(1), 0xD000, 0xDFFF)
      || !InRange(otmb.at(2), 0xD000, 0xDFFF)
      || !InRange(otmb.at(3), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasExtraOTMBEndWords() const{
    if(!parsed) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<5) return false;
    return InRange(otmb.at(otmb.size()-5), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingOTMBEndWords() const{
    if(!parsed) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<4) return true;
    return !InRange(otmb.at(otmb.size()-4), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-3), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-2), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-1), 0xD000, 0xDFFF);
  }
}
