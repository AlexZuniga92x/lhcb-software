// $Id: Calo2MC.h,v 1.1 2006-02-23 21:30:45 odescham Exp $
#ifndef EVENT_CALO2MC_H 
#define EVENT_CALO2MC_H 1

class LHCb::MCParticle;
class LHCb::CaloCluster;
class LHCb::CaloDigit;
template <class FROM,class TO,class WEIGHT> class IRelationWeighted; 
template <class FROM,class TO> class LinkerWithKey;
template <class FROM,class TO> class LinkedTo;
namespace LHCb{ 
  template <class FROM,class TO,class WEIGHT> class RelationWeighted1D;
};

/** @class Calo2MC Calo2MC.h Event/Calo2MC.h
 *  
 *
 *  @author Olivier Deschamps
 *  @date   2006-02-21
 */
namespace LHCb { 
  namespace Calo2MC {
    typedef IRelationWeighted< LHCb::CaloCluster , LHCb::MCParticle , float >  IClusterTable ; 
    typedef LHCb::RelationWeighted1D<LHCb::CaloCluster,LHCb::MCParticle,float> ClusterTable ;
    
    typedef IRelationWeighted<LHCb::CaloDigit,LHCb::MCParticle,float>          IDigitTable; 
    typedef LHCb::RelationWeighted1D<LHCb::CaloDigit,LHCb::MCParticle,float>   DigitTable ; 

    typedef LinkerWithKey<LHCb::MCParticle,LHCb::CaloDigit>                   DigitLink ;
    typedef LinkedTo<LHCb::MCParticle,LHCb::CaloDigit>                        DigitLinkTo;

    //    typedef RelationWeighted1D<TrgCaloCluster,MCParticle,float> TrgClusterTable ;
    
  };
  
};

#endif // EVENT_CALO2MC_H 
