#include "globals.h"
#include "taggingutils.h"

#ifdef __CINT__
#undef __attribute__
#define __attribute__(X)
#endif
#include <dirent.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//utils
#include <Riostream.h>
#include "TString.h"
#include "TObjString.h"
#include "TObjArray.h"
#include <cstdlib> 
#include <algorithm>
#include <map>
#include <utility>
#include <cmath>
#include <sstream>


double PI(3.1416);
static vector< pair<int, int> > filevec(0);
static bool lock_filevec(false);
extern TString filevec_name;
propertyMap property;
propertyStringMap stringproperty;

//////////////////////////////////////////////////////////////////////////
template < class T > string toString(const T &arg){
  ostringstream out;
  out << arg;
  return out.str();
}

bool isinTextFile(int run, int evt) {

  if(filevec_name=="none") return true;

  if(!lock_filevec) {//init

    if(stringproperty["EventListFileName"]=="DATAFILE") {

      TString datafile = readString("datafile");

      //drop root filename.root from path if any
      if(datafile.EndsWith(".root")){
        TObjArray * words = datafile.Tokenize("/");
        TObjString* tmp   = (TObjString*) words->Last();
        TString rootname  = (TString)"/" + tmp->GetString().Data();
        datafile.ReplaceAll(rootname, "");
        delete words;
      }
      
      std::vector<TString> listfile = getFiles(toString(datafile),".eventlist");
      if(listfile.size()>1) {
        fatal()<<" more than one eventlist found in "<<datafile<<endmsg;
        exit(1);
      }
      if(listfile.empty()) {//no existe
        fatal()<< datafile
               <<" (*.eventlist) file does not exist or is empty.\n"
               <<" You can change DATAFILE to none in tag.opts to disable it."
               <<endmsg;
        exit(1);
      }
      filevec_name=listfile.at(0);
    }

    info()<<"\nReading list "<< filevec_name <<endmsg;
    filevec.clear(); lock_filevec=true; 

    ifstream indata; 
    indata.open(filevec_name); // opens the event list file
    if(!indata) {
      fatal() << " in isinTextFile: "<< filevec_name
              << " could not be opened." << endmsg;
      exit(1);
    }
    while ( !indata.eof() ) { 
      int a, b;
      indata >> a >> b;
      //cout<<a<<"   "<<b<<endl;
      pair<int, int> p(a,b);
      filevec.push_back(p);
    }
    indata.close();
  }//end init

  pair< int, int > value(run,evt);
  vector< pair< int, int > >::iterator location = 
    find(filevec.begin(), filevec.end(), value);

  if (location != filevec.end()) {
    filevec.erase(location);
    return true;
  }
  
  return false;

}

//////////////////////////////////////////////////////////////////////////
bool isNonResonantD(float id) {
  
  int aid = (int)fabs(id);
  if(aid==411 || aid==421 || aid==431) return true;

  return false;
}
//////////////////////////////////////////////////////////////////////////
bool isD(int id) {
  
  int aid = abs(id);
  if(aid>410&&aid<446)     return true;
  if(aid>4100&&aid<4556)   return true;
  if(aid>10410&&aid<10454) return true;
  if(aid>20410&&aid<20454) return true;

  return false;
}
////////////////////////////////////////////////////////////////////////
double dQ(const TLorentzVector& BSpart, const TLorentzVector& ipart) {
  return (BSpart + ipart).M() - BSpart.M();
}
double dPhi(double f1, double f2){
  double a = fabs(f1-f2);
  if(a>PI) a = 2*PI-a;
  return a;
}
////////////////////////////////////////////////////////////////////////
TLorentzVector build4V(double p, double pt, double phi, double ID) { 
  //if ID is a fractionary nr <9 it will be assumed as a mass in GeV
  if(!p) return 0.0;
  double a = p*p - pt*pt;
  if(a<0) {
    err()<<"in build4V: Pt greater than P!"<<endmsg;
    return 0.0;
  }
  double pid = abs((int)ID);

  double m=0;
  if( ID>0 && ID<9 ) m = ID;//take it as a mass 
  else if(pid==211)  m = 0.13957;
  else if(pid==321)  m = 0.493677;
  else if(pid==2212) m = 0.938272;
  else if(pid==11)   m = 0.000511;
  else if(pid==13)   m = 0.105658;
  else if(pid>3000)  return 0.0;
  else { 
    err()<<"in build4V: unknown particle ID="<<ID <<"  (Pt="<<pt<<")"<<endmsg; 
    return 0.0;
  }

  TLorentzVector v( pt*cos(phi), pt*sin(phi), sqrt(a), sqrt(p*p+m*m) );
  return v;
}

////////////////////////////////////////////////////////////////////////
double calc_dQ(const TLorentzVector& BSpart, const TLorentzVector& ipart) {
  return (BSpart + ipart).M() - BSpart.M();
}

//======================================================================
void PrintAdvance(int n, float nmax, TString& filename) {
  if(DBGLEVEL!=3) return;
  if(n==0) info()<<DEL1L<<nmax<<" evts in "<<AMARILLO<<filename
                 <<ROJO2<<" |<<<<<<<<<<<<<<  ";
  float r= float(n)/int(nmax/15.);
  if(r==int(r)) cout<<ROJO2<<"\b\b| \b"<<flush<<ENDC;
}

//======================================
// buffers and streams

int DBGLEVEL(3); //global

nullbuf cnull_obj;
std::ostream cnull(&cnull_obj);
//wnullbuf wcnull_obj;
//std::wostream wcnull(&wcnull_obj); 

ostream& verbose() {if(DBGLEVEL>1) return cnull; else return cout;}
ostream& debug()   {if(DBGLEVEL>2) return cnull; else return cout;}
ostream& info()    {if(DBGLEVEL>3) return cnull; else return cout;}
ostream& warning() {if(DBGLEVEL>4) return cnull; else return cout<<ROJO2<<"WARNING "<<ENDC;}
ostream& err()     {if(DBGLEVEL>5) return cnull; else return cout<<ROJO2<<"ERROR "<<ENDC;}
ostream& fatal()   {if(DBGLEVEL>6) return cnull; else return cout<<ROJO2<<"FATAL "<<ENDC;}


//functions/////////////////////////////////////////////////////////////////
TString readString(TString varname) { //< function parsing the tag.opts file

  ifstream indata; 
  indata.open("tag.opts"); // opens the file
  if(!indata) { // file couldn't be opened
    fatal() << " tag.opts could not be opened" << endmsg;
    exit(1);
  }

  TString line, value="";
  // assumes empty line at eof!
  while (!line.ReadLine(indata).eof()) { 
    if(line.BeginsWith("#")) continue; //comments out whole line by # sign
    if(line.BeginsWith("EndOfOptions")) break;
    
    TObjArray * words = line.Tokenize(" ,;:");
    int nwords = words->GetEntriesFast();

    //single object
    if(nwords==1){

      TObjString* tmp1= (TObjString*)words->operator[](0);   
      TString word1   = tmp1->GetString().Data();
      if( word1.Contains("=") && !word1.EndsWith("=") && !word1.BeginsWith("=")){
        TObjArray * subwords = line.Tokenize(" =,;:");
        if(subwords->GetEntriesFast()!=2) {
          fatal()<<"Wrong statement: "<<word1<<endmsg;
          exit(1);
        }
        TObjString* subtmp1= (TObjString*)subwords->operator[](0);   
        TObjString* subtmp2= (TObjString*)subwords->operator[](1); 
        TString subword1   = subtmp1->GetString().Data();
        TString subword2   = subtmp2->GetString().Data();
        if(subword1 == varname) value= subword2;
        delete subwords;
      }
    }

    for(int i = 0; i!=nwords-1; i++ ) {

      TObjString* tmp1= (TObjString*)words->operator[](i);   
      TObjString* tmp2= (TObjString*)words->operator[](i+1); 
      TString word1   = tmp1->GetString().Data();
      TString word2   = tmp2->GetString().Data();

      if(word2 == "=" && nwords>2) {
        TObjString* tmp3= (TObjString*)words->operator[](i+2); 
        TString word3   = tmp3->GetString().Data();
        if(word1 == varname) value= word3;
      } 

      if( (word1.Length()>1 && word1.EndsWith("=") ) ||
          (word2.Length()>1 && word2.BeginsWith("=")) ) {
        word1.ReplaceAll("=","");
        word2.ReplaceAll("=","");
        if(word1 == varname) value= word2;
      }
    }
    delete words;
  }

  indata.close();
  // if(value=="") {
  // err()<<"Input "<<varname<<" not found in tag.opts! Stop.\n";
  // exit(1);
  //}
  return value;
}
//======================================
double read(const char* varname  ) { return read(TString(varname)); }

double read(TString varname) {
  TString s = readString( varname );
  if(s=="") return -12345;
  if(s.IsAlpha()) err()<<"Expecting numeric value for "<<varname
                       <<"   "<<s <<endmsg;
  return s.Atof();
}

//=====================================================================
std::vector<TString> getFiles(string dir, const char* extension) {

  std::vector<TString> filelist(0);
  ifstream fin;
  string filepath;
  DIR *dp;
  struct dirent *dirp;
  struct stat filestat;

  dp = opendir( dir.c_str() );
  if (dp == NULL) {
    return filelist;
  }
  while ((dirp = readdir( dp ))) {
    filepath = dir + "/" + dirp->d_name;
    // If the file is a directory (or is in some way invalid) we'll skip it 
    if (stat( filepath.c_str(), &filestat )) continue;
    if (S_ISDIR( filestat.st_mode ))         continue;
    //check that filepath word contains extension
    const char* conroot = strstr( filepath.c_str(), extension );
    if( !conroot ) continue;
    //cout<<conroot<<"file  "<<filepath<<endl;
    filelist.push_back(filepath);
  }
  closedir( dp );
  std::sort(filelist.begin(), filelist.end());
  return filelist;
}

//======================================
//N.B.: these fncs cannot be merged...
void declareProperty( const TString& variablename, double& value) {
  propertyMap::iterator i = property.find(variablename);
  if( i != property.end() ){
    if( i->second != value ) {
      warning()<<SUBVIOLETA<<"Trying to replace property "<< variablename<<"="
               << i->second <<" with "<< value<< ", ignored."<<endmsg;
    }
    return;
  }
  double newvalue = read(variablename);
  if( newvalue!=-12345 && newvalue != value ) { //found item to update
    cout<< " *** double " <<variablename << " = "<<value
        <<"\t\tchanged to "<<newvalue<<endl;
    value = newvalue;
  } else { //not found, leave as it is assigned in called fnc
    //cout<<variablename<<" not found."<<endl;
  }
  property.insert(make_pair(variablename, value)); 
}
void declareProperty( const TString& variablename, int& value) {
  int newvalue = read(variablename);
  propertyMap::iterator i = property.find(variablename);
  if( i != property.end() ){
    if( (int) i->second != value ) {
      warning()<<SUBVIOLETA<<"Trying to replace property "<< variablename<<"="
               << i->second <<" with "<< value<< ", ignored."<<ENDC<<endmsg;
    }
    return;
  }
  if( newvalue!=-12345 && newvalue != value ) { //found item to update
    cout<< " *** int " <<variablename << " = "<<value
        <<"\t\tchanged to "<<newvalue<<endl;
    value = newvalue;
  } else { //not found, leave as it is assigned in called fnc
    //cout<<variablename<<" not found."<<endl;
  }
  property.insert(make_pair(variablename, value)); 
}
void declareProperty( const TString& variablename, bool& value) {
  int newvalue = read(variablename);
  propertyMap::iterator i = property.find(variablename);
  if( i != property.end() ){
    if( (int) i->second != value ) {
      warning()<<SUBVIOLETA<<"Trying to replace property "<< variablename<<"="
               << i->second <<" with "<< value<< ", ignored."<<ENDC<<endmsg;
    }
    return;
  }
  if( newvalue!=-12345 && newvalue != value ) { //found item to update
    cout<< " *** bool " <<variablename << " = "<<value
        <<"\t\tchanged to "<<newvalue<<endl;
    value = newvalue;
  } else { //not found, leave as it is assigned in called fnc
    //cout<<variablename<<" not found."<<endl;
  }
  property.insert(make_pair(variablename, value)); 
}
void declareProperty( const TString& variablename, TString& value) {
  TString newvalue = readString(variablename);
  propertyStringMap::iterator i = stringproperty.find(variablename);
  if( i != stringproperty.end() ){
    if( i->second != value ) {
      warning()<<SUBVIOLETA<<"Trying to replace property "<< variablename<<"="
               << i->second <<" with "<< value<< ", ignored."<<ENDC<<endmsg;
    }
    return;
  }
  if( newvalue!="" && newvalue != value ) { //found item to update
    cout<< " *** string " <<variablename << " = "<<value
        <<"\t\tchanged to "<<newvalue<<endl;
    value = newvalue;
  } else { //not found, leave as it is assigned in called fnc
    //cout<<variablename<<" not found."<<endl;
  }
  stringproperty.insert(make_pair(variablename, value)); 
}
