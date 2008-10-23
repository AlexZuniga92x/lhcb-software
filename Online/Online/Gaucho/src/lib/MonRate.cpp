#include "Gaucho/MonRate.h"

MonRate::MonRate(IMessageSvc* msgSvc, const std::string& source, int version):
  MonProfile(msgSvc, source, version)
{
  //std::cout << "MonRate constructor" << std::endl;
  m_typeName=s_monRate;
  m_dimPrefix="MonR";
  m_runNumber = new int(0);
  m_cycleNumber = new int(0);
  m_deltaT = new double(0);
  m_offsetTimeFirstEvInRun = new double(0);
  m_offsetTimeLastEvInCycle = new double(0);
  m_offsetGpsTimeLastEvInCycle = new double(0);
  isServer = true;
  m_maxNumCounters = 0;
  m_numCounters = 0;
}
  
MonRate::~MonRate(){
}

void MonRate::save(boost::archive::binary_oarchive & ar, const unsigned int version){
  MonObject::save(ar, version);
  MsgStream msg = createMsgStream();
//   msg <<MSG::INFO<<"**********************************************" << endreq;
  
  if (isServer) {
    //msg <<MSG::INFO<<"THIS IS A SERVER " << endreq;
    m_numCounters = m_counterMap.size();
//     msg <<MSG::INFO<<"m_numCounters: " << m_numCounters << endreq;
    m_profile = new TProfile("profile","MonRate Profile", m_maxNumCounters+7, 0, m_maxNumCounters+7);
//    m_profile = new TProfile("profile","MonRate Profile", m_numCounters+7, 0, m_numCounters+7);
    m_profile->Fill(0.50, *m_offsetTimeFirstEvInRun, 1.00);
    m_profile->Fill(1.50, *m_offsetTimeLastEvInCycle, 1.00);
    m_profile->Fill(2.50, *m_offsetGpsTimeLastEvInCycle, 1.00);
    m_profile->Fill(3.50, 1.00, 1.00);
    m_profile->Fill(4.50, (double) (*m_runNumber), 1.00);
    m_profile->Fill(5.60, (double) (*m_cycleNumber), 1.00);
    m_profile->Fill(6.50, (double) (*m_deltaT), 1.00);
    
 /*   msg <<MSG::INFO<<"bin[1]=" << (*m_offsetTimeFirstEvInRun) << ", OffsetTimeFirstEvInRun" << endreq;    
    msg <<MSG::INFO<<"bin[2]=" << (*m_offsetTimeLastEvInCycle) << ", OffsetTimeLastEvInCycle" << endreq;
    msg <<MSG::INFO<<"bin[3]=" << (*m_offsetGpsTimeLastEvInCycle) << ", OffsetGpsTimeLastEvInCycle" << endreq;
    msg <<MSG::INFO<<"bin[4]=" << 1 << ", one" << endreq;
    msg <<MSG::INFO<<"bin[5]=" << (*m_runNumber) << ", RunNumber" << endreq;
    msg <<MSG::INFO<<"bin[6]=" << (*m_cycleNumber) << ", CycleNumber" << endreq;
    msg <<MSG::INFO<<"bin[7]=" << (*m_deltaT) << ", deltaT" << endreq; */
    int i = 8;
    for (m_counterMapIt = m_counterMap.begin(); m_counterMapIt != m_counterMap.end(); m_counterMapIt++) {
        
      if (m_counterMapIt->second.second.first.compare("int") ==0 ){
        msg <<MSG::DEBUG<<"Counter :" << m_counterMapIt->first << " is int !! "<< endreq;
        m_profile->Fill((double)i - 0.5, (*(int*)(m_counterMapIt->second.second.second)), 1.00);
      } 
      else if (m_counterMapIt->second.second.first.compare("double") ==0 ){
        msg <<MSG::DEBUG<<"Counter :" << m_counterMapIt->first << " is double !! "<< endreq;
        m_profile->Fill((double)i - 0.5, (*(double*)(m_counterMapIt->second.second.second)), 1.00);
      } 
      else if (m_counterMapIt->second.second.first.compare("long") ==0 ){
        msg <<MSG::DEBUG<<"Counter :" << m_counterMapIt->first << " is long !! "<< endreq;
        m_profile->Fill((double)i - 0.5, (*(long*)(m_counterMapIt->second.second.second)), 1.00);
      } 
      else {
        msg <<MSG::ERROR<<"Incompatible counter type for MonRate.." << endreq;
      }
//       msg <<MSG::INFO<<"bin [" << i << "]="<< (double)(*(m_counterMapIt->second.first)) << endreq;
      i++;
    }
//    for (int j = 7+m_numCounters; j < 7+m_maxNumCounters; j++) {
//       m_profile->Fill((double)j - 0.5, 0, 1.00);
//    }
    
    m_profile->GetXaxis()->SetBinLabel(1, "OffsetTimeFirstEvInRun");
    m_profile->GetXaxis()->SetBinLabel(2, "OffsetTimeLastEvInCycle");
    m_profile->GetXaxis()->SetBinLabel(3, "OffsetGpsTimeLastEvInCycle");
    m_profile->GetXaxis()->SetBinLabel(4, "One");
    m_profile->GetXaxis()->SetBinLabel(5, "RunNumber");
    m_profile->GetXaxis()->SetBinLabel(6, "CycleNumber");
    m_profile->GetXaxis()->SetBinLabel(7, "deltaT");
        
    i = 8;
    for (m_counterMapIt = m_counterMap.begin(); m_counterMapIt != m_counterMap.end(); m_counterMapIt++) {
      //msg <<MSG::INFO<<"description: " << (*(m_counterMapIt->second.second)).c_str() << endreq;
      m_profile->GetXaxis()->SetBinLabel(i, (*(m_counterMapIt->second.first)).c_str());
      i++;
    }
//    for (int j = 7+m_numCounters; j < 7+m_maxNumCounters; j++) {
 //     m_profile->GetXaxis()->SetBinLabel(i, " ");
 //   }
    
  }  
  else{
    //msg <<MSG::INFO<<"THIS IS A CLIENT " << endreq;
    //m_profile =  profile(); //NEVER CALL THIS FUNCTION IN ADDERS
  }

//  msg <<MSG::INFO<<"m_numCounters = " << m_profile->GetNbinsX() << endreq;
//  for (int j = 0; j< m_profile->GetNbinsX()+2; j++) {
//    msg <<MSG::INFO<<"bin["<<j<<"] = " << (ulonglong) m_profile->GetBinContent(j) << ", description: "<< m_profile->GetXaxis()->GetBinLabel(j) << ", errors: " << m_profile->GetBinError(j) << endreq;
 // }
  
  
  MonProfile::save(ar, version);
  
  ar & m_numCounters;
  if (isServer) {
    delete m_profile; m_profile = 0;
  }
}

void MonRate::load(boost::archive::binary_iarchive  & ar, const unsigned int version)
{
  MsgStream msg = createMsgStream();
  isServer = false;
  MonObject::load(ar, version);
  MonProfile::load(ar, version);
  ar & m_numCounters;
  
/*  m_profile = profile();
  for (int j = 0; j< m_profile->GetNbinsX()+2; j++) {
    msg <<MSG::INFO<<"bin["<<j<<"] = " << (ulonglong) m_profile->GetBinContent(j) << ", description: "<< m_profile->GetXaxis()->GetBinLabel(j) << ", entries: " << m_profile->GetBinEntries(j) << endreq;
  }*/
  
}

void MonRate::combine(MonObject * monObject) {
  MsgStream msg = createMsgStream();
  if (monObject->typeName() != this->typeName()){
    msg <<MSG::ERROR<<"Trying to combine "<<this->typeName() <<" and "<< monObject->typeName() << " failed." << endreq;
    return;
  }
  if (monObject->endOfRun() != this->endOfRun()){
    msg <<MSG::WARNING<<"Trying to combine two objects with diferent endOfRun flag failed." << endreq;
    return;
  }
  MonProfile::combine(monObject);  
}

void MonRate::copyFrom(MonObject * monObject){
  MsgStream msg = createMsgStream();
  isServer = false;
  if (monObject->typeName() != this->typeName()) {
    MsgStream msgStream = createMsgStream();
    msgStream <<MSG::ERROR<<"Trying to copy "<<this->typeName() <<" and "<<monObject->typeName() << " failed." << endreq;
    return;
  }
  MonProfile::copyFrom(monObject);
  
  m_numCounters = ((MonRate*)monObject)->numCounters();
}


void MonRate::reset(){
  MonProfile::reset();
}

void MonRate::print(){
  //MonObject::print();
  MonProfile::print();
/*  MsgStream msgStream = createMsgStream();
  msgStream << MSG::INFO << "*************************************"<<endreq;
  msgStream << MSG::INFO << " runNumber: "<<  runNumber() << endreq;
  msgStream << MSG::INFO << " cycleNumber: "<<  cycleNumber() << endreq;
  msgStream << MSG::INFO << " deltaT: "<<  deltaT() << endreq;
  msgStream << MSG::INFO << " offsetTimeFirstEvInRun: "<<  offsetTimeFirstEvInRun() << endreq;
  msgStream << MSG::INFO << " offsetTimeLastEvInCycle: "<<  offsetTimeLastEvInCycle() << endreq;
  msgStream << MSG::INFO << " offsetGpsTimeLastEvInCycle: "<<  offsetGpsTimeLastEvInCycle() << endreq;
  msgStream << MSG::INFO << "*************************************"<<endreq;
  msgStream << MSG::INFO << m_typeName << " counter size :"<< m_counterMap.size() << endreq;
  msgStream << MSG::INFO << m_typeName << " values :" << endreq;
  
  for (m_counterMapIt = m_counterMap.begin(); m_counterMapIt != m_counterMap.end(); ++m_counterMapIt)
    msgStream << MSG::INFO << " counter: "<<  m_counterMapIt->first << ", value: " << (*m_counterMapIt->second.first) << ", description: "<< (*m_counterMapIt->second.second) << endreq;
  msgStream << MSG::INFO << "*************************************"<<endreq;*/
}

