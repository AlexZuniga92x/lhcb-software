// $Id: IOHandler.cpp,v 1.3 2010-03-03 14:30:47 frankb Exp $
//====================================================================
//
//  Package    : RootCnv
//
//  Description: Generate TClassStreamers for special Gaudi classes
//  @author      M.Frank
//====================================================================
#include "GaudiKernel/ObjectContainerBase.h"
#include "GaudiKernel/LinkManager.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartRef.h"
#include "GaudiKernel/System.h"
#include <stdexcept>
#include <iostream>
#include "TROOT.h"
#include "TClass.h"
#include "TSystem.h"
#include "TInterpreter.h"
#include "TClassStreamer.h"
#include <memory>

namespace GaudiRoot {
  static const DataObject* last_link_object = 0;
  static int               last_link_hint = -1;
  void resetLastLink() {
    last_link_object = 0;
    last_link_hint   = -1;
  }
  void pushCurrentDataObject(DataObject** pobjAddr) {
    Gaudi::pushCurrentDataObject(pobjAddr);
    resetLastLink();
  }
  void popCurrentDataObject() {
    Gaudi::popCurrentDataObject();
    resetLastLink();
  }

  union RefAccessor {
    void*                       Ptr;
    SmartRefBase*               Base;
    SmartRef<DataObject>*       ObjectRef;
    SmartRef<ContainedObject>*  ContainedRef;
    RefAccessor(void* obj) { Ptr = obj; }
  };

  /**@class IOHandler
   */
  template <class T> class IOHandler : public TClassStreamer {
  protected:
    /// ROOT persistent class description
    TClass*   m_root;
  public:
    /// Initializing constructor
    IOHandler(TClass* c) : m_root(c) {  }
    /// Standard destructor
    virtual ~IOHandler() {  }
    /// ROOT I/O callback
    virtual void operator()(TBuffer &b, void *obj)  {
      try {
	if ( b.IsReading() ) 
	  get(b,obj);
	else
	  put(b,obj);
      }
      catch( const std::exception& e )    {
	std::string err = "Class:" + std::string(m_root->GetName()) + "> Exception in object I/O";
	err += e.what();
	throw std::runtime_error(err);
      }
      catch( ... )    {
	std::string err = "Class:" + std::string(m_root->GetName()) + "> Exception in object I/O";
	throw std::runtime_error(err);
      }
    }
    /// Callback for reading the object
    virtual void get(TBuffer &b, void* obj);
    /// Callback for writing the object
    virtual void put(TBuffer &b, void* obj);
  };

  template <> void IOHandler<SmartRefBase>::get(TBuffer &b, void* obj) {
    RefAccessor r(obj);
    UInt_t start, count;
    Version_t version = b.ReadVersion(&start, &count, m_root);
    m_root->ReadBuffer(b, obj, version, start, count);
    switch( r.Base->objectType() ) {
    case SmartRefBase::DATAOBJECT:
      (*r.ObjectRef)(Gaudi::getCurrentDataObject());
      break;
    case SmartRefBase::CONTAINEDOBJECT:
      (*r.ContainedRef)(Gaudi::getCurrentDataObject());
      break;
    default:
      std::cout << "Hit uninitialized smartRef!!!!" << std::endl;
      break;
    }
  }

  template <> void IOHandler<SmartRefBase>::put(TBuffer &b, void* obj) {
    RefAccessor r(obj);
    ContainedObject* p;
    DataObject* curr, *pDO;
    int hint, link;

    curr = Gaudi::getCurrentDataObject();
    pDO  = r.ObjectRef->data();
    hint = r.ObjectRef->hintID();
    link = r.ObjectRef->linkID();
    if ( pDO )  {
      switch( r.Base->objectType() ) {
      case SmartRefBase::CONTAINEDOBJECT:
	p = r.ContainedRef->data();
	if ( p )  {
	  const ObjectContainerBase* parent = p->parent();
	  if ( parent )  {
	    link = p->index();
	    pDO  = const_cast<ObjectContainerBase*>(parent);
	    break;
	  }
	}
	pDO = 0;
	std::cout << "IOHandler<SmartRefBase>::onWrite> "
		  << "Found invalid smart reference with object "
		  << "having no parent."
		  << std::endl;
	throw std::runtime_error("IOHandler<SmartRefBase>::onWrite> "
				 "Found invalid smart reference with object "
				 "having no parent.");
	break;
      case SmartRefBase::DATAOBJECT:
	link = StreamBuffer::INVALID;
	break;
      default:
	break;
      }
      if ( pDO == last_link_object )  {
	r.Base->set(curr, last_link_hint, link);
	m_root->WriteBuffer(b, obj);
	return;
      }
      else if ( pDO ) {
	LinkManager* mgr = curr->linkMgr();
	IRegistry*   reg = pDO->registry();
	if ( reg && mgr )  {
	  hint = mgr->addLink(reg->identifier(), pDO);
	  last_link_hint   = hint;
	  last_link_object = pDO;
	}
      }
      else {
	hint = link = StreamBuffer::INVALID;
      }
    }
    r.Base->set(curr, hint, link);
    m_root->WriteBuffer(b, obj);
  }

  template <> void IOHandler<ContainedObject>::get(TBuffer &b, void* obj) {
    UInt_t start, count;
    Version_t version = b.ReadVersion(&start, &count, m_root);
    m_root->ReadBuffer(b, obj, version, start, count);
    ContainedObject* p = (ContainedObject*)obj;
    p->setParent((ObjectContainerBase*)Gaudi::getCurrentDataObject());
  }
  
  template <> void IOHandler<ContainedObject>::put(TBuffer &b, void* obj) {
    m_root->WriteBuffer(b, obj);
  }

  template <class T> static bool makeStreamer(MsgStream& log)  {
    std::string cl_name = System::typeinfoName(typeid(T));
    TClass* c = gROOT->GetClass(cl_name.c_str());
    if ( c ) {
      TClassStreamer* s = new IOHandler<T>(c);
      c->AdoptStreamer(s);
      log << MSG::DEBUG << "Installed IOHandler for class " << cl_name << endmsg;
      return true;
    }
    log << MSG::ERROR << "[No ROOT TClass] Failed to install IOHandler for class " << cl_name << endmsg;
    return false;
  }

  bool patchStreamers(MsgStream& s)  {
    static bool first = true;
    if ( first ) {
      first = false;
      gSystem->Load("libCintex");
      gROOT->ProcessLine("#include <vector>");
      gInterpreter->EnableAutoLoading();
      gInterpreter->AutoLoad("DataObject");
      gROOT->ProcessLine("Cintex::Cintex::Enable()");    

      bool b1 = makeStreamer<SmartRefBase>(s);
      bool b2 = makeStreamer<ContainedObject>(s);
      return b1 && b2;
    }
    return true;
  }
}
