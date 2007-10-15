#uses "ctrlUtils.cpp"
#uses "fwDIM"
#uses "fwFsmTreeDisplay.ctl"
#uses "StreamControlPanel.cpp"

//=============================================================================
int StreamTaskMgr_install()  {
  dyn_dyn_string names;
  dyn_dyn_int types;
  names[1]  = makeDynString ("FSM_DimTask","","","");
  names[2]  = makeDynString ("","Name","","");
  names[3]  = makeDynString ("","Type","","");
  names[4]  = makeDynString ("","Node","","");
  names[5]  = makeDynString ("","Command","","");
  names[6]  = makeDynString ("","State","","");
  names[7]  = makeDynString ("","Messages","","");
  names[8]  = makeDynString ("","","severity","");
  names[9]  = makeDynString ("","","msg","");
  names[10] = makeDynString ("","","source","");
  names[11] = makeDynString ("","Class","","");
  names[12] = makeDynString ("","InUse","","");
  names[13] = makeDynString ("","Priority","","");
  names[14] = makeDynString ("","FMC_Start","","");
  types[1]  = makeDynInt (DPEL_STRUCT,0,0,0);
  types[2]  = makeDynInt (0,DPEL_STRING,0,0);
  types[3]  = makeDynInt (0,DPEL_STRING,0,0);
  types[4]  = makeDynInt (0,DPEL_STRING,0,0);
  types[5]  = makeDynInt (0,DPEL_STRING,0,0);
  types[6]  = makeDynInt (0,DPEL_STRING,0,0);
  types[7]  = makeDynInt (0,DPEL_STRUCT,0,0);
  types[8]  = makeDynInt (0,0,DPEL_INT,0);
  types[9]  = makeDynInt (0,0,DPEL_STRING,0);
  types[10] = makeDynInt (0,0,DPEL_STRING,0);
  types[11] = makeDynInt (0,DPEL_INT,0,0);
  types[12] = makeDynInt (0,DPEL_INT,0,0);
  types[13] = makeDynInt (0,DPEL_INT,0,0);
  types[14] = makeDynInt (0,DPEL_STRING,0,0);
  ctrlUtils_installDataType(names,types);

  names[1]  = makeDynString ("StreamTaskCreator","","","");
  names[2]  = makeDynString ("","Name","","");
  names[3]  = makeDynString ("","Start","","");
  names[4]  = makeDynString ("","Stop","","");
  names[5]  = makeDynString ("","Kill","","");
  types[1]  = makeDynInt (DPEL_STRUCT,0,0,0);
  types[2]  = makeDynInt (0,DPEL_STRING,0,0);
  types[3]  = makeDynInt (0,DPEL_STRING,0,0);
  types[4]  = makeDynInt (0,DPEL_STRING,0,0);
  types[5]  = makeDynInt (0,DPEL_STRING,0,0);
  ctrlUtils_installDataType(names,types);

  names[1]  = makeDynString ("StreamControl","","","");
  names[2]  = makeDynString ("","ActivePartitions","","");
  names[3]  = makeDynString ("","Command","","");
  names[4]  = makeDynString ("","InUse","","");
  names[5]  = makeDynString ("","PartitionDesc","","");
  names[6]  = makeDynString ("","RecvNodes","","");
  names[7]  = makeDynString ("","RecvSlices","","");
  names[8]  = makeDynString ("","State","","");
  names[9]  = makeDynString ("","StreamNodes","","");
  names[10] = makeDynString ("","StreamSlices","","");
  types[1]  = makeDynInt (DPEL_STRUCT,0,0,0);
  types[2]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[3]  = makeDynInt (0,DPEL_STRING,0,0);
  types[4]  = makeDynInt (0,DPEL_DYN_INT,0,0);
  types[5]  = makeDynInt (0,DPEL_DYN_DPID,0,0);
  types[6]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[7]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[8]  = makeDynInt (0,DPEL_STRING,0,0);
  types[9]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[10] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  ctrlUtils_installDataType(names,types);

  names[1]  = makeDynString ("StreamConfigurator","","","");
  names[2]  = makeDynString ("","Command","","");
  names[3]  = makeDynString ("","State","","");
  types[1]  = makeDynInt (DPEL_STRUCT,0,0,0);
  types[2]  = makeDynInt (0,DPEL_STRING,0,0);
  types[3]  = makeDynInt (0,DPEL_STRING,0,0);
  ctrlUtils_installDataType(names,types);

  names[1]  = makeDynString ("StreamPartition","","","");
  names[2]  = makeDynString ("","FSMSlice","","");
  names[3]  = makeDynString ("","InUse","","");
  names[4]  = makeDynString ("","Name","","");
  names[5]  = makeDynString ("","RunInfo","","");
  names[6]  = makeDynString ("","RecvNodes","","");
  names[7]  = makeDynString ("","RecvSlices","","");
  names[8]  = makeDynString ("","StreamNodes","","");
  names[9]  = makeDynString ("","StreamSlices","","");
  names[10] = makeDynString ("","DataSenders","","");
  names[11] = makeDynString ("","RecvInfrastructure","","");
  names[12] = makeDynString ("","RecvReceivers","","");
  names[13] = makeDynString ("","RecvSenders","","");
  names[14] = makeDynString ("","StreamInfrastructure","","");
  names[15] = makeDynString ("","StreamReceivers","","");
  names[16] = makeDynString ("","StreamSenders","","");
  types[1]  = makeDynInt (DPEL_STRUCT,0,0,0);
  types[2]  = makeDynInt (0,DPEL_STRING,0,0);
  types[3]  = makeDynInt (0,DPEL_INT,0,0);
  types[4]  = makeDynInt (0,DPEL_STRING,0,0);
  types[5]  = makeDynInt (0,DPEL_STRING,0,0);
  types[6]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[7]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[8]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[9]  = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[10] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[11] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[12] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[13] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[14] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[15] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  types[16] = makeDynInt (0,DPEL_DYN_STRING,0,0);
  ctrlUtils_installDataType(names,types);
}
//=============================================================================
int StreamTaskMgr_uninstall()  {
  ctrlUtils_uninstallDataType("FSM_DimTask",true);
  ctrlUtils_uninstallDataType("StreamControl",true);
  ctrlUtils_uninstallDataType("StreamPartition",true);
}
//=============================================================================
int StreamTaskMgr_connectTaskManager(string stream)  {
  dyn_string recvNodes, strmNodes;
  string cfg = "DimStorage";
  fwDim_createConfig(cfg);
  int res = dpGet(stream+"Alloc.RecvNodes",recvNodes,stream+"Alloc.StreamNodes",strmNodes);
  if ( 0 == res )  {
    dynAppend(recvNodes,strmNodes);
    for(int i=1; i<=dynlen(recvNodes); ++i)  {
      string name = strtoupper(recvNodes[i]);
      string dp_name = name+"_StreamTaskCreator";
      string svc_name = "/" + name + "/task_manager";
      fwDim_unSubscribeCommandsByDp(cfg,dp_name+"*");
      fwDim_unSubscribeServicesByDp(cfg,dp_name+"*");
      if ( !dpExists(dp_name) ) dpCreate(dp_name,"StreamTaskCreator");
      dpSet(dp_name+".Name",name);
      fwDim_subscribeCommand(cfg,svc_name+"/start",dp_name+".Start");
      fwDim_subscribeCommand(cfg,svc_name+"/stop",dp_name+".Stop");
      fwDim_subscribeCommand(cfg,svc_name+"/kill",dp_name+".Kill");
      DebugN("Connect "+svc_name+" to "+dp_name);
    }
    DebugN("All Done.");
  }
  ctrlUtils_checkErrors(res);
  return 1;
}
//=============================================================================
string StreamTaskMgr_addTreeNode(string parent, string node, string type, int isCU) {
  if ( 0 == fwFsmTree_isNode(node) ) {
    DebugN("Create Node:"+parent+"::"+node+" ["+type+"]");
    return fwFsmTree_addNode(parent, node, type, isCU);
  }
  DebugN("Node "+parent+"::"+node+" Exists already....");
  return node;
}
//=============================================================================
int StreamTaskMgr_installStream(string stream)  {
    string stream_node = StreamTaskMgr_addTreeNode("FSM", "Data"+stream, "StreamCluster", 1);
    string stream_alloc = stream+"Alloc";
    if ( !dpExists(stream_alloc) )  {
      dpCreate(stream_alloc,"StreamControl");
    }
    string pref = stream;
    string st = StreamTaskMgr_addTreeNode(stream_node,stream_alloc,"StreamControl",0);
    fwFsmTree_refreshTree();
}
//=============================================================================
int StreamTaskMgr_uninstallStream(string stream)  {
  DebugN("Delete tree:"+"Data"+stream);
  fwFsmTree_removeNode("FSM","Data"+stream,1);
  fwFsmTree_refreshTree();
  DebugN("All Done.");
  return 1;
}
//=============================================================================
int StreamTaskMgr_generateFSM(string node)   {
  DebugN("Generate FSM:"+node);
  fwFsmTree_generateTreeNode(node);
  DebugN("All Done.");
  return 1;
}
//=============================================================================
int StreamTaskMgr_startFSMNode(string node)   {
  DebugN("Start Tree:"+node);
  fwFsmTree_startTreeNode(node);
  DebugN("All Done.");
  return 1;
}
//=============================================================================
int StreamTaskMgr_stopFSMNode(string node)   {
  DebugN("Start Tree:"+node);
  fwFsmTree_stopTreeNode(node);
  DebugN("All Done.");
  return 1;
}
//=============================================================================
int StreamTaskMgr_genStreamFSM(string stream)   
{ return StreamTaskMgr_generateFSM(stream+"Alloc");   }
//=============================================================================
int StreamTaskMgr_startStreamTree(string stream)  
{ return StreamTaskMgr_startFSMNode(stream+"Alloc");  }
//=============================================================================
int StreamTaskMgr_stopStreamTree(string stream)  
{ return StreamTaskMgr_stopFSMNode(stream+"Alloc");  }
//=============================================================================
int StreamTaskMgr_createNodeTasks(string node, string name, int howmany, int first) {
  string dev_name;
  for(int j=first; j<howmany+first; j++)  {
    sprintf(dev_name,"%s_%03d",name,j);
    if ( !dpExists(dev_name) )  {
      dpCreate(dev_name,"FSM_DimTask");
    }
    dpSet(dev_name+".State","UNKNOWN");
    string task = StreamTaskMgr_addTreeNode(node, dev_name, "FSM_DimTask", 0);
  }
  return 1;
}
//=============================================================================
int StreamTaskMgr_createNode(string stream_node,string name,string type,int num)  {
  string node = StreamTaskMgr_addTreeNode(stream_node, name, type, 0);
  if ( num > 0 && node != "" )  {
    StreamTaskMgr_createNodeTasks(node,name,num,0);
  }
  return 1;
}
//=============================================================================
int StreamTaskMgr_createTree(string stream, 
			     string slice_name, 
			     int num_recvTasks,
			     int num_strmTasks,
			     string monitoringInput,
			     int num_monTasks,
			     int refresh=1)  
{
  dyn_string recvNodes, strmNodes, sendNodes;
  // Get stream configuration from the corresponding StreamControl datapoint
  int res = dpGet(stream+"Alloc.RecvNodes",recvNodes,stream+"Alloc.StreamNodes",strmNodes);
  DebugN(stream+"> Generating tree:"+slice_name+" "+num_recvTasks+" "+
         num_strmTasks+" Monitoring:"+monitoringInput+" "+num_monTasks);
  if ( num_monTasks>0 && 0 == res )   {
    res = dpGet(monitoringInput+"Alloc.RecvNodes",sendNodes);
  }
  if ( 0 == res )  {
    string name, node = stream+"_"+slice_name;
    string slice_node = StreamTaskMgr_addTreeNode("FSM", stream+"_Slices", "FSM_Holder", 1);
    string stream_node = StreamTaskMgr_addTreeNode(stream+"_Slices", node, "FSM_Slice", 1);
    DebugN("Slices:"+slice_node+" Stream:"+stream_node);
    if ( stream_node != "" )   {
      StreamTaskMgr_addTreeNode(stream_node, node+"_Config", "StreamConfigurator", 0);
      fwFsmTree_setNodeLabel(node+"_Config","Configurator");
      for(int i=1; i<=dynlen(sendNodes); ++i)  {
        name = node+"_"+sendNodes[i];
        StreamTaskMgr_createNode(stream_node,name,"FSM_Tasks",num_monTasks);
        fwFsmTree_setNodeLabel(name,sendNodes[i]);
      }
      for(int i=1; i<=dynlen(recvNodes); ++i)  {
        name = node+"_"+recvNodes[i];
        StreamTaskMgr_createNode(stream_node,name,"FSM_Tasks",num_recvTasks);
        fwFsmTree_setNodeLabel(name,recvNodes[i]);
      }
      for(int i=1; i<=dynlen(strmNodes); ++i)  {
        name = node+"_"+strmNodes[i];
        StreamTaskMgr_createNode(stream_node,name,"FSM_Tasks",num_strmTasks);
        fwFsmTree_setNodeLabel(name,strmNodes[i]);
      }
    }    
    if ( refresh ) fwFsmTree_refreshTree();
    DebugN("All Done.");
  }
  ctrlUtils_checkErrors(res);
  return 1;
}
//=============================================================================
int StreamTaskMgr_createAllTree(string stream,
                                int num_recvTasks,
                                int num_strmTasks,
                                string monitoringInput,
                                int num_monTasks)
{
  for(int i=0; i<16; ++i)  {    
    string node;
    sprintf(node,"Slice%02X",i);
    StreamTaskMgr_createTree(stream,node,
                             num_recvTasks,num_strmTasks,
                             monitoringInput,num_monTasks,0);
  }
  fwFsmTree_refreshTree();
  DebugN("All Done.");
}
//=============================================================================
int StreamTaskMgr_deleteTree(string stream, string slice_name, int refresh=1)  {
  string node = stream+"_"+slice_name;
  DebugN("Delete tree:"+node);
  fwFsmTree_removeNode(stream+"_Slices",node,1);
  dyn_string tasks = dpNames(node+"*","FSM_DimTask");
  for(int i=1, n=dynlen(tasks); i<=n; ++i)  {
    if ( dpExists(tasks[i]) ) dpDelete(tasks[i]);
  }
  if ( refresh ) fwFsmTree_refreshTree();
  DebugN("All Done...Deleted tree and "+dynlen(tasks)+" Tasks.");
}
//=============================================================================
int StreamTaskMgr_genFSM(string stream, string slice_name)
{ return StreamTaskMgr_generateFSM(stream+"_"+slice_name);   }
//=============================================================================
int StreamTaskMgr_startTree(string stream, string slice_name)  
{ return StreamTaskMgr_startFSMNode(stream+"_"+slice_name);  }
//=============================================================================
int StreamTaskMgr_stopTree(string stream, string slice_name)
{ return StreamTaskMgr_stopFSMNode(stream+"_"+slice_name);  }
