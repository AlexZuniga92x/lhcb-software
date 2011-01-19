// $Id: Calo2Calo.cpp,v 1.9 2009-09-16 16:07:06 odescham Exp $
// ============================================================================
// Include files 
// ============================================================================
// from Gaudi
// ============================================================================
#include "GaudiKernel/ToolFactory.h" 
// ============================================================================
#include "Kernel/CaloCellCode.h"
// ============================================================================
// CaloUtils
// ============================================================================
#include "CaloUtils/CaloDataFunctor.h"
#include "CaloUtils/CaloAlgUtils.h"
// ============================================================================
// local
// ============================================================================
#include "Calo2Calo.h"
// ============================================================================
/** @file 
 *  Implementation file for class : Calo2Calo
 *  @date 2007-05-29 
 *  @author Olivier Deschamps
 */
// ============================================================================
// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( Calo2Calo )
// ============================================================================


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
Calo2Calo::Calo2Calo( const std::string& type,
                      const std::string& name,
                      const IInterface* parent )
  : GaudiTool ( type, name , parent )
    ,m_fromCalo("??")
    ,m_toCalo("??")
    ,m_cells()
    ,m_digits()
    ,m_energy()
    ,m_count(0)
{
  declareInterface<ICalo2Calo>(this);
  declareProperty("IdealGeometry", m_geo = true );
  declareProperty("FromCalo"     , m_fromCalo);
  declareProperty("ToCalo"       , m_toCalo );
  declareProperty("GetterName"   , m_getterName = "CaloGetter" );

}
//=============================================================================
// Destructor
//=============================================================================
Calo2Calo::~Calo2Calo() {}
//=============================================================================
StatusCode Calo2Calo::initialize()
{
  StatusCode sc = GaudiTool::initialize();
  debug() << "Initialize Calo2Calo tool " << endmsg;
  
  // get getter tool
  m_getter = tool<ICaloGetterTool>("CaloGetterTool",m_getterName,this);

  // CaloDigit locations
  m_loc["Ecal"]=  LHCb::CaloAlgUtils::CaloDigitLocation( "Ecal" , context() );
  m_loc["Hcal"]=  LHCb::CaloAlgUtils::CaloDigitLocation( "Hcal" , context() );
  m_loc["Prs"] =  LHCb::CaloAlgUtils::CaloDigitLocation( "Prs"  , context() );
  m_loc["Spd"] =  LHCb::CaloAlgUtils::CaloDigitLocation( "Spd"  , context() );
  // DeCalorimeter* pointers
  m_det["Ecal"]=getDet<DeCalorimeter>( DeCalorimeterLocation::Ecal );
  m_det["Hcal"]=getDet<DeCalorimeter>( DeCalorimeterLocation::Hcal );
  m_det["Prs"]=getDet<DeCalorimeter>( DeCalorimeterLocation::Prs   );
  m_det["Spd"]=getDet<DeCalorimeter>( DeCalorimeterLocation::Spd   );
  // CellSize reference (outer section)  Warning : factor 2 for Hcal
  m_refSize["Hcal"] = (*(m_det["Hcal"]->cellParams().begin())).size()/2.;
  m_refSize["Ecal"] = (*(m_det["Ecal"]->cellParams().begin())).size(); 
  m_refSize["Prs"]  = (*(m_det["Prs"]->cellParams().begin())).size(); 
  m_refSize["Spd"]  = (*(m_det["Spd"]->cellParams().begin())).size(); 
  // CaloPlanes
  m_plane["Hcal"] = m_det["Hcal"]->plane(CaloPlane::ShowerMax );
  m_plane["Ecal"] = m_det["Ecal"]->plane(CaloPlane::ShowerMax );
  m_plane["Prs"]  = m_det["Prs"]->plane(CaloPlane::ShowerMax );
  m_plane["Spd"]  = m_det["Spd"]->plane(CaloPlane::ShowerMax );

  reset();
  if((m_fromCalo == "Hcal" || m_fromCalo == "Ecal" || m_fromCalo == "Prs" || m_fromCalo == "Spd" ) &&  
     (m_toCalo   == "Hcal" || m_toCalo   == "Ecal" || m_toCalo   == "Prs" || m_toCalo   == "Spd" ) )setCalos(m_fromCalo,m_toCalo);
  return sc;
}

void Calo2Calo::reset(){
  m_digits.clear();
  m_cells.clear();
  m_energy = 0;
  m_count  = 0;
}

void Calo2Calo::setCalos
( const std::string& fromCalo ,
  const std::string& toCalo   )
{ 
  m_fromCalo = fromCalo;
  m_toCalo   = toCalo;
  m_fromDet  = m_det[m_fromCalo];
  m_fromSize = m_refSize[m_fromCalo];
  m_toDet    = m_det[m_toCalo];
  m_toPlane  = m_plane[m_toCalo];
  m_toSize   = m_refSize[m_toCalo];
  m_toLoc    = m_loc[ m_toCalo ];
 }
//=======================================================================================================
const std::vector<LHCb::CaloCellID>& 
Calo2Calo::cellIDs ( const LHCb::CaloCluster& fromCluster , 
                     const std::string&       toCalo      )
{
  
  reset();
  LHCb::CaloCellID seedID = fromCluster.seed();
  std::string fromCalo = CaloCellCode::CaloNameFromNum( seedID.calo() );
  if( toCalo != m_toCalo || fromCalo != m_fromCalo)setCalos(fromCalo,toCalo);
  
  for(LHCb::CaloCluster::Entries::const_iterator ent = fromCluster.entries().begin();
      ent != fromCluster.entries().end(); ++ent)
  {
    const LHCb::CaloDigit* digit = ent->digit();
    if ( 0 == digit ) { continue ; }
    cellIDs( digit->cellID() , toCalo, false );
  }
  return m_cells;
}
//=======================================================================================================
const std::vector<LHCb::CaloCellID>& Calo2Calo::addCell
( const LHCb::CaloCellID& id     , 
  const std::string&      toCalo )
{
  // consistency check
  if( CaloCellCode::CaloNameFromNum(id.calo()) != m_toCalo || toCalo != m_toCalo ){
    Warning("CellID is not consistent with Calo setting").ignore();
    return m_cells;
  }
  // check duplicate
  if ( m_cells.end() != std::find ( m_cells.begin() , m_cells.end() , id ) ) { return m_cells ; }
  
  // add the cells
  m_cells.push_back( id );
  
  // added by VB.
  if ( 0 == m_digs ) 
  {
    Error ( "Digits* points to NULL") ;
    return m_cells ;
  }

  LHCb::CaloDigit* digit = m_digs->object( id );
  if( NULL != digit ) {
    m_digits.push_back( digit );
    m_energy += digit->e();
    m_count++;
  }
  return m_cells;
}


//=======================================================================================================
const std::vector<LHCb::CaloCellID>& 
Calo2Calo::cellIDs ( const LHCb::CaloCellID& fromId , 
                     const std::string&      toCalo , bool init )
{
  
  if( init )reset();
  std::string fromCalo = CaloCellCode::CaloNameFromNum( fromId.calo() );
  if( toCalo != m_toCalo || fromCalo != m_fromCalo)setCalos(fromCalo,toCalo);

  LHCb::CaloCellID toId = fromId;
  // ---- Assume ideal geometry : trivial mapping for detectors having the same granularity (Prs/Spd/Ecal)
  if( ( m_geo && (m_fromCalo == "Ecal" || m_fromCalo == "Prs" || m_fromCalo == "Spd") 
        && (m_toCalo == "Ecal" || m_toCalo == "Prs" || m_toCalo == "Spd") ) 
      || m_fromCalo == m_toCalo ){
    toId.setCalo( CaloCellCode::CaloNumFromName( m_toCalo ));
    debug() << "Add cell from trivial mapping" << endmsg;
    return addCell( toId , m_toCalo);
  }
  
  // ---- Else use the actual geometry to connet detectors
  double scale = 1.;
  Gaudi::XYZPoint  center = m_fromDet->cellCenter( fromId );
  if( m_fromCalo != m_toCalo ){
    // z-scaling
    scale = m_toSize / m_fromSize ;
    center  = m_toPlane.ProjectOntoPlane( m_fromDet->cellCenter( fromId )*scale );
    // connect
    toId  = m_toDet->Cell( center );
  }
  double fromSize = m_fromDet->cellSize( fromId )*scale;
  //cell-center is outside 'toCalo' - check corners  
  if( LHCb::CaloCellID() == toId){
    for( int i = 0 ; i != 2; ++i){
      for( int j = 0 ; j != 2; ++j){
        double x = m_fromDet->cellCenter( fromId ).X() + (i*2-1) * fromSize;        
        double y = m_fromDet->cellCenter( fromId ).Y() + (j*2-1) * fromSize;
        Gaudi::XYZPoint  corner = Gaudi::XYZPoint(x,y,center.Z());
        LHCb::CaloCellID cornerId  = m_toDet->Cell( corner );
        if( LHCb::CaloCellID() == cornerId)continue;
        toId=cornerId;
      }   
    }
  }
  if( LHCb::CaloCellID() == toId)return m_cells;
  int pad = 1;
  double x0  = center.X();
  double y0  = center.Y();
  if( m_fromDet != m_toDet ){
    double toSize   = m_toDet->cellSize( toId ) ;
    pad =  (int) floor( fromSize  / toSize + 0.25); // warning int precision
    if(pad < 1)pad=1;
    x0 = center.X() - (pad-1)*fromSize/2./pad;
    y0 = center.Y() - (pad-1)*fromSize/2./pad;  
  }
  for( int i = 0 ; i != pad; ++i){
    for( int j = 0 ; j != pad; ++j){
      double x = x0 + i * fromSize/pad;
      double y = y0 + j * fromSize/pad;    
      Gaudi::XYZPoint  pos(x,y,center.Z());
      if( m_fromDet != m_toDet ) toId  = m_toDet->Cell( pos ) ;
      if( LHCb::CaloCellID() == toId)continue;
      addCell( toId, m_toCalo);
    }        
  }
  return m_cells;
}


// Digits
const std::vector<LHCb::CaloDigit*>& Calo2Calo::digits
( const LHCb::CaloCellID& fromId ,
  const std::string& toCalo )
{  
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs( fromId, toCalo ) ;
  return m_digits;
}
const std::vector<LHCb::CaloDigit*>& Calo2Calo::digits
( const LHCb::CaloCluster& fromCluster , 
  const std::string& toCalo ) 
{  
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs( fromCluster, toCalo);
  return m_digits;
}  
// Energy
double Calo2Calo::energy 
( const LHCb::CaloCellID& fromId , 
  const std::string&      toCalo )
{
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs ( fromId , toCalo );
  return m_energy;
}
int Calo2Calo::multiplicity
( const LHCb::CaloCellID& fromId , 
  const std::string&      toCalo )
{
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs(fromId, toCalo);
  return m_count;
}
double Calo2Calo::energy
( const LHCb::CaloCluster& fromCluster , 
  const std::string&  toCalo )
{
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs(fromCluster, toCalo);
  return m_energy;
}
int Calo2Calo::multiplicity
( const LHCb::CaloCluster& fromCluster , 
  const std::string& toCalo )
{
  m_digs     = m_getter->digits( m_toLoc );  
  cellIDs(fromCluster, toCalo);
  return m_count;
}

// ============================================================================
// Additional method : isLocalMax
bool Calo2Calo::isLocalMax ( const LHCb::CaloDigit& digit)
{
  const LHCb::CaloCellID& id = digit.cellID();
  std::string    calo = CaloCellCode::CaloNameFromNum( id.calo() );
  DeCalorimeter* det  = m_det[ calo ];
  const LHCb::CaloDigits* digits = m_getter->digits( m_loc[ calo ] ); 
  //
  return LHCb::CaloDataFunctor::isLocalMax ( &digit , det , digits ) ;
}
