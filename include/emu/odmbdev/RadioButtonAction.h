#ifndef __emu_odmbdev_RadioButtonAction_h__
#define __emu_odmbdev_RadioButtonAction_h__

#include "emu/odmbdev/Action.h"

using namespace std;
using namespace emu::pc;

namespace emu{
  namespace odmbdev {
    class RadioButtonAction : public Action {
    public:
      RadioButtonAction(Crate* crate, Manager* manager, string buttonLabel);

      void display(xgi::Output* out);
      void respond(xgi::Input* in, ostringstream& out);
    protected:
      string buttonLabel;
      bool highStat;
    };
  }
}

#endif
