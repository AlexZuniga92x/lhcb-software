// $Id:$
//====================================================================
//  AIDA Detector description implementation for LCD
//--------------------------------------------------------------------
//
//  Author     : M.Frank
//
//====================================================================

#include "DetDesc/lcdd/Readout.h"

using namespace std;
using namespace DetDesc::Geometry;

/// Initializing constructor to create a new object
Readout::Readout(const LCDD& /* lcdd */, const string& nam)
{
  assign(new Value<TNamed,Object>(),nam,"readout");
}

/// Access IDDescription structure
Ref_t Readout::idSpec() const   {
  return _data().id;
}

/// Access segmentation structure
Segmentation Readout::segmentation() const  {
  return _data().segmentation;
}

/// Assign IDDescription to readout structure
void Readout::setIDDescriptor(const Ref_t& new_descriptor)  const   {
  if ( isValid() )  {                    // Remember: segmentation is NOT owned by readout structure!
    if ( new_descriptor.isValid() )  {   // Do NOT delete!
      _data().id = new_descriptor;
      return;
    }
  }
  throw runtime_error("Readout::setSegmentation: Cannot assign ID descriptor [Invalid Handle]");
}

/// Assign segmentation structure to readout
void Readout::setSegmentation(const Segmentation& seg)   const  {
  if ( isValid() )  {
    Object& ro = _data();
    Segmentation::Implementation* e = ro.segmentation.ptr();
    if ( e )  { // Remember: segmentation is owned by readout structure!
      delete e; // Need to delete the segmentation object
    }
    if ( seg.isValid() )  {
      ro.segmentation = seg;
      return;
    }
  }
  throw runtime_error("Readout::setSegmentation: Cannot assign segmentation [Invalid Handle]");
}

/// Initializing constructor to create a new object
Alignment::Alignment(const LCDD& /* lcdd */, const string& nam)
{
  assign(new Value<TNamed,Object>(),nam,"alignment");
}

/// Initializing constructor to create a new object
Conditions::Conditions(const LCDD& /* lcdd */, const string& nam)
{
  assign(new Value<TNamed,Object>(),nam,"conditions");
}
