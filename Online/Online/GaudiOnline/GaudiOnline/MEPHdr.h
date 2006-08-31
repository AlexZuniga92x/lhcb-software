#ifndef GAUDIONLINE_MEPHDR_H
#define GAUDIONLINE_MEPHDR_H
//	============================================================
//
//	MEPHdr.h
//	------------------------------------------------------------
//
//	Package   : GaudiOnline
//
//	Author    : Niko Neufeld 
//   //
//	===========================================================
#include "RTL/types.h"

#ifdef _WIN32
#pragma pack(push, MEP, 1)
#define PACKIT 
#else
#include <sys/types.h>
#define PACKIT __attribute__((__packed__))
#endif

/* according to EDMS xxx */
namespace LHCb
{

  struct PACKIT MEPHdr  {
	u_int32_t m_l0ID;
	u_int16_t m_nEvt;
	u_int16_t m_totLen;
	u_int32_t m_partitionID;
    };
  struct PACKIT MEPFrgHdr {
	u_int16_t m_l0IDlow;
	u_int16_t m_len;
    };
}
/* 
 * A MEP request is a minimal legal Ethernet packet
 * The length of the IP payload is hence 44 bytes
 */
#define MEP_REQ_LEN 44
struct PACKIT MEPReq  
{
  u_int8_t nmep;   /* number of meps requested */
  u_int8_t reserved[MEP_REQ_LEN - 1];
};

#ifdef _WIN32
#pragma pack(pop, MEP)
#endif // _WIN32

#endif /* GAUDIONLINE_MEPHDR_H */
