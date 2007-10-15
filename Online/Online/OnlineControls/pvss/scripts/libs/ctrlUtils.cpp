#uses "fwDIM"
#uses "fwFsmTreeDisplay.ctl"
//=============================================================================
void ctrlUtils_trace(string msg)  {
  DebugN(msg);
}
//=============================================================================
void ctrlUtils_setListItems(string shape_name, dyn_string values)  {
  shape s = getShape(shape_name);
  s.deleteAllItems();
  for(int k=1; k<=dynlen(values); k++) 
    s.appendItem(values[k]);
}
//=============================================================================
dyn_string ctrlUtils_getStrings(string dp, string type)  {
  StreamControl_trace("ctrlUtils_getStrings:"+dp+" ["+type+"]");
  dyn_string items = dpNames(dp,type);
  dyn_string values, result;
  int res = dpGet(items,values);
  if ( ctrlUtils_checkErrors(res) )  {
    for(int i=1; i<=dynlen(values); i++) {
      if ( strlen(values[i])>0 ) dynAppend(result, values[i]);
    }
  }
  dynSortAsc(values);
  return result;
}
//=============================================================================
dyn_string ctrlUtils_getStringVectors(string dp,string type)  {
  dyn_dyn_string slots;
  dyn_string values, items = dpNames(dp,type);
  int res = dpGet(items,slots);
  if ( ctrlUtils_checkErrors(res) )  {
    for(int i=1; i<=dynlen(slots); i++) {
      for(int j=1; j<=dynlen(slots[i]); j++) {
        dynAppend(values,slots[i][j]);
      }
    }
  }
  dynSortAsc(values);
  return values;
}
//=============================================================================
bool ctrlUtils_checkErrors(int res, string msg="")  {
  if ( res != 0 || dynlen(getLastError()) > 0 )  {
    string err = " Err="+res+","+getLastError();
    DebugN(" Err="+res+","+getLastError());
    if ( strlen(msg)>0 ) DebugN(msg);
    ChildPanelOn("vision/MessageInfo1","Error occurred",makeDynString(err),50,50);
  }
  return res == 0;
}
//=============================================================================
int ctrlUtils_installDataType(dyn_dyn_string& names, dyn_dyn_int& types)  {
  string type=names[1][1];
  if ( dynlen(dpTypes(type)) ==0 ) {
    int result = dpTypeCreate(names,types);
    ctrlUtils_checkErrors(result,"Failed to install data type '"+type+"'");
    dynClear(names);
    dynClear(types);
    ctrlUtils_trace("Installation of data type '"+type+"' finished.");
    return 1;
  }
  dynClear(names);
  dynClear(types);
  ctrlUtils_trace("Data type '"+type+"' are already defined....Nothing to do.");
  return 0;
}
//=============================================================================
int ctrlUtils_uninstallDataType(string type,bool delete_datapoints=false)  {
  if ( dynlen(dpTypes(type)) > 0 ) {
    dyn_string dps = dpNames("*",type);
    for (int i=1; i <= dynlen(dps); ++i)
      dpDelete(dps[i]);
    int result = dpTypeDelete(type);
    ctrlUtils_checkErrors(result,"Failed to deinstall data type '"+type+"'");
    ctrlUtils_trace("Deletion of data type '"+type+"' finished.");
    return 1;
  }
  ctrlUtils_trace("Data type '"+type+"' does not exist....Nothing to do.");
  return 0;
}
//=============================================================================
string ctrlUtils_addFsmTreeNode(string parent, string node, string type, int isCU) {
  if ( 0 == fwFsmTree_isNode(node) ) {
    DebugN("Create Node:"+parent+"::"+node+" ["+type+"]");
    return fwFsmTree_addNode(parent, node, type, isCU);
  }
  DebugN("Node "+parent+"::"+node+" Exists already....");
  return node;
}
//=============================================================================
int ctrlUtils_createFsmTasks(string node, string name, int howmany, int first) {
  string dev_name;
  string set_name = ctrlUtils_addFsmTreeNode(node, name, "FSM_Tasks", 0);
  for(int j=first; j<howmany+first; j++)  {
    sprintf(dev_name,"%s_%03d",name,j);
    if ( !dpExists(dev_name) )  {
      dpCreate(dev_name,"FSM_DimTask");
    }
    string task = ctrlUtils_addFsmTreeNode(set_name, dev_name, "FSM_DimTask", 0);
  }
  return 1;
}
//=============================================================================
int ctrlUtils_createFsmTaskTree(string stream, string slice, dyn_string sets, int tasks_per_set, bool refresh=1)  {
  string name, node  = stream+"_"+slice;
  string slices_node = stream+"_Slices";
  if ( 0 == fwFsmTree_isNode(slices_node) ) {
    slices_node  = ctrlUtils_addFsmTreeNode("FSM", slices_node, "FSM_Holder", 1);
  }
  if ( 0 == fwFsmTree_isNode(node) ) {
    node = ctrlUtils_addFsmTreeNode(slices_node, node, "FSM_Slice", 1);
  }
  DebugN("Slices:"+slices_node+" Stream:"+node,sets,tasks_per_set);
  if ( node != "" )   {
    ctrlUtils_addFsmTreeNode(node, node+"_Config", "StreamConfigurator", 0);
    fwFsmTree_setNodeLabel(node+"_Config","Configurator");
    for(int i=1; i<=dynlen(sets); ++i)  {
      name = node+"_"+sets[i];
      ctrlUtils_createFsmTasks(node,name,tasks_per_set,0);
      fwFsmTree_setNodeLabel(name,sets[i]);
    }
  }    
  if ( refresh ) fwFsmTree_refreshTree();
  DebugN("All Done.");
  return 1;
}
//=============================================================================
int ctrlUtils_deleteAllTree(string stream)  {
  fwFsmTree_removeNode("FSM",stream+"_Slices",1);
  fwFsmTree_refreshTree();
  DebugN("All Done.");
}
//=============================================================================
int ctrlUtils_genAllFSM(string stream="")  {
  fwFsmTree_generateAll();
  DebugN("All Done.");
}
//=============================================================================
int ctrlUtils_startAllTree(string stream="")  {
  fwFsmTree_startTree();
  DebugN("All Done.");
}
//=============================================================================
int ctrlUtils_stopAllTree(string stream="")  {
  fwFsmTree_stopTree();
  DebugN("All Done.");
}
/// Install manager
int ctrlUtils_installPythonManager(int num, string name, string script)  {
  string path = PROJ_PATH+"/bin/"+name;
  file f = fopen(path,"w");
  fprintf(f,"#!/bin/bash\n. `dirname $0`/PVSS00api.sh -NAME "+name+" -DLL PVSSInterface -FUN pvss_pymain $*\n\n");
  fclose(f);  
  system("/bin/chmod +x "+path);
  fwInstallation_addManager(name,"always", 30, 3, 3, "-num "+num+" -SCRIPT "+script);
}
