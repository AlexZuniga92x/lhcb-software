
//-----------------------------------------------------------------------------
/** @file RichSmartIDTool.h
 *
 *  Header file for tool : Rich::SmartIDTool
 *
 *  CVS Log :-
 *  $Id: RichSmartIDTool.h,v 1.23 2007-07-20 09:27:46 jonrob Exp $
 *
 *  @author Antonis Papanestis
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   2003-10-28
 */
//-----------------------------------------------------------------------------

#ifndef RICHTOOLS_RICHSMARTIDTOOL_H
#define RICHTOOLS_RICHSMARTIDTOOL_H 1

// Base class and interface
#include "RichKernel/RichToolBase.h"
#include "RichKernel/IRichSmartIDTool.h"

// from RichKernel
#include "RichKernel/BoostArray.h"
#include "RichKernel/RichSmartIDSorter.h"
#include "RichKernel/RichPixelCluster.h"

// RichDet
#include "RichDet/DeRichHPDPanel.h"
#include "RichDet/DeRichSystem.h"

namespace Rich
{

  //-----------------------------------------------------------------------------
  /** @class SmartIDTool RichSmartIDTool.h
   *
   *  A tool to preform the manipulation of RichSmartID channel identifiers
   *
   *  @author Antonis Papanestis
   *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
   *  @date   2003-10-28
   *
   *  @todo Move application of panel offset in RichSmartIDTool::globalToPDPanel
   *        into DeRichHPDPanel class
   */
  //-----------------------------------------------------------------------------

  class SmartIDTool : public Rich::ToolBase,
                      virtual public ISmartIDTool
  {

  public: // Methods for Gaudi Framework

    /// Standard constructor
    SmartIDTool( const std::string& type,
                 const std::string& name,
                 const IInterface* parent);

    virtual ~SmartIDTool( ); ///< Destructor

    // Initialization of the tool after creation
    virtual StatusCode initialize();

  public: // methods (and doxygen comments) inherited from interface

    // Converts a RichSmartID channel identification into a position in global LHCb coordinates.
    virtual StatusCode globalPosition ( const LHCb::RichSmartID smartid,
                                        Gaudi::XYZPoint& detectPoint  ) const;

    // Finds the average position of a cluster of RichSmartIDs, in global LHCb coordinates
    virtual StatusCode globalPosition ( const Rich::HPDPixelCluster& cluster,
                                        Gaudi::XYZPoint& detectPoint ) const;

    // Converts a RichSmartID channel identification into a position in
    //  global LHCb coordinates, on the pixel anode chip.
    virtual StatusCode anodeGlobalPosition ( const LHCb::RichSmartID smartid,
                                             Gaudi::XYZPoint& detectPoint ) const;

    // Finds the average position of a cluster of RichSmartIDs, in global LHCb coordinates,
    //  on the pixel anode chip.
    virtual StatusCode anodeGlobalPosition ( const Rich::HPDPixelCluster& cluster,
                                             Gaudi::XYZPoint& detectPoint ) const;

    // Converts an HPD RichSmartID identification into a position in global LHCb coordinates.
    virtual StatusCode hpdPosition ( const LHCb::RichSmartID hpdid,
                                     Gaudi::XYZPoint& hpdPoint ) const;

    // Computes the global position coordinate for a given position in local
    virtual Gaudi::XYZPoint globalPosition ( const Gaudi::XYZPoint& localPoint,
                                             const Rich::DetectorType rich,
                                             const Rich::Side side ) const;

    // Converts a position in global coordinates to the corresponding RichSmartID
    virtual StatusCode smartID( const Gaudi::XYZPoint&  globalPoint,
                                LHCb::RichSmartID& smartid ) const;

    // Supplies a vector of all currently active and valid channels in the RICH detectors
    virtual const LHCb::RichSmartID::Vector& readoutChannelList( ) const;

    // Converts a position in global coordinates to the local coordinate system.
    virtual Gaudi::XYZPoint globalToPDPanel ( const Gaudi::XYZPoint& globalPoint ) const;

  private:

    typedef boost::array<DeRichHPDPanel*, 2> HPDPanelsPerRich;
    boost::array<HPDPanelsPerRich, 2> m_photoDetPanels;

    /// List of active RichSmartIDs
    mutable LHCb::RichSmartID::Vector m_readoutChannels;

    /// RichSystem object
    DeRichSystem* m_richS;

  };

}

#endif // RICHDETTOOLS_RICHSMARTIDTOOL_H
