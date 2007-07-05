#include "Tim.h"


Tim::Tim(int ttime, std::vector<std::vector<DimInfoHistos*> > tinfo)
  : DimTimer(ttime)
 {
      
      info=tinfo;
 //     for (int i=0;i<= (int)info.size()-1;i++) {
 //        cout << "Timer constructor for histos " << info[i]->m_histoname << endl;
 //     }
      time=ttime;
      start(time);
      
 };
 

void Tim::timerHandler()
   {
      for (int j=0;j<=(int)info.size()-1;j++) {
         for (int i=0;i<=(int)info[j].size()-1;i++) {
 	    info[j][0]->add(info[j][i]);
	    if (info[j][0]->add(info[j][i])==false) {
               info[j][0]->add2d(info[j][i]);
	       if (info[j][0]->add2d(info[j][i])==false) {
	          info[j][0]->addp(info[j][i]);
	       }	  
	    }
         } 
	 info[j][0]->updateSum(); 
      } 	 	 
   //  cout << "Sum updated time " << time << endl;
      start(time);
   };
  
