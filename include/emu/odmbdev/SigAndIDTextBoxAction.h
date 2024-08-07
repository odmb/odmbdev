#ifndef __emu_odmbdev_SigAndIDTextBoxAction_h__
#define __emu_odmbdev_SigAndIDTextBoxAction_h__

#include "emu/odmbdev/Action.h"
#include "emu/odmbdev/LogAction.h"

using namespace std;
using namespace emu::pc;

namespace emu { namespace odmbdev {
    class SigAndIDTextBoxAction : public LogAction {
    public:
      SigAndIDTextBoxAction(Crate * crate, Manager * manager, string buttonLabel);

      void display(xgi::Output * out);
      void respond(xgi::Input * in, xgi::Output * out, ostringstream & ssout);
    protected:
      string buttonLabel;
      string SigTextBoxContent;
      string IDTextBoxContent;

    };
  }
}

#endif
