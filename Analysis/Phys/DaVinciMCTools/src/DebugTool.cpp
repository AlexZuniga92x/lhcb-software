// $Id: DebugTool.cpp,v 1.13 2007-01-12 14:03:52 ranjard Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ParticleProperty.h"

// from Event
#include "Event/MCParticle.h"
#include "Event/MCVertex.h"
#include "Event/Particle.h"

// from LHCb
#include "GaudiKernel/PhysicalConstants.h"

// local
#include "DebugTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DebugTool
//
// 29/03/2001 : Olivier Dormond
//-----------------------------------------------------------------------------

// Declaration of the AlgTool Factory

DECLARE_TOOL_FACTORY( DebugTool );

using namespace Gaudi::Units;

//=============================================================================
// Standard creator, initializes variables
//=============================================================================
DebugTool::DebugTool( const std::string& type,
                      const std::string& name,
                      const IInterface* parent )
  : GaudiTool( type, name, parent ), m_ppSvc(0), m_keys(0), 
    m_energyUnitName("MeV"), m_lengthUnitName("mm")
{

  declareInterface<IDebugTool>(this);

  declareProperty( "PrintDepth", m_depth = 999 );
  declareProperty( "TreeWidth", m_treeWidth = 20 );
  declareProperty( "FieldWidth", m_fWidth = 10 );
  declareProperty( "FieldPrecision", m_fPrecision = 2 );
  declareProperty( "Arrow", m_arrow = "+-->" );
  declareProperty( "Informations", m_informationsDeprecated = "" );
  declareProperty( "Information", m_information = "Name E M P Pt phi Vz" );
  declareProperty( "EnergyUnit", m_energyUnit = MeV );
  declareProperty( "LengthUnit", m_lengthUnit = mm );
}

//=============================================================================
// initialise
//=============================================================================
StatusCode DebugTool::initialize( void ){
  StatusCode sc = GaudiTool::initialize();
  if (!sc) return sc;

  if( serviceLocator() ) {
    sc = service("ParticlePropertySvc",m_ppSvc);
  }
  if( !m_ppSvc ) {
    throw GaudiException( "ParticlePropertySvc not found",
                          "DebugException",
                          StatusCode::FAILURE );
  }

  if ( m_informationsDeprecated != "" ){
    warning() << "You are using the deprecated option ``informations''." << endmsg ;
    warning() << "Use ``Information'' instead." << endmsg ;
    m_information = m_informationsDeprecated ;
  }
  

  std::size_t oldpos = 0, pos;
  do {
    pos=m_information.find( ' ', oldpos );
    std::string tok(m_information, oldpos, pos-oldpos);
    if( tok=="Name" )       m_keys.push_back(Name);
    else if( tok=="E" )     m_keys.push_back(E);
    else if( tok=="M" )     m_keys.push_back(M);
    else if( tok=="P" )     m_keys.push_back(P);
    else if( tok=="Pt" )    m_keys.push_back(Pt);
    else if( tok=="Px" )    m_keys.push_back(Px);
    else if( tok=="Py" )    m_keys.push_back(Py);
    else if( tok=="Pz" )    m_keys.push_back(Pz);
    else if( tok=="Vx" )    m_keys.push_back(Vx);
    else if( tok=="Vy" )    m_keys.push_back(Vy);
    else if( tok=="Vz" )    m_keys.push_back(Vz);
    else if( tok=="theta" ) m_keys.push_back(theta);
    else if( tok=="phi" )   m_keys.push_back(phi);
    else if( tok=="eta" )   m_keys.push_back(eta);
    else if( tok=="IDCL" )  m_keys.push_back(idcl);
    else
      err() << "Unknown output key '" << tok << "'. Ignoring it."
          << endreq;
    if( pos != std::string::npos ) oldpos = pos+1;
    else                           oldpos = pos;
  }
  while( pos != std::string::npos );

  if (m_energyUnit == TeV) m_energyUnitName = "TeV" ;
  else if (m_energyUnit == GeV) m_energyUnitName = "GeV" ;
  else if (m_energyUnit == MeV) m_energyUnitName = "MeV" ;
  else if (m_energyUnit <= 0) {
    err() << "You have chosen a unit for energies: " 
        << m_energyUnit << endreq;
    return StatusCode::FAILURE ;
  } 
  else {
    warning() << "You have chosen a non-standard unit for energies: " 
        << m_energyUnit << endreq;
    m_energyUnitName = "???" ;
  }
  if (m_lengthUnit == mm) m_lengthUnitName = "mm" ;
  else if (m_lengthUnit == cm) m_lengthUnitName = "cm" ;
  else if (m_lengthUnit == m) m_lengthUnitName = "m" ;
  else if (m_lengthUnit <= 0) {
    err() << "You have chosen a unit for lengths: " 
        << m_lengthUnit << endreq;
    return StatusCode::FAILURE ;
  } 
  else {
    warning() << "You have chosen a non-standard unit for lengths: " 
        << m_lengthUnit << endreq;
    m_lengthUnitName = "??" ;
    
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// printHeader
//=============================================================================
void DebugTool::printHeader( MsgStream &log, 
                             bool mcfirst, 
                             bool associated )
{
  static const std::string mctitle = " MCParticle ";
  static const std::string title = " Particle ";
  const std::string *title1, *title2;
  if( mcfirst ) {
    title1 = &mctitle;
    title2 = &title;
  } else {
    title1 = &title;
    title2 = &mctitle;
  }

  bool name_key = false;
  std::vector<InfoKeys>::iterator i;
  for( i = m_keys.begin(); i!= m_keys.end(); i++ )
    if( *i == Name )
      name_key = true;
  int n_keys = m_keys.size() - (name_key ? 1 : 0);
  int width1 = n_keys*m_fWidth + (name_key ? m_treeWidth : 0);
  int width2 = n_keys*m_fWidth + (name_key ? m_fWidth : 0);
  int left1  = (width1-title1->length()-2)/2;
  int right1 =  width1-title1->length()-2 - left1;
  int left2  = (width2-title2->length()-2)/2;
  int right2 =  width2-title2->length()-2 - left2;

  if( left1 >= 0 )
    log << '<' << std::string(left1,'-');
  log << *title1;
  if( right1 >= 0 )
    log<< std::string(right1,'-') << '>';

  if( associated ) {
    if( left2 >= 0 )
      log << '<' << std::string(left2,'-');
    log << *title2;
    if( right2 >= 0 )
      log<< std::string(right2,'-') << '>';
  }
  log << std::endl;

  for( i = m_keys.begin(); i!= m_keys.end(); i++ )
    switch( *i ) {
    case Name:      log << std::setw(m_treeWidth) << "Name";   break;
    case E:         log << std::setw(m_fWidth) << "E";         break;
    case M:         log << std::setw(m_fWidth) << "M";         break;
    case P:         log << std::setw(m_fWidth) << "P";         break;
    case Pt:        log << std::setw(m_fWidth) << "Pt";        break;
    case Px:        log << std::setw(m_fWidth) << "Px";        break;
    case Py:        log << std::setw(m_fWidth) << "Py";        break;
    case Pz:        log << std::setw(m_fWidth) << "Pz";        break;
    case Vx:        log << std::setw(m_fWidth) << "Vx";        break;
    case Vy:        log << std::setw(m_fWidth) << "Vy";        break;
    case Vz:        log << std::setw(m_fWidth) << "Vz";        break;
    case theta:     log << std::setw(m_fWidth) << "theta";     break;
    case phi:       log << std::setw(m_fWidth) << "phi";       break;
    case eta:       log << std::setw(m_fWidth) << "eta";       break;
    case idcl:      log << std::setw(m_fWidth) << "ID CL";     break;
    }
  if( associated )
    for( i = m_keys.begin(); i!= m_keys.end(); i++ )
      switch( *i ) {
      case Name:    log << std::setw(m_fWidth) << "Name";      break;
      case E:       log << std::setw(m_fWidth) << "E";         break;
      case M:       log << std::setw(m_fWidth) << "M";         break;
      case P:       log << std::setw(m_fWidth) << "P";         break;
      case Pt:      log << std::setw(m_fWidth) << "Pt";        break;
      case Px:      log << std::setw(m_fWidth) << "Px";        break;
      case Py:      log << std::setw(m_fWidth) << "Py";        break;
      case Pz:      log << std::setw(m_fWidth) << "Pz";        break;
      case Vx:      log << std::setw(m_fWidth) << "Vx";        break;
      case Vy:      log << std::setw(m_fWidth) << "Vy";        break;
      case Vz:      log << std::setw(m_fWidth) << "Vz";        break;
      case theta:   log << std::setw(m_fWidth) << "theta";     break;
      case phi:     log << std::setw(m_fWidth) << "phi";       break;
      case eta:     log << std::setw(m_fWidth) << "eta";       break;
      case idcl:    log << std::setw(m_fWidth) << "ID CL";     break;
      }
  log << std::endl;

  for( i = m_keys.begin(); i!= m_keys.end(); i++ )
    switch( *i ) {
    case Name:      log << std::setw(m_treeWidth) << " ";      break;
    case E:         log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case M:         log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case P:         log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case Pt:        log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case Px:        log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case Py:        log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case Pz:        log << std::setw(m_fWidth) << m_energyUnitName;       break;
    case Vx:        log << std::setw(m_fWidth) << m_lengthUnitName;        break;
    case Vy:        log << std::setw(m_fWidth) << m_lengthUnitName;        break;
    case Vz:        log << std::setw(m_fWidth) << m_lengthUnitName;        break;
    case theta:     log << std::setw(m_fWidth) << "mrad";      break;
    case phi:       log << std::setw(m_fWidth) << "mrad";      break;
    case eta:       log << std::setw(m_fWidth) << "prap";      break;
    case idcl:      log << std::setw(m_fWidth) << " ";         break;
    }
  if( associated )
    for( i = m_keys.begin(); i!= m_keys.end(); i++ )
      switch( *i ) {
      case Name:    log << std::setw(m_fWidth) << " ";         break;
      case E:       log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case M:       log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case P:       log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case Pt:      log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case Px:      log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case Py:      log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case Pz:      log << std::setw(m_fWidth) << m_energyUnitName;       break;
      case Vx:      log << std::setw(m_fWidth) << m_lengthUnitName;        break;
      case Vy:      log << std::setw(m_fWidth) << m_lengthUnitName;        break;
      case Vz:      log << std::setw(m_fWidth) << m_lengthUnitName;        break;
      case theta:   log << std::setw(m_fWidth) << "mrad";      break;
      case phi:     log << std::setw(m_fWidth) << "mrad";      break;
      case eta:     log << std::setw(m_fWidth) << "prap";      break;
      case idcl:    log << std::setw(m_fWidth) << " ";         break;
      }
  log << std::endl;
}

//=============================================================================
// printInfo (MCParticle)
//=============================================================================
void DebugTool::printInfo( const std::string &prefix, 
                           const LHCb::MCParticle *part,
                           Particle2MCLinker* assoc, MsgStream &log )
{
  ParticleProperty* p = m_ppSvc->findByStdHepID( part->particleID().pid() );
  const LHCb::MCVertex *origin = part->originVertex();

  std::vector<InfoKeys>::iterator i;
  for( i = m_keys.begin(); i!= m_keys.end(); i++ )
    switch( *i ) {
    case Name:
      {
        std::string p_name = p ? p->particle() : "N/A";
        int p_len = p_name.length();
        if( prefix.length() == 0 ) {
          if( p_len > m_treeWidth )
            p_len = m_treeWidth - 1;
          log << p_name << std::string( m_treeWidth - p_len - 1,' ' );
        } else {
          if( p_len > (int)(m_treeWidth-prefix.length()-m_arrow.length()) )
            p_len = m_treeWidth - prefix.length() - m_arrow.length() - 1;
          log << prefix.substr(0, prefix.length()-1) << m_arrow << p_name
              << std::string( m_treeWidth - prefix.length()
                              - m_arrow.length() - p_len, ' ' );
        }
      }
      break;
    case E:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().e()/m_energyUnit;
      break;
    case M:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().M()/m_energyUnit;
      break;
    case P:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().Vect().R()/m_energyUnit;
      break;
    case Pt:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().Pt()/m_energyUnit;
      break;
    case Px:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().px()/m_energyUnit;
      break;
    case Py:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().py()/m_energyUnit;
      break;
    case Pz:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().pz()/m_energyUnit;
      break;
    case Vx:
      if( origin )
        log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
            << origin->position().x()/m_lengthUnit;
      else
        log << std::setw(m_fWidth) << " N/A ";
      break;
    case Vy:
      if( origin )
        log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
            << origin->position().y()/m_lengthUnit;
      else
        log << std::setw(m_fWidth) << " N/A ";
      break;
    case Vz:
      if( origin )
        log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
            << origin->position().z()/m_lengthUnit;
      else
        log << std::setw(m_fWidth) << " N/A ";
      break;
    case theta:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().theta()/mrad;
      break;
    case phi:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().phi()/mrad;
      break;
    case eta:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << part->momentum().Eta();
      break;
    case idcl:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << 1.0;
      break;
    default:
      break;
    }

  if( assoc ) {
    const LHCb::Particle* reco = assoc->firstP( part );
    if( reco )
      for( i = m_keys.begin(); i!= m_keys.end(); i++ )
        switch( *i ) {
        case Name:
          {
            p = m_ppSvc->findByStdHepID( reco->particleID().pid() );
            std::string p_name = p ? p->particle() : "N/A";
            int p_len = p_name.length();
            if( p_len > m_fWidth-1 )
              p_len = m_fWidth-2;
            log << ' ' << p_name
                << std::string(m_fWidth-1-p_len, ' ');
          }
          break;
        case E:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().e()/m_energyUnit;
          break;
        case M:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().M()/m_energyUnit;
          break;
        case P:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().Vect().R()/m_energyUnit;
          break;
        case Pt:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().Pt()/m_energyUnit;
          break;
        case Px:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().px()/m_energyUnit;
          break;
        case Py:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().py()/m_energyUnit;
          break;
        case Pz:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().pz()/m_energyUnit;
          break;
        case Vx:
          if( origin )
            log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                << reco->referencePoint().x()/m_lengthUnit;
          else
            log << std::setw(m_fWidth) << " N/A ";
          break;
        case Vy:
          if( origin )
            log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                << reco->referencePoint().y()/m_lengthUnit;
          else
            log << std::setw(m_fWidth) << " N/A ";
          break;
        case Vz:
          if( origin )
            log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                << reco->referencePoint().z()/m_lengthUnit;
          else
            log << std::setw(m_fWidth) << " N/A ";
          break;
        case theta:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().theta()/mrad;
          break;
        case phi:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().phi()/mrad;
          break;
        case eta:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->momentum().Eta();
          break;
        case idcl:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << reco->confLevel();
          break;
        default:
          break;          
        }
    else 
      log << "  No associated particle";
  }
  log << std::endl;
}

//=============================================================================
// printInfo
//=============================================================================
void DebugTool::printInfo( const std::string &prefix, 
                           const LHCb::Particle *reco,
                           Particle2MCLinker* assoc, MsgStream &log )
{
  ParticleProperty* p = m_ppSvc->findByStdHepID( reco->particleID().pid() );
  std::vector<InfoKeys>::iterator i;
  for( i = m_keys.begin(); i!= m_keys.end(); i++ )
    switch( *i ) {
    case Name:
      {
        std::string p_name = p ? p->particle() : "N/A";
        int p_len = p_name.length();
        if( prefix.length() == 0 ) {
          if( p_len > m_treeWidth )
            p_len = m_treeWidth - 1;
          log << p_name << std::string( m_treeWidth - p_len - 1,' ' );
        } else {
          if( p_len > (int)(m_treeWidth-prefix.length()-m_arrow.length()) )
            p_len = m_treeWidth - prefix.length() - m_arrow.length() - 1;
          log << prefix.substr(0, prefix.length()-1) << m_arrow << p_name
              << std::string( m_treeWidth - prefix.length() - m_arrow.length()
                              - p_len, ' ' );
        }
      }
      break;
    case E:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().e()/m_energyUnit;
      break;
    case M:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().M()/m_energyUnit;
      break;
    case P:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().Vect().R()/m_energyUnit;
      break;
    case Pt:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().Pt()/m_energyUnit;
      break;
    case Px:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().px()/m_energyUnit;
      break;
    case Py:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().py()/m_energyUnit;
      break;
    case Pz:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().pz()/m_energyUnit;
      break;
    case Vx:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->referencePoint().x()/m_lengthUnit;
      break;
    case Vy:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->referencePoint().y()/m_lengthUnit;
      break;
    case Vz:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->referencePoint().z()/m_lengthUnit;
      break;
    case theta:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().theta()/mrad;
      break;
    case phi:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().phi()/mrad;
      break;
    case eta:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->momentum().Eta();
      break;
    case idcl:
      log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
          << reco->confLevel();
      break;
    default:
      break;
    }

  if( assoc ) {
    const LHCb::MCParticle* part = assoc->firstMCP( reco );
    if( part )
      for( i = m_keys.begin(); i!= m_keys.end(); i++ )
        switch( *i ) {
        case Name:
          {
            p = m_ppSvc->findByStdHepID( reco->particleID().pid() );
            std::string p_name = p ? p->particle() : "N/A";
            int p_len = p_name.length();
            if( p_len > m_fWidth-1 )
              p_len = m_fWidth-2;
            log << ' ' << p_name
                << std::string(m_fWidth-1-p_len, ' ');
          }
          break;
        case E:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().e()/m_energyUnit;
          break;
        case M:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().M()/m_energyUnit;
          break;
        case P:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().Vect().R()/m_energyUnit;
          break;
        case Pt:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().Pt()/m_energyUnit;
          break;
        case Px:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().px()/m_energyUnit;
          break;
        case Py:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().py()/m_energyUnit;
          break;
        case Pz:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().pz()/m_energyUnit;
          break;
        case Vx:
          {
            const LHCb::MCVertex *origin = part->originVertex();
            if( origin )
              log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                  << origin->position().x()/m_lengthUnit;
            else
              log << std::setw(m_fWidth) << " N/A ";
          }
          break;
        case Vy:
          {
            const LHCb::MCVertex *origin = part->originVertex();
            if( origin )
              if( origin )
                log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                    << origin->position().y()/m_lengthUnit;
              else
                log << std::setw(m_fWidth) << " N/A ";
          }
          break;
        case Vz:
          {
            const LHCb::MCVertex *origin = part->originVertex();
            if( origin )
              if( origin )
                log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
                    << origin->position().z()/m_lengthUnit;
              else
                log << std::setw(m_fWidth) << " N/A ";
          }
          break;
        case theta:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().theta()/mrad;
          break;
        case phi:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().phi()/mrad;
          break;
        case eta:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << part->momentum().Eta();
          break;
        case idcl:
          log << std::setw(m_fWidth) << std::setprecision(m_fPrecision)
              << 1.0;
          break;
        default:
          break;
        }
    else
      log << "  No associated particle";
  }
  log << std::endl;  
}

//=============================================================================
// Print decay tree for a given MCparticle
//=============================================================================
void DebugTool::printTree( const LHCb::MCParticle *mother, int maxDepth = -1 )
{
  printTree( mother, NULL, maxDepth );
}

//=============================================================================
// Print decay tree for a given MCparticle
//=============================================================================
void DebugTool::printTree( const LHCb::MCParticle *mother, 
                           Particle2MCLinker* assoc,
                           int maxDepth = -1 )
{
  if( maxDepth == -1 )
    maxDepth = m_depth;
  
  MsgStream log(msgSvc(), name());

  if( !mother ) {
    err() << "printTree called with NULL MCParticle" << endreq;
    return;
  }

  log << MSG::INFO << std::endl;
  printHeader( log, true, assoc != NULL );

  log.setf(std::ios::fixed,std::ios::floatfield);
  printDecayTree( mother, assoc, "", maxDepth, log );
  log << endreq;
}

//=============================================================================
// printEventAsTree (MCParticle)
//=============================================================================
void DebugTool::printEventAsTree( const LHCb::MCParticle::ConstVector &event )
{
  printEventAsTree( event, NULL );
}

//=============================================================================
// printEventAsTree (MCParticle)
//=============================================================================
void DebugTool::printEventAsTree( const LHCb::MCParticle::ConstVector &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());

  log << MSG::INFO << std::endl;
  printHeader( log, true, assoc != NULL );

  log.setf(std::ios::fixed,std::ios::floatfield);
  LHCb::MCParticle::ConstVector::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++ ) {
    if( ((*i)->originVertex() == NULL) ||
        ((*i)->originVertex()->mother() == NULL) )
      printDecayTree( *i, assoc, "", m_depth, log );
  }
  log << endreq;
}

//=============================================================================
// printEventAsTree (MCParticle) (KeyedContainer)
//=============================================================================
void DebugTool::printEventAsTree( const LHCb::MCParticles &event )
{
  printEventAsTree( event, NULL );
}

//=============================================================================
// printEventAsTree (MCParticle) (KeyedContainer)
//=============================================================================
void DebugTool::printEventAsTree( const LHCb::MCParticles &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());

  log << MSG::INFO << std::endl;
  printHeader( log, true, assoc != NULL );

  log.setf(std::ios::fixed,std::ios::floatfield);
  LHCb::MCParticles::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++ ) {
    if( ((*i)->originVertex() == NULL) ||
        ((*i)->originVertex()->mother() == NULL) )
      printDecayTree( *i, assoc, "", m_depth, log );
  }
  log << endreq;
}

//=============================================================================
// printDecayTree (Particle)
//=============================================================================
void DebugTool::printDecayTree( const LHCb::MCParticle *mother, 
                                Particle2MCLinker* assoc,
                                const std::string &prefix,
                                int depth, MsgStream &log )
{
  printInfo( prefix, mother, assoc, log );

  if( depth ) {
    SmartRefVector<LHCb::MCVertex>::const_iterator iv;
    for ( iv = mother->endVertices().begin();
          iv != mother->endVertices().end(); iv++ ) {
      SmartRefVector<LHCb::MCParticle>::const_iterator idau;
      for ( idau = (*iv)->products().begin();
            idau != (*iv)->products().end(); idau++ ) {
        if ( (*idau == (*iv)->products().back())
             && (*iv == mother->endVertices().back()) )
          printDecayTree( *idau, assoc, prefix+' ', depth-1, log );
        else
          printDecayTree( *idau, assoc, prefix+'|', depth-1, log );
      }
    }
  }
}

//=============================================================================
// printTree (Particle)
//=============================================================================
void DebugTool::printTree( const LHCb::Particle *mother, int maxDepth = -1 )
{
  printTree( mother, NULL, maxDepth );
}

//=============================================================================
// printTree (Particle)
//=============================================================================
void DebugTool::printTree( const LHCb::Particle *mother, 
                           Particle2MCLinker* assoc,
                           int maxDepth = -1 )
{
  if( maxDepth == -1 )
    maxDepth = m_depth;
  
  MsgStream log(msgSvc(), name());

  if( !mother ) {
    err() << "printTree called with NULL Particle" << endreq;
    return;
  }

  log << MSG::INFO << std::endl;
  printHeader( log, false, assoc != NULL );

  log.setf(std::ios::fixed,std::ios::floatfield);
  printDecayTree( mother, assoc, "", maxDepth, log );
  log << endreq;
}

//=============================================================================
// printDecayTree (Particle)
//=============================================================================
void DebugTool::printDecayTree( const LHCb::Particle *mother, 
                                Particle2MCLinker* assoc,
                                const std::string &prefix,
                                int depth, MsgStream &log ){
  printInfo( prefix, mother, assoc, log );

  if( depth ) {
    if( !mother->endVertex() )
      return;
    SmartRefVector<LHCb::Particle>::const_iterator iprod;
    for ( iprod = mother->endVertex()->outgoingParticles().begin();
          iprod != mother->endVertex()->outgoingParticles().end(); iprod++ ) {
      if ( (*iprod) != mother->endVertex()->outgoingParticles().back() )
        printDecayTree( *iprod, assoc, prefix+'|', depth-1, log );
      else
        printDecayTree( *iprod, assoc, prefix+' ', depth-1, log );
    }
  }
}

//=============================================================================
// printAncestor (MCParticle)
//=============================================================================
void DebugTool::printAncestor( const LHCb::MCParticle *child )
{
  MsgStream log( msgSvc(), name() );
  ParticleProperty *p = m_ppSvc->findByStdHepID(child->particleID().pid());
  std::string decay = p ? p->particle() : "N/A";
  const LHCb::MCVertex *origin = child->originVertex();
  while( origin && (child = origin->mother()) ) {
    p = m_ppSvc->findByStdHepID(child->particleID().pid());
    decay = (p ? p->particle() : std::string("N/A")) + " -> "+ decay;
    origin = child->originVertex();
  }
  log << MSG::INFO << decay << endreq;
}

//=============================================================================
// printEventAsList (Particle)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::Particle::ConstVector &event )
{
  printEventAsList( event, NULL );
}

//=============================================================================
// printEventAsList (Particle)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::Particle::ConstVector &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << std::endl;
  printHeader( log, false, assoc != NULL );

  LHCb::Particle::ConstVector::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++ )
    printInfo( "", *i, assoc, log );
  log << endreq;
}

//=============================================================================
// printEventAsList (MCParticle)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::MCParticle::ConstVector &event )
{
  printEventAsList( event, NULL );
}

//=============================================================================
// printEventAsList (MCParticle)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::MCParticle::ConstVector &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << std::endl;
  printHeader( log, true, assoc != NULL );
  
  int c = 0;
  LHCb::MCParticle::ConstVector::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++, c++ )
    printInfo( "", *i, assoc, log );
  log << endreq;
}

//=============================================================================
// printEventAsList (Particle) (KeyedContainer)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::Particles &event )
{
  printEventAsList( event, NULL );
}

//=============================================================================
// printEventAsList (Particle) (KeyedContained)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::Particles &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << std::endl;
  printHeader( log, false, assoc != NULL );

  LHCb::Particles::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++ )
    printInfo( "", *i, assoc, log );
  log << endreq;
}

//=============================================================================
// printEventAsList (MCParticle) (KeyedContainer)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::MCParticles &event )
{
  printEventAsList( event, NULL );
}

//=============================================================================
// printEventAsList (MCParticle) (KeyedContainer)
//=============================================================================
void DebugTool::printEventAsList( const LHCb::MCParticles &event,
                                  Particle2MCLinker* assoc )
{
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << std::endl;
  printHeader( log, true, assoc != NULL );
  
  int c = 0;
  LHCb::MCParticles::const_iterator i;
  for( i=event.begin(); i!=event.end(); i++, c++ )
    printInfo( "", *i, assoc, log );
  log << endreq;
}

