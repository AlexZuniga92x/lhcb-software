#uses "ctrlUtils.cpp"
#uses "StreamTaskMgr.cpp"
aaamain()   {
  string stream = "Monitoring";
  //StreamTaskMgr_installStream(stream);
  StreamTaskMgr_connectTaskManager(stream);
  //stream = "STORAGE:Storage";
  //StreamTaskMgr_connectTaskManager(stream);
}
main()   {
  int num_relay_tasks  = 45;
  int num_sender_tasks = 10;
  int num_worker_tasks = 20;
  string stream        = "Monitoring";
  string ctrl_node     = strtoupper(getHostname());
  string storage_sys   = "STORAGE:Storage";
  ctrlUtils_stopAllTree();
  if ( ctrl_node == "MONA09" ) {
    num_worker_tasks = 20;
    stream = "Reconstruction";
    StreamTaskMgr_installStream(stream);
    if ( ctrl_node == "MONA09" ) {
      for(int i=0; i<4; ++i) {
        string n;
        sprintf(n,"Slice%02X",i);
        StreamTaskMgr_createTree(stream,n,num_relay_tasks,num_worker_tasks,storage_sys,num_sender_tasks);
        fwFsmTree_setNodePanel(stream+"_"+n,"StreamControl/RecoEditor.pnl");
      }
    }
  }
  else {
    if ( ctrl_node == "MONA08" ) {
      num_worker_tasks = 20;
      num_relay_tasks  = 20;
    }
    StreamTaskMgr_installStream(stream);
    StreamTaskMgr_createAllTree(stream,
                                num_relay_tasks,
                                num_worker_tasks,
                                storage_sys,
                                num_sender_tasks);  
  }
  fwInstallation_addManager("WCCOAdim","always", 30, 3, 3, "-num 40 -dim_dp_config "+ctrlUtils_dimMapName()+" -dim_dns_node "+ctrl_node);
  StreamTaskMgr_connectTaskManager(storage_sys);
  StreamTaskMgr_connectTaskManager(stream);
  ctrlUtils_installPythonManager(59,"WCCOAMonitoring","../python/WCCOA_Monitoring.py");
  // Finally generate all FSMs
  ctrlUtils_genAllFSM();
  ctrlUtils_startAllTree();
  exit(0);
}
