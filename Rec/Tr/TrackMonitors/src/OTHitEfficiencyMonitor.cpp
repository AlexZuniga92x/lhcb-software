#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/ToolHandle.h"
#include "GaudiAlg/GaudiHistoAlg.h"
#include "Event/Track.h"
#include "Event/FitNode.h"
#include "Event/OTMeasurement.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrackInterfaces/ITrackExtrapolator.h"
#include "TrackInterfaces/ITrackInterpolator.h"
#include "OTDet/DeOTDetector.h"
#include "OTDet/DeOTModule.h"
#include "DetDesc/StaticArray.h"
#include "OTDAQ/IOTRawBankDecoder.h"
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram1D.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

namespace {

  struct LayerEfficiencyHistograms
  {
    AIDA::IProfile2D* m_2DHitEfficiency ;
    AIDA::IProfile2D* m_2DHotEfficiency ;
    //AIDA::IHistogram2D* m_2DHitOccupancy;  
    AIDA::IHistogram2D* m_2DExpOccupancy;  
    //AIDA::IHistogram2D* m_2DHotOccupancy;

    
    LayerEfficiencyHistograms(GaudiHistoAlg& alg,  size_t LayerID ) {
      // construct a directory name
      std::string Layername = std::string("Layer") + boost::lexical_cast<std::string>( LayerID ) + "/" ;
      m_2DHitEfficiency = alg.bookProfile2D( Layername + "2DHitEfficiency", "2DHitEfficiency", 
                                             -3157.0, 3157.0, 74, -2800,2800,100);
      m_2DHotEfficiency = alg.bookProfile2D( Layername + "2DHotEfficiency", "2DHotEfficiency",
                                             -3157.0, 3157.0, 74, -2800,2800,100);

      //m_2DHitOccupancy = alg.book2D( Layername + "2DHitOccupancy", "2D Hit Occupancy",-3157.0, 3157.0, 74, -2800,2800,100); 
      m_2DExpOccupancy = alg.book2D( Layername + "2DExpOccupancy", "2D Exp Occupancy", 
                                     -3157.0, 3157.0, 74, -2800,2800,100);
      //m_2DHotOccupancy = alg.book2D( Layername + "2DHotOccupancy", "2D Hot Occupancy", -3157.0, 3157.0, 74, -2800,2800,100);
    }
  };
  
  struct ModuleEfficiencyHistograms
  {
    AIDA::IProfile1D* m_effvsmonocoord ;
    AIDA::IProfile1D* m_effvsdist ;
    AIDA::IProfile1D* m_receffvsmonocoord ;
    AIDA::IProfile1D* m_effvsyfrac ;
    AIDA::IProfile1D* m_receffvsyfrac ;
    AIDA::IHistogram2D* m_clustersize ;
    AIDA::IHistogram1D* m_deltatdc ;
    
    ModuleEfficiencyHistograms(GaudiHistoAlg& alg,  size_t moduleID ) {
      // construct a directory name
      std::string modulename = std::string("Module") + boost::lexical_cast<std::string>( moduleID ) + "/" ;
      m_effvsmonocoord = alg.bookProfile1D( modulename + "effvsmonocoord", 
						"hit efficiency versus coordinate in mono layer plane", -3, 3, 200) ;
      m_effvsdist     = alg.bookProfile1D( modulename + "effvsdist", 
					   "hit efficiency versus distance", -7.5, 7.5, 200) ;
      m_effvsyfrac    = alg.bookProfile1D( modulename + "effvsy", 
					   "hit efficiency versus y-coordinate in mono layer plane", -0.1, 1.1, 120) ;
      m_receffvsmonocoord = alg.bookProfile1D( modulename + "hoteffvsmonocoord", 
						"hot efficiency versus coordinate in mono layer plane", -3, 3, 200) ;
      m_receffvsyfrac    = alg.bookProfile1D( modulename + "hoteffvsy", 
					      "hot efficiency versus y-coordinate in mono layer plane", -0.1, 1.1, 120) ;
      
      m_clustersize = alg.book2D( modulename + "clustersize", 
				  "clustersize versus slope", -1,1,20,-0.5,10.5,11) ;
      m_deltatdc = alg.book1D( modulename + "deltatdc", 
			       "tdc_neighbour - tdc_central for 2-cell clusters",-128,128,256) ;
    }
  } ;
  
  struct ModuleHitMap
  {
    enum { MaxChan = 128} ;
    LHCb::OTChannelID hashit[MaxChan] ;
  } ;
  
}


class OTHitEfficiencyMonitor : public GaudiHistoAlg 
{
public:

   /** Standard construtor */
  OTHitEfficiencyMonitor( const std::string& name, ISvcLocator* pSvcLocator );

  /** Destructor */
  virtual ~OTHitEfficiencyMonitor();

  /** Algorithm initialize */
  virtual StatusCode initialize();

  /** Algorithm finalize */
  virtual StatusCode finalize();

  /** Algorithm execute */
  virtual StatusCode execute();
private:
  void setNormalization(const std::string& hisname)  ;
  // this should be moved to OTChannelID  
  enum {NumUniqueStation=3, NumUniqueLayer=12, NumUniqueModule=432, NumUniqueOtis=432*4  } ;

  inline int uniqueLayer( const LHCb::OTChannelID& channelID) {
    return  (channelID.station()-1) * 4 + channelID.layer() ;
  }
  inline int uniqueQuarter( const LHCb::OTChannelID& channelID) {
    return uniqueLayer( channelID) * 4 + channelID.quarter() ;
  }
  inline int uniqueModule( const LHCb::OTChannelID& channelID) {
    return uniqueQuarter(channelID)*9 + channelID.module() - 1 ;
  }
  inline int uniqueOtis( const LHCb::OTChannelID& channelID) {
    return uniqueModule(channelID)*4 + (channelID.straw()-1)/32 ;
  }

  void fillEfficiency(const LHCb::Track& track, const DeOTModule& module,
		      double strawpos[2], double yFrac[2], const LHCb::State& refstate ) ;
  void fillEfficiency(const LHCb::Track& track, const DeOTModule& module, const LHCb::State& state) ;
  void fillEfficiency(const LHCb::Track& track, const DeOTModule& module,const LHCb::FitNode* nodeA,const LHCb::FitNode* nodeB) ;
  void fillEfficiency(const LHCb::Track& track) ;
private:
  ToolHandle<IOTRawBankDecoder> m_decoder ;
  ToolHandle<ITrackInterpolator> m_interpolator ;
  ToolHandle<ITrackExtrapolator> m_linearextrapolator ;
  const DeOTDetector* m_otdet ;
  std::string m_trackLocation;
  size_t m_minOTHitsPerTrack ;
  double m_maxChi2PerDoF ;
  double m_maxDistError ;

  // event data. we'll keep it such that we don't have to move it around.
  ModuleHitMap m_moduleHitMap[NumUniqueModule] ;

  // booked histograms
  AIDA::IProfile1D* m_efficiencyPerModulePr ;
  AIDA::IProfile1D* m_efficiencyPerOtisPr ;
  std::vector<ModuleEfficiencyHistograms*> m_moduleHistograms ;
  std::vector<LayerEfficiencyHistograms*> m_layerHistograms ;


} ;

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( OTHitEfficiencyMonitor );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
OTHitEfficiencyMonitor::OTHitEfficiencyMonitor( const std::string& name,
						ISvcLocator* pSvcLocator)
  : GaudiHistoAlg( name , pSvcLocator ),
    m_decoder("OTRawBankDecoder"),
    m_interpolator("TrackInterpolator"),
    m_linearextrapolator("TrackLinearExtrapolator")
{
  declareProperty( "TrackLocation", m_trackLocation = LHCb::TrackLocation::Default  );
  declareProperty( "MinOTHits",m_minOTHitsPerTrack = 5   ) ;
  declareProperty( "MaxChi2PerDoF",m_maxChi2PerDoF = 2   ) ;
  declareProperty( "MaxDistError", m_maxDistError = 0.2 ) ;
  declareProperty( "RawBankDecoder",m_decoder ) ;
}

//=============================================================================
// Destructor
//=============================================================================
OTHitEfficiencyMonitor::~OTHitEfficiencyMonitor() 
{
}


//=============================================================================
// Initialization
//=============================================================================
StatusCode OTHitEfficiencyMonitor::initialize() 
{
  StatusCode sc = GaudiHistoAlg::initialize(); // must be executed first
  if(sc.isFailure()) return sc ;
  sc = m_decoder.retrieve() ;
  if(sc.isFailure()) return sc ;
  sc = m_interpolator.retrieve() ;
  if(sc.isFailure()) return sc ;
  sc = m_linearextrapolator.retrieve() ;
  if(sc.isFailure()) return sc ;

  m_otdet = getDet<DeOTDetector>(DeOTDetectorLocation::Default );

  // book all histograms
  setHistoTopDir("OT/") ;
  m_efficiencyPerModulePr = bookProfile1D( "efficiencyVsModule","efficiency per module",
					   -0.5,NumUniqueModule-0.5,NumUniqueModule) ;
  m_efficiencyPerOtisPr = bookProfile1D( "efficiencyVsOtis","efficiency per otis",
					 -0.5,NumUniqueOtis-0.5,NumUniqueOtis) ;
  m_moduleHistograms.reserve(9) ;
  m_layerHistograms.reserve(12) ;

  for(int i=0; i<9; ++i)
    m_moduleHistograms.push_back( new ModuleEfficiencyHistograms(*this,i+1) ) ;
  
  for(int i=0; i<12; ++i)
    m_layerHistograms.push_back( new LayerEfficiencyHistograms(*this, i) );
  

  return sc;
}

//=============================================================================
// Initialization
//=============================================================================
StatusCode OTHitEfficiencyMonitor::finalize() 
{
  // here we should create some 'summary' histograms
  
  // the delete the structure that holds the histograms
  BOOST_FOREACH ( ModuleEfficiencyHistograms* hist, m_moduleHistograms ) delete hist ;
  BOOST_FOREACH ( LayerEfficiencyHistograms* hist, m_layerHistograms ) delete hist ;

  
  // rescale the occupancy histogram
  m_decoder.release().ignore() ;
  m_interpolator.release().ignore() ;
  m_linearextrapolator.release().ignore() ;
  return GaudiHistoAlg::finalize() ;
}

//=========================================================================
// 
//=========================================================================
StatusCode OTHitEfficiencyMonitor::execute()
{ 
  // first clear the hit map
  for(size_t imod=0; imod<NumUniqueModule; ++imod)
    for(size_t istraw=0; istraw<ModuleHitMap::MaxChan; ++istraw)
      m_moduleHitMap[imod].hashit[istraw] = 0 ;
  
  // decode all OT hits
  LHCb::OTLiteTimeContainer ottimes;
  StatusCode sc = m_decoder->decode( ottimes ) ;

  if( sc ) {
    // fill all the hits in the hit map
    for( LHCb::OTLiteTimeContainer::const_iterator ihit = ottimes.begin() ;
	 ihit != ottimes.end(); ++ihit) 
      m_moduleHitMap[ uniqueModule( ihit->channel() ) ].hashit[  ihit->channel().straw()-1 ] = ihit->channel() ;
    
    // now loop over the tracks
    LHCb::Track::Range tracks = get<LHCb::Track::Range>(m_trackLocation) ;
    BOOST_FOREACH(const LHCb::Track* track, tracks) {
      if( track->fitStatus() == LHCb::Track::Fitted && track->nDoF()>1 &&
	  track->chi2PerDoF() < m_maxChi2PerDoF ) {
	// count the number
	size_t sumn(0) ;
	LHCb::Track::ConstNodeRange nodes = track->nodes() ;
	for( LHCb::Track::ConstNodeRange::const_iterator inode = nodes.begin() ;
	     inode != nodes.end(); ++inode ) 
	  if( (*inode)->type() == LHCb::Node::HitOnTrack
	      && (*inode)->measurement().type() == LHCb::Measurement::OT ) ++sumn ;
	if( sumn >= m_minOTHitsPerTrack ) 
	  fillEfficiency(*track) ;
      }
    }
  }
  return sc ;
}

void OTHitEfficiencyMonitor::fillEfficiency( const LHCb::Track& track,
					     const DeOTModule& module,
					     double strawpos[2], double yfrac[2],
					     const LHCb::State& refstate)
{
  // make a temporary structure that tells which hits are in this
  // module. if this takes time, then we should do it once per event
  LHCb::OTChannelID modid = module.elementID() ;
  
  double pitch = module.xPitch() ;
  int uniquemodule  = uniqueModule(modid) ;
    
  size_t nstraws = module.nChannels() ;
  
  ModuleEfficiencyHistograms* modulehist = m_moduleHistograms[module.elementID().module()-1] ;
  LayerEfficiencyHistograms* layerhist = m_layerHistograms[module.elementID().uniqueLayer()-4] ;
  
  // compute the direction in the local frame. this can be done a lot more efficient, but okay:
  Gaudi::XYZVector localslopes = module.geometry()->toLocal( refstate.slopes() ) ;
  double localTx  = localslopes.x()/localslopes.z() ;
  double cosalpha = 1/std::sqrt( 1+localTx*localTx) ;

  for(size_t imono = 0; imono<2 ; ++imono ) {
    // find the hits in a +- 3 straw window
    int monooffset = imono * nstraws/2 - 1 ;
    int minstraw = std::max(int(strawpos[imono]) - 2,1) ;
    int maxstraw = std::min(int(strawpos[imono]) + 3,int(nstraws/2)) ;

    for( int istraw = minstraw; istraw<=maxstraw; ++istraw) {
      LHCb::OTChannelID channel = m_moduleHitMap[ uniquemodule ].hashit[ istraw + monooffset ] ;
      bool foundhit = channel != 0 ;
      bool foundhot = foundhit && track.isOnTrack( LHCb::LHCbID( channel ) ) ;
      double monocoord = strawpos[imono] - istraw ;
      double dstraw    = monocoord*pitch*cosalpha  ;
      modulehist->m_effvsdist->fill(dstraw,foundhit) ;
      modulehist->m_effvsmonocoord->fill(monocoord,foundhit) ;
      modulehist->m_receffvsmonocoord->fill(monocoord,foundhot) ;
      if( std::abs(dstraw) < 1.25 ) {
      modulehist->m_effvsyfrac->fill(yfrac[imono],foundhit) ;
      modulehist->m_receffvsyfrac->fill(yfrac[imono],foundhot) ;
      m_efficiencyPerModulePr->fill( uniquemodule, foundhit ) ;	
      m_efficiencyPerOtisPr->fill( uniquemodule*4 + (istraw+monooffset-1)/32, foundhit ) ;

      //Filling the 2D expected occupancy, hiteff and hoteff
      layerhist->m_2DExpOccupancy->fill(refstate.x(),refstate.y());
      layerhist->m_2DHitEfficiency->fill(refstate.x(),refstate.y(),foundhit);
      layerhist->m_2DHotEfficiency->fill(refstate.x(),refstate.y(),foundhot);

      //Filling a 2D histo with the multiplicity of the hits
      //if (foundhit){
      //layerhist->m_2DHitOccupancy->fill(refstate.x(),refstate.y());
      //}

      //if(foundhot){
      //layerhist->m_2DHotOccupancy->fill(refstate.x(),refstate.y());    
      //}
      
      }
    }
    // monitor 'cluster size'. cluster size is zero if there is no hit in 'closest straw'.
    int closeststraw = int( strawpos[imono] + 0.5 ) ;
    if( 1<=closeststraw && closeststraw <=int(nstraws/2) ) {
      StaticArray<LHCb::OTChannelID,128> clusterhits ;
      LHCb::OTChannelID tmp ;
      for( int istraw = closeststraw; 
	   istraw<=int(nstraws/2) && (tmp = m_moduleHitMap[ uniquemodule ].hashit[ istraw + monooffset ]) ;
	   ++istraw) clusterhits.push_back( tmp ) ;
      for( int istraw = closeststraw-1; 
	   istraw>=1 && (tmp = m_moduleHitMap[ uniquemodule ].hashit[ istraw + monooffset ]) ;
	   --istraw) clusterhits.push_back( tmp ) ;
      modulehist->m_clustersize->fill( localTx, clusterhits.size() ) ;
      if( clusterhits.size() ==2 ) 
	modulehist->m_deltatdc->fill( double(clusterhits[1].tdcTime()) - double(clusterhits[0].tdcTime()) ) ;
    }
  }
}

void OTHitEfficiencyMonitor::fillEfficiency(const LHCb::Track& track,
					    const DeOTModule& module,
					    const LHCb::State& state )
{
  double strawpos[2], yfrac[2] ;
  for(size_t imono = 0 ; imono<2; ++imono) 
    module.monoLayerIntersection( imono,
				  state.position(),
				  state.tx(),
				  state.ty(),
				  strawpos[imono], yfrac[imono] ) ;
  fillEfficiency(track,module,strawpos,yfrac,state) ;
}

void OTHitEfficiencyMonitor::fillEfficiency(const LHCb::Track& track,
					 const DeOTModule& module,
					 const LHCb::FitNode* nodeMonoA,
					 const LHCb::FitNode* nodeMonoB)
{
  double strawpos[2], yfrac[2] ;
  const LHCb::State* refstate(0) ;
#define SAFEBUTSLOW
#ifdef SAFEBUTSLOW
  // this is the expensive method of getting two unbiased straw positions:
  LHCb::State states[2] ;
  states[0] = nodeMonoA ? nodeMonoA->unbiasedState() : nodeMonoB->state() ;
  states[1] = nodeMonoB ? nodeMonoB->unbiasedState() : nodeMonoA->state() ;
  for(size_t imono = 0 ; imono<2; ++imono) 
    module.monoLayerIntersection( imono,
				  states[imono].position(),
				  states[imono].tx(),
				  states[imono].ty(),
				  strawpos[imono], yfrac[imono] ) ;
  refstate = &(states[1]) ;
#else
  // but there is a second method that is much faster. it is just slightly harder toget things right:
  const LHCb::FitNode* nodes[2]={ 0,0};
  nodes[0] = nodeMonoA ;
  nodes[1] = nodeMonoB ;
  for(size_t imono=0; imono<2; ++imono) {
    const LHCb::FitNode* node = nodes[imono] ; 
    const LHCb::State& state = node ? node->state() : nodes[(imono+1)%2]->state() ;
    refstate = &state ;
    module.monoLayerIntersection( imono,
                                  state.position(),
                                  state.tx(),
                                  state.ty(),
                                  strawpos[imono], yfrac[imono] ) ;
    if( node ) { // okay, need to unbias 
      const LHCb::OTMeasurement* meas =
        static_cast<const LHCb::OTMeasurement*>(&(node->measurement())) ;
      int thisstraw = meas->channel().straw() % (module.nChannels()/2) ;
      double biaseddoca     = node->doca() ;
      double measurement    = node->residual() + biaseddoca ;
      double unbiaseddoca   = measurement - node->unbiasedResidual() ;
      double biaseddstraw   = strawpos[imono] - thisstraw ;
      double unbiaseddstraw = biaseddstraw * unbiaseddoca / biaseddoca ;
      double unbiasedstraw  = thisstraw + unbiaseddstraw ;
      strawpos[imono] = unbiasedstraw ;
    }
  }
#endif  
  fillEfficiency(track,module,strawpos,yfrac,*refstate) ;
}


void OTHitEfficiencyMonitor::fillEfficiency(const LHCb::Track& track)
{
  // first collect all modules that are already on the track. for these we need to unbias.
  typedef std::pair<const LHCb::Node*, const LHCb::Node*> MonoLayerNodes ;
  typedef std::map< const DeOTModule*, MonoLayerNodes > ModulesOnTrack ;
  ModulesOnTrack modulesOnTrack ;
  
  LHCb::Track::ConstNodeRange nodes = track.nodes() ;
  for( LHCb::Track::ConstNodeRange::const_iterator inode = nodes.begin() ;
       inode != nodes.end(); ++inode ) 
    if( (*inode)->type() == LHCb::Node::HitOnTrack &&
	(*inode)->measurement().type() == LHCb::Measurement::OT ) { 
      const LHCb::OTMeasurement* meas =
	static_cast<const LHCb::OTMeasurement*>(&(*inode)->measurement()) ;
      const DeOTModule* module = &(meas->module()) ;
      
      // only take the closest node in a mono-layer. we can anyway only unbias 1.
      int mono = (meas->channel().straw()-1) / (module->nChannels()/2) ;
      const LHCb::Node*& nodeInModule = mono == 0 ? 
	modulesOnTrack[ module ].first :  modulesOnTrack[ module ].second ;
      if( nodeInModule == 0 ||
	  std::abs(nodeInModule->residual()) >
	  std::abs((*inode)->residual()) ) nodeInModule = *inode ;
      
      //modulesOnTrack[ module ].push_back( *inode ) ;
    }

  // process the efficiency for these module. make sure to unbias the
  // state when necessary. keep track of the layers that we have seen,
  // such that we can skip them below
  std::set< const DeOTLayer* > layersOnTrack ;
  for( ModulesOnTrack::const_iterator imod = modulesOnTrack.begin() ; 
       imod != modulesOnTrack.end(); ++imod ) {
    layersOnTrack.insert( m_otdet->findLayer( imod->first->elementID() ) ) ;

    const LHCb::FitNode* nodeMonoA = static_cast<const LHCb::FitNode*>( imod->second.first ) ;
    const LHCb::FitNode* nodeMonoB = static_cast<const LHCb::FitNode*>( imod->second.second ) ;
    fillEfficiency(track, *(imod->first), nodeMonoA, nodeMonoB ) ;
  }
  
  // cache the z-positions of the layers
  static std::vector<double> layerzpos ;
  if( layerzpos.empty() ) {
    for( DeOTDetector::Layers::const_iterator ilay = m_otdet->layers().begin() ;
	 ilay !=  m_otdet->layers().end(); ++ilay ) 
      layerzpos.push_back((*ilay)->geometry()->toGlobal(Gaudi::XYZPoint(0.0, 0.0, 0.0)).z()) ;
  }
  
  // now find the modules in the missing planes
  LHCb::State state ;
  int layindex(0) ;
  std::vector< std::pair< const DeOTModule*, LHCb::State > > modulesNotOnTrack ;
  for( DeOTDetector::Layers::const_iterator ilay = m_otdet->layers().begin() ;
       ilay !=  m_otdet->layers().end(); ++ilay, ++layindex ) 
    if( layersOnTrack.find( *ilay ) == layersOnTrack.end() ) {
      // propagate to the z-coordinate of this layer
      m_interpolator->interpolate( track, layerzpos[layindex] , state ).ignore();
      
      // only consider if state has reasonably small error. we should
      // still fine-tune this a little bit, make a propor projecion
      // etc. the best is to cut only when you have unbiased the
      // state. cutting to tight will introduce a significant bias in
      // the efficiency measurement.
      if( std::sqrt( state.covariance()(0,0) ) < m_maxDistError ) {
	
	// to get a slightly more precise answer, intersect with the plane
	double tolerance = 0.1 ;
	m_linearextrapolator->propagate( state, (*ilay)->plane(), tolerance).ignore() ;
	
	// get a single module. this goes wrong if there are large misalignments!
	const DeOTModule* module = m_otdet->findModule(state.position());
	
	if(module ) {
	  // fill the efficiency
	  fillEfficiency(track, *module, state ) ;	  
			  
	  // keep track of what we have seen
	  modulesNotOnTrack.push_back( std::make_pair( module, state ) ) ;
	}
      }
    }

}
