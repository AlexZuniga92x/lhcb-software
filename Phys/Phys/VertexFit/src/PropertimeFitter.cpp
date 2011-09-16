// $Id: PropertimeFitter.cpp,v 1.8 2008-03-27 11:04:18 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 
#include "GaudiKernel/PhysicalConstants.h"

#include "Event/Vertex.h"
#include "Event/RecVertex.h"
#include "Event/Particle.h"

#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"

// local
#include "PropertimeFitter.h"

using namespace LHCb ;
using namespace Gaudi::Units;
//-----------------------------------------------------------------------------
// Implementation file for class : PropertimeFitter
//
// 2006-06-07 : Yuehong Xie
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory

DECLARE_TOOL_FACTORY( PropertimeFitter )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PropertimeFitter::PropertimeFitter( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<ILifetimeFitter>(this);

  declareProperty( "applyBMassConstraint", m_applyBMassConstraint = false);
  declareProperty( "maxIter", m_maxIter = 10);
  declareProperty( "MaxDeltaChi2", m_maxDeltaChi2 = 0.001);

}

//=============================================================================
// Destructor
//=============================================================================
PropertimeFitter::~PropertimeFitter() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode PropertimeFitter::initialize(){
  StatusCode sc = GaudiTool::initialize();
  if (!sc) return sc;

  m_ppSvc = svc<LHCb::IParticlePropertySvc>("LHCb::ParticlePropertySvc");

  return sc;
}

//=============================================================================
// Get propertime
// inputs: Vertex corresponding to the assumed production point
//         LHCb::Particle itself
// output: resulting propertime and error, chisq.
//=============================================================================
StatusCode PropertimeFitter::fit( const LHCb::VertexBase& PV, const LHCb::Particle& B,
                                  double& propertime, double& error,
                                  double& chisq) const 
{

  Gaudi::XYZPoint PVposition = PV.position();
  Gaudi::SymMatrix3x3 CovPV = PV.covMatrix();

  Gaudi::XYZPoint BPosition= B.referencePoint();
  Gaudi::LorentzVector BLmom= B.momentum();

  double BMeasuredMass = sqrt(BLmom.T()*BLmom.T()-BLmom.X()*BLmom.X()-BLmom.Y()*BLmom.Y()-BLmom.Z()*BLmom.Z());

  Gaudi::SymMatrix7x7 CovB = B.covMatrix(); 

  Gaudi::Matrix7x7 Te2m = ROOT::Math::SMatrixIdentity();
  Te2m(6,3) = -BLmom.X()/BMeasuredMass;
  Te2m(6,4) = -BLmom.Y()/BMeasuredMass;
  Te2m(6,5) = -BLmom.Z()/BMeasuredMass;
  Te2m(6,6) =  BLmom.T()/BMeasuredMass;

  Gaudi::SymMatrix7x7 CovBm = ROOT::Math::Similarity<double,7,7>(Te2m, CovB);

  ROOT::Math::SVector<double, 10> X;
  X[0] = PVposition.x();
  X[1] = PVposition.y();
  X[2] = PVposition.z();
  X[3] = BPosition.x();
  X[4] = BPosition.y();
  X[5] = BPosition.z();
  X[6] = BLmom.X();
  X[7] = BLmom.y();
  X[8] = BLmom.z();
  X[9] = BMeasuredMass;

  SymMatrix10x10 Cx;

  for(int l1=0;l1<=2; l1++) 
    for(int l2=0;l2<=l1; l2++) Cx(l1,l2) = CovPV(l1,l2);

  for(int l1=0;l1<=6; l1++)
    for(int l2=0;l2<=l1; l2++) Cx(l1+3,l2+3) = CovBm(l1,l2);

  for(int l1=3;l1<=9; l1++)
    for(int l2=0;l2<=2; l2++) Cx(l1,l2) = 0.0;
  
  // xpv, ypv, zpv, xb, yb, zb, pxb, pyb, pzb, mb 
  ROOT::Math::SVector<double, 10> vfit=X;

  SymMatrix10x10 cfit = Cx;

  bool converged=false;
  int iter=0;

  double chi2PreviousFit=9999.;
  double chi2Fit=999.;

  while(!converged && iter< m_maxIter)  {
    iter++;
    verbose() << ":-) Iteration   " << iter << endreq;

    //f(0)=(xb-xpv)*pzb-(zb-zpv)*pxb
    //f(1)=(yb-ypv)*pzb-(zb-zpv)*pyb
    
    ROOT::Math::SVector<double, 2> f;
    f(0)= (vfit[3]-vfit[0])*vfit[8] - (vfit[5]-vfit[2])*vfit[6];
    f(1)= (vfit[4]-vfit[1])*vfit[8] - (vfit[5]-vfit[2])*vfit[7];

    //D is the derivative matrix
    ROOT::Math::SMatrix<double, 2, 10> D;

    D(0,0) = - vfit[8];
    D(0,1) = 0.0;
    D(0,2) = vfit[6];
    D(0,3) = vfit[8];
    D(0,4) = 0.0;
    D(0,5) = -vfit[6];
    D(0,6) = -(vfit[5]-vfit[2]);
    D(0,7) = 0.0;
    D(0,8) = (vfit[3]-vfit[0]);
    D(0,9) = 0.0;

    D(1,0) = 0.0;
    D(1,1) = -vfit[8];
    D(1,2) = vfit[7];
    D(1,3) = 0.0;
    D(1,4) = vfit[8];
    D(1,5) = -vfit[7];
    D(1,6) = 0.0;
    D(1,7) = -(vfit[5]-vfit[2]);
    D(1,8) = (vfit[4]-vfit[1]);
    D(1,9) = 0.0;

    ROOT::Math::SVector<double, 2> d = f - D*vfit;

    Gaudi::SymMatrix2x2 VD=ROOT::Math::Similarity<double,2,10>(D, Cx);
    if(!VD.Invert()) {
      debug() << "could not invert matrix VD in fit! " <<endreq;
      return StatusCode::FAILURE;
    }

    ROOT::Math::SVector<double, 2> alpha=D*X+d;
                                                                                                                               
    ROOT::Math::SVector<double, 2> lambda=VD*alpha;
                                                                                                                               
    ROOT::Math::SMatrix<double, 10,2> DT = ROOT::Math::Transpose(D);
                                                                                                                               
    vfit=X-Cx*DT*lambda;

    SymMatrix10x10 delataC1=ROOT::Math::Similarity<double,10,2>(DT, VD);
                                                                                                                               
    SymMatrix10x10 delataC2=ROOT::Math::Similarity<double,10,10>(Cx, delataC1);
                                                                                                                               
    cfit=Cx -delataC2;
                                                                                                                               
    chi2Fit=ROOT::Math::Dot(alpha,lambda);
    //chi2Fit+= 2*ROOT::Math::Dot(lambda,f);

    if(fabs(chi2Fit-chi2PreviousFit)<m_maxDeltaChi2) {
      converged=true;
    } else {
      chi2PreviousFit=chi2Fit;
    } 
  }  // end chi2 minimization iteration

  if(!converged)  return StatusCode::FAILURE;

  if(m_applyBMassConstraint) {
    //int Bpid = B.particleID().pid();
    //ParticleProperty*  partProp = m_ppSvc->findByStdHepID(Bpid  );
    const LHCb::ParticleProperty*  partProp = m_ppSvc->find(B.particleID());
    double nominalBMass = partProp->mass();
    ROOT::Math::SMatrix<double,1,10> DD;
    Gaudi::Vector1 dd;
    DD(0,9)=1.;
    dd[0]=-nominalBMass;
    Gaudi::SymMatrix1x1 Cd = ROOT::Math::Similarity<double, 1, 10>(DD,cfit);
    if ( !Cd.Invert() ) {
      debug() << "could not invert matrix Cd in fit" << endmsg;
      return StatusCode::FAILURE;
    }
    vfit-= cfit*ROOT::Math::Transpose(DD)*Cd*(DD*vfit+dd);
    SymMatrix10x10 deltacfit1=ROOT::Math::SimilarityT<double,1,10>(DD,Cd);
    SymMatrix10x10 deltacfit2=ROOT::Math::Similarity<double,10,10>(cfit,deltacfit1);
    cfit -= deltacfit2;

    for(int i=0;i<=9;i++) cfit(9,i)=0.0;

  }

  //propertime = (xb-xpv)*mb/pxb ;
  propertime =  (vfit[3]-vfit[0])*vfit[9]/vfit[6];  

  ROOT::Math::SMatrix<double, 1, 10> JA;
  JA(0,0) = -vfit[9]/vfit[6];
  JA(0,1) = 0.0;
  JA(0,2) = 0.0; 
  JA(0,3) = vfit[9]/vfit[6];
  JA(0,4) = 0.0;
  JA(0,5) = 0.0;
  JA(0,6) = -(vfit[3]-vfit[0])*vfit[9]/vfit[6]/vfit[6];
  JA(0,7) = 0.0;
  JA(0,8) = 0.0;
  JA(0,9) = (vfit[3]-vfit[0])/vfit[6];

  Gaudi::SymMatrix1x1 CovTau = ROOT::Math::Similarity<double,1,10>(JA, cfit);

  error = sqrt(CovTau(0,0));

  // convert c*tau to tau
  propertime /=  c_light;   
  error /= c_light;

  chisq =  chi2Fit;

  return StatusCode::SUCCESS ;
}
