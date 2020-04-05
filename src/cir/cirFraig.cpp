/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
struct pair_hash {
   template <class T1, class T2>
   std::size_t operator () (const std::pair<T1,T2> &p) const {
      auto h1 = std::hash<T1>{}(p.first);
      auto h2 = std::hash<T2>{}(p.second);
      
      //size_t a = h1^( h2 + 0x9e3779b9 + (h1<<6) + (h1>>2) );
      //cout <<" hash of " << h1 <<" and " << h2 << " is " << a << endl;
      
      return h1^( h2 + 0x9e3779b9 + (h1<<6) + (h1>>2) ); // boost method
   }
};
typedef pair<unsigned, unsigned> finskey;
typedef unordered_map< finskey,unsigned,pair_hash> AIGmap;
typedef unordered_map< unsigned,unsigned > EQmap;

finskey k;
unordered_map<CirGate*, Var> _gateVars;
unordered_set<CirGate*> _todelete;
unordered_map<CirGate*, size_t> _goodsim;
unordered_map<unsigned, unsigned> _AIGOrder; // id -> dfs_position;
unordered_map<set<unsigned>*, unsigned> _FECAnchor;//set -> *lit*;

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unordered_set<CirGate*>::iterator it_gate, it2_gate;
static unordered_multiset<size_t>::iterator it_size;
static AIGmap::iterator it_pair;
static EQmap::iterator it_uint;
static unsigned nSAT = 0;
struct CirFraigSort
{
   bool operator() (set<unsigned>* a, set<unsigned>* b)
   { return _AIGOrder[_FECAnchor[a]/2]  < _AIGOrder[_FECAnchor[b]/2]; }
};
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
   AIGmap AIGhash;
   // don't need POhash because we don't merge POs
   
   
   buildAIG();
   //unordered_set<CirGate*> todelete;
   for (unsigned i = 0; i < _AIGList.size(); ++i) {
      
      unsigned mainID = _AIGList[i] -> getID();
      
      CirGate* g1 = toPtr(_AIGList[i] -> getFanin(0));
      bool g1_inv = _AIGList[i] -> getFanin(0) & INV; // if the gate is inverse
      size_t g1EQ = g1 -> getEQ() ^ (size_t)g1_inv ; // if the child gate equivalence is inverse
      
      CirGate* g2 = toPtr(_AIGList[i] -> getFanin(1));
      bool g2_inv = _AIGList[i] -> getFanin(1) & INV; // if the gate is inverse
      size_t g2EQ = g2 -> getEQ() ^ (size_t)g2_inv ; // if the child gate equivalence is inverse
      
      if (g1EQ > g2EQ) { // swap
          k = {g2EQ,g1EQ};
      } else {
          k = {g1EQ,g2EQ};
      }
      
      
      it_pair = AIGhash.find(k);
      if (it_pair != AIGhash.end()) {
         cout << "Strashing: " << it_pair->second/2<<" merging "<<mainID <<"..." <<endl;
         _AIGList[i] -> changeEQ((unsigned)it_pair->second);
         _todelete.insert(_AIGList[i]);
         //* also delete _FECgrps if exists 
         //* (can only be executed from cirfraig because cirsim -> cirstr is forbidden)
//         if (!_FECgrps.empty()) {
//            assert(_AIGList[i] -> _FECgrp != 0);
//            bool inv = _AIGList[i] -> _FECgrp &INV;
//            set<unsigned>* target_grp = (set<unsigned>*)toPtr(_AIGList[i] -> _FECgrp);
//            target_grp -> erase(_AIGList[i] -> getID() * 2 + (unsigned)inv);
//         }
          
         //* _AIGList[i] is history. delete this fanout of its fanins
         unsigned size1 = g1 -> getFanoutSize();//DEBUG
         g1 -> rmFanout((size_t)_AIGList[i] ^ (size_t)g1_inv); //* 你否定我，我就否定你 :)
         unsigned size2 = g1 -> getFanoutSize();//DEBUG
         assert(size1 > size2);//DEBUG
         
         //size1 = g2 -> getFanoutSize();//DEBUG
         g2 -> rmFanout((size_t)_AIGList[i] ^ (size_t)g2_inv); //* 你否定我，我就否定你 :)
         //size2 = g1 -> getFanoutSize();//DEBUG
         //assert(size1 > size2);//DEBUG
      }
      else {
         AIGhash.insert({k, _AIGList[i] -> getEQ()});
         //* Compare fanin's equvilant gate ptr with its own ptr(this)
         //* if not the same, it means that the gate has changed representation (and will be deleted)
         //* reconnect fanin to the equivalent gate
         if (g1 -> getID() != g1EQ /2) {
            _AIGList[i] -> changeFinFout(0,(size_t)_gateList[_idList[g1EQ/2]] | (size_t)(g1EQ%2));
         }
         if (g2 -> getID() != g2EQ /2) {
            _AIGList[i] -> changeFinFout(1,(size_t)_gateList[_idList[g2EQ/2]] | (size_t)(g2EQ%2));
         }
      }
   }
   //* and also reconnect POs, but dont strash
   for (size_t i = _PINum+1; i < _PINum + _PONum+1; ++i) {
      PO* g = dynamic_cast<PO*>(_gateList[i]);
      CirGate* g1 = toPtr(g -> getFanin());
      bool g1_inv = g -> getFanin() & INV; // if the gate is inverse
      size_t g1EQ = g1 -> getEQ() ^ (size_t)g1_inv ; // if the child gate equivalence is inverse
      if (g1 -> getID() != g1EQ / 2) {
         g -> changeFinFout((size_t)_gateList[_idList[g1EQ/2]] | (size_t)(g1EQ%2));
      }
      //CirGate* gtemp = toPtr(g1 -> getEQ()); //DEBUG
      //cout <<"DEBUG"<< g ->getID()<<" child changed ID to"<< gtemp -> getID()<< endl;
   }
   //* marked as duplicate
   
   //* finally delete gates
   for (it_gate = _todelete.begin(); it_gate != _todelete.end(); ++it_gate) {
      _gateList[_idList[(*it_gate) -> getID()]] = 0;
      _AIGNum--;
      delete *it_gate;
      //cout << "DEBUG deleted " << (*it_gate) -> getID() <<" from strash"<< endl;
   }
   _todelete.clear();
   //* reconstruct DFS list
   _AIGList.clear();
   
}

void
CirMgr::fraig()
{
   //strash();
   if (_Fraiged && *(_FECgrps[0] -> begin()) == 0) { 
      delete _FECgrps[0];
      if (_FECgrps.size() == 1 ) _FECgrps.clear();
      else {_FECgrps[0] = _FECgrps.back();_FECgrps.pop_back();}
      //cerr << "DEBUG: second time omit 0s" << endl;
   }
   if(_FECgrps.empty()) return;
   buildAIG();
   
   SatSolver _s;
   _s.initialize();
   
   
   Aag2Sat(_s);
   bool regrp = false;
   while (!_FECgrps.empty()) {
      //auto it_null = _FECgrps.begin();
      //++it_null;
      //std::sort(it_null,_FECgrps.end(),CirFraigSort());
      
      // find anchor for each _FECgrps
      for(size_t i = 0; i < _FECgrps.size(); ++i) {
         auto it_uint = _FECAnchor.find(_FECgrps[i]);
         //if (it_uint == _FECAnchor.end()) { // no anchor determined for this grp
            auto it = _FECgrps[i] -> begin();
            unsigned thisAnchor = *(it);
            for (++it; it != _FECgrps[i] -> end(); ++it) {
               assert(_AIGOrder.find(*it / 2) != _AIGOrder.end());
               //cout << "anchor was " << thisAnchor << endl;
               //cout << "cmp" << _AIGOrder[*it / 2] << _AIGOrder[thisAnchor/2] << endl;
               if (_AIGOrder[*it / 2] < _AIGOrder[thisAnchor/2]) thisAnchor = *it;
               //cout << "anchor is " << thisAnchor << endl;
               //cout <<"DEBUG"<< *it / 2 <<":" <<_AIGOrder[*it / 2] << endl;
            }
            _FECAnchor[_FECgrps[i]] = thisAnchor;
            
         //}
         //cout << "FECgrp "<< i << " has anchor " << _FECAnchor[_FECgrps[i]] << endl;
      }
      for(size_t i = 0; i < _FECgrps.size(); ++i){
         
         FEC2Sat(_s, i);
         //SatRun(_s, i);
         if (nSAT == 64) {
            cout << "\rUpdating by UNSAT ...";
            reFEC();
            cout << endl;
            //cout << "DEBUG" << endl;
            //printFECPairs();
            FraigSplitFEC();
            nSAT = 0;
            regrp = true;
            break; // recalculate FECgrp
         }
      }
      if (regrp) {regrp = false; continue;}
      cout << "\rUpdating by UNSAT ... ";
      reFEC();
      cout << endl;
      //cout << "DEBUG" << endl;
      //printFECPairs();
      if (nSAT) {
         FraigSplitFEC();
         //cout << "DEBUG" << endl;
         //printFECPairs();
         nSAT = 0;
      }
      //if (_FECgrps.size() < 1600)
      //   break;
   
   }
   FraigStrash();
   strash();
   _Fraiged = true;
   _AIGList.clear();
   //buildAIG();
   //cout << endl << "AIG DFS after fraig = " << _AIGList.size() << endl;
   //DEBUG
//   CirGate* g1 = toPtr(*(_FECgrps[0]->begin()));
//   CirGate* g2 = _gateList[1];
//   cout << "DEBUG proving : " << g1 -> getID() ;
//   cout << " and "<< g2 -> getID()<<  endl;
//   
//   cout << "isSat : " << isSat<< endl;
//   //getSatAssignment(_s, patterns);
//   int a = _s.getValue(_gateVars[g1]);
//   int b = _s.getValue(_gateVars[g2]);
//   cout << a << " " << b << endl;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void 
CirMgr::Aag2Sat(SatSolver& s){
   //* Generates sat compatible representations of aag files
   //* Each Var can be found by the CirGate* of the gate
   _gateVars.clear();
   //* CONST0
   
   Var _const0 = s.newVar();
   //Var _genconst0 = s.newVar();
   _gateVars.insert({_gateList[0], _const0});
   s.assertProperty(_const0, false);
   _AIGOrder[0] = 0; // patch CONST0, always anchor
   _goodsim[_gateList[0]] = 0;
   //s.addAigCNF(_const0, 
         //_genconst0, true, 
         //_genconst0, false);
   //* PI
   for (unsigned i = 1; i < _PINum+1; i++)
      _gateVars[_gateList[i]] = s.newVar();
   //* AIG
   for(unsigned i = 0; i < _AIGList.size(); ++i){
      //* build AIG order for merge criteria
      _AIGOrder[_AIGList[i]->getID()] = i+1;
      //cout << _AIGList[i]->getID() <<" " << i+1<<endl;
      
      //* initialize goodsim
      _goodsim[_AIGList[i]] = _AIGList[i] -> _simVal;
      _gateVars[_AIGList[i]] = s.newVar();
      //cout << "DEBUG: AIG"<<_AIGList[i] -> getID() << endl;
      s.addAigCNF(_gateVars[_AIGList[i]], 
         _gateVars[toPtr(_AIGList[i]->getFanin(0))], (bool)(_AIGList[i]->getFanin(0) & INV), 
         _gateVars[toPtr(_AIGList[i]->getFanin(1))], (bool)(_AIGList[i]->getFanin(1) & INV));
   }
   //* PO -- none
   
}

void 
CirMgr::FEC2Sat(SatSolver& s, unsigned i){
   //for (set<unsigned>::iterator it_gatelit = _FECgrps[i]->begin(); it_gatelit != _FECgrps[i]->end(); ) {
      unsigned anchorlit = _FECAnchor[_FECgrps[i]];
      unsigned anchorid = anchorlit / 2;
      bool anchor_inv = anchorlit % 2;
      CirGate* g_a = _gateList[_idList[anchorid]];
      for (set<unsigned>::iterator it2_gatelit = _FECgrps[i]->begin(); it2_gatelit != _FECgrps[i]->end();) { 
         unsigned mergelit = *it2_gatelit;
         if (mergelit == anchorlit) {++it2_gatelit;continue;}
         unsigned mergeid = mergelit/2;
         bool merge_inv = mergelit % 2;
         
         CirGate* g_m = _gateList[_idList[mergeid]];
         assert(_AIGOrder[anchorid] < _AIGOrder[mergeid]);
         
         
         cout << "\rProving ";
         if (anchorid == 0) cout << (merge_inv % 2 == 0) << " = " << mergeid;
         else { // non zero prove
            cout << "(";
            if (anchor_inv) cout << "!";
            cout << anchorid <<" , "; 
            if (merge_inv) cout << "!";
            cout << mergeid << ")";
            //if (reverse) cout << "r";
         }
         
         Var n = s.newVar();
         if (anchorid != 0) {
            s.addXorCNF( n, 
               _gateVars[g_m], merge_inv, 
               _gateVars[g_a], anchor_inv);
            s.assumeRelease();
            s.assumeProperty(n, true);
         } else {
            n = _gateVars[g_m];
            s.assumeRelease();
            s.assumeProperty(n, (bool)(merge_inv % 2 == 0));
         }

         bool isSat = s.assumpSolve();
         if (isSat) {
            
            cout << "...SAT" << flush;
            //resim();
            int sim_a = s.getValue(_gateVars[g_a]);
            int sim_m = s.getValue(_gateVars[g_m]);
            //cout << g_a  << " " << g_m  << endl;
            //cout << g_a -> getTypeStr() << " " << g_m -> getID() << endl;
            //cout << sim_a << " " << sim_m << endl;
            assert((sim_a == sim_m)==(merge_inv!=anchor_inv));
            for (auto it_var = _gateVars.begin(); it_var != _gateVars.end(); ++it_var) {
               if (it_var -> first -> getType() != AIG_GATE) continue;
               int sim_on_gate = s.getValue(it_var -> second);
               assert(sim_on_gate!= -1);
               CirGate* g = it_var -> first;
               //if (g -> getID() == 2710) cout <<endl<< "2710 "<< sim_on_gate << endl; 
               //if (g -> getID() == 3393) cout <<endl<< "3393 "<< sim_on_gate << endl;
               //if (g -> getID() == 3391) cout <<endl<< "3391 "<< sim_on_gate << endl;
               auto it_sim = _goodsim.find(g);
               //cout << g -> getID() << endl;
               //cout << g -> getTypeStr() << endl;
               assert(it_sim != _goodsim.end());
               it_sim -> second ^= (-(size_t)sim_on_gate ^ it_sim -> second) & (1UL << nSAT);
               //|= ((size_t)sim_on_gate << nSAT);
            }
            nSAT ++;
            break;
            //it2_gatelit++;
            //it_gatelit++;
         } else {
            //if (anchorid == 0) s.assertProperty(n,merge_inv % 2);
            //else s.assertProperty(n,false);
            cout << "...UNSAT" << flush;
//            if (!reverse) {
            _FECgrps[i] -> erase(it2_gatelit++);
            g_m -> changeEQ(anchorlit ^ (unsigned)(merge_inv));//((size_t)g_a | (size_t)(merge_inv != anchor_inv));
            _todelete.insert(g_m);
//            } else {
//               _FECgrps[i] -> erase(it_gatelit++);
//               it2_gatelit++;
//               g_a -> changeEQ((size_t)g_m | (size_t)(merge_inv != anchor_inv));
//               _todelete.insert(g_a);
//            }
         }
      }
      //if (*it_gatelit / 2 == 0) break;
      //break;
      //++it_gatelit;
      
   //}
   
}

void
CirMgr::SatRun(SatSolver& s, unsigned i) {
   bool isSat = s.assumpSolve();  //* 0 -> same gate, 1 -> different gate
   cerr << "DEBUG: prove result " << isSat << endl;
   if (isSat) {
      
      //resim();
      for (auto it_gateidinv = _FECgrps[i]->begin();it_gateidinv != _FECgrps[i]->end();++it_gateidinv) { 
         CirGate* g = toPtr(*(it_gateidinv));
         int v = s.getValue(_gateVars[g]);
         cout << "DEBUG UNSAT grp" << g -> getID() << " has value " << v << endl;
      }
   } else {
      _AIGNum --;
      CirGate* anchor = toPtr(*(_FECgrps[i]->begin()));
      for (auto it_gateidinv = ++_FECgrps[i]->begin();it_gateidinv != _FECgrps[i]->end();++it_gateidinv) { 
         CirGate* g = toPtr(*(it_gateidinv));
         cout << "Fraig: " << anchor -> getID() <<" merging "<< g -> getID() <<"..." <<endl;
         _AIGNum --;
      }
      
   }
}

void 
CirMgr::FraigSplitFEC() {
   //cout << "DEBUG splitFEC called" <<endl;
   unsigned grps = _FECgrps.size();
   unsigned grpidx = 0;
   
   for (auto it_ptr = _FECgrps.begin(); grpidx != grps; ++it_ptr, ++ grpidx) {
      
      unordered_map <size_t, set<unsigned>*> temp_simval_map_set;
      temp_simval_map_set.clear();
      assert(_FECAnchor.find(*it_ptr)!=_FECAnchor.end());
      unsigned grpAnchor = _FECAnchor[*it_ptr];
      size_t anchor_simval_key = _goodsim[_gateList[_idList[grpAnchor/2]]];
      temp_simval_map_set.insert({anchor_simval_key,*it_ptr});
      //cout << "DEBUG constructing base of this FECgrp" << *it_ptr << endl;
      //cout << " with " << fixed_size << " elements " << endl;
      //continue;
      //unsigned fixed_size = (*it_ptr) -> size();
      for (auto it_gateid = (*it_ptr) -> begin(); it_gateid != (*it_ptr) -> end(); ) {
         if ((*it_gateid) == grpAnchor) {++it_gateid; continue;} 
         CirGate* thisgate = _gateList[_idList[(*it_gateid)/2]];
         assert(_goodsim.find(thisgate) != _goodsim.end());
         size_t simval_key = _goodsim[thisgate];
         
         //DEBUG
         //if (thisgate -> getType() == CONST_GATE) {assert(simval_key==0);++it_gateid;continue;}
         //AIG* AIGg = dynamic_cast<AIG*>(thisgate);
         //CirGate* g1 = toPtr(AIGg -> getFanin(0));
         //CirGate* g2 = toPtr(AIGg -> getFanin(1));
         //bool g1_inv = AIGg -> getFanin(0) & INV;
         //bool g2_inv = AIGg -> getFanin(1) & INV;
         
         //size_t g1_sim = g1_inv ? ~_goodsim[g1] : _goodsim[g1];
         //size_t g2_sim = g2_inv ? ~_goodsim[g2] : _goodsim[g2];
         //if (g1 -> getType() == AIG_GATE && g2 -> getType() == AIG_GATE) {
            // DEBUG for AIG properties
            //cout << "----"<<endl;
            //cout << thisgate->getID()<< " "<<_goodsim[thisgate] << endl;
            //cout << g1_inv<< " " << g1->getID()<<" "<<g1_sim << endl;
            //cout << g1_inv<<" "<<g2->getID()<<" "<<g2_sim << endl;
            //assert(_goodsim[thisgate] == (g1_sim & g2_sim));
         //}
         bool inv = (*it_gateid) % 2;
         if (inv) simval_key = ~simval_key;
        
         auto it_size_ptr = temp_simval_map_set.find(simval_key);
         if (it_size_ptr == temp_simval_map_set.end()) { // new sim pattern found
            
            //* new FECgrp
            set<unsigned>* v = new set<unsigned>;
            _FECgrps.push_back(v);
            //cout << "DEBUG new FEC group found" << _FECgrps.size() << endl;
            
            //* update hash map
            temp_simval_map_set.insert({simval_key,v});
            //cout << "DEBUG newgrp" << endl;
            
            //* FEC splitted
            //FECsplitted = true;
         } else if (it_size_ptr -> second == *it_ptr) {
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
   cout << "\rUpdating by SAT ... ";
   reFEC();
   cout << endl;
   
}
void 
CirMgr::FraigStrash (){
   for(size_t i = 0; i < _AIGList.size(); ++i){
      CirGate* g1 = toPtr(_AIGList[i] -> getFanin(0));
      CirGate* g2 = toPtr(_AIGList[i] -> getFanin(1));
      bool g1_inv = _AIGList[i] -> getFanin(0) & INV;
      bool g2_inv = _AIGList[i] -> getFanin(1) & INV;
      unsigned g1EQ = g1 -> getEQ() ^ (size_t)g1_inv;
      unsigned g2EQ = g2 -> getEQ() ^ (size_t)g2_inv;
      auto it = _todelete.find(_AIGList[i]);
      if (it != _todelete.end()) {
         // disconnect fanin
         g1 -> rmFanout((size_t)_AIGList[i] ^ (size_t)g1_inv); 
         g2 -> rmFanout((size_t)_AIGList[i] ^ (size_t)g2_inv);
         CirGate* geq = _gateList[_idList[_AIGList[i] -> getEQ()/2]];
         bool geq_inv = _AIGList[i] -> getEQ() & INV;
         cout << "Fraig: " << geq -> getID() << " merging ";
         if(geq_inv) cout << "!";
         cout << _AIGList[i] -> getID() << "..." <<endl; 
         
      } else {
         if (g1 -> getID() != g1EQ /2) 
            _AIGList[i] -> changeFinFout(0,(size_t)_gateList[_idList[g1EQ/2]] | (size_t)(g1EQ%2));
         if (g2 -> getID() != g2EQ /2) 
            _AIGList[i] -> changeFinFout(1,(size_t)_gateList[_idList[g2EQ/2]] | (size_t)(g2EQ%2));
         //auto it_debug = _AIGList[i].find(_AIGList[i]);
      }
         
   }
   //* and also reconnect POs, but dont strash
   for (size_t i = _PINum+1; i < _PINum + _PONum+1; ++i) {
      PO* g = dynamic_cast<PO*>(_gateList[i]);
      CirGate* g1 = toPtr(g -> getFanin());
      bool g1_inv = g -> getFanin() & INV; // if the gate is inverse
      size_t g1EQ = g1 -> getEQ() ^ (size_t)g1_inv ; // if the child gate equivalence is inverse
      if (g1 -> getID() != g1EQ /2) {
         g -> changeFinFout((size_t)_gateList[_idList[g1EQ/2]] | (size_t)(g1EQ%2));
      }
      //CirGate* gtemp = toPtr(g1 -> getEQ()); //DEBUG
      //cout <<"DEBUG"<< g ->getID()<<" child changed ID to"<< gtemp -> getID()<< endl;
   }
   
   for (it_gate = _todelete.begin(); it_gate != _todelete.end(); ++it_gate) {
      _gateList[_idList[(*it_gate) -> getID()]] = 0;
      _AIGNum--;
      //cout << "DEBUG deleted " << (*it_gate) -> getID() <<" from fraig"<< endl;
      delete *it_gate;
   }
   _todelete.clear();
   _AIGList.clear();
}
/***************************************************/
/*   DEPRECATED CODE                               */
/***************************************************

unordered_multiset<size_t>* fouts = (*it_gate) -> getFanoutPtr();
for (it_size = (*fouts).begin(); it_size != (*fouts).end(); ) {
   CirGate* gn = toPtr(*it_size);
   //cout << "DEBUG inspecting fanin: " <<gn <<" id "<< gn -> getID()<<endl;
   //if(toPtr(*it_size).getFanin(0))//TODO
   it2_gate = todelete.find(gn);
   if (it2_gate != todelete.end()){
      //cout << "DEBUG deleting:"<<(*it2_gate) -> getID() << " of " << (*it_gate) -> getID() <<endl;
      (it_size) = (*it_gate) -> _fanoutList.erase(it_size);
   } else {
      ++it_size;
   }
}
*/
