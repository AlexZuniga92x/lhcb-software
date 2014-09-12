#ifndef LIB_TMVATRANSFORM_H 
#define LIB_TMVATRANSFORM_H 1


//#include "GaudiKernel/MsgStream.h"
#include <strstream>
#include "Kernel/IParticleDictTool.h"
#include "TMVA/Reader.h"

#include "MVADictTools/Options.h"

/** @class TMVATransform
 *  Policy class to be used by the DictTransform template
 *  Implementing the TMVA Reader backend
 *
 *  @author Sam Hall
 *  @date   2013-07-29
 */

class TMVATransform
{

public:

  typedef std::map<std::string, std::string> optmap;

private:

  bool m_setup_success;
  bool m_keep_all_vars;
  std::string m_reader_opts;
  std::string m_weightfile;
  std::string m_name;
  std::string m_branchname;
  std::string m_default_path;
  std::vector<std::string> m_spectator;

  // TMVA bits
  TMVA::Reader* m_reader;

  bool m_debug;

private:

  // Helper Functions
  void readWeightsFile(std::ostream&);
  void setupReader(std::ostream&);
  bool parseOpts(const optmap&, std::ostream&);

public:

  std::vector<std::string> m_variables; /// variables needed by the classifier

public:

  TMVATransform();
  ~TMVATransform();

  // the policy methods neede for collaboration with DictTransform
  bool Init(const optmap& options, std::ostream& info, const bool debug = false );
  bool operator()(const IParticleDictTool::DICT& in, IParticleDictTool::DICT& out) const;
  bool checkWeightsFile(std::ostream& info);

};

#endif // LIB_TMVATRANSFORM_H
