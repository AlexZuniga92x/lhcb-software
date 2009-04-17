
//---------------------------------------------------------------------------------
/** @file RichVaryGasPressureTemperatureAlg.h
 *
 * Header file for class : Rich::VaryGasPressureTemperatureAlg
 *
 * CVS Log :-
 * $Id: RichVaryGasPressureTemperatureAlg.h,v 1.1 2009-04-17 11:04:35 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date   2009/03/27
 */
//---------------------------------------------------------------------------------

#ifndef RICHALGORITHMS_RichVaryGasPressureTemperatureAlg_H
#define RICHALGORITHMS_RichVaryGasPressureTemperatureAlg_H 1

// STL
#include <cmath>

// RichKernel
#include "RichKernel/RichAlgBase.h"
#include "RichKernel/BoostArray.h"

// RichDet
#include "RichDet/DeRichRadiator.h"

namespace Rich
{

  //---------------------------------------------------------------------------------
  /** @class VaryGasPressureTemperatureAlg RichVaryGasPressureTemperatureAlg.h
   *
   *  Varies the gas temperature and pressures, to simulate varing conditions in MC data.
   *
   *  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
   *  @date   2009/03/27
   */
  //---------------------------------------------------------------------------------

  class VaryGasPressureTemperatureAlg : public Rich::AlgBase
  {

  public:

    /// Standard constructor
    VaryGasPressureTemperatureAlg( const std::string& name, ISvcLocator* pSvcLocator );

    virtual ~VaryGasPressureTemperatureAlg( ); ///< Destructor

    virtual StatusCode initialize();    ///< Algorithm initialization
    virtual StatusCode execute   ();    ///< Algorithm execution

  private:

    /// typedef of array of DeRichRadiators
    typedef boost::array<const DeRichRadiator *, Rich::NRadiatorTypes> Radiators;

    /// typedef of array of doubles for radiators
    typedef boost::array<double, Rich::NRadiatorTypes> RadiatorData;

    /// typedef of array of bools for radiators
    typedef boost::array<bool, Rich::NRadiatorTypes> RadiatorBools;

  private:

    /// Update the given radiator
    StatusCode updateRadiator( const Rich::RadiatorType rad );

  private:

    /// Flag for first event
    RadiatorBools m_firstEvent;

    /// Array of radiators
    Radiators m_radiators;

    /// Nominal pressure values
    RadiatorData m_nomPressure;

    /// Nominal pressure values
    RadiatorData m_nomTemperature;

    /// Event count
    unsigned long int m_nEvt;

    /// Pressure variation amplitude (Pa)
    double m_presVaryAmp;

    /// Pressure variation period (# events)
    unsigned long int m_presVaryPeriod;

    /// Tolerance on the pressure measurement values (Pa)
    double m_presTol;

    /// Overall constant shift in the pressure value
    double m_presVaryShift;

    /// Temperature variation amplitude (K)
    double m_tempVaryAmp;

    /// Temperature variation period (# events)
    unsigned int m_tempVaryPeriod;

    /// Tolerance on the temperature measurement values (K)
    double m_tempTol;

    /// Overall constant shift in the temperature value
    double m_tempVaryShift;

  };

}

#endif // RICHALGORITHMS_RichVaryGasPressureTemperatureAlg_H
