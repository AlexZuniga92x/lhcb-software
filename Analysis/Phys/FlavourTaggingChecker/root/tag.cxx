#include "tag.h"

int main () {
#include "tag_histo.h" //define here histograms to fill.

  TApplication appKey("appKey",0,0);
  
  TaggerMuonTool            mutool ; //tagging tools to be used
  TaggerElectronTool        eletool;
  TaggerKaonOppositeTool    ktool  ;
  TaggerKaonSameTool        kStool ;
  TaggerPionSameTool        pStool ;
  TaggerVertexChargeTool    vtxtool;
  TaggerFragmentationOppositeTool fragtool;

  CombineTaggersProbability combProbtool;
  CombineTaggersPID         combPIDtool ;
  //CombineTaggersNN          combNNtool;

  NNTuner nnetune(NNetTrain);
  PerformanceMonitor perfmon, perfmonNN;
  EventViewer viewer;

  //FILE LOOP ////////////////////////////////////////////////////////////////
  for( std::vector<TString>::iterator ifile=filelist.begin(); 
       ifile!=filelist.end(); ++ifile ) {
    if(Nmax!=-1 && perfmon.N()>=Nmax) break;
    NtpTag tree(*ifile); //interface between ntuple and event class
    int nentries = tree.size();
        
    //EVENT LOOP >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    for( int i=0; i<nentries; ++i ) {

      Event event; 
      if(! tree.execute(i, event) ) continue; //load event
      if(Nmax!=-1 && perfmon.N()>=Nmax) break;
      PrintAdvance( i, nentries, *ifile );

      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      // CUTS TO SELECT EVENT
      if( requireL0   && event.L0()  ==0 ) continue;
      if( requireHlt1 && event.Hlt1()==0 ) continue;
      if( requireHlt2 && event.Hlt2()==0 ) continue;
      if( requireTis  && event.L0TisTos()&2) continue;
      //if( event.backgroundCategory() != 0) continue;
      //if( event.pileup() == 1 )   continue;
      //if( event.multiplicity()<30) continue;
      //if( event.TrueTag()== -1 ) continue;
      if(!isinTextFile(event.runNr(),event.eventNr())) continue;
      //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

      debug()<<"============================ Process Event"<<endl;
      Particles parts = event.particles();//candidate particles

      //Individual tagger's response:
      Tagger* tmuon = mutool .tag(event);
      Tagger* tele  = eletool.tag(event);
      Tagger* tkaon = ktool  .tag(event);
      Tagger* tsame = event.isBs()? kStool.tag(event):pStool.tag(event);
      Tagger* tvtx  = vtxtool.tag(event);
      Tagger* tfrag = fragtool.tag(event);
   
      //Combination of tagger's decisions:
      Taggers taggers; 
      taggers.push_back(tmuon); 
      taggers.push_back(tele); 
      taggers.push_back(tkaon); 
      taggers.push_back(tsame); 
      taggers.push_back(tvtx); 
      taggers.push_back(tfrag); 
      FlavourTag* theTag = combProbtool.combineTaggers( taggers );

      int TrueTag = event.TrueTag();
      theTag->setTrueTag(TrueTag);

      nnetune.Fill(event, theTag);//fill info for retuning, set by NNetTrain
      perfmon.increment(event, theTag);//update counters for this event
 

      //************************************************************************
      //************************************************************************
      //  FROM THIS POINT JUST HISTOGRAM FILLING!                              *
      //************************************************************************
      //************************************************************************

      //shortcuts
      Particle* BSpart = event.signalB();
      Particle* BOpart = event.oppositeB();
      double     Btime = event.BProperTime(); //in ps
      bool        isBs = event.isBs();
      bool       isTis = event.L0TisTos()&2;
      bool       isTos = event.L0TisTos()&1;
      int     category = theTag->category();
      int  tagdecision = theTag->decision();
      double     omega = theTag->omega();
      int       nparts = parts.size();
      int         krec = event.pileup();
      //get the particle used by tagger:
      Particle*  imuon = tmuon->decision()? tmuon->taggerparts().at(0) : NULL;
      Particle*   iele = tele ->decision()? tele ->taggerparts().at(0) : NULL;
      Particle*  ikaon = tkaon->decision()? tkaon->taggerparts().at(0) : NULL;
      Particle*  isame = tsame->decision()? tsame->taggerparts().at(0) : NULL;

      //store first events to visualize
      if(tagdecision && i<10) viewer.storeClone( event );

      //****************************************
      //event histos
      h1->Fill(BSpart->M());//do not remove this.
      h2->Fill(krec);
      h3->Fill(BOpart->pt());
      h4->Fill(event.multiplicity());

      double var = BSpart->pt();
      h6->Fill(var);
      if(tagdecision && tagdecision==TrueTag) h6r->Fill(var); else h6w->Fill(var);
      if(isame){
        if(tsame->decision()==TrueTag) h6rSS->Fill(var); else h6wSS->Fill(var);
      }
      if(tagdecision && isTis){
        if(tagdecision==TrueTag) h6rtis->Fill(var); else h6wtis->Fill(var);
      }
      if(tagdecision && isTos){
        if(tagdecision==TrueTag) h6rtos->Fill(var); else h6wtos->Fill(var);
      }
      if(tagdecision && isTis && isTos){
        if(tagdecision==TrueTag) h6rtistos->Fill(var); else h6wtistos->Fill(var);
      }

      //tagger histos
      if(iele) {
        h1010->Fill( iele->PIDe() );
        if( iele->absMCID() == 11 ) h1000->Fill(iele->PIDe());
        if( iele->MCID() == 0  )    h1020->Fill(iele->PIDe());
      }
      if(imuon) {
        h1011->Fill(imuon->PIDm());
        if( imuon->absMCID()== 13 ) h1001->Fill(imuon->PIDm());
        if( imuon->MCID()== 0  )    h1021->Fill(imuon->MCID());
      }
      if(ikaon) {
        h1012->Fill(ikaon->PIDk()); h1015->Fill(ikaon->PIDk()-ikaon->PIDp());
        if( ikaon->absMCID()== 321) h1002->Fill(ikaon->PIDk());
        if( ikaon->absMCID()== 321) h1005->Fill(ikaon->PIDk()-ikaon->PIDp());
        if( ikaon->MCID()== 0)      h1022->Fill(ikaon->PIDk());
        if( ikaon->MCID()== 0)      h1025->Fill(ikaon->PIDk()-ikaon->PIDp());

        bool okk = (tkaon->decision() == TrueTag);
        bool k3  = (krec==3);
        bool k1  = (krec==1);
        h70->Fill(ikaon->pt());if(okk&&k1)   h80->Fill(ikaon->pt());
        if(okk&&k3) h80_3->Fill(ikaon->pt()); 
        h71->Fill(ikaon->p()); if(okk&&k1)   h81->Fill(ikaon->p()); 
        if(okk&&k3) h81_3->Fill(ikaon->p()); h72->Fill(ikaon->IPs()); 
        if(okk&&k1) h82->Fill(ikaon->IPs());
        if(okk&&k3) h82_3->Fill(ikaon->IPs());
        h73->Fill(ikaon->LCS());  if(okk&&k1) h83->Fill(ikaon->LCS()); 
        if(okk&&k3) h83_3->Fill(ikaon->LCS()); 
        h74->Fill(ikaon->likelihood());if(okk&&k1) h84->Fill(ikaon->likelihood());
        if(okk&&k3) h84_3->Fill(ikaon->likelihood()); 
        h75->Fill(ikaon->PIDk());  if(okk&&k1) h85->Fill(ikaon->PIDk()); 
        if(okk&&k3) h85_3->Fill(ikaon->PIDk()); 
        h76->Fill(ikaon->PIDk()-ikaon->PIDp());  
        if(okk&&k1) h86->Fill(ikaon->PIDk()-ikaon->PIDp()); 
        if(okk&&k3) h86_3->Fill(ikaon->PIDk()-ikaon->PIDp()); 
        h77->Fill(ikaon->distPhi());  
        if(okk&&k1) h87->Fill(ikaon->distPhi()); 
        if(okk&&k3) h87_3->Fill(ikaon->distPhi()); 
      }
      if(isBs && isame) {
        h1013->Fill(isame->PIDk());  h1014->Fill(isame->PIDk()-isame->PIDp());
        if( isame->absMCID()== 321 ) 
        { h1003->Fill(isame->PIDk()); h1004->Fill(isame->PIDk()-isame->PIDp()); }
        if( isame->absMCID()== 0   ) 
        { h1023->Fill(isame->PIDk()); h1024->Fill(isame->PIDk()-isame->PIDp()); }
      }

      /////////////////////////
      if(imuon) { 
        if(tmuon->decision()==TrueTag) { 
          hr_mu_p->Fill(imuon->p()); 
          hr_mu_pt->Fill(imuon->pt()); 
          hr_mu_ips->Fill(imuon->IPs()); 
          if(krec>1) hr_mu_ippu->Fill(imuon->IPPU());
          hr_mu_lcs->Fill(imuon->LCS()); 
          hr_mu_pid->Fill(imuon->PIDm()); 
          hr_mu_tsal->Fill(imuon->likelihood()); 
          hr_mu_mult->Fill(nparts);         
          homphi_mu_r->Fill(imuon->distPhi());
        } else {
          hw_mu_p->Fill(imuon->p()); 
          hw_mu_pt->Fill(imuon->pt());
          hw_mu_ips->Fill(imuon->IPs()); 
          if(krec>1) hw_mu_ippu->Fill(imuon->IPPU());
          hw_mu_lcs->Fill(imuon->LCS()); 
          hw_mu_pid->Fill(imuon->PIDm()); 
          hw_mu_tsal->Fill(imuon->likelihood()); 
          hw_mu_mult->Fill(nparts); 
          homphi_mu_w->Fill(imuon->distPhi());
        }
      }
      if(iele) { 
        if(tele->decision()==TrueTag) { 
          hr_ele_p->Fill(iele->p()); 
          hr_ele_pt->Fill(iele->pt()); 
          hr_ele_ips->Fill(iele->IPs()); 
          if(krec>1) hr_ele_ippu->Fill(iele->IPPU());
          hr_ele_lcs->Fill(iele->LCS()); 
          hr_ele_pid->Fill(iele->PIDe()); 
          hr_ele_tsal->Fill(iele->likelihood()); 
          hr_ele_mult->Fill(nparts); 
        } else {
          hw_ele_p->Fill(iele->p()); 
          hw_ele_pt->Fill(iele->pt());
          hw_ele_ips->Fill(iele->IPs()); 
          if(krec>1) hw_ele_ippu->Fill(iele->IPPU());
          hw_ele_lcs->Fill(iele->LCS()); 
          hw_ele_pid->Fill(iele->PIDe()); 
          hw_ele_tsal->Fill(iele->likelihood()); 
          hw_ele_mult->Fill(nparts); 
        }
      }
      if(ikaon) { 
        if(tkaon->decision()==TrueTag) { 
          hr_k_p->Fill(ikaon->p()); 
          hr_k_pt->Fill(ikaon->pt()); 
          hr_k_ips->Fill(ikaon->IPs()); 
          if(krec>1) hr_k_ippu->Fill(ikaon->IPPU());
          hr_k_lcs->Fill(ikaon->LCS()); 
          hr_k_pid->Fill(ikaon->PIDk()); 
          hr_k_tsal->Fill(ikaon->likelihood()); 
          hr_k_mult->Fill(nparts); 
          homphi_kO_r->Fill(ikaon->distPhi());
        } else {
          hw_k_p->Fill(ikaon->p()); 
          hw_k_pt->Fill(ikaon->pt());
          hw_k_ips->Fill(ikaon->IPs());
          if(krec>1) hw_k_ippu->Fill(ikaon->IPPU());
          hw_k_lcs->Fill(ikaon->LCS()); 
          hw_k_pid->Fill(ikaon->PIDk()); 
          hw_k_tsal->Fill(ikaon->likelihood()); 
          hw_k_mult->Fill(nparts); 
          homphi_kO_w->Fill(ikaon->distPhi());
        }
      }
      if(isame) { 
        if(tsame->decision()==TrueTag) { 
          hr_kS_p->Fill(isame->p()); 
          hr_kS_pt->Fill(isame->pt()); 
          hr_kS_ips->Fill(isame->IPs()); 
          if(krec>1) hr_kS_ippu->Fill(isame->IPPU());
          hr_kS_lcs->Fill(isame->LCS()); 
          hr_kS_pid->Fill(isame->PIDk()); 
          hr_kS_tsal->Fill(isame->likelihood()); 
          hr_kS_mult->Fill(nparts); 
      	  homphi_kS_r->Fill(isame->distPhi());
        }
        if(tsame->decision()==-TrueTag) { 
          hw_kS_p->Fill(isame->p()); 
          hw_kS_pt->Fill(isame->pt());
          hw_kS_ips->Fill(isame->IPs()); 
          if(krec>1) hw_kS_ippu->Fill(isame->IPPU());
          hw_kS_lcs->Fill(isame->LCS()); 
          hw_kS_pid->Fill(isame->PIDk()); 
          hw_kS_tsal->Fill(isame->likelihood()); 
          hw_kS_mult->Fill(nparts); 
          homphi_kS_w->Fill(isame->distPhi());
        }
      }

      if(iele) { 
        double var = iele->VeloCharge();
        if(tele->decision()==TrueTag) hright->Fill(var); else hwrong->Fill(var);
      }

      //more histos:
      if(TrueTag== tagdecision) {h100->Fill(1-omega); h103->Fill(omega);}
      if(TrueTag==-tagdecision) {h102->Fill(1-omega); h104->Fill(omega);}
      
      if(imuon){
        if(tmuon->decision()==TrueTag) h613->Fill(tmuon->RawNNetOutput()); 
        else h513->Fill(tmuon->RawNNetOutput());
        if(tmuon->decision()==TrueTag) h613p->Fill(tmuon->omega()); 
        else h513p->Fill(tmuon->omega());
      }
      if(iele){
        if(tele->decision()==TrueTag) h611->Fill(tele->RawNNetOutput());
        else h511->Fill(tele->RawNNetOutput());
        if(tele->decision()==TrueTag) h611p->Fill(tele->omega());
        else h511p->Fill(tele->omega());
      }
      if(ikaon){
        if(tkaon->decision()==TrueTag) h6321->Fill(tkaon->RawNNetOutput()); 
        else h5321->Fill(tkaon->RawNNetOutput());
        if(tkaon->decision()==TrueTag) h6321p->Fill(tkaon->omega()); 
        else h5321p->Fill(tkaon->omega());
      }
      if(isame){
        if(tsame->decision()==TrueTag) h535->Fill(tsame->RawNNetOutput()); 
        else h635->Fill(tsame->RawNNetOutput());
        if(tsame->decision()==TrueTag) h535p->Fill(tsame->omega()); 
        else h635p->Fill(tsame->omega());	
      }
      if(tvtx->decision()){
        if(tvtx->decision()==TrueTag) h540->Fill(tvtx->RawNNetOutput()); 
        else h640->Fill(tvtx->RawNNetOutput());
        if(tvtx->decision()==TrueTag) h540p->Fill(tvtx->omega()); 
        else h640p->Fill(tvtx->omega());	
      }

      //oscillation plots:
      int Btag = TrueTag;
      if(imuon) { if(Btag*tmuon->decision()>0)
          h2011->Fill(Btime); else h2012->Fill(Btime); }
      if(iele) { if(Btag*tele->decision()>0) 
          h2021->Fill(Btime); else h2022->Fill(Btime); }
      if(ikaon) { if(Btag*tkaon->decision()>0) 
          h2031->Fill(Btime); else h2032->Fill(Btime); }
      if(isame) { if(Btag*tsame->decision()>0) 
          h2041->Fill(Btime); else h2042->Fill(Btime); }
      if(tvtx->decision())  { if(Btag*tvtx->decision()>0) 
          h2051->Fill(Btime); else h2052->Fill(Btime); }

      int iosc = TrueTag*tagdecision;
      if     (category==1) {if(iosc>0) h3011->Fill(Btime);else h3012->Fill(Btime);}
      else if(category==2) {if(iosc>0) h3021->Fill(Btime);else h3022->Fill(Btime);}
      else if(category==3) {if(iosc>0) h3031->Fill(Btime);else h3032->Fill(Btime);}
      else if(category==4) {if(iosc>0) h3041->Fill(Btime);else h3042->Fill(Btime);}
      else if(category==5) {if(iosc>0) h3051->Fill(Btime);else h3052->Fill(Btime);}
      if     (category!=0) {if(iosc>0) h3061->Fill(Btime);else h3062->Fill(Btime);}
      
      if(tree.enableMC) {
        if( BSpart->hasOscillated() ) iosc = -iosc;
        if( BSpart->hasOscillated() ) iosc = -1*TrueTag; else iosc = 1*TrueTag;
        if(iosc>0) h4061->Fill(Btime); else h4062->Fill(Btime);
      }

      //dump some info if in debug mode////////////////////////
      if(DBGLEVEL<3 //&& tagdecision && event.multiplicity()<15
         ) {
        event.Print(); 
        cout<< event.runNr()  << " " << event.eventNr(); theTag->Print();
        Particles::iterator ipa;
        int npcount=1;
        for(ipa=parts.begin(); ipa!=parts.end(); ipa++) {
          debug()<< npcount++; (*ipa)->Print();
        }
      }

    } //end of loop on event

  }//end of loop on files
  //////////////////////////////////////////////////////////////////////////

  //dump full list of declared properties:
  propertyMap::const_iterator ipro;
  for( ipro=property.begin(); ipro!=property.end(); ++ipro ) {
    debug()<<"Property "<<(*ipro).first<<" = "<<(*ipro).second<<endreq;
  }

  if(NNetTrain!="none") {
    nnetune.Write();
    nnetune.TrainNNet();
  } else {
    perfmon.printStats();
  }
 
  fhisto.Write(); fhisto.Close();

  //viewer.DrawLHCb(); //draw openGL view of detector
  if(property["ViewOptionLHCb"]>=0)
    viewer.DrawStoredEvents(); //also draw tracks on top of it

  timesum += (clock()-timestart)/(double)CLOCKS_PER_SEC;
  info()<<timesum<<" seconds elapsed time ("
        <<timesum*1000./((double)perfmon.N()+1)<<" ms/evt)."<<endmsg;

  return 0;
} 
