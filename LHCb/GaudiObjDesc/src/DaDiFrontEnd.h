// $Header: /afs/cern.ch/project/cvs/reps/lhcb/GaudiObjDesc/src/DaDiFrontEnd.h,v 1.5 2002-02-27 13:25:35 mato Exp $
#ifndef DADI_FRONTEND_H 
#define DADI_FRONTEND_H 1

#include "DaDiTools.h"
#include "DaDiPackage.h"

#include "dom/DOM_Node.hpp"

#include <iostream>
#include <list>

/** @class DDFE DaDiFrontEnd.h
 *  
 *
 *  @author Stefan Roiser
 *  @date   29/06/2001
 */


class DDFE
{
public:

 DDFE() {};
 ~DDFE() {};

static DaDiPackage* DaDiFrontEnd(char* filename);
//static void parseImport(DOM_Node node, DaDiPackage* gddPackage);
//static void parseImport(DOM_Node node, DaDiClass* gddClass);
//static void parsePackage(DOM_Node node, DaDiPackage* gddPackage);
//static void parseClass(DOM_Node node, DaDiClass* gddClass);
//static void parseBaseClass(DOM_Node node, DaDiClass* gddClass);
//static void parseEnum(DOM_Node node, DaDiClass* gddClass);
//static void parseMethod(DOM_Node node, DaDiClass* gddClass);
//static void parseConstructor(DOM_Node node, DaDiClass* gddClass);
//static void parseDestructor(DOM_Node node, DaDiClass* gddClass);
//static void parseAttribute(DOM_Node node, DaDiClass* gddClass);
//static void parseRelation(DOM_Node node, DaDiClass* gddClass);

};



#endif // DADI_FRONTEND_H
