#ifndef __emu_odmbdev_ThreeTextBoxAction_h__
#define __emu_odmbdev_ThreeTextBoxAction_h__

#include "emu/odmbdev/Action.h"

using namespace std;
using namespace emu::pc;

namespace emu{
  namespace odmbdev{
    class ThreeTextBoxAction : public Action {
    public:
      ThreeTextBoxAction(Crate * crate, Manager * manager, const std::string& buttonLabel);
      ThreeTextBoxAction(Crate * crate, Manager * manager, const std::string& buttonLabel,
                         const std::string& textBoxContent_in,
                         const std::string& textBoxContent2_in,
                         const std::string& textBoxContent3_in,
                         const std::string& label1,
                         const std::string& label2,
                         const std::string& label3,
                         const std::string& button_style,
                         const std::string& style1,
                         const std::string& style2,
                         const std::string& style3);

      void display(xgi::Output * out);
      void respond(xgi::Input * in, ostringstream & out);
      virtual void respond(xgi::Input* in, ostringstream& out,
                           const string& textBoxContent3_in);
    protected:
      string buttonLabel;
      string textBoxContent;
      string textBoxContent2;
      string textBoxContent3;
      std::string label1_;
      std::string label2_;
      std::string label3_;
      std::string button_style_;
      std::string style1_;
      std::string style2_;
      std::string style3_;
    };
  }
}

#endif
