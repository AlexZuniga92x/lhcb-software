// $Id: Config.h,v 1.2 2009/07/28 20:44:43 frankb Exp $
//====================================================================
//  Stomp
//--------------------------------------------------------------------
//
//  Package    : Stomp
//
//  Description: DIM enabled Stomp plugin for Apache
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /local/reps/lhcb/Online/Stomp/Stomp/Config.h,v 1.2 2009/07/28 20:44:43 frankb Exp $
#ifndef STOMP_CONFIG_H
#define STOMP_CONFIG_H 1

#ifdef _WIN32
#include "yvals.h"
#undef _GLOBAL_USING
#ifdef APR_DECLARE_EXPORT
#define STOMP_API_DECLARE  __declspec(dllexport)
#else
#define STOMP_API_DECLARE  __declspec(dllimport)
#endif
#endif

#endif /* STOMP_CONFIG_H */

