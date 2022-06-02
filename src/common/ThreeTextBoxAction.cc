#include "emu/odmbdev/ThreeTextBoxAction.h"
#include "emu/odmbdev/Manager.h"

namespace emu{
  namespace odmbdev {
    ThreeTextBoxAction::ThreeTextBoxAction(Crate* crate,
                                           emu::odmbdev::Manager* manager,
                                           const std::string& buttonLabel_in):
      Action(crate, manager),
      buttonLabel(buttonLabel_in),
      textBoxContent("4.5  3.0"),
      textBoxContent2("0.1"),
      textBoxContent3("300"),
      label1_("V1 V2"),
      label2_("tol"),
      label3_("N"),
      button_style_("width: 120px;"),
      style1_("width: 100px;"),
      style2_("width: 35px;"),
      style3_("width: 80px;"){
    }

    ThreeTextBoxAction::ThreeTextBoxAction(Crate* crate,
                                           emu::odmbdev::Manager* manager,
                                           const std::string& buttonLabel_in,
                                           const std::string& textBoxContent_in,
                                           const std::string& textBoxContent2_in,
                                           const std::string& textBoxContent3_in,
                                           const std::string& label1,
                                           const std::string& label2,
                                           const std::string& label3,
                                           const std::string& button_style,
                                           const std::string& style1,
                                           const std::string& style2,
                                           const std::string& style3):
      Action(crate, manager),
      buttonLabel(buttonLabel_in),
      textBoxContent(textBoxContent_in),
      textBoxContent2(textBoxContent2_in),
      textBoxContent3(textBoxContent3_in),
      label1_(label1),
      label2_(label2),
      label3_(label3),
      button_style_(button_style),
      style1_(style1),
      style2_(style2),
      style3_(style3){
    }

    void ThreeTextBoxAction::display(xgi::Output * out){
      addButtonWithThreeTextBoxes(out, buttonLabel, "textbox", textBoxContent,
                                  "textbox2", textBoxContent2, "textbox3",
                                  textBoxContent3, label1_, label2_, label3_, button_style_, style1_,
                                  style2_, style3_);
    }

    void ThreeTextBoxAction::respond(xgi::Input* in, ostringstream& out){
      textBoxContent = getFormValueString("textbox", in);
      textBoxContent2 = getFormValueString("textbox2", in);
      textBoxContent3 = getFormValueString("textbox3", in);
      respond(in, out, textBoxContent3);
    }

    void ThreeTextBoxAction::respond(xgi::Input* in, ostringstream& out,
                                     const string& textBoxContent3_in){
      textBoxContent3=textBoxContent3_in;
    }
  }
}
