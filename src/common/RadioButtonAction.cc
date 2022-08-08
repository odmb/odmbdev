#include <string>

#include "emu/odmbdev/RadioButtonAction.h"
#include "emu/odmbdev/Manager.h"

#include "cgicc/Cgicc.h"

using namespace std;

namespace emu{
  namespace odmbdev{
    RadioButtonAction::RadioButtonAction(Crate* crate,
      emu::odmbdev::Manager* manager,
      string buttonLabel_in,
      string opt1_in, string opt2_in):
      Action(crate, manager),
      buttonLabel(buttonLabel_in),
      opt1(opt1_in),
      opt2(opt2_in),
      default_opt(true)
    {
    }

    void RadioButtonAction::display(xgi::Output * out){
      //kludge, for now xDCFEB is only toggle, so we can get default from there -MO
      addButtonWithRadio(out, buttonLabel, opt1, opt2, !(manager_->get_is_xdcfeb()));
    }

    void RadioButtonAction::respond(xgi::Input * in, ostringstream & out){
      const cgicc::Cgicc cgi(in);
      std::string form_value;
      cgicc::const_form_iterator name = cgi.getElement(buttonLabel);
      if(name != cgi.getElements().end()) {
          //form_value = cgi[buttonLabel]->getValue();
        if (**name == opt1) default_opt = true;
        else default_opt = false;
      }
      else {
          cout << "Error, no form found." << endl;
      }
    }
  }
}
