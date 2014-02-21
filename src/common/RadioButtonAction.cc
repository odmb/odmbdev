#include "emu/odmbdev/RadioButtonAction.h"
#include "emu/odmbdev/Manager.h"

#include "cgicc/Cgicc.h"

namespace emu{
  namespace odmbdev{
    RadioButtonAction::RadioButtonAction(Crate* crate,
					     emu::odmbdev::Manager* manager,
					     string buttonLabel_in):
      Action(crate, manager),
      buttonLabel(buttonLabel_in),
      highStat(false)
    {
    }

    void RadioButtonAction::display(xgi::Output * out){
      addButtonWithRadio(out, buttonLabel, "lowStat", "highStat");
    }

    void RadioButtonAction::respond(xgi::Input * in, ostringstream & out){
      cgicc::Cgicc cgi;
      if(cgi.queryCheckbox("highStat")) highStat = true;
      else highStat = false;
    }
  }
}
