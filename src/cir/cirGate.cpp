/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"


using namespace std;

extern CirMgr *cirMgr;

unsigned CirGate::_globalref = 0;
unsigned CirGate::_printOrder = 0;
string CirGate::_indent = "";

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   CIRGATE member functions         */
/**************************************/

//void CirGate::removeFanout(size_t key) {
//   _fanoutList.erase(key);
//}

/**************************************/
/*        PI member functions         */
/**************************************/

// Format:
// [order] PI ID (_symbol)
// e.g.
// [0] PI 1 (2GAT)
// 1. Base case, just print out the current gate.
// 2. Get _printOrder to decide the number to print.
void PI::getDFS(bool out) 
{
   if (_ref == _globalref) return;
   if (out) {
      cout << "[" << _printOrder++ << "] PI  " << _id;
      if (_symbol.size()) cout << " (" << _symbol << ")";
      cout << endl;
   } else {
      //cirMgr -> _dfsList.push_back(_id);
   }
   _ref = _globalref;
}

void PI::reportGate() const
{
   stringstream ss;
   ss << getTypeStr() << "(" << _id << ")";
   if (_symbol.size())
      ss << "\"" << _symbol << "\"";
   ss << ", line " << _lineNo+1;
   cout << "================================================================================" << endl;
   cout << "= "; 
   //cout << setw(47) << left;
   cout << ss.str() << endl;//"=\n";
   cout << "= FECs:";
   cout << endl;
   cout << "= Value: ";
   printSim();
   cout <<endl; 
   cout << "================================================================================" << endl;
}

void PI::reportFanin(int level, bool inv)
{
   assert(level>=0);
   cout << _indent;
   if (inv) cout << "!";
   cout << "PI " << _id << endl;
}

// 1. Print PI (ID)
// 2. Keep a static member "string _indent" and properly handle it
//    everytime calling and returning a function.
// 3. Set the _ref to _globalref so that next time it's call,
//    it will not print out its fanouts again. 
// 4. Check its _ref, if it's not printed yet, call the report function
//    of next level. Otherwise, print a " (*) ".
// 5. When leaving a function, decrement the "indent".
void PI::reportFanout(int level, bool inv)
{
   assert(level>=0);
   assert(_indent == "");
   setGlobalref();
   if (inv) cout << "!";
   cout << "PI " << _id;
   if (level == 0)
   { cout << endl; return; }
   cout << endl;
   _indent += "  ";
   for (auto itr = _fanoutList.begin(); itr != _fanoutList.end(); ++itr){
      bool inv_next = (*itr & INV);
      (toPtr(*itr))->reportFanout(level-1, inv_next);
   }
   _indent.resize(_indent.size()-2);

}

// 1. If fo is assigned, save this ID temperarily to _fanout.
//    (Must explicitly copy the value of pointer)
// 2. If fo is not assigned (i.e. as default value=ULONG_MAX), convert
//    the currently saved value through cirMgr to cirGate*.

//DEPRECATED
//void PI::setFanout(size_t fin)
//{
//   _fanoutList.insert(fin);
//   //std::sort(_fanoutList.begin(), _fanoutList.end(), CirSort());
//}

/**************************************/
/*        PO member functions         */
/**************************************/

// Format:
// [order] PO ID _faninID (_symbol)
// e.g.
// [3] PO 11 3 (GATE$3)
// 1. Call the getDFS(out) of _fanin.
// 2. Get _globalref to determine the printing order.
// 3. Increment _global ref.
// 4. Detect it whether its fanin is floating, if so,
//    put a '*' sign before '!' if existing.
// 3. if out is set to true, print the gate, else record it in the DFS list
void PO::getDFS(bool out)
{
   if (_ref == _globalref) return;
   CirGate* fin = toPtr(_fanin);
   fin->getDFS(out);
   if (out) {
      bool inv = (_fanin & INV);
      cout << "[" << _printOrder++ << "] PO  " << _id << " ";
      if (fin->getType() == UNDEF_GATE) cout << "*";
      if (inv) cout << "!";
      cout << fin->getID();
      if (_symbol.size()) cout << " (" << _symbol << ")";
      cout << endl;
   } else {
      //cirMgr ->_dfsList.push_back(_id);
   }
   _ref = _globalref;
}

void PO::reportGate() const
{
   stringstream ss;
   ss << getTypeStr() << "(" << _id << ")";
   if (_symbol.size())
      ss << "\"" << _symbol << "\"";
   ss << ", line " << _lineNo+1;
   cout << "================================================================================" << endl;
   cout << "= "; 
   //cout << setw(47) << left;
   cout << ss.str() <<endl;//<< "=\n";
   cout << "= FECs:";
   cout << endl;
   cout << "= Value: ";
   printSim();
   cout <<endl; 
   cout << "================================================================================" << endl;
}

void PO::reportFanin(int level, bool inv)
{
   assert(level>=0);
   assert(_indent=="");
   setGlobalref();
   if (inv) cout << "!";
   cout << getTypeStr() << " " << _id;
   if (level == 0)
   { cout << endl; return; }
   _indent += "  ";
   cout << endl;
   bool inv_next = (_fanin & INV);
   ((CirGate*)(_fanin & ~INV))->reportFanin(level-1, inv_next);
   _indent.resize(_indent.size()-2);
}

void PO::reportFanout(int level, bool inv)
{
   assert(level>=0);
   cout << _indent;
   if (inv) cout << "!";
   cout << "PO " << _id << endl;
}
void PO::changeFinFout(size_t gid)
{
   _fanin = gid;
   size_t p = gid & INV ? (size_t)this|INV : (size_t)this;
   (toPtr(gid))->setFanout(p);
}
// 1. If fi is assigned, store this value temperarily into _fanin.
// 2. If fi is not assigned (i.e. as its default value=ULONG_MAX), 
//    convert the _fanin through cirMgr into cirGate*, 
//    and call _fanin->setFanout recursively.
// 3. If fi==ULONG_MAX while _fanin is still not assigned, do nothing.
void PO::setFanin(size_t fi)
{
   if (fi != ULONG_MAX){
      assert(_fanin==ULONG_MAX);
      _fanin = (size_t)fi; 
   } else {
      assert(_fanin != ULONG_MAX);
      
      bool inv = (_fanin%2 != 0);
      CirGate* t = cirMgr->getGate(_fanin/2);
      if (t == 0)
         _fanin = (size_t)cirMgr->setUndef(_fanin/2);
      else
         _fanin = (size_t)t;
      size_t p = inv? (size_t)this|INV : (size_t)this;
      ((CirGate*)_fanin)->setFanout(p);
      if (inv) _fanin |= INV;
   }
}

bool PO::noFanin() const
{
   return (toPtr(_fanin))->getTypeStr() == "UNDEF";
}
/**************************************/
/*        AIG member functions        */
/**************************************/

void AIG::getDFS(bool out)
{
   if (_ref == _globalref) return;
   for (size_t i=0; i<2; ++i)
      (toPtr(_fanin[i]))->getDFS(out);
   if (out) {
      cout << "[" << _printOrder++ << "] AIG " << _id;
      for (size_t i=0; i<2; ++i){
         cout << " ";
         if ( (toPtr(_fanin[i]))->getType() == UNDEF_GATE )
            cout << "*";
         if ( _fanin[i] & INV )
            cout << "!";
         cout << (toPtr(_fanin[i]))->getID();
      }
   } else {
      //cirMgr ->_dfsList.push_back(_id);
   }
   cout << endl;
   _ref = _globalref;
}

void AIG::reportGate() const
{
   stringstream ss;
   ss << getTypeStr() << "(" << _id << "), line " << _lineNo+1;
   cout << "================================================================================" << endl;
   cout << "= "; 
   //cout << setw(47) << left;
   cout << ss.str() << endl;
   cout << "= FECs:";
   printFECgrp();
   cout << endl;
   cout << "= Value: ";
   printSim();
   cout <<endl;  
   cout << "================================================================================" << endl;
}

void AIG::reportFanin(int level, bool inv)
{
   assert(level>=0);
   if (_indent == "") setGlobalref();
   cout << _indent;
   if (inv) cout << "!";
   cout << getTypeStr() << " " << _id;
   if (level == 0)
   { cout << endl; return; }
   if (_ref != _globalref){
      _indent += "  ";
      cout << endl;
      for (size_t i=0; i<2; ++i){
         bool inv_next = (_fanin[i] & INV);
         (toPtr(_fanin[i]))->reportFanin(level-1, inv_next);
      }
      _ref = _globalref;
      _indent.resize(_indent.size()-2);
   } else {
      cout << " (*)" << endl;
   }
}

void AIG::reportFanout(int level, bool inv)
{
   assert(level>=0);
   if (_indent == "") setGlobalref();
   cout << _indent;
    if (inv) cout << "!";
    cout << "AIG " << _id;
    if (level == 0 || noFanout())
    { cout << endl; return; }
    if (_ref != _globalref){
       _indent += "  ";
       cout << endl;
       for (auto itr = _fanoutList.begin(); itr != _fanoutList.end(); ++itr){
         assert(*itr != 0);
         bool inv_next = (*itr & INV);
         (toPtr(*itr))->reportFanout(level-1, inv_next);
      }
       _indent.resize(_indent.size()-2);
       _ref = _globalref;
    } else {
       cout << " (*)" << endl;
    }
 }
 void AIG::changeFinFout(int i, size_t gid) 
 {
   _fanin[i] = gid;
   size_t p = gid & INV ? (size_t)this|INV : (size_t)this;
   (toPtr(gid))->setFanout(p);
 }
 // 1. If fi is assigned, store this number.
 //    Note that the size of fanin is exactly two.
 //    Check which position the input value should be placed.
 // 2. If fi is not assigned (i.e. as its default value ULONG_MAX), convert
 //    it through cirMgr into cirGate*.
 void AIG::setFanin(size_t fi)
 {
    if (fi != ULONG_MAX){
       if (_fanin[0] == ULONG_MAX) _fanin[0] = (size_t)fi;
       else if (_fanin[1] == ULONG_MAX) _fanin[1] = (size_t)fi;
       else assert(0);
    } else {
      for (size_t i=0;i<2;i++){
         assert(_fanin[i] != ULONG_MAX);
         bool inv = (_fanin[i]%2 != 0);
         CirGate* t = cirMgr->getGate(_fanin[i]/2);
         if (t == 0)
            _fanin[i] = (size_t)(cirMgr->setUndef(_fanin[i]/2));
         else 
            _fanin[i] = (size_t)t;
         size_t p = inv? (size_t)this|INV : (size_t)this;
         ((CirGate*)_fanin[i])->setFanout(p);
         if (inv) _fanin[i] |= INV;
      }
   }
}

// This function is called by its fanout, just store the value.
// DEPRECATED
//void AIG::setFanout(size_t fo)
//{
//   _fanoutList.insert(fo);
//   //std::sort(_fanoutList.begin(), _fanoutList.end(), CirSort());
//}

bool AIG::noFanin() const
{
   return ( (toPtr(_fanin[0]))->getTypeStr()=="UNDEF" ||
            (toPtr(_fanin[1]))->getTypeStr()=="UNDEF"   );
}


/**************************************/
/*       Const member functions       */
/**************************************/
void Const::getDFS(bool out) 
{
   assert(_fanoutList.size() != 0);
   if (_ref == _globalref) return;
   if (out)
      cout << "[" << _printOrder++ << "] CONST0" << endl;
   else 
      ;//cirMgr ->_dfsList.push_back(0);
   
   _ref = _globalref;
}

void Const::reportGate() const
{
   cout << "================================================================================" << endl;
   cout << "= ";
   //cout << setw(47) << left;
   cout << "CONST(0), line 0" << endl; 
   cout << "= FECs:";
   printFECgrp();
   cout << endl;
   cout << "= Value: ";
   printSim();
   cout <<endl; 
   cout << "================================================================================" << endl;
}

void Const::reportFanin(int level, bool inv)
{
   assert(level>=0);
   cout << _indent;
   if (inv) cout << "!";
   cout << "CONST 0" << endl;
}

void Const::reportFanout(int level, bool inv)
{
   assert(level>=0);
   assert(_indent=="");
   setGlobalref();
   if (inv) cout << "!";
   cout << "CONST " << _id;
   if (level == 0 || noFanout())
   { cout << endl; return; }
   _indent += "  ";
   cout << endl;
   for (auto itr = _fanoutList.begin(); itr != _fanoutList.end(); ++itr){
      bool inv_next = (*itr & INV);
      (toPtr(*itr))->reportFanout(level-1, inv_next);
   }
   _indent.resize(_indent.size()-2);
}

/**************************************/
/*       Undef member functions       */
/**************************************/
void Undef::getDFS(bool out) 
{
   assert(_fanoutList.size() != 0);
   if (_ref == _globalref) return;
   if (out)
      ;//cout << "[" << _printOrder++ << "] UNDEF" << endl;
   else 
      ;//cirMgr ->_dfsList.push_back(_id);
   _ref = _globalref;
}
void Undef::reportGate() const
{
   stringstream ss;
   ss << "UNDEF(" << _id << "), line 0";
   cout << "================================================================================" << endl;
   cout << "= ";
   //cout << setw(47) << left;
   cout << ss.str() << endl;//"=" << endl;
   cout << "= FECs:";
   cout << endl;
   cout << "= Value: ";
   printSim();
   cout <<endl; 
   cout << "================================================================================" << endl;
}

void Undef::reportFanin(int level, bool inv)
{
   assert(level>=0);
   cout << _indent;
   if (inv) cout << "!";
   cout << "UNDEF " << _id << endl;
}

void Undef::reportFanout(int level, bool inv)
{
   assert(level>=0);
   assert(_indent=="");
   setGlobalref();
   if (inv) cout << "!";
   cout << "UNDEF " << _id;
   if (level == 0 || noFanout())
   { cout << endl; return; }
   _indent += "  ";
   cout << endl;
   for (auto itr = _fanoutList.begin(); itr != _fanoutList.end(); ++itr){
      bool inv_next = (*itr & INV);
      (toPtr(*itr))->reportFanout(level-1, inv_next);
   }
   _indent.resize(_indent.size()-2);
}
