#include "emu/odmbdev/RepeatTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu { namespace odmbdev {
  RepeatTextBoxAction::RepeatTextBoxAction(Crate * crate, emu::odmbdev::Manager * manager, string buttonLabel)
    : Action(crate, manager)
  {
    this->buttonLabel = buttonLabel;
    this->textBoxContent = string("1");
  }

  void RepeatTextBoxAction::display(xgi::Output * out){
    addButtonWithRepeatOpt(out,
				this->buttonLabel,
				"textbox",
				this->textBoxContent);
  }
  // remember to call this base method with you override it, otherwise
  // textBoxContents will be empty!
  void RepeatTextBoxAction::respond(xgi::Input * in, ostringstream & out){
   this->textBoxContent = getFormValueString("textbox", in);
  }
}
}
