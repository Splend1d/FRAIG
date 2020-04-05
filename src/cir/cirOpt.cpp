/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/


#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
static unordered_map<CirGate*,unsigned>::iterator it_map, it2_map;
static unordered_set<CirGate*>::iterator it_gate, it2_gate;
static unordered_multiset<size_t>::iterator it_size;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   //* DEBUG for before sweep
//   for (unsigned i = 0; i < _gateList.size() ; ++i) {
//      cout << "before sweep:" <<i<<" "<<_gateList[i];
//      if (_gateList[i] != 0)
//         cout << _gateList[i] -> getTypeStr();
//      cout << endl;
//   }
   buildAIG();
   unordered_set<CirGate*> dfsset;
   unordered_set<CirGate*> rmset;
   for (unsigned i = 0; i < _AIGList.size(); ++i) {
      dfsset.insert(_AIGList[i]);
      assert(_gateList[_idList[_AIGList[i] ->getID()]] != 0);
   }
   //cout << _gateList[_MaxDefLine] << endl;
   //cout << _gateList[_MaxDefLine+1] << endl; // 0
   //cout << _gateList[_MaxDefLine+2] << endl;
   for (unsigned i = _PINum + _PONum + 1; i <= _MaxDefLine+1; ++i){
      if (_gateList[i] == 0) continue;
      auto it_gate = dfsset.find(_gateList[i]);
      if (it_gate == dfsset.end()){
         assert(_gateList[i] -> getType() ==  AIG_GATE);
         rmset.insert(_gateList[i]);
      }
   }
   unsigned AIGs = 0;
   unsigned noAIGs = 0;
   for (unsigned i = _PINum + _PONum + 1; i <= _MaxDefLine+1; ++i){
      if (_gateList[i] == 0) {noAIGs++;continue;}
      else AIGs ++;
      
   }
   //cout << "original : "<< _AIGNum << " after sweep assert = " << _AIGList.size() << endl;
   //cout << " now # of dfs gates : " << AIGs << endl << " now # of deleted gates " << noAIGs << endl;
   //cerr << "DEBUG sweeped " << rmset.size() << " gates" << endl; 
   //* delete gate fanouts which have been deleted
   for (it_gate = rmset.begin(); it_gate!=rmset.end(); ++it_gate) {
      AIG* g = dynamic_cast<AIG*>(*it_gate);
      CirGate* g1 = toPtr(g -> getFanin(0));
      bool g1_inv = (g -> getFanin(0)&INV);
      CirGate* g2 = toPtr(g -> getFanin(1));
      bool g2_inv = (g -> getFanin(1)&INV);
      //cout << "DEBUG sweeping " << g -> getID() << endl;
      //cout << "DEBUG fanin1 " << g1 -> getID() << " " ;
      //cout << g1 -> getTypeStr()<<endl;
      //cout << "DEBUG fanin2 " << g2 -> getID() << " " ;
      //cout << g2 -> getTypeStr()<<endl;
      if (g1 != 0) {
         if (rmset.find(g1) == rmset.end()) {
            g1 -> rmFanout((size_t)*it_gate | (size_t)g1_inv);
//            unordered_multiset<size_t>* fouts = g1 -> getFanoutPtr();
//            if (!fouts -> empty()) {
//               
//               g1 -> _fanoutList.find((size_t)*it_gate | (size_t)g1_inv);
//               //auto it = fouts -> find((size_t)g | (size_t)g1_inv);
//               //if (it != fouts -> end()) fouts -> erase(it);
//            }
            if (g1 -> getFanoutSize() == 0 && g1 -> getType() == UNDEF_GATE){
               _gateList[_idList[g1 -> getID()]] = 0; delete g1;
               cout << "Sweeping: UNDEF(" << g1 -> getID()<<") removed..."<<endl; 
            }
            
         }  
      }
      if (g2 != 0) {
         if (rmset.find(g2) == rmset.end()) {
            g2 -> rmFanout((size_t)*it_gate | (size_t)g2_inv);
//            unordered_multiset<size_t>* fouts = g2 -> getFanoutPtr();
//            if (!fouts -> empty()) {
//               
//               fouts -> find((size_t)*it_gate | (size_t)g2_inv);
//               //auto it = fouts -> find((size_t)g | (size_t)g2_inv);
//               //if (it != fouts -> end()) fouts -> erase(it);
//            }
            if (g2 -> getFanoutSize() == 0 && g2 -> getType() == UNDEF_GATE){
               _gateList[_idList[g2 -> getID()]] = 0; delete g2;
               cout << "Sweeping: UNDEF(" << g2 -> getID()<<") removed..."<<endl; 
            }
         }
      }
      
      _gateList[_idList[g -> getID()]] = 0;
      cout << "Sweeping: AIG(" << g -> getID()<<") removed..."<<endl;
      delete *it_gate;
      _AIGNum --;
      
      
   }
   //* DEBUG for after sweep
//   for (unsigned i = 0; i < _gateList.size() ; ++i) {
//      cout << "after sweep:" <<i<<" "<<_gateList[i];
//      if (_gateList[i] != (CirGate*)0) {
//         cout <<" "<< _gateList[i] -> getTypeStr() << _gateList[i] -> getID();
//      }
//      cout << endl;
//   }

}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   //* get the dfs (for AIG only)
   buildAIG();
   vector<AIG*> unmerged;
   unordered_set<CirGate*> todelete;
   unordered_set<CirGate*> reconnect;
   unsigned thisgeq;
   bool flag = false; // if no elements have been changed, don't check the aig's children
   unsigned CONST0 = 0;
   unsigned CONST1 = 1;
   for (size_t i = 0; i < _AIGList.size(); ++i) {
      //cout << "DEBUGGING AIG:"<<_AIGList[i] -> getID() << " = " <<_AIGList[i] -> getEQ() << endl;
      unsigned thisid = _AIGList[i] -> getID();
      //* fanin 1
      CirGate* g1 = toPtr(_AIGList[i] -> getFanin(0));
      bool g1_inv = _AIGList[i] -> getFanin(0) & INV; // if the gate is inverse
      unsigned g1eq = g1 -> getEQ() ^ (unsigned)g1_inv ; // if the child gate equivalence is inverse
      //cout << "CHILD1eq:"<< g1eq << " " << g1 -> getEQ()<< " " << g1_inv << endl;
      //* fanin 2
      CirGate* g2 = toPtr(_AIGList[i] -> getFanin(1));
      bool g2_inv = _AIGList[i] -> getFanin(1) & INV; // if the gate is inverse
      unsigned g2eq = g2 -> getEQ() ^ (unsigned)g2_inv ; // if the child gate equivalence is inverse
      //cout << "CHILD2eq:"<< g2eq << " " << g2 -> getEQ()<< " " << g2_inv << endl;
      
      //* case1 : exists constant 0 fanin
      if (g1eq == CONST0 || g2eq == CONST0) {
         thisgeq = CONST0;
         
         //DEPRECATED
         //reconnect.insert(toPtr(g1eq)); // these gate have changed fanouts
         //reconnect.insert(toPtr(g2eq));
      }
      //* case2-1 : exists constant 1 fanin
      else if (g1eq == CONST1) {
         thisgeq = g2eq;
         
      }
      //* case2-2 : exists constant 1 fanin
      else if (g2eq == CONST1) {
         thisgeq = g1eq;
      }
      //* case3 : both fanins are equivalent
      else if (g1eq == g2eq) { // same gate
         thisgeq = g1eq;
      }
      //* case4 : both fanins are equivalent but mutually inverse
      else if (g1eq == (g2eq ^ (unsigned)INV)) { // same gate inverse
         thisgeq = CONST0;
         //cout << "inv!" << endl; 
      }
      //* case default : cannot be simplified via opt
      else {
         //cout << "DEBUG"<<_AIGList[i] -> getID() << " is unchanged ";
         //cout << "DEBUG" << g1 <<" "<< toPtr(g1eq) << "DEBUG" << toPtr(g2eq) <<" "<< g2;
         //cout << "DEBUG: childs" << g1 -> getID() <<g1 -> getTypeStr()<<endl\
         //<< g2 -> getID() <<endl<< (toPtr(g1eq)) -> getID() <<endl<< (toPtr(g2eq)) -> getID() << endl;
         //* reconnect fanout connections of the fanin if the fanin gate won't be deleted
         //* check from comparing it's equvilant gate ptr with its own ptr(this)
         //* if not the same, it means that the gate has changed representation (and will be deleted)
         if (g1 ->getID() != g1eq / 2) {
            _AIGList[i] -> changeFinFout(0,(size_t)_gateList[_idList[g1eq/2]] | (size_t)(g1eq%2));
            //cout << "and has changed child" <<g1 ->getID()<< endl;
         }
         if (g2->getID() != g2eq / 2) {
            _AIGList[i] -> changeFinFout(1,(size_t)_gateList[_idList[g2eq/2]] | (size_t)(g2eq%2));
            //cout << "and has changed child" <<g2 ->getID()<< endl;
         }
         continue;
      }
      //* delete fanout connections of the fanin if the fanin gate won't be deleted
      //* check from comparing it's equvilant gate ptr with its own ptr(this)
      //* if not the same, it means that the gate has changed representation (and will be deleted)
     
      //unsigned size1 = g1 -> getFanoutSize();//DEBUG
      g1 -> rmFanout((size_t)_AIGList[i] | (size_t)g1_inv); //* 你否定我，我就否定你 :)
      //unsigned size2 = g1 -> getFanoutSize();//DEBUG
      //assert(size1 > size2);//DEBUG
      g2 -> rmFanout((size_t)_AIGList[i] | (size_t)g2_inv); //* 你否定我，我就否定你 :)
         
      _AIGList[i] -> changeEQ(thisgeq);
      todelete.insert(_AIGList[i]);
      //cout << "DEBUG todelete inserted" << _AIGList[i] -> getID() << endl;
      //reconnect.erase(_AIGList[i]); // if need to remove, dont need to reconnect
      //reconnect.insert(toPtr(thisgeq));
      cout << "Simplifying: "<< _gateList[_idList[thisgeq/2]] -> getID() <<" merging ";
      if (thisgeq & INV) cout << "!";
      cout<< thisid <<"..."<<endl;
      //cout << "DEBUG by fanins " << g1-> getID() << " " << g2-> getID() << endl;
      
   }
   
   
   //* and also check POs
   for (size_t i = _PINum+1; i < _PINum + _PONum+1; ++i) {
      PO* g = dynamic_cast<PO*>(_gateList[i]);
      CirGate* g1 = toPtr(g -> getFanin());
      bool g1_inv = g -> getFanin() & INV; // if the gate is inverse
      unsigned g1eq = g1 -> getEQ() ^ (size_t)g1_inv ; // if the child gate equivalence is inverse
      if (g1 ->getID() != g1eq / 2) {
         g -> changeFinFout((size_t)_gateList[_idList[g1eq/2]] | (size_t)(g1eq%2));
      }
      // CirGate* gtemp = toPtr(g1 -> getEQ()); //DEBUG
      // cout <<"DEBUG"<< g ->getID()<<" child changed ID to"<< gtemp -> getID()<< endl;
   }
   //* finally delete gates
   for (it_gate = todelete.begin(); it_gate != todelete.end(); ++it_gate) {
      _gateList[_idList[(*it_gate) -> getID()]] = 0;
      delete *it_gate;
      _AIGNum--;
   }
   //* reconstruct DFS list
   _AIGList.clear();
   //buildAIG(); // force replace;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

/***************************************************/
/*   DEPRECATED CODE                               */
/***************************************************


//* 
GateList FltIns; 
   getDefUnused(FltIns);
   unordered_map<CirGate*, unsigned> rmfanouts; 
   while (!FltIns.empty()) {
      CirGate* RMGate = FltIns.back();
      //cout << RMGate -> getID() << endl;
      //cout << RMGate -> getType() << endl;
      FltIns.pop_back();
      //* get child fanins of oldgate, remove fanout
      if (RMGate -> getType() == AIG_GATE) {
         AIG* RMGateT = dynamic_cast<AIG*>(RMGate);
         for (size_t i = 0; i < 2; i++) {
            CirGate* intemp = toPtr(RMGateT -> getFanin(i));
            rmfanouts[intemp]; //* if not in unordered_map, initialize, initial value is 0
            rmfanouts[intemp] += 1;
            if (rmfanouts[intemp] == intemp -> getFanoutSize()) {
               FltIns.push_back(intemp);
            }
         }
         _gateList[_idList[RMGate->getID()]] = 0;
         cout <<"Sweeping: AIG("<<RMGate ->getID()<<") removed..."<<endl;
         _AIGNum --;
         rmfanouts[RMGate] = 0;
         delete RMGate;
      } else if (RMGate -> getType() == PI_GATE || RMGate -> getType() == CONST_GATE) {
         //* dont remove PIs or CONSTs
         continue;
      } else if (RMGate -> getType() == UNDEF_GATE) {
         //* JUST DELETE IT !!
         _gateList[_idList[RMGate->getID()]] = 0;
         cout <<"Sweeping: UNDEF("<<RMGate ->getID()<<") removed..."<<endl;
         rmfanouts[RMGate] = 0;
         delete RMGate;
      }
      
   }


remove fanout connections that have been removed
   
   for (it_gate = reconnect.begin(); it_gate != reconnect.end(); ++it_gate) {
      //cout << "DEBUG checking fanouts of "<<(*it_gate) ->getID()<< " loaction("<< *it_gate <<")"<<endl;
      unordered_multiset<size_t>* fouts = (*it_gate) -> getFanoutPtr();
      for (it_size = (*fouts).begin(); it_size != (*fouts).end(); ) {
         CirGate* gn = toPtr(*it_size);
         //cout << "DEBUG inspecting fanin: " <<gn <<" id "<< gn -> getID()<<endl;
         
         it2_gate = todelete.find(gn);
         if (it2_gate != todelete.end()){
            //cout << "DEBUG deleting:"<<(*it2_gate) -> getID() << " of " << (*it_gate) -> getID() <<endl;
            (it_size) = (*it_gate) -> _fanoutList.erase(it_size);
         } else {
            ++it_size;
         }
      }
      //cout << "DEBUG after deletion, gate"<< (*it_gate) -> getTypeStr()\
           << (*it_gate) -> getID() <<  " has fanout size "<< (*it_gate) -> getFanoutSize()<<endl;
      if ((*it_gate) -> noFanout() && (*it_gate) -> getType() == UNDEF_GATE) {
         //* an UNDEF_GATE with no fanout will be removed
         assert(_gateList[ _idList[(*it_gate) -> getID()]]==(*it_gate));
         _gateList[ _idList[(*it_gate) -> getID()]] = 0;
         //cout <<"DEBUG gate "<< (*it_gate) -> getID() << " deleted " << endl;
         delete *it_gate;
         
      } 
   }
   
   //* since merged gates are deleted, we only need to change fanins of unmerged gates 
   for (size_t i = 0; i < unmerged.size(); ++i) {
      //cout << "DEBUG AIG change fanin :" << unmerged[i] -> getEQ() << endl;
      //* fanin 1
      CirGate* g1 = toPtr(unmerged[i] -> getFanin(0));
      bool g1_inv = unmerged[i] -> getFanin(0) & INV; // if the gate is inverse
      size_t g1eq = g1 -> getEQ() ^ (size_t)g1_inv ; // if the child gate equivalence is inverse
      //cout << "DEBUG CHILD1eq:"<< g1eq << " " << g1 -> getEQ()<< " " << g1_inv << endl;
      //* fanin 2
      CirGate* g2 = toPtr(unmerged[i] -> getFanin(1));
      bool g2_inv = unmerged[i] -> getFanin(1) & INV; // if the gate is inverse
      size_t g2eq = g2 -> getEQ() ^ (size_t)g2_inv ; // if the child gate equivalence is inverse
      //cout << "DEBUG CHILD2eq:"<< g2eq << " " << g2 -> getEQ()<< " " << g2_inv << endl;
      
      if (g1eq/2 != g1 -> getID()) { // fanin 1 changed
         unmerged[i] -> changeFinFout(0,g1eq);
         CirGate* gtemp = toPtr(unmerged[i] -> getFanin(0));
         //cout <<"DEBUG"<< unmerged[i] ->getID()<<" child 0 changed ID to"<< gtemp -> getID()<< endl;
      } 
      if (g2eq/2 != g2 -> getID()) { // fanin 2 changed
         unmerged[i] -> changeFinFout(1,g2eq);
         CirGate* gtemp = toPtr(unmerged[i] -> getFanin(1));
         //cout <<"DEBUG"<< unmerged[i] ->getID()<<" child 0 changed ID to"<< gtemp -> getID()<< endl;
      } 
      
   }
   //* finally delete gates
   for (it_gate = todelete.begin(); it_gate != todelete.end(); ++it_gate) {
      _gateList[(*it_gate) -> getLineNo()] = 0;
      delete *it_gate;
   }
*/
