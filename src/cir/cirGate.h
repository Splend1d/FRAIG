/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#define toPtr(p)             \
(CirGate*)(p & ~size_t(0x1))
#define INV size_t(0x1)
using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   friend class CirMgr;
   CirGate(unsigned l, unsigned ID) : 
      _lineNo(l), _id(ID), _EQGate(ID*2){}//_fanoutList.rehash(10);_fanoutList.max_load_factor(1000);}
   virtual ~CirGate() {}

   // Basic access methods
   virtual string getTypeStr() const = 0;
   virtual GateType getType() const = 0;
   unsigned getLineNo() const { return _lineNo; }
   virtual bool isAig() const { return false; }
   unsigned getID() const { return _id; }
   unsigned getEQ() const { return _EQGate; } // for ciropt
   virtual void changeEQ(unsigned s) {_EQGate = s; } // for ciropt
   virtual bool noFanin() const = 0;
   virtual bool noFanout() const = 0;
   //virtual vector<size_t> getFanout() const =0;
   vector<size_t>* getFanoutPtr() {return &_fanoutList;}
   unsigned getFanoutSize() const {return _fanoutList.size();}

   // Printing functions
   virtual void getDFS(bool) = 0;
   virtual void reportGate() const = 0;
   virtual void reportFanin(int level, bool inv=false) = 0;
   virtual void reportFanout(int level, bool inv=false) = 0;
   virtual void setFanin(size_t fi=ULONG_MAX) = 0;
   virtual void setFanout(size_t fo) { 
      assert(fo != 0);
      _fanoutList.push_back(fo); 
   }
   void rmFanout(size_t fo) {
      for (unsigned i = 0;i < _fanoutList.size();++i) {
         if (_fanoutList[i] == fo){
            _fanoutList[i] = _fanoutList.back();
            _fanoutList.pop_back();
            break;
         }
      }
      for (unsigned i = 0;i < _fanoutList.size();++i) {
         if (_fanoutList[i] == fo){
            _fanoutList[i] = _fanoutList.back();
            _fanoutList.pop_back();
            break;
         }
      }
   }
   void printSim() const {
      for (int i = SIZE_T - 1; i > -1 ; --i) {
         char a = (char)((((size_t)(255) << (i*8))&_simVal)>>(i*8));
         //char a = (char)(_simVal>>(i*8));
         std::bitset<8> x(a);
         cout << x;
         if (i != 0)cout << "_";
      }
      //cout << endl;
      //std::bitset<64> x(_simVal);
      //cout << x;
   }
   virtual void printFECgrp() const {
      if (_FECgrp == 0) return;
      set<unsigned>::iterator it_uint;
      bool anchor = _FECgrp & INV;
      bool flag = false; //DEBUG
      set<unsigned>* s = (set<unsigned>*)toPtr(_FECgrp);
      for (it_uint = s -> begin(); it_uint != s -> end(); ++it_uint) {
         if ((*it_uint)/2 == getID()) {
            flag = true;
            continue;
         }
         cout << " ";
         if ((*it_uint)%2 != anchor) cout << "!";
         cout <<(*it_uint)/2;
      }
      //assert(flag == true);
   }
   
   static void resetPrintOrder() { _printOrder = 0; }
   static void setGlobalref() { ++_globalref; }
   static void resetIndent() { _indent = ""; }
   void setRef() const { _ref = _globalref; }
   unsigned getRef() const { return _ref; }
   bool refMatch() const { return _ref == _globalref; }

private:

protected:  
   unsigned                _lineNo;
   unsigned                _id;
   size_t                  _EQGate;
   static unsigned         _globalref;
   static unsigned         _printOrder;
   static string           _indent;
   mutable unsigned        _ref;
   vector<size_t>_fanoutList;
   size_t                  _simVal = 0;
   size_t                  _FECgrp = 0;
};

class PI : public CirGate
{
public:
   PI(unsigned l, unsigned ID) : CirGate(l,ID) {}
   //arg = { lineNo, ID, fanout }
   ~PI() {}
   string getTypeStr() const { return "PI"; }
   GateType getType() const {return PI_GATE;}
   void getDFS(bool);
   void reportGate() const;
   void changeEQ(unsigned s) {assert(0);} // for ciropt
   void reportFanin(int level, bool inv); 
   void reportFanout(int level, bool inv);
   void setSymbol(string s) { _symbol = s; }
   void setFanin(size_t fi) { assert(0); }
   //void setFanout(size_t fo);
   bool noFanin() const { return false; }
   bool noFanout() const { 
      return (_fanoutList.empty()); 
   }
   //vector<size_t> getFanout() const {return _fanoutList;}
   string getSymbol() const { return _symbol; }
private:
   string                  _symbol;
   
};

class PO : public CirGate
{
public:
   PO(unsigned l, unsigned ID) : CirGate(l, ID), _fanin(ULONG_MAX){}
   //arg = { lineNo, ID, fanin }
   ~PO() {}
   string getTypeStr() const { return "PO"; }
   GateType getType() const {return PO_GATE;}
   void getDFS(bool);
   void reportGate() const;
   void reportFanin(int level, bool inv);
   void reportFanout(int level, bool inv);
   void setSymbol(string s) { _symbol = s; }
   void changeFinFout(size_t);
   void setFanin(size_t fi);
   void setFanout(size_t fo) { assert(0); }
   bool noFanin() const; 
   bool noFanout() const { return false; }
   size_t getFanin() const { return _fanin; }
   //vector<size_t> getFanout() const {assert(0);}
   //void removeFanout(size_t key) {assert(0);}
   string getSymbol() const { return _symbol; }
private:
   string                  _symbol;
   size_t                  _fanin;
};

class AIG : public CirGate
{
public:
   AIG(unsigned l, unsigned ID) : CirGate(l, ID)
   { _fanin[0] = _fanin[1] = ULONG_MAX;}
   ~AIG() {}
   bool isAig() const { return true; }
   string getTypeStr() const { return "AIG"; }
   GateType getType() const {return AIG_GATE;}
   void getDFS(bool);
   void reportGate() const;
   void reportFanin(int level, bool inv);
   void reportFanout(int level, bool inv);
   void changeFinFout(int,size_t);
   void setFanin(size_t fi);
   //void setFanout(size_t fo);
   bool noFanin() const ; // DEPRECATE
   bool noFanout() const { 
      return (_fanoutList.empty()); 
   }
   size_t getFanin(int i=0) const { return _fanin[i]; }
   //vector<size_t> getFanout() const {return _fanoutList;}
   
private:
   size_t                  _fanin[2];
   
   
};

class Const : public CirGate
{
public:
   Const() : CirGate(0,0){}
   string getTypeStr() const { return "CONST0"; }
   GateType getType() const {return CONST_GATE;}
   void getDFS(bool);
   void reportGate() const;
   void reportFanin(int level, bool inv);
   void reportFanout(int level, bool inv);
   void setFanin(size_t fi) { assert(0); }
   
   bool noFanin() const { return false; }
   bool noFanout() const { 
      return (_fanoutList.empty()||\
      all_of(_fanoutList.begin(), _fanoutList.end(), [](int i) { return i==0; })); 
   }
   //vector<size_t> getFanout() const {return _fanoutList;}
private:
};

class Undef : public CirGate
{
public:
   Undef(unsigned ID) : CirGate(0,ID) {}
   string getTypeStr() const { return "UNDEF"; }
   GateType getType() const {return UNDEF_GATE;}
   void getDFS(bool);
   void reportGate() const;
   void reportFanin(int level, bool inv);
   void reportFanout(int level, bool inv);
   void setFanin(size_t fi) { assert(0); }
   //void setFanout(size_t fo) { _fanoutList.insert(fo); }
   bool noFanin() const { return true; }
   bool noFanout() const { 
      return (_fanoutList.empty()||\
      all_of(_fanoutList.begin(), _fanoutList.end(), [](int i) { return i==0; })); 
   }
   //vector<size_t> getFanout() const {return _fanoutList;}
private:
};

#endif // CIR_GATE_H
