#ifndef __emu_odmbdev_TwoTextBoxAction_h__
#define __emu_odmbdev_TwoTextBoxAction_h__

#include "emu/odmbdev/Action.h"

using namespace std;
using namespace emu::pc;

namespace emu { namespace odmbdev {
    class TwoTextBoxAction : public Action {
    public:
      TwoTextBoxAction(Crate * crate, Manager * manager, string buttonLabel);

      void display(xgi::Output * out);
      void respond(xgi::Input * in, ostringstream & out);
    protected:
      string buttonLabel;
      string buttonLabel2;
      string textBoxContent;
      string textBoxContent2;
    };
  }
}

#endif
