#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unistd.h>

typedef std::vector<uint16_t>::size_type svust;
typedef std::vector<uint16_t>::iterator svuit;
typedef std::vector<uint16_t> svu;

void UpdateLastFewWords(const uint16_t x, svu &buf){
  if(buf.size()!=0){
    for(svust index(buf.size()-1); index!=0; --index){
      buf.at(index)=buf.at(index-1);
    }
    buf.at(0)=x;
  }
}

bool FindDDUHeader(std::ifstream &ifs, svu &header){
  header.clear();
  header.resize(12);
  bool found_match(false);
  uint16_t word(0);
  while(!found_match && ifs.read((char*)(&word), sizeof(word))){
    UpdateLastFewWords(word, header);
    unsigned int matches_count(0);
    if(header.at(4)==0x8000) ++matches_count;
    if(header.at(5)==0x0001) ++matches_count;
    if(header.at(6)==0x8000) ++matches_count;
    if(matches_count==3) found_match=true;
  }
  std::reverse(header.begin(), header.end());
  return found_match;
}

void PrintBuffer(const svu &buffer, const unsigned int words_per_line){
  for(unsigned int index(0); index<buffer.size(); ++index){
    if(index && !(index%words_per_line)) std::cout << std::endl;
    std::cout << std::hex << std::setw(4) << std::setfill('0') << buffer.at(index) << " ";
  }
  std::cout << std::endl;
}

bool FindODMBHeaderStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<words_in_event.size(); ++index){
    unsigned int ninewords_count(0), awords_count(0);
    //This is horribly inefficient. Fix this later...
    for(svust position(index); position<words_in_event.size() && position<index+8; ++position){
      if(words_in_event.at(position)>=0x9000 && words_in_event.at(position)<=0x9FFF){
	++ninewords_count;
      }
      if(words_in_event.at(position)>=0xA000 && words_in_event.at(position)<=0xAFFF){
	++awords_count;
      }
    }
    if(ninewords_count+awords_count>=8){
      found=true;
      start=index;
    }
  }
  return found;
}

bool FindALCTStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<words_in_event.size(); ++index){
    unsigned int dwords_count(0);
    //This is horribly inefficient. Fix this later...
    for(svust position(index); position<words_in_event.size() && position<index+4; ++position){
      if(words_in_event.at(position)>=0xD000 && words_in_event.at(position)<=0xDFFF){
	++dwords_count;
      }
    }
    if(dwords_count==4){
      found=true;
      start=index;
    }
  }
  return found;
}

bool FindTMBStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<words_in_event.size(); ++index){
    unsigned int dwords_count(0);
    //This is horribly inefficient. Fix this later...
    for(svust position(index); position<words_in_event.size() && position<index+8; ++position){
      if(words_in_event.at(position)>=0xD000 && words_in_event.at(position)<=0xDFFF){
	++dwords_count;
      }
    }
    if(dwords_count>=8){
      found=true;
      start=index+4;
    }
  }
  return found;
}

bool FindDCFEBStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<words_in_event.size(); ++index){
    unsigned int dwords_count(0);
    //This is horribly inefficient. Fix this later...
    for(svust position(index); position<words_in_event.size() && position<index+4; ++position){
      if(words_in_event.at(position)>=0xD000 && words_in_event.at(position)<=0xDFFF){
	++dwords_count;
      }
    }
    if(dwords_count>=3){
      found=true;
      start=index+4;
    }
  }
  return found;
}

bool FindODMBTrailerStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<words_in_event.size(); ++index){
    unsigned int ewords_count(0), fwords_count(0);
    //This is horribly inefficient. Fix this later...
    for(svust position(index); position<words_in_event.size() && position<index+8; ++position){
      if(words_in_event.at(position)>=0xE000 && words_in_event.at(position)<=0xEFFF){
	++ewords_count;
      }
      if(words_in_event.at(position)>=0xF000){
	++fwords_count;
      }
    }
    if(ewords_count+fwords_count==8){
      found=true;
      start=index;
    }
  }
  return found;
}

bool FindDDUTrailerStart(const svu &words_in_event, const svust &min_start, svust &start){
  bool found(false);
  for(svust index(min_start); !found && index<=words_in_event.size()-12; ++index){
    unsigned int matches_count(0);
    if(words_in_event.at(index) == 0x8000) ++matches_count;
    if(words_in_event.at(index+1) == 0x8000) ++matches_count;
    if(words_in_event.at(index+2) == 0xFFFF) ++matches_count;
    if(words_in_event.at(index+3) == 0x8000) ++matches_count;
    if(matches_count==4){
      found=true;
      start=index;
    }
  }
  return found;
}

void GetPart(svu &part, const svu &whole, const svust &start, const svust &end){
  part.clear();
  for(svust index(start); index<end; ++index){
    part.push_back(whole.at(index));
  }
}

bool GetODMBHeader(std::ifstream &ifs, svu &odmb_header){
  odmb_header.resize(8);
  bool found_header(false);
  uint16_t word;
  for(unsigned int word_num(8);
      word_num>0 && ifs.read((char*)(&word), sizeof(word));
      --word_num){
    odmb_header.at(word_num-1)=word;
  }
  
  unsigned int nineword_count(0), aword_count(0);
  for(svuit it(odmb_header.begin()); it!=odmb_header.end(); ++it){
    if(*it>=0x9000 && *it<=0x9FFF) ++nineword_count;
    if(*it>=0xA000 && *it<=0xAFFF) ++aword_count;
  }
  if(odmb_header.size()==8 && nineword_count+aword_count==8){
    found_header=true;
  }
  return found_header;
}

bool GetWordsInEvent(std::ifstream &ifs, svu &words_in_event){
  uint16_t word(0);
  bool found_ddu_trailer(false);
  while(!found_ddu_trailer && ifs.read((char*)(&word), sizeof(word))){
    words_in_event.push_back(word);
    svust junk(0);
    found_ddu_trailer=FindDDUTrailerStart(words_in_event, 0, junk);
  }
  return found_ddu_trailer;
}

int main(int argc, char *argv[]){
  unsigned int words_per_line(24);
  std::string filename("");
  unsigned int entry_to_find(1);

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

  std::ifstream ifs(filename.c_str(),std::ifstream::in | std::ifstream::binary);
  if(ifs.is_open()){
    svu words_in_event(0);
    unsigned int entry(0);
    for(entry=0; entry<entry_to_find && FindDDUHeader(ifs, words_in_event); ++entry){
    }
    if(entry==entry_to_find){
      bool read_success(GetWordsInEvent(ifs, words_in_event));
      svust odmb_header_start(0);
      svust alct_start(0);
      svust tmb_start(0);
      svust dcfeb_start(0);
      svust odmb_trailer_start(0);
      svust ddu_trailer_start(0);
	
      read_success &= FindODMBHeaderStart(words_in_event,
					  0,
					  odmb_header_start);
      read_success &= FindALCTStart(words_in_event,
				    odmb_header_start+1,
				    alct_start);
      read_success &= FindTMBStart(words_in_event,
				   alct_start+1,
				   tmb_start);
      read_success &= FindDCFEBStart(words_in_event,
				     tmb_start+1,
				     dcfeb_start);
      read_success &= FindODMBTrailerStart(words_in_event,
					   dcfeb_start+1,
					   odmb_trailer_start);
      read_success &= FindDDUTrailerStart(words_in_event,
					  odmb_trailer_start+1,
					  ddu_trailer_start);

      if(!read_success){
	std::cout << "Misread Event:" << std::endl;
	PrintBuffer(words_in_event, words_per_line);
      }else{
	svu ddu_header(0), odmb_header(0);
	svu alct(0), tmb(0);
	svu dcfeb(0);
	svu odmb_trailer(0), ddu_trailer(0);
	GetPart(ddu_header, words_in_event, 0, odmb_header_start);
	GetPart(odmb_header, words_in_event, odmb_header_start, alct_start);
	GetPart(alct, words_in_event, alct_start, tmb_start);
	GetPart(tmb, words_in_event, tmb_start, dcfeb_start);
	GetPart(dcfeb, words_in_event, dcfeb_start, odmb_trailer_start);
	GetPart(odmb_trailer, words_in_event, odmb_trailer_start, ddu_trailer_start);
	GetPart(ddu_trailer, words_in_event, ddu_trailer_start, words_in_event.size());

	std::cout << "DDU Header:" << std::endl;
	PrintBuffer(ddu_header, words_per_line);
	std::cout << std::endl << "ODMB Header:" << std::endl;
	PrintBuffer(odmb_header, words_per_line);
	std::cout << std::endl << "ALCT:" << std::endl;
	PrintBuffer(alct, words_per_line);
	std::cout << std::endl << "TMB:" << std::endl;
	PrintBuffer(tmb, words_per_line);
	std::cout << std::endl << "DCFEB:" << std::endl;
	PrintBuffer(dcfeb, words_per_line);
	std::cout << std::endl << "ODMB Trailer:" << std::endl;
	PrintBuffer(odmb_trailer, words_per_line);
	std::cout << std::endl << "DDU Trailer:" << std::endl;
	PrintBuffer(ddu_trailer, words_per_line);
      }
    }
    ifs.close();
  }
}
