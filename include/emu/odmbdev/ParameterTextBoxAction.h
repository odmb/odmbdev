#ifndef __emu_odmbdev_ParameterTextBoxAction_h__
#define __emu_odmbdev_ParameterTextBoxAction_h__

#include "emu/odmbdev/Action.h"

using namespace std;
using namespace emu::pc;

namespace emu { namespace odmbdev {
    class ParameterTextBoxAction : public Action {
    public:
      ParameterTextBoxAction(Crate * crate, Manager * manager, string buttonLabel, string parameter);

      void display(xgi::Output * out);
      void respond(xgi::Input * in, ostringstream & out);
    protected:
      string buttonLabel;
      string textBoxContent;
      string parameter;
    };
  }
}

#endif
