#ifndef H_DATAPACKET
#define H_DATAPACKET

#include <vector>
#include <string>
#include "stdint.h"

namespace Packet{
  typedef std::vector<uint16_t> svu;
  typedef svu::size_type svust;
  typedef std::vector<svu>::size_type svsvust;
  typedef std::vector<svust>::size_type svsvustst;

  template<typename t1, typename t2, typename t3> bool InRange(const t1 &x, const t2 &low,
							       const t3 &high){
    return x>=low && x<=high;
  }
  bool AllInRange(const svu &, const svust &, const svust &,
		  const uint16_t &, const uint16_t &);
  void PrintBuffer(const svu &, const unsigned int &);
  void PutInRange(svust &, svust &, const svust &, const svust &);

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

    void Print(const unsigned int &) const;

    enum ErrorType{
      good			=0x0000,
      no_dcfebs			=0x0001,
      no_ddu_header		=0x0002,
      no_ddu_trailer		=0x0004,
      unused_words		=0x0008,
      extra_alct_start		=0x0010,
      missing_alct_start	=0x0020,
      extra_alct_end		=0x0040,
      missing_alct_end		=0x0080,
      extra_otmb_start		=0x0100,
      missing_otmb_start	=0x0200,
      extra_otmb_end		=0x0400,
      missing_otmb_end		=0x0800,
      no_alct			=0x1000,
      no_otmb			=0x2000,
      no_odmb_header		=0x4000,
      no_odmb_trailer		=0x8000
    };
    ErrorType GetPacketType() const;

  private:
    svu full_packet;
    mutable svust ddu_header_start, ddu_header_end;
    mutable svust odmb_header_start, odmb_header_end;
    mutable svust alct_start, alct_end;
    mutable svust otmb_start, otmb_end;
    mutable std::vector<svust> dcfeb_start, dcfeb_end;
    mutable svust odmb_trailer_start, odmb_trailer_end;
    mutable svust ddu_trailer_start, ddu_trailer_end;
    mutable bool parsed;

    void Parse() const;

    void FindDDUHeader() const;
    void FindODMBHeader() const;
    void FindALCTandOTMBData() const;
    void FindDCFEBData() const;
    void FindODMBTrailer() const;
    void FindDDUTrailer() const;

    svu GetComponent(const svust &, const svust &) const;
    void PrintComponent(const std::string &, const svust &,
			const svust &, const unsigned int &) const;

    void FindRunInRange(svust &, svust &, const svust &, const svust &,
			const uint16_t &, const uint16_t &) const;
    svust SplitALCTandOTMB(const svust &, const svust &) const;

    unsigned short GetContainingRanges(const svust &) const;

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
