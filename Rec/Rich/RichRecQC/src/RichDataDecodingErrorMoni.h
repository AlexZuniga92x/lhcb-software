
//-----------------------------------------------------------------------------
/** @file RichDataDecodingErrorMoni.h
 *
 *  Header file for algorithm class : Rich::DAQ::DataDecodingErrorMoni
 *
 *  @author Chris Jones       Christopher.Rob.Jones@cern.ch
 *  @date   05/04/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECQC_RichDataDecodingErrorMoni_H
#define RICHRECQC_RichDataDecodingErrorMoni_H 1

// STD
#include <sstream>

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// base class
#include "RichKernel/RichHistoAlgBase.h"

// Interfaces
#include "RichKernel/IRichRawBufferToSmartIDsTool.h"

// Event
#include "Event/ODIN.h"

// boost
#include "boost/assign/list_of.hpp"

namespace Rich
{
  namespace DAQ
  {
    //-----------------------------------------------------------------------------
    /** @class DataDecodingErrorMoni RichDataDecodingErrorMoni.h
     *
     *  Simple monitor for the hits in each HPD
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   05/04/2002
     */
    //-----------------------------------------------------------------------------

    class DataDecodingErrorMoni : public HistoAlgBase
    {

    public:

      /// Standard constructor
      DataDecodingErrorMoni( const std::string& name,
                             ISvcLocator* pSvcLocator );

      virtual ~DataDecodingErrorMoni( ); ///< Destructor

      virtual StatusCode initialize();    // Algorithm initialization
      virtual StatusCode execute   ();    // Algorithm execution

    protected:
      
      /// Pre-Book all (non-MC) histograms
      virtual StatusCode prebookHistograms();
      
    private: // data

      /// Raw Buffer Decoding tool
      const Rich::DAQ::IRawBufferToSmartIDsTool * m_decoder;

    };

  }
}

#endif // RICHRECQC_RichDataDecodingErrorMoni_H
