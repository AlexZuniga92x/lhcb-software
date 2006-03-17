
//-----------------------------------------------------------------------------
/** @file RichTrSegMakerFromMCRichTracks.h
 *
 *  Header file for tool : RichTrSegMakerFromMCRichTracks
 *
 *  CVS Log :-
 *  $Id: RichTrSegMakerFromMCRichTracks.h,v 1.3 2006-03-17 15:55:32 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   14/01/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHMCTOOLS_RICHTRSEGMAKERFROMMCRICHTRACKS_H
#define RICHMCTOOLS_RICHTRSEGMAKERFROMMCRICHTRACKS_H 1

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/GaudiException.h"

// LHCbKernel
#include "Kernel/RichSmartID.h"

// RichKernel
#include "RichKernel/BoostArray.h"
#include "RichKernel/RichToolBase.h"

// interfaces
#include "RichRecBase/IRichTrSegMaker.h"
#include "RichRecBase/IRichRecMCTruthTool.h"
#include "RichKernel/IRichMCTruthTool.h"

// RichEvent
#include "RichEvent/RichTrackSegment.h"
#include "RichEvent/RichGeomPhoton.h"

// Event
#include "Event/Track.h"
#include "Event/MCParticle.h"
#include "Event/MCRichTrack.h"

// RichDet
#include "RichDet/DeRich.h"
#include "RichDet/DeRichRadiator.h"

// CLHEP
#include "Kernel/PhysicalConstants.h"

//-----------------------------------------------------------------------------
/** @class RichTrSegMakerFromMCRichTracks RichTrSegMakerFromMCRichTracks.h
 *
 *  Tool to create RichTrackSegments using Monte Carlo information
 *  from MCRichTracks and MCRichSegments.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   14/01/2002
 */
//-----------------------------------------------------------------------------

class RichTrSegMakerFromMCRichTracks : public RichToolBase,
                                       virtual public IRichTrSegMaker 
{

public: // methods for Gaudi Framework

  /// Standard Constructor
  RichTrSegMakerFromMCRichTracks( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent );

  /// Standard Destructor
  virtual ~RichTrSegMakerFromMCRichTracks( );

  // Initialization of the tool after creation
  virtual StatusCode initialize();

  // Finalization of the tool before deletion
  virtual StatusCode finalize  ();

public: // methods (and doxygen comments) inherited from interface

  // Create RichTrackSegments for a given tracking object
  int constructSegments( const ContainedObject * track,
                         std::vector<LHCb::RichTrackSegment*>& segments ) const;

private: // methods

  /** Access the associated MCRichTrack object for a given data object
   *  Various ways of finding the asscoiated MCRichTrack are tried, depending
   *  on the runtime type of the object passed
   *
   * @param obj Pointer to a object deriving from the ContainedObject base class
   *
   * @return Pointer to associated MCRichTrack object
   * @retval NULL  Monte Carlo association was not possible
   * @retval !NULL Association was successful
   */
  const LHCb::MCRichTrack * mcRichTrack( const ContainedObject * obj ) const;

private: // data

  /// Rich1 and Rich2 detector objects
  const DeRich* m_rich[Rich::NRiches];

  /// typedef of array of DeRichRadiators
  typedef boost::array<DeRichRadiator*, Rich::NRadiatorTypes> Radiators;
  /// Array of radiators
  Radiators m_radiators;

  /// Pointer to the RICH MC truth tool
  const IRichMCTruthTool * m_truth;

  /// Pointer to the RICH reconstruction MC truth tool
  const IRichRecMCTruthTool * m_rectruth;

  /// Flags to turn on/off individual radiators
  std::vector<bool> m_usedRads;

  /// Min path length for each radiator
  std::vector<double> m_minPathL;

  /// Min number of photons for each radiator
  std::vector<double> m_minPhots;

};

#endif // RICHMCTOOLS_RICHTRSEGMAKERFROMMCRICHTRACKS_H
