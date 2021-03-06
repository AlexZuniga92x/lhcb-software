
//=============================================================================================
/** @file RichDAQHeaderPDBase.cpp
 *
 *  Implementation file for RICH DAQ utility class : Rich::DAQ::HeaderPDBase
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   23/04/2007
 */
//=============================================================================================

// local
#include "RichKernel/RichDAQHeaderPDBase.h"

using namespace Rich::DAQ;

void HeaderPDBase::readFromDataStream( const LongType *& data )
{
  for ( auto i = 0u; i < nHeaderWords(); ++i )
  {
    headerWords()[i] = *(data++);
  }
}
