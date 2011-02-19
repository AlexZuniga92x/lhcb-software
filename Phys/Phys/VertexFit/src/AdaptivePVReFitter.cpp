// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

#include "Event/VertexBase.h"
#include "Event/RecVertex.h"
#include "Event/ProtoParticle.h"
#include "Event/Particle.h"

#include "TrackInterfaces/ITrackExtrapolator.h"

// local
#include "AdaptivePVReFitter.h"

using namespace LHCb ;
using namespace Gaudi::Units;
//-----------------------------------------------------------------------------
// Implementation file for class : AdaptivePVReFitter
//
// 2007-11-05 : Yuehong Xie
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory

DECLARE_TOOL_FACTORY(AdaptivePVReFitter)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
AdaptivePVReFitter::AdaptivePVReFitter( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IPVReFitter>(this);

  declareProperty( "maxIter", m_maxIter = 6);
  declareProperty( "reweightTracks", m_reweightTracks = true);
  declareProperty( "temperatures", m_temperatures );
  declareProperty( "criticalPoint", m_criticalPoint = 9.0 );
  declareProperty( "maxDeltaChi2", m_maxDeltaChi2 = 0.001);
  declareProperty( "fullExtrapolatorName", 
                   m_fullExtrapolatorName = "TrackMasterExtrapolator");
  declareProperty( "veloExtrapolatorName", 
                   m_veloExtrapolatorName = "TrackLinearExtrapolator");
  declareProperty( "minTrNumber", m_minTrNumber = 4);

  m_temperatures.push_back(10.0);
  m_temperatures.push_back(4.0);
  m_temperatures.push_back(1.0);
  m_temperatures.push_back(0.001);
  m_temperatures.push_back(0.001);

}

//=============================================================================
// Destructor
//=============================================================================
AdaptivePVReFitter::~AdaptivePVReFitter() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode AdaptivePVReFitter::initialize(){
  StatusCode sc = GaudiTool::initialize();
  if (!sc) return sc;

  m_fullExtrapolator = tool<ITrackExtrapolator>( m_fullExtrapolatorName );
  m_veloExtrapolator = tool<ITrackExtrapolator>( m_veloExtrapolatorName );

  return sc;
}

//=============================================================================
// refit PV
//=============================================================================
StatusCode AdaptivePVReFitter::reFit(LHCb::VertexBase* PV) const {

  debug() <<"Now reFit PV"<< endreq;

  StatusCode sc = StatusCode::SUCCESS;

  if(!(PV->isPrimary())) {
    debug() <<"AdaptivePVReFitter is used to reFit a  non-PV"<< endreq;
    return StatusCode::FAILURE;    
  }

  RecVertex* primvtx=dynamic_cast<RecVertex*>(PV);
  if(!primvtx) return StatusCode::FAILURE;
 
  std::vector<PVFitTrack> fittracks;

  const SmartRefVector<LHCb::Track>& pvtracks = primvtx->tracks();
  SmartRefVector<LHCb::Track>::const_iterator trIt = pvtracks.begin();
  SmartRefVector<LHCb::Track>::const_iterator trItEnd = pvtracks.end();

  for( ; trItEnd != trIt; trIt++) {
    const LHCb::Track* track = *trIt;
    if(!track) continue;
    PVFitTrack aFitTrack(track);
    //    aFitTrack.setTrack(track);
    fittracks.push_back(aFitTrack);
  }

  sc = fitPV(primvtx, fittracks);

  return sc;
}
 
//=============================================================================
// remove track used for a LHCb::Particle and refit PV
//=============================================================================
StatusCode AdaptivePVReFitter::remove(const LHCb::Particle* part,  
                                      LHCb::VertexBase* PV) const {

  debug() <<"Now remove and reFit "<< endreq;

  if(!(PV->isPrimary())) {
    debug() <<"AdaptivePVReFitter is used to remove a trackfrom a  non-PV"<< endreq;
    return StatusCode::FAILURE;
  }

  RecVertex* primvtx=dynamic_cast<RecVertex*>(PV);
  if(!primvtx) return StatusCode::FAILURE;

  std::vector<const LHCb::Track*> dautracks;
  getFinalTracks(part, dautracks);

  StatusCode sc = StatusCode::SUCCESS;

  for( std::vector<const LHCb::Track*>::const_iterator idtrack = dautracks.begin();
       dautracks.end() != idtrack; idtrack++ ) {
    const LHCb::Track* dauTrack = *idtrack;
    bool isPVtr=false;
    const SmartRefVector<LHCb::Track>& pvtracks = primvtx->tracks();
    for( SmartRefVector<LHCb::Track>::const_iterator trIt = pvtracks.begin();
         pvtracks.end() != trIt; trIt++) {
      const LHCb::Track* tmptr = *trIt;
      if(tmptr==dauTrack) { isPVtr=true; break; }
    }
    if(!isPVtr) continue;
    primvtx->removeFromTracks(dauTrack);
  }
  
  sc = reFit(PV);

  return sc;

}


//=============================================================================
// fit PV from a vector of tracks
//=============================================================================
StatusCode AdaptivePVReFitter::fitPV(LHCb::RecVertex* PV, 
                                     std::vector<PVFitTrack>& fittracks) const {
  StatusCode sc = StatusCode::SUCCESS;

  debug() <<"Now entering fitPV!"<<endreq;

  const size_t nTracks = fittracks.size();
  

  if(nTracks < m_minTrNumber) {
    debug() << "number of track left for the PV "<< nTracks <<endreq;
    return StatusCode::FAILURE;
  }

  debug()<< "old position " << PV->position()  <<endreq;
  debug()<< "old position covariance" << PV->covMatrix() <<endreq;

  bool converged = false;
  double chi2Previous=9999.;
  double chi2Fit=0.;

  int Iter=0;

  const std::vector<PVFitTrack>::iterator iFitTrBegin = fittracks.begin();
  const std::vector<PVFitTrack>::iterator iFitTrEnd = fittracks.end();
  std::vector<PVFitTrack>::iterator iFitTr;

  while(!converged && Iter < m_maxIter) 
  {
    debug() <<" Iteration " << Iter <<endreq;

    const Gaudi::XYZPoint& PosPV = PV->position();

    //expansion point
    const ROOT::Math::SVector<double, 3> Xn0(PosPV.x(),
                                             PosPV.y(),
                                             PosPV.z() );

    const double zref = Xn0[2];

    //fit parameters
    ROOT::Math::SVector<double, 3> Xn(0., 0., 0.);
    Gaudi::SymMatrix3x3 Cn;
    
    chi2Fit=0.;

    Cn(0,0) = 0.;
    Cn(1,1) = 0.;
    Cn(2,2) = 0.;

    iFitTr = iFitTrBegin;
    for( ; iFitTr != iFitTrEnd ; ++iFitTr )
    {
      PVFitTrack& aFitTrack = *iFitTr;
      const LHCb::Track* tr =  aFitTrack.getTrack();

      const double wk = aFitTrack.getWeight();
      debug() <<" weight " << wk << endreq; 

      const LHCb::State& statetr =  tr->firstState();
      LHCb::State newstate =  statetr;

      const bool isVeloOnly = (tr->checkType( Track::Velo )) || (tr->checkType( Track::VeloR ));
      if(isVeloOnly) sc = m_veloExtrapolator->propagate( newstate, zref );
      else sc = m_fullExtrapolator->propagate( newstate, zref );
      if( sc.isFailure ()) {
        debug() << "failed to propagate tarck state in addTr!" << endreq;
        return sc;
      }    

      aFitTrack.setState(newstate);

      //pk is the measurements vector
      ROOT::Math::SVector<double, 4> pk(newstate.x(),
                                        newstate.y(),
                                        newstate.tx(),
                                        newstate.ty() );

      ROOT::Math::SVector<double, 2> qk0 = aFitTrack.getFittedSlopes();
      if(Iter==0) {
         qk0[0] = pk[2];
         qk0[1] = pk[3];
      }

      //Ak, Bk and ck are calculated at the expansion point (Xn0, qk0)
      ROOT::Math::SMatrix<double, 4, 3> Ak;
      Ak(0,0) = 1.;
      Ak(0,1) = 0.;
      Ak(0,2) = -qk0[0];
      Ak(1,0) = 0.;
      Ak(1,1) = 1.;
      Ak(1,2) = -qk0[1];
      Ak(2,0) = 0.;
      Ak(2,1) = 0.;
      Ak(2,2) = 0.;
      Ak(3,0) = 0.;
      Ak(3,1) = 0.;
      Ak(3,2) = 0.;

      ROOT::Math::SMatrix<double, 4, 2> Bk;
      Bk(0,0) = 0.;
      Bk(0,1) = 0.;
      Bk(1,0) = 0.;
      Bk(1,1) = 0.;
      Bk(2,0) = 1.;
      Bk(2,1) = 0.;
      Bk(3,0) = 0.;
      Bk(3,1) = 1.;

      ROOT::Math::SVector<double, 4> ck =  -Ak*Xn0 - Bk*qk0;
      ck[0] += Xn0[0];
      ck[1] += Xn0[1];
      ck[2] += qk0[0];
      ck[3] += qk0[1];

      Gaudi::SymMatrix2x2 Vx = newstate.errPosition().Sub<Gaudi::SymMatrix2x2>(0,0);
      if(!Vx.Invert()) {
        debug() << "could not invert matrix Vx in fitPV! " <<endmsg;
        return StatusCode::FAILURE;
      }

      Gaudi::SymMatrix4x4 GBk;
      GBk.Place_at(Vx,0,0);
      GBk(2,2)=0.;
      GBk(3,3)=0.;

      Cn +=  wk*(ROOT::Math::SimilarityT<double,4,3>(Ak, GBk));
    }

    if(!Cn.Invert()) {
      debug() << "could not invert matrix Cn in fitPV! " <<endmsg;
      return StatusCode::FAILURE;
    }
    PV->setCovMatrix(Cn);

    iFitTr = iFitTrBegin;    
    for ( ; iFitTr != iFitTrEnd; ++iFitTr )
    {
      PVFitTrack& aFitTrack = *iFitTr;

      const double wk = aFitTrack.getWeight();

      const LHCb::State& statetr =   aFitTrack.getState();

      ROOT::Math::SVector<double, 4> pk(statetr.x(),
                                        statetr.y(),
                                        statetr.tx(),
                                        statetr.ty() );

      ROOT::Math::SVector<double, 2> qk0 = aFitTrack.getFittedSlopes();
      if(Iter==0) {
         qk0[0] = pk[2];
         qk0[1] = pk[3];
      }

      ROOT::Math::SMatrix<double, 4, 3> Ak;
      Ak(0,0) = 1.;
      Ak(0,1) = 0.;
      Ak(0,2) = -qk0[0];
      Ak(1,0) = 0.;
      Ak(1,1) = 1.;
      Ak(1,2) = -qk0[1];
      Ak(2,0) = 0.;
      Ak(2,1) = 0.;
      Ak(2,2) = 0.;
      Ak(3,0) = 0.;
      Ak(3,1) = 0.;
      Ak(3,2) = 0.;

      ROOT::Math::SMatrix<double, 4, 2> Bk;
      Bk(0,0) = 0.;
      Bk(0,1) = 0.;
      Bk(1,0) = 0.;
      Bk(1,1) = 0.;
      Bk(2,0) = 1.;
      Bk(2,1) = 0.;
      Bk(3,0) = 0.;
      Bk(3,1) = 1.;

      ROOT::Math::SVector<double, 4> ck =  -Ak*Xn0 - Bk*qk0;
      ck[0] += Xn0[0];
      ck[1] += Xn0[1];
      ck[2] += qk0[0];
      ck[3] += qk0[1];

      Gaudi::SymMatrix2x2 Vx = statetr.errPosition().Sub<Gaudi::SymMatrix2x2>(0,0);
      if(!Vx.Invert()) {
        debug() << "could not invert matrix Vx in fitPV! " <<endmsg;
        return StatusCode::FAILURE;
      }

      Gaudi::SymMatrix4x4 GBk;
      GBk.Place_at(Vx,0,0);
      GBk(2,2)=0.;
      GBk(3,3)=0.;

      Xn +=  Cn*(wk*(ROOT::Math::Transpose(Ak))*GBk*(pk-ck));
    }

    PV->setPosition(Gaudi::XYZPoint(Xn[0], Xn[1], Xn[2]));

    iFitTr = iFitTrBegin;    
    for( ; iFitTr != iFitTrEnd ; ++iFitTr )
    {
      PVFitTrack& aFitTrack = *iFitTr;

      const double wk = aFitTrack.getWeight();

      const LHCb::State& statetr =   aFitTrack.getState();

      ROOT::Math::SVector<double, 4> pk(statetr.x(),
                                        statetr.y(),
                                        statetr.tx(),
                                        statetr.ty() );

      ROOT::Math::SVector<double, 2> qk0 = aFitTrack.getFittedSlopes();
      if(Iter==0) {
         qk0[0] = pk[2];
         qk0[1] = pk[3];
      }

      ROOT::Math::SMatrix<double, 4, 3> Ak;
      Ak(0,0) = 1.;
      Ak(0,1) = 0.;
      Ak(0,2) = -qk0[0];
      Ak(1,0) = 0.;
      Ak(1,1) = 1.;
      Ak(1,2) = -qk0[1];
      Ak(2,0) = 0.;
      Ak(2,1) = 0.;
      Ak(2,2) = 0.;
      Ak(3,0) = 0.;
      Ak(3,1) = 0.;
      Ak(3,2) = 0.;

      ROOT::Math::SMatrix<double, 4, 2> Bk;
      Bk(0,0) = 0.;
      Bk(0,1) = 0.;
      Bk(1,0) = 0.;
      Bk(1,1) = 0.;
      Bk(2,0) = 1.;
      Bk(2,1) = 0.;
      Bk(3,0) = 0.;
      Bk(3,1) = 1.;

      ROOT::Math::SVector<double, 4> ck =  -Ak*Xn0 - Bk*qk0;
      ck[0] += Xn0[0];
      ck[1] += Xn0[1];
      ck[2] += qk0[0];
      ck[3] += qk0[1];

      Gaudi::SymMatrix2x2 Vx = statetr.errPosition().Sub<Gaudi::SymMatrix2x2>(0,0);
      if(!Vx.Invert()) {
        debug() << "could not invert matrix Vx in fitPV! " <<endmsg;
        return StatusCode::FAILURE;
      }

      //      Gaudi::SymMatrix2x2 Vs = statetr.errSlopes().Sub<Gaudi::SymMatrix2x2>(0,0);

      const ROOT::Math::SMatrix<double, 2, 2> Vxt = statetr.covariance().Sub<Gaudi::Matrix2x2>(0,2);     

      const ROOT::Math::SMatrix<double, 2, 2> Vtx = ROOT::Math::Transpose(Vxt);

      ROOT::Math::SMatrix<double, 2, 4> SBG ;
      const ROOT::Math::SMatrix<double, 2, 2> VtxVx = -Vtx*Vx ;
      SBG.Place_at(VtxVx,0,0);
      SBG(0,2) =1.0;
      SBG(1,3) =1.0;

      //qk not explicitly affected by weight
      const ROOT::Math::SVector<double, 2> qk = SBG*(pk-ck-Ak*Xn);

      aFitTrack.setFittedSlopes(qk);

      const ROOT::Math::SVector<double, 4> Delta4 = pk - ck - Ak*Xn - Bk*qk;

      Gaudi::SymMatrix4x4 Gk = statetr.covariance().Sub<Gaudi::SymMatrix4x4>(0,0);
      if(!Gk.Invert()) {
        debug() << "could not invert matrix Gk in fitPV! " <<endmsg;
        return StatusCode::FAILURE;
      }

      double distSq =  ROOT::Math::Dot(Delta4, Gk*Delta4);
      // debug() <<"change of chi2 " << distSq << endreq;
      chi2Fit+= wk*distSq;

      if(m_reweightTracks) {
        const double wknew = getNewWeight(distSq, Iter);
        aFitTrack.setWeight(wknew);   
      }
    }
    debug() <<"chi2Fit " << chi2Fit <<endreq;
 
    if(fabs(chi2Fit-chi2Previous)<m_maxDeltaChi2) {
      converged=true;
    } else {
      chi2Previous=chi2Fit;
    }
    ++Iter;
  }

  debug()<< "new position " << PV->position()  <<endreq;
  debug()<< "new position covariance" << PV->covMatrix() <<endreq;

  
  PV->setChi2(chi2Fit);
  const int nDoF = 2*nTracks-3;
  PV->setNDoF(nDoF);

  return sc;
}


//=============================================================================
// get final tracks of a particle
//=============================================================================
void AdaptivePVReFitter::getFinalTracks(const LHCb::Particle* part,
                                        LHCb::Track::ConstVector& tracks) const
{
  const LHCb::ProtoParticle*   proto  = part->proto() ;

  if( proto ) {
    const LHCb::Track* thetrack = proto->track();
    if(thetrack) tracks.push_back(thetrack);
  } else {
    const SmartRefVector< LHCb::Particle > & Prods = part->daughters();
    SmartRefVector< LHCb::Particle >::const_iterator iProd;
    for (iProd=Prods.begin(); iProd !=Prods.end(); ++iProd){
      const LHCb::Particle* daughter = *iProd;
      getFinalTracks(const_cast<LHCb::Particle*>(daughter), tracks);
    }
  }
}

//=============================================================================
// compute new weight
//=============================================================================

double AdaptivePVReFitter::getNewWeight(double& x, int& i) const
{
  double T;
  const int tsize = m_temperatures.size();
  if(i<tsize) T = m_temperatures[i];
  else T = m_temperatures[tsize-1];

  const double w = 1/(1+exp((x-m_criticalPoint)/2./T));

  return w;
}

