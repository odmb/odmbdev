#ifndef H_UNPACKER
#define H_UNPACKER

#include <vector>
#include <string>
#include "utils.hpp"

namespace Packet{
  class Unpacker{
  public:
    Unpacker();
    Unpacker(const std::vector<dcfeb_data>& data, const uint_fast32_t l1a_in, const unsigned dcfeb_in);
    void SetData(const std::vector<dcfeb_data>& data, const uint_fast32_t l1a_in, const unsigned dcfeb_in);

    unsigned GetCutoff() const;
    float GetMuonPeak() const;
    float GetMuonAverage() const;
    float GetPedestalAverage() const;
    float GetAverageLayer() const;
    float GetAverageStrip() const;
    float GetAverageTimeBin() const;
    float GetMuonPedestalRatio() const;

    const uint_fast32_t& l1a() const{return l1a_;}
    uint_fast32_t& l1a(){return l1a_;}

    const unsigned& dcfeb() const{return dcfeb_;}
    unsigned& dcfeb(){return dcfeb_;}

    void MakePlot(const std::string& file) const;
    void PrintData() const;

  private:
    mutable std::vector<dcfeb_data> data_;
    uint_fast32_t l1a_;
    unsigned dcfeb_;
    mutable unsigned cutoff_;
    mutable bool cutoff_cached_;

    void CalcCutoff() const;
    bool IsNeighbor(const dcfeb_data& a, const dcfeb_data&b) const;
  }; 
}
#endif
