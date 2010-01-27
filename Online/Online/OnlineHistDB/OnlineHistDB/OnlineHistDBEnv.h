// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/OnlineHistDB/OnlineHistDB/OnlineHistDBEnv.h,v 1.46 2010-01-27 10:04:08 ggiacomo Exp $
#ifndef ONLINEHISTDBENV_H
#define ONLINEHISTDBENV_H 1
/** @class  OnlineHistDBEnv OnlineHistDBEnv.h OnlineHistDB/OnlineHistDBEnv.h
 * 
 *  environment class for C++ interface to the Online Monitoring Histogram DB
 *  containing constants and common OCI-based tools
 *  @author Giacomo Graziani (INFN Firenze)
 *  @date 1/12/2006
 */

#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <oci.h>
class OnlineTaskStorage;
class OnlineHistogramStorage;
class OnlinePageStorage;

namespace OnlineHistDBEnv_constants {
  static const std::string DB="HISTDB";
  static const std::string ACCOUNT="HIST_READER";
  static const std::string PASSWORD="reader";
  static const unsigned int DBschema = 15;
  static const std::string version = "v6r3";

  static const int NHTYPES=6;
  static const char HistTypeName[][4] = {
    "H1D", "H2D", "P1D", "P2D", "CNT", "SAM"
  };  

  static const std::string StdRefRoot = "Reference"; 
  static const std::string StdSavesetsRoot = "Savesets"; 
  static const std::string m_SetSeparator = "_$";

  static const unsigned int VSIZE_SN          = 131;
  static const unsigned int VSIZE_HID         =  13;
  static const unsigned int VSIZE_TYPE        =   4;
  static const unsigned int VSIZE_HSTITLE     = 101;
  static const unsigned int VSIZE_SUBTITLE    =  51;
  static const unsigned int VSIZE_HSTASK      =  65;
  static const unsigned int VSIZE_HSALG       =  65;
  static const unsigned int VSIZE_DESCR       =4001;
  static const unsigned int VSIZE_DOC         = 201;
  static const unsigned int VSIZE_ALGNAME     =  31; 
  static const unsigned int VSIZE_NAME        = 131;
  static const unsigned int VSIZE_FOLDER      = 301;
  static const unsigned int VSIZE_PAGENAME    = 351;
  static const unsigned int VSIZE_PAGEDOC     =2001;
  static const unsigned int VSIZE_SSNAME      =  11;
  static const unsigned int VSIZE_REFERENCE   = 101;
  static const unsigned int VSIZE_PARAMETERS  =  16;
  static const unsigned int VSIZE_FITFUN      =  16;
  static const unsigned int VSIZE_SAVESET     = 501;
  static const unsigned int VSIZE_ALEVEL      =   8;
  static const unsigned int VSIZE_MESSAGE     =4001;
  static const unsigned int VSIZE_ANANAME     = 301;
  static const unsigned int VSIZE_PGPATTERN   = 101;
  static const unsigned int VSIZE_ANAMSG      = 201;

  static std::string unknown = "unknown";
}




class OnlineHistDBEnv {
 public:
  /// get verbosity level (0 for no debug messages, up to 3)
  typedef enum { H1D=0, H2D, P1D, P2D, CNT, SAM} HistType;
  inline int debug() const { return m_debug;}
  /// set verbosity level
  void setDebug(int DebugLevel) { m_debug=DebugLevel;}
  /// exception level: 0 means never throw exc. to client code, 1 means only
  /// severe errors (default value), 2 means throw all exceptions.
  /// The default value is 1, meaning that exceptions are thrown only in
  /// case of severe DB inconsistency. All other errors, e.g. syntax errors,
  /// can be checked from the method return values and from the warning
  /// messages on the standard output.
  /// Exceptions can be catched using {\it catch(std::string message)}
  int excLevel() const {return m_excLevel;}
  ///
  void setExcLevel(int ExceptionLevel) {m_excLevel=ExceptionLevel;}
  /// check the syntax of the page full name, returning the correct syntax and the folder name 
  std::string PagenameSyntax(std::string &fullname, std::string &folder);
  /// gets the number of parameters, and optionally the number of input histograms, needed by algorithm AlgName.
  int getAlgorithmNpar(std::string& AlgName,
		       int* Ninput = NULL);
  /// gets the name of parameter Ipar (starting from 1) of algorithm AlgName
  std::string getAlgParName(std::string& AlgName,
			    int Ipar);
  /// get reference histograms root directory 
  inline std::string& refRoot() {return (m_refRoot ? (*m_refRoot) : OnlineHistDBEnv_constants::unknown);}
  /// get saveset root directory 
  inline std::string& savesetsRoot() {return (m_savesetsRoot ? (*m_savesetsRoot) : OnlineHistDBEnv_constants::unknown);}

  void warningMessage(std::string Error) const; 
  inline OCIEnv *envhp() const {return m_envhp;}
  inline OCIError *errhp() const {return m_errhp;}
  inline OCISvcCtx *svchp() const {return m_svchp;}
  inline void setStmtMethod(std::string Caller) {m_StmtMethod = Caller;}

 protected:
  OnlineHistDBEnv(std::string passwd,
                  std::string user,
                  std::string db, 
                  bool newsession=false); 
  OnlineHistDBEnv(OnlineHistDBEnv &m);
  void copyEnv(OnlineHistDBEnv &m);
  OnlineHistDBEnv();
  
  virtual ~OnlineHistDBEnv() ;

  bool m_ownEnv;
  // methods for OCI statements and OCI error handling
  std::string m_StmtMethod;
  typedef enum { 
    NORMAL,      // error that may occur normally  
    MODERATE,    // error that can be tolerated 
    SEVERE       // severe error that always triggers an exception
  } OCIErrorLevel;
  sword checkerr(sword status,
		OCIErrorLevel level = MODERATE);


  void initOCIBinds();
  void getOCITypes();

  sword prepareOCIStatement(OCIStmt* & stmt, 
			    const char *sqlcommand,
			    bool cleanBinds=true);
  sword prepareOCITaggedStatement(OCIStmt* & stmt, 
				  const char *sqlcommand,
				  const char *StmtKey,
				  bool cleanBinds=true);
  sword releaseOCIStatement(OCIStmt* & stmt);
  sword releaseOCITaggedStatement(OCIStmt* & stmt, 
				  const char *StmtKey);
  sword myOCIBindInt(OCIStmt* stmt,
		     const char *str, 
		     int &var,
		     sb2 * ind = NULL);
  sword myOCIBindFloat(OCIStmt* stmt,
		       const char *str, 
		       float &var,
		       sb2 * ind = NULL);
  sword myOCIBindString(OCIStmt* stmt,
			const char *str, 
			std::string &var,
			sb2 * ind = NULL);
  sword myOCIBindString(OCIStmt* stmt,
			const char *str, 
			text* var,
			int size,
			sb2 * ind = NULL);
  sword myOCIBindObject(OCIStmt* stmt,
			const char *str, 
			void** var,
			OCIType* type,
			void** ind_struct = NULL);
  sword myOCIDefineInt(OCIStmt* stmt,
		      int position, 
		      int &var,
		      sb2 *isNULL = NULL);
  sword myOCIDefineFloat(OCIStmt* stmt,
			 int position, 
			 float &var,
			 sb2 *isNULL = NULL);
  sword myOCIDefineString(OCIStmt* stmt,
			  int position, 
			  text* var,
			  int size,
			  sb2 *isNULL = NULL);
  sword myOCISelectExecute(OCIStmt* stmt,
			OCIErrorLevel level = MODERATE);
  sword myOCIStmtExecute(OCIStmt* stmt,
			OCIErrorLevel level = MODERATE);
  int myOCIFetch(OCIStmt* stmt,
		 int Nf);
  int myOCIFetchedRows(OCIStmt* stmt);
    
  void intVarrayToVector(OCIColl* col, std::vector<int> &v);
  void floatVarrayToVector(OCIColl* col, std::vector<float> &v);
  void stringVarrayToVector(OCIColl* col, std::vector<std::string> &v);
  void floatVectorToVarray(std::vector<float> &v, OCIColl* col);
  void stringVectorToVarray(std::vector<std::string> &v,OCIColl* col);
  OCIEnv *m_envhp;
  OCIError *m_errhp;
  OCISvcCtx *m_svchp;

  OCIType *OCIthresholds;
  OCIType *OCIparameters;
  OCIType *OCIlabels;
  OCIType *OCIintlist;
  OCIType *OCIinttlist;
  OCIType *OCIanalist;
  OCIType *OCIhnalist;
  OCIType *OCIflolist;
  OCIType *OCIdispopt;

  std::set<std::string> *m_TaggedStatement;
  std::string *m_refRoot;
  std::string *m_savesetsRoot;

  OnlineTaskStorage *m_TStorage;
  OnlineHistogramStorage *m_HStorage;
  OnlinePageStorage *m_PStorage;

 private: 
  std::string m_user;

  OCIBind* m_bnd[40];
  unsigned int m_curBind;

  void checkCurBind();
  void resetTaggedStatements();
  void toUpper(std::string& str);
  int m_debug;
  int m_excLevel;
};

#endif // ONLINEHISTDBENV_H
