#include <string>
#include <vector>

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IMagneticFieldSvc.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/IGiGaSvc.h"
#include "GaudiKernel/IGiGaSetUpSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IMagneticFieldSvc.h"
#include "GaudiKernel/IObjManager.h"
#include "GaudiKernel/ICnvManager.h"
#include "GaudiKernel/ICnvFactory.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataDirectory.h"

#include "GaudiKernel/System.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/RegistryEntry.h"

/// from GiGa 
#include "GiGa/GiGaCnvSvc.h"

///
GiGaCnvSvc::GiGaCnvSvc( const std::string&   ServiceName       , 
			ISvcLocator*         ServiceLocator    ,
			const unsigned int   StorageType       )
  : ConversionSvc(                           ServiceName       , 
				             ServiceLocator    , 
				             StorageType       )  
  ///
  , m_leaves      (                       )
  ///
  , m_dpName      ( "UndefinedName"       )
  , m_dpSvc       (     0                 )
  /// 
  , m_evtName     ( "EventDataSvc"        )
  , m_evtSvc      (     0                 )
  ///
  , m_detName     ( "DetectorDataSvc"     )
  , m_detSvc      (     0                 )
  ///
  , m_gigaName    ( "GiGaSvc"             ) 
  , m_gigaSvc     (     0                 ) 
  ///
  , m_setupName   ( "GiGaSvc"             ) 
  , m_setupSvc    (     0                 ) 
  ///
  , m_ppName      ( "ParticlePropertySvc" )  
  , m_ppSvc       (     0                 ) 
  ///
  , m_chronoName  ( "ChronoStatSvc"       )  
  , m_chronoSvc   (     0                 ) 
  ///
  , m_mfName      ( "MagneticFieldSvc"    )  
  , m_mfSvc       (     0                 ) 
  ///
  , m_omName      ( "ApplicationMgr"      )  
  , m_objMgr      (     0                 ) 
  ///
  , m_inName      ( "ApplicationMgr"      )  
  , m_incSvc      (     0                 ) 
  ///
{ 
  ///
  declareProperty   ( "EventDataProviderService"        , m_evtName     );
  declareProperty   ( "DetectorDataProviderService"     , m_detName     );
  declareProperty   ( "GiGaService"                     , m_gigaName    ); 
  declareProperty   ( "GiGaSetUpService"                , m_setupName   ); 
  declareProperty   ( "ParticlePropertyService"         , m_ppName      );
  declareProperty   ( "MagneticFieldService"            , m_mfName      );
  declareProperty   ( "ChronoStatService"               , m_chronoName  );
  declareProperty   ( "ObjectManager"                   , m_omName      );
  declareProperty   ( "IncidentService"                 , m_inName      );
  ///
};
///
GiGaCnvSvc::~GiGaCnvSvc(){}; 
///
StatusCode GiGaCnvSvc::initialize()
{
  ///
  StatusCode st = ConversionSvc::initialize() ; 
  if( st.isFailure()     )  { return Error("Initialize::Could not initialize base class ConversionSvc", st); } 
  ///
  if( !m_dpName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_dpName , m_dpSvc );
      if( status.isFailure() ) { return Error("Initialize::Could not locate IDataProvider="+m_dpName, status );}      
      if( 0 == dpSvc()       ) { return Error("Initialize::Could not locate IDataProvider="+m_dpName         );}
      dpSvc()->addRef(); 
      setStore( dpSvc() ); 
      Print( " Located DataProvider="+m_dpName, MSG::VERBOSE ); 
    } 
  else { return Error(" IDataProvider is not requested to be located!") ;} 
  ///
  if( !m_gigaName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_gigaName , m_gigaSvc ) ;
      if( status.isFailure() ) { return Error("Initialize::Could not locate IGiGaSvc="+m_gigaName, status );}      
      if( 0 == gigaSvc()     ) { return Error("Initialize::Could not locate IGiGaSvc="+m_gigaName         );}
      gigaSvc()->addRef(); 
      Print( " Located GiGa Service="+m_gigaName, MSG::VERBOSE ); 
    } 
  else { return Error(" IGiGaSvc is not requested to be located!") ;} 
  ///
  if( !m_setupName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_setupName , m_setupSvc ) ;
      if( status.isFailure() ) { return Error("Initialize::Could not locate IGiGaSetUpSvc="+m_setupName, status );}      
      if( 0 == setupSvc()    ) { return Error("Initialize::Could not locate IGiGaSetUpSvc="+m_setupName         );}
      setupSvc()->addRef(); 
      Print( " Located GiGa SetUp Service="+m_setupName, MSG::VERBOSE ); 
    } 
  else { return Error(" IGiGaSetUpSvc is not requested to be located!") ;} 
  ///
  if( !m_evtName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_evtName , m_evtSvc ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IDataProvider="+m_evtName, status );}      
      if( 0 == evtSvc()      ) { return Error("Initialize::Could not locate IDataProvider="+m_evtName         );}
      evtSvc()->addRef(); 
      Print( " Located Event Data  Service="+m_evtName, MSG::VERBOSE ); 
    } 
  else { Warning(" Event Data Service is not requested to be located!") ;} 
  ///
  if( !m_detName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_detName , m_detSvc ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IDataProvider="+m_detName, status );}      
      if( 0 == detSvc()      ) { return Error("Initialize::Could not locate IDataProvider="+m_detName         );}
      detSvc()->addRef(); 
      Print( " Located Detector Data  Service="+m_detName, MSG::VERBOSE ); 
    } 
  else { Warning(" Detector Data Service is not requested to be located!") ;} 
  ///
  if( !m_ppName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_ppName , m_ppSvc  ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IParticlePropertySvc="+m_ppName, status );}      
      if( 0 == ppSvc()       ) { return Error("Initialize::Could not locate IParticlePropertySvc="+m_ppName         );}
      ppSvc()->addRef(); 
      Print( " Located Particle Properties  Service="+m_ppName, MSG::VERBOSE ); 
    } 
  else { Warning(" Particle Property Service is not requested to be located!") ;} 
  ///
  if( !m_mfName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_mfName , m_mfSvc ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IMagneticFieldSvc="+m_mfName, status );}      
      if( 0 == mfSvc()       ) { return Error("Initialize::Could not locate IMagneticFieldSvc="+m_mfName         );}
      mfSvc()->addRef(); 
      Print( " Located Magnetic Field Service="+m_mfName, MSG::VERBOSE ); 
    } 
  else { Warning(" Magnetic Field Service is not requested to be located!") ;} 
  ///
  if( !m_chronoName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_chronoName , m_chronoSvc  ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IChronoStatSvc="+m_chronoName, status );}      
      if( 0 == chronoSvc()   ) { return Error("Initialize::Could not locate IChronoStatSvc="+m_chronoName         );}
      chronoSvc()->addRef(); 
      Print( " Located Chrono & Stat Service="+m_chronoName, MSG::VERBOSE ); 
    } 
  else { Warning(" Chrono & Stat Service is not requested to be located!") ;} 
  ///
  if( !m_omName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_omName , m_objMgr  ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IObjManager="+m_omName, status );}      
      if( 0 == objMgr()      ) { return Error("Initialize::Could not locate IObjManager="+m_omName         );}
      objMgr()->addRef(); 
      Print( " Located ObjectManager "+m_omName, MSG::VERBOSE ); 
    } 
  else { Warning(" Object Manager is not requested to be located!") ;} 
  ///
  if( !m_inName.empty() ) 
    {
      StatusCode status = serviceLocator()->service( m_inName , m_incSvc  ) ; 
      if( status.isFailure() ) { return Error("Initialize::Could not locate IIncidentSvc="+m_inName, status );}      
      if( 0 == objMgr()      ) { return Error("Initialize::Could not locate IIncidentSvc="+m_inName         );}
      incSvc()->addRef(); 
      Print( " Located Incident Service  "+m_inName, MSG::VERBOSE ); 
      incSvc()->addListener( this , "BeginOfEvent" );
    } 
  else { return Error(" Incident Service is not requested to be located!") ;} 
  ///
  ///
  /// here we need to locate all converter factories and converters 
  for( ICnvManager::CnvIterator it = cnvManager()->cnvBegin() ; cnvManager()->cnvEnd() != it ; ++it )
    {
      if( 0 == *it || ( repSvcType() != (*it)->repSvcType()  ) ) { continue ; }
      StatusCode st = addConverter( **it ); 
      if( st.isFailure() ) { return Error("Initialize::Could not add converter="+(*it)->typeName() , st ) ; }
    } 
  ///
  return StatusCode::SUCCESS; 
  ///
};
///
StatusCode GiGaCnvSvc::finalize()
{
  ///
  if ( 0 != dpSvc     () ) { dpSvc     ()->release() ; m_dpSvc     = 0 ; } 
  if ( 0 != evtSvc    () ) { evtSvc    ()->release() ; m_evtSvc    = 0 ; } 
  if ( 0 != detSvc    () ) { detSvc    ()->release() ; m_detSvc    = 0 ; } 
  if ( 0 != gigaSvc   () ) { gigaSvc   ()->release() ; m_gigaSvc   = 0 ; } 
  if ( 0 != setupSvc  () ) { setupSvc  ()->release() ; m_setupSvc  = 0 ; } 
  if ( 0 != ppSvc     () ) { ppSvc     ()->release() ; m_ppSvc     = 0 ; } 
  if ( 0 != mfSvc     () ) { mfSvc     ()->release() ; m_mfSvc     = 0 ; } 
  if ( 0 != chronoSvc () ) { chronoSvc ()->release() ; m_chronoSvc = 0 ; } 
  if ( 0 != objMgr    () ) { objMgr    ()->release() ; m_objMgr    = 0 ; } 
  ///
  m_leaves.clear();
  ///
  StatusCode st = ConversionSvc::finalize(); 
  ///
  return st; 
  ///
};
///
StatusCode GiGaCnvSvc::queryInterface( const IID& iid , void** ppI )
{ 
  if( 0 == ppI                ) { return StatusCode::FAILURE                        ; } 
  *ppI = 0 ;
  if( IGiGaCnvSvc::interfaceID()        == iid ) { *ppI = static_cast<IGiGaCnvSvc*>       (this)     ; } 
  if( IIncidentListener::interfaceID()  == iid ) { *ppI = static_cast<IIncidentListener*> (this)     ; } 
  else                                           { return ConversionSvc::queryInterface( iid , ppI ) ; } 
  addRef();
  return StatusCode::SUCCESS;  
};
///
StatusCode GiGaCnvSvc::declareObject( const GiGaCnvSvc::Leaf& leaf )
{
  m_leaves.push_back( leaf );
  return StatusCode::SUCCESS ;
};
/// 
StatusCode GiGaCnvSvc::declareObject( const std::string & Path  ,
                                      const CLID        & Clid  ,
                                      const std::string & Addr1 ,
                                      const std::string & Addr2 )
{ return declareObject( Leaf( Path ,  Clid , Addr1 , Addr2 ) );  };
///
void       GiGaCnvSvc::handle         ( const Incident& inc )
{
  ///
  if( inc.type() != "BeginOfEvent" ) 
    { Warning("Unknown Incident is handled="+inc.type() ); return ; }
  ///
  DataObject* root = 0 ;  
  StatusCode sc = dpSvc()->findObject( "/Event" , root) ;
  if( sc.isFailure() || 0 == root ) 
    { Error("Could not locate root object '/Event' !") ; return ; }
  ///
  IDataDirectory* dir = root->directory(); 
  RegistryEntry*  ent = dynamic_cast<RegistryEntry*> (dir); 
  if( 0 == ent ) { Error("Could not cast to RegistryEnytry*! (1)") ; return ; }
  ///
  RegistryEntry* gtop = ent->findLeaf("/G4"); 
  if( 0 == gtop ) 
    {
      DataObject* tmp = new DataObject();
      sc = dpSvc()->registerObject( root , "/G4" , tmp ); 
      if( sc.isFailure() ) 
	{ Error("Could not create '/Event/G4' directrory !") ; return ; }
      gtop = dynamic_cast<RegistryEntry*> ( tmp->directory() ) ;
    } 
  if( 0 == gtop ) { Error("Could not cast to RegistryEnytry*! (2)") ; return ; }
  ///
  for( Leaves::iterator it = m_leaves.begin() ; m_leaves.end() != it ; ++it ) 
    {
      std::string::size_type pos = it->path().find("/Event/G4/");
      if( std::string::npos == pos ) { Error("Wrong address ="+it->path() ); return ; }
      std::string name( it->path() , pos , pos + 10  );
      IOpaqueAddress* Address = 0 ; 
      sc = createAddress( repSvcType() , it->clid() , it->addr1() , it->addr2() , -1 , Address );   
      if( sc.isFailure() || 0 == Address ) 
	{ Error("Could not create IOpaqueAddress for "+it->path()+" name="+name ); return ; }
      gtop->add( name , Address );
    }
  ///  
};
///








