// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Det/DetDesc/src/component/XmlCnvSvc.h,v 1.5 2001-11-26 10:31:47 sponce Exp $

#ifndef DETDESC_XMLCNVSVC_H
#define DETDESC_XMLCNVSVC_H

// Include files
#include "DetDesc/IXmlSvc.h"
#include "DetDesc/IXmlParserSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/ICnvManager.h"
#include "GaudiKernel/System.h"
#include "CLHEP/Evaluator/Evaluator.h"

// Forward and external declarations
template <class TYPE> class SvcFactory;

/** @class XmlCnvSvc XmlCnvSvc.h DetDesc/XmlCnvSvc.h
 *
 *  A conversion service for XML based data. It is based on the generic
 *  conversion service and implements in addition the IXmlSvc interface.
 *
 *  @author Sebastien Ponce
 *  @author Radovan Chytracek
 *  @author Pere Mato
 */
class XmlCnvSvc : public ConversionSvc,
                  virtual public IXmlSvc {

  /// Friend needed
  friend class SvcFactory<XmlCnvSvc>;
  
public:
  
  /**
   * Initializes the service
   * @return status depending on the completion of the call
   */
  virtual StatusCode initialize();
  
  /**
   * Finalizes the service
   * @return status depending on the completion of the call
   */
  virtual StatusCode finalize();
  
  /**
   * Queries interfaces of Interface.
   * @param riid ID of Interface to be retrieved
   * @param ppvInterface Pointer to Location for interface pointer
   * @return status depending on the completion of the call
   */
  virtual StatusCode queryInterface (const IID& riid, void** ppvInterface);

  /**
   * Create an XML address using explicit arguments to identify a single object
   * @param svc_type the service type
   * @param CLID the CLID of the XML Element for which an address is created
   * @param par an array of two strings containing the dbname and objectname 
   * in this order
   * @param ip can be ignored for XML addresses
   * @param refpAddress the new address created
   * @return a StatusCode giving the status of the address creation
   */
  virtual StatusCode createAddress(unsigned char svc_type,
                                   const CLID& clid,
                                   const std::string* par, 
                                   const unsigned long* /*ip*/,
                                   IOpaqueAddress*& refpAddress);
  

  ///////////////////////////////////////////////////
  // implementation of the IXmlParserSvc interface //
  ///////////////////////////////////////////////////

  /**
   * This method parses an xml file and produces the corresponding DOM
   * document.
   * @param fileName the name of the file to parse
   * @return the document issued from the parsing
   */
  virtual DOM_Document parse (const char* fileName);

  /**
   * This method parses XML from a string and produces the corresponding DOM
   * document.
   * @param source the string to parse
   * @return the document issued from the parsing
   */
  virtual DOM_Document parseString (std::string source);

  /**
   * This clears the cache of previously parsed xml files.
   */
  virtual void clearCache();

  /////////////////////////////////////////////
  // implementation of the IXmlSvc interface //
  /////////////////////////////////////////////

  /** 
   * Evaluates a numerical expresion
   * @param expr expresion to evaluate. It may include units and parameters
   * @param check boolean to control if the value needs to be check for being a
   * dimentioned magnitude (with units)
   * @return return double value
   */
  virtual double eval( const char* expr, bool check = true );

  /** 
   * Evaluates a numerical expresion
   * @param expr expresion to evaluate. It may include units and parameters
   * @param check boolean to control if the value needs to be check for being a
   * dimentioned magnitude (with units)
   * @return return double value
   */
  virtual double eval( const std::string& expr, bool check = true );

  /**
   * Adds a parameter in the list of known parameters. The value can also be an
   * expression with units and other parameters.
   * @param name parameter name
   * @param value string which defines the value of the parameter.
   * @return true if success 
   */
  virtual bool addParameter (const std::string& name,
                             const std::string& value);

  /**
   * Adds a parameter in the list of known parameters. The value can also be an
   * expression with units and other parameters.
   * @param name parameter name
   * @param value string which defines the value of the parameter.
   * @return true if success 
   */
  virtual bool addParameter( const char* name, const char* value );

  /**
   * Adds a parameter in the list of known parameters. The value can also be an
   * expression with units and other parameters.
   * @param name parameter name
   * @param value string which defines the value of the parameter.
   * @return true if success 
   */
  virtual bool addParameter( const char* name, double value );

  /**
   * Removes a parameter from the list of known parameters
   * @param name parameter name
   * @return true if success 
   */
  virtual bool removeParameter( const std::string& name );

  /**
   * Removes a parameter from the list of known parameters
   * @param name parameter name
   * @return true if success 
   */
  virtual bool removeParameter( const char* name );

  /**
   * Accessor to m_genericConversion.
   * Tells whether generic conversion of user defined detector elements should
   * be used in case the corresponding user defined converter is not available
   * @return true if generic conversion is allowed
   */
  bool allowGenericCnv() { return m_genericConversion; }

  
protected:
  
  /**
   * Standard Constructor
   * @param name String with service name
   * @param svc Pointer to service locator interface
   * @return Reference to CdfPersCnvSvc
   */
  XmlCnvSvc (const std::string& name, ISvcLocator* svc);
  
  /**
   * Default destructor
   */
  virtual ~XmlCnvSvc();


private:

  /// XmlParserSvc used to parse xmlfiles
  IXmlParserSvc* m_parserSvc;

  /// Numerical expressions parser
  HepTool::Evaluator m_xp;

  /**
   * Property triggering generic conversion of user defined detector
   * elements in case the corresponding user defined converter is not
   * available
   */
  bool  m_genericConversion;

};

#endif    // DETDESC_XMLCNVSVC_H
