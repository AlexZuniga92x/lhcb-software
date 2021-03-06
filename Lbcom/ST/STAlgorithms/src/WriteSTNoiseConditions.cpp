// local
#include "WriteSTNoiseConditions.h"

// STDet
#include "STDet/DeSTDetector.h"
#include "STDet/DeSTSector.h"


// STKernel
#include "Kernel/STXMLUtils.h"

DECLARE_ALGORITHM_FACTORY( WriteSTNoiseConditions )

//--------------------------------------------------------------------
//
//  WriteSTNoiseConditions
//
//--------------------------------------------------------------------

WriteSTNoiseConditions::WriteSTNoiseConditions( const std::string& name, 
                                                  ISvcLocator* pSvcLocator ) :
  ST::AlgBase(name, pSvcLocator)
{
  // constructer
  declareProperty("footer", m_footer = "</DDDB>");
  declareProperty("startTag", m_startTag = "<condition");
  declareProperty("outputFile",m_outputFileName = "NoiseValuesInfo.xml");
  declareProperty("depths", m_depth = 3u );
  declareProperty("precision", m_precision = 4u);
  declareProperty("removeCondb", m_removeCondb = false);
  declareProperty("author", m_author = "Joe Bloggs");
  declareProperty("tag", m_tag = "None");
  declareProperty("description", m_desc = "BlahBlahBlah"); 

  setForcedInit();
}

StatusCode WriteSTNoiseConditions::execute()
{
  // execute once per event
  return StatusCode::SUCCESS;
}

StatusCode WriteSTNoiseConditions::initialize()
{
  StatusCode sc = ST::AlgBase::initialize();
  if (sc.isFailure()) {
    return Error("Failed to intialize");
  }

  info() << "detector   = " << detType() << endmsg;
  info() << "footer     = " << m_footer << endmsg;
  info() << "startTag   = " << m_startTag << endmsg;
  info() << "outputFile = " << m_outputFileName << endmsg;
  info() << "precision  = " << m_precision << endmsg;
  
  return sc;
}


StatusCode WriteSTNoiseConditions::finalize()
{
  info() << "Writing alignment conditions to file" << endmsg;
  // Print out the full tree
  info() << "Trying for top element " << tracker()->name() << endmsg;

  // get the sectors 
  const DeSTDetector::Sectors& sectors = tracker()->sectors();

  std::ofstream outputFile(m_outputFileName.c_str());
  if (outputFile.fail() ){
    return Warning("Failed to open output file",StatusCode::FAILURE);
  }

  // write the xml headers 
  outputFile << header(sectors.front()->noiseCondition()->toXml("", 
                                                                 true, 
                                                                 m_precision))
             << std::endl;

  // add comments
  std::ostringstream comment;
  ST::XMLUtils::fullComment(comment, m_author, m_tag, m_desc);
  outputFile << comment.str() << std::endl; 

  for( DeSTDetector::Sectors::const_iterator iterS = sectors.begin();
       iterS != sectors.end(); ++iterS){
    const Condition* aCon = (*iterS)->noiseCondition();
    std::string temp = strip(aCon->toXml("", false, m_precision));
    outputFile << temp << "\n"  << std::endl; 
  } // sectors
  
  // write the footer
  outputFile << footer() << std::endl;

  return ST::AlgBase::finalize();
}

std::string WriteSTNoiseConditions::footer() const
{
  std::string temp = m_footer;
  temp.insert(0, "</catalog>" );
  return temp;
} 

std::string WriteSTNoiseConditions::header(const std::string& conString) const
{
  // get the header
  std::string::size_type startpos = conString.find(m_startTag);
  std::string temp = conString.substr(0,startpos);
  temp.insert(startpos, "<catalog name=\"NoiseValues\">" );

  // correct the location of the DTD
  if( m_removeCondb ) {
    ST::XMLUtils::replace(temp,"conddb:", "");
    std::string location;
    for (unsigned int i = 0;  i< m_depth; ++i) location += "../";
    std::string::size_type pos = temp.find("/DTD/");
    temp.insert(pos,location);
    ST::XMLUtils::replace(temp,"//", "/");
  }

  return temp;
} 


std::string WriteSTNoiseConditions::strip(const std::string& conString) const
{
  std::string::size_type startpos = conString.find(m_startTag);
  std::string::size_type endpos = conString.find(m_footer);
  return conString.substr(startpos, endpos - startpos);
} 
