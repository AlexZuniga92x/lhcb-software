
//-----------------------------------------------------------------------------
/** @file TStationHitManager.h
 *
 *  Header file for class : Tf::TStationHitManager
 *
 *  $Id: TStationHitManager.h,v 1.27 2010-04-07 05:17:03 wouter Exp $
 *
 *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
 *  @date   2007-06-01
 */
//-----------------------------------------------------------------------------

#ifndef TFTOOLS_TSTATIONHITMANAGER_H
#define TFTOOLS_TSTATIONHITMANAGER_H 1

// Include files
#include <functional>
#include <memory>
#include "boost/iterator/transform_iterator.hpp"
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"

// Tf framework
#include "TfKernel/IndexedHitContainer.h"
#include "TfKernel/IndexedBitArray.h"
#include "TfKernel/IOTHitCreator.h"
#include "TfKernel/IITHitCreator.h"
#include "TfKernel/ITTHitCreator.h"
#include "TfKernel/IUTHitCreator.h"
#include "TfKernel/IOTHitCleaner.h"
#include "TfKernel/ISTHitCleaner.h"
#include "TfKernel/LineHit.h"
#include "TfKernel/HitExtension.h"
#include "TfKernel/RecoFuncs.h"
#include "TfKernel/RegionID.h"
#include "TfKernel/TfIDTypes.h"
#include "TfKernel/RegionSelectors.h"
#include "TfKernel/IStationSelector.h"

/// Static interface ID
static const InterfaceID IID_TStationHitManager ( "TStationHitManager", 1, 0 );

namespace Tf
{
  /** @class TStationHitManager TStationHitManager.h TfKernel/TStationHitManager.h 
   *
   *  T station hit manager. Used to manage extended hit objects for the T
   *  Stations (OT and IT). 
   *
   *  Methods are provided to return the hits in a selected part of the detectors.
   *  E.g.
   *
   *  @code
   *  // Get all the hits in the T stations
   *  Tf::TStationHitManager::HitRange range = hitMan->hits();
   *
   *  // Get all the hits in one specific T station
   *  Tf::TStationID sta = ...;
   *  Tf::TStationHitManager::HitRange range = hitMan->hits(sta);
   *
   *  // Get all the hits in one specific layer of one T station
   *  Tf::TStationID sta = ...;
   *  Tf::TLayerID   lay = ...;
   *  Tf::TStationHitManager::HitRange range = hitMan->hits(sta,lay);
   *
   *  // Get all the hits in a specific 'region' of one layer of one T station
   *  Tf::TStationID sta = ...;
   *  Tf::TLayerID   lay = ...;
   *  Tf::TRegionID  reg = ...;
   *  Tf::TStationHitManager::HitRange range = hitMan->hits(sta,lay,reg);
   *  @endcode
   *
   *  In addition, it is possible to perform a custom selection of hits based on
   *  a user defined selection object :-
   *
   *  @code
   *  // Get all the hits selected by a specfic selector object
   *  LHCb::State * test_state = ....;
   *  const double nsigma = 3.0;
   *  StateRegionSelector selector( *test_state, nsigma );
   *  hitMan->prepareHits(selector);
   *  // Can now use any of the hits(..) methods to access hit ranges, e.g.
   *  Tf::TStationHitManager::HitRange range = hitMan->hits();
   *  // Which gives the selected hits in all stations, layers and regions in the T stations
   *  @endcode
   *
   *  The only requirement on the selector object used is it must implement one specific method.
   *  See Tf::StateRegionSelector for an example of such an object.
   *
   *  In all cases the returned Range object acts like a standard vector or container :-
   *
   *  @code
   *   // Iterate over the returned range
   *  for ( Tf::TStationHitManager::HitRange::const_iterator iR = range.begin();
   *        iR != range.end(); ++iR )
   *  {
   *    // do something with the hit
   *  }
   *  @endcode
   *
   *  @author S. Hansmann-Menzemer, W. Hulsbergen, C. Jones, K. Rinnert
   *  @date   2007-06-01
   **/

  template < class Hit >
  class TStationHitManager : public GaudiTool,
                             public IIncidentListener
  {

  public:
    typedef  IndexedHitContainer< Tf::RegionID::OTIndex::kNStations, 
                                  Tf::RegionID::OTIndex::kNLayers, 
                                  Tf::RegionID::OTIndex::kNRegions + Tf::RegionID::ITIndex::kNRegions, 
                                  Hit* > Hits;
    /// range object for Hits
    typedef typename Hits::HitRange HitRange;

  public:

    /// InterfaceID for this tool
    static const InterfaceID& interfaceID() { return IID_TStationHitManager; }

    /// Standard Constructor
    TStationHitManager( const std::string& type,
                        const std::string& name,
                        const IInterface* parent ) ;

    /// Destructor
    ~TStationHitManager( ) { }

    /// Tool initialization
    StatusCode initialize ( );

    /// Tool finalization
    StatusCode finalize ( );

    /// Handle method for incident service callbacks
    void handle ( const Incident& incident )
    {
      if ( IncidentType::BeginEvent == incident.type() ) clearHits() ;
    }

  public:

    /** Initialise the hits for the current event using the given selector object
     *
     *  Any selector object can be used, the only requirement is it must implement the
     *  method :-
     *  @code
     *    inline XYSearchWindow searchWindow( const double z ) const
     *  @endcode
     *  Which returns the search window for a given z position.
     *
     *  Example usage, using the StateRegionSelector class which creates an N sigma
     *  search window around a track state, using a 2nd order parameterisation :-
     *  @code
     *   LHCb::State * test_state = ....;
     *   const double nsigma = 3.0;
     *   StateRegionSelector selector( *test_state, nsigma );
     *   hitMan->prepareHits(selector);
     *  @endcode 
     *
     *  @param[in] selector The selector object.
     */
    virtual void prepareHitsInWindow( const IStationSelector & selector );

    /** Load the hits for a given region of interest
     *
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     *
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hits( const TStationID sta,
                          const TLayerID   lay,
                          const TRegionID  region ) const
    {
      if ( !allHitsPrepared(sta,lay,region) ) { prepareHits(sta,lay,region); }
      return m_hits.range( sta,lay, region );
    }

    /** Load the hits for a given region of interest
     *
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hits( const TStationID sta,
                          const TLayerID   lay ) const
    {
      if ( !allHitsPrepared(sta,lay) ) { prepareHits(sta,lay); }
      return m_hits.range(sta,lay) ; 
    }

    /** Load the hits for a given region of interest
     *
     *  @param[in] sta    Station ID
     *
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hits( const TStationID sta ) const
    {
      if ( !allHitsPrepared(sta) ) { prepareHits(sta); }
      return m_hits.range(sta);
    }

    /** Load the all hits
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hits( ) const
    {
      if ( !allHitsPrepared() ) { prepareHits(); }
      return m_hits.range();
    }

    // Not clear to me if these following methods  should be in the common interface
    // Are they pat specific, so should move to the pat implemation ?

    /** Load the hits for a given region of interest
     *  In addition, specify a minimum x value
     *
     *  @param[in] xMin   Minimum x value of hit (at y=0)
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     *
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hitsWithMinX( const double xMin,
                                  const TStationID sta,
                                  const TLayerID   lay,
                                  const TRegionID  region ) const
    {
#ifndef __GCCXML__
      if ( !allHitsPrepared(sta,lay,region) ) { prepareHits(sta,lay,region); }
      HitRange range = m_hits.range(sta,lay,region);
      return HitRange( std::lower_bound( range.begin(), range.end(),
                                         xMin,
                                         Tf::compByX_LB< Hit >() ),
                       range.end()); 
#else
      return HitRange();
#endif
    }

    /** Load the hits for a given region of interest
     *  In addition, specify an x range
     *
     *  @param[in] xMin   Minimum x value of hit (at y=0)
     *  @param[in] xMax   Maximum x value of hit (at y=0)
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     *
     *  @return Range object for the hits in the selected region of interest
     */
    inline HitRange hitsInXRange( const double xMin, const double xMax,
                                  const TStationID sta,
                                  const TLayerID   lay,
                                  const TRegionID  region ) const
    {
      if ( !allHitsPrepared(sta,lay,region) ) { prepareHits(sta,lay,region); }
      auto range = m_hits.range(sta,lay,region);
      auto first = std::lower_bound( std::begin(range), std::end(range),
                                     xMin,
                                     Tf::compByX< Hit >() ) ;
      return { first, std::upper_bound( first, std::end(range), 
                                     xMax, 
                                     Tf::compByX< Hit >() ) } ;
    }

    /** Retrieve the Region for a certain IT or OT region ID. The region
     *   knows its 'size' and gives access to its hits.
     *
     *  Note that the method returns a pointer to the base class Tf::EnvelopeBase
     *  This object knows most basic questions, but for more complex tasks the user
     *  can dynamic cast it to the actual region object for OT or IT
     *
     *  @param[in] iStation Station ID
     *  @param[in] iLayer   Station layer ID
     *  @param[in] iRegion  Region within the layer
     *
     *  @return Pointer to the Tf::EnvelopeBase object
     */
    inline const Tf::EnvelopeBase* region( const TStationID iStation,
                                           const TLayerID   iLayer,
                                           const TRegionID  iRegion ) const
    {
      return ( iRegion > maxOTRegions()-1 ?
               static_cast<const Tf::EnvelopeBase*>(this->itHitCreator()->region(iStation,
                                                                                 iLayer,
                                                                                 iRegion.itRegionID())) :
               static_cast<const Tf::EnvelopeBase*>(this->otHitCreator()->region(iStation,
                                                                                 iLayer,
                                                                                 iRegion.otRegionID())) );
    }

  public:

    /** Sort the hits in a given station, layer and region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */
    template < typename SORTER >
    inline void sortHits( const TStationID sta,
                          const TLayerID   lay,
                          const TRegionID  region ) const
    {
      std::pair<typename Hits::iterator,typename Hits::iterator>  rng = m_hits.range_(sta,lay,region);
      std::sort ( rng.first, rng.second, SORTER() );
    }

    
    /// Clear the hit containers for a new event or for running on 
    /// full event after decoding on demand
    void clearHits () const;
  
  protected:

    /** Set the hits ready flag for given region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     *  @param[in] ok     The status flag (true means hits ready, false means not ready)
     */
    inline void setAllHitsPrepared( const TStationID sta,
                                    const TLayerID   lay,
                                    const TRegionID  region
                                  ) const { m_hits_ready.set(sta,lay,region); }

    /** Are all the hits ready
     *  @return boolean indicating if all the hits in the given region are ready or not
     *  @retval TRUE  Hits are ready
     *  @retval FALSE Hits are not ready
     */
    inline bool allHitsPrepared() const { return m_hits_ready.all(); }

    /** Are all the hits ready in the given region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     *  @return boolean indicating if all the hits in the given region are ready or not
     *  @retval TRUE  Hits are ready
     *  @retval FALSE Hits are not ready
     */
    inline bool allHitsPrepared(const TStationID sta,
                                const TLayerID   lay,
                                const TRegionID  region) const
    { return m_hits_ready.test(sta,lay,region); }

    /** Are all the hits ready in the given layer
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @return boolean indicating if all the hits in the given region are ready or not
     *  @retval TRUE  Hits are ready
     *  @retval FALSE Hits are not ready
     */
    inline bool allHitsPrepared(const TStationID sta,
                                const TLayerID   lay) const
    { return m_hits_ready.all(sta,lay); }

    /** Are all the hits ready in the given station
     *  @param[in] sta    Station ID
     *  @return boolean indicating if all the hits in the given region are ready or not
     *  @retval TRUE  Hits are ready
     *  @retval FALSE Hits are not ready
     */
    inline bool allHitsPrepared(const TStationID sta) const
    { return m_hits_ready.all(sta); }

  protected:

    /// The underlying OT hit creator
    inline const Tf::IOTHitCreator * otHitCreator() const { return m_othitcreator; }

    /// The underlying IT hit creator
    inline const Tf::IITHitCreator * itHitCreator() const { return m_ithitcreator; }

  protected:

    /// Access the maximum number of stations
    inline TStationID maxStations()   const { return TStationID(Tf::RegionID::OTIndex::kNStations);   }
    /// Access the maximum number of layers
    inline TLayerID maxLayers()       const { return TLayerID(Tf::RegionID::OTIndex::kNLayers);     }
    /// Access the maximum number of OT regions
    inline OTRegionID maxOTRegions()  const { return OTRegionID(Tf::RegionID::OTIndex::kNRegions); }
    /// Access the maximum number of IT regions
    inline ITRegionID maxITRegions()  const { return ITRegionID(Tf::RegionID::ITIndex::kNRegions); }
    /// Access the maximum number of regions
    inline TRegionID maxRegions()     const { return TRegionID( Tf::RegionID::OTIndex::kNRegions + Tf::RegionID::ITIndex::kNRegions );    }

    /// Initialise the IT hits for the current event using the given selector object
    void prepareITHitsInWindow( const IStationSelector & selector );

    /// Initialise the OT hits for the current event using the given selector object
    void prepareOTHitsInWindow( const IStationSelector & selector );

    /// Is OT hit cleaning activated
    inline bool cleanOTHits() const { return m_cleanOTHits; }

    /// Is IT hit cleaning activated
    inline bool cleanITHits() const { return m_cleanITHits; }

    /** Process an OT hit range (create extended hits)
     *  @param[in] othits The OTHit range to process
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */
    void processRange( const Tf::OTHitRange & othits,
                       const TStationID sta,
                       const TLayerID   lay,
                       const TRegionID  region ) const;

    /** Process an ST hit range (create extended hits)
     *  @param[in] sthits The STHit range to process
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */
    void processRange( const Tf::STHitRange & sthits,
                       const TStationID sta,
                       const TLayerID   lay,
                       const TRegionID  region ) const;

  public:

    /** Initialise all the hits for the current event in the given region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */
    void prepareHits(const TStationID sta,
		     const TLayerID   lay,
		     const TRegionID  region) const;

    /** Initialise all the hits for the current event in the given region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     */
    void prepareHits(const TStationID sta,
                     const TLayerID   lay) const;

    /** Initialise all the hits for the current event in the given region
     *  @param[in] sta    Station ID
     */
    void prepareHits(const TStationID sta) const;
    
    /** Initialise all the hits for the current event
     */
    virtual void prepareHits() const;

  private:

     /** Prepare all hits in the given IT region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */

    void prepareITHits(const TStationID sta,
                       const TLayerID   lay,
                       const ITRegionID region) const;

    /** Prepare all hits in the given OT region
     *  @param[in] sta    Station ID
     *  @param[in] lay    Station layer ID
     *  @param[in] region Region within the layer
     */
    void prepareOTHits(const TStationID sta,
                       const TLayerID   lay,
                       const OTRegionID region) const;

    /// The underlying OT hit creator
    Tf::IOTHitCreator* m_othitcreator ;

    /// The underlying IT hit creator
    Tf::IITHitCreator* m_ithitcreator ;

    /// The OT hit cleaner
    Tf::IOTHitCleaner * m_otCleaner;

    /// The ST hit cleaner
    Tf::ISTHitCleaner * m_itCleaner;

    mutable Hits m_hits;

    // Flags to indicate which hits are ready
    mutable IndexedBitArray< Tf::RegionID::OTIndex::kNStations, 
                             Tf::RegionID::OTIndex::kNLayers, 
                             Tf::RegionID::OTIndex::kNRegions + Tf::RegionID::ITIndex::kNRegions
                           >   m_hits_ready; ///< Flags to indicate which regions have hits ready

    /// Should OT hit cleaning be performed ?
    bool m_cleanOTHits;

    /// Should IT hit cleaning be performed ?
    bool m_cleanITHits;

  };

#ifndef __GCCXML__

  template<class Hit>
  TStationHitManager<Hit>::TStationHitManager( const std::string& type,
                                               const std::string& name,
                                               const IInterface* parent) :
    GaudiTool (type, name, parent),
    m_othitcreator(nullptr), m_ithitcreator(nullptr),
    m_otCleaner    { nullptr },
    m_itCleaner    { nullptr },
    m_hits         { 16384 } // initial capacity of container...
  {
    declareInterface<TStationHitManager<Hit> >(this);
    declareProperty( "CleanOTHits", m_cleanOTHits = false );
    declareProperty( "CleanITHits", m_cleanITHits = false );
  }

  template<class Hit>
  StatusCode TStationHitManager<Hit>::initialize ( )
  {
    StatusCode sc = GaudiTool::initialize();
    if ( sc.isFailure() ) return sc;

    // set up to be told about each new event
    incSvc()->addListener(this, IncidentType::BeginEvent);

    // load our hit creators

    m_othitcreator = tool<IOTHitCreator>("Tf::OTHitCreator", "OTHitCreator");
    m_ithitcreator = tool<IITHitCreator>("Tf::STHitCreator<Tf::IT>", "ITHitCreator");
    if (!m_othitcreator || !m_ithitcreator) return StatusCode::FAILURE;

    // load private hit cleaners, if needed
    if ( cleanOTHits() ) m_otCleaner = this->tool<Tf::IOTHitCleaner>("Tf::OTHitCleaner","OTHitCleaner",this);
    if ( cleanITHits() ) m_itCleaner = this->tool<Tf::ISTHitCleaner>("Tf::STHitCleaner","ITHitCleaner",this);

    // make sure we are ready for first event
    this->clearHits();

    return sc;
  }

  template<class Hit>
  inline StatusCode TStationHitManager<Hit>::finalize ( )
  {
    this->clearHits();
    return GaudiTool::finalize();
  }

  template<class Hit>
  inline void TStationHitManager<Hit>::clearHits() const
  {
    HitRange rng = m_hits.range();
    std::for_each( rng.begin(), rng.end(), std::default_delete<Hit>() ) ; 
    m_hits.clear();
    m_hits_ready.clear();
  }

  template<class Hit>
  inline void TStationHitManager<Hit>::processRange( const Tf::OTHitRange & othits,
                                                     const TStationID sta,
                                                     const TLayerID   lay,
                                                     const TRegionID  region ) const
  {
    if ( cleanOTHits() )
    {
      // clean hits
      Tf::OTHits selectedhits;
      m_otCleaner->cleanHits( othits, selectedhits );
      
      // convert only those selected
      auto fun = [&](Tf::OTHits::const_reference hit) { return new Hit(*hit); } ;
      auto b =  boost::make_transform_iterator(std::begin(selectedhits), std::cref(fun));
      auto e =  boost::make_transform_iterator(std::end(selectedhits), std::cref(fun));
      m_hits.insert( sta, lay, region, b, e );
    } else {
      // no cleaning, so just convert everything
      auto fun = [&](Tf::OTHitRange::const_reference hit) { return new Hit(*hit); } ; 
      auto b =  boost::make_transform_iterator(std::begin(othits), std::cref(fun));
      auto e =  boost::make_transform_iterator(std::end(othits), std::cref(fun));
      m_hits.insert( sta, lay, region, b, e );
    }
  }

  template<class Hit>
  inline void TStationHitManager<Hit>::processRange( const Tf::STHitRange & sthits,
                                                     const TStationID sta,
                                                     const TLayerID   lay,
                                                     const TRegionID  region ) const
  {
    if ( cleanITHits() ) {
      // clean hits
      Tf::STHits selectedhits;
      m_itCleaner->cleanHits( sthits, selectedhits );
      // convert only those selected
      auto fun = [&](Tf::STHits::const_reference hit) { return new Hit(*hit); };
      auto b =  boost::make_transform_iterator(std::begin(selectedhits), std::cref(fun));
      auto e =  boost::make_transform_iterator(std::end(selectedhits),   std::cref(fun));
      m_hits.insert( sta, lay, region, b, e );
    } else {
      // no cleaning, so just convert everything
      auto fun = [&](Tf::STHitRange::const_reference hit) { return new Hit(*hit); };
      auto b   = boost::make_transform_iterator(std::begin(sthits), std::cref(fun));
      auto e   = boost::make_transform_iterator(std::end(sthits),   std::cref(fun));
      m_hits.insert( sta, lay, region, b, e );
    }
  }


  template<class Hit>
  inline void TStationHitManager<Hit>::prepareITHits(const TStationID sta,
                                                     const TLayerID   lay,
                                                     const ITRegionID region) const
  {
    this -> processRange ( this->itHitCreator()->hits(sta,lay,region) , 
                           sta, lay, TRegionID(region) );
  }

  template<class Hit>
  inline void TStationHitManager<Hit>::prepareOTHits(const TStationID sta,
                                                     const TLayerID   lay,
                                                     const OTRegionID region) const
  {
    this -> processRange ( this->otHitCreator()->hits(sta,lay,region) , 
                           sta, lay, TRegionID(region) );
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareHits() const
  {
    if ( !this->allHitsPrepared() ) {
      for ( TStationID sta = 0; sta < this->maxStations(); ++sta ) {
        this->prepareHits(sta);
      }
    }
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareHits(const TStationID sta) const
  {
    if ( !this->allHitsPrepared(sta) ) {
      for ( TLayerID lay = 0; lay<this->maxLayers(); ++lay ) {
        this->prepareHits(sta,lay);
      }
    }
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareHits(const TStationID sta,
                                            const TLayerID   lay) const
  {
    if ( !this->allHitsPrepared(sta,lay) ) {
      for ( TRegionID ir = 0; ir < maxRegions(); ++ir ) {
        this->prepareHits(sta,lay,ir);
      }
    }
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareHits(const TStationID sta,
                                            const TLayerID lay,
                                            const TRegionID region) const
  {
    if ( !this->allHitsPrepared(sta,lay,region) ) {
      if ( region >= this->maxOTRegions() ) {
        this->prepareITHits(sta,lay,region.itRegionID());
      } else {
        this->prepareOTHits(sta,lay,region.otRegionID());
      }
      this->setAllHitsPrepared(sta,lay,region);
    }
  }

  //--------------------------------------------------------------------------------------------

  template<class Hit>
  void TStationHitManager<Hit>::prepareHitsInWindow( const IStationSelector & selector )
  {
    this->clearHits();             // Clear any previous hits
    this->prepareOTHitsInWindow(selector); // select the OT hits
    this->prepareITHitsInWindow(selector); // select the IT hits
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareITHitsInWindow( const IStationSelector & selector )
  {
    for (TStationID sta=0; sta < this->maxStations(); ++sta) {
      for (TLayerID lay=0; lay < this->maxLayers(); ++lay) {
        for (ITRegionID it=0; it < this->maxITRegions(); ++it) {
          // The z value for this region
          const double z = this->itHitCreator()->region(sta,lay,it)->z();
          // get the search window
          XYSearchWindow win = selector.searchWindow(z);
          // get the hits in the search window
          Tf::STHitRange sthits = this->itHitCreator()->hits(sta,lay,it,
                                                             win.minX(),win.maxX(),
                                                             win.minY(),win.maxY() );
          processRange ( sthits, sta, lay, it + this->maxOTRegions() );
          this->setAllHitsPrepared(sta,lay,it);
        }
      }// layer
    } // station
  }

  template<class Hit>
  void TStationHitManager<Hit>::prepareOTHitsInWindow( const IStationSelector & selector )
  {
    for (TStationID sta=0; sta<this->maxStations(); ++sta) {
      for (TLayerID lay=0; lay<this->maxLayers(); ++lay) {
        for (OTRegionID ot=0; ot<this->maxOTRegions(); ++ot) {
          // The z value for this region
          const double z = this->otHitCreator()->region(sta,lay,ot)->z();
          // get the search window
          XYSearchWindow win = selector.searchWindow(z);
          // get the hits in the search window
          Tf::OTHitRange othits = this->otHitCreator()->hits(sta,lay,ot,
                                                             win.minX(),win.maxX(),
                                                             win.minY(),win.maxY() );
          processRange ( othits, sta, lay, ot );
          this->setAllHitsPrepared(sta,lay,ot);
        }
      }// layer
    } // station
  }
#endif

} // end Tf namespace

#endif // TFTOOLS_TSTATIONHITMANAGER_H
