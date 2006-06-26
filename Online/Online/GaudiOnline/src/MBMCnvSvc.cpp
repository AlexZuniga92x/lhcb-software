// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/src/MBMCnvSvc.cpp,v 1.6 2006-06-26 08:45:15 frankb Exp $
//	====================================================================
//  RawBufferCreator.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiOnline/MBMCnvSvc.h"
#include "MDF/RawEventHelpers.h"
#include "MDF/StorageTypes.h"
#include "MDF/MDFHeader.h"
#include "MBM/Producer.h"
#include "MBM/mepdef.h"
#include "Event/RawBank.h"

using MBM::Producer;
using MBM::EventDesc;

DECLARE_NAMESPACE_SERVICE_FACTORY(LHCb,MBMCnvSvc)

namespace  {
  Producer* producerFromIODescriptor(void* ioDesc)   {
    return (Producer*)ioDesc;
  }
}

/// Initializing constructor
LHCb::MBMCnvSvc::MBMCnvSvc(const std::string& nam, ISvcLocator* loc) 
: RawDataCnvSvc(nam, loc, RAWDATA_StorageType)
{
  m_genChecksum = 0;
  m_compress = 0;
}

/// Initializing constructor for inheritance
LHCb::MBMCnvSvc::MBMCnvSvc(const std::string& nam, ISvcLocator* loc, long type)
: RawDataCnvSvc(nam, loc, type)
{
  m_genChecksum = 0;
  m_compress = 0;
}

/// Standard destructor
LHCb::MBMCnvSvc::~MBMCnvSvc() {
}

/// Allocate data space for output
std::pair<char*,int> LHCb::MBMCnvSvc::getDataSpace(void* const ioDesc, size_t len)   {
  if ( ioDesc )   {
    Producer* prod = producerFromIODescriptor(ioDesc);
    if ( prod )  {
      if ( prod->getSpace(len) == MBM_NORMAL )  {
        EventDesc& e = prod->event();
        return std::pair<char*,int>((char*)e.data, e.len);
      }
    }
  }
  return std::pair<char*,int>(0,0);
}

/// Declare event to data space
StatusCode 
LHCb::MBMCnvSvc::writeBuffer(void* const ioDesc, const void* data, size_t len)    {
  if ( ioDesc )   {
    Producer* prod = producerFromIODescriptor(ioDesc);
    if ( prod )  {
      RawBank* hdrBank = (RawBank*)data;
      MDFHeader* h  = (MDFHeader*)hdrBank->data();
      MDFHeader::SubHeader sh = h->subHeader();
      const unsigned int *trMask = sh.H1->triggerMask();
      EventDesc& e = prod->event();
      e.type    = EVENT_TYPE_EVENT;
      e.mask[0] = trMask[0];
      e.mask[1] = trMask[1];
      e.mask[2] = trMask[2];
      e.mask[3] = trMask[3];
      e.len     = len;
      if ( prod->sendEvent() == MBM_NORMAL )  {
        return StatusCode::SUCCESS;
      }
    }
  }
  return StatusCode::FAILURE;
}

/// Open MBM buffer manager
void* LHCb::MBMCnvSvc::openIO(const std::string& fname, const std::string&  mode) const {
  if ( strncasecmp(mode.c_str(),"N",1)==0 || strncasecmp(mode.c_str(),"REC",3)==0 )  {
    // Writing: requires producer
    if ( fname.find("mbm://") == 0 )  {
      int partID;
      size_t id1 = fname.find(".",6);
      size_t id2 = fname.find(".0x",id1+1);
      std::string buff = fname.substr(6, id1-6);
      std::string proc = fname.substr(id1+1, id2-id1-1);
      ::sscanf(fname.c_str()+id2+1,"0x%X",&partID);
      Producer* p = new Producer(buff,proc,partID);
      return p;
    }
  }
  else if ( strncasecmp(mode.c_str(),"O",1)==0 || strncasecmp(mode.c_str(),"REA",3)==0 )  {
    // Reading: requires consumer
  }
  return 0;
}

/// Close MBM buffer manager
StatusCode LHCb::MBMCnvSvc::closeIO(void* ioDesc) const   {
  if ( ioDesc )   {
    Producer* prod = producerFromIODescriptor(ioDesc);
    if ( prod )  {
      delete prod;
      return StatusCode::SUCCESS;
    }
  }
  return StatusCode::FAILURE;
}

/// Read raw banks
StatusCode LHCb::MBMCnvSvc::readRawBanks(void* , std::vector<RawBank*>& , long long& )  {
  return StatusCode::FAILURE;
}
