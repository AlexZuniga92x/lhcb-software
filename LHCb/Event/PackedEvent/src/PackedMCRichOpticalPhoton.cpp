// $Id: PackedMCRichOpticalPhoton.cpp,v 1.3 2009-10-22 09:34:19 jonrob Exp $

// local
#include "Event/PackedMCRichOpticalPhoton.h"

// Checks
#include "Event/PackedEventChecks.h"

// Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"

using namespace LHCb;

void MCRichOpticalPhotonPacker::pack( DataVector       & phots,
                                      PackedDataVector & pphots,
                                      const unsigned int version ) const
{
  pphots.data().reserve( phots.size() );
  if ( 0 == version )
  {
    for ( DataVector::const_iterator iD = phots.begin();
          iD != phots.end(); ++iD )
    {
      Data & phot = **iD;

      pphots.data().push_back( PackedData() );
      PackedData & pphot = pphots.data().back();

      pphot.key   = phot.key();

      pphot.hpdx  = m_pack.position( phot.pdIncidencePoint().x() );
      pphot.hpdy  = m_pack.position( phot.pdIncidencePoint().y() );
      pphot.hpdz  = m_pack.position( phot.pdIncidencePoint().z() );

      pphot.pmirx = m_pack.position( phot.sphericalMirrorReflectPoint().x() );
      pphot.pmiry = m_pack.position( phot.sphericalMirrorReflectPoint().y() );
      pphot.pmirz = m_pack.position( phot.sphericalMirrorReflectPoint().z() );

      pphot.smirx = m_pack.position( phot.flatMirrorReflectPoint().x() );
      pphot.smiry = m_pack.position( phot.flatMirrorReflectPoint().y() );
      pphot.smirz = m_pack.position( phot.flatMirrorReflectPoint().z() );

      pphot.aerox = m_pack.position( phot.aerogelExitPoint().x() );
      pphot.aeroy = m_pack.position( phot.aerogelExitPoint().y() );
      pphot.aeroz = m_pack.position( phot.aerogelExitPoint().z() );

      pphot.theta = m_pack.fltPacked( phot.cherenkovTheta() );
      pphot.phi   = m_pack.fltPacked( phot.cherenkovPhi()   );

      pphot.emisx = m_pack.position( phot.emissionPoint().x() );
      pphot.emisy = m_pack.position( phot.emissionPoint().y() );
      pphot.emisz = m_pack.position( phot.emissionPoint().z() );

      pphot.energy = m_pack.energy( phot.energyAtProduction() * PhotEnScale );

      pphot.pmomx  = m_pack.energy( phot.parentMomentum().x() );
      pphot.pmomy  = m_pack.energy( phot.parentMomentum().y() );
      pphot.pmomz  = m_pack.energy( phot.parentMomentum().z() );

      pphot.hpdqwx = m_pack.position( phot.hpdQWIncidencePoint().x() );
      pphot.hpdqwy = m_pack.position( phot.hpdQWIncidencePoint().y() );
      pphot.hpdqwz = m_pack.position( phot.hpdQWIncidencePoint().z() );

      if ( NULL != phot.mcRichHit() )
      {
        pphot.mcrichhit = m_pack.reference( &pphots,
                                            phot.mcRichHit()->parent(),
                                            phot.mcRichHit()->index() );
      }
    }
  }
  else
  {
    std::ostringstream mess;
    mess << "Unknown packed data version " << version;
    throw GaudiException( mess.str(), "MCRichOpticalPhotonPacker", StatusCode::FAILURE );
  }
}

void MCRichOpticalPhotonPacker::unpack( PackedDataVector & pphots,
                                        DataVector       & phots ) const
{
  phots.reserve( pphots.data().size() );
  if ( 0 == pphots.version() )
  {
    for ( PackedDataVector::Vector::const_iterator iD = pphots.data().begin();
          iD != pphots.data().end(); ++iD )
    {
      const PackedData & pphot = *iD;
      Data * phot  = new Data();
      phots.insert( phot, pphot.key );

      phot->setPdIncidencePoint( Gaudi::XYZPoint( m_pack.position(pphot.hpdx),
                                                  m_pack.position(pphot.hpdy),
                                                  m_pack.position(pphot.hpdz) ) );

      phot->setSphericalMirrorReflectPoint( Gaudi::XYZPoint( m_pack.position(pphot.pmirx),
                                                             m_pack.position(pphot.pmiry),
                                                             m_pack.position(pphot.pmirz) ) );

      phot->setFlatMirrorReflectPoint( Gaudi::XYZPoint( m_pack.position(pphot.smirx),
                                                        m_pack.position(pphot.smiry),
                                                        m_pack.position(pphot.smirz) ) );

      phot->setAerogelExitPoint( Gaudi::XYZPoint( m_pack.position(pphot.aerox),
                                                  m_pack.position(pphot.aeroy),
                                                  m_pack.position(pphot.aeroz) ) );

      phot->setCherenkovTheta ( (float)m_pack.fltPacked(pphot.theta) );
      phot->setCherenkovPhi   ( (float)m_pack.fltPacked(pphot.phi)   );

      phot->setEmissionPoint( Gaudi::XYZPoint( m_pack.position(pphot.emisx),
                                               m_pack.position(pphot.emisy),
                                               m_pack.position(pphot.emisz) ) );

      phot->setEnergyAtProduction( (float) ( (double)m_pack.energy(pphot.energy)/PhotEnScale) );

      phot->setParentMomentum( Gaudi::XYZVector( m_pack.energy(pphot.pmomx),
                                                 m_pack.energy(pphot.pmomy),
                                                 m_pack.energy(pphot.pmomz) ) );

      phot->setHpdQWIncidencePoint( Gaudi::XYZPoint( m_pack.position(pphot.hpdqwx),
                                                     m_pack.position(pphot.hpdqwy),
                                                     m_pack.position(pphot.hpdqwz) ) );

      if ( -1 != pphot.mcrichhit )
      {
        int hintID(0), key(0);
        m_pack.hintAndKey( pphot.mcrichhit, &pphots, &phots, hintID, key );
        SmartRef<LHCb::MCRichHit> ref(&phots,hintID,key);
        phot->setMcRichHit( ref );
      }

    }
  }
  else
  {
    std::ostringstream mess;
    mess << "Unknown packed data version " << pphots.version();
    throw GaudiException( mess.str(), "MCRichOpticalPhotonPacker", StatusCode::FAILURE );
  }

}

StatusCode MCRichOpticalPhotonPacker::check( const DataVector & dataA,
                                             const DataVector & dataB,
                                             GaudiAlgorithm & parent ) const
{
  StatusCode sc = StatusCode::SUCCESS;

  // checker
  const DataPacking::DataChecks ch(parent);

  // Loop over data containers together and compare
  DataVector::const_iterator iA(dataA.begin()), iB(dataB.begin());
  for ( ; iA != dataA.end() && iB != dataB.end(); ++iA, ++iB )
  {
    // assume OK frm the start
    bool ok = true;
    // Key
    ok &= (*iA)->key() == (*iB)->key();
    // Hit position
    ok &= ch.comparePoints( "HPD In. Point", (*iA)->pdIncidencePoint(), (*iB)->pdIncidencePoint() );
    // primary mirror point
    ok &= ch.comparePoints( "Prim. Mirr.", (*iA)->sphericalMirrorReflectPoint(),
                            (*iB)->sphericalMirrorReflectPoint() );
    // secondary mirror point
    ok &= ch.comparePoints( "Sec. Mirr.", (*iA)->flatMirrorReflectPoint(),
                            (*iB)->flatMirrorReflectPoint() );
    // aerogel exit point
    ok &= ch.comparePoints( "Aero. Exit", (*iA)->aerogelExitPoint(), (*iB)->aerogelExitPoint() );
    // CK theta and phi
    ok &= ch.compareDoubles( "Cherenkov Theta", (*iA)->cherenkovTheta(), (*iB)->cherenkovTheta() );
    ok &= ch.compareDoubles( "Cherenkov Phi", (*iA)->cherenkovPhi(), (*iB)->cherenkovPhi() );
    // emission point
    ok &= ch.comparePoints( "Emission Point", (*iA)->emissionPoint(), (*iB)->emissionPoint() );
    // energy
    ok &= ch.compareEnergies( "Energy", (*iA)->energyAtProduction(), (*iB)->energyAtProduction(), 1.0e-7 );
    // parent momentum
    ok &= ch.compareEnergies( "Parent Momentum", (*iA)->parentMomentum(), (*iB)->parentMomentum() );
    // HPD QW point
    ok &= ch.comparePoints( "HPD QW Point", (*iA)->hpdQWIncidencePoint(), (*iB)->hpdQWIncidencePoint() );
    // MCRichHit
    ok &= ( (*iA)->mcRichHit() == (*iB)->mcRichHit() );

    // force printout for tests
    //ok = false;
    // If comparison not OK, print full information
    if ( !ok )
    {
      parent.warning() << "Problem with MCRichOpticalPhoton data packing :-" << endmsg
                       << "  Original Photon : " << **iA
                       << endmsg
                       << "  Unpacked Photon : " << **iB
                       << endmsg;
      sc = StatusCode::FAILURE;
    }
  }

  // return final status
  return sc;
}
