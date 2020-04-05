/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H


#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr();
   ~CirMgr();

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const; 

   // Member functions about circuit construction
   bool readCircuit(const string&);
   
   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   

   // Member functions about fraig
   void strash();
   void printFEC() const; 
   void splitFEC();
   void siminit();
   void clearSim();
   void printSim(unsigned) const; // print sim value of a gate;
   void writeSim(unsigned) ;
   void printFECPairs() const; // print 
   void printFEC_EQs(unsigned) const; // print equivalent gate of a gate;
   void fraig();
   
   //Intermediate member functions about circuit reporting before print
   void getFltIn(GateList& ) const;
   void getDefUnused(GateList& ) const;

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   
   void writeAag(ostream&) ;
   void writeGate(ostream&, CirGate*);

   CirGate* setUndef(unsigned ID);
private:
   // Helper function:
   void buildAIG();
   void buildAIGsub(CirGate*);
   void pushAIG(CirGate*);
   void propSim(bool);
   
   void Aag2Sat(SatSolver&);
   void FEC2Sat(SatSolver&, unsigned );
   void SatRun(SatSolver&, unsigned );
   void FraigSplitFEC();
   void reFEC();
   void FraigStrash();
   // Member variables:
   GateList                _gateList;           // Sorted in order of lineNo.
   IdList                  _idList;             // Mapping ID to lineNo.
   unsigned                _maxV;
   unsigned                _MaxDefLine;         // _idList[x] is UNDEF_GATE for x > _MaxDefLine
   unsigned                _PINum;
   unsigned                _PONum;
   unsigned                _AIGNum;
   vector<AIG*>            _AIGList;            // for print Netlist, OPTimize, writeAAG, dfsList
   
   // Member variables for FECs:z
   vector< set<unsigned>* >  _FECgrps;    
   ofstream                *_simLog;
   bool                     _Fraiged = false;   // boolean for fraig optimization
};

struct CirSort
{
   bool operator() (CirGate* a, CirGate* b)
   { return a->getID() < b->getID() ; }
   bool operator() (size_t a, size_t b)
   { return (toPtr(a))->getID() < (toPtr(b))->getID(); }
   bool operator() (set<unsigned>* a, set<unsigned>* b)
   { return *(a -> begin()) < *(b -> begin()); }
};


#endif // CIR_MGR_H
