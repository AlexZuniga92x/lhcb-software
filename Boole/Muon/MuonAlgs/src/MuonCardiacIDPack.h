#ifndef MuonAlgs_MuonCardiacIDPack_H 
#define MuonAlgs_MuonCardiacIDPack_H 1

namespace MuonCardiacIDPack{

static const unsigned int bitReadout=1;
static const unsigned int bitPhChIDX=8;
static const unsigned int bitPhChIDY=5;
static const unsigned int bitChamber =8;
static const unsigned int bitQuadrant=2;
static const unsigned int bitRegion=2;
static const unsigned int bitStation=3;


static const unsigned int shiftReadout  = 0 ;
static const unsigned int shiftPhChIDX  = shiftReadout+bitReadout ;
static const unsigned int shiftPhChIDY  = shiftPhChIDX  + bitPhChIDX ;
static const unsigned int shiftChamber  = shiftPhChIDY  + bitPhChIDY ;
static const unsigned int shiftQuadrant = shiftChamber  + bitChamber ;
static const unsigned int shiftRegion   = shiftQuadrant + bitQuadrant ;
static const unsigned int shiftStation  = shiftRegion   + bitRegion ;

 
static const unsigned int maskStation =  ( ( ( (unsigned int) 1 ) << bitStation  ) - 1  ) << shiftStation ;     
static const unsigned int maskRegion =  ( ( ( (unsigned int) 1 ) << bitRegion ) - 1  ) << shiftRegion ;     
static const unsigned int maskQuadrant =  ( ( ( (unsigned int) 1 ) << bitQuadrant  ) - 1  ) << shiftQuadrant ;     
static const unsigned int maskChamber =  ( ( ( (unsigned int) 1 ) << bitChamber  ) - 1  ) << shiftChamber;     
static const unsigned int maskPhChIDX =  ( ( ( (unsigned int) 1 ) << bitPhChIDX  ) - 1  ) << shiftPhChIDX ;     
static const unsigned int maskPhChIDY =  ( ( ( (unsigned int) 1 ) << bitPhChIDY  ) - 1  ) << shiftPhChIDY ;     
static const unsigned int maskReadout =  ( ( ( (unsigned int) 1 ) << bitReadout  ) - 1  ) << shiftReadout ;     


inline unsigned int getBit(unsigned int  packedValue, unsigned int shift,
unsigned int mask){
 return ((packedValue&mask)>>shift);
}

inline void  setBit(unsigned int &origValue,
                     unsigned int shift, unsigned int mask,
                     unsigned int bitValue){
 unsigned int                           store   = origValue;
 unsigned int       result=(((bitValue<<shift)&mask)| (store&~mask));
  origValue=result;
} 

}
#endif
