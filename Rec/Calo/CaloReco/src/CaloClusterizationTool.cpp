// $Id: CaloClusterizationTool.cpp,v 1.5 2009-06-17 18:24:00 odescham Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 
#include "GaudiKernel/SystemOfUnits.h"
// ============================================================================
#include "DetDesc/IGeometryInfo.h"
// ============================================================================
#include "Event/CaloDigit.h"
#include "Event/CaloCluster.h"
#include "CaloUtils/CaloDataFunctor.h"
#include "Event/CellID.h"
#include "CaloKernel/CaloUtil.h"
// ============================================================================
#include "CaloUtils/ClusterFunctors.h"
// local
#include "CaloClusterizationTool.h"
#include "TaggedCellFunctor.h"
// ============================================================================
// Boost 
// ============================================================================
#include "boost/lexical_cast.hpp"

//-----------------------------------------------------------------------------
// Implementation file for class : CaloClusterizationTool
//
// 2008-04-04 : Victor Egorychev
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( CaloClusterizationTool );

// ============================================================================
inline bool CaloClusterizationTool::isLocMax
( const LHCb::CaloDigit*                      digit ,
  const CaloClusterizationTool::DirVector&   hits  ,
  const DeCalorimeter*                  det   ) 
{

  const CaloNeighbors& ns = det->neighborCells( digit -> cellID() ) ;
  double e = digit -> e() ;
  
  LHCb::CaloDataFunctor::EnergyTransverse<const LHCb::CaloDigit*,const DeCalorimeter*> et ( det ) ;

  double eT = 0 ;

  if ( m_withET ) { eT = et ( digit ) ; }
  
  
  for ( CaloNeighbors::const_iterator iN = ns.begin() ; ns.end() != iN ; ++iN ){
    const CelAutoTaggedCell* cell = hits[*iN];
    if ( 0 == cell   ) { continue     ; }  
    const LHCb::CaloDigit* nd = cell->digit() ;
    if ( 0 == nd     ) { continue     ; }
    
    if ( m_withET ) { eT += et ( nd ) ; }
    
    if ( nd->e() > e ) { return false ; }
  }

  if ( m_withET && eT < m_ETcut ) { return false ; }
  
  return true ;
};
// ============================================================================

// ============================================================================
/** Application of rules of tagging on one cell
 *  - No action if no clustered neighbor 
 *   - Clustered if only one clustered neighbor
 *   - Edge if more then one clustered neighbor 
 */
// ============================================================================
inline void 
CaloClusterizationTool::appliRulesTagger( CelAutoTaggedCell*             cell ,
                                          CaloClusterizationTool::DirVector&  hits ,
                                          const DeCalorimeter*           det  ){
  
  // Find in the neighbors cells tagged before, the clustered neighbors cells
  const LHCb::CaloCellID&    cellID = cell->cellID() ;
  const CaloNeighbors& ns     = det->neighborCells ( cellID ) ;
  bool hasEdgeNeighbor = false;
  bool hasClusteredNeighbor = false;
  std::vector<LHCb::CaloCellID> neighborSeeds;
  for ( CaloNeighbors::const_iterator iN = ns.begin() ; ns.end() != iN ; ++iN ){
    const CelAutoTaggedCell* nei = hits[ *iN ] ;
    if ( 0 == nei                 ) { continue ; }
    // 
    if( nei->isEdge() && m_release ){
      hasEdgeNeighbor = true;
      for( std::vector<LHCb::CaloCellID>::const_iterator id = nei->seeds().begin();id != nei->seeds().end();id++){
        bool ok = true;
        for(std::vector<LHCb::CaloCellID>::iterator iid = neighborSeeds.begin();iid != neighborSeeds.end();iid++){
          if( *id == *iid ){
            ok = false;
            break;
          }
        }
        if( ok )cell->addSeed( *id );
      }
    }
    if ( !nei->isClustered()      ) { continue ; }
    hasClusteredNeighbor = true;
    const LHCb::CaloCellID& seed = nei->seedForClustered() ;
    if ( cell->isWithSeed( seed ) ) { continue ; }
    cell->addSeed ( seed ) ;
  }
  
  // Tag or or not the cell
  
  switch ( cell -> numberSeeds() ) {
  case 0:
    if( !m_release)break;
    if( hasEdgeNeighbor && !hasClusteredNeighbor){
      cell->setEdge();
      for(std::vector<LHCb::CaloCellID>::iterator iid = neighborSeeds.begin();iid != neighborSeeds.end();iid++){
        cell->addSeed( *iid );
      }      
    }
    break;
  case 1: 
    cell->setClustered();
    break;
  default: 
    cell->setEdge();
    break;
  } 
};

// ============================================================================
inline void CaloClusterizationTool::setEXYCluster
( LHCb::CaloCluster* cluster,
  const DeCalorimeter* detector ) 
{
  ///
  double E, X, Y;
  ///
  StatusCode sc = LHCb::ClusterFunctors::calculateEXY
    ( cluster->entries().begin() ,
      cluster->entries().end  () ,
      detector , E , X , Y      );
  ///
  if( sc.isSuccess() )
  {
    cluster->position().parameters()( LHCb::CaloPosition::E ) = E ;
    cluster->position().parameters()( LHCb::CaloPosition::X ) = X ;
    cluster->position().parameters()( LHCb::CaloPosition::Y ) = Y ;
  }
  else 
  { Error( " E,X and Y of cluster could not be evaluated!",sc); }
  ///
};
// ============================================================================

// ============================================================================
// Looking neigbours around cell
// ============================================================================
void CaloClusterizationTool::look_neig( std::set<LHCb::CaloCellID> in_cells ,
                                        const DeCalorimeter* m_detector,
                                        std::set<LHCb::CaloCellID>& out_cells ){
  
  for(std::set<LHCb::CaloCellID>::const_iterator icell = in_cells.begin();
      in_cells.end() != icell; ++icell){
    const CaloNeighbors& nei = m_detector->neighborCells( (*icell) );
    out_cells.insert( nei.begin(), nei.end() );
  }
}

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CaloClusterizationTool::CaloClusterizationTool( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : GaudiTool ( type, name , parent )  
  , m_withET           ( false )
  , m_ETcut            ( -10. * Gaudi::Units::GeV)
{
  // eTcut
  declareProperty ( "withET" , m_withET ) ;
  declareProperty ( "ETcut" , m_ETcut ) ;
  declareProperty ( "CellSelectorForEnergy" , m_used = "3x3");

  declareInterface<ICaloClusterization>(this);
};

// ============================================================================
/// initialization
// ============================================================================
StatusCode CaloClusterizationTool::initialize() 
{
  StatusCode sc = GaudiTool::initialize() ;
  if( sc.isFailure() ) 
    { return Error("Could not initialize base class CaloAlgorithm",sc);}
  
  return StatusCode::SUCCESS;
}

template <class TYPE>
StatusCode CaloClusterizationTool::_clusterize(std::vector<LHCb::CaloCluster*>&     clusters   ,
                                        const TYPE& hits, const DeCalorimeter* m_detector, 
                                        const std::vector<LHCb::CaloCellID>&  _cell_list, 
                                        const unsigned int m_neig_level){

  //
  m_cellSelector.setSelector(m_used);  
  m_cellSelector.setDet(m_detector);
  m_release = false;
  bool useData = false;
  //
  std::set<LHCb::CaloCellID> out_cells;
  std::vector<LHCb::CaloCellID> cell_list;

  // fill with data if level >0 and no predefined seed list
  if( cell_list.empty() && m_neig_level>0){
    useData = true;

    typename TYPE::const_iterator it;
    for(it = hits.begin() ; it != hits.end(); it++){
      const CaloNeighbors& neighbors = m_detector->neighborCells( (*it) -> cellID() ) ;
      double e = (*it)->e();
      bool loc = true;
      for(CaloNeighbors::const_iterator in = neighbors.begin(); in!=neighbors.end();in++){
        if( !m_detector->valid( *in ))continue;
        LHCb::CaloDigit* dig = hits( *in );
        if( dig == NULL)continue;
        if( dig->e() > e){
          loc = false;
          break;
        }  
      }
      if(loc)cell_list.push_back( (*it)->cellID() );
    }      
  }
  else{
    cell_list = _cell_list;
  }
  

  // if list of "seed" is not empty
  if( !cell_list.empty() ){

    // look Cell neigbours
    std::set<LHCb::CaloCellID> cells;

    for( std::vector<LHCb::CaloCellID>::const_iterator icell = 
           cell_list.begin() ; cell_list.end() != icell ; ++icell){  
      const CaloNeighbors& nei = m_detector->neighborCells( (*icell) );
      cells.insert( nei.begin(), nei.end() );
      cells.insert( (*icell) );
    }

    // look Cell neigbours for next levels
    std::set<LHCb::CaloCellID> in_cells = cells;

    for(int i = m_neig_level; i > 1; i--){      
      look_neig(in_cells, m_detector, out_cells);
      in_cells = out_cells;
    }
    
  }
  
  // z-position of cluster 
  LHCb::ClusterFunctors::ZPosition zPosition( m_detector );
  
  // Create access direct and sequential on the tagged cells  
  DirVector taggedCellsDirect( (CelAutoTaggedCell*) 0 ) ;
  SeqVector taggedCellsSeq                              ;
  typedef std::vector<CelAutoTaggedCell> _Local ;

  size_t local_size = cell_list.empty() ? hits.size() : out_cells.size();
  const CelAutoTaggedCell cell0_ = CelAutoTaggedCell () ; 
  _Local local_cells ( local_size  , cell0_ ) ;  

  if( cell_list.empty() ){
    taggedCellsDirect.reserve ( hits.size() ) ;
    taggedCellsDirect.setSize ( 14000        ) ;
    taggedCellsSeq.reserve    ( hits.size() ) ;
  }
  else{
    taggedCellsDirect.reserve ( out_cells.size() ) ;
    taggedCellsDirect.setSize ( 14000            ) ;
    taggedCellsSeq.reserve    ( out_cells.size() ) ;
  }

  if( cell_list.empty() ){ 
    // fill with the data
    size_t index = 0 ;

    for( LHCb::CaloDigits::const_iterator ihit = 
           hits.begin() ; hits.end() != ihit ; ++ihit , ++index ){
      const LHCb::CaloDigit* digit   = *ihit ;
      if ( 0 == digit ) { continue ; }  // CONTINUE !!! 
      CelAutoTaggedCell& taggedCell = *(local_cells.begin() + index ) ;
      taggedCell = digit ;

      taggedCellsDirect.addEntry ( &taggedCell , digit->cellID() ) ;
      taggedCellsSeq.push_back   ( &taggedCell                   ) ;

    }    
  }else{//fill for HLT    
    size_t index = 0 ;

    for(std::set<LHCb::CaloCellID>::const_iterator icell = out_cells.begin();
        out_cells.end() != icell; ++icell , ++index){

      const LHCb::CaloDigit* digit   = hits(*icell);
      if ( 0 == digit ) { continue ; }  // CONTINUE !!! 
      
      CelAutoTaggedCell& taggedCell = *(local_cells.begin() + index ) ;
      taggedCell = digit ;

      taggedCellsDirect.addEntry ( &taggedCell , digit->cellID() ) ;
      taggedCellsSeq.push_back   ( &taggedCell                   ) ;
    }
    
  }


   // Find and mark the seeds (local maxima) 
  if(useData){
    for(std::vector<LHCb::CaloCellID>::iterator seed = cell_list.begin();seed!=cell_list.end();seed++){
      taggedCellsDirect[*seed]->setIsSeed();
    }    
  }
  else{
    for( SeqVector::iterator itTag = taggedCellsSeq.begin(); taggedCellsSeq.end() != itTag ; ++itTag ){ 
      if ( isLocMax ( (*itTag)->digit() ,taggedCellsDirect ,m_detector)       ) { 
        (*itTag)->setIsSeed(); 
      } 
    }
  }
  
  /// Tag the cells which are not seeds
  SeqVector::iterator itTagLastSeed = std::stable_partition( taggedCellsSeq.begin () ,
                                                             taggedCellsSeq.end   () ,
                                                             TaggedCellFunctor::isClustered() );

  SeqVector::iterator itTagLastClustered = itTagLastSeed      ;     
  SeqVector::iterator itTagFirst         = itTagLastClustered ;
  m_pass = 0;
  while ( itTagLastClustered != taggedCellsSeq.end() ) {

    // Apply rules tagger for all not tagged cells
    for ( SeqVector::iterator itTag = itTagLastClustered ; taggedCellsSeq.end() != itTag ; ++itTag ){
      appliRulesTagger( (*itTag),  taggedCellsDirect , m_detector ); 
    }
    
    // Valid result
    std::for_each ( itTagFirst, taggedCellsSeq.end(), TaggedCellFunctor::setStatus() );
    
    itTagLastClustered = std::stable_partition( itTagFirst,
                                                taggedCellsSeq.end(),
                                                TaggedCellFunctor::isClusteredOrEdge() );

    // Test if cells are tagged in this pass    
    if ( itTagLastClustered == itTagFirst && m_release ){
      const long number = taggedCellsSeq.end() - itTagLastClustered ;
      debug() << " TAGGING NOT FULL - Remain " 
              << boost::lexical_cast<std::string> ( number  )
              << " not clustered cells" << endmsg ;
      itTagLastClustered = taggedCellsSeq.end();
    }
    if( itTagLastClustered == itTagFirst )m_release = true; // try additional passes releasing appliRulesTagger criteria
    m_pass++;
    itTagFirst = itTagLastClustered;
  }  


  
  LHCb::CaloDigitStatus::Status used   = LHCb::CaloDigitStatus::UseForEnergy  | LHCb::CaloDigitStatus::UseForPosition |
    LHCb::CaloDigitStatus::UseForCovariance  ;
  LHCb::CaloDigitStatus::Status seed   = LHCb::CaloDigitStatus::SeedCell | LHCb::CaloDigitStatus::LocalMaximum | used ;
  
  
  itTagLastClustered = std::stable_partition( itTagLastSeed                    ,
                                              taggedCellsSeq.end()             ,
                                              TaggedCellFunctor::isClustered() ) ;  
  SeqVector::iterator itTagSeed = taggedCellsSeq.begin();
  SeqVector::iterator itTagClustered1;
  SeqVector::iterator itTagClustered2;
  itTagClustered1 = itTagLastSeed;
  while ( itTagSeed != itTagLastSeed ){
    LHCb::CaloCellID seedID = (*itTagSeed)->cellID();
    LHCb::CaloCluster* cluster = new  LHCb::CaloCluster();
    const LHCb::CaloDigit* digit = (*itTagSeed)->digit() ;
    cluster->entries().push_back( LHCb::CaloClusterEntry( digit , seed ) );
    cluster->setSeed( digit->cellID() );
    
    itTagClustered2 = std::stable_partition( itTagClustered1                                        ,
                                             itTagLastClustered                                     ,
                                             TaggedCellFunctor::isWithSeed( (*itTagSeed)->cellID()) ) ;
    // Owned cells
    for (  ; itTagClustered1 != itTagClustered2 ; ++itTagClustered1 ){
      LHCb::CaloCellID  cellID = (*itTagClustered1)->cellID();
      const LHCb::CaloDigit* digit = (*itTagClustered1)->digit() ;
      LHCb::CaloDigitStatus::Status owned  = LHCb::CaloDigitStatus::OwnedCell ;
      if( m_cellSelector( seedID, cellID) > 0.)owned |= used;
      cluster->entries().push_back( LHCb::CaloClusterEntry( digit , owned ) );
    }
    // Shared cells 
    SeqVector::iterator itTagFirstEdge = itTagLastClustered ;
    SeqVector::iterator itTagLastEdge  = std::stable_partition( itTagLastClustered                                       ,
                                                                taggedCellsSeq.end()                                     ,
                                                                TaggedCellFunctor::isWithSeed ( (*itTagSeed)->cellID() ) ) ;
    for(  ; itTagFirstEdge != itTagLastEdge ; ++itTagFirstEdge  ){
      const LHCb::CaloDigit* digit = (*itTagFirstEdge)->digit() ;
      LHCb::CaloCellID  cellID = (*itTagFirstEdge)->cellID();
      LHCb::CaloDigitStatus::Status shared = LHCb::CaloDigitStatus::SharedCell;
      if( m_cellSelector( seedID, cellID) > 0.)shared |= used;     
      cluster->entries().push_back( LHCb::CaloClusterEntry( digit , shared ) );
    };

    setEXYCluster ( cluster, m_detector );
    
    cluster->setType( LHCb::CaloCluster::CellularAutomaton ) ;
    cluster->position().setZ( zPosition( cluster )  );
    
    //  put cluster to the output
    clusters.push_back( cluster );
  
    itTagClustered1 = itTagClustered2;
    itTagSeed++;
  }
  
  // clear local storages 
  taggedCellsSeq    .clear () ;
  taggedCellsDirect .clear () ;
  local_cells       .clear () ;



  return StatusCode::SUCCESS;
}



//=============================================================================
// Destructor
//=============================================================================
CaloClusterizationTool::~CaloClusterizationTool() {} 

//=============================================================================
