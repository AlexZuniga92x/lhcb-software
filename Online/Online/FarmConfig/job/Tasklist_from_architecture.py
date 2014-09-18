from xml.dom import minidom
import sys
import os
import OnlineEnvBase

def TaskListfromArch(arch, tasklist):
    xmldoc = minidom.parse(arch)
    itemlist = xmldoc.getElementsByTagName('task')
#    tasklist = []
    for s in itemlist:
        tasklist.append(s.attributes['name'].value)
def OptionsfromTasks(tasklist,level,ofile,pname,dohostdns):
    f = open(ofile,'w')
    f.write("//  Adder Level "+level+"\n")
    if level == "1":
        InDns = os.getenv("InDns","<dns>")
        OutDns = os.getenv("OutDns","<dns>")
        InDns = "\""+InDns+"\""
        OutDns = "\""+OutDns+"\""
        f.write("""#include "$INFO_OPTIONS"

ApplicationMgr.ExtSvc               += {"MonitorSvc","BusySvc"};

ApplicationMgr.EventLoop             = "LHCb::OnlineRunable/EmptyEventLoop";
ApplicationMgr.Runable               = "LHCb::OnlineRunable/Runable";
ApplicationMgr.HistogramPersistency  = "NONE";
ApplicationMgr.EvtSel                = "NONE";

Runable.Wait                         = 3;  // 1 of running as daemon (Class1 task)

MessageSvc.fifoPath                  = "$LOGFIFO";
MessageSvc.LoggerOnly                = true;  // Set to false for stdout
MessageSvc.OutputLevel               = @OnlineEnv.OutputLevel;
MonitorSvc.OutputLevel               = @OnlineEnv.OutputLevel;
HistogramPersistencySvc.Warnings     = false;
BusySvc.BogusMips                    = 0.0;
MonitorSvc.CounterUpdateInterval     = 5;
""")
        histsvc = []
        cntsvc = []
        for s in tasklist:
            if 'NodeAdder' in s:
                continue
            if 'AligAdder' in s:
                continue
            hsvc = s#+"HistAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+hsvc+"HistAdder\"};\n")
            csvc = s#+"CountAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+csvc+"CountAdder\"};\n")
            histsvc.append(hsvc)
            cntsvc.append(csvc)
        f.write("\n")

        for s in histsvc:
            svc = s+"HistAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_<node>_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_<node>_"+s+"_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_<node>_"+s+"_(.*)/Histos/\";\n")
            f.write(svc+".AdderClass  = \"hists\";\n")
            f.write(svc+".ReceiveTimeout = 3;\n")
            f.write("\n")
            if dohostdns:
                f.write(svc+".InDNS = "+InDns+";\n")
                f.write(svc+".OutDNS = "+OutDns+";\n")
        for s in cntsvc:
            svc = s+"CountAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_<node>_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_<node>_"+s+"_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_<node>_"+s+"_(.*)/Counter/\";\n")
            f.write(svc+".AdderClass  = \"Counter\";\n")
            f.write(svc+".ReceiveTimeout = 2;\n")
            if pname == "LHCbA":
              f.write(svc+".GotoPause = true;\n")
              f.write(svc+".ReceiveTimeout = 0;\n")
            if dohostdns:
                f.write(svc+".InDNS = "+InDns+";\n")
                f.write(svc+".OutDNS = "+OutDns+";\n")
            f.write("\n")
    elif level == "2":
        InDns = os.getenv("InDns","<node>")
        OutDns = os.getenv("OutDns","hlt01")
        InDns = "\""+InDns+"\""
        OutDns = "\""+OutDns+"\""
        f.write("""#include "$INFO_OPTIONS"

ApplicationMgr.ExtSvc               += {"MonitorSvc","AdderSvc/BusyAdder"};

ApplicationMgr.EventLoop             = "LHCb::OnlineRunable/EmptyEventLoop";
ApplicationMgr.Runable               = "LHCb::OnlineRunable/Runable";
ApplicationMgr.HistogramPersistency  = "NONE";
ApplicationMgr.EvtSel                = "NONE";

Runable.Wait                         = 3;  // 1 of running as daemon (Class1 task)

MessageSvc.fifoPath                  = "$LOGFIFO";
MessageSvc.LoggerOnly                = true;  // Set to false for stdout
MessageSvc.OutputLevel               = @OnlineEnv.OutputLevel;
MonitorSvc.OutputLevel               = @OnlineEnv.OutputLevel;
HistogramPersistencySvc.Warnings     = false;
BusySvc.BogusMips                    = 0.0;
MonitorSvc.CounterUpdateInterval     = 5;

BusyAdder.MyName                = "<part>_<node>_Busy";
BusyAdder.PartitionName         = @OnlineEnv.PartitionName;
BusyAdder.TaskPattern           = "<part>_<node>[0-9][0-9]_NodeAdder_0";
BusyAdder.ServicePattern        = "MON_<part>_<node>[0-9][0-9]_NodeAdder_0/Counter/";
BusyAdder.AdderClass            = "Counter";
BusyAdder.InDNS                 = "<node>";
BusyAdder.OutDNS                = "hlt01";
BusyAdder.ReceiveTimeout          = 3;
""")
        f.write("BusyAdder.InDns     = "+InDns+";\n")
        f.write("BusyAdder.OutDns     = "+OutDns+";\n")
        histsvc = []
        cntsvc = []
        for s in tasklist:
            if 'SubFarmAdder' in s:
                continue
            hsvc = s#+"HistAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+hsvc+"HistAdder\"};\n")
            csvc = s#+"CountAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+csvc+"CountAdder\"};\n")
            histsvc.append(hsvc)
            cntsvc.append(csvc)
        f.write("\n")

        for s in histsvc:
            svc = s+"HistAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_<node>_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_<node>[0-9][0-9]_NodeAdder_0\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_<node>[0-9][0-9]_"+s+"/Histos/\";\n")
            f.write(svc+".AdderClass  = \"hists\";\n")
            f.write(svc+".ReceiveTimeout = 6;\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            f.write("\n")


        for s in cntsvc:
            svc = s+"CountAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_<node>_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_<node>[0-9][0-9]_NodeAdder_0\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_<node>[0-9][0-9]_"+s+"/Counter/\";\n")
            f.write(svc+".ReceiveTimeout = 6;\n")
            f.write(svc+".AdderClass = \"Counter\";\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            if pname == "LHCbA":
              f.write(svc+".GotoPause = true;\n")
              f.write(svc+".ReceiveTimeout = 0;\n")
            f.write("\n")
    elif level == "3":
        InDns = os.getenv("InDns","hlt01")
        OutDns = os.getenv("OutDns","mona08")
        InDns = "\""+InDns+"\""
        OutDns = "\""+OutDns+"\""
        f.write("""#include "$INFO_OPTIONS"

ApplicationMgr.ExtSvc               += {"MonitorSvc","AdderSvc/BusyCountAdder"};

ApplicationMgr.EventLoop             = "LHCb::OnlineRunable/EmptyEventLoop";
ApplicationMgr.Runable               = "LHCb::OnlineRunable/Runable";
ApplicationMgr.HistogramPersistency  = "NONE";
ApplicationMgr.EvtSel                = "NONE";

Runable.Wait                         = 3;  // 1 of running as daemon (Class1 task)

MessageSvc.fifoPath                  = "$LOGFIFO";
MessageSvc.LoggerOnly                = true;  // Set to false for stdout
MessageSvc.OutputLevel               = @OnlineEnv.OutputLevel;
MonitorSvc.OutputLevel               = @OnlineEnv.OutputLevel;
HistogramPersistencySvc.Warnings     = false;
BusySvc.BogusMips                    = 0.0;
MonitorSvc.CounterUpdateInterval     = 5;

""")
        histsvc = []
        cntsvc = []
        histsvc.append("Adder")
        cntsvc.append("Busy")
        for s in tasklist:
            hsvc = s#+"HistAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+hsvc+"HistAdder\"};\n")
            csvc = s#+"CountAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+csvc+"CountAdder\"};\n")
            histsvc.append(hsvc)
            cntsvc.append(csvc)
        f.write("\n")

        for s in histsvc:
            svc = s+"HistAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_<node>_"+s+"_00\";\n")
            f.write(svc+".TaskPattern = \"<part>_HLT[a-z][0-9][0-9]_SubFarmAdder_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_hlt[a-z][0-9][0-9]_"+s+"/Histos/\";\n")
            f.write(svc+".AdderClass  = \"hists\";\n")
            f.write(svc+".ReceiveTimeout = 12;\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            f.write(svc+".SaveRootDir = \"/hist/Savesets\";\n");
            f.write(svc+".IsSaver = true;\n");
            f.write(svc+".SaveSetTaskName= \""+svc+"\";\n");
            f.write("\n")

        for s in cntsvc:
            svc = s+"CountAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_"+s+"_00\";\n")
            f.write(svc+".TaskPattern = \"<part>_HLT[a-z][0-9][0-9]_SubFarmAdder_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_hlt[a-z][0-9][0-9]_"+s+"/Counter/\";\n")
            f.write(svc+".AdderClass  = \"Counter\";\n")
            f.write(svc+".ReceiveTimeout = 12;\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            if pname == "LHCbA":
              f.write(svc+".ReceiveTimeout = 0;\n")
#              f.write(svc+".GotoPause = true;\n")
            f.write("\n")
    elif level == "4":
        InDns = os.getenv("InDns","mona08")
        OutDns = os.getenv("OutDns","mona08")
        InDns = "\""+InDns+"\""
        OutDns = "\""+OutDns+"\""
        f.write("""#include "$INFO_OPTIONS"

ApplicationMgr.ExtSvc               += {"MonitorSvc","AdderSvc/BusyCountAdder"};

ApplicationMgr.EventLoop             = "LHCb::OnlineRunable/EmptyEventLoop";
ApplicationMgr.Runable               = "LHCb::OnlineRunable/Runable";
ApplicationMgr.HistogramPersistency  = "NONE";
ApplicationMgr.EvtSel                = "NONE";

Runable.Wait                         = 3;  // 1 of running as daemon (Class1 task)

MessageSvc.fifoPath                  = "$LOGFIFO";
MessageSvc.LoggerOnly                = true;  // Set to false for stdout
MessageSvc.OutputLevel               = @OnlineEnv.OutputLevel;
MonitorSvc.OutputLevel               = @OnlineEnv.OutputLevel;
HistogramPersistencySvc.Warnings     = false;
BusySvc.BogusMips                    = 0.0;
MonitorSvc.CounterUpdateInterval     = 5;

""")
        histsvc = []
        cntsvc = []
#        histsvc.append("Adder")
#        cntsvc.append("Busy")
        for s in tasklist:
            hsvc = s#+"HistAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+hsvc+"HistAdder\"};\n")
            csvc = s#+"CountAdder"
            f.write("ApplicationMgr.ExtSvc               += {\"AdderSvc/"+csvc+"CountAdder\"};\n")
            histsvc.append(hsvc)
            cntsvc.append(csvc)
        f.write("\n")

        for s in histsvc:
            svc = s+"HistAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_X_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_HLT01_PartAdder_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_hlt01_"+s+"/Histos/\";\n")
            f.write(svc+".AdderClass  = \"hists\";\n")
            f.write(svc+".ReceiveTimeout = 12;\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            f.write("\n")

        for s in cntsvc:
            svc = s+"CountAdder"
            f.write(svc+".PartitionName  = @OnlineEnv.PartitionName;\n")
            f.write(svc+".MyName  = \"<part>_X_"+s+"\";\n")
            f.write(svc+".TaskPattern = \"<part>_HLT01_PartAdder_(.*)\";\n")
            f.write(svc+".ServicePattern  = \"MON_<part>_hlt01_"+s+"/Counter/\";\n")
            f.write(svc+".AdderClass  = \"Counter\";\n")
            f.write(svc+".ReceiveTimeout = 12;\n")
            f.write(svc+".InDNS = "+InDns+";\n")
            f.write(svc+".OutDNS = "+OutDns+";\n")
            if pname == "LHCbA":
              f.write(svc+".ReceiveTimeout = 0;\n")
            f.write("\n")

tasklist = []
arch = OnlineEnvBase.HltArchitecture
part = OnlineEnvBase.PartitionName
arch = os.getenv("ARCH",arch)
hostdns = False
if arch == "Calib":
    hostdns = True
arch = "/group/online/dataflow/architectures/lbDataflowArch_"+arch+".xml"
level = sys.argv[1]
TaskListfromArch(arch, tasklist)
ofile = ""
if len(sys.argv) >= 3:
    ofile = sys.argv[2]
if ofile == "":
    ofile = "/tmp/AdderOptions.opts"
OptionsfromTasks(tasklist,level,ofile,part,hostdns)
