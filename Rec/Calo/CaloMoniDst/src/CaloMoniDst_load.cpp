// $Id: CaloMoniDst_load.cpp,v 1.2 2005-11-07 12:16:38 odescham Exp $
// ============================================================================
// CVS tag $Name: not supported by cvs2svn $
// ============================================================================
// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2004/10/25 09:00:25  ibelyaev
// // The new pckage: code is imported from Calo/CaloMonitor
// 
// ============================================================================
// Include files 
// ============================================================================
#include "GaudiKernel/DeclareFactoryEntries.h"
// ============================================================================

/** @file 
 *  The mandatory file to build shared component library
 *  @author Vanya BELYAEV belyaev@lapp.in2p3.fr
 *  @date 2004-10-25
 */

DECLARE_FACTORY_ENTRIES(CaloMoniDst) {
  
  DECLARE_ALGORITHM( CaloClusterMCTruthMonitor ) ;
  
  DECLARE_ALGORITHM( CaloClusterMatchMonitor   ) ;
  DECLARE_ALGORITHM( CaloHypoMatchMonitor      ) ;
  
  DECLARE_ALGORITHM( CaloClusterMonitor        ) ;
  DECLARE_ALGORITHM( CaloHypoMonitor           ) ;
  
  DECLARE_ALGORITHM( CaloMCPi0Monitor          ) ;
  DECLARE_ALGORITHM( CaloPi0Monitor            ) ;
  DECLARE_ALGORITHM( CaloPhotonMonitor         ) ;
  DECLARE_ALGORITHM( CaloPIDsMonitor           ) ;

};

// ============================================================================
// The END 
// ============================================================================
