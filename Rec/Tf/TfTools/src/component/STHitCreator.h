
//-----------------------------------------------------------------------------
/** @file STHitCreator.h
 *
 *  Header file for class : Tf::STHitCreator
 *
 *  $Id: STHitCreator.h,v 1.5 2010-04-07 05:13:57 wouter Exp $
 *
 *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
 *  @date   2007-06-01
 */
//-----------------------------------------------------------------------------

#ifndef TFKERNEL_STHitCreator_H
#define TFKERNEL_STHitCreator_H 1

#include <string>

#include "GaudiAlg/GaudiTool.h"

#include "GaudiKernel/IIncidentListener.h"
#include "TfKernel/STHit.h"

namespace Tf

{

  // forward declaration of the class that is holding the hit data
  namespace HitCreatorGeom {
    class STDetector ;
  }

  /** @class STHitCreator STHitCreator.h
   *
   *  Implementation of Tf::IITHitCreator and Tf::ITTHitCreator
   *
   *  Creates the Tf::STHit objects for the Tf tracking framework
   *
   *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
   *  @date   2007-06-01
   */

  template<class Trait>
  class STHitCreator final  : public GaudiTool,
                      virtual public Trait::ISTHitCreator,
                      virtual public IIncidentListener

  {

  public:
    /// The region type for TT hit (could get this from the Trait::ISTHitCreator)
    typedef Tf::Region<STHit> STRegion ;

    /// constructor
    STHitCreator(const std::string& type,
                 const std::string& name,
                 const IInterface* parent);

    /// destructor
    virtual ~STHitCreator();

    /// initialize
    StatusCode initialize() override;

    /// finalize
    StatusCode finalize() override;
 private:
    /// incident service handle
    void handle( const Incident& incident ) override;
 public:
    /// update manager handle
    StatusCode updateGeometry() ;

    // RestUsed flag for all OT hits
    void resetUsedFlagOfHits() const override;

    // Load all the IT hits
    STHitRange hits() const override;

    // Load the ST hits for a given region of interest
    STHitRange hits(const typename Trait::StationID iStation,
                    const typename Trait::LayerID iLayer) const override;

    // Load the ST hits for a given region of interest
    STHitRange hits(const typename Trait::StationID iStation,
                    const typename Trait::LayerID iLayer,
                    const typename Trait::RegionID iRegion) const override;

    // Load the ST hits for a given region of interest
    STHitRange hits(const typename Trait::StationID iStation,
                    const typename Trait::LayerID iLayer,
                    const typename Trait::RegionID iRegion,
                    const double xmin,
                    const double xmax) const override;

    // Load the ST hits for a given region of interest
    STHitRange hitsLocalXRange(const typename Trait::StationID iStation,
		       const typename Trait::LayerID iLayer,
		       const typename Trait::RegionID iRegion,
		       const double xmin,
		       const double xmax) const override;
    
    // Load the hits for a given region of interest
    STHitRange hits(const typename Trait::StationID iStation,
                    const typename Trait::LayerID iLayer,
                    const typename Trait::RegionID iRegion,
                    const double xmin,
                    const double xmax,
                    const double ymin,
                    const double ymax) const override;

    // Retrieve the STRegion for a certain region ID. The region
    const STRegion* region(const typename Trait::StationID iStation,
                           const typename Trait::LayerID iLayer,
                           const typename Trait::RegionID  iRegion) const override;

    // create a single STHit from an STChannelID
    Tf::STHit hit(LHCb::STChannelID stid) const override;

  private:

    /// Load the hits
    void loadHits() const ;

  private:
    const DeSTDetector* m_stdet ;
    mutable std::unique_ptr<HitCreatorGeom::STDetector> m_detectordata ;
    std::string m_clusterLocation;
    std::string m_detectorLocation;
  };

}

#endif // TFKERNEL_STHitCreator_H

