// $Id: PartitionListener.h,v 1.4 2009-05-05 18:35:31 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Base class for displays for readout monitoring 
//               in the LHCb experiment
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/ROMon/PartitionListener.h,v 1.4 2009-05-05 18:35:31 frankb Exp $
#ifndef ROMON_PARTITIONLISTENER_H
#define ROMON_PARTITIONLISTENER_H 1

// Framework includes
#include "CPP/Interactor.h"

// C++ include files
#include <string>

/*
 *   ROMon namespace declaration
 */
namespace ROMon {

  /**@class PartitionListener PartitionListener.h ROMon/PartitionListener.h
   *
   *   Listen to RunInfo datapoints published by DIM
   *
   *   @author M.Frank
   */
  class PartitionListener {

  protected:
    Interactor* m_parent;
    std::string m_name;
    int         m_subFarmDP;
    int         m_partIdDP;
    int         m_runStateDP;
  public:
    /// Standard constructor with object setup through parameters
    PartitionListener(Interactor* parent,const std::string& name);
    /// Standard constructor with object setup through parameters
    PartitionListener(Interactor* parent,const std::string& name,bool files_only);
    /// Standard destructor
    virtual ~PartitionListener();
    /// DIM command service callback to retrieve HLT subfarms
    static void subFarmHandler(void* tag, void* address, int* size);
    /// DIM command service callback to retrieve run state
    static void runStateHandler(void* tag, void* address, int* size);
    /// DIM command service callback to retrieve partition ID
    static void partIdHandler(void* tag, void* address, int* size);
  };
}      // End namespace ROMon
#endif /* ROMON_PARTITIONLISTENER_H */
