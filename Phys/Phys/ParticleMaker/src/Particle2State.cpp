// $Id: Particle2State.cpp,v 1.1 2009-04-22 08:53:28 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// LHCb
#include "GaudiKernel/GenericMatrixTypes.h"
#include "LHCbMath/MatrixManip.h"

// local
#include "Particle2State.h"


namespace Gaudi {
  typedef ROOT::Math::SMatrix<double, 2, 3> Matrix2x3;
  typedef ROOT::Math::SMatrix<double, 3, 2> Matrix3x2;
}
//-----------------------------------------------------------------------------
// Implementation file for class : Particle2State
//
// 2006-05-16 : Patrick Koppenburg
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( Particle2State );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
Particle2State::Particle2State( const std::string& type,
                                const std::string& name,
                                const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IParticle2State>(this);

}
//=============================================================================
// Destructor
//=============================================================================
Particle2State::~Particle2State() {} 

//=============================================================================
//=============================================================================
/// Fill Particle from a state
//=============================================================================
StatusCode Particle2State::state2Particle( const LHCb::State& state, 
                                           LHCb::Particle& particle ) const {

  if (0 == particle.charge()) {
    return Error("Neutral particle given as input to state2Particle");
  }
  verbose() << "New particle " << particle.particleID().pid() << endmsg ;

  
  // point on the track and error 
  particle.setReferencePoint( state.position() ) ;
  verbose() << "    Ref Point is " << particle.referencePoint() << endmsg ;
  
  // momentum
  const Gaudi::XYZVector mom = state.momentum();
  const double mass = particle.measuredMass();
  const double e = sqrt( state.p()*state.p()+mass*mass );
  particle.setMomentum(  Gaudi::XYZTVector(mom.X(),mom.Y(),mom.Z(),e) ) ;
  verbose() << "    Momentum is " << particle.momentum() << endmsg ;

  // error 
  Gaudi::Matrix5x5 Jacob = stateJacobian(particle.charge(),mom);
  if (!Jacob.Invert()){
    err() << "Could not invert jacobian \n" << Jacob 
          << "\n for particle " << particle.particleID().pid() << endmsg;
    return StatusCode::FAILURE;
  }

  ROOT::Math::SMatrix<double, 7, 5> Jacob7;
  Jacob7.Place_at(Jacob.Sub<Matrix2x5>( 0, 0 ), 0, 0 );
  Jacob7.Place_at(Jacob.Sub<Matrix3x5>( 2, 0 ), 3, 0 );
  const double q2p = state.qOverP();
  verbose() << "    E=  " << e << " q/P = " << q2p << endmsg ;
  Jacob7(6,4) = -1./e/q2p/q2p/q2p;

  verbose() << "    Jacob 7 is : \n" << Jacob7 << endmsg ;
  verbose() << "    state.covariance() is \n" << state.covariance() << endmsg ;

  const Gaudi::SymMatrix7x7 cov = 
    ROOT::Math::Similarity<double,7,5>(Jacob7, state.covariance() );

  verbose() << "    Gets cov \n" << cov << endmsg ;  

  // error on position
  Gaudi::SymMatrix3x3 errPos;
  errPos.Place_at(cov.Sub<Gaudi::SymMatrix3x3>( 0, 0 ), 0, 0 );
  particle.setPosCovMatrix( errPos  ) ;
  verbose() << "    Position Covariance Matrix is \n" 
            << particle.posCovMatrix() << endmsg ;

  // error on momentum
  Gaudi::SymMatrix4x4 errMom;
  errMom.Place_at(cov.Sub<Gaudi::SymMatrix4x4>( 3, 3 ), 0, 0 );
  particle.setMomCovMatrix(errMom);
  verbose() << "    Momentum Covariance Marix is \n" 
            << particle.momCovMatrix() << endmsg ;

  // correlation
  Gaudi::Matrix4x3 errPosMom;
  errPosMom.Place_at(cov.Sub<Gaudi::Matrix4x3>( 3, 0 ), 0, 0 );
  particle.setPosMomCovMatrix(errPosMom);
  verbose() << "    Position x Momentum Covariance Matrix is \n" 
            << particle.posMomCovMatrix() << endmsg ;
  
  return StatusCode::SUCCESS ;
  
}
//=============================================================================
// make a state from a Particle
//=============================================================================
StatusCode Particle2State::particle2State(const LHCb::Particle& part, 
                                          LHCb::State& s) const { 

  verbose() << "Making a state for " << part.key() << " a " 
            << part.particleID().pid() << endmsg ;
  verbose() << "Particle " << part << "\n  slopes " << part.slopes() 
            << " Q/p " << part.charge()/part.p() << endmsg;

  if (0 == part.charge()) {
    return Error("Neutral particle given as input to particle2State");
  }

  s.setState(part.referencePoint().X(),
             part.referencePoint().Y(),
             part.referencePoint().Z(),
             part.slopes().X(),
             part.slopes().Y(),
             part.charge()/part.p());

  debug() << "Produced state at pos " << s.position() 
          << " mom " << s.momentum() << " slopes " 
          << s.slopes() << " p() " << s.p()<< endmsg ;

  Gaudi::Matrix5x5 cov5Dtmp ;

  verbose() << "Placing \n" << part.covMatrix() << "\n"<< endmsg ;

  // 2x2 for x and y
  cov5Dtmp.Place_at( part.covMatrix().Sub<Gaudi::SymMatrix2x2>( 0, 0 ), 0, 0 );

  // 2x3 for pos and mom
  cov5Dtmp.Place_at( part.covMatrix().Sub<Gaudi::Matrix3x2>( 3, 0 ), 2, 0 );

  // 2x3 for pos and mom
  cov5Dtmp.Place_at( part.covMatrix().Sub<Gaudi::Matrix2x3>( 0, 3 ), 0, 2 );

  // 3x3 for pos
  cov5Dtmp.Place_at( part.covMatrix().Sub<Gaudi::SymMatrix3x3>( 3, 3 ), 2, 2 );

  const Gaudi::SymMatrix5x5 cov5DtmpSym = Gaudi::Math::Symmetrize(cov5Dtmp);  

  verbose() << "Getting \n" << cov5DtmpSym << "\n"<< endmsg ;
  // 5x5 Jacobian
  const Gaudi::Matrix5x5 Jacob = stateJacobian( part.charge(),  
                                                part.momentum().Vect());

  const Gaudi::SymMatrix5x5 cov = 
    ROOT::Math::Similarity<double,5,5>(Jacob, cov5DtmpSym);

  verbose() << "Covariance after similarity \n" << cov << "\n"<< endmsg ;

  s.setCovariance(cov);

  return StatusCode::SUCCESS;
}  
//=============================================================================
/// Get jacobian
//=============================================================================
Gaudi::Matrix5x5 Particle2State::stateJacobian(int charge, 
                                               const Gaudi::XYZVector& Momentum) const 
{
  verbose() << "Making Jacobian with " << charge << " " << Momentum << endmsg ;
  Gaudi::Matrix5x5 Jacob ;
  // the variables
  const double Px = Momentum.X() ;
  const double Py = Momentum.Y() ;
  const double Pz = Momentum.Z() ;
  const double P  = Momentum.R() ;

  //  const double Tx = Momentum.X()/Momentum.Z();   // Tx = Px/Pz
  //  const double Ty = Momentum.Y()/Momentum.Z();   // Ty = Py/Pz
  // const double o  = part->charge()/part->p();                    // o = charge/sqrt(Px^2+Py^2+Pz^2)

  // unit matrix in the top left corner
  Jacob(0,0) = 1. ;                     //  dx/dy
  Jacob(1,1) = 1. ;                     //  dy/dy
  // non unit matrix in the bottom right corner
  Jacob(2,2) = 1/Pz ;                   //  dTx/dPx = 1/Pz
  Jacob(2,3) = 0. ;                     //  dTx/dPy = 0.
  Jacob(2,4) = -Px/(Pz*Pz) ;            //  dTx/dPz = -Px/Pz^2
  Jacob(3,2) = 0. ;                     //  dTy/dPx = 0.
  Jacob(3,3) = 1/Pz ;                   //  dTy/dPy = 1/Pz
  Jacob(3,4) = -Py/(Pz*Pz) ;            //  dTy/dPz = -Py/Pz^2
  Jacob(4,2) = -charge*Px/(P*P*P) ; //  do/dPx  = -charge*Px/P^3
  Jacob(4,3) = -charge*Py/(P*P*P) ; //  do/dPy  = -charge*Py/P^3
  Jacob(4,4) = -charge*Pz/(P*P*P) ; //  do/dPz  = -charge*Pz/P^3
  
  verbose() << "Returning Jacobian\n" << Jacob << endmsg ;
  return Jacob;
      
}
//=============================================================================
/// Test the tool
//=============================================================================
StatusCode Particle2State::test(const LHCb::Particle& p ) const {
  info() << "Testing with Particle \n" << p << "\n" << endmsg;
  LHCb::State s ;
  StatusCode sc = particle2State( p, s );
  if (!sc) return sc;
  info() << "Got State \n" << s << "\n" << endmsg;
  verbose() << "This does not work: \n" << s.posMomCovariance() << "\n"<< endmsg ;
  LHCb::Particle newPart = LHCb::Particle(p);
  sc = state2Particle( s, newPart );
  if (!sc) return sc;
  info() << "And Particle \n" << newPart << "\n" << endmsg;
  return sc;
}

//=============================================================================
/// Test the tool
//=============================================================================
StatusCode Particle2State::test(const LHCb::State& s ) const {
  info() << "Testing with State \n" << s << "\n" << endmsg;
  verbose() << "This does not work: \n" << s.posMomCovariance() << "\n"<< endmsg ;
  int pid = 211 ;
  if (s.qOverP()<0.) pid=-pid;
  LHCb::Particle p = LHCb::Particle(LHCb::ParticleID(pid)) ;
  StatusCode sc = state2Particle( s, p );
  if (!sc) return sc;
  info() << "Got Particle \n" << p << "\n" << endmsg;
  LHCb::State newState = LHCb::State(s);
  sc = particle2State( p, newState );
  if (!sc) return sc;
  info() << "And State \n" << newState << "\n" << endmsg;
  return sc;
}
