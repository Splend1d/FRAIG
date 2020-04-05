/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
enum CirSimError {
   LENGTH_MISMATCH,
   NON_BOOL_IN,

   DUMMY_END
};
vector<string> buf;
unordered_map<size_t, set<unsigned>*> simval_map_set_init;
RandomNumGen rG(0);
bool FECsplitted = false;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static string errStr;
static int errInt;

static set< set<unsigned> >::iterator it_IdSet;
static vector<set<unsigned>*>::iterator it_ptr;
static set<unsigned>::iterator it_gateid;
static unordered_map<size_t, set<unsigned>*>::iterator it_size_ptr;

static bool
simError(CirSimError err)
{
   switch (err) {
      case LENGTH_MISMATCH:
         cerr << "\nError: Pattern(" << errStr <<") length(" << errStr.size() 
              << ") does not match the number of inputs("
              << errInt << ") in a circuit!!" << endl;
         break;
      case NON_BOOL_IN:
         cerr << "Error: Pattern("<< errStr <<") contains a non-0/1 character('"<<errStr[errInt]<<"')." << endl;
      break;
      default: break;
   }
   cout << "0 patterns simulated." << endl;
   return false;
}

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
   buildAIG();
   bool init;
   //cout <<"DEBUG size of FEC groups " <<  _FECgrps.size() << endl;
   if (_FECgrps.empty()) init = true;
   else init = false;
   if (init) siminit();
   unsigned lvl = 0; //* stopping criteria
   unsigned gate = _PINum > 12 ? 20 : (unsigned)(2 << _PINum)/32 + 1;  //* stopping criteria
   //cout << "DEBUG: max tolerance # of failures is " << gate << endl;
   bool strict = (_PINum <= 12); // if strict, lvl == counter, dont care sim results
   unsigned n_ptrns = 0;
   while (lvl < gate) {
      for (unsigned i = 0; i < _PINum; ++i){
         size_t sim_temp = 0;
         for (unsigned bit = 0; bit < SIZE_T * 8 ; bit += 32) {
            sim_temp |= (size_t)rG(INT_MAX) << bit;
            //* first bit always 0 in int_max, so also random this bit with boolean
            sim_temp |= (size_t)rG(2) << (bit + 31); 
         }
         //cout << "DEBUG generated a pattern: " << sim_temp << endl;
         _gateList[i+1] -> _simVal = sim_temp;
      } // successfully added a series of entry with 64/32 parellel bits
      if (_simLog != 0) {
         for (unsigned pi = 0; pi < _PINum; ++pi){
            size_t tempVal = _gateList[pi+1] -> _simVal;
            for (unsigned i = 0; i < SIZE_T * 8; ++i) {
               if (pi == 0) buf.push_back("");
               buf[i] += (tempVal & (1ULL << i) ? '1' : '0');
            }
         }
         for (unsigned i = 0; i < SIZE_T * 8; ++i) {
            buf[i] += " ";
         }
      }
      
      //* start simulation
      //cout << "DEBUG 64 units done epoch : " <<n_ptrns/64<<endl;
      propSim(init);
      writeSim(SIZE_T*8);
      init = false;
      if (!FECsplitted || strict) lvl ++;
      else FECsplitted = false, lvl = 0;
      n_ptrns += SIZE_T * 8;
      //TODO : stopping criteria
      }
   cout << n_ptrns<<" patterns simulated." << endl;
   std::sort(_FECgrps.begin(), _FECgrps.end(),CirSort());
   clearSim();
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   
   if(!patternFile.is_open()) return;
   buildAIG(); 
   string temp_str;
   //size_t sim_ptrn[_PINum] = {0};
   //vector<size_t> _FECList = _FECList;
   bool init;
   //cout <<"DEBUG size of FEC groups " _FECgrps.size() << endl;
   if (_FECgrps.empty()) init = true;
   else init = false;
   if (init) siminit();
   unsigned n_ptrns = 0;
   while(patternFile >> temp_str) {
      if(temp_str.size() != _PINum && temp_str.size() != 0) {
         errStr = temp_str;
         errInt = _PINum;
         simError(LENGTH_MISMATCH);
         clearSim();
         return;
      } 
      n_ptrns ++;
      for (unsigned i = 0; i < _PINum; ++i){
         //cout << "DEBUG checking pattern "<< (n_ptrns) <<" of input " << i << endl;
         if (n_ptrns % (SIZE_T*8) == 1) _gateList[i+1] -> _simVal = 0;
         if (temp_str[i] == '1'){
            //cout << "DEBUG before add ptrn: " << (void*)_gateList[i+1] -> _simVal << endl;
            _gateList[i+1] -> _simVal |= ((size_t)1<<((n_ptrns+SIZE_T * 8-1)%(SIZE_T * 8)));
            //cout << "DEBUG after add ptrn: " << (void*)_gateList[i+1] -> _simVal << endl;
         }
         else if (temp_str[i] != '0'){
            errStr = temp_str;
            errInt = i;
            simError(NON_BOOL_IN);
            clearSim();
            return;
         }
         
      }//* sucessfully added an entry
      if (_simLog != 0) buf.push_back(temp_str + ' ');
      
      if (n_ptrns % (SIZE_T*8) == 0) {
         //* start simulation
         //cout << "DEBUG 64 units done epoch : " <<n_ptrns/64<<endl;
         propSim(init);
         writeSim(SIZE_T*8);
         init = false;
         
      } 
   }
   //* there is left pattern unsimulated
   if (n_ptrns % (SIZE_T*8)) {
      //* start simulation
      propSim(init);
      writeSim(n_ptrns % (SIZE_T*8));
      init = false;
      
   }
   //DEBUG
   //for (unsigned i = 0; i < _PINum; ++i){
   //   cout << "DEBUG pattern of gate " << i << " is " <<  (void*)_gateList[i+1] -> _simVal <<endl;
   //}
   std::sort(_FECgrps.begin(), _FECgrps.end(),CirSort());
   cout << n_ptrns<<" patterns simulated." << endl;
   clearSim();
}

/*************************************************/
/*   Private member functions about Simulation   */
/**********************unordered_map***************************/
void
CirMgr::propSim(bool init){
   
//   for (unsigned i = 0 ; i < _PINum; ++i) {
//      //CirGate* g = _gateList[i+1];
//      _FECList[i] = sim_ptrn[i];
//      sim_ptrn[i] = 0;
//   } 
   
   //cout << "DEBUG propSim called " ;
   //cout <<  "init " << init << endl;
   
   for (size_t i = 0; i < _AIGList.size(); ++i) {
      unsigned thisid = _AIGList[i] -> getID();
      //cout << "DEBUG propogating AIG with id " << _AIGList[i] -> getID() << endl;
      //* fanin 1
      CirGate* g1 = toPtr(_AIGList[i] -> getFanin(0));
      unsigned g1ID = g1 -> getID();
      bool g1_inv = _AIGList[i] -> getFanin(0) & INV; // if the gate is inverse
      size_t g1FECValEQ = g1 -> _simVal ;
      if (g1_inv) g1FECValEQ = ~g1FECValEQ;
      
      //* fanin 2
      CirGate* g2 = toPtr(_AIGList[i] -> getFanin(1));
      unsigned g2ID = g2 -> getID();
      bool g2_inv = _AIGList[i] -> getFanin(1) & INV; // if the gate is inverse
      size_t g2FECValEQ = g2 -> _simVal ;
      if (g2_inv) g2FECValEQ = ~g2FECValEQ;
      
      size_t simval = g1FECValEQ & g2FECValEQ;
      _AIGList[i] -> _simVal = simval;
      //* construct FECgrps if not constructed yet
      if (init) {
         bool sim_inv = simval & INV;
         size_t simval_key = sim_inv ? ~simval : simval; 
         it_size_ptr = simval_map_set_init.find(simval_key);
         
         if (it_size_ptr == simval_map_set_init.end()) { // new sim pattern found
            //* new FECgrp
            set<unsigned>* v = new set<unsigned>;
            _FECgrps.push_back(v);
            //cout << "DEBUG new FEC group found" << _FECgrps.size() << endl;
            //* update hash map
            simval_map_set_init.insert({simval_key,v});
         } 
         //* insert new gateid to FECgrp set   
         simval_map_set_init[simval_key] -> insert(_AIGList[i] -> getID() * 2 + (unsigned)sim_inv);
         //cout << "DEBUG inserted " << ((size_t)_AIGList[i] | (size_t)sim_inv) << endl;
         _AIGList[i]-> _FECgrp = (size_t)simval_map_set_init[simval_key] | (size_t)sim_inv;
      } else {
         ;
      }
         
   }
   
   
   //* split FEC groups
   if (!init) splitFEC();
   //* delete single FEC group gates
   reFEC();
   cout<<"\r"<<flush;
   //DEBUG
   //printFECPairs();
   for (unsigned i = _PINum +1; i < _PINum + _PONum + 1; ++i) {
      PO* g = dynamic_cast<PO*>(_gateList[i]);
      unsigned thisid = g -> getID();
      //* fanin 1
      CirGate* g1 = toPtr(g -> getFanin());
      unsigned g1ID = g1 -> getID();
      bool g1_inv = g -> getFanin() & INV; // if the gate is inverse
      size_t g1FECValEQ = g1_inv ?  ~ g1 -> _simVal : g1 -> _simVal ;
      g -> _simVal = g1FECValEQ;
      for (unsigned i = 0; i < buf.size(); ++i) {
         buf[i] += (g1FECValEQ & (1ULL << i) ? '1' : '0');
      }
   }
}

void
CirMgr::writeSim(unsigned e) {
   
   if(_simLog == 0) return;
   for(unsigned i = 0; i < e; ++i){
      *_simLog << buf[i] << endl;
   }
   buf.clear();
}

void
CirMgr::siminit(){
   //* initialize fecgrps 
   set<unsigned>* v = new set<unsigned>;
   _FECgrps.push_back(v);
   v -> insert(0);
   
   //* initialize simval comparing map
   simval_map_set_init.clear();
   simval_map_set_init.insert({0,_FECgrps[0]}); 
}

void
CirMgr::splitFEC(){
   //cout << "DEBUG splitFEC called" <<endl;
   unsigned grps = _FECgrps.size();
   unsigned grpidx = 0;
   
   for (it_ptr = _FECgrps.begin(); grpidx != grps; ++it_ptr, ++ grpidx) {
      
      unordered_map<size_t, set<unsigned>*> temp_simval_map_set;
      temp_simval_map_set.clear();
      set<unsigned>* anchor;
      //unsigned fixed_size = (*it_ptr) -> size();
      for (it_gateid = (*it_ptr) -> begin(); it_gateid != (*it_ptr) -> end(); ) {
         CirGate* thisgate = _gateList[_idList[(*it_gateid)/2]];
         size_t simval_key = thisgate ->_simVal;
         bool inv = (*it_gateid) % 2;
         if (inv) simval_key = ~simval_key;
         
         
         if (it_gateid == (*it_ptr) -> begin()){ //* first element in this FEC group, set tone
            //anchor = simval_key & INV;
            temp_simval_map_set.insert({simval_key,*it_ptr});
            anchor = *it_ptr;
            it_gateid++;
            //cout << "DEBUG constructing base of this FECgrp" << *it_ptr;
            //cout << " with " << fixed_size << " elements " << endl;
            continue;
            
         }
         it_size_ptr = temp_simval_map_set.find(simval_key);
         if (it_size_ptr == temp_simval_map_set.end()) { // new sim pattern found
            
            //* new FECgrp
            set<unsigned>* v = new set<unsigned>;
            _FECgrps.push_back(v);
            //cout << "DEBUG new FEC group found" << _FECgrps.size() << endl;
            
            //* update hash map
            temp_simval_map_set.insert({simval_key,v});
            
            //* FEC splitted
            FECsplitted = true;
         } else if (it_size_ptr -> second == anchor) {
            //cout << "DEBUG gate "<<(*it_gateid) /2 <<" in FEC group has not changed" << endl;
            it_gateid++;
            continue;
         } 
            
         //* remove gate original FECgrp set
         //cout << "DEBUG erasing uint" << *it_gateid << " with sim value "<< simval_key << endl;
         temp_simval_map_set[simval_key] -> insert(thisgate -> getID() * 2 + (unsigned)inv);
         //* append gate to the FECgrp set
         thisgate -> _FECgrp = (size_t)temp_simval_map_set[simval_key] | (size_t)inv;
         (*it_ptr) -> erase(it_gateid++);
         
         //cout << "DEBUG new size of this FECgrp is " << (*it_ptr) -> size() << endl;
         //cout << "DEBUG remaining size : " << fixed_size << endl;
      }
   }
   //cout << "DEBUG splitFEC end" <<endl; 
}

void 
CirMgr::reFEC() {
   // delete 1s
   for (int i = 0; i < _FECgrps.size(); ++i) { // new sim pattern found
      if (_FECgrps[i] -> size() == 1) {
         //auto it = _FECgrps[i] -> begin();
         _gateList[_idList[*(_FECgrps[i] -> begin())/2]] -> _FECgrp = 0;
         delete _FECgrps[i];
         _FECgrps[i] = _FECgrps.back();
         _FECgrps.pop_back();
         i--;
         
      }
   }
   cout << "Total #FEC Group = " << _FECgrps.size();
}
void 
CirMgr::clearSim() {
   buf.clear();
   simval_map_set_init.clear();
}


