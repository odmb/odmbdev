#ifndef H_DATAPACKET
#define H_DATAPACKET

#include <vector>
#include <set>
#include <string>
#include <utility>
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

  template<typename t1, typename t2, typename t3> bool InRange(const t1& x,
                                                               const t2& low,
                                                               const t3& high){
    return x>=low && x<=high;
  }

  template<typename T>
  std::set<T> GetUniques(const std::vector<T>& vec){
    std::set<T> set;
    for(unsigned i(0); i<vec.size(); ++i){
      set.insert(vec.at(i));
    }
    return set;
  }

  bool AllInRange(const svu&, const unsigned, const unsigned,
                  const uint16_t, const uint16_t);
  void PutInRange(unsigned&, unsigned&, const unsigned, const unsigned);
  bool GetBit(const unsigned, const unsigned);
  
  void PrintWithStars(const std::string&, const unsigned);

  class DataPacket{
  public:
    DataPacket();
    DataPacket(const svu&);

    void SetData(const svu&);
    svu GetData() const;

    svu GetDDUHeader() const;
    svu GetODMBHeader(const unsigned) const;
    svu GetALCTData(const unsigned) const;
    svu GetOTMBData(const unsigned) const;
    std::vector<svu> GetDCFEBData(const unsigned) const;
    svu GetODMBTrailer(const unsigned) const;
    svu GetDDUTrailer() const;

    void Print(const unsigned, const unsigned, const uint_fast64_t, const bool=false) const;

    std::vector<bool> GetDMBType() const;

    enum ErrorType{
      kGood               = 0x0000000000u,
      kDDUStatus          = 0x00FFFFFFFFu,
      kDCFEBL1AMismatch   = 0x0100000000u,
      kOTMBL1AMismatch    = 0x0200000000u,
      kALCTL1AMismatch    = 0x0400000000u,
      kODMBL1AMismatch    = 0x0800000000u,
      kUncategorizedWords = 0x1000000000u
    };
    ErrorType GetPacketType() const;

    enum ComponentType{
      kDDU, kODMB, kALCT, kOTMB, kDCFEB
    };

  private:
    typedef std::pair<ComponentType, uint_fast32_t> l1a_t;

    svu full_packet_;
    mutable std::vector<bool> colorize_;
    mutable unsigned ddu_header_start_, ddu_header_end_;
    mutable std::vector<unsigned> odmb_header_start_, odmb_header_end_;
    mutable std::vector<unsigned> alct_start_, alct_end_;
    mutable std::vector<unsigned> otmb_start_, otmb_end_;
    mutable std::vector<std::vector<unsigned> > dcfeb_start_, dcfeb_end_;
    mutable std::vector<unsigned> odmb_trailer_start_, odmb_trailer_end_;
    mutable unsigned ddu_trailer_start_, ddu_trailer_end_;
    mutable std::vector<l1a_t> dcfeb_l1as_;
    mutable bool odmb_l1a_mismatch_, alct_l1a_mismatch_, otmb_l1a_mismatch_, dcfeb_l1a_mismatch_;
    mutable bool parsed_, checked_l1as_;

    void Parse() const;

    void FindDDUHeader() const;
    
    void FindAllODMBHeadersAndTrailers() const;
    void FixNumberOfODMBPackets() const;
    bool FindODMBHeader(const unsigned, const unsigned) const;
    bool FindODMBTrailer(const unsigned, const unsigned) const;
    void FindALCTandOTMBData(const unsigned) const;
    void FindDCFEBData(const unsigned) const;
    void FindDDUTrailer() const;

    bool IsDCFEB(const unsigned, const unsigned) const;
    bool IsALCT(const unsigned, const unsigned) const;

    svu GetComponent(const unsigned, const unsigned) const;
    void PrintComponent(const std::string&, const unsigned,
                        const unsigned, const unsigned,
                        const bool=false) const;
    void PrintODMB(const std::string& uncat,
                   const unsigned odmb,
                   const unsigned words_per_line,
                   const uint_fast64_t kill_mask,
                   const unsigned text_mode) const;
    void PrintHeader(const std::vector<std::string>&, const unsigned words_per_line) const;

    bool FindRunInRange(unsigned&, unsigned&, const unsigned, const unsigned,
                        const unsigned, const uint16_t, const uint16_t) const;
    unsigned SplitALCTandOTMB(const unsigned, const unsigned) const;

    void PrintBuffer(const svu&, const unsigned, const unsigned,
                     const bool=false) const;

    unsigned short GetContainingRanges(const unsigned) const;

    bool HasL1AMismatch() const;

    bool HasODMBL1AMismatch() const;
    bool HasALCTL1AMismatch() const;
    bool HasOTMBL1AMismatch() const;
    bool HasDCFEBL1AMismatch() const;

    std::vector<unsigned> GetValidDCFEBs(const unsigned) const;
    std::string GetDCFEBText(const unsigned) const;

    std::vector<l1a_t> GetL1As() const;
    std::string GetL1AText(const bool=false) const;
    std::string GetODMBText() const;

    bool HasUncategorizedWords() const;

    uint_fast32_t GetDDUStatus() const;
  };
}

#endif
