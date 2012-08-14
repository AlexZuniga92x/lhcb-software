#include "HLTFileEqualizer.h"
#include <math.h>
#include <time.h>
#include "ROMon/Utilities.h"

static FILE *outf;
static bool inhibit_act;
HLTFileEqualizer::HLTFileEqualizer()
{
  DimUpdatedInfo *sfinfo;
  m_InfoHandler = new DefHltInfoHandler(this);
  m_MBMInfoHandler = new MBMInfoHandler(this);
  static int NoLink=-1;
  char sf[128];
  char row;
  m_nnodes = 0;
  m_nfiles = 0;
  m_nfiles2 = 0;
  m_low = 10;
  m_high = 20;
  m_enabledFarm.clear();
  m_NodeList = 0;
  m_NodeListDiff = 0;
  for (row='b';row<='e';row++)
  {
    for (int rack=1;rack<=11;rack++)
    {
      if (rack==5) continue;
      sprintf(sf,"/RO/hlt%c%02d/ROpublish/HLTDefer",row,rack);
      sfinfo = new DimUpdatedInfo(sf,(void*)&NoLink,sizeof(int),m_InfoHandler);
      m_infoMap.insert(std::make_pair(std::string(sf),sfinfo));
      fprintf(outf,"%s\n",sf);
      sprintf(sf,"/RO/hlt%c%02d/ROpublish",row,rack);
      sfinfo = new DimUpdatedInfo(sf,(void*)&NoLink,sizeof(int),m_MBMInfoHandler);
      m_infoMap.insert(std::make_pair(std::string(sf),sfinfo));
      fprintf(outf,"%s\n",sf);
    }
  }
  row = 'f';
  for (int rack=1;rack<=7;rack++)
  {
    if (rack==5) continue;
    sprintf(sf,"/RO/hlt%c%02d/ROpublish/HLTDefer",row,rack);
    sfinfo = new DimUpdatedInfo(sf,(void*)&NoLink,sizeof(int),m_InfoHandler);
    m_infoMap.insert(std::make_pair(std::string(sf),sfinfo));
    fprintf(outf,"%s\n",sf);
    sprintf(sf,"/RO/hlt%c%02d/ROpublish",row,rack);
    sfinfo = new DimUpdatedInfo(sf,(void*)&NoLink,sizeof(int),m_MBMInfoHandler);
    m_infoMap.insert(std::make_pair(std::string(sf),sfinfo));
    fprintf(outf,"%s\n",sf);
  }
//  m_enabledFarm.insert(std::string("hltb01"));
//  m_enabledFarm.insert(std::string("hltb02"));
}
namespace
{
//  inline void toLowerCase(std::string &s)
//  {
//    for (unsigned int i=0;i<s.size();i++)
//    {
//      s[i] = tolower(s[i]);
//    }
//  }
  inline void toUpperCase(std::string &s)
  {
    for (unsigned int i=0;i<s.size();i++)
    {
      s[i] = toupper(s[i]);
    }
  }
}
void HLTFileEqualizer::Analyze()
{
  myActionMap Actions;
  dim_lock();
  float av_files;
  float rms;
  m_nnodes = 0;
  m_nfiles = 0;
  m_nfiles2 = 0;

  for (myNodeMap::iterator nit=m_Nodes.begin();nit != m_Nodes.end();nit++)
  {
    myNode *nod = (*nit).second;
    if (nod->m_excl)
    {
      continue;
    }
    m_nfiles += nod->m_nofiles;
    m_nfiles2 += nod->m_nofiles*nod->m_nofiles;
    m_nnodes++;
  }
  if (0 != m_nnodes)
  {
    av_files = float(this->m_nfiles)/this->m_nnodes;
    rms = double(m_nfiles2)/m_nnodes-double(av_files)*double(av_files);
    if (0.0<rms)
    {
      rms = sqrt(rms);
    }
    else
    {
      rms = 0.0;
    }
  }
  else
  {
    if (0 != m_nfiles)
    {
      av_files = 100000000.0;
    }
    else
    {
      av_files = 0.0;
    }
    rms = double(m_nfiles2)/m_nnodes-double(av_files)*double(av_files);
    if (0.0<rms)
    {
      rms = ::sqrt(rms);
    }
    else
    {
      rms = 0.0;
    }
  }
  fprintf(outf,"Analysis Summary based on %d nodes: ",m_nnodes);
  {
    time_t rawtime;
    time(&rawtime);
    fprintf(outf,"%s",asctime(localtime(&rawtime)));
  }
  int n_ena = 0;
  int n_dis = 0;
  int nfiles = 0;
  long nfiles2 = 0;
  int nnodes = 0;
  int tot_ena = 0;
  int tot_dis = 0;
  int ROC_tot_ena = 0;
  int ROC_tot_dis = 0;
  bool act = (m_DefStateInfo->getInt() == 1);
  fprintf(outf,"Analyzer: First round of analysis Average number of files per node: %f +/- %f\n",av_files,rms);
  for (myNodeMap::iterator nit=m_Nodes.begin();nit != m_Nodes.end();nit++)
  {
    myNode *nod = (*nit).second;
    if ((nod->m_nofiles > av_files+2.0*rms) || (nod->m_nofiles <av_files-2.0*rms))
    {
      continue;
    }
    if (nod->m_excl)
    {
      continue;
    }
    nfiles += nod->m_nofiles;
    nfiles2 += nod->m_nofiles*nod->m_nofiles;
    nnodes++;
  }
  if (nnodes >0)
  {
    av_files = float(nfiles)/nnodes;
    rms = double(nfiles2)/nnodes - double(av_files*av_files);
    if (rms>0)
    {
      rms = ::sqrt(rms);
    }
    else
    {
      rms = 0.0;
    }
  }
  else
  {
    av_files = 0;
    rms = 0.0;
  }
  for (myNodeMap::iterator nit=m_Nodes.begin();nit != m_Nodes.end();nit++)
  {
    myNode *nod = (*nit).second;
    if (nod->m_excl)
    {
      continue;
    }
    if (nod->m_nofiles > av_files+m_high)
    {
      if (nod->m_ROC_state == 'Y')
      {
        std::string farm;
        farm = nod->m_name.substr(0,6);
        Actions[farm].push_back(std::make_pair(nod->m_name,0));
        nod->m_state = 0;
      }
      n_dis++;
    }
    else if (nod->m_nofiles < av_files+m_low)
    {
      std::string farm;
      farm = nod->m_name.substr(0, 6);
      nod->m_state = 1;
      Actions[farm].push_back(std::make_pair(nod->m_name,1));
      n_ena++;
    }
    if (nod->m_state == 0) tot_dis++;
    else tot_ena++;
    if (nod->m_ROC_state == 'Y') ROC_tot_ena++;
    else ROC_tot_dis++;
  }
  fprintf(outf,"Analyzer: Second round (within +/- 5 sigma) of analysis Average number of files per node: %f +/- %f\n",av_files,rms);
  fprintf(outf,"%d Nodes enabled; %d Nodes disabled\n",n_ena,n_dis);
  fprintf(outf,"Total %d Nodes enabled; Total %d Nodes disabled\n",tot_ena,tot_dis);
  fprintf(outf,"From ROcollect: Total %d Nodes enabled; Total %d Nodes disabled\n",ROC_tot_ena,ROC_tot_dis);
  m_nnodes = 0;
  m_nfiles = 0;
  m_nfiles2 = 0;
  if (act && !inhibit_act)
  {
    myActionMap::iterator fit;
    for (fit = Actions.begin(); fit != Actions.end(); fit++)
    {
      //    DimClient::setDnsNode((*fit).first.c_str());
      if (!m_enabledFarm.empty() && (m_enabledFarm.find((*fit).first)
          == m_enabledFarm.end()))
      {
        continue;
      }
      //    fprintf(outf,"On Farm %s:\n",(*fit).first.c_str());
      std::list<std::pair<std::string, int> >::iterator i;
      std::string sf_mesg = "";
      std::string endisSvc;
      endisSvc = (*fit).first + "_HLTDefBridge/EnDisCommand";
      for (i = (*fit).second.begin(); i != (*fit).second.end(); i++)
      {
        std::string svcname;
        std::string node = (*i).first;
        toUpperCase(node);
        svcname = node + "_MEPRx_01/setOverflow";
        char cmd[1024];
        sprintf(cmd, "%s %d|", svcname.c_str(), (*i).second);
        sf_mesg.append(cmd);

      }
      DimClient::sendCommandNB(endisSvc.c_str(), (char*) sf_mesg.c_str());
    }
  }
  else
  {
    fprintf(outf,"Defered HLT disabled. Not acting...\n");
  }
  fprintf(outf,"==================\n");
  fflush(outf);
  m_servdat.erase();
  for (myNodeMap::iterator nit=m_AllNodes.begin();nit != m_AllNodes.end();nit++)
  {
    myNode *nod = (*nit).second;
    char nfile[32];
    sprintf(nfile,"%s %d|",nod->m_name.c_str(),nod->m_nofiles);
    m_servdat += nfile;
  }
  m_servdat += '\0';
  m_NodeList->setData((void*)m_servdat.c_str(),m_servdat.size());
  m_NodeList->updateService();
  m_servdatDiff.erase();
  m_servdatNodesRunsFiles.erase();
  for (NodeSet::iterator nit=m_recvNodes.begin();nit != m_recvNodes.end();nit++)
  {
    std::string nname;
    nname = *nit;
    myNodeMap::iterator nodeit = m_AllNodes.find(nname);
    myNode *nod = (*nodeit).second;
    char nfile[32];
    sprintf(nfile,"%s %d|",nod->m_name.c_str(),nod->m_nofiles);
    m_servdatDiff += nfile;
    if (nod->m_nofiles >0)
    {
      if (nod->m_ROC_state == 'Y')
      {
        m_servdatNodesRunsFiles += nod->m_name+" 1 ";
      }
      else
      {
        m_servdatNodesRunsFiles += nod->m_name+" 0 ";
      }
      RunMap::iterator k;
      for (k = nod->m_runmap.begin();k!=nod->m_runmap.end();k++)
      {
        sprintf(nfile,"%d/%d,",(*k).first,(*k).second);
        m_servdatNodesRunsFiles += nfile;
      }
      m_servdatNodesRunsFiles += "|";
    }
    else
    {
      if (nod->m_ROC_state == 'Y')
      {
        m_servdatNodesRunsFiles += nod->m_name+" 1 0/0,|";
      }
      else
      {
        m_servdatNodesRunsFiles += nod->m_name+" 0 0/0,|";
      }
    }
  }
  m_servdatNodesRunsFiles += '\0';
  m_servdatDiff += '\0';
  m_NodeListDiff->setData((void*)m_servdatDiff.c_str(),m_servdatDiff.size());
  m_NodeListDiff->updateService();
  m_NodesRunsFiles->setData((void*)m_servdatNodesRunsFiles.c_str(),m_servdatNodesRunsFiles.size());
  m_NodesRunsFiles->updateService();
  float stat[2];
  stat[0] = av_files;
  stat[1] = rms;
  m_StatServ->setData(stat,sizeof(stat));
  m_StatServ->updateService();
  m_recvNodes.clear();

  m_servdatNodesBuffersEvents.erase();
  for (NodeSet::iterator nit=m_BufferrecvNodes.begin();nit != m_BufferrecvNodes.end();nit++)
  {
    std::string nname;
    nname = *nit;
    myNodeMap::iterator nodeit = m_AllNodes.find(nname);
    myNode *nod = (*nodeit).second;
    char Line[1024];
    long dtime = nod->ReadTime-nod->ReadTime_prev;
    nod->Events.calcRate(nod->Events_prev,dtime);
    nod->Overflow.calcRate(nod->Overflow_prev,dtime);
    nod->Send.calcRate(nod->Send_prev,dtime);
    sprintf(Line,"%s %s/%d/%d/%0.4f/%0.4f,%s/%d/%d/%0.4f/%0.4f,%s/%d/%d/%0.4f/%0.4f|",nod->m_name.c_str(),
        nod->Events.name.c_str(),nod->Events.produced,nod->Events.seen,nod->Events.p_rate,nod->Events.s_rate,
        nod->Overflow.name.c_str(),nod->Overflow.produced,nod->Overflow.seen,nod->Overflow.p_rate,nod->Overflow.s_rate,
        nod->Send.name.c_str(),nod->Send.produced,nod->Send.seen,nod->Send.p_rate,nod->Send.s_rate);
    m_servdatNodesBuffersEvents += Line;
    nod->ReadTime_prev = nod->ReadTime;
    nod->Events_prev = nod->Events;
    nod->Overflow_prev = nod->Overflow;
    nod->Send_prev = nod->Send;
  }
  m_servdatNodesBuffersEvents += '\0';

  m_NodesBuffersEvents->setData((void*)m_servdatNodesBuffersEvents.c_str(),m_servdatNodesBuffersEvents.size());
  m_NodesBuffersEvents->updateService();
  BufferDump();
  m_BufferrecvNodes.clear();
  dim_unlock();
}
void HLTFileEqualizer::BufferDump()
{
  fprintf(outf,"Dump of Events Buffer Distribution at ");
  {
    time_t rawtime;
    time(&rawtime);
    fprintf(outf,"%s\n",asctime(localtime(&rawtime)));
  }
  fprintf(outf,"          ");
  for (int i=1;i<=32;i++)
  {
    fprintf(outf,"  %02i  ",i);

  }
  std::string cfarm="";
  std::string eline(256,' ');
  std::string line=eline;
  for (NodeSet::iterator nit=m_BufferrecvNodes.begin();nit != m_BufferrecvNodes.end();nit++)
  {
    std::string nname;
    nname = *nit;
    myNodeMap::iterator nodeit = m_AllNodes.find(nname);
    myNode *nod = (*nodeit).second;
    if (cfarm != nod->m_subfarm)
    {
      if (cfarm.size()>2)
      {
        fprintf(outf,"\n%s",line.substr(0,line.find_last_not_of(" ")+1).c_str());
      }
      line = eline;
      line.replace(0,nod->m_subfarm.size(),nod->m_subfarm);
//      fprintf(outf,"\n%s",nod->m_subfarm);
      cfarm = nod->m_subfarm;
    }
    int indx;
    sscanf(nod->m_name.substr(6,2).c_str(),"%d",&indx);
    char nfil[10];
    sprintf(nfil,"%4.3f",nod->Events.p_rate);
    line.replace(9+(indx-1)*6,5,nfil);
  }
  fprintf(outf,"\n%s\n",line.substr(0,line.find_last_not_of(" ")+1).c_str());
  fprintf(outf,"          ");
  for (int i=1;i<=32;i++)
  {
    fprintf(outf,"  %02i  ",i);

  }
  fprintf(outf,"\n");
  fflush(outf);
}
DefHltInfoHandler::DefHltInfoHandler(HLTFileEqualizer *e)
{
  m_Equalizer = e;
//      m_subfarm = sf;
}

MBMInfoHandler::MBMInfoHandler(HLTFileEqualizer *e)
{
  m_Equalizer = e;
}
using namespace ROMon;
typedef Nodeset::Nodes               Nodes;
typedef Node::Buffers                Buffers;

void MBMInfoHandler::infoHandler()
{
  int siz;
//  int status;
  siz = this->itsService->getSize();
//      gettimeofday()
  if (siz == sizeof(int)) return;
  m_sfstatus = (_MBMSF *)itsService->getData();
  const _MBMSF *ns = m_sfstatus;
  Nodes::const_iterator n;
    for (n=ns->nodes.begin();n!=ns->nodes.end(); n=ns->nodes.next(n))
    {
      std::string nname = (*n).name;
      m_Equalizer->m_BufferrecvNodes.insert(nname);
      myNodeMap::iterator anit;
      anit = m_Equalizer->m_AllNodes.find(nname);
      myNode* nod;
      if (anit == m_Equalizer->m_AllNodes.end())
      {
        nod = new myNode(nname);
        m_Equalizer->m_AllNodes.insert(std::make_pair(nname,nod));
      }
      else
      {
        nod = (*anit).second;
      }
//      nod->ReadTime_prev = nod->ReadTime;
      nod->ReadTime = (*n).time;
      nod->ReadTime *= 1000;
      nod->ReadTime += (*n).millitm;
      const Buffers& buffs = *(*n).buffers();
      for(Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))
      {
        const Buffers::value_type::Control& c = (*ib).ctrl;
        std::string bnam = (*ib).name;
        if (bnam == std::string("Events"))
        {
//          nod->Events_prev = nod->Events;
          nod->Events.name = bnam;
          nod->Events.produced = c.tot_produced;
          nod->Events.seen  = c.tot_seen;
        }
        else if (bnam == std::string("Overflow"))
        {
//          nod->Overflow_prev = nod->Overflow;
          nod->Overflow.name = bnam;
          nod->Overflow.produced = c.tot_produced;
          nod->Overflow.seen  = c.tot_seen;
        }
        else if (bnam == std::string("Send"))
        {
//          nod->Send_prev = nod->Send;
          nod->Send.name = bnam;
          nod->Send.produced = c.tot_produced;
          nod->Send.seen  = c.tot_seen;
        }
      }
    }
}

void DefHltInfoHandler::infoHandler()
{
  typedef _DHLTSF::Nodes               _N;
  typedef _DHLTSF::Node::Runs          _R;
  int siz;
//  int status;
  siz = this->itsService->getSize();
//      gettimeofday()
  if (siz == sizeof(int)) return;
  m_sfstatus = (_DHLTSF*)this->itsService->getData();
  const _DHLTSF* stats = m_sfstatus;
  const _N& nodes = *(stats->nodes());
//      _N::const_iterator i;
  for (_N::const_iterator i = nodes.begin();i!= nodes.end();i=nodes.next(i))
  {
    const _R& runs = (*i).runs;
    std::string nname;
    nname = (*i).name;
    toLowerCase(nname);
    myNodeMap::iterator nit;
    myNodeMap::iterator anit;
    m_Equalizer->m_recvNodes.insert(nname);
    anit = m_Equalizer->m_AllNodes.find(nname);
    myNode* nod;
    if (anit == m_Equalizer->m_AllNodes.end())
    {
      nod = new myNode(nname);
      m_Equalizer->m_AllNodes.insert(std::make_pair(nname,nod));
    }
    else
    {
      nod = (*anit).second;
    }
    NodeSet::iterator en = m_Equalizer->m_exclNodes.find(nod->m_name);
    nod->m_excl = (en != m_Equalizer->m_exclNodes.end());
    nod->m_runmap.clear();
//    fprintf(outf,"%s: ",(*i).name);
    int nfiles=0;
    for (_R::const_iterator j = runs.begin();j!= runs.end();j=runs.next(j))
    {
      nfiles += (*j).second;
      nod->m_runmap[(*j).first] = (*j).second;
    }
//    fprintf(outf,"%d Files\n",nfiles);
    if (nfiles>0)
    {
      nit = m_Equalizer->m_Nodes.find(nname);
      if (nit == m_Equalizer->m_Nodes.end())
      {
        anit = m_Equalizer->m_AllNodes.find(nname);
        myNode* nod;
        if (anit == m_Equalizer->m_AllNodes.end())
        {
          nod = new myNode(nname);
          m_Equalizer->m_AllNodes.insert(std::make_pair(nname,nod));
          m_Equalizer->m_Nodes.insert(std::make_pair(nname,nod));
        }
        else
        {
          nod = (*anit).second;
          m_Equalizer->m_Nodes.insert(std::make_pair(nname,nod));
        }
      }
      m_Equalizer->m_Nodes[nname]->m_nofiles = nfiles;
      m_Equalizer->m_Nodes[nname]->m_ROC_state = (*i).overflowState;
//      m_Equalizer->m_Nodes[nname]->m_state *= (*i).overflowState;
      m_Equalizer->m_nfiles += nfiles;
      m_Equalizer->m_nfiles2 += nfiles*nfiles;
      m_Equalizer->m_nnodes++;
    }
    else
    {
      m_Equalizer->m_AllNodes[nname]->m_nofiles = nfiles;
      nit = m_Equalizer->m_Nodes.find(nname);
      if (nit != m_Equalizer->m_Nodes.end())
      {
//        myNode* nod;
//        nod = (*nit).second;
        m_Equalizer->m_Nodes.erase(nit);
      }
    }
  }
//  status =1;
  return;
}




void HLTFileEqualizer::Dump()
{
  fprintf(outf,"Dump of File Distribution at ");
  {
    time_t rawtime;
    time(&rawtime);
    fprintf(outf,"%s\n",asctime(localtime(&rawtime)));
  }
  fprintf(outf,"          ");
  for (int i=1;i<=32;i++)
  {
    fprintf(outf," %02i  ",i);

  }
//  fprintf(outf,"\n");
  std::string cfarm="";
  std::string eline(256,' ');
  std::string line=eline;
//  int maxidx = -1;
  for (myNodeMap::iterator nit=m_Nodes.begin();nit != m_Nodes.end();nit++)
  {
    myNode *nod = (*nit).second;
    if (cfarm != nod->m_subfarm)
    {
      if (cfarm.size()>2)
      {
        fprintf(outf,"\n%s",line.substr(0,line.find_last_not_of(" ")+1).c_str());
      }
      line = eline;
      line.replace(0,nod->m_subfarm.size(),nod->m_subfarm);
//      fprintf(outf,"\n%s",nod->m_subfarm);
      cfarm = nod->m_subfarm;
    }
    int indx;
    sscanf(nod->m_name.substr(6,2).c_str(),"%d",&indx);
    char nfil[10];
    if (nod->m_ROC_state == 'Y')
    {
      sprintf(nfil,"%4d",nod->m_nofiles);
      line.replace(9+(indx-1)*5,4,nfil);
    }
    else
    {
      sprintf(nfil,"%4d*",nod->m_nofiles);
      line.replace(9+(indx-1)*5,5,nfil);
    }
  }
  fprintf(outf,"\n%s\n",line.c_str());
  fflush(outf);
};






class ExitCommand : public DimCommand
{
public:
  myNodeMap *m_nodemap;
  HLTFileEqualizer *m_equl;
  ExitCommand(const char *name, char *format, myNodeMap *nodm,HLTFileEqualizer *elz):  DimCommand(name, format)
  {
    m_nodemap = nodm;
    m_equl = elz;
  }
  void ableAll(int StateValue)
  {
  myActionMap Actions;
  myActionMap::iterator fit;
  myNodeMap::iterator nit;
  for (nit = m_nodemap->begin();nit != m_nodemap->end();nit++)
  {
    Actions[(*nit).second->m_subfarm].push_back(std::make_pair((*nit).first,StateValue));
  }
  for (fit = Actions.begin();fit!=Actions.end();fit++)
  {
    std::list<std::pair<std::string,int> >::iterator i;
    std::string sf_mesg = "";
    std::string endisSvc;
    endisSvc = (*fit).first+"_HLTDefBridge/EnDisCommand";
    for (i =(*fit).second.begin();i != (*fit).second.end();i++)
    {
      std::string svcname;
      std::string node = (*i).first;
      toUpperCase(node);
      svcname = node+"_MEPRx_01/setOverflow";
//      DimClient::sendCommand(svcname.c_str(), (*i).second);
      char cmd[1024];
//      sprintf(cmd,"dim_send_command.exe %s %d -dns %s -s -i&",svcname.c_str(),(*i).second,(*fit).first.c_str());
      sprintf(cmd,"%s %d|",svcname.c_str(),StateValue);
      sf_mesg.append(cmd);

//      ::system(cmd);
//      fprintf(outf,"\tMEPRX on Node %s (%s) value %d\n",(*i).first.c_str(),svcname.c_str(),(*i).second);
    }
    DimClient::sendCommandNB(endisSvc.c_str(),(void*)(sf_mesg.c_str()),sf_mesg.size());
//    printf("message to Subfarm %s:\n%s\n",(*fit).first.c_str(),sf_mesg.c_str());
  }
}

  void enableAll()
  {
    ableAll(1);
  }
  void disableAll()
  {
    ableAll(0);
  }
  virtual void commandHandler()
  {
    int command = getInt();
    switch (command)
    {
      case 0:
      {
        ableAll(0);
        break;
      }
      case 1:
      {
        ableAll(1);
        break;
      }
      case 2:
      {
        ableAll(1);
        ::sleep(5);
        ::exit(0);
      }
      case 3:
      {
        this->m_equl->Dump();
        break;
      }

    }
  }
};

LHCb1RunStatus::LHCb1RunStatus(char *name, int nolink,HLTFileEqualizer *e) : DimInfo(name,nolink)
{
  m_nolink = nolink;
  m_equalizer = e;
};

void LHCb1RunStatus::infoHandler()
{
#define READY 1
  int data;
  data = getInt();
  if ((m_state == 0) && (data == READY))
  {
    m_equalizer->Dump();
  }
  m_state = data;
};

ExclInfo::ExclInfo(char *name, NodeSet *nodeset) : DimInfo(name,(char*) "\0")
{
  this->m_exclNodes = nodeset;
}

void ExclInfo::infoHandler()
{
  char *input;
  input = getString();
  dyn_string *nlist = Strsplit(input,"\0");
  m_exclNodes->clear();
  for (size_t i=0;i<nlist->size();i++)
  {
    if (nlist->at(i).size()>0)
    {
      toLowerCase(nlist->at(i));
      m_exclNodes->insert(nlist->at(i));
    }
  }
}

int main(int argc, char **argv)
{
  char *ofile = getenv("HLTEQ_LOGF");
  if (ofile == 0)
  {
    outf = fopen("/group/online/HLTFileEqualizer.log","a+");
  }
  else
  {
    outf = fopen(ofile,"a+");

  }
  fprintf(outf,"HLTFileEqualizer starting at...");
  {
    time_t rawtime;
    time(&rawtime);
    fprintf(outf,"%s",asctime(localtime(&rawtime)));
  }

  DimClient::setDnsNode("ecs03");
  char *disact=getenv("HLTEQ_INHIBIT_ACT");
  inhibit_act = (disact != 0);
  char *dns = getenv("HLTEQ_DNSNODE");
  if (dns == 0)
  {
    DimServer::setDnsNode("ecs03");
  }
  else
  {
    DimServer::setDnsNode(dns);
  }
  HLTFileEqualizer elz;
  int m_DefState = -1;
  DimServer::start("HLTFileEqualizer");
  DimServer::autoStartOn();
  int low,high;
  if (argc == 3)
  {
    sscanf(argv[1],"%d",&low);
    sscanf(argv[2],"%d",&high);
    elz.m_low = low;
    elz.m_high = high;
  }
  ExclInfo exclInfo((char*)"HLT/ExcludedNodes",&elz.m_exclNodes);
  DimInfo *defstate=new DimInfo("RunInfo/LHCb/DeferHLT",m_DefState);
  elz.m_DefStateInfo = defstate;
  ExitCommand EnableandExit("HLTFileEqualizer/EnableAndExit",(char*)"I",&elz.m_AllNodes,&elz);
  LHCb1RunStatus LHCb1runstatus((char*)"RunInfo/LHCb1/RunStatus",-1,&elz);
  DimService *m_NodeService = new DimService("HLTFileEqualizer/NodeList", "C",(void*)"\0",1);
  elz.m_NodeList= m_NodeService;
  DimService *m_NodeServiceDiff = new DimService("HLTFileEqualizer/NodeListDiff", "C",(void*)"\0",1);
  elz.m_NodeListDiff = m_NodeServiceDiff;
  DimService *m_NodesRunsFiles= new DimService("HLTFileEqualizer/NodesRunsFiles", "C",(void*)"\0",1);
  elz.m_NodesRunsFiles = m_NodesRunsFiles;
  DimService *m_NodesBuffersEvents = new DimService("HLTFileEqualizer/NodesBuffersEvents", "C",(void*)"\0",1);
  elz.m_NodesBuffersEvents = m_NodesBuffersEvents;
  float stat[2];
  stat[0] = -1.0;
  stat[1] = 0.0;
  DimService *m_Statistics = new DimService("HLTFileEqualizer/Statistics", "F",stat,sizeof(stat));
  elz.m_StatServ = m_Statistics;
  fflush(outf);
  while (1)
  {
    sleep (60);
    elz.Analyze();
  }
  return 0;
}


