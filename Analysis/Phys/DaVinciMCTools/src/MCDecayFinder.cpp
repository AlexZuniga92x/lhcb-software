// $Id: MCDecayFinder.cpp,v 1.1.1.1 2002-05-21 07:32:02 gcorti Exp $
// Include files 
#include <list>
#include <functional>
#include <algorithm>

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/MsgStream.h" 
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"
#include "CLHEP/Units/SystemOfUnits.h"

// from EventSys
#include "Event/MCParticle.h"

// local
#include "MCDecayFinder.h"

//-----------------------------------------------------------------------------
// Implementation file for class : MCDecayFinder
//
// 20/04/2002 : Olivier Dormond
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<MCDecayFinder>          s_factory ;
const        IToolFactory& MCDecayFinderFactory = s_factory ; 


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MCDecayFinder::MCDecayFinder( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : AlgTool ( type, name , parent ),
    m_ppSvc(0), m_source("B0 -> pi+ pi-"), m_decay(0)
{
  if( serviceLocator() ) {
    StatusCode sc = StatusCode::FAILURE;
    sc = serviceLocator()->service("ParticlePropertySvc",m_ppSvc);
  }
  if( !m_ppSvc ) {
    throw GaudiException( "ParticlePropertySvc not found",
                          "MCDecayFinderException",
                          StatusCode::FAILURE );
  }

  declareInterface<IMCDecayFinder>(this);

  declareProperty( "Decay", m_source );
  declareProperty( "ResonnanceThreshold", m_resThreshold = 1e-15*second );
}

//=============================================================================
// Standard destructor
//=============================================================================

MCDecayFinder::~MCDecayFinder( )
{
  if( m_decay )
    delete m_decay;
}

//=============================================================================

StatusCode MCDecayFinder::initialize()
{
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "==> Initializing" << endreq;
  if( compile(m_source) )
  {
    log << MSG::DEBUG << "The compilation of the decay successfull" << endreq;
    return StatusCode::SUCCESS;
  }
  log << MSG::DEBUG << "Could not compile the decay description" << endreq;
  return StatusCode::FAILURE;
}

#include "mclexer.icpp"
#include "mcparser.icpp"

bool MCDecayFinder::compile( std::string &source )
{
  MsgStream log(msgSvc(), name());
  yy_buffer_state *bs = yy_scan_string( source.c_str() );
  try
  {
    if( yparse() )
      throw("Syntax Error");
  }
  catch( DescriptorError e )
  {
    log << MSG::ERROR << "Invalid decay description '"
        << m_source << "'" << endreq;
    log << MSG::ERROR << e.cause() << endreq;
    yy_delete_buffer(bs);
    return false;
  }
  yy_delete_buffer(bs);
  return true;
}

bool MCDecayFinder::hasDecay( const std::vector<MCParticle*> &event )
{
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "About to test the event" << endreq;
  const MCParticle *drop_me = NULL;
  return m_decay->test( event, drop_me );
}

bool MCDecayFinder::findDecay( const std::vector<MCParticle*> &event,
                                   const MCParticle *&previous_result )
{
  MsgStream log( msgSvc(), name() );
  log << MSG::DEBUG << "About to test the event" << endreq;
  return m_decay->test( event, previous_result );
}

MCDecayFinder::Descriptor::Descriptor( IParticlePropertySvc *ppSvc,
                                       double rThre)
  : mother(0), daughters(0), skipResonnance(false),
    elipsis(false), m_resThreshold(rThre), m_ppSvc(ppSvc), alternate(0)
{}

MCDecayFinder::Descriptor::Descriptor( Descriptor &copy )
  : mother(0), daughters(0), skipResonnance(false),
    elipsis(false), m_resThreshold(0), m_ppSvc(0), alternate(0)
{
  if( copy.mother )
    mother = new ParticleMatcher(*copy.mother);
  std::vector<Descriptor *>::iterator d;
  for( d=copy.daughters.begin(); d!=copy.daughters.end(); d++ )
    daughters.push_back(new Descriptor(**d));
  skipResonnance = copy.skipResonnance;
  elipsis  = copy.elipsis;
  m_resThreshold = copy.m_resThreshold;
  m_ppSvc = copy.m_ppSvc;
}

MCDecayFinder::Descriptor::Descriptor( ParticleMatcher *m,
                                       IParticlePropertySvc *ppSvc,
                                       double rThre)
  : mother(m), daughters(0), skipResonnance(false),
    elipsis(false), m_resThreshold(rThre), m_ppSvc(ppSvc), alternate(0)
{}

MCDecayFinder::Descriptor::~Descriptor()
{
  if( mother )
    delete mother;
  std::vector<Descriptor *>::iterator di;
  for( di = daughters.begin(); di != daughters.end(); di++ )
    delete *di;
  if( alternate )
    delete alternate;
}

bool MCDecayFinder::Descriptor::test( const std::vector<MCParticle*> &event,
                                      const MCParticle *&previous_result )
{
  std::vector<MCParticle *>::const_iterator start;
  if( previous_result &&
      ((start=std::find(event.begin(),event.end(),previous_result))
       == event.end()) )
  {
    previous_result = NULL;
    return false; // Bad previous_result
  }
  if( previous_result )
    start++;

  if( mother == NULL ) // No mother == pp collision
  {
    std::list<const MCParticle*> prims;
    std::vector<MCParticle*>::const_iterator i;
    for( i=(previous_result ? start : event.begin()); i != event.end(); i++ )
    {
      MCVertex *origin = (*i)->originVertex();
      if( origin && origin->mother() )
        continue;
      prims.push_back(*i);
    }
    if( skipResonnance )
      filterResonnances( prims );
    if( testDaughters(prims) )
    {
      previous_result = (const MCParticle *)1;
      return true;
    }
    return false;
  }

  std::vector<MCParticle*>::const_iterator part_i;
  part_i = (previous_result ? start : event.begin());
  while( (part_i != event.end()) && (test(*part_i) == false) )
    part_i++;

  if( part_i != event.end() )
  {
    previous_result = *part_i;
    return true;
  }
  return false;
}

bool MCDecayFinder::Descriptor::test( const MCParticle *part )
{
  bool result = false;
  if( mother && mother->test(part) )
  {
    if( daughters.empty() )
      return true; // Nothing to test for the daughters.

    std::list<const MCParticle *> parts;
    SmartRefVector<MCVertex>::const_iterator vi;
    for ( vi = part->endVertices().begin();
          vi != part->endVertices().end(); vi++)
    {
      SmartRefVector<MCParticle>::const_iterator idau;
      for ( idau = (*vi)->products().begin();
            idau != (*vi)->products().end(); idau++ )
      {
        parts.push_back(*idau);
      }
    }
    if( skipResonnance )
      filterResonnances( parts );

    result = testDaughters(parts);
  }
  if( result )
    return true;
  if( alternate )
    return alternate->test(part);
  return false;
}

bool MCDecayFinder::Descriptor::testDaughters( std::list<const MCParticle*>
                                               &parts )
{
  std::vector<Descriptor *>::iterator di;
  for( di = daughters.begin();
       (di != daughters.end()) && !parts.empty(); di++ )
  {
    std::list<const MCParticle *>::iterator p = parts.begin();
    while( p != parts.end() && ((*di)->test(*p) == false) )
      p++;
    if( p == parts.end() )
      return false;   // None of the parts has matched the test
    parts.erase(p);
  }
  if( di != daughters.end() ) // not enough particles
    return false;
  else if( parts.empty() )    // just the right number of particles
    return true;
  else if( elipsis )          // too many particles but we don't care
    return true;
  return false;               // too many particles
}

void MCDecayFinder::Descriptor::addDaughter( Descriptor *daughter )
{
  if( daughter->mother == NULL )
    return; // empty daughter of a {}

  if( daughter->mother->getQmark() )
  {
    daughters.push_back( daughter );
    return;
  }
  else if( daughter->mother->getExact() )
  {
    daughters.insert( daughters.begin(), daughter );
    return;
  }
  std::vector<Descriptor *>::iterator d;
  d=daughters.begin();
  while( d!=daughters.end() && (*d)->mother->getExact() )
    d++;
  daughters.insert( d, daughter );
}

void MCDecayFinder::Descriptor::addNonResonnantDaughters(
                                           std::list<const MCParticle*> &parts,
                                           const MCParticle *part )
{
  SmartRefVector<MCVertex>::const_iterator vi;
  for ( vi = part->endVertices().begin();
        vi != part->endVertices().end(); vi++ )
  {
    SmartRefVector<MCParticle>::const_iterator idau;
    for ( idau = (*vi)->products().begin();
          idau != (*vi)->products().end(); idau++ )
    {
      ParticleProperty *pp = 
        m_ppSvc->findByStdHepID( (*idau)->particleID().pid() );
      if( pp->lifetime() >= m_resThreshold )
        parts.push_front(*idau);
      else
        addNonResonnantDaughters( parts, *idau );
    }
  }
}

void MCDecayFinder::Descriptor::filterResonnances( std::list<const MCParticle*>
                                                   &parts )
{
  std::list<const MCParticle*>::iterator pi;
  std::list<const MCParticle*>::iterator npi;
  for( pi=parts.begin(); pi!=parts.end(); pi = npi )
  {
    ParticleProperty *pp = 
      m_ppSvc->findByStdHepID( (*pi)->particleID().pid() );
    if( pp->lifetime() < m_resThreshold )
    {
      const MCParticle *part = *pi;
      npi = pi;
      npi++;
      parts.erase(pi);
      addNonResonnantDaughters( parts, part ); // Daughters are prepended.
    }
    else
      npi = ++pi;
  }
}

void MCDecayFinder::Descriptor::conjugate( void )
{
  mother->conjugateID();
  std::vector<Descriptor *>::iterator d;
  for( d = daughters.begin(); d != daughters.end(); d++ )
    (*d)->conjugate();
}

MCDecayFinder::ParticleMatcher::ParticleMatcher( IParticlePropertySvc *ppSvc )
  : type(notest), qmark(false), conjugate(false), oscilate(false),
    inverse(false), stable(false), m_ppSvc(ppSvc)
{}

MCDecayFinder::ParticleMatcher::ParticleMatcher( ParticleMatcher &copy )
  : type(notest), qmark(false), conjugate(false), oscilate(false),
    inverse(false), stable(false), m_ppSvc(0)
{
  type = copy.type;
  qmark = copy.qmark;
  conjugate = copy.conjugate;
  oscilate = copy.oscilate;
  inverse = copy.inverse;
  stable = copy.stable;
  m_ppSvc = copy.m_ppSvc;
  switch( type )
  {
  case id:
    parms.stdHepID = copy.parms.stdHepID;
    break;
  case quark:
    parms.quarks.q1 = copy.parms.quarks.q1;
    parms.quarks.q2 = copy.parms.quarks.q2;
    parms.quarks.q3 = copy.parms.quarks.q3;
    break;
  case quantum:
    parms.relation.q = copy.parms.relation.q;
    parms.relation.r = copy.parms.relation.r;
    parms.relation.d = copy.parms.relation.d;
    break;
  case notest:
  default:
    break;
  }
}

MCDecayFinder::ParticleMatcher::ParticleMatcher( char *name,
                                                 IParticlePropertySvc *ppSvc )
  : type(id), qmark(false), conjugate(false), oscilate(false),
    inverse(false), stable(false), m_ppSvc(ppSvc)
{
  ParticleProperty *pp = m_ppSvc->find(name);
  if( pp )
    parms.stdHepID = pp->jetsetID();
  else
    throw DescriptorError(std::string("Unknown particle '")+name+"'");
}

MCDecayFinder::ParticleMatcher::ParticleMatcher(Quarks q1, Quarks q2, Quarks q3,
                                                IParticlePropertySvc *ppSvc )
  : type(quark), qmark(false), conjugate(false), oscilate(false),
    inverse(false), stable(false), m_ppSvc(ppSvc)
{
  parms.quarks.q1 = q1;
  parms.quarks.q2 = q2;
  parms.quarks.q3 = q3;
}

MCDecayFinder::ParticleMatcher::ParticleMatcher(Quantums q,Relations r,double d,
                                                IParticlePropertySvc *ppSvc )
  : type(quantum), qmark(false), conjugate(false), oscilate(false),
    inverse(false), stable(false), m_ppSvc(ppSvc)
{
  parms.relation.q = q;
  parms.relation.r = r;
  parms.relation.d = d;
}

bool MCDecayFinder::ParticleMatcher::test( const MCParticle *part )
{
  switch( type )
  {
  case notest:
    return true;
  case id:
    {
      bool result = false;
      ParticleProperty *pp = 
        m_ppSvc->findByStdHepID( part->particleID().pid() );
      if( !pp )
        return false;
      result = (parms.stdHepID == pp->jetsetID());
      if( conjugate )
      {
        int cc_id = conjugatedID(parms.stdHepID);
        result = result || (cc_id == pp->jetsetID());
      }
      if( oscilate )
        result = result && part->hasOscillated();
      if( inverse )
        result = !result;
      if( stable )
      {
        int n = 0;
        SmartRefVector<MCVertex>::const_iterator vi;
        for ( vi = part->endVertices().begin();
              vi != part->endVertices().end(); vi++)
        {
          n += (*vi)->products().size();
        }
        result = result && (n == 0);
      }
      return result;
    }
  case quark:
    // ******* NOT IMPLEMENTED YES *******
    return false;
  case quantum:
    // ******* NOT IMPLEMENTED YES *******
    return false;
  }
  // Should never come here
  return false;
}

void MCDecayFinder::ParticleMatcher::conjugateID( void )
{
  if( type == id )
    parms.stdHepID = conjugatedID( parms.stdHepID );
  else if( type != notest )
    throw DescriptorError("Charge conjugate only allowed"
                          " on explicit particle or '?'");
}

int MCDecayFinder::ParticleMatcher::conjugatedID( int id )
{
  int cc_id = -id;
  switch( cc_id )
  { // Particles which are their own anti-particle
  case -12:     // nu_e
  case -22:     // gamma
  case -23:     // Z0
  case -25:     // H_10
  case -35:     // H_20
  case -36:     // H_30
  case -111:    // pi0
  case -113:    // rho(770)
  case -115:    // a_2(1320)0
  case -221:    // eta
  case -223:    // omega(782)
  case -331:    // eta'
  case -333:    // phi(1020)
  case -441:    // eta_c(1S)
  case -443:    // J/psi(1S)
  case -445:    // chi_c2(1P)
  case -10441:  // chi_c0(1P)
  case -20113:  // a_1(1260)0
  case -20443:  // chi_c1(1P)
  case -100443: // psi(2S)
  case -9000111:// a_0(980)0
    cc_id *= -1;
    break;
  default:
    break;
  }
  return cc_id;
}
