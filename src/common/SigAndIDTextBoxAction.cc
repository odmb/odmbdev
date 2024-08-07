#include "emu/odmbdev/SigAndIDTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu { namespace odmbdev {
  SigAndIDTextBoxAction::SigAndIDTextBoxAction(Crate * crate, emu::odmbdev::Manager * manager, string buttonLabel)
    : LogAction(crate, manager)
  {
    this->buttonLabel = buttonLabel;
    this->SigTextBoxContent = string("");
    this->IDTextBoxContent = string("");
  }

  void SigAndIDTextBoxAction::display(xgi::Output * out){
    addButtonWithNameAndIDBoxes(out,
				this->buttonLabel,
				"SigTextbox",
				this->SigTextBoxContent,	
				"IDTextbox",
				this->IDTextBoxContent);
  }
  // remember to call this base method with you override it, otherwise
  // textBoxContents will be empty!
  void SigAndIDTextBoxAction::respond(xgi::Input * in, xgi::Output * out, ostringstream & ssout){
   this->SigTextBoxContent = getFormValueString("SigTextbox", in);
   this->IDTextBoxContent  = getFormValueString("IDTextbox", in);
  }
}
}


