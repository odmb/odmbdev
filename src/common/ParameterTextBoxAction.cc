#include "emu/odmbdev/ParameterTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu { namespace odmbdev {
  ParameterTextBoxAction::ParameterTextBoxAction(Crate * crate, emu::odmbdev::Manager * manager, string buttonLabel, string parameter, unsigned int default_val)
    : Action(crate, manager)
  {
    this->buttonLabel = buttonLabel;
    this->parameter = parameter; // units of the parameter (e.g., times, packets) -- JB-F
    char s_default[20];
    sprintf (s_default,"%d",default_val);
    this->textBoxContent = s_default;
  }

  void ParameterTextBoxAction::display(xgi::Output * out){
    addButtonWithParameter(out,
			   this->parameter,
			   this->buttonLabel,
			   "textbox",
			   this->textBoxContent);
  }
  // remember to call this base method with you override it, otherwise
  // textBoxContents will be empty!
  void ParameterTextBoxAction::respond(xgi::Input * in, ostringstream & out){
    this->textBoxContent = getFormValueString("textbox", in);
  }
}
}
