/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H


#include <vector>
#include "myHashMap.h"
#include <climits>
#include <cassert>
#include <algorithm>
#include <map> 
#include <unordered_map> 
#include <unordered_set>
#include <set>
#include <bitset>
#define SIZE_T  sizeof(size_t)
using namespace std;

// TODO: define your own typedef or enum
class CirGate;
class CirMgr;
typedef vector<CirGate*>           GateList;
typedef vector<unsigned>           IdList;
enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,

   TOT_GATE
};

class SatSolver;

#endif // CIR_DEF_H
