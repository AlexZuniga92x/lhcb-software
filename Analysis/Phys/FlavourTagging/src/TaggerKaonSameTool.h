// $Id: TaggerKaonSameTool.h,v 1.8 2009-11-30 22:42:51 musy Exp $
#ifndef USER_TAGGERKAONSAMETOOL_H 
#define USER_TAGGERKAONSAMETOOL_H 1

// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolFactory.h"
// from Event
#include "Event/FlavourTag.h"
#include "Event/ProtoParticle.h"
#include "Kernel/ITagger.h"

// from local
#include "INNetTool.h"
#include "ITaggingUtils.h"

/** @class TaggerKaonSameTool TaggerKaonSameTool.h 
 *
 *  Tool to tag the B flavour with a KaonSame Tagger
 *
 *  @author Marco Musy
 *  @date   30/06/2005
 */

class TaggerKaonSameTool : public GaudiTool, 
	virtual public ITagger {

public: 

  /// Standard constructor
  TaggerKaonSameTool( const std::string& type,
		      const std::string& name,
		      const IInterface* parent );
  virtual ~TaggerKaonSameTool( ); ///< Destructor
  StatusCode initialize();    ///<  initialization
  StatusCode finalize  ();    ///<  finalization

  //-------------------------------------------------------------
  virtual LHCb::Tagger tag( const LHCb::Particle*, const LHCb::RecVertex*,
			    std::vector<const LHCb::Vertex*>&, 
			    LHCb::Particle::ConstVector&);
  //-------------------------------------------------------------

private:

  INNetTool* m_nnet;
  ITaggingUtils* m_util;
  std::string m_CombinationTechnique, m_NeuralNetName;

  //properties 
  double m_Pt_cut_kaonS;
  double m_P_cut_kaonS;
  double m_IP_cut_kaonS;
  double m_phicut_kaonS;
  double m_etacut_kaonS;
  double m_dQcut_kaonS;
  double m_lcs_cut;
  double m_KaonSPID_extracut;
  
  double m_AverageOmega;
};

//===============================================================//
#endif // USER_TAGGERKAONSAMETOOL_H
