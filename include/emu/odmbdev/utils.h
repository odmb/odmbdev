#ifndef __emu_odmbdev_utils_h__
#define __emu_odmbdev_utils_h__

#include "xgi/Input.h"
#include <string>
#include <sstream>
#include <time.h>

#include "emu/odmbdev/VMEWrapper.h"

namespace emu{ 
  namespace odmbdev{

    std::string fix_width(const double number, const std::streamsize width);

    unsigned int binaryStringToUInt(const std::string& s);
    std::string withoutSpecialChars(const std::string& s);
    
    int getFormValueInt(const std::string& form_element, xgi::Input* in);
    int getFormValueIntHex(const std::string& form_element, xgi::Input* in);
    float getFormValueFloat(const std::string& form_element, xgi::Input* in);
    std::string getFormValueString(const std::string& form_element, xgi::Input* in);

    template <typename T>
    std::string numberToString(T number)
    {
      std::stringstream convert;
      convert << number;
      return convert.str();
    }
    
    std::string GetLogFileName(unsigned int port_number);
    std::string FixLength(unsigned int Number, unsigned int Length, bool isHex);
    void UpdateLog(VMEWrapper* vme_wrapper, unsigned int slot, std::ostringstream& out_local);
    void JustifyHdr(string &hdr);

    namespace Packet{
      typedef std::pair<uint_least16_t, std::pair<uint_least8_t, std::pair<uint_least8_t, uint_least8_t> > > dcfeb_data;
    }

    template<typename T>
      double Sum(T begin, T end){
      double sum(0.0);
      volatile double correction(0.0);
      for(; begin!=end; ++begin){
	const double corrected_val(*begin-correction);
	const double temp_sum(sum+corrected_val);
	correction=(temp_sum-sum)-corrected_val;
	sum=temp_sum;
      }
      return sum;
    }
    
    template<typename T>
      double Mean(T begin, T end){
      return Sum(begin, end)/static_cast<double>(std::distance(begin,end));
    }
  }
}

#endif //__emu_odmbdev_utils_h__

