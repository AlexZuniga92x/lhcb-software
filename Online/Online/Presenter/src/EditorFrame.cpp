#include <iostream>
#include <vector>

#include <TApplication.h>
#include <TCanvas.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButton.h>
#include <TGTab.h>
#include <TTimer.h>
#include <TThread.h>
#include <TGFrame.h>
#include <TGListTree.h>
#include <TGListView.h>
#include <TGComboBox.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TPad.h>
#include <TMap.h>
#include <TMath.h>
#include <TSystem.h>
#include <TPRegexp.h>
#include <TObjArray.h>
//#include <TImage.h>
//#include <TGVSplitter.h>

#include <TH1.h>
#include <TH2.h>

#include "DbRootHist.h"
#include "EditorFrame.h"
#include "PresenterMainFrame.h"
#include "HistoPropDialog.h"

using namespace std;
using namespace pres;

ClassImp(EditorFrame)

EditorFrame::EditorFrame(TGWindow* p, UInt_t w, UInt_t h, DimBrowser& dim, MsgLevel v) :
    TGCompositeFrame(p, w, h),
    m_verbosity(v),
    m_mainFrame(dynamic_cast<PresenterMainFrame*>(p)),
    m_dimBrowser(&dim),
    m_histogramDB(NULL),
    m_mainCanvasWidth(w),
    m_mainCanvasHeight(h),
    m_histoPadOffset(0)
{
  SetCleanup(kDeepCleanup);
  buildGUI();
  refreshDimSvcListTree();
}

EditorFrame::~EditorFrame()
{
  Cleanup();
}

void EditorFrame::buildGUI() 
{
  // composite frame
  m_editorCanvasMainFrame = new TGCompositeFrame(this,548,502,kVerticalFrame);

  // composite frame
  TGCompositeFrame* fCompositeFrame649 = new TGCompositeFrame(m_editorCanvasMainFrame,548,502,kVerticalFrame);

  // horizontal frame
  TGHorizontalFrame* fHorizontalFrame650 = new TGHorizontalFrame(fCompositeFrame649,544,498,kHorizontalFrame);

  // horizontal frame
  TGVerticalFrame* fVerticalFrame651 = new TGVerticalFrame(fHorizontalFrame650,106,494,kVerticalFrame);

  // canvas widget
  m_dimSvcListTreeContainterCanvas = new TGCanvas(fVerticalFrame651,170,243);

  // canvas viewport
  TGViewPort* fViewPort664 = m_dimSvcListTreeContainterCanvas->GetViewPort();

  m_dimSvcListTree = new TGListTree(m_dimSvcListTreeContainterCanvas, kHorizontalFrame);
  //  m_dimSvcListTree->SetAutoCheckBoxPic(false); ROOT v5.16(?)...
  m_dimSvcListTree->AddRoot("DIM");
  m_dimSvcListTree->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
      "EditorFrame", this,
      "clickedDimTreeItem(TGListTreeItem*, Int_t, Int_t, Int_t)");

  fViewPort664->AddFrame(m_dimSvcListTree);
  m_dimSvcListTree->SetLayoutManager(new TGHorizontalLayout(m_dimSvcListTree));
  m_dimSvcListTree->MapSubwindows();
  m_dimSvcListTreeContainterCanvas->SetContainer(m_dimSvcListTree);
  m_dimSvcListTreeContainterCanvas->MapSubwindows();
  fVerticalFrame651->AddFrame(m_dimSvcListTreeContainterCanvas, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY,2,2,2,2));

  m_dimContextMenu = new TGPopupMenu(fClient->GetRoot());
    m_dimContextMenu->AddEntry("Add checked to Database", M_AddDimToDB_COMMAND);
  m_dimContextMenu->AddEntry("Add checked to Page", M_AddDimToPage_COMMAND);
  m_dimContextMenu->AddSeparator();
  m_dimContextMenu->AddEntry("Check all children", M_DimCheckAllChildren_COMMAND);
  m_dimContextMenu->AddEntry("Uncheck all children", M_DimUnCheckAllChildren_COMMAND);
  m_dimContextMenu->AddSeparator();
  m_dimContextMenu->AddEntry("Collsapse all children", M_DimCollapseAllChildren_COMMAND);
  m_dimContextMenu->AddSeparator();
  m_dimContextMenu->AddEntry("Refresh", M_RefreshHistoDIMListTree_COMMAND);
  m_dimContextMenu->Connect("Activated(Int_t)","EditorFrame", this,
      "handleCommand(Command)");
  // canvas widget
  TGCanvas* fCanvas652 = new TGCanvas(fVerticalFrame651,170,243);

  // canvas viewport
  m_pagesFromHistoDBViewPort = fCanvas652->GetViewPort();

  // list tree
  m_pagesFromHistoDBListTree = new TGListTree(fCanvas652,kHorizontalFrame);
  m_pagesFromHistoDBListTree->AddRoot("Pages");
  m_pagesFromHistoDBListTree->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
      "EditorFrame", this,
      "clickedPageTreeItem(TGListTreeItem*, Int_t, Int_t, Int_t)");

  m_pagesFromHistoDBViewPort->AddFrame(m_pagesFromHistoDBListTree);
  m_pagesFromHistoDBListTree->SetLayoutManager(new TGHorizontalLayout(m_pagesFromHistoDBListTree));
  m_pagesFromHistoDBListTree->MapSubwindows();
  fCanvas652->SetContainer(m_pagesFromHistoDBListTree);
  fCanvas652->MapSubwindows();
  fVerticalFrame651->AddFrame(fCanvas652, new TGLayoutHints(kLHintsRight | kLHintsBottom | kLHintsExpandY,2,2,2,2));

  fHorizontalFrame650->AddFrame(fVerticalFrame651, new TGLayoutHints(kLHintsRight | kLHintsTop | kLHintsExpandY,2,2,2,2));

  m_pagesContextMenu = new TGPopupMenu(fClient->GetRoot());
  //  m_pagesContextMenu->AddEntry("Refresh", M_RefreshDBPagesListTree_COMMAND);
  m_pagesContextMenu->AddEntry("Load Page", M_LoadPage_COMMAND);
  m_pagesContextMenu->Connect("Activated(Int_t)","EditorFrame", this,
      "handleCommand(Command)");

  // vertical frame
  TGVerticalFrame* fVerticalFrame674 = new TGVerticalFrame(fHorizontalFrame650,430,494,kVerticalFrame | kHorizontalFrame);

  // embedded canvas
  editorEmbCanvas = new TRootEmbeddedCanvas(0,fVerticalFrame674,m_mainCanvasWidth, m_mainCanvasHeight);
  Int_t wm_editorEmbCanvas = editorEmbCanvas->GetCanvasWindowId();
  editorEmbCanvas->SetBit(kNoContextMenu);
  editorCanvas = new TCanvas("editor canvas", m_mainCanvasWidth, m_mainCanvasHeight, wm_editorEmbCanvas);
  //  editorCanvas->SetFixedAspectRatio(true);
  editorEmbCanvas->AdoptCanvas(editorCanvas);
  fVerticalFrame674->AddFrame(editorEmbCanvas, new TGLayoutHints(kLHintsRight | kLHintsBottom | kLHintsExpandX | kLHintsExpandY,2,2,2,2));
  editorCanvas->Connect("ProcessedEvent(Int_t,Int_t,Int_t,TObject*)","EditorFrame",this, 
      "EventInfo(Int_t,Int_t,Int_t,TObject*)");
  //  editorCanvas->Connect("Selected(TPad*, TObject*, Int_t)","EditorFrame",this, 
  //                     "highlightSelectedPad(TPad*, TObject*, Int_t)");
  editorCanvas->SetGrid();
  editorCanvas->SetBit(kNoContextMenu);  

  // vertical frame
  m_histoDBFilterFrame = new TGVerticalFrame(fVerticalFrame674,155,490,kVerticalFrame);

  ULong_t ucolor;        // will reflect user color changes
  gClient->GetColorByName("#ffffff",ucolor);

  // combo box
  m_histoDBFilterComboBox = new TGComboBox(m_histoDBFilterFrame,-1,kHorizontalFrame | kSunkenFrame | kDoubleBorder | kOwnBackground);
  m_histoDBFilterComboBox->AddEntry("Folder/Page", ByFolderAndPage); // #0
  m_histoDBFilterComboBox->AddEntry("Tasks/Algorithm", ByTask); // #1
  m_histoDBFilterComboBox->AddEntry("Subsystems", BySubsystem); // #2
  m_histoDBFilterComboBox->AddEntry("With auto analysis", ByHistogramsWithAnalysis); //#3
  m_histoDBFilterComboBox->AddEntry("Produced by analysis", ByAnalysisHistograms); //#4
  m_histoDBFilterComboBox->AddEntry("Full list", AllHistos); // #5
  // @enum FilterCriteria
  m_histoDBFilterComboBox->Resize(149,22);
  m_histoDBFilterComboBox->Select(ByTask);
  m_histoDBFilterComboBox->Connect("Selected(Int_t)", "EditorFrame", this, "refreshHistoDBListTree(FilterCriteria)");
  m_histoDBFilterFrame->AddFrame(m_histoDBFilterComboBox, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX,2,2,2,2));

  // canvas widget
  m_histoDBCanvas = new TGCanvas(m_histoDBFilterFrame,124,460);

  // canvas viewport
  m_histoDBCanvasViewPort = m_histoDBCanvas->GetViewPort();

  // list tree
  m_databaseHistogramTreeList = new TGListTree(m_histoDBCanvas,kHorizontalFrame);
  m_databaseHistogramTreeList->AddRoot("Histograms");
  m_databaseHistogramTreeList->Connect("Clicked(TGListTreeItem*, Int_t, Int_t, Int_t)",
      "EditorFrame", this,
      "clickedHistoDBTreeItem(TGListTreeItem*, Int_t, Int_t, Int_t)");   

  m_histoDBContextMenu = new TGPopupMenu(fClient->GetRoot());
  m_histoDBContextMenu->AddEntry("Add checked histogram(s) to Page", M_AddDBHistoToPage_COMMAND);
  m_histoDBContextMenu->AddEntry("Set properties of checked histogram(s)", M_SetHistoPropertiesInDB_COMMAND);
  m_histoDBContextMenu->AddEntry("Delete checked histogram(s)", M_DeleteDBHisto_COMMAND);
  m_histoDBContextMenu->AddSeparator();
  m_histoDBContextMenu->AddEntry("Check all children", M_DBHistoCheckAllChildren_COMMAND);
  m_histoDBContextMenu->AddEntry("Uncheck all children", M_DBHistoUnCheckAllChildren_COMMAND);
  m_histoDBContextMenu->AddSeparator();
  m_histoDBContextMenu->AddEntry("Collsapse all children", M_DBHistoCollapseAllChildren_COMMAND);  
  m_histoDBContextMenu->Connect("Activated(Int_t)","EditorFrame", this,
      "handleCommand(Command)");

  m_histoDBCanvasViewPort->AddFrame(m_databaseHistogramTreeList);
  m_databaseHistogramTreeList->SetLayoutManager(new TGHorizontalLayout(m_databaseHistogramTreeList));
  m_databaseHistogramTreeList->MapSubwindows();
  m_histoDBCanvas->SetContainer(m_databaseHistogramTreeList);
  m_histoDBCanvas->MapSubwindows();
  m_histoDBFilterFrame->AddFrame(m_histoDBCanvas, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY,2,2,2,2));

  fVerticalFrame674->AddFrame(m_histoDBFilterFrame, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandY,2,2,2,2));

  fHorizontalFrame650->AddFrame(fVerticalFrame674, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY,2,2,2,2));

  fCompositeFrame649->AddFrame(fHorizontalFrame650, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY,2,2,2,2));

  m_editorCanvasMainFrame->AddFrame(fCompositeFrame649, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  AddFrame(m_editorCanvasMainFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  MapSubwindows();
  Resize();
  Resize(this->GetDefaultSize());
  MapWindow();
}

void EditorFrame::hideDBTools()
{
  m_histoDBFilterComboBox->SetEnabled(false);

  m_histoDBCanvasViewPort->HideFrame(m_databaseHistogramTreeList);
  m_pagesFromHistoDBViewPort->HideFrame(m_pagesFromHistoDBListTree);
  //  m_databaseHistogramTreeList->SetBackgroundColor();
  //  m_databaseHistogramTreeList->SetToolTipText(std::string("Please login to DB for Histogram list").c_str(), 1, 1, 250);
}

void EditorFrame::showDBTools()
{
  m_histoDBFilterComboBox->SetEnabled(true);
  //  m_databaseHistogramTreeList->SetToolTipText(std::string("Please select Histogram from list").c_str(), 1, 1, 20);
  m_databaseHistogramTreeList->SetBackgroundColor(GetWhitePixel());
  m_histoDBCanvasViewPort->ShowFrame(m_databaseHistogramTreeList);
  m_pagesFromHistoDBViewPort->ShowFrame(m_pagesFromHistoDBListTree);
}

void EditorFrame::refreshHistoDBListTree(FilterCriteria filterCriteria)
{
  //TODO: add threading(?!)...
  if (m_verbosity >= Verbose) { std::cout << m_message << std::endl; }
  m_mainFrame->readFromHistoDatabase(m_databaseHistogramTreeList,
      filterCriteria, WithHistograms);
}

void EditorFrame::refreshPagesDBListTree()
{
  if (m_mainFrame->isConnectedToDatabase()) {
    m_histoDBContextMenu->EnableEntry(M_AddDBHistoToPage_COMMAND);
    m_histoDBContextMenu->EnableEntry(M_DeleteDBHisto_COMMAND);    
  } else {
    m_histoDBContextMenu->DisableEntry(M_AddDBHistoToPage_COMMAND);
    m_histoDBContextMenu->DisableEntry(M_DeleteDBHisto_COMMAND);
  }

  //TODO: add threading...
  m_mainFrame->readFromHistoDatabase(m_pagesFromHistoDBListTree,
      ByFolderAndPage, WithoutHistograms);
}

void EditorFrame::refreshDimSvcListTree()
{
  if (m_mainFrame->isConnectedToDatabase()) {
    m_dimContextMenu->EnableEntry(M_AddDimToDB_COMMAND);
    m_dimContextMenu->DisableEntry(M_AddDimToPage_COMMAND);    
  } else {
    m_dimContextMenu->DisableEntry(M_AddDimToDB_COMMAND);
    m_dimContextMenu->EnableEntry(M_AddDimToPage_COMMAND);
  }

  // See what we can do about DIM services...  
  m_knownDimServices.clear();

  if (m_dimSvcListTree && m_dimBrowser) {
    const int nDimServers = m_dimBrowser->getServers();
    if (m_verbosity >= Verbose) {
      std::cout << "nDimServers:\t" << nDimServers << std::endl;
    }

    TString dimServerName;
    TString dimServerNodeName;  
    TString dimServiceName;
    TString dimServiceFormat;  
    TString statusMessage;

    bool knownDimSvcType = false;

    // If some servers found, discover what services are available
    if (nDimServers > 0) {
      // First look for a DNS
      const char* dimDnsServerNode = DimClient::getDnsNode();
      if (m_verbosity >= Verbose) {
        std::cout << std::endl << "DNS: " << dimDnsServerNode << std::endl;
      }
      m_mainFrame->setStatusBarText(dimDnsServerNode, 0);

      char* dimServer;
      char* dimServerNode;            
      char* dimService;
      char* dimFormat;  
      int   dimType;

      TGListTreeItem* histoNode = 0;

      TObjArray* dimServiceNameItems = 0;
      TIterator* dimServiceNameItemsIt = 0;
      TObject* dimServiceNameItem = 0;

      TGListTreeItem* node = m_dimSvcListTree->GetFirstItem();
      //      node->CheckItem(false);

      // name DNS root node
      m_dimSvcListTree->RenameItem(node, dimDnsServerNode);
      m_dimSvcListTree->DeleteChildren(node);

      while (m_dimBrowser->getNextServer(dimServer, dimServerNode)) {
        m_dimBrowser->getServerServices(dimServer);
        dimServerName     =  TString(dimServer);
        dimServerNodeName =  TString(dimServerNode);

        // get name of this server
        if (m_verbosity >= Verbose) {
          std::cout << "\t|_ " << dimServerNodeName << "\t-\t" << dimServerName << std::endl;
        }  

        // insert DNS root node
        if (dimServerName.BeginsWith("DIS_DNS")) {          
          m_dimSvcListTree->RenameItem(node, dimDnsServerNode);
          //          m_dimSvcListTree->SetCheckBox(node, kTRUE);
          //          m_dimSvcListTree->ToggleItem(node);
        } else {
          while(dimType = m_dimBrowser->getNextServerService(dimService,
                dimFormat)) {                                                           
            knownDimSvcType = false;
            if (TString(dimService).BeginsWith(H1D) ||
                TString(dimService).BeginsWith(H2D) ||
                TString(dimService).BeginsWith(P1D) ||
                TString(dimService).BeginsWith(HPD) ||
                TString(dimService).BeginsWith(P2D) ||
                TString(dimService).BeginsWith(CNT) ) {
              knownDimSvcType = true;
            }

            // gauchocomment: CCPCHist/Gaucho name prefix workaround
            if (knownDimSvcType && !TString(dimService).EndsWith("gauchocomment")) {

              dimServiceName = TString(dimService);   

              m_knownDimServices.push_back(dimServiceName);

              // TODO: Counters            
              dimServiceFormat = TString(dimFormat);
              if (m_verbosity >= Verbose) {
                std::cout << "\t|\t|_ " << dimServiceName << "\t:\t" << dimServiceFormat << std::endl;
              }

              // TODO: // TODO: H1D/OTMon/OTCommissioningMonitoring/TELL1Mult_$T3/L2/Q0/myTell                           
              dimServiceNameItems = dimServiceName.Tokenize("/");
              dimServiceNameItemsIt = dimServiceNameItems->MakeIterator();
              node = m_dimSvcListTree->GetFirstItem();

              if (m_verbosity >= Verbose) {
                std::cout << "\t|\t\t";
                while (dimServiceNameItem = dimServiceNameItemsIt->Next()) {
                  std::cout << dimServiceNameItem->GetName() << " ";
                }
                std::cout << std::endl;
                dimServiceNameItemsIt->Reset();
              }
              while (dimServiceNameItem = dimServiceNameItemsIt->Next()) {
                if (m_dimSvcListTree->FindChildByName(node, dimServiceNameItem->GetName())) {
                  node = m_dimSvcListTree->FindChildByName(node, dimServiceNameItem->GetName());                
                } else {
                  node = m_dimSvcListTree->AddItem(node, dimServiceNameItem->GetName());
                  //                m_dimSvcListTree->SetCheckBox(node, kTRUE);
                  //                m_dimSvcListTree->ToggleItem(node);
                }
                node->SetUserData(0);
                if (dimServiceNameItem == dimServiceNameItems->At(dimServiceNameItems->GetEntriesFast()-2)) {              
                  histoNode = m_dimSvcListTree->AddItem(node, dimServiceNameItems->Last()->GetName());
                  m_dimSvcListTree->SetCheckBox(histoNode, kTRUE);
                  m_dimSvcListTree->ToggleItem(histoNode);
                  // TODO: new -> delete...                
                  histoNode->SetUserData(new TString(dimServiceName));
                  m_mainFrame->setTreeNodeIcon(histoNode, std::string(dimService).substr(0,3));
                  break;
                }
              }          
            }
          }
        }
      }
    } else {
      m_dimSvcListTree->RenameItem(m_dimSvcListTree->GetFirstItem(), "No DIM");
      //      if (m_verbosity >= Verbose) {
      //        std::cout << "Sorry, no DIM server found." << std::endl;
      //      }
      //      new TGMsgBox(fClient->GetRoot(), this, "DIM Error",
      //               "Sorry, no DIM server found", kMBIconExclamation,
      //               kMBOk, &m_msgBoxReturnCode);
    }
  }
  fClient->NeedRedraw(m_dimSvcListTree);
}

void EditorFrame::clickedDimTreeItem(TGListTreeItem* node,
    MouseButton btn, Int_t x, Int_t y)
{
  // if right button, popup context menu
  if (node && btn == RightMouseButton) {
    m_dimContextMenu->PlaceMenu(x, y, 1, 1);
  }
}

void EditorFrame::clickedHistoDBTreeItem(TGListTreeItem* node,
    MouseButton btn, Int_t x, Int_t y)
{
  // if right button, popup context menu  
  if (node && btn == RightMouseButton) {
    m_histoDBContextMenu->PlaceMenu(x, y, 1, 1);
  }
}

void EditorFrame::clickedPageTreeItem(TGListTreeItem* node,
    MouseButton btn, Int_t x, Int_t y)
{
  // if right button, popup context menu
  if (node && node->GetText() && btn == RightMouseButton && node->IsChecked()) {
    m_pagesContextMenu->PlaceMenu(x, y, 1, 1);
  }
}

void EditorFrame::handleCommand(Command cmd)
{
  // ROOT GUI workaround: slot mechanism differs on different signals from
  // different widgets... void* 4 messages :-(
  if (X_ENIGMA_COMMAND == cmd) {
    TGButton* btn = reinterpret_cast<TGButton*>(gTQSender);
    cmd = static_cast<Command>(btn->WidgetId());
  }

  switch (cmd) {
    case M_AddDimToDB_COMMAND:
      addDimSvcToHistoDB();
      break;
    case M_AddDimToPage_COMMAND:
      addDimSvcToPage();
      break;
    case M_DimCheckAllChildren_COMMAND:
      dimCheckAllChildren();
      break;
    case M_DimUnCheckAllChildren_COMMAND:
      dimUnCheckAllChildren();
      break;
    case M_DimCollapseAllChildren_COMMAND:
      dimCollapseAllChildren();
      break;                  
    case M_RefreshHistoDIMListTree_COMMAND:
      refreshDimSvcListTree();
      break;
    case M_AddDBHistoToPage_COMMAND:
      if (m_mainFrame->isConnectedToDatabase()) {
        addDbHistoToPage();
      } else {
        addDimSvcToPage();
      }
      break;
    case M_DBHistoCheckAllChildren_COMMAND:
      dbHistoCheckAllChildren();
      break;
    case M_DBHistoUnCheckAllChildren_COMMAND:
      dbHistoUnCheckAllChildren();
      break;
    case M_DBHistoCollapseAllChildren_COMMAND:
      dbHistoCollapseAllChildren();
      break;
    case M_DeleteDBHisto_COMMAND:
      deleteHistoFromDB();
      break;
    case M_SetHistoPropertiesInDB_COMMAND:
      setHistoPropertiesInDB();
      break;

      //    TODO: lock treeview...      
      //    case M_RefreshHistoDBListTree_COMMAND:      
      //      refreshHistoDBListTree(static_cast<FilterCriteria>(m_histoDBFilterComboBox->GetSelected()));              
      //      break;
    case M_LoadPage_COMMAND:
      loadPage();
      break;
    default:
      if (m_verbosity >= Debug) {
        std::cout << "zut. TGButton WidgetId from gTQSender is corrupt" <<
          std::endl;
      }
      break;
  }
}

void EditorFrame::addDimSvcToHistoDB()
{      

  m_mainFrame->disableAutoCanvasLayoutBtn();

  disableClearHistos();


  TGListTree *list = new TGListTree();
  m_mainFrame->checkedTreeItems(list, m_dimSvcListTree);

  TGListTreeItem *currentNode;
  currentNode = list->GetFirstItem();
  while(currentNode) {
    try {
      m_histogramDB = m_mainFrame->getDatabase();
      if (m_histogramDB) {      
        m_histogramDB->declareHistByServiceName(std::string(*static_cast<TString*>(currentNode->GetUserData())));
      }
    } catch (SQLException sqlException) {
      // TODO: add error logging backend
      if (m_verbosity >= Verbose) { std::cout << sqlException.getMessage(); }

      new TGMsgBox(fClient->GetRoot(), this, "Database Error",
          Form("OnlineHistDB server:\n\n%s\n",               
            sqlException.getMessage().c_str()), kMBIconExclamation,
          kMBOk, &m_msgBoxReturnCode);
    }  


    currentNode = currentNode->GetNextSibling();    
  }
  try {
    m_histogramDB = m_mainFrame->getDatabase();
    if (m_histogramDB) {      
      if (m_histogramDB->sendHistBuffer()) { m_histogramDB->commit(); }
      refreshHistoDBListTree(static_cast<FilterCriteria>
          (m_histoDBFilterComboBox->GetSelected()));
    }
  } catch (SQLException sqlException) {
    // TODO: add error logging backend
    if (m_verbosity >= Verbose) { std::cout << sqlException.getMessage(); }

    new TGMsgBox(fClient->GetRoot(), this, "Database Error",
        Form("OnlineHistDB server:\n\n%s\n",               
          sqlException.getMessage().c_str()), kMBIconExclamation,
        kMBOk, &m_msgBoxReturnCode);
  }

  list->Delete();
  delete list;
  
  m_mainFrame->checkTreeChildrenItems(m_dimSvcListTree->GetFirstItem(), UncheckTreeItems);
  fClient->NeedRedraw(m_dimSvcListTree);
  
  m_mainFrame->enableAutoCanvasLayoutBtn();
}

// GG next two methods could be merged ?
void EditorFrame::addDimSvcToPage()
{
  m_mainFrame->disableAutoCanvasLayoutBtn();
  // TODO: add locking, exclusive, disable autolayout
  disableClearHistos();

  fClient->WaitFor(dynamic_cast<TGWindow*>(new HistoPropDialog(m_mainFrame, 646,435, m_verbosity)));

  TGListTree *list = new TGListTree();
  m_mainFrame->checkedTreeItems(list, m_dimSvcListTree);

  TGListTreeItem *currentNode;
  currentNode = list->GetFirstItem();
  while(currentNode) {

    int newHistoInstance = 0; // Instance
    // see if the histogram object exists already
    for (m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
        m_DbHistosOnPageIt != m_DbHistosOnPage.end();
        ++m_DbHistosOnPageIt) {
      int existingHistoInstance = (*m_DbHistosOnPageIt)->instance();
  
      if ( 0 == (m_mainFrame->convDimToHistoID(std::string(*static_cast<TString*>(currentNode->GetUserData())))).compare((*m_DbHistosOnPageIt)->identifier()) &&
          (newHistoInstance < existingHistoInstance) ) {
        newHistoInstance = existingHistoInstance;
      }
    }
    if (newHistoInstance > 998) { newHistoInstance = 0; } // ? limit
    newHistoInstance++;
    //  Identifier, dimServiceName, refreshrate, instance, histogramDB    
    DbRootHist* dbRootHist = new DbRootHist(
        m_mainFrame->convDimToHistoID(std::string(*static_cast<TString*>(currentNode->GetUserData()))),
        std::string(*static_cast<TString*>(currentNode->GetUserData())),
        2,
        newHistoInstance,
        NULL,
        m_mainFrame->getAnalib());

    m_DbHistosOnPage.push_back(dbRootHist);

    // Set Properties
    TString paintDrawXLabel = m_mainFrame->m_bulkHistoOptions.m_xLabel;
    TString paintDrawYLabel = m_mainFrame->m_bulkHistoOptions.m_yLabel;
    int paintLineWidth = m_mainFrame->m_bulkHistoOptions.m_lineWidth;
    int paintFillColour = m_mainFrame->m_bulkHistoOptions.m_fillColour;
    int paintFillStyle = m_mainFrame->m_bulkHistoOptions.m_fillStyle;  
    int paintLineStyle = m_mainFrame->m_bulkHistoOptions.m_lineStyle; 
    int paintLineColour = m_mainFrame->m_bulkHistoOptions.m_lineColour;
    int paintStats = m_mainFrame->m_bulkHistoOptions.m_statsOption;
    dbRootHist->rootHistogram->SetXTitle(paintDrawXLabel.Data());
    dbRootHist->rootHistogram->SetYTitle(paintDrawYLabel.Data());
    dbRootHist->rootHistogram->SetLineWidth(paintLineWidth);
    dbRootHist->rootHistogram->SetFillColor(paintFillColour);
    dbRootHist->rootHistogram->SetFillStyle(paintFillStyle);
    dbRootHist->rootHistogram->SetLineStyle(paintLineStyle);  
    dbRootHist->rootHistogram->SetLineColor(paintLineColour);
    dbRootHist->rootHistogram->SetStats((bool)paintStats);

    paintHist(dbRootHist);

    currentNode = currentNode->GetNextSibling();    
  }
  list->Delete();
  delete list; 
  
  m_mainFrame->checkTreeChildrenItems(m_dimSvcListTree->GetFirstItem(), UncheckTreeItems);
  fClient->NeedRedraw(m_dimSvcListTree);
  
  editorCanvas->Update();

  m_mainFrame->enableAutoCanvasLayoutBtn();
}

void EditorFrame::dimCheckAllChildren()
{
  if (m_dimSvcListTree && m_dimSvcListTree->GetSelected()) {
    m_mainFrame->checkTreeChildrenItems(m_dimSvcListTree->GetSelected(), CheckTreeItems);
    fClient->NeedRedraw(m_dimSvcListTree);
  }
}

void EditorFrame::dimUnCheckAllChildren()
{
  if (m_dimSvcListTree && m_dimSvcListTree->GetSelected()) {
    m_mainFrame->checkTreeChildrenItems(m_dimSvcListTree->GetSelected(), UncheckTreeItems);
    fClient->NeedRedraw(m_dimSvcListTree);
  }  
}

void EditorFrame::dimCollapseAllChildren()
{
  if (m_dimSvcListTree && m_dimSvcListTree->GetSelected()) {  
    m_mainFrame->collapseTreeChildrenItems(m_dimSvcListTree, m_dimSvcListTree->GetSelected());
    fClient->NeedRedraw(m_dimSvcListTree);
  }
}

void EditorFrame::dbHistoCheckAllChildren()
{
  if (m_databaseHistogramTreeList && m_databaseHistogramTreeList->GetSelected()) {
    m_mainFrame->checkTreeChildrenItems(m_databaseHistogramTreeList->GetSelected(), CheckTreeItems);
    fClient->NeedRedraw(m_databaseHistogramTreeList);
  }
}

void EditorFrame::dbHistoUnCheckAllChildren()
{
  if (m_databaseHistogramTreeList && m_databaseHistogramTreeList->GetSelected()) {
    m_mainFrame->checkTreeChildrenItems(m_databaseHistogramTreeList->GetSelected(), UncheckTreeItems);
    fClient->NeedRedraw(m_databaseHistogramTreeList);
  }  
}

void EditorFrame::dbHistoCollapseAllChildren()
{
  if (m_databaseHistogramTreeList && m_databaseHistogramTreeList->GetSelected()) {  
    m_mainFrame->collapseTreeChildrenItems(m_databaseHistogramTreeList, m_databaseHistogramTreeList->GetSelected());
    fClient->NeedRedraw(m_databaseHistogramTreeList);
  }
}

void EditorFrame::addDbHistoToPage()
{
  m_mainFrame->disableAutoCanvasLayoutBtn();
  try {
    m_histogramDB = m_mainFrame->getDatabase();
    if (m_histogramDB) {

      TGListTree *list = new TGListTree();
      m_mainFrame->checkedTreeItems(list, m_databaseHistogramTreeList);

      TGListTreeItem *currentNode;
      currentNode = list->GetFirstItem();
      while(currentNode) {
        int newHistoInstance = 0; // Instance
        // see if the histogram object exists already
        for (m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
            m_DbHistosOnPageIt != m_DbHistosOnPage.end();
            ++m_DbHistosOnPageIt) {              
          int existingHistoInstance = (*m_DbHistosOnPageIt)->instance();
          if ( 0 == (std::string(*static_cast<TString*>(currentNode->GetUserData()))).compare((*m_DbHistosOnPageIt)->identifier()) &&
              (newHistoInstance < existingHistoInstance) ) {
            newHistoInstance = existingHistoInstance;
          }
        }
        if (newHistoInstance > 998) { newHistoInstance = 0; } // ? limit
        newHistoInstance++;

        OnlineHistogram* dimQuery = new OnlineHistogram(*(dynamic_cast<OnlineHistDBEnv*>(m_histogramDB)),
            std::string(*static_cast<TString*>(currentNode->GetUserData())),
            std::string("_NONE_"),
            1);          
        //  Identifier, dimServiceName, refreshrate, instance, histogramDB    
        DbRootHist* dbRootHist = new DbRootHist(
            std::string(*static_cast<TString*>(currentNode->GetUserData())),
            dimQuery->dimServiceName(),
            2,
            newHistoInstance,
            m_histogramDB,
	    m_mainFrame->getAnalib());             

        m_DbHistosOnPage.push_back(dbRootHist);
        paintHist(dbRootHist);

        currentNode = currentNode->GetNextSibling();    
      }
      list->Delete();
      delete list; 
    }
  } catch (SQLException sqlException) {
    // TODO: add error logging backend
    if (m_verbosity >= Verbose) { std::cout << sqlException.getMessage(); }

    new TGMsgBox(fClient->GetRoot(), this, "Database Error",
        Form("OnlineHistDB server:\n\n%s\n",               
          sqlException.getMessage().c_str()), kMBIconExclamation,
        kMBOk, &m_msgBoxReturnCode);
  }
  
  m_mainFrame->checkTreeChildrenItems(m_databaseHistogramTreeList->GetFirstItem(), UncheckTreeItems);
  fClient->NeedRedraw(m_databaseHistogramTreeList);

  editorCanvas->Update();
  m_mainFrame->enableAutoCanvasLayoutBtn();
}

void EditorFrame::setHistoPropertiesInDB()
{
  try {
    m_histogramDB = m_mainFrame->getDatabase();
    if (m_histogramDB) {

      fClient->WaitFor(dynamic_cast<TGWindow*>(new HistoPropDialog(m_mainFrame, 646,435, m_verbosity)));

      TGListTree* list = new TGListTree();
      m_mainFrame->checkedTreeItems(list, m_databaseHistogramTreeList);

      TGListTreeItem* currentNode;
      currentNode = list->GetFirstItem();
      while(currentNode) {
        OnlineHistogram* onlineHistogramToSet = m_histogramDB->
          getHistogram(std::string(*static_cast<TString*>(currentNode->GetUserData())));

        std::string paintDrawXLabel = m_mainFrame->m_bulkHistoOptions.m_xLabel.Data();
        std::string paintDrawYLabel = m_mainFrame->m_bulkHistoOptions.m_yLabel.Data();
        int paintLineWidth = m_mainFrame->m_bulkHistoOptions.m_lineWidth;
        int paintFillColour = m_mainFrame->m_bulkHistoOptions.m_fillColour;
        int paintFillStyle = m_mainFrame->m_bulkHistoOptions.m_fillStyle;  
        int paintLineStyle = m_mainFrame->m_bulkHistoOptions.m_lineStyle; 
        int paintLineColour = m_mainFrame->m_bulkHistoOptions.m_lineColour;
        int paintStats = m_mainFrame->m_bulkHistoOptions.m_statsOption;

        if (H1D == (std::string)(onlineHistogramToSet->hstype()).c_str() ||
            P1D == (std::string)(onlineHistogramToSet->hstype()).c_str() ||
            "HPD" == (std::string)(onlineHistogramToSet->hstype()).c_str()) {
          m_drawOption = m_mainFrame->m_bulkHistoOptions.m_1DRootDrawOption;
        } else if (H2D == (std::string)(onlineHistogramToSet->hstype()).c_str()) {
          m_drawOption = m_mainFrame->m_bulkHistoOptions.m_2DRootDrawOption;    
        }  
        std::string paintDrawOption = TString(m_drawOption + TString(" ") + m_mainFrame->m_bulkHistoOptions.m_genericRootDrawOption).Data();

        onlineHistogramToSet->setHistDisplayOption("LABEL_X", &paintDrawXLabel);          
        onlineHistogramToSet->setHistDisplayOption("LABEL_Y", &paintDrawYLabel);          
        onlineHistogramToSet->setHistDisplayOption("FILLSTYLE", &paintFillStyle);
        onlineHistogramToSet->setHistDisplayOption("FILLCOLOR", &paintFillColour);
        onlineHistogramToSet->setHistDisplayOption("LINESTYLE", &paintLineStyle);
        onlineHistogramToSet->setHistDisplayOption("LINECOLOR", &paintLineColour);
        onlineHistogramToSet->setHistDisplayOption("LINEWIDTH", &paintLineWidth);
        onlineHistogramToSet->setHistDisplayOption("STATS", &paintStats);          
        onlineHistogramToSet->setHistDisplayOption("DRAWOPTS", &paintDrawOption);

        currentNode = currentNode->GetNextSibling();    
      }
      list->Delete();
      delete list;
      m_histogramDB->commit();        
    }
  } catch (SQLException sqlException) {
    // TODO: add error logging backend
    if (m_verbosity >= Verbose) { std::cout << sqlException.getMessage(); }

    new TGMsgBox(fClient->GetRoot(), this, "Database Error",
        Form("OnlineHistDB server:\n\n%s\n",               
          sqlException.getMessage().c_str()), kMBIconExclamation,
        kMBOk, &m_msgBoxReturnCode);
  }  

  refreshHistoDBListTree(static_cast<FilterCriteria>
      (m_histoDBFilterComboBox->GetSelected()));
}

void EditorFrame::deleteHistoFromDB()
{
  try {
    m_histogramDB = m_mainFrame->getDatabase();
    if (m_histogramDB) {
      TGListTree *list = new TGListTree();
      m_mainFrame->checkedTreeItems(list, m_databaseHistogramTreeList);

      TGListTreeItem *currentNode;
      currentNode = list->GetFirstItem();
      while(currentNode) {
        OnlineHistogram* histoToDelete = m_histogramDB->
          getHistogram(std::string(*static_cast<TString*>(currentNode->GetUserData())));
        m_histogramDB->removeHistogram(histoToDelete, false);
        currentNode = currentNode->GetNextSibling();    
      }
      list->Delete();
      delete list;
      m_histogramDB->commit();

    }
  } catch (SQLException sqlException) {
    // TODO: add error logging backend
    if (m_verbosity >= Verbose) { std::cout << sqlException.getMessage(); }

    new TGMsgBox(fClient->GetRoot(), this, "Database Error",
        Form("OnlineHistDB server:\n\n%s\n",               
          sqlException.getMessage().c_str()), kMBIconExclamation,
        kMBOk, &m_msgBoxReturnCode);
  }  

  refreshHistoDBListTree(static_cast<FilterCriteria>
      (m_histoDBFilterComboBox->GetSelected()));
}

void EditorFrame::enableLoadPage() {
  m_pagesContextMenu->EnableEntry(M_LoadPage_COMMAND);
}

void EditorFrame::loadPage()
{
  m_mainFrame->disableAutoCanvasLayoutBtn();
  m_mainFrame->clearPage();

  if (m_mainFrame->isConnectedToDatabase()) {

    OnlineHistDB* m_histogramDB = m_mainFrame->getDatabase();
    TGListTreeItem* node = m_pagesFromHistoDBListTree->GetSelected();

    if (node && (0 !=node->GetUserData())) {

      std::string path = std::string(*static_cast<TString*>(node->GetUserData()));
      try {
        float xlow = 0.0;
        float ylow = 0.0;
        float xup = 0.0;
        float yup = 0.0;

        clearHistosOnPage();
        OnlineHistPage* page = m_histogramDB->getPage(path);
        if (page) {  
          page->getHistogramList(&m_onlineHistosOnPage);
          m_onlineHistosOnPageIt = m_onlineHistosOnPage.begin();
          while (m_onlineHistosOnPageIt != m_onlineHistosOnPage.end()) {

            DbRootHist* dbRootHist = new DbRootHist((*m_onlineHistosOnPageIt)->identifier(),
                (*m_onlineHistosOnPageIt)->dimServiceName(),
                1,2, m_histogramDB, m_mainFrame->getAnalib());
            dbRootHist->setdbHist(*m_onlineHistosOnPageIt);
            dbRootHist->setTH1FromDB();

            m_DbHistosOnPage.push_back(dbRootHist);

            page->getHistLayout(*m_onlineHistosOnPageIt, xlow, ylow, xup, yup, (*m_onlineHistosOnPageIt)->instance());
            TPad* pad = new TPad(dbRootHist->histoRootName().c_str(), TString(""), TMath::Abs(xlow), TMath::Abs(ylow), TMath::Abs(xup), TMath::Abs(yup));
            pad->SetBit(kNoContextMenu);
            pad->SetFillColor(10);
            editorCanvas->cd();
            pad->Draw();
            pad->cd();

            if(dbRootHist && pad ) {
              if (dbRootHist->rootHistogram) {
                dbRootHist->Draw();
              }
              dbRootHist->hostingPad = pad;
            }
            m_onlineHistosOnPageIt++;
          }
        }        

      } catch (SQLException sqlException) {
        // TODO: add error logging backend - MsgStream?
        std::string m_message = sqlException.getMessage();
        m_mainFrame->setStatusBarText(m_message.c_str(), 0);
        if (m_verbosity >= Verbose) { std::cout << m_message << std::endl; }

        new TGMsgBox(fClient->GetRoot(), this, "Database Error",
            Form("Could save the page to OnlineHistDB:\n\n%s\n",
              m_message.c_str()), kMBIconExclamation,
            kMBOk, &m_msgBoxReturnCode);
      }
    }
  }
  editorCanvas->Update();
  m_mainFrame->enableAutoCanvasLayoutBtn(); 
}

void EditorFrame::setHistoParFromDB(TH1* histogram, OnlineHistogram* onlineHistogram) {

  histogram->SetNameTitle(onlineHistogram->dimServiceName().c_str(), onlineHistogram->hsname().c_str());
}

bool EditorFrame::paintHist(DbRootHist* histogram)
{

  //  xlow [0,1] is the position of the bottom left point of the pad
  //             expressed in the mother pad reference system
  //  ylow [0,1] is the Y position of this point.
  //  xup  [0,1] is the x position of the top right point of the pad
  //             expressed in the mother pad reference system
  //  yup  [0,1] is the Y position of this point.

  float xlow = 0.1 + 0.01*m_histoPadOffset;
  float ylow = xlow;
  float xup = 0.5 + 0.01*m_histoPadOffset;
  float yup = xup;

  if (0.5 == xlow) {
    m_histoPadOffset = 0;
  }

  m_histoPadOffset++;

  TPad* pad;
  if(histogram) {
    pad = new TPad(histogram->histoRootName().c_str(), TString("Title"), xlow, ylow, xup, yup);
    pad->SetBit(kNoContextMenu);
    pad->SetFillColor(10);
    editorCanvas->cd();
    pad->Draw();
    pad->cd();
    if(pad ) {
      if (histogram->rootHistogram) {
        std::string paintDrawOption;
        if (m_mainFrame->isConnectedToDatabase()) {
          histogram->Draw();      
        } else {
          if (H1D == histogram->hstype() ||
              P1D == histogram->hstype() || HPD == histogram->hstype()) {
            m_drawOption = m_mainFrame->m_bulkHistoOptions.m_1DRootDrawOption;          
            paintDrawOption = TString(m_drawOption + TString(" ") + m_mainFrame->m_bulkHistoOptions.m_genericRootDrawOption).Data();
            histogram->rootHistogram->Draw(paintDrawOption.c_str());          
          } else if (H2D == histogram->hstype()) {
            m_drawOption = m_mainFrame->m_bulkHistoOptions.m_2DRootDrawOption;
            paintDrawOption = TString(m_drawOption + TString(" ") + m_mainFrame->m_bulkHistoOptions.m_genericRootDrawOption).Data();          
            //        if (histogram->histogramImage) { histogram->histogramImage->Draw(); } // paintDrawOption.c_str()
            histogram->rootHistogram->Draw(paintDrawOption.c_str());    
          }  
        }
      }
      //    histogram->setHostPad(pad);
      histogram->hostingPad = pad;
    }
  }
  //editorCanvas->Update();
  return 0;
}

void EditorFrame::autoCanvasLayout()
{

  float xmargin = 0.01;
  float ymargin = 0.01;
  int nx = (int) TMath::Ceil(TMath::Sqrt((double)m_DbHistosOnPage.size()));
  int ny = nx;

  if (nx <= 0) nx = 1;
  if (ny <= 0) ny = 1;
  Int_t ix, iy;
  double x1, y1, x2, y2;
  double dx, dy;

  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();

  if (xmargin > 0 && ymargin > 0) {
    dy = 1/double(ny);
    dx = 1/double(nx);
    for (iy = 0;iy < ny; ++iy) {
      if (m_DbHistosOnPage.end() == m_DbHistosOnPageIt)  { break; }
      y2 = 1 - iy*dy - ymargin;
      y1 = y2 - dy + 2*ymargin;
      if (y1 < 0) { y1 = 0; }
      if (y1 > y2) { continue; }
      for (ix = 0; ix < nx; ++ix) {
        if (m_DbHistosOnPage.end() == m_DbHistosOnPageIt)  { break; }
        x1 = ix*dx + xmargin;
        x2 = x1 + dx - 2*xmargin;
        if (x1 > x2) { continue; }
        if ((*m_DbHistosOnPageIt) && (*m_DbHistosOnPageIt)->hostingPad) {
          ((*m_DbHistosOnPageIt)->hostingPad)->SetPad(x1, y1, x2, y2);
          ((*m_DbHistosOnPageIt)->hostingPad)->Modified();
        }
        ++m_DbHistosOnPageIt;        
      }
    }
  }
  editorCanvas->Update();
}

void EditorFrame::deleteHistoFromCanvas()
{}

void EditorFrame::refreshPage()
{
  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {      
    (*m_DbHistosOnPageIt)->fillHistogram();
    ++m_DbHistosOnPageIt;
  }
  editorCanvas->Update();
}

void EditorFrame::enableClearHistos()
{

  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {      
    (*m_DbHistosOnPageIt)->enableClear();
    ++m_DbHistosOnPageIt;
  }
}

void EditorFrame::disableClearHistos()
{
  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {      
    (*m_DbHistosOnPageIt)->disableClear();
    ++m_DbHistosOnPageIt;
  }
}

void EditorFrame::enablePageRefresh()
{
  editorCanvas->SetEditable(false);
  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {      
    (*m_DbHistosOnPageIt)->disableEdit();
    ++m_DbHistosOnPageIt;
  }
}

void EditorFrame::disablePageRefresh()
{
  editorCanvas->SetEditable(true);
  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {      
    (*m_DbHistosOnPageIt)->enableEdit();
    ++m_DbHistosOnPageIt;
  }
}

void EditorFrame::clearHistosOnPage()
{ 
  disablePageRefresh();
  //  to clean up the page you can now use
  //   bool OnlineHistPage::removeAllHistograms();  

  m_onlineHistosOnPage.clear();

  m_DbHistosOnPageIt = m_DbHistosOnPage.begin();
  while (m_DbHistosOnPageIt != m_DbHistosOnPage.end()) {
    if (*m_DbHistosOnPageIt) { delete *m_DbHistosOnPageIt; }
    ++m_DbHistosOnPageIt;
  }

  m_DbHistosOnPage.clear();

  m_histoPadOffset = 0;

  editorCanvas->cd();
  editorCanvas->Clear();
  editorCanvas->Update();
}

void EditorFrame::EventInfo(int , int px, int py, TObject *selected)
{
  const char *text0, *text1, *text3;
  text0 = selected->GetTitle();
  m_mainFrame->setStatusBarText(text0, 0);
  text1 = selected->GetName();
  m_mainFrame->setStatusBarText(text1, 1);
  text3 = selected->GetObjectInfo(px,py);

  m_mainFrame->setStatusBarText(text3, 2);
}
