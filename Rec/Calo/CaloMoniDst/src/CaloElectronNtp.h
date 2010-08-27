// $Id: CaloElectronNtp.h,v 1.1 2010/05/20 09:55:38 odescham Exp $
#ifndef CALOELECTRONNTP_H 
#define CALOELECTRONNTP_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTupleAlg.h"
#include "Event/ProtoParticle.h"
#include "CaloUtils/ICaloElectron.h"
#include "CaloInterfaces/ICaloHypo2Calo.h"
#include "TrackInterfaces/ITrackExtrapolator.h"
#include "CaloDet/DeCalorimeter.h"
#include "GaudiKernel/IEventTimeDecoder.h"

/** @class CaloElectronNtp CaloElectronNtp.h
 *  
 *
 *  @author Olivier Deschamps
 *  @date   2009-12-11
 */
class CaloElectronNtp : public GaudiTupleAlg {
public: 
  /// Standard constructor
  CaloElectronNtp( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~CaloElectronNtp( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization


  ITrackExtrapolator* extrapolator(){return m_extrapolator;};

protected:

private:
  bool inRange( std::pair<double,double> range, double value){
    return ( value >= range.first) && (value <= range.second);
  }
  

  ITrackExtrapolator*  m_extrapolator;
  ICaloElectron* m_caloElectron;
  bool m_pairing;
  std::string m_extrapolatorType;
  std::pair<double,double> m_e;
  std::pair<double,double> m_et;
  std::pair<double,double> m_prs;
  std::pair<double,double> m_eop;
  std::string m_input;
  ICaloHypo2Calo* m_toSpd ;   
  ICaloHypo2Calo* m_toPrs ;
  DeCalorimeter* m_calo;
  IEventTimeDecoder* m_odin;
  bool m_histo;
  bool m_tupling;
};
#endif // CALOELECTRONNTP_H
