#ifndef H_DATAPACKET
#define H_DATAPACKET

#include <vector>
#include <string>
#include "stdint.h"

namespace Packet{
  namespace io{
    const std::string bold("\033[1m");
    const std::string fg_black("\033[30m");
    const std::string fg_red("\033[31m");
    const std::string fg_green("\033[32m");
    const std::string fg_yellow("\033[33m");
    const std::string fg_blue("\033[34m");
    const std::string fg_magenta("\033[35m");
    const std::string fg_cyan("\033[36m");
    const std::string fg_white("\033[37m");
    const std::string bg_black("\033[40m");
    const std::string bg_red("\033[41m");
    const std::string bg_green("\033[42m");
    const std::string bg_yellow("\033[43m");
    const std::string bg_blue("\033[44m");
    const std::string bg_magenta("\033[45m");
    const std::string bg_cyan("\033[46m");
    const std::string bg_white("\033[47m");
    const std::string normal("\033[0m");
  }

  typedef std::vector<uint16_t> svu;

  template<typename t1, typename t2, typename t3> bool InRange(const t1 &x,
							       const t2 &low,
							       const t3 &high){
    return x>=low && x<=high;
  }
  bool AllInRange(const svu &, const unsigned &, const unsigned &,
		  const uint16_t &, const uint16_t &);
  void PutInRange(unsigned &, unsigned &, const unsigned &, const unsigned &);

  class DataPacket{
  public:
    DataPacket();
    DataPacket(const svu &);

    void SetData(const svu &);
    svu GetData() const;

    svu GetDDUHeader() const;
    svu GetODMBHeader() const;
    svu GetALCTData() const;
    svu GetOTMBData() const;
    std::vector<svu> GetDCFEBData() const;
    svu GetODMBTrailer() const;
    svu GetDDUTrailer() const;

    void Print(const unsigned int &, const bool=false) const;

    enum ErrorType{
      kGood		= 0x0000,
      kNoDCFEBs		= 0x0001,
      kNoDDUHeader	= 0x0002,
      kNoDDUTrailer	= 0x0004,
      kUnusedWords	= 0x0008,
      kExtraALCTStart	= 0x0010,
      kMissingALCTStart	= 0x0020,
      kExtraALCTEnd	= 0x0040,
      kMissingALCTEnd	= 0x0080,
      kExtraOTMBStart	= 0x0100,
      kMissingOTMBStart	= 0x0200,
      kExtraOTMBEnd	= 0x0400,
      kMissingOTMBEnd	= 0x0800,
      kNoALCT		= 0x1000,
      kNoOTMB		= 0x2000,
      kNoODMBHeader	= 0x4000,
      kNoODMBTrailer	= 0x8000
    };
    ErrorType GetPacketType() const;

  private:
    svu full_packet_;
    mutable std::vector<bool> colorize_;
    mutable unsigned ddu_header_start_, ddu_header_end_;
    mutable unsigned odmb_header_start_, odmb_header_end_;
    mutable unsigned alct_start_, alct_end_;
    mutable unsigned otmb_start_, otmb_end_;
    mutable std::vector<unsigned> dcfeb_start_, dcfeb_end_;
    mutable unsigned odmb_trailer_start_, odmb_trailer_end_;
    mutable unsigned ddu_trailer_start_, ddu_trailer_end_;
    mutable bool parsed_;

    void Parse() const;

    void FindDDUHeader() const;
    void FindODMBHeader() const;
    void FindALCTandOTMBData() const;
    void FindDCFEBData() const;
    void FindODMBTrailer() const;
    void FindDDUTrailer() const;

    svu GetComponent(const unsigned &, const unsigned &) const;
    void PrintComponent(const std::string &, const unsigned &,
			const unsigned &, const unsigned int &,
			const bool=false) const;

    void FindRunInRange(unsigned &, unsigned &, const unsigned &, const unsigned &,
			const uint16_t &, const uint16_t &) const;
    unsigned SplitALCTandOTMB(const unsigned &, const unsigned &) const;

    void PrintBuffer(const svu &, const unsigned int &, const unsigned &,
		     const bool=false) const;

    unsigned short GetContainingRanges(const unsigned &) const;

    bool HasUnusedWords() const;
    bool HasNoDDUHeader() const;
    bool HasNoODMBHeader() const;
    bool HasNoALCT() const;
    bool HasNoOTMB() const;
    bool HasNoDCFEBs() const;
    bool HasNoODMBTrailer() const;
    bool HasNoDDUTrailer() const;
    bool HasExtraALCTStartWords() const;
    bool HasMissingALCTStartWords() const;
    bool HasExtraALCTEndWords() const;
    bool HasMissingALCTEndWords() const;
    bool HasExtraOTMBStartWords() const;
    bool HasMissingOTMBStartWords() const;
    bool HasExtraOTMBEndWords() const;
    bool HasMissingOTMBEndWords() const;
  };
}

#endif
