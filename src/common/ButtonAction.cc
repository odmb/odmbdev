#include "emu/odmbdev/ButtonAction.h"
 
namespace emu { namespace odmbdev {
    ButtonAction::ButtonAction(Crate * crate, string buttonLabel)
      : Action(crate)
    {
      this->buttonLabel = buttonLabel;
    }

    void ButtonAction::display(xgi::Output * out){
      addButton(out, this->buttonLabel);
    }
  }
}
