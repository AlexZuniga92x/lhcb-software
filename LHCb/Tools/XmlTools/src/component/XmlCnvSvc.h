// $Id: XmlCnvSvc.h,v 1.7 2005-07-07 13:01:10 marcocle Exp $

#ifndef DETDESCCNV_XMLCNVSVC_H
#define DETDESCCNV_XMLCNVSVC_H

// Include files
#include "XmlTools/IXmlSvc.h"
#include "XmlTools/IXmlParserSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/ConversionSvc.h"
#include "GaudiKernel/ICnvManager.h"
#include "GaudiKernel/System.h"
#include "CLHEP/Evaluator/Evaluator.h"

// Forward and external declarations
template <class TYPE> class SvcFactory;

/** @class XmlCnvSvc XmlCnvSvc.h DetDescCnv/XmlCnvSvc.h
 *
 *  A conversion service for XML based data. It is based on the generic
 *  conversion service and implements in addition the IXmlSvc interface.
 *
 *  @author Sebastien Ponce
 *  @author Radovan Chytracek
 *  @author Pere Mato
 *  @author Marco Clemencic
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
   * Reinitializes the service
   * @return status depending on the completion of the call
   */
  virtual StatusCode reinitialize();
  
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
  virtual StatusCode queryInterface ( const InterfaceID& riid, 
                                      void** ppvInterface );

  /**
   * Create an XML address using explicit arguments to identify a single object
   * @param svc_type the service type
   * @param clid the CLID of the XML Element for which an address is created
   * @param par an array of two strings containing the dbname and objectname 
   * in this order
   * @param refpAddress the new address created
   * @return a StatusCode giving the status of the address creation
   * @note unsigned long fourth argument can be ignored for XML addresses
   */
  virtual StatusCode createAddress(long  svc_type,
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
  virtual xercesc::DOMDocument* parse (const char* fileName);

  /**
   * This method parses XML from a string and produces the corresponding DOM
   * document.
   * @param source the string to parse
   * @return the document issued from the parsing
   */
  virtual xercesc::DOMDocument* parseString (std::string source);

  /**
   * This clears the cache of previously parsed xml files.
   */
  virtual void clearCache();

  /// Method to remove the lock from a document in the cache or to delete the document
  /// generated from a string.
  virtual void releaseDoc(xercesc::DOMDocument* doc);

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

// -----------------------------------------------------------------------
// Useful functions to check units in parameters
// -----------------------------------------------------------------------
//
// The aim of all this is to say whether a given expression has a unit
// or not. The language we deal with is the following :
//    sum :=   product '+' sum
//           | product '-' sum
//    product :=   expr '*' product
//               | expr '/' product
//    expr :=   '-'? alphanumeric_token
//            | '-'? function_name? '(' sum ')'
// Where ? means that it is optionnal and | means or. alphanumeric_token
// contains only alphanumeric characters, except for a single '-' character
// if preceded by a digit and an 'e'
//
// The way it is implemented is the following :
//    - every function receives the string to be parsed and the
//  currently interesting part of it (between start or baseIndex
//  included and end or lastIndex exculded).
//    - the skip* functions return the index of the first character
//  following the sum/product/expr. This value is less or equal to end
//    - the *HasUnit functions tell whether a given sum/product/expr
//  has a unit
//
// The assumptions made are the following :
//    - a sum has a unit if and only if each sub product has one
//    - a product has a unit if any sub expr has one (which may be wrong
//  if you divide a unit by itself for example)
//    - an expression has a unit in only 3 cases :
//         o it is of the form -? '(' sum ')' and the sum has one
//         o it is of the form '-'? alphanumeric token and the token
//  starts with a letter. This statement makes the assumption that this
//  token is either describing a unit or is a variable that has a unit
//  itself, which may be wrong
//         o it is of the form '-'? function_name '(' sum ')' and the function
//  is "sqrt" or "abs" and sum has a unit
//    - note that the result of a function is supposed to have no unit
//  in general

  unsigned int skipSum (std::string s,
                        unsigned int start,
                        unsigned int end);
  unsigned int skipProduct (std::string s,
                            unsigned int start,
                            unsigned int end);
  unsigned int skipExpr (std::string s,
                         unsigned int start,
                         unsigned int end);
  
  bool sumHasUnit (std::string s,
                   unsigned int baseIndex,
                   unsigned int lastIndex);
  bool productHasUnit (std::string s,
                       unsigned int baseIndex,
                       unsigned int lastIndex);
  bool exprHasUnit (std::string s,
                    unsigned int baseIndex,
                    unsigned int lastIndex);
  

private:

  /// Name of the XmlParserSvc to use (option XmlCnvSvc.XmlParserSvc, default "XmlParserSvc")
  std::string m_parserSvcName;

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

  /**
   * Property specifying the directory containing standard DTD files.
   * This is necessary when parsing XML strings rather than XML files.
   */
  std::string m_dtdLocation;

  /**
   * tells whether to check parameters for units or not.
   */
  bool m_checkUnits;
};

#endif    // DETDESCCNV_XMLCNVSVC_H
