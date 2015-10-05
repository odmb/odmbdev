#ifndef H_DATAPACKET
#define H_DATAPACKET

#include <vector>
#include <set>
#include <string>
#include <utility>
#include <stdint.h>
#include "utils.hpp"

namespace Packet{
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
      kDDUStatus          = 0xFFFFFFFF00u,
      kDCFEBBXNMismatch   = 0x0000000001u,
      kOTMBBXNMismatch    = 0x0000000002u,
      kALCTBXNMismatch    = 0x0000000004u,
      kODMBBXNMismatch    = 0x0000000008u,
      kDCFEBL1AMismatch   = 0x0000000010u,
      kOTMBL1AMismatch    = 0x0000000020u,
      kALCTL1AMismatch    = 0x0000000040u,
      kODMBL1AMismatch    = 0x0000000080u,
      kEmptyODMB          = 0x0000000100u,
      kUncategorizedWords = 0x0000000200u
    };
    ErrorType GetPacketType() const;

    enum ComponentType{
      kDDU, kODMB, kALCT, kOTMB, kDCFEB
    };

    std::vector<std::pair<unsigned, std::vector<dcfeb_data> > > GetValidDCFEBData() const;
    uint_fast32_t GetL1A() const;
    uint_fast32_t GetBXN() const;

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
    mutable std::vector<std::pair<unsigned, std::vector<dcfeb_data> > > dcfeb_data_;
    mutable bool odmb_bxn_mismatch_, alct_bxn_mismatch_, otmb_bxn_mismatch_, dcfeb_bxn_mismatch_;
    mutable bool odmb_l1a_mismatch_, alct_l1a_mismatch_, otmb_l1a_mismatch_, dcfeb_l1a_mismatch_;
    mutable bool parsed_, unpacked_, checked_l1as_, checked_bxns_;

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
    bool HasBXNMismatch() const;

    bool HasODMBL1AMismatch() const;
    bool HasALCTL1AMismatch() const;
    bool HasOTMBL1AMismatch() const;
    bool HasDCFEBL1AMismatch() const;

    bool HasODMBBXNMismatch() const;
    bool HasALCTBXNMismatch() const;
    bool HasOTMBBXNMismatch() const;
    bool HasDCFEBBXNMismatch() const;

    std::vector<unsigned> GetValidDCFEBs(const unsigned) const;
    std::string GetDCFEBText(const unsigned) const;

    std::vector<l1a_t> GetL1As() const;
    std::vector<l1a_t> GetBXNs() const;
    std::string GetL1AText(const bool=false) const;
    std::string GetBXNText(const bool=false) const;
    std::string GetODMBText() const;

    bool HasUncategorizedWords() const;

    uint_fast32_t GetDDUStatus() const;

    bool HasEmptyODMB() const;

    void Unpack() const;
  };
}

#endif
