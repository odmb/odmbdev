#include "emu/odmbdev/TwoTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu { namespace odmbdev {
  TwoTextBoxAction::TwoTextBoxAction(Crate * crate, emu::odmbdev::Manager * manager, string buttonLabel)
    : Action(crate, manager)
  {
    this->buttonLabel = buttonLabel;
    this->buttonLabel2 = buttonLabel2;
    this->textBoxContent = string("4.00 0");
    this->textBoxContent2 = string(".05");
  }

  void TwoTextBoxAction::display(xgi::Output * out){
    addButtonWithTwoTextBoxes(out,
				this->buttonLabel,
				"textbox",
				this->textBoxContent,
                                "textbox2",
                                this->textBoxContent2,
                                "width: 250px;",
                                "width: 60px;",
                                "width: 35px;");
  }
  // remember to call this base method with you override it, otherwise
  // textBoxContents will be empty!
  void TwoTextBoxAction::respond(xgi::Input * in, ostringstream & out){
   this->textBoxContent = getFormValueString("textbox", in);
   this->textBoxContent2 = getFormValueString("textbox2", in);
  }
}
}
