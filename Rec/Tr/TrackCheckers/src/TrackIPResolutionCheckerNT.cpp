// Include files
#include <map>

// from Gaudi
#include "GaudiAlg/GaudiTupleAlg.h"
#include "TrackInterfaces/ITrackSelector.h"

#include "GaudiUtils/Aida2ROOT.h"

// from Event
#include "Event/MCHeader.h"
#include "Event/Track.h"
#include "Event/RecVertex.h"
#include "Event/State.h"
#include "Kernel/LHCbID.h"
#include "Event/MCParticle.h"
#include "Event/MCHit.h"
#include "Event/TrackFitResult.h"
#include "Kernel/HitPattern.h"
#include "Linker/LinkedTo.h"

#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "LHCbMath/ValueWithError.h"

#include "TrackKernel/CubicStateInterpolationTraj.h"

#include "TH2.h"
#include "TH1.h"
#include "TF1.h"

// Boost
#include <boost/foreach.hpp>

/** @class TrackIPResolutionCheckerNT TrackIPResolutionCheckerNT.cpp "TrackCheckers/TrackIPResolutionCheckerNT"
 *
 * Class for making tuples for the TrackIP tool
 *  @author W. Hulsbergen and P. Tsopelas
 *  @date   21-01-2013
 */

class TrackIPResolutionCheckerNT : public GaudiTupleAlg {
    
public:
    
    /** Standard construtor */
    TrackIPResolutionCheckerNT( const std::string& name, ISvcLocator* pSvcLocator );
    
    /** Destructor */
    virtual ~TrackIPResolutionCheckerNT();
    
    /** Algorithm execute */
    virtual StatusCode execute();
    
    /** Algorithm initialize */
    virtual StatusCode initialize();
    
    /** Algorithm finalize */
    
    
private:
    
private:
    std::string m_tracklocation ;    ///< Input Tracks container location
    std::string m_linkerlocation ;
    std::string mchitlocation;
    std::string m_pvlocation ;
    int m_minNumTracksReconstructablePV ;
    void createResolutionProfile( const HistoID& inputname, const HistoID& outputname ) ;
};

DECLARE_ALGORITHM_FACTORY( TrackIPResolutionCheckerNT )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackIPResolutionCheckerNT::TrackIPResolutionCheckerNT(const std::string& name,
                                                       ISvcLocator* pSvcLocator ) :
GaudiTupleAlg( name , pSvcLocator )
{
    declareProperty( "TrackContainer", m_tracklocation = LHCb::TrackLocation::Velo );
    declareProperty( "LinkerLocation", m_linkerlocation ) ;
    declareProperty( "MCHitContainer", mchitlocation = "/Event/MC/Velo/Hits");
    declareProperty( "PVContainer", m_pvlocation = LHCb::RecVertexLocation::Primary ) ;
    declareProperty( "MinNumTracksReconstructablePV", m_minNumTracksReconstructablePV = 5 ) ;
}

//=============================================================================
// Destructor
//=============================================================================
TrackIPResolutionCheckerNT::~TrackIPResolutionCheckerNT() {}

//=============================================================================
// Initialization. Check parameters
//=============================================================================
StatusCode TrackIPResolutionCheckerNT::initialize()
{
    // Mandatory initialization of GaudiAlgorithm
    StatusCode sc = GaudiTupleAlg::initialize();
    if ( sc.isFailure() ) { return sc; }
    if( m_linkerlocation.empty() ) m_linkerlocation = m_tracklocation ;
    return StatusCode::SUCCESS;
}

namespace {
    
    const LHCb::MCVertex* findpv(const LHCb::MCParticle& p)
    {
        const LHCb::MCVertex* vertex = p.originVertex() ;
        if( vertex && !vertex->isPrimary() ) {
            const LHCb::MCParticle* mother = vertex->mother() ;
            // if the mother origin vertex is less than 1 micron away from this vertex, then we also consider the mother's vertex as origin.
            const double tolerance = 1e-6 ; // 1 micron^2
            if( mother && mother->originVertex() &&
               (mother->originVertex() == vertex ||
                (mother->originVertex()->position() - vertex->position()).Mag2() < tolerance ) ) // less than 1 micron
            {
                vertex = findpv( *mother ) ;
            } else {
                vertex = 0 ;
            }
        }
        return vertex ;
    }
    
    const LHCb::MCVertex* findMCOriginVertex( const LHCb::MCParticle& particle,
                                             double decaylengthtolerance)
    {
        // take this particle, walk up the decay tree and determine its originvertex
        const LHCb::MCVertex* originvertex = particle.originVertex() ;
        if( originvertex ) {
            const LHCb::MCParticle* mother = originvertex->mother() ;
            if( mother && mother != &particle) {
                const LHCb::MCVertex* motheroriginvertex =
                mother->originVertex() ;
                if( motheroriginvertex &&
                   ( motheroriginvertex == originvertex ||
                    ((motheroriginvertex->position() -
                      originvertex->position()).R()) < decaylengthtolerance) )
                    originvertex = findMCOriginVertex( *mother,
                                                      decaylengthtolerance ) ;
            }
        }
        return originvertex ;
    }
    
    int mcVertexType(const LHCb::MCVertex& vertex)
    {
        int rc(-1);
        if( vertex.isPrimary() ) rc=0 ;
        else if(vertex.mother()) {
            const LHCb::MCParticle* mother =  vertex.mother() ;
            if( mother->particleID().hasBottom() ) rc = 3 ;
            else if( mother->particleID().hasCharm() ) rc = 2 ;
            else if( mother->particleID().hasStrange() ) rc = 1 ;
            else rc = 4;
        }
        return rc ;
    }
    
    struct ForwardMCHitSorter
    {
        // copy constructor & destructor made by default
        ForwardMCHitSorter(){}
        bool operator()( const LHCb::MCHit* lhs, const LHCb::MCHit* rhs)
        {
            return lhs->entry().z() < rhs->entry().z() ;
        }
    } ;
    struct BackwardMCHitSorter
    {
        BackwardMCHitSorter(){}
        bool operator()( const LHCb::MCHit* lhs, const LHCb::MCHit* rhs)
        {
            return rhs->entry().z() < lhs->entry().z() ;
        }
    } ;
    
} // end of empty namespace


//=============================================================================
// Execute
//=============================================================================
StatusCode TrackIPResolutionCheckerNT::execute()
{
    // get the list of tracks
    const LHCb::Track::Range tracks = get<LHCb::Track::Range>(m_tracklocation);
    
    // get the list of reconstructed PVs as well
    const LHCb::RecVertex::Range pvs = get<LHCb::RecVertex::Range>(m_pvlocation) ;
    
    // get the linker table track -> mcparticle
    LinkedTo<LHCb::MCParticle> linker( evtSvc(), msgSvc(), m_linkerlocation );
    
    // create the list of true PVs. count how many reconstructed tracks in each PV.
    std::map< const LHCb::MCVertex*, int > truepvs ;
    
    // create the ntuple
    Tuple theTuple = nTuple( "tracks" , "" , CLID_ColumnWiseTuple );
    
    // create a map from all MCParticles to MCHits
    typedef std::vector<const LHCb::MCHit*> MCHitVector ;
    typedef std::map<const LHCb::MCParticle *, MCHitVector > MCHitMap ;
    MCHitMap mchitmap ;
    
    const LHCb::MCHits* hits(0) ;
    if( exist<LHCb::MCHits*>(mchitlocation) )
        hits = get<LHCb::MCHits>( mchitlocation ) ;
    
    if ( hits ){
        BOOST_FOREACH( const LHCb::MCHit* mchit, *hits ) {
            if( mchit->mcParticle() )
                mchitmap[ mchit->mcParticle() ].push_back( mchit ) ;
        }
    }
    
    // we also just like to have the total number of true PVs
    const LHCb::MCHeader* mcheader = get<const LHCb::MCHeader*>(LHCb::MCHeaderLocation::Default) ;

    int itrack=0 ;
    
    BOOST_FOREACH(const LHCb::Track* track, tracks) {
        
        // keep track of track multiplicity
        theTuple->column("itrack",int(itrack)) ;
        ++itrack ;
        theTuple->column("ntrack",int(tracks.size())) ;
        theTuple->column("numtruePV",int(mcheader->numOfPrimaryVertices())) ;
        
        //tree->Draw("ntrack","itrack==0") ;
        //tree->Draw("numrecPV","itrack==0") ;
        
        
        double x = track->firstState().x() ;
        double y = track->firstState().y() ;
        double z = track->firstState().z() ;
        double tx = track->firstState().tx() ;
        double ty = track->firstState().ty() ;
        
        theTuple->column("probChi2", track->probChi2() );
        theTuple->column("chi2",track->chi2() );
        theTuple->column("ndof",track->nDoF() );
        theTuple->column("type",track->type() ) ;
        theTuple->column("x", x ) ;
        theTuple->column("y", y ) ;
        theTuple->column("z", z ) ;
        theTuple->column("tx", tx ) ;
        theTuple->column("ty", ty ) ;
        theTuple->column("qop", track->firstState().qOverP() ) ;
        theTuple->column("veloChi2", track->info(LHCb::Track::FitVeloChi2,0)) ;
        theTuple->column("veloNdof", track->info(LHCb::Track::FitVeloNDoF,0)) ;
        theTuple->column("TChi2", track->info(LHCb::Track::FitTChi2,0)) ;
        theTuple->column("TNdof", track->info(LHCb::Track::FitTNDoF,0)) ;
        theTuple->column("backward",track->checkFlag( LHCb::Track::Backward)) ;
        
        LHCb::HitPattern hitpattern(track->lhcbIDs()) ;
        theTuple->column("numVeloStations",hitpattern.numVeloStations()) ;
        theTuple->column("numVeloStationsOverlap",hitpattern.numVeloStationsOverlap()) ;
        theTuple->column("numITStationsOverlap",hitpattern.numITStationsOverlap()) ;
        theTuple->column("numITOTStationsOverlap",hitpattern.numITOTStationsOverlap()) ;
        theTuple->column("numVeloHoles",hitpattern.numVeloHoles()) ;
        theTuple->column("numTHoles",hitpattern.numTHoles()) ;
        theTuple->column("numTLayers",hitpattern.numTLayers()) ;
        theTuple->column("numVeloStations",hitpattern.numVeloStations()) ;
        theTuple->column("numVeloClusters",hitpattern.numVeloR()+hitpattern.numVeloPhi()) ;
        theTuple->column("numhits",track->lhcbIDs().size()) ;
        
        // find the closest PV, again using the minimal distance
        
        const LHCb::State& state = track->firstState() ;
        LHCb::CubicStateInterpolationTraj tracktraj( state, Gaudi::XYZVector() ) ;
        Gaudi::XYZPoint trkpos( state.position() ) ;
        Gaudi::XYZVector trkdir( state.slopes().Unit() ) ;
        
        int trackWasFitted = track->fitResult() && !track->fitResult()->nodes().empty() ;
        theTuple->column("trackWasFitted", trackWasFitted);
        
        // We also want to monitor the reconstructed IP, so the IP with respect to the reconstructed PVs.
        theTuple->column("numrecPV", pvs.size() ) ;
        const LHCb::RecVertex* recpv(0) ;
        double bestip2(0);
        
        BOOST_FOREACH(const LHCb::RecVertex* thispv, pvs) {
            Gaudi::XYZVector dx = trkpos - thispv->position()  ;
            Gaudi::XYZVector delta = dx - trkdir * dx.Dot(trkdir) ;
            double ip2 = delta.Mag2() ;
            if( recpv==0 || ip2 < bestip2 ) {
                bestip2 = ip2 ;
                recpv = thispv ;
            }
        } // end of BOOST over pvs
        
        if (recpv) {
            LHCb::State stateAtVtx = tracktraj.state( recpv->position().z() ) ;
            // now compute the errors. this isn't quite right because:
            // - PV is biased
            // - correction for Z error is approximate
            double tx = stateAtVtx.tx() ;
            double recipxerr = std::sqrt(state.covariance()(0,0) + recpv->covMatrix()(0,0) +
                                         2*tx*recpv->covMatrix()(0,2)+ tx*tx * recpv->covMatrix()(2,2)) ;
            double ty = stateAtVtx.ty() ;
            double recipyerr = std::sqrt(state.covariance()(1,1) + recpv->covMatrix()(1,1) +
                                         2*ty*recpv->covMatrix()(1,2)+ ty*ty * recpv->covMatrix()(2,2)) ;
            
            
            theTuple->column("recIP3D", std::sqrt(bestip2) ) ;
            theTuple->column("recIPx", stateAtVtx.x() - recpv->position().x() ) ;
            theTuple->column("recIPy", stateAtVtx.y() - recpv->position().y() ) ;
            theTuple->column("recIPxerr", recipxerr ) ;
            theTuple->column("recIPyerr", recipyerr ) ;
        } // end of recpv check
        
        // now do things linked to MC truth
        const LHCb::MCParticle* mcparticle = linker.first(track) ;
        bool hasMCMatch = mcparticle && mcparticle->originVertex() ;
        theTuple->column("waslinked",hasMCMatch) ;
        
        // The "typeofprefix" tag is used to see which particles come from a True Primary Vertex,
        // not a True Primary vertex or whether they are ghosts.
        bool isFromPV = hasMCMatch && mcparticle->originVertex()->isPrimary() ;
        double typeofprefix;
        typeofprefix = isFromPV ? 0 : (hasMCMatch ? 1 : 2) ;
        theTuple->column("typeofprefix", typeofprefix) ;
        
        
        int mcOVT = -1 ; // set to -1 for ghosts
        if( hasMCMatch ) {
            Gaudi::XYZPoint trueorigin = mcparticle->originVertex()->position() ;
            const LHCb::MCParticle* mother = mcparticle->originVertex()->mother() ;
            const LHCb::MCVertex* UltOrigVertex =findMCOriginVertex(mcparticle, 1e-3) ;
            const LHCb::MCParticle* ulmother = UltOrigVertex ? UltOrigVertex->mother() : 0 ;
            mcOVT = mcVertexType( UltOrigVertex ) ;
            theTuple->column("truepid", mcparticle->particleID().pid() ) ;
            theTuple->column("ulmotherHasBottom", ulmother && ulmother->particleID().hasBottom() ? 1 : 0 );
            theTuple->column("ulmotherHasCharm", ulmother && ulmother->particleID().hasCharm() ? 1 : 0 );
            theTuple->column("ulmotherHasStrange", ulmother && ulmother->particleID().hasStrange() ? 1 : 0 );
            theTuple->column("truemotherpid", mother ? mother->particleID().pid() : 0 ) ;
            theTuple->column("truevertexpid", ulmother ? ulmother->particleID().pid() : 0 ) ;
            theTuple->column("truex",trueorigin.x()) ;
            theTuple->column("truey",trueorigin.y()) ;
            theTuple->column("truez",trueorigin.z()) ;
            theTuple->column("truemom",mcparticle->p()/ Gaudi::Units::GeV) ;
            theTuple->column("truepx", mcparticle->momentum().Px()/ Gaudi::Units::GeV ) ;
            theTuple->column("truepy", mcparticle->momentum().Py()/ Gaudi::Units::GeV ) ;
            theTuple->column("truepz", mcparticle->momentum().Pz()/ Gaudi::Units::GeV ) ;
            theTuple->column("truept", mcparticle->momentum().Pt() / Gaudi::Units::GeV ) ;
            theTuple->column("trueeta",mcparticle->momentum().eta()) ;
            theTuple->column("truephi",mcparticle->momentum().phi()) ;
            
            // we can use the CubicStateInterpolationTraj (or later tracktraj)
            // to do the error propagation. it is just a line!
            LHCb::State state = tracktraj.state( trueorigin.z() ) ;
            double tx = state.tx() ;
            double ty = state.ty() ;
            double IPx = state.x() - trueorigin.x() ;
            double IPy = state.y() - trueorigin.y() ;
            double IP3D = std::sqrt( (IPx*IPx+IPy*IPy)/(1 + tx*tx + ty*ty) ) ;
            theTuple->column("IPx",IPx) ;
            theTuple->column("IPy",IPy) ;
            theTuple->column("IP3D",IP3D);
            theTuple->column("IPxerr", std::sqrt( state.covariance()(0,0) ) );
            theTuple->column("IPyerr", std::sqrt( state.covariance()(1,1) ) ) ;
            
            
            if( hits ) {
                // store the z-position of the first MC hit
                MCHitVector& mchits = mchitmap[mcparticle] ;
                if( mcparticle->momentum().Pz() > 0)
                    std::sort( mchits.begin(), mchits.end(), ForwardMCHitSorter() ) ;
                else
                    std::sort( mchits.begin(), mchits.end(), BackwardMCHitSorter() ) ;
                
                theTuple->column("nummchits",mchits.size()) ;
                const LHCb::MCHit* mchit = mchits.front() ;
                const LHCb::MCHit* mchitL = mchits.back() ;
                Gaudi::XYZPoint poshit = mchit->entry() ;
                theTuple->column("edep",mchit->energy());
                theTuple->column("firstmchitx",poshit.x()) ;
                theTuple->column("firstmchity",poshit.y()) ;
                theTuple->column("firstmchitz",poshit.z()) ;
                theTuple->column("firstmchitdz",mchit->displacement().z()) ;
                theTuple->column("lastmchitz",mchitL->entry().z()) ;
                theTuple->column("truetxfirsthit",mchit->dxdz()) ;
                theTuple->column("truetyfirsthit",mchit->dydz()) ;
                double dz = poshit.z() - z ;
                theTuple->column("IPxfirsthit",(x + dz * tx) - poshit.x()) ;
                theTuple->column("IPyfirsthit",(y + dz * ty) - poshit.y()) ;
                
                // let's now extrapolate the mchit of the first hit to the z position of the vertex,
                // as if there were no scattering
                dz = trueorigin.z() - poshit.z() ;
                double extrapolatedmchitx = poshit.x() + dz * mchit->dxdz() ;
                double extrapolatedmchity = poshit.y() + dz * mchit->dydz() ;
                
                dz = trueorigin.z() - state.z() ;
                theTuple->column("IPxfirsthitatvertex",(state.x() + dz* state.tx()) - extrapolatedmchitx) ;
                theTuple->column("IPyfirsthitatvertex",(state.y() + dz* state.ty()) - extrapolatedmchity) ;
                
            } // hits check
            
        }   // end of hasMCMatch check
        
        theTuple->column("mcOriginVertexType", mcOVT );
        
        theTuple->write();
        
	    
    } // end of tracks
    
    return StatusCode::SUCCESS ;
}
