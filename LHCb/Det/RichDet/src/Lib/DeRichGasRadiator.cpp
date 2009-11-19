
//----------------------------------------------------------------------------
/** @file DeRichGasRadiator.cpp
 *
 *  Implementation file for detector description class : DeRichGasRadiator
 *
 *  CVS Log :-
 *  $Id: DeRichGasRadiator.cpp,v 1.17 2009-11-19 13:15:38 papanest Exp $
 *
 *  @author Antonis Papanestis a.papanestis@rl.ac.uk
 *  @date   2006-03-02
 */
//----------------------------------------------------------------------------

// Include files

// Gaudi
#include "GaudiKernel/IUpdateManagerSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

// local
#include "RichDet/DeRichGasRadiator.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DeRichGasRadiator
//
// 2006-03-02 : Antonis Papanestis
//-----------------------------------------------------------------------------

const CLID CLID_DeRichGasRadiator = 12042;  // User defined

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DeRichGasRadiator::DeRichGasRadiator(const std::string & name) :
  DeRichSingleSolidRadiator(name),
  m_temperatureCond (),
  m_hltTemperatureCond (),
  m_pressureCond    (),
  m_hltPressureCond (),
  m_scaleFactorCond ()
{}

//=============================================================================
// Destructor
//=============================================================================
DeRichGasRadiator::~DeRichGasRadiator() {}

//=========================================================================
// Retrieve Pointer to class defininition structure
//=========================================================================
const CLID& DeRichGasRadiator::classID() { return CLID_DeRichGasRadiator; }

//=========================================================================
//  initialize
//=========================================================================
StatusCode DeRichGasRadiator::initialize ( )
{

  MsgStream msg = msgStream( "DeRichGasRadiator" );
  msg << MSG::DEBUG << "Initialize " << name() << endmsg;

  StatusCode sc = DeRichSingleSolidRadiator::initialize();
  if ( sc.isFailure() ) return sc;

  // configure refractive index updates

  // temperature
  if ( hasCondition( "GasTemperature" ) )
  {
    m_temperatureCond = condition( "GasTemperature" );
    updMgrSvc()->registerCondition(this, m_temperatureCond.path(),
                                   &DeRichGasRadiator::updateProperties );
  }
  else
  {
    m_temperatureCond = 0;
    msg << MSG::WARNING << "Cannot load Condition GasTemperature" << endmsg;
    return StatusCode::FAILURE;
  }

  // pressure
  if ( hasCondition( "GasPressure" ) )
  {
    m_pressureCond = condition( "GasPressure" );
    updMgrSvc()->registerCondition( this, m_pressureCond.path(),
                                    &DeRichGasRadiator::updateProperties );
  }
  else
  {
    m_pressureCond = 0;
    msg << MSG::WARNING << "Cannot load Condition GasPressure" << endmsg;
    return StatusCode::FAILURE;
  }

  // scale factor
  if ( hasCondition( "RefractivityScaleFactor" ) )
  {
    m_scaleFactorCond = condition( "RefractivityScaleFactor" );
    updMgrSvc()->registerCondition( this, m_scaleFactorCond.path(),
                                    &DeRichGasRadiator::updateProperties );
  }

  sc = updMgrSvc()->update(this);
  if ( sc.isFailure() )
  {
    msg << MSG::ERROR << "First UMS update failed" << endmsg;
    return sc;
  }

  // HLT temperature
  if ( hasCondition( "HltGasTemperature" ) )
  {
    m_hltTemperatureCond = condition( "HltGasTemperature" );
    if ( m_hltTemperatureCond == 0 )
    {
      msg << MSG::FATAL << "Cannot access condition HltGasTemperature" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  else
    m_hltTemperatureCond = m_temperatureCond;

  // HLT pressure
  if ( hasCondition( "HltGasPressure" ) )
  {
    m_hltPressureCond = condition( "HltGasPressure" );
    if ( m_hltPressureCond == 0 )
    {
      msg << MSG::FATAL << "Cannot access condition HltGasPressure" << endmsg;
      return StatusCode::FAILURE;
    }
  }
  else
    m_hltPressureCond =  m_pressureCond;

  msg << MSG::DEBUG << "Initialisation Complete" << endmsg;
  m_firstUpdate = false;

  // return
  return sc;
}

//=========================================================================
// updateRefIndex
//=========================================================================
StatusCode DeRichGasRadiator::updateProperties ( )
{

  // load parameters
  // get temperature and pressure
  const double photonEnergyLowLimit     = param<double>("PhotonMinimumEnergy");
  const double photonEnergyHighLimit    = param<double>("PhotonMaximumEnergy");
  const double ckvPhotonEnergyLowLimit  = param<double>("PhotonCkvMinimumEnergy");
  const double ckvPhotonEnergyHighLimit = param<double>("PhotonCkvMaximumEnergy");
  const unsigned int photonEnergyNumBins  = param<int>("PhotonEnergyNumBins");
  const unsigned int ckvPhotonEnergyNumBins = param<int>("CkvPhotonEnergyNumBins");

  if ( !m_firstUpdate )
  {
    const double curPressure = m_pressureCond->param<double>("CurrentPressure");
    const double curTemp     = m_temperatureCond->param<double>("CurrentTemperature");
    info() << "Refractive index update triggered : Pressure = " << curPressure/Gaudi::Units::Pa
           << " Pa Temperature = " << curTemp << " K"
           << endmsg;
  }

  if ( (photonEnergyHighLimit < ckvPhotonEnergyHighLimit ) ||
       (ckvPhotonEnergyLowLimit < photonEnergyLowLimit ) )
  {
    error() << "Inadmissible photon energy limits "
            << photonEnergyHighLimit << " " << ckvPhotonEnergyHighLimit
            << ckvPhotonEnergyLowLimit << " " << photonEnergyLowLimit << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<double> photonMomentumVect;
  StatusCode sc = prepareMomentumVector( photonMomentumVect, photonEnergyLowLimit,
                                         photonEnergyHighLimit, photonEnergyNumBins );
  if ( !sc ) return sc;

  // calculate the refractive index and update Tabulated property
  sc = calcSellmeirRefIndex( photonMomentumVect, m_refIndexTabProp,
                             m_pressureCond, m_temperatureCond );
  if ( !sc ) return sc;

  std::vector<double> ckvPhotonMomentumVect;
  sc = prepareMomentumVector( ckvPhotonMomentumVect, ckvPhotonEnergyLowLimit,
                              ckvPhotonEnergyHighLimit, ckvPhotonEnergyNumBins );
  if ( !sc ) return sc;

  sc = calcSellmeirRefIndex( ckvPhotonMomentumVect, m_chkvRefIndexTabProp,
                             m_pressureCond, m_temperatureCond );
  if ( !sc ) return sc;

  // Hack - Update interpolators in base class after first update
  sc = initTabPropInterpolators();

  return sc;
}

//=========================================================================
//  calcSellmeirRefIndex
//=========================================================================
StatusCode DeRichGasRadiator::calcSellmeirRefIndex (const std::vector<double>& momVect,
                                                    const TabulatedProperty* tabProp,
                                                    SmartRef<Condition> pressureCond,
                                                    SmartRef<Condition> temperatureCond ) const
{

  // test the tab property pointer
  if ( !tabProp )
  {
    error() << "NULL TabulatedProperty pointer" << endmsg;
    return StatusCode::FAILURE;
  }

  // get temperature and pressure
  const double curPressure = pressureCond->param<double>("CurrentPressure");
  const double curTemp     = temperatureCond->param<double>("CurrentTemperature");
  double scaleFactor( 1.0 );
  if ( m_scaleFactorCond )
    scaleFactor = m_scaleFactorCond->param<double>("CurrentScaleFactor");

  // reset table
  TabulatedProperty* modTabProp = const_cast<TabulatedProperty*>( tabProp );
  TabulatedProperty::Table& aTable = modTabProp->table();
  aTable.clear();
  aTable.reserve( momVect.size() );

  // Get parameters
  const double SellE1 = param<double>("SellE1Param");
  const double SellE2 = param<double>("SellE2Param");
  const double SellF1 = param<double>("SellF1Param");
  const double SellF2 = param<double>("SellF2Param");
  const double SellLorGasFac = param<double>("SellmeirLorenzFact");
  const double RhoEffectiveSellDefault = param<double>("RhoEffectiveSellParam");
  const double GasMolWeight   = param<double>("GasMolWeightParam");
  double GasRhoCur( 0.0 );

  if ( material()->name().find("C4F10") != std::string::npos ) {
    const double RefTemperature = param<double>("C4F10ReferenceTemp");
    GasRhoCur = RhoEffectiveSellDefault*(curPressure/Gaudi::Units::STP_Pressure)*
      ( RefTemperature/curTemp );
  }
  else {
    GasRhoCur = RhoEffectiveSellDefault*(curPressure/Gaudi::Units::STP_Pressure)*
      (Gaudi::Units::STP_Temperature/curTemp);
  }

  // calculate ref index
  for ( unsigned int ibin = 0; ibin<momVect.size(); ++ibin )
  {
    const double epho = momVect[ibin]/Gaudi::Units::eV;
    const double pfe  = (SellF1/( (SellE1* SellE1) - (epho * epho) ) )+
      (SellF2/( (SellE2*SellE2) - (epho * epho) ));
    const double cpfe = SellLorGasFac * (GasRhoCur / GasMolWeight ) * pfe;
    const double nMinus1 = scaleFactor * (sqrt((1.0+2*cpfe)/(1.0-cpfe)) - 1.0);
    const double curRindex = 1.0+nMinus1;
    aTable.push_back( TabulatedProperty::Entry(epho*Gaudi::Units::eV,curRindex));
  }

  debug() << "Table in TabulatedProperty " << tabProp->name()
          << " updated with " << momVect.size() << " bins" << endmsg;

  return StatusCode::SUCCESS;
}

//=========================================================================
//  generateHltRefIndex
//=========================================================================
const Rich::TabulatedProperty1D* DeRichGasRadiator::generateHltRefIndex() const
{
  m_hltRefIndexTabProp = new TabulatedProperty("HltRefIndexTabProperty");

  DeRichGasRadiator* nonConstSelf = const_cast<DeRichGasRadiator*>(this);

  // temperature
  if ( m_hltTemperatureCond !=  m_temperatureCond )
    updMgrSvc()->registerCondition(nonConstSelf, m_hltTemperatureCond.path(),
                                   &DeRichGasRadiator::updateHltProperties );

  // pressure
  if ( m_hltPressureCond != m_pressureCond )
    updMgrSvc()->registerCondition(nonConstSelf, m_hltPressureCond.path(),
                                   &DeRichGasRadiator::updateHltProperties );

  if (  m_hltTemperatureCond !=  m_temperatureCond || m_hltPressureCond != m_pressureCond )
  {
    StatusCode sc = updMgrSvc()->update(nonConstSelf);
    if ( sc.isFailure() )
    {
      error() << "First UMS update failed for HLT properties" << endmsg;
      return 0;
    }
  }
  else
    nonConstSelf->updateHltProperties();


  return m_hltRefIndex;
}

//=========================================================================
//  updateHltProperties
//=========================================================================
StatusCode DeRichGasRadiator::updateHltProperties ( )
{

  // load parameters
  const double photonEnergyLowLimit     = param<double>("PhotonMinimumEnergy");
  const double photonEnergyHighLimit    = param<double>("PhotonMaximumEnergy");
  const unsigned int photonEnergyNumBins  = param<int>("PhotonEnergyNumBins");

  if ( photonEnergyHighLimit < photonEnergyLowLimit )
  {
    error() << "Inadmissible photon energy limits "
            << photonEnergyHighLimit << " " << photonEnergyLowLimit << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<double> photonMomentumVect;
  StatusCode sc = prepareMomentumVector( photonMomentumVect, photonEnergyLowLimit,
                                         photonEnergyHighLimit, photonEnergyNumBins );
  if ( !sc ) return sc;

  // calculate the refractive index and update Tabulated property
  sc = calcSellmeirRefIndex( photonMomentumVect, m_hltRefIndexTabProp,
                             m_hltPressureCond, m_hltTemperatureCond );
  if ( !sc ) return sc;


  if ( !m_hltRefIndex )
  { m_hltRefIndex = new Rich::TabulatedProperty1D( m_hltRefIndexTabProp ); }
  else
  { m_hltRefIndex->initInterpolator( m_hltRefIndexTabProp ); }
  if ( !m_hltRefIndex->valid() )
  {
    error() << "Invalid RINDEX Rich::TabulatedProperty1D for "
            << m_hltRefIndexTabProp->name() << endmsg;
    return StatusCode::FAILURE;
  }

  return sc;
}
//=============================================================================
