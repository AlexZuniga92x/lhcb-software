// $Id: CaloCorrectionBase.h,v 1.1 2010-05-20 09:47:06 odescham Exp $
#ifndef CALOCORRECTIONBASE_H 
#define CALOCORRECTIONBASE_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "DetDesc/Condition.h"
#include "CaloUtils/CaloAlgUtils.h"
#include "Kernel/CaloCellID.h"
#include "CaloUtils/ClusterFunctors.h"
#include "CaloDet/DeCalorimeter.h"
#include "Event/CaloHypo.h"
#include "Event/CaloDigit.h"
#include "GaudiKernel/Point3DTypes.h"
#include "CaloUtils/CaloAlgUtils.h"

static const InterfaceID IID_CaloCorrectionBase ( "CaloCorrectionBase", 1, 0 );

/** @class CaloCorrectionBase CaloCorrectionBase.h
 *  
 *
 *  @author Olivier Deschamps
 *  @date   2010-05-07
 */


namespace CaloCorrectionUtils{
  class DigitFromCalo : public std::unary_function<const LHCb::CaloDigit*,bool>{ 
  public:
    explicit DigitFromCalo( const std::string& calo ): m_calo( CaloCellCode::CaloNumFromName( calo ) ) {} ;
    explicit DigitFromCalo( const int  calo ): m_calo(                                calo   ) {} ;
    inline bool operator() ( const LHCb::CaloDigit* digit ) const{
      if( 0 == digit ) { return false ; }
      return ( ( (int) digit->cellID().calo() ) == m_calo ) ;
    };
  private:
    DigitFromCalo();
  private:
    int m_calo ;
  };
};

namespace CaloCorrection {
  enum  Function{
    InversPolynomial=0,
    Polynomial,
    ExpPolynomial,
    ReciprocalPolynomial,
    Unknown  // MUST be the last item 
  };
  typedef std::pair<CaloCorrection::Function , std::vector<double> > Parameters;
  enum Type{
    // E-Correction parameters
    alphaG,
    alphaE,
    alphaB,
    alphaX,
    alphaY,
    beta,
    globalC,
    // L-Correction parameters
    gamma0,
    delta0,
    gammaP,
    deltaP,
    lastType  // MUST BE THE LAST LINE
  };
  static const int nT = lastType+1;
  static const int nF = Unknown+1;
  static const std::string typeName[nT] = { "alphaG", "alphaE","alphaB","alphaX","alphaY","beta","globalC"
                                            ,"gamma0","delta0","gammaP","deltaP"
                                            ,"Unknown"};    
  static const std::string funcName[nF] = { "InversPolynomial", "Polynomial","ExpPolynomial","ReciprocalPolynomial","Unknown"};  
}


class CaloCorrectionBase : public GaudiTool {
public: 

  // Return the interface ID
  static const InterfaceID& interfaceID() { return IID_CaloCorrectionBase; }

  /// Standard constructor
  CaloCorrectionBase( const std::string& type, 
        const std::string& name,
        const IInterface* parent);

  virtual ~CaloCorrectionBase( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  void setOrigin(Gaudi::XYZPoint origin){m_origin = origin;}
  StatusCode updParams();
protected:
  double getCorrection(CaloCorrection::Type type, const LHCb::CaloCellID id , double var = 0.,double def = 1.) const;
  std::string m_conditionName;
  std::vector<std::string> m_corrections;
  //
  typedef std::vector<LHCb::CaloHypo::Hypothesis>   Hypotheses  ;
  typedef std::vector<int>                          Hypotheses_ ;
  Hypotheses  m_hypos     ;
  Hypotheses_ m_hypos_    ;
  LHCb::ClusterFunctors::ClusterArea      m_area;
  LHCb::ClusterFunctors::ClusterFromCalo  m_calo;
  CaloCorrectionUtils::DigitFromCalo    m_spd;
  CaloCorrectionUtils::DigitFromCalo    m_prs;
  std::string m_detData;
  const DeCalorimeter* m_det;
  void getPrsSpd(const LHCb::CaloHypo* hypo,double& ePrs,double& eSpd) const{
    typedef const LHCb::CaloHypo::Digits   Digits   ;
    const Digits& digits = hypo->digits();
    for( Digits::const_iterator d = digits.begin() ; digits.end() != d ; ++d ){ 
      if     ( *d == 0     ) { continue           ; }
      else if( m_prs( *d ) ) { ePrs  += (*d)->e() ; } 
      else if( m_spd( *d ) ) { eSpd  += (*d)->e() ; }
    }
  }
  Gaudi::XYZPoint  m_origin;

private:

  bool accept(std::string name){
    for( std::vector<std::string>::iterator it = m_corrections.begin() ; m_corrections.end() != it ; ++it){
      if( name == *it || *it == "All")return true;
    }    
    return false;
  }

  
  StatusCode setOptParams();
  StatusCode setDBParams();
  CaloCorrection::Parameters getParams(CaloCorrection::Type type, const LHCb::CaloCellID id) const;
  void checkParams();
  
  

  std::map<std::string, std::vector<double> > m_params;
  bool m_useCondDB;
  std::map<std::string, std::vector<double> > m_optParams;
  Condition* m_cond;
};



#endif // CALOCORRECTIONBASE_H
