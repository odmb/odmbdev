#include "emu/odmbdev/ThreeTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu { namespace odmbdev {
  ThreeTextBoxAction::ThreeTextBoxAction(Crate * crate, emu::odmbdev::Manager * manager, string buttonLabel)
    : Action(crate, manager)
  {
    this->buttonLabel = buttonLabel;
    this->textBoxContent = string("4.00 0");
    this->textBoxContent2 = string(".05");
    this->textBoxContent3 = string("100");
  }

  void ThreeTextBoxAction::display(xgi::Output * out){
    addButtonWithThreeTextBoxesLVMB(out,
				this->buttonLabel,
				"textbox",
				this->textBoxContent,
                                "textbox2",
                                this->textBoxContent2,
                                "textbox3",
                                this->textBoxContent3,
                                "width: 250px;",
                                "width: 60px;",
                                "width: 35px;"
                                "width: 20px;");
  }
  // remember to call this base method with you override it, otherwise
  // textBoxContents will be empty!
  void ThreeTextBoxAction::respond(xgi::Input * in, ostringstream & out){
   this->textBoxContent = getFormValueString("textbox", in);
   this->textBoxContent2 = getFormValueString("textbox2", in);
   this->textBoxContent3 = getFormValueString("textbox3", in);
  }
}
}
