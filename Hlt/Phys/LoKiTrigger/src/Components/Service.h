// $Id$
// ============================================================================
#ifndef LOKITRIGGER_SERVICE_H 
#define LOKITRIGGER_SERVICE_H 1 
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/VectorMap.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/StatEntity.h"
// ============================================================================
// HltBase 
// ============================================================================
#include "HltBase/HltSelection.h"
// ============================================================================
// Kernel
// ============================================================================
#include "Kernel/IANNSvc.h"
// ============================================================================
// LoKi 
// ============================================================================
#include "LoKi/ILoKiSvc.h"
#include "HltBase/IHltRegister.h"
#include "HltBase/IHltData.h"
#include "HltBase/IHltInspector.h"
// ============================================================================
// LoKi 
// ============================================================================
#include "LoKi/Interface.h"
// ============================================================================
/// forward declarations 
// ============================================================================
/// Incident Service 
class IIncidentSvc ;                                        // incident service 
/// "Assigned Numbers & Names" service 
class IANNSvc      ;                      // "Assigned Numbers & Names" service 
// ============================================================================
namespace Hlt 
{
  // ==========================================================================
  /** @class Service
   *  
   *
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2009-03-17
   */
  class Service 
    : public ::Service 
    , virtual public Hlt::IRegister 
    , virtual public Hlt::IData
    , virtual public Hlt::IInspector
    , virtual public IIncidentListener 
  {
    // ========================================================================
    /// the friend factory for instantiation 
    friend class SvcFactory<Hlt::Service> ;
    // ========================================================================
  public:
    // ========================================================================
    /// the actual type of the key 
    typedef Hlt::IData::Key Key ;                 // the actual type of the key 
    // ========================================================================
  public:  // Hlt::IRegister (public) 
    // ========================================================================    
    /** register the output selection during the allowed transaction for 
     *  locked service 
     *  @see Hlt::IRegister
     *  @param selection the selection to be registered 
     *  @param producer  the algorithm/producer
     *  @return status code 
     */
    StatusCode        registerOutput 
    ( Hlt::Selection* selection ,               // the output selection 
      const IAlgorithm*       producer  )  override;    //             producer 
    /** register the input selection  dirung the allower transactions for 
     *  locked service 
     *  @see Hlt::IRegister
     *  @param selection the selection to be registered 
     *  @param consumer the algorithm/consumer
     *  @return status code 
     */
    StatusCode           registerInput 
    ( const Hlt::IRegister::Key& selection ,             // the input selection 
      const IAlgorithm*          consumer  ) override;   //            consumer 
    // ========================================================================
    /** register the requiest to TES-selection 
     *  @attention the service much be locked properly during the transaction!
     *  @param location TES location to be registered
     *  @param consumer algorithm/consumer 
     *  @return Status Code 
     */
    StatusCode  registerTESInput
    ( const Key&                 location ,              //        TES location 
      const IAlgorithm*          consumer ) override ;   //            consumer   
    // ========================================================================
  protected: // Hlt::IRegister (public) 
    // ========================================================================
    /** start the transaction/lock the registrator 
     *  @see Hlt::IRegister
     *  @see Hlt::IRegister::lock 
     *  @see Hlt::IRegister::Lock 
     *  @param alg the algorithm which starts the transaction
     *  @param status code
     */
    StatusCode lock   ( const IAlgorithm* alg ) override;
    /** end the transaction/unlock the registrator 
     *  @see Hlt::IRegister
     *  @see Hlt::IRegister::unlock 
     *  @see Hlt::IRegister::Lock 
     *  @param alg the algorithm which ends the transaction
     *  @param status code
     */
    StatusCode unlock ( const IAlgorithm* alg ) override;  
    // ========================================================================
  public:  // Hlt::IData
    // ========================================================================
    /** existing selection ? 
     *  @see Hlt::IData 
     *  @see Hlt::IInspector
     */
    bool hasSelection ( const Key& key ) const override { return inMap ( key ) ; }
    /*  retrieve the selection by name  (conts-access by consumer)
     *  @see Hlt::IData 
     *  @param key the selection key/name 
     *  @param alg the consumer 
     *  @return pointer to the selection (0 for missing selections)
     */
    const Hlt::Selection* selection 
    ( const Hlt::IData::Key&      key          ,          // selection key/name
      const IAlgorithm*           consumer     ) const override;  //       the consumer
    // ========================================================================
    /** retrieve the selection by name  (non-const access, only by producer)
     *  @param alg the producer
     *  @param key the selection key/name 
     *  @return pointer to the selection (0 for missing selections)
     */
    Hlt::Selection*  retrieve
    ( const IAlgorithm*      producer ,                  //        the producer
      const Hlt::IData::Key& key      ) const override;  //  selection key/name
    // ========================================================================
  public: // TES locations
    // ========================================================================
    /** retrieve the object from TES 
     *  @param reader the algorithm
     *  @param location TES-location 
     *  @return object for TES 
     */
    const DataObject* tes 
    ( const IAlgorithm* reader   ,                              //       reader 
      const Key&        location ) const override;              // TES-location
    // ========================================================================
  public:   // Hlt::IInspector 
    // ========================================================================
    /** get the producer for the given selection 
     *  @param selection the selection 
     *  @return the producer 
     */
    const IAlgorithm*  producer 
    ( const Hlt::Selection*    selection ) const override;
    /** get the producer for the given selection 
     *  @param selection the selection 
     *  @return the producer 
     */
    const IAlgorithm* producer 
    ( const std::string&      selection ) const override;
    /** get all nominal consumers for the given selection 
     *  @param selection the selection 
     *  @param alglist (OUTPUT) list of consumers 
     *  @param number of nominal consumers 
     */
    size_t consumers 
    ( const Hlt::Selection* selection , 
      AlgList&              alglist   ) const  override ;  
    /** get all nominal consumers for the given selection 
     *  @param selection the selection 
     *  @param alglist (OUTPUT) list of consumers 
     *  @param number of nominal consumers 
     */
    size_t consumers 
    ( const std::string&    selection , 
      AlgList&              alglist   ) const override;
    // ========================================================================
  public:   // Hlt::Inspector: ispection by algorithms: inputs/outputs  
    // ========================================================================
    /** get the inputs for the given algorithm 
     *  @param algorithm the algorithm (consumer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of input selections 
     */
    size_t inputs 
    ( const IAlgorithm*  algorithm , 
      SelList&           selections ) const override;
    /** get the inputs for the given algorithm 
     *  @param algorithm the algorithm (consumer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of input selections 
     */
    size_t inputs 
    ( const IAlgorithm*  algorithm , 
      KeyList&           selections ) const override;
    /** get the inputs for the given algorithm 
     *  @param algorithm the algorithm (consumer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of input selections 
     */
    size_t inputs 
    ( const std::string& algorithm , 
      SelList&           selections ) const override;
    /** get the inputs for the given algorithm 
     *  @param algorithm the algorithm (consumer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of input selections 
     */
    size_t inputs 
    ( const std::string& algorithm , 
      KeyList&           selections ) const override;
    // ========================================================================
    /** get the outputs for the given algorithm 
     *  @param algorithm the algorithm (producer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of output selections 
     */
    size_t outputs 
    ( const IAlgorithm*  algorithm , 
      SelList&           selections ) const override;
    /** get the outputs for the given algorithm 
     *  @param algorithm the algorithm (producer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of output selections 
     */
    size_t outputs 
    ( const IAlgorithm*  algorithm , 
      KeyList&           selections ) const override;
    /** get the outputs for the given algorithm 
     *  @param algorithm the algorithm (producer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of output selections 
     */
    size_t outputs
    ( const std::string& algorithm , 
      SelList&           selections ) const override;
    /** get the outputs for the given algorithm 
     *  @param algorithm the algorithm (producer) 
     *  @param selections (OUTPUT) the list of input selections 
     *  @return number of output selections 
     */
    size_t outputs 
    ( const std::string& algorithm , 
      KeyList&           selections ) const override;
    // ========================================================================
    /// registered algorithm?
    bool hasAlgorithm ( const IAlgorithm*  alg ) const override;
    /// registered algorithm?
    bool hasAlgorithm ( const std::string& alg ) const override;
    /// registered producer?
    bool hasProducer  ( const IAlgorithm*  alg ) const override;
    /// registered producer?
    bool hasProducer  ( const std::string& alg ) const override;
    /// registered consumer
    bool hasConsumer  ( const IAlgorithm*  alg ) const override;
    /// registered producer?
    bool hasConsumer  ( const std::string& alg ) const override;
    /// registered selection ?
    bool hasSelection ( const Hlt::Selection* selection ) const override;
    // ========================================================================
    /// existing selection ? 
    bool hasSelection ( const std::string& key ) const override 
    { return inMap ( key ) ; }
    // ========================================================================
  public: // Get All
    // ========================================================================
    /** get all algorithms 
     *  @param algs (OUTPUT) the list of algorithms 
     *  @return number of algorithms 
     */
    size_t algorithms ( AlgList& algs ) const override;
    /** get all selections 
     *  @param keys (OUTPUT) the list of selections 
     *  @return number of selections  
     */
    size_t selections ( KeyList& keys ) const override;
    // ========================================================================
  public:  // TES selections  
    // ========================================================================
    /** get the "readers" for the given TES-location
     *  @param location TES-location
     *  @param alglist (OUTPUT) the list of readers 
     *  @return number of readers 
     */
    size_t readers 
    ( const std::string&    location  , 
      AlgList&              alglist   ) const override;
    /** get all readers 
     *  @param alglist (OUTPUT) the list of readers 
     *  @return number of readers 
     */
    size_t allReaders 
    ( AlgList&              alglist   ) const override;
    /** get the input TES locations for the given reader 
     *  @param reader the algorithm
     *  @param locations (OUTPUT) the list of input TES locations 
     *  @return number of locations 
     */
    size_t readTES 
    ( const IAlgorithm*     reader    , 
      KeyList&              locations ) const override;
    /** get the input TES locations for the given reader 
     *  @param reader the algorithm
     *  @param locations (OUTPUT) the list of input TES locations 
     *  @return numebr of locations 
     */
    size_t readTES 
    ( const std::string&    reader    , 
      KeyList&              locations ) const override;
    /** get all TES-inptu locations 
     *  @param locations (OUTPUT) the list of input TES locations 
     *  @return number of locations 
     */
    size_t allTES 
    ( KeyList&              locations ) const override;
    // ========================================================================
  public: // IIncidentListener
    // ========================================================================
    /** handle the incident
     *  @see IIncidentListener 
     *  @param inc incident to be handled 
     */
    void handle ( const Incident& inc ) override;
    // ========================================================================
  public: // IService/IInterface etc...
    // ========================================================================
    /** query the interface 
     *  @see IInterface 
     *  @param iid the uniqye interface ID 
     *  @param ppi (OUTPUT) the interface 
     *  @return status code 
     */
    StatusCode queryInterface
    (  const InterfaceID& iid ,
       void**             ppi ) override;
    /** standard initialization 
     *  @see IService 
     *  @return status code 
     */
    StatusCode initialize () override;
    /** standard finalization
     *  @see IService 
     *  @return status code 
     */
    StatusCode finalize   () override;
    // ========================================================================
  protected:
    // ========================================================================
    /// produce the error message 
    StatusCode Error 
    ( const std::string& message , 
      const StatusCode&  sc      = 
      StatusCode ( StatusCode::FAILURE , true ) ) const ;
    // ========================================================================
    /// produce the warning message 
    StatusCode Warning
    ( const std::string& message , 
      const StatusCode&  sc      = 
      StatusCode ( StatusCode::FAILURE , true ) ) const ;
    /// assertion 
    inline void Assert 
    ( const bool         ok      , 
      const std::string& message , 
      const StatusCode&  sc      = 
      StatusCode ( StatusCode::FAILURE , true ) ) const 
    { if  ( !ok ) { Exception ( message , sc ) ; } }
    /// assertion 
    inline void Assert 
    ( const bool         ok      , 
      const char*        message , 
      const StatusCode&  sc      = 
      StatusCode ( StatusCode::FAILURE , true ) ) const 
    { if  ( !ok ) { Exception ( message , sc ) ; } }
    /// thow the exception 
    void Exception 
    ( const std::string& message , 
      const StatusCode&  sc      = 
      StatusCode ( StatusCode::FAILURE , true ) ) const ;
    // ========================================================================
  protected:
    // ========================================================================
    /** Standard constructor (protected) 
     *  @param name the service instance name 
     *  @param pSvc service locator 
     */
    Service 
    ( const std::string& name ,                        // service instance name 
      ISvcLocator*       pSvc ) ;                      //       Service Locator 
    /// virtual & protected destructor 
    ~Service() override = default;             // virtual & protected destructor
    // ========================================================================
  private:
    // ========================================================================
    /// the default constructor is disabled 
    Service() ;                          // the default constructor is disabled 
    /// copy constructor is disabled 
    Service ( const Service& ) ;         //       copy constriuctor is disabled 
    /// assignement operator is disabled 
    Service& operator=( const Service& ) ; //           assignement is disabled 
    // ========================================================================
  protected: // to be removed to some base class 
    // ========================================================================
    /// Predefined configurable message stream for the efficient printouts
    inline MsgStream& msgStream ( const MSG::Level level ) const 
    {
      if ( 0 == m_msgStream.get() ) 
      { m_msgStream.reset ( new MsgStream( msgSvc(), name() ) ); }
      return (*m_msgStream) << level;
    }
    /// shortcut for the method msgStream ( MSG::ALWAYS )
    inline MsgStream&  always () const { return msgStream ( MSG::  ALWAYS ) ; }
    /// shortcut for the method msgStream ( MSG::FATAL   )
    inline MsgStream&   fatal () const { return msgStream ( MSG::   FATAL ) ; }
    /// shortcut for the method msgStream ( MSG::ERROR   )
    inline MsgStream&     err () const { return msgStream ( MSG::   ERROR ) ; }
    /// shortcut for the method msgStream ( MSG::ERROR   )
    inline MsgStream&   error () const { return msgStream ( MSG::   ERROR ) ; }
    /// shortcut for the method msgStream ( MSG::WARNING )
    inline MsgStream& warning () const { return msgStream ( MSG:: WARNING ) ; }
    /// shortcut for the method msgStream ( MSG::INFO    )
    inline MsgStream&    info () const { return msgStream ( MSG::    INFO ) ; }
    /// shortcut for the method msgStream ( MSG::DEBUG   )
    inline MsgStream&   debug () const { return msgStream ( MSG::   DEBUG ) ; }
    /// shortcut for the method msgStream ( MSG::VERBOSE )
    inline MsgStream& verbose () const { return msgStream ( MSG:: VERBOSE ) ; }
    /// shortcut for the method msgStream ( MSG::INFO    )
    inline MsgStream&     msg () const { return msgStream ( MSG::    INFO ) ; }
    // ========================================================================
    inline bool msgLevel( const MSG::Level level ) const 
    { return outputLevel() <= level ; }
    // ========================================================================
  private:
    // ========================================================================
    /// the message stream 
    mutable std::auto_ptr<MsgStream>     m_msgStream ; // message stream 
    // ========================================================================
  protected:
    // ============================================================================
    /// Print statistics 
    std::size_t printStat         ( const MSG::Level level  ) const ;
    /// handler for 'StatPrint'-proeprty 
    void        printStatHandler  ( Property& /* theProp */ )       ;
   // ========================================================================
  private:
    // ========================================================================
    /// frozen: all transactions are frozen
    bool  m_frozen ;                   // frozen: all transactions are disabled 
    // ========================================================================
  private:
    // ========================================================================
    typedef const IAlgorithm*            Alg    ;
    typedef LoKi::Interface<IAlgorithm>  Locker ;
    typedef std::vector<Locker>          Locked ;
    // ========================================================================
    /// Locker: the algorithm which currently makes transaction,IRegister
    /// Locker m_locker ;                  //                    the current locker 
    Locked m_lockers ;                 //                    the current locker 
    /// the list of already locked algorithms 
    Locked m_locked  ;                  // the list of already locked algorithms 
    // ========================================================================
  private:
    // ========================================================================
    typedef GaudiUtils::VectorMap<Key,Hlt::Selection*> SelMap ;
    /// global map: id -> selection 
    SelMap   m_selections ;                      // global map: id -> selection
    // ========================================================================
  protected:
    // ========================================================================
    // check the presence of selection ID in the map
    bool inMap ( const Key&            key       ) const
    { return m_selections.end() != m_selections.find ( key ) ; }
    // check the presence of selection in the map
    bool inMap ( const Hlt::Selection* selection ) const
    { 
      return std::any_of( std::begin(m_selections), std::end(m_selections), 
                          [&](SelMap::const_reference item ) { return item.second == selection; } );
    }
    // ========================================================================
  private:
    // ========================================================================
    /// the actual a type of structure for output selections 
    typedef GaudiUtils::VectorMap<Alg,SelMap>  OutputMap ; // output selections
    /// the actul a type of structure for input  selections 
    typedef GaudiUtils::VectorMap<Alg,SelMap>  InputMap  ; //  input selections 
    /// The map: algorithm --> output selections 
    OutputMap  m_outputs ;          // The map: algorithm --> output selections 
    /// The map: algorithm --> input selections 
    InputMap   m_inputs  ;          // The map: algorithm -->  input selections
    // ========================================================================
  private: // TES-input selections/locations 
    // ========================================================================
    /// the list of TES-input locations 
    typedef std::set<std::string>  TESLocs ; // the list of TES-input locations 
    /// the actual type of structure for TES-locations 
    typedef GaudiUtils::VectorMap<Alg,TESLocs>     TESMap ; 
    /// the map: algorithm -> input TES locations 
    TESMap   m_tesmap ;            // the map: algorithm -> input TES locations
    // ========================================================================
  protected : // Helpers 
    // ========================================================================
    /// incident service 
    IIncidentSvc*   incSvc  () const ;                     // incindent service 
    /// "Assigned Numbers & Names" service
    IANNSvc*        annSvc  () const ;    // "Assigned Numbers & Names" service
    /// the major LoKi service
    LoKi::ILoKiSvc* lokiSvc () const ;                // the major LoKi Service 
    // ========================================================================    
  private: // Helpers 
    // ========================================================================
    /// incindent service 
    mutable LoKi::Interface<IIncidentSvc> m_incSvc ;        // incident service 
    /// "Assigned Numbers & Names" service
    mutable LoKi::Interface<IANNSvc>      m_annSvc ;// Assigned Numbers & Names
    /// the major LoKi Service 
    mutable LoKi::Interface<LoKi::ILoKiSvc> m_lokiSvc ;         // LoKi Service 
    // ========================================================================
  private: // Properties
    // ========================================================================
    /// produce pedantic warning messages 
    bool m_pedantic  ;                     // produce pedantic warning messages 
    /// allow anonymous access to the selections 
    bool m_anonymous ;              // allow anynymous access to the selections 
    // spy mode? 
    bool m_spy       ;                                             // spy mode?
    // ========================================================================
  private: // counters 
    // ========================================================================
    typedef std::pair<StatEntity,StatEntity>                CntPair ;
    typedef GaudiUtils::HashMap<Gaudi::StringKey,CntPair>   CntMap  ;
    /// the actual map of counters:  
    CntMap m_cntMap    ;
    /// the proper treatment of the strange order of incidents in Gaudi 
    bool   m_cntEvent  ;
    /// print statistics at the end?
    bool   m_statPrint ;
    // ========================================================================
  }; //                                                        end of the class 
  // ==========================================================================
} //                                                      end  of namespace Hlt
// ============================================================================
//                                                                      The END 
// ============================================================================
#endif // LOKITRIGGER_SERVICE_H 
// ============================================================================
