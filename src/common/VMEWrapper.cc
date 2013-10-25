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
        logger_ << "W  " << std::setw(4) << hex << address << std::setw(6) << hex << command
        		<< "  S" << std::setw(2) << dec << slot 
        		<< "    " << comment.c_str() << endl;
    }
    unsigned short int VMEWrapper::VMERead (unsigned short int address, unsigned int slot, string comment) {
    	unsigned int shiftedSlot = slot << 19;
    	int addr = (address & 0x07ffff) | shiftedSlot;
        unsigned short int data;
        char rcv[2];
        crate_->vmeController()->vme_controller(2,addr,&data,rcv);
        unsigned short int result = (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
        logger_ << "R  " << std::setw(4) << hex << address << std::setw(6) << "0"
        	    << "  S" << std::setw(2) << dec<< slot << std::setw(16) << hex << result 
        	    << "    " << comment.c_str() << endl;
        return result;
    }
  }
}
