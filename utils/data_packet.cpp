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
      if(a<min || a>max) a=min;
      if(b<min || b>max) b=min;
      if(b<a) a=b;
    }
  }

  DataPacket::DataPacket():
    full_packet_(0),
    ddu_header_start_(-1), ddu_header_end_(-1),
    odmb_header_start_(-1), odmb_header_end_(-1),
    alct_start_(-1), alct_end_(-1),
    otmb_start_(-1), otmb_end_(-1),
    dcfeb_start_(0), dcfeb_end_(0),
    odmb_trailer_start_(-1), odmb_trailer_end_(-1),
    ddu_trailer_start_(-1), ddu_trailer_end_(-1),
    parsed_(false){
  }

  DataPacket::DataPacket(const svu &packet_in):
    full_packet_(packet_in),
    ddu_header_start_(-1), ddu_header_end_(-1),
    odmb_header_start_(-1), odmb_header_end_(-1),
    alct_start_(-1), alct_end_(-1),
    otmb_start_(-1), otmb_end_(-1),
    dcfeb_start_(0), dcfeb_end_(0),
    odmb_trailer_start_(-1), odmb_trailer_end_(-1),
    ddu_trailer_start_(-1), ddu_trailer_end_(-1),
    parsed_(false){
  }

  void DataPacket::SetData(const svu &packet_in){
    parsed_=false;
    full_packet_=packet_in;
  }

  svu DataPacket::GetData() const{
    return full_packet_;
  }

  svu DataPacket::GetDDUHeader() const{
    if(!parsed_) Parse();
    return GetComponent(0, ddu_header_end_);
  }

  svu DataPacket::GetODMBHeader() const{
    if(!parsed_) Parse();
    return GetComponent(odmb_header_start_, odmb_header_end_);
  }

  svu DataPacket::GetALCTData() const{
    if(!parsed_) Parse();
    return GetComponent(alct_start_, alct_end_);
  }

  svu DataPacket::GetOTMBData() const{
    if(!parsed_) Parse();
    return GetComponent(otmb_start_, otmb_end_);
  }

  std::vector<svu> DataPacket::GetDCFEBData() const{
    if(!parsed_) Parse();
    std::vector<svu> data(0);
    for(svsvustst dcfeb(0); dcfeb<dcfeb_start_.size(); ++dcfeb){
      data.push_back(GetComponent(dcfeb_start_.at(dcfeb), dcfeb_end_.at(dcfeb)));
    }
    return data;
  }

  svu DataPacket::GetODMBTrailer() const{
    if(!parsed_) Parse();
    return GetComponent(odmb_trailer_start_, odmb_trailer_end_);
  }

  svu DataPacket::GetDDUTrailer() const{
    if(!parsed_) Parse();
    return GetComponent(ddu_trailer_start_, ddu_trailer_end_);
  }

  svu DataPacket::GetComponent(const svust &start, const svust &end) const{
    if(start<=end && end<=full_packet_.size()){
      return svu(full_packet_.begin()+start, full_packet_.begin()+end);
    }else{
      return svu(0);
    }
  }

  DataPacket::ErrorType DataPacket::GetPacketType() const{
    if(!parsed_) Parse();
    return static_cast<ErrorType>((HasNoDCFEBs()?kNoDCFEBs:kGood)
				  | (HasNoDDUHeader()?kNoDDUHeader:kGood)
				  | (HasNoDDUTrailer()?kNoDDUTrailer:kGood)
				  | (HasExtraALCTStartWords()?kExtraALCTStart:kGood)
				  | (HasMissingALCTStartWords()?kMissingALCTStart:kGood)
				  | (HasExtraALCTEndWords()?kExtraALCTEnd:kGood)
				  | (HasMissingALCTEndWords()?kMissingALCTEnd:kGood)
				  | (HasExtraOTMBStartWords()?kExtraOTMBStart:kGood)
				  | (HasMissingOTMBStartWords()?kMissingOTMBStart:kGood)
				  | (HasExtraOTMBEndWords()?kExtraOTMBEnd:kGood)
				  | (HasMissingOTMBEndWords()?kMissingOTMBEnd:kGood)
				  | (HasNoALCT()?kNoALCT:kGood)
				  | (HasNoOTMB()?kNoOTMB:kGood)
				  | (HasUnusedWords()?kUnusedWords:kGood)
				  | (HasNoODMBHeader()?kNoODMBHeader:kGood)
				  | (HasNoODMBTrailer()?kNoODMBTrailer:kGood));
  }
  
  void DataPacket::Parse() const{
    FindDDUHeader();
    FindODMBHeader();
    FindALCTandOTMBData();
    FindDCFEBData();
    FindODMBTrailer();
    FindDDUTrailer();
    parsed_=true;
  }

  void DataPacket::FindDDUHeader() const{
    ddu_header_start_=-1;
    ddu_header_end_=-1;
    for(svust index(0); index+11<full_packet_.size(); ++index){
      if(full_packet_.at(index+5)==0x8000
	 && full_packet_.at(index+6)==0x0001
	 && full_packet_.at(index+7)==0x8000){
	ddu_header_start_=index;
	ddu_header_end_=index+12;
	return;
      }
    }
  }

  void DataPacket::FindODMBHeader() const{
    odmb_header_start_=-1;
    odmb_header_end_=-1;
    for(svust index(0); index+7<full_packet_.size(); ++index){
      if(InRange(full_packet_.at(index), 0x9000, 0x9FFF)
	 && InRange(full_packet_.at(index+1), 0x9000, 0x9FFF)
	 && InRange(full_packet_.at(index+2), 0x9000, 0x9FFF)
	 && InRange(full_packet_.at(index+3), 0x9000, 0x9FFF)
	 && InRange(full_packet_.at(index+4), 0xA000, 0xAFFF)
	 && InRange(full_packet_.at(index+5), 0xA000, 0xAFFF)
	 && InRange(full_packet_.at(index+6), 0xA000, 0xAFFF)
	 && InRange(full_packet_.at(index+7), 0xA000, 0xAFFF)){
	odmb_header_start_=index;
	odmb_header_end_=index+8;
	return;
      }
    }
  }

  void DataPacket::FindALCTandOTMBData() const{
    const unsigned int d_run_threshhold(3);
    const svust bad_index(-1);
    alct_start_=bad_index;
    alct_end_=bad_index;
    otmb_start_=bad_index;
    otmb_end_=bad_index;
    svust d_run_start_1(bad_index), d_run_end_1(bad_index);
    svust d_run_start_2(bad_index), d_run_end_2(bad_index);
    svust d_run_start_3(bad_index), d_run_end_3(bad_index);
    FindRunInRange(d_run_start_1, d_run_end_1, 0, d_run_threshhold, 0xD000, 0xDFFF);
    FindRunInRange(d_run_start_2, d_run_end_2, d_run_end_1, d_run_threshhold, 0xD000, 0xDFFF);
    FindRunInRange(d_run_start_3, d_run_end_3, d_run_end_2, d_run_threshhold, 0xD000, 0xDFFF);
    if(d_run_start_3==bad_index){
      if(d_run_start_2==bad_index){
	if(d_run_start_1!=bad_index){
	  alct_start_=d_run_start_1;
	  alct_end_=d_run_end_1;
	}
      }else{
	alct_start_=d_run_start_1;
	alct_end_=d_run_end_2;
      }
    }else{
      alct_start_=d_run_start_1;
      otmb_start_=SplitALCTandOTMB(d_run_start_2, d_run_end_2);
      alct_end_=otmb_start_;
      otmb_end_=d_run_end_3;
    }
  }

  void DataPacket::FindDCFEBData() const{
    dcfeb_start_.clear();
    dcfeb_end_.clear();
    std::vector<svust> temp_dcfeb_end(0);
    for(svust index(99); index<full_packet_.size(); ++index){
      if(full_packet_.at(index)==0x7FFF &&
	 (index==99 || full_packet_.at(index-100)==0x7FFF
	  || InRange(full_packet_.at(index-100), 0xD000, 0xDFFF)
	  || InRange(full_packet_.at(index-100), 0XA000, 0xAFFF))){
	temp_dcfeb_end.push_back(index+1);
      }
    }
    for(svsvustst dcfeb(7); dcfeb<temp_dcfeb_end.size(); dcfeb+=8){
      dcfeb_start_.push_back(temp_dcfeb_end.at(dcfeb-7)-100);
      dcfeb_end_.push_back(temp_dcfeb_end.at(dcfeb));
    }
  }

  void DataPacket::FindODMBTrailer() const{
    odmb_trailer_start_=-1;
    odmb_trailer_end_=-1;
    for(svust index(0); index+7<full_packet_.size(); ++index){
      if(InRange(full_packet_.at(index), 0xF000, 0xFFFF)
	 && InRange(full_packet_.at(index+1), 0xF000, 0xFFFF)
	 && InRange(full_packet_.at(index+2), 0xF000, 0xFFFF)
	 && InRange(full_packet_.at(index+3), 0xF000, 0xFFFF)
	 && InRange(full_packet_.at(index+4), 0xE000, 0xEFFF)
	 && InRange(full_packet_.at(index+5), 0xE000, 0xEFFF)
	 && InRange(full_packet_.at(index+6), 0xE000, 0xEFFF)
	 && InRange(full_packet_.at(index+7), 0xE000, 0xEFFF)){
	odmb_trailer_start_=index;
	odmb_trailer_end_=index+8;
      }
    }
  }

  void DataPacket::FindDDUTrailer() const{
    ddu_trailer_start_=-1;
    ddu_trailer_end_=-1;
    for(svust index(0); index+11<full_packet_.size(); ++index){
      if(full_packet_.at(index)==0x8000
	 && full_packet_.at(index+1)==0x8000
	 && full_packet_.at(index+2)==0xFFFF
	 && full_packet_.at(index+3)==0x8000){
	ddu_trailer_start_=index;
	ddu_trailer_end_=index+12;
      }
    }
  }

  void DataPacket::FindRunInRange(svust &start, svust &end, const svust &left,
				  const svust &min_length, const uint16_t &low,
				  const uint16_t &high) const{
    start=-1;
    end=-1;
    svust index(left);
    for(; index+min_length-1<full_packet_.size(); ++index){
      if(AllInRange(full_packet_, index, index+min_length, low, high)){
	start=index;
	end=full_packet_.size();
	break;
      }
    }
    for(index=index+min_length; index<full_packet_.size(); ++index){
      if(!InRange(full_packet_.at(index), low, high)){
	end=index;
	break;
      }
    }
  }

  svust DataPacket::SplitALCTandOTMB(const svust &start, const svust &end) const{
    if(start<end){
      for(svust index(start); index<end && index<full_packet_.size(); ++index){
	if(full_packet_.at(index)==0xDB0C) return index;
      }
      for(svust index(start); index<end && index<full_packet_.size(); ++index){
	if(full_packet_.at(index)==0xDB0A) return index;
      }
      return static_cast<svust>(ceil(start+0.5*(end-start)));
    }else{
      return end;
    }
  }

  void DataPacket::Print(const unsigned int &words_per_line) const{
    if(!parsed_) Parse();
    svust ddu_header_start_temp(ddu_header_start_), ddu_header_end_temp(ddu_header_end_);
    svust odmb_header_start_temp(odmb_header_start_), odmb_header_end_temp(odmb_header_end_);
    svust alct_start_temp(alct_start_), alct_end_temp(alct_end_);
    svust otmb_start_temp(otmb_start_), otmb_end_temp(otmb_end_);
    std::vector<svust> dcfeb_start_temp(dcfeb_start_), dcfeb_end_temp(dcfeb_end_);
    svust odmb_trailer_start_temp(odmb_trailer_start_), odmb_trailer_end_temp(odmb_trailer_end_);
    svust ddu_trailer_start_temp(ddu_trailer_start_), ddu_trailer_end_temp(ddu_trailer_end_);

    const svsvustst num_dcfebs(dcfeb_start_.size());
    const std::string uncat("Uncategorized");

    PutInRange(ddu_header_start_temp, ddu_header_end_temp, 0, full_packet_.size());
    PutInRange(odmb_header_start_temp, odmb_header_end_temp,
	       ddu_header_end_temp, full_packet_.size());
    PutInRange(alct_start_temp, alct_end_temp, odmb_header_end_temp, full_packet_.size());
    PutInRange(otmb_start_temp, otmb_end_temp, alct_end_temp, full_packet_.size());
    if(num_dcfebs>0){
      PutInRange(dcfeb_start_temp.at(0), dcfeb_end_temp.at(0),
		 otmb_end_temp, full_packet_.size());
      for(svsvustst dcfeb(1); dcfeb<num_dcfebs; ++dcfeb){
	PutInRange(dcfeb_start_temp.at(dcfeb), dcfeb_end_temp.at(dcfeb),
		   dcfeb_start_temp.at(dcfeb-1), full_packet_.size());
      }
      PutInRange(odmb_trailer_start_temp, odmb_trailer_end_temp,
		 dcfeb_end_temp.at(num_dcfebs-1), full_packet_.size());
    }else{
      PutInRange(odmb_trailer_start_temp, odmb_trailer_end_temp,
		 otmb_end_temp, full_packet_.size());
    }
    PutInRange(ddu_trailer_start_temp, ddu_trailer_end_temp,
	       odmb_trailer_end_temp, full_packet_.size());

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
    PrintComponent(uncat, ddu_trailer_end_temp, full_packet_.size(), words_per_line);
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
    if(!parsed_) Parse();
    for(svust word(0); word<full_packet_.size(); ++word){
      if(InRange(word, ddu_header_start_, ddu_header_end_)) continue;
      if(InRange(word, odmb_header_start_, odmb_header_end_)) continue;
      if(InRange(word, alct_start_, alct_end_)) continue;
      if(InRange(word, otmb_start_, otmb_end_)) continue;
      bool found_in_dcfeb(false);
      for(svsvustst dcfeb(0); dcfeb<dcfeb_start_.size() && !found_in_dcfeb; ++dcfeb){
	if(InRange(word, dcfeb_start_.at(dcfeb), dcfeb_end_.at(dcfeb))){
	  found_in_dcfeb=true;
	  continue;
	}
      }
      if(found_in_dcfeb) continue;
      if(InRange(word, odmb_trailer_start_, odmb_trailer_end_)) continue;
      if(InRange(word, ddu_trailer_start_, ddu_trailer_end_)) continue;
      return true;
    }
    return false;
  }

  unsigned short DataPacket::GetContainingRanges(const svust &word) const{
    unsigned short num_ranges(0);
    if(InRange(word, ddu_header_start_, ddu_header_end_-1)) ++num_ranges;
    if(InRange(word, odmb_header_start_, odmb_header_end_-1)) ++num_ranges;
    if(InRange(word, alct_start_, alct_end_-1)) ++num_ranges;
    if(InRange(word, otmb_start_, otmb_end_-1)) ++num_ranges;
    if(InRange(word, odmb_trailer_start_, odmb_trailer_end_-1)) ++num_ranges;
    if(InRange(word, ddu_trailer_start_, ddu_trailer_end_-1)) ++num_ranges;
    for(svsvustst dcfeb(0); dcfeb<dcfeb_start_.size(); ++dcfeb){
      if(InRange(word, dcfeb_start_.at(dcfeb), dcfeb_end_.at(dcfeb)-1)) ++num_ranges;
    }
    return num_ranges;
  }

  bool DataPacket::HasNoDDUHeader() const{
    if(!parsed_) Parse();
    return ddu_header_end_==ddu_header_start_;
  }

  bool DataPacket::HasNoODMBHeader() const{
    if(!parsed_) Parse();
    return odmb_header_end_==odmb_header_start_;
  }

  bool DataPacket::HasNoALCT() const{
    if(!parsed_) Parse();
    return alct_end_==alct_start_;
  }

  bool DataPacket::HasNoOTMB() const{
    if(!parsed_) Parse();
    return otmb_end_==otmb_start_;
  }

  bool DataPacket::HasNoDCFEBs() const{
    if(!parsed_) Parse();
    return dcfeb_start_.size()==0;
  }

  bool DataPacket::HasNoODMBTrailer() const{
    if(!parsed_) Parse();
    return odmb_trailer_end_==odmb_trailer_start_;
  }

  bool DataPacket::HasNoDDUTrailer() const{
    if(!parsed_) Parse();
    return ddu_trailer_end_==ddu_trailer_start_;
  }

  bool DataPacket::HasExtraALCTStartWords() const{
    if(!parsed_) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<5) return false;
    return InRange(alct.at(4), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingALCTStartWords() const{
    if(!parsed_) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<4) return true;
    return !InRange(alct.at(0), 0xD000, 0xDFFF)
      || !InRange(alct.at(1), 0xD000, 0xDFFF)
      || !InRange(alct.at(2), 0xD000, 0xDFFF)
      || !InRange(alct.at(3), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasExtraALCTEndWords() const{
    if(!parsed_) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<5) return false;
    return InRange(alct.at(alct.size()-5), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingALCTEndWords() const{
    if(!parsed_) Parse();
    const svu alct(GetALCTData());
    if(alct.size()<4) return true;
    return !InRange(alct.at(alct.size()-4), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-3), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-2), 0xD000, 0xDFFF)
      || !InRange(alct.at(alct.size()-1), 0xD000, 0xDFFF);
  }
  bool DataPacket::HasExtraOTMBStartWords() const{
    if(!parsed_) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<5) return false;
    return InRange(otmb.at(4), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingOTMBStartWords() const{
    if(!parsed_) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<4) return true;
    return !InRange(otmb.at(0), 0xD000, 0xDFFF)
      || !InRange(otmb.at(1), 0xD000, 0xDFFF)
      || !InRange(otmb.at(2), 0xD000, 0xDFFF)
      || !InRange(otmb.at(3), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasExtraOTMBEndWords() const{
    if(!parsed_) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<5) return false;
    return InRange(otmb.at(otmb.size()-5), 0xD000, 0xDFFF);
  }

  bool DataPacket::HasMissingOTMBEndWords() const{
    if(!parsed_) Parse();
    const svu otmb(GetOTMBData());
    if(otmb.size()<4) return true;
    return !InRange(otmb.at(otmb.size()-4), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-3), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-2), 0xD000, 0xDFFF)
      || !InRange(otmb.at(otmb.size()-1), 0xD000, 0xDFFF);
  }
}
