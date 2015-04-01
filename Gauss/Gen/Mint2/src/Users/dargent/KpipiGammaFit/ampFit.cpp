// author: Jonas Rademacker (Jonas.Rademacker@bristol.ac.uk), Jack Benton (Jack.B.Benton@bristol.ac.uk)
// status:  Fri 28 Jun 2013 11:21:01 GMT
#include "Mint/FitParameter.h"
#include "Mint/NamedParameter.h"
#include "Mint/Minimiser.h"
#include "Mint/Neg2LL.h"
#include "Mint/Neg2LLSum.h"
#include "Mint/DalitzEventList.h"
#include "Mint/NamedDecayTreeList.h"
#include "Mint/DecayTree.h"
#include "Mint/DiskResidentEventList.h"

#include "Mint/CLHEPPhysicalConstants.h"


#include "Mint/PdfBase.h"
#include "Mint/DalitzPdfBase.h"
#include "Mint/DalitzPdfBaseFastInteg.h"
#include "Mint/FitAmplitude.h"
#include "Mint/FitAmpSum.h"
#include "Mint/FitAmpIncoherentSum.h"

#include "Mint/DalitzEvent.h"

#include "Mint/AmpRatios.h"

#include "Mint/IEventGenerator.h"
#include "Mint/DalitzBWBoxSet.h"
#include "Mint/DalitzBoxSet.h"

#include "Mint/SignalGenerator.h"
#include "Mint/FromFileGenerator.h"

#include "Mint/DalitzSumPdf.h"
#include "Mint/cexp.h"


#include "TGraph.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TNtupleD.h"
#include "TTree.h"
#include "TFile.h"


#include "TRandom2.h"
#include "TRandom3.h"

#include <ctime>

#include <iostream>
#include <fstream>

#include "Mint/DalitzPdfNormChecker.h"
#include "Mint/IFastAmplitudeIntegrable.h"
#include "Mint/DalitzPdfSaveInteg.h"


#include "Mint/Chi2Binning.h"
#include "Mint/FitAmpIncoherentSum.h"

#include "Mint/FitAmpList.h"

using namespace std;
using namespace MINT;


class AmpsPdf
  : public DalitzPdfBaseFastInteg
{
protected:
  TRandom* _localRnd;
  SignalGenerator* _sgGen;
  FromFileGenerator* _fileGen;
  IEventGenerator<IDalitzEvent>* _chosenGen;
  NamedParameter<std::string> _integratorSource;
  std::string _integratorFileName;
public:
  double un_normalised_noPs(IDalitzEvent& evt){
    double ampSq =  _amps->RealVal(evt);
    return ampSq;// * getEvent()->phaseSpace();
  }
  
  AmpsPdf(DalitzEventPattern& pat, IFastAmplitudeIntegrable* amps
	  , double precision=1.e-2
	  , std::string method="efficient"
	  , std::string fname =  "SignalIntegrationEvents.root", bool genMoreEvents = false
	  )
    : DalitzPdfBaseFastInteg(pat, 0, amps, precision)
    , _localRnd(0)
    , _sgGen(0)
    , _fileGen(0)
    , _chosenGen(0)
    , _integratorSource("IntegratorSource", (std::string) "new", (char*) 0)
    , _integratorFileName(fname)
  {
    cout << " AmpsPdf with integ method " << method << endl;
    bool nonFlat = "efficient" == method;

    bool generateNew = ((string)_integratorSource == (string)"new");
    if(nonFlat){
      cout << "AmpsPdf uses nonFlat integration." << endl;
      if(generateNew){
	_sgGen =  new SignalGenerator(pat, _amps);
	_sgGen->setWeighted();
	_sgGen->setSaveEvents(_integratorFileName);
	_chosenGen = _sgGen;
      }else{
	// here, SignalGenerator is used by FromFileGenerator, to fill
	// up missing events in case more are needed than found in the
	// file.  Since we don't know with which random seed the
	// events in the file were generated, we supply a random
	// number generator with randomised seed.
	_localRnd = new TRandom3(time(0));
	_sgGen =  new SignalGenerator( pat, _amps, _localRnd);
	_sgGen->setWeighted();
	_sgGen->dontSaveEvents();// saving events is done by FromFileGenerator
	if(genMoreEvents) _fileGen   = new FromFileGenerator( _integratorFileName, _sgGen);
	else{
	  _fileGen = new FromFileGenerator(_integratorFileName, 0);
	  cout << "not going to generate any more events" << endl;
	  cout << "using file " << (std::string) _integratorFileName << endl;
	}
	_chosenGen = _fileGen;
      }
      this->setEventGenerator(_chosenGen);
    }else{
      cout << "AmpsPdf uses flat integration." << endl;
    }
  }
  
  IFastAmplitudeIntegrable* getAmpSum(){ return _amps;}
  
  ~AmpsPdf(){
    if(0 != _fileGen)  delete _fileGen;
    if(0 != _sgGen)    delete _sgGen;
    if(0 != _localRnd) delete _localRnd;
  }
};

int ampFit(){
  time_t startTime = time(0);
  TRandom3 ranLux;
  NamedParameter<int> RandomSeed("RandomSeed", 0);
  ranLux.SetSeed((int)RandomSeed);
  gRandom = &ranLux;
  
  FitAmplitude::AutogenerateFitFile();
  
  NamedParameter<string> InputFileName("InputFileName", (std::string) "");
  NamedParameter<string> InputTreeName("InputTreeName", (std::string) "DalitzEventList");
  std::string inputFile = InputFileName;
  std::string inputTreeName = InputTreeName;
  bool generateNew = (std::string) InputFileName == "";
  std::cout << "InputFileName: " << InputFileName << std::endl;

  NamedParameter<string> IntegratorEventFile("IntegratorEventFile"
					     , (std::string) "SignalIntegrationEvents.root"
					     , (char*) 0);
  NamedParameter<string> IntegratorInputFile("IntegratorInputFile"
					     , (std::string) "sgIntegrator"
					     , (char*) 0);
  NamedParameter<string> OutputRootFile("OutputRootFile"
					     , (std::string) "OutputRootFile.root"
					     , (char*) 0);

  NamedParameter<string> IntegratorOutputFile("IntegratorOutputFile"
					     , (std::string) "sgIntegrator"
					     , (char*) 0);

  NamedParameter<int>  Nevents("Nevents", 1000);
  NamedParameter<double> integPrecision("IntegPrecision", 1.e-2);
  NamedParameter<std::string> integMethod("IntegMethod", (std::string)"efficient");
  
  
  NamedParameter<int> EventPattern("Event Pattern", 521, 321, 211, -211, 443);
  DalitzEventPattern pat(EventPattern.getVector());
  cout << " got event pattern: " << pat << endl;

  FitAmpSum fas(pat);
    
  DalitzEventList eventList;

  if(generateNew){
    	SignalGenerator sg(pat);
    	cout << "Generating " << Nevents << " MC events." << endl;
    	sg.FillEventList(eventList, Nevents);
    	eventList.saveAsNtuple(OutputRootFile);
  }

  if(!generateNew){
		TFile *_InputFile =  TFile::Open(inputFile.c_str());
		TTree* in_tree;
		in_tree=dynamic_cast<TTree*>(_InputFile->Get(inputTreeName.c_str()));
		cout << "reading events from file " << inputFile << endl;
		eventList.fromNtuple(in_tree,1);
		cout << " I've got " << eventList.size() << " events." << endl;
		_InputFile->Close();
  }

  DalitzHistoSet datH = eventList.histoSet();
  datH.draw("data_","eps");

  AmpsPdf amps(pat, &fas, integPrecision,integMethod, (std::string) IntegratorEventFile);
  amps.setIntegratorFileName(IntegratorInputFile);

  Neg2LL fcn(amps, eventList);
  Minimiser mini(&fcn);
  mini.doFit();
  mini.printResultVsInput();
  amps.doFinalStats(&mini);
  amps.saveIntegrator(IntegratorOutputFile);

  cout << "Saving each amplitude in a histogram." << endl;
  amps.saveEachAmpsHistograms("EachAmpHistos");

/*
    bool doChi = false;
    if (doChi){
    	DiskResidentEventList chi2Events("GeneratedSignalEvents.root");
		cout << "Plotting Chi." << endl;
		Chi2Binning chi2;
		cout << "finished Chi2Binning chi2;" << endl;
		chi2.createBinning(&eventList, 10, 100);
		cout << "here after chi2.createBinning(&eventList, 10, 100);" << endl;
		chi2.setEventsAndPdf(&eventList, &chi2Events, &amps);
		int degrees = 0;
		for(unsigned int i=0; i < fasSet.size(); i++){
		    if(0 == fasSet.getParPtr(i)) continue;
		    if(fasSet.getParPtr(i)->iFixInit() == 1) continue;
		    degrees++;
		}
		cout << "Got number of degrees of freedom: " << degrees << endl;
		int bins = chi2.numBins();
		int denominator = bins - degrees;
		double chi2perdof = ((chi2.getChi2_perBin())*bins)/denominator;
		cout << "Chi2 oer d.o.f: " << chi2perdof << endl;

    }
*/


  DalitzHistoSet fitH = amps.histoSet();
  //fitH.save("plotsFromIntegrator.root");
  //fitH.draw("fitPlots_","pdf");
  datH.drawWithFitNorm(fitH, "datFit_","eps");
      
  //std::vector<DalitzHistoSet> EachAmpsHistos = amps.GetEachAmpsHistograms();
  //datH.drawWithFitAndEachAmps(datH, fitH, EachAmpsHistos, "WithAmps", "pdf");
  //EachAmpsHistos[1].draw("EachAmpsTest","pdf");
  //datH.save("plotFromEvtList.root");
  //datH.draw("dataPlotsEff_","pdf");

  cout << "==============================================" << endl;
  cout << " Done. " << " Total time since start " << (time(0) - startTime)/60.0 << " min." << endl;
  cout << "==============================================" << endl;

  return 0;
}


int main(){
  return ampFit();
}
//
