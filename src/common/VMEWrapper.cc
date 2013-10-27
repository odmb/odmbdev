#include "emu/odmbdev/VMEWrapper.h" 
#include "emu/odmbdev/Manager.h"
#include "emu/odmbdev/utils.h"
#include <iomanip>
 
namespace emu { namespace odmbdev {
    VMEWrapper::VMEWrapper(Crate * crate) : 
    crate_(crate),
    port_number_(Manager::getPortNumber()),
    logFile_(GetLogFileName(port_number_))
    {
      logger_.open(logFile_.c_str(), std::ofstream::out | std::ofstream::app );
    }
    
    VMEWrapper::VMEWrapper( const VMEWrapper& other ) : 
    crate_(other.crate_),
    port_number_(other.port_number_),
    logFile_(other.logFile_)
    {
      // copy constructor
      logger_.open(logFile_.c_str(), std::ofstream::out | std::ofstream::app );
    }
    
    void VMEWrapper::VMEWrite (unsigned short int address, unsigned short int command, unsigned int slot, string comment) {
    	unsigned int shiftedSlot = slot << 19;
    	int addr = (address & 0x07ffff) | shiftedSlot;
        char rcv[2];
        crate_->vmeController()->vme_controller(3,addr,&command,rcv);
        // Get timestamp
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char timestamp[30];
        strftime (timestamp,30,"%D %X",timeinfo);
        // Write to log
        logger_ << "W" << std::setw(7) << FixLength(address,4,true)
        		<< std::setw(7) << FixLength(command,4,true)
        		<< std::setw(15) << dec << slot 
        		<< "   " << timestamp
        	    << "    " << comment.c_str() << endl;
    }
    unsigned short int VMEWrapper::VMERead (unsigned short int address, unsigned int slot, string comment) {
    	unsigned int shiftedSlot = slot << 19;
    	int addr = (address & 0x07ffff) | shiftedSlot;
        unsigned short int data;
        char rcv[2];
        crate_->vmeController()->vme_controller(2,addr,&data,rcv);
        unsigned short int result = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
        
        // Get timestamp
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char timestamp[30];
        strftime (timestamp,30,"%D %X",timeinfo);
        // Get formatting
        bool readHex = true;
        if((address >= 0x321C && address <= 0x337C) || (address >= 0x33FC && address < 0x35AC)  || 
	       (address > 0x35DC && address <= 0x3FFF) || address == 0x500C || address == 0x510C ||
	       address == 0x520C || address == 0x530C || address == 0x540C   || address == 0x550C || address == 0x560C 
           || address == 0x8004 ||  (address == 0x5000 && result < 0x1000)) {
	      		readHex = false;
	      }
	    // Write to log
        logger_ << "R" << std::setw(7) << FixLength(address,4,true);
        if (readHex) logger_<< std::setw(14) << FixLength(result,4,true);
        else logger_<< std::setw(12) << dec << result << "_d";
        logger_ << std::setw(8) << dec << slot 
        	    << "   " << timestamp
        	    << "    " << comment.c_str() << endl;
        return result;
    }
  }
}
