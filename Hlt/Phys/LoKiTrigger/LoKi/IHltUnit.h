// $Id: IHltUnit.h,v 1.5 2009-07-12 15:59:10 ibelyaev Exp $
// ============================================================================
#ifndef LOKI_IHLTUNIT_H 
#define LOKI_IHLTUNIT_H 1
// ============================================================================
// Include files
// ============================================================================
// STD & STL 
// ============================================================================
// #include <vector>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgorithm.h"
// ============================================================================
// HltBase 
// ============================================================================
#include "HltBase/stringKey.h"
#include "HltBase/HltSelection.h"
// ============================================================================
class DataObject ;
// ============================================================================
namespace Hlt 
{
  class IRegister  ;
  class IData      ; 
}
// ============================================================================
namespace LoKi
{
  // =========================================================================
  // the base for all functors 
  class AuxFunBase ; // the base for all functors 
  // ==========================================================================
  /** @class IHltUnit LoKi/IHltUnit.h
   *  Simple interface, suitable to act as facade unit for the Hlt Algorithm
   *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
   *  @date   2008-11-10
   */
  class IHltUnit : virtual public IInterface 
  {
  public:
    // ========================================================================
    /// the actual type of functor 
    typedef LoKi::AuxFunBase Client ;             // the actual type of functor 
    /// the actual type of Key
    typedef stringKey        Key    ;       // the actual type of selection key 
    // ========================================================================
  public:
    // ========================================================================
    /** declare (the templated) selection
     *  @param key the key to be used for selection 
     *  @return pointer to the registered seelction (or NULL)
     */
    template <class T>
    Hlt::TSelection<T>* 
    declareOutput 
    ( const Key&    key    , 
      const Client& client ) const ;
    // ========================================================================
    /** register the selection 
     *  @param selection the selection to be registered 
     *  @param client the client 
     *  @return status code 
     */
    virtual StatusCode 
    registerOutput 
    ( Hlt::Selection* selection , 
      const Client&   client    ) const = 0 ;
    // ========================================================================
    /** declare the input selection 
     *  @param key the selection key 
     *  @param client the client 
     */
    virtual const Hlt::Selection* 
    declareInput 
    ( const Key&      key      , 
      const Client&   client   ) const = 0 ;
    // ========================================================================
    /** register the query  to TES-selection 
     *  @param location TES location to be registered
     *  @param consumer algorithm/consumer 
     *  @return Status Code 
     */
    virtual StatusCode   
    registerTESInput
    ( const Key&         location  ,                   //          TES location 
      const Client&      client    ) const = 0 ;       //                client
    // ========================================================================
  public:
    // ========================================================================
    /** get the (const) selection by key 
     *  @param key the key 
     *  @return pointer to the selection 
     */
    virtual const Hlt::Selection* 
    selection 
    ( const Key&      key      , 
      const Client&   client   ) const = 0 ;
    // =========================================================================
  public:
    // =========================================================================
    /** get the (const) selection by key  (anonymous)  
     *  @param key the key 
     *  @return pointer to the selection 
     */
    virtual const Hlt::Selection* 
    selection ( const Key& key ) const = 0 ;
    // =========================================================================
  public:
    // =========================================================================
    /** get the object form TES 
     *  @param client the client 
     *  @param location the TES location 
     *  @return the object 
     */
    virtual const DataObject*
    tes 
    ( const Client& client   , 
      const Key&    location ) const = 0 ;
    // =========================================================================
  public:
    // =========================================================================    
    /** get the data form TES 
     *  @param client the client 
     *  @param TES -location 
     *  @return the data 
     */
    template <class TYPE>
    const TYPE* tesData  
    ( const Client& client   , 
      const Key&    location ) const ;
    // ========================================================================
  public:
    // =========================================================================
    /** get the selection by key (non-const)
     *  @param key the key 
     *  @return pointer to the selection 
     */
    virtual Hlt::Selection* 
    retrieve  
    ( const Client& client , 
      const Key&    key    ) const = 0 ;
    // ========================================================================
  public: // IInterface related stuff 
    // ========================================================================
    /// the unique interface identifier 
    static const InterfaceID& interfaceID() ;
    // ========================================================================
  protected:
    // ========================================================================
    /// virtual & protected destructor 
    virtual ~IHltUnit () ;                    // virtual & protected destructor 
    // ========================================================================
  } ;
  // ==========================================================================
} // end of namespace LoKi 
// ============================================================================
/*  declare (the templated) selection
 *  @param key the key to be used for selection 
 *  @return pointer to the registered seelction (or NULL)
 */
// ============================================================================
template <class T>
Hlt::TSelection<T>* 
LoKi::IHltUnit::declareOutput 
( const LoKi::IHltUnit::Key&    key    , 
  const LoKi::IHltUnit::Client& client ) const 
{
  Hlt::TSelection<T>* selection = new Hlt::TSelection<T>( key ) ;
  StatusCode sc = this->registerOutput ( selection , client ) ;
  if ( sc.isSuccess() ) { return selection ; }                    // RETURN 
  delete selection ;
  return 0 ;
}
// ============================================================================
/*  get the data form TES 
 *  @param client the client 
 *  @param TES -location 
 *  @return the data 
 */
// ============================================================================
template <class TYPE>
const TYPE* LoKi::IHltUnit::tesData  
( const LoKi::IHltUnit::Client& client   , 
  const LoKi::IHltUnit::Key&    location ) const 
{
  const DataObject* obj = this->tes ( client , location ) ;
  if ( 0 == obj ) { return 0 ; }
  return dynamic_cast<const TYPE*> ( obj ) ;      
}
// ============================================================================

// ============================================================================
// The END 
// ============================================================================
#endif // LOKI_IHLTFACADE_H
// ============================================================================
