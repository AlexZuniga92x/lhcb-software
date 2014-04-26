
#include "HltBase/IConfigTreeEditor.h"

/// Return the interface ID
const InterfaceID& IConfigTreeEditor::interfaceID()
{
    static const InterfaceID id{"IConfigTreeEditor", 2, 0};
    return id;
}
