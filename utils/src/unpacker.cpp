#include "unpacker.hpp"
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "TCanvas.h"
#include "TH2D.h"

namespace Packet{
  Unpacker::Unpacker():
    data_(0),
    cutoff_(0),
    cutoff_cached_(false){
  }

  Unpacker::Unpacker(const std::vector<dcfeb_data>& data, const uint_fast32_t l1a_in, const unsigned dcfeb_in):
    data_(data),
    l1a_(l1a_in),
    dcfeb_(dcfeb_in),
    cutoff_(0),
    cutoff_cached_(false){
  }

  void Unpacker::SetData(const std::vector<dcfeb_data>& data, const uint_fast32_t l1a_in, const unsigned dcfeb_in){
    data_=data;
    l1a_=l1a_in;
    dcfeb_=dcfeb_in;
    cutoff_cached_=false;
  }

  unsigned Unpacker::GetCutoff() const{
    CalcCutoff();
    return cutoff_;
  }

  void Unpacker::CalcCutoff() const{
    if(!cutoff_cached_){
      std::sort(data_.begin(), data_.end(), std::greater<dcfeb_data>());
      std::vector<uint_fast16_t> plain_data(data_.size());
      for(unsigned i(0); i<data_.size(); ++i){
        plain_data.at(i)=data_.at(i).first;
      }
  
      for(cutoff_=0;
          plain_data.at(cutoff_)-Mean(plain_data.begin()+cutoff_, plain_data.end())>3.21552019394337*sqrt(Variance(plain_data.begin()+cutoff_, plain_data.end()));
          ++cutoff_);
      cutoff_cached_=true;
    }
  }

  float Unpacker::GetMuonPeak() const{
    CalcCutoff();
    return data_.size()?data_.at(0).first:0.0;
  }

  float Unpacker::GetMuonAverage() const{
    CalcCutoff();
    if(cutoff_>0){
      std::vector<uint_least16_t> plain_data(data_.size());
      for(unsigned i(0); i<data_.size(); ++i){
        plain_data.at(i)=data_.at(i).first;
      }
      return static_cast<float>(std::accumulate(plain_data.begin(), plain_data.begin()+cutoff_, 0.0))/cutoff_;
    }else{
      return 0.0;
    }
  }

  float Unpacker::GetPedestalAverage() const{
    CalcCutoff();
    if(data_.size()-cutoff_>0){
      std::vector<uint_least16_t> plain_data(data_.size());
      for(unsigned i(0); i<data_.size(); ++i){
        plain_data.at(i)=data_.at(i).first;
      }
      return static_cast<float>(std::accumulate(plain_data.begin()+cutoff_, plain_data.end(), 0.0))/(data_.size()-cutoff_);
    }else{
      return 0.0;
    }
  }

  float Unpacker::GetMuonPedestalRatio() const{
    CalcCutoff();
    return GetMuonAverage()/GetPedestalAverage();
  }

  bool Unpacker::IsNeighbor(const dcfeb_data& a, const dcfeb_data& b) const{
    const unsigned short dt(std::abs(a.second.first-b.second.first));
    const unsigned short ds(std::abs(a.second.second.first-b.second.second.first));
    const unsigned short dl(std::abs(a.second.second.second-b.second.second.second));
    return (dt==0 && ds<=1 && dl<=1) || (dt==1 && ds==0 && dl==0);
  }

  float Unpacker::GetAverageLayer() const{
    CalcCutoff();
    const float pedestal_avg(GetPedestalAverage());

    float layer_total(0.0), weight_total(0.0);
    for(unsigned i(0); i<cutoff_; ++i){
      const float delta(data_.at(i).first-pedestal_avg);
      layer_total+=data_.at(i).second.second.second*delta;
      weight_total+=delta;
    }
    if(weight_total>0.0){
      return 1.0+layer_total/weight_total;
    }else{
      return 0.0;
    }
  }

  float Unpacker::GetAverageStrip() const{
    CalcCutoff();
    const float pedestal_avg(GetPedestalAverage());
    
    float strip_total(0.0), weight_total(0.0);
    for(unsigned i(0); i<cutoff_; ++i){
      const float delta(data_.at(i).first-pedestal_avg);
      strip_total+=data_.at(i).second.second.first*delta;
      weight_total+=delta;
    }
    if(weight_total>0.0){
      return 1.0+strip_total/weight_total;
    }else{
      return 0.0;
    }
  }

  float Unpacker::GetAverageTimeBin() const{
    CalcCutoff();
    const float pedestal_avg(GetPedestalAverage());

    float time_total(0.0), weight_total(0.0);
    for(unsigned i(0); i<cutoff_; ++i){
      const float delta(data_.at(i).first-pedestal_avg);
      time_total+=data_.at(i).second.first*delta;
      weight_total+=delta;
    }
    if(weight_total>0.0){
      return 1.0+time_total/weight_total;
    }else{
      return 0.0;
    }
  }

  void Unpacker::MakePlot(const std::string& file) const{
    TCanvas canvas;
    canvas.Divide(4,2);
    TH2D h1("h1","Time Sample 1;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h2("h2","Time Sample 2;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h3("h3","Time Sample 3;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h4("h4","Time Sample 4;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h5("h5","Time Sample 5;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h6("h6","Time Sample 6;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h7("h7","Time Sample 7;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    TH2D h8("h8","Time Sample 8;Strip;Layer", 16, 0.5, 16.5, 6, 0.5, 6.5);
    double min_val(static_cast<double>(0xFFFu));
    double max_val(0.0);
    for(unsigned i(0); i<data_.size(); ++i){
      const double value(data_.at(i).first);
      const int time(data_.at(i).second.first+1);
      const int strip(data_.at(i).second.second.first+1);
      const int layer(data_.at(i).second.second.second+1);
      TH2D* h(NULL);
      switch(time){
      case 1: h=&h1; break;
      case 2: h=&h2; break;
      case 3: h=&h3; break;
      case 4: h=&h4; break;
      case 5: h=&h5; break;
      case 6: h=&h6; break;
      case 7: h=&h7; break;
      case 8: h=&h8; break;
      default: h=NULL; break;
      }
      h->SetBinContent(strip, layer, value);
      if(value<min_val) min_val=value;
      if(value>max_val) max_val=value;
    }
    
    for(unsigned time(1); time<=8; ++time){
      TH2D* h(NULL);
      switch(time){
      case 1: h=&h1; break;
      case 2: h=&h2; break;
      case 3: h=&h3; break;
      case 4: h=&h4; break;
      case 5: h=&h5; break;
      case 6: h=&h6; break;
      case 7: h=&h7; break;
      case 8: h=&h8; break;
      default: h=NULL; break;
      }
      canvas.cd(time);
      h->SetMinimum(min_val);
      h->SetMaximum(max_val);
      h->SetStats(0);
      h->Draw("COLZ");      
    }
    canvas.cd(0);
    canvas.Print(file.c_str());
  }

  void Unpacker::PrintData() const{
    CalcCutoff();
    for(unsigned i(0); i<data_.size() && i<cutoff_; ++i){
      std::cout << ' ' << std::setw(8) << i
                << ' ' << std::setw(8) << std::hex << static_cast<unsigned>(data_.at(i).first) << std::dec
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).first)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.first)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.second.first)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.second.second)
                << std::endl;
    }
    for(unsigned i(0); i<54; ++i) std::cout << '-';
    std::cout << std::endl;
    for(unsigned i(cutoff_); i<data_.size(); ++i){
      std::cout << ' ' << std::setw(8) << i
                << ' ' << std::setw(8) << std::hex << static_cast<unsigned>(data_.at(i).first) << std::dec
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).first)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.first+1)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.second.first+1)
                << ' ' << std::setw(8) << static_cast<unsigned>(data_.at(i).second.second.second+1)
                << std::endl;
    }
  }
}
