// $Id: MicroDSTCommon.h,v 1.11 2010-08-12 14:47:51 jpalac Exp $
#ifndef MICRODST_MICRODSTCOMMON_H 
#define MICRODST_MICRODSTCOMMON_H 1

// Include files
#include <MicroDST/Functors.hpp>
#include <GaudiKernel/KeyedObject.h>

class ISvcLocator;
class IInterface;

/** @class MicroDSTCommon MicroDSTCommon.h MicroDST/MicroDSTCommon.h
 *  
 *
 *  @author Juan PALACIOS
 *  @date   2007-11-30
 */
template <class PBASE>
class MicroDSTCommon : public PBASE {
public: 
  /// Standard constructor

  MicroDSTCommon();

  /// Standard algorithm-like constructor
  MicroDSTCommon( const std::string& name,
                  ISvcLocator* pSvcLocator )
    : 
    PBASE(name, pSvcLocator),
    m_outputPrefix("microDST")
  {
    this->declareProperty( "OutputPrefix",  m_outputPrefix  );
  }
  

  /// Standard tool-like constructor
  MicroDSTCommon( const std::string& type,
                  const std::string& name,
                  const IInterface* parent )
    : 
    PBASE(type, name, parent),
    m_outputPrefix("microDST")
  {
    this->declareProperty( "OutputPrefix",  m_outputPrefix  );
  }
  

  virtual ~MicroDSTCommon( ); ///< Destructor

  virtual StatusCode initialize();
  
  virtual StatusCode finalize();

  /**
   * Copy an object of type T from the TES onto another TES location.
   * Uses the copy constructor of type T for copying. Do not copy if object
   * already exists in that location.
   * 
   * @param from The TES location of the object to be copied
   * @param to   The TES location where the copy should be stored
   * @return     Const pointer to the cloned object.
   *
   * @author Juan Palacios juancho@nikhef.nl
   * @author Ulrich Kerzel
   */
  template <class CopyFunctor>
  inline const typename CopyFunctor::Type* copyAndStoreObject( const std::string& from,
                                                               const std::string& to   );


  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class Cloner >
  inline const typename Cloner::Type::Container* copyKeyedContainer( const std::string& from, 
                                                                     Cloner* cloner    ) ;

  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class Cloner >
  inline const typename Cloner::Type::Container* copyKeyedContainer( const typename Cloner::Type::Container* from, 
                                                                     Cloner* cloner    ) ;

  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class Cloner>
  inline typename Cloner::Type* cloneKeyedContainerItem( const typename Cloner::Type* item );

  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class T>
  inline const T* getStoredClone( const KeyedObject<int>* original    ) const;

  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class T>
  inline T* getStoredClone( const KeyedObject<int>* original    ) ;


  /**
   * Get the TES container in the TES location to be stored on the
   * MicroDST. 
   * @param locTES The address in the original TES
   * @return pointer to the container in the MicroDST TES
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class T>
  inline T* getOutputContainer( const std::string& location );

  /**
   *
   * @author Juan Palacios juancho@nikhef.nl
   */
  template <class T>
  inline const T* getInputContainer( const std::string& location ) ;


  template <class T>
  inline bool exist(const std::string& location) {
    return PBASE:: template exist<T>(location);
  }

  template <class T>
  inline T* get(const std::string& location) {
    return PBASE:: template get<T>(location);
  }

protected:

  inline const std::string& outputPrefix() const 
  {
    return m_outputPrefix;
  }

  inline const std::string outputTESLocation(const std::string& inputLocation) const
  {
    return "/Event/"+ this->outputPrefix() + "/" + 
      MicroDST::niceLocationName(inputLocation);
  }

private:

  std::string m_outputPrefix;
  std::string m_fullOutputTESLocation;
  
};
// templated method implementations.
#include "MicroDSTCommon.icpp"
#endif // MICRODST_MICRODSTCOMMON_H
