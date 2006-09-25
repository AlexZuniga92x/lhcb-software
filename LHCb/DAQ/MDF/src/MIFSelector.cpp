// $Id: MIFSelector.cpp,v 1.1 2006-09-25 12:32:27 frankb Exp $
//====================================================================
//	MIFSelector.cpp
//--------------------------------------------------------------------
//
//	Package    : MIF
//
//	Author     : M.Frank
//====================================================================

// Include files
#include "GaudiKernel/MsgStream.h"
#include "MDF/MIFHeader.h"
#include "MDF/MDFIO.h"
#include "MDF/RawEventHelpers.h"
#include "MDF/RawDataSelector.h"
#include <map>

/*
 *  LHCb namespace declaration
 */
namespace LHCb  {
  /** @class MIFSelector
    */
  class MIFSelector : public RawDataSelector  {
  public:
    /// Create a new event loop context
    /** @param refpCtxt   [IN/OUT]  Reference to pointer to store the context
      * 
      * @return StatusCode indicating success or failure
      */
    virtual StatusCode createContext(Context*& refpCtxt) const;

    /// Service Constructor
    MIFSelector( const std::string& nam, ISvcLocator* svcloc )
    : RawDataSelector( nam, svcloc)   {     }

    /// Standard destructor
    virtual ~MIFSelector()  {}
  };

  /** @class MIFContext
    *
    *  @author  M.Frank
    *  @version 1.0
    */
  class MIFContext : public RawDataSelector::LoopContext, protected MDFIO {
    typedef std::vector<RawBank*> Banks;
    Banks        m_banks;
    long long    m_fileOffset;
    StreamBuffer m_data;
    size_t       m_dataLen;
    MIFHeader*   m_header;
    struct MDFMapEntry  {
      std::string                name;
      StreamDescriptor::Access   bind;
      StreamDescriptor::Access   con;
    };
    const MIFSelector* m_mifSel;
    typedef std::map<int,MDFMapEntry*> FidMap;
    FidMap m_fidMap;
    int    m_mifFID;
    IMessageSvc* m_msg;
  public:
    /// Standard constructor
    MIFContext(const MIFSelector* pSel) 
      : RawDataSelector::LoopContext(pSel), 
        MDFIO(MDFIO::MDF_RECORDS,pSel->name()), 
        m_fileOffset(0), m_dataLen(0), m_mifSel(pSel), m_mifFID(0)
    { 
      m_header = (MIFHeader*)new char[1024];
      m_msg = pSel->messageService();
    }
    /// Standard destructor 
    virtual ~MIFContext();
    std::pair<char*,int> getDataSpace(void* const /* ioDesc */, size_t len)  {
      m_data.reserve(len);
      return std::pair<char*,int>(m_data.data(), m_data.size());
    }
    virtual StatusCode connect(const std::string& spec);
    /// Receive event and update communication structure
    virtual StatusCode receiveData(IMessageSvc* msg);
    /// Read raw byte buffer from input stream
    virtual StatusCode readBuffer(void* const ioDesc, void* const data, size_t len);
    long long offset()  const                       { return m_fileOffset;            }
    /// Raw data buffer (if it exists)
    virtual const void* data() const                { return m_data.data();           }
    /// Raw data buffer length (if it exists)
    virtual const size_t dataLength() const         { return m_dataLen;               }
    /// Access to RawBank array
    virtual const Banks& banks()  const             { return m_banks;                 }
    /// Access to RawBank array (NON const)
    Banks& banks()                                  { return m_banks;                 }
  };

}

/// Create a new event loop context
StatusCode LHCb::MIFSelector::createContext(Context*& refpCtxt) const {
  refpCtxt = new MIFContext(this);
  return StatusCode::SUCCESS;
}

/// Standard destructor 
LHCb::MIFContext::~MIFContext()  { 
  delete [] (char*)m_header;
  for(FidMap::iterator i=m_fidMap.begin(); i!=m_fidMap.end();++i)  {
    MDFMapEntry* ent = (*i).second;
    DSC::close(ent->con);
    DSC::close(ent->bind);
    delete ent;
    (*i).second = 0;
  }
  m_fidMap.clear();
}

/// Read raw byte buffer from input stream
StatusCode 
LHCb::MIFContext::readBuffer(void* const ioDesc, void* const data, size_t len)  
{
  DSC::Access* ent = (DSC::Access*)ioDesc;
  if ( ent && ent->ioDesc > 0 ) {
    if ( StreamDescriptor::read(*ent,data,len) )  {
      m_dataLen = len;
      return StatusCode::SUCCESS;
    }
  }
  m_dataLen = 0;
  return StatusCode::FAILURE;
}

StatusCode LHCb::MIFContext::connect(const std::string& spec)  {
  int fid = LHCb::genChecksum(1,spec.c_str(),spec.length()+1);
  FidMap::iterator i = m_fidMap.find(fid);
  if ( i == m_fidMap.end() )  {
    StatusCode sc = RawDataSelector::LoopContext::connect(spec);
    if ( sc.isSuccess() )  {
      MDFMapEntry* ent = new MDFMapEntry;
      ent->name = spec;
      ent->bind = m_bindDsc;
      ent->con  = m_accessDsc;
      m_fidMap.insert(std::make_pair(fid,ent));
      m_mifFID = fid;
      return sc;
    }
    MsgStream err(m_msg,"MIFSelector");
    err << MSG::ERROR << "Failed to connect to:" << spec << endmsg;
    return sc;
  }
  return StatusCode::SUCCESS;
}

/// Receive event and update communication structure
StatusCode LHCb::MIFContext::receiveData(IMessageSvc* msg)  {
  char buff[1024];
  MDFMapEntry* ent = 0;
  MIFHeader *hdr = (MIFHeader*)buff;
  FidMap::iterator i = m_fidMap.find(m_mifFID);
  setupMDFIO(msg,0);
  m_banks.clear();
  if ( i != m_fidMap.end() )  {
    MDFMapEntry* ent = (*i).second;
Next:
    if ( StreamDescriptor::read(ent->con,hdr,sizeof(int)+2*sizeof(char)) )  {
      if ( StreamDescriptor::read(ent->con,buff+sizeof(int)+2*sizeof(char),hdr->size()) )  {
        FidMap::iterator i = m_fidMap.find(hdr->fid());
        if ( i == m_fidMap.end() )  {
          if ( hdr->type() == MIFHeader::MIF_FID )  {
            int id = m_mifFID;  // need to save MIF FID - will be overwritten in connect()
            StatusCode sc = connect(hdr->data<char>());
            m_mifFID = id;
            if ( !sc.isSuccess() )  {
              return sc;
            }
            goto Next;
          }
          else  {
            MsgStream err(msg,"MIFSelector");
            err << MSG::ERROR << "Unknown input specification:"
                << std::hex << std::setw(8) << hdr->fid() << endmsg;
            return StatusCode::FAILURE;
          }
        }
        ent = (*i).second;
      }
      m_fileOffset = StreamDescriptor::seek(ent->con,0,SEEK_CUR);
      std::pair<char*,int> res = readBanks(&ent->con, 0 == m_fileOffset);
      if ( res.second > 0 )  {
        return decodeRawBanks(res.first,res.first+res.second,m_banks);
      }
    }
  }
  return StatusCode::FAILURE;
}

#include "GaudiKernel/DeclareFactoryEntries.h"
DECLARE_NAMESPACE_SERVICE_FACTORY(LHCb,MIFSelector);
