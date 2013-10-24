#include "emu/odmbdev/VMEWrapper.h" 
 
namespace emu { namespace odmbdev {
    VMEWrapper::VMEWrapper(Crate * crate) : crate_(crate)
    {
      // blank constructor
    }
    
    VMEWrapper::VMEWrapper( const VMEWrapper& other ) : crate_( other.crate_ )
    {
    // copy constructor
    }
    
    void VMEWrapper::VMEWrite (unsigned short int address, unsigned short int command, unsigned int slot) {
    	unsigned int shiftedSlot = slot << 19;
    	int addr = (address & 0x07ffff) | shiftedSlot;
    	char rcv[2];
        crate_->vmeController()->vme_controller(3,addr,&command,rcv);
    }
    unsigned short int VMEWrapper::VMERead (unsigned short int address, unsigned int slot) {
    	unsigned int shiftedSlot = slot << 19;
    	int addr = (address & 0x07ffff) | shiftedSlot;
        unsigned short int data;
        char rcv[2];
        crate_->vmeController()->vme_controller(2,addr,&data,rcv);
        return (rcv[1] & 0xff) * 0x100 + (rcv[0] & 0xff);
    }
  }
}
