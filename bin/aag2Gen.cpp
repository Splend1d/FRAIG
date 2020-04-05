#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <climits>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <cctype>
#include <cstring>

using namespace std;

#define LITERAL(T) (T * 2)

int main(int argc, char** argv)
{
   if(argc != 7){
      cerr << "Usage : ./aag2Gen OUT_FILE_NAME M I L O A" << endl;
      cerr << "e.g. ./aag2Gen dag01.aag 20 5 0 5 10 " << endl;
      return -1;
   }

   srand((unsigned)time(NULL));

   size_t MM = atoi(argv[2]);
   size_t II = atoi(argv[3]);
   size_t LL = atoi(argv[4]);
   size_t OO = atoi(argv[5]);
   size_t AA = atoi(argv[6]); 

   ofstream outf;
   outf.open(argv[1], ios::out);

   // aag line
   outf << "aag " << argv[2] << " " << argv[3] << " " << argv[4] << " "
        << argv[5] << " " << argv[6] << endl;
   
   // input
   for(size_t i = 1; i <= II; ++i)
      outf << (i * 2) << endl;

   // output
   vector<size_t> validGate;
   for(size_t i = II; i < MM; ++i)
      validGate.push_back(i+1);

   size_t validGateSize = validGate.size();

   for(size_t i = 0; i < MM * 2; ++i){
      size_t pos1 = rand() % validGateSize;
      size_t pos2 = rand() % validGateSize;
      size_t tmp = validGate[pos1];
      validGate[pos1] = validGate[pos2];
      validGate[pos2] = tmp;
   }

   validGate.resize(AA);

   vector<size_t> validInGate;
   validInGate = validGate;
   for(size_t i = 0; i <= II; ++i)
      validInGate.push_back(i);

   ::sort(validInGate.begin(), validInGate.end());
   ::sort(validGate.begin(), validGate.end());

   for(size_t i = 0, s = validGate.size() - 1; i < OO; ++i)
      outf << LITERAL(validGate[s - i]) << endl;
   
   // aig
   size_t validInGateSize = validInGate.size();

   for(size_t i = 0; i < AA; ++i){
      size_t aigId = validGate[i];
      size_t i1 = validInGate[(rand() % (II + 1 + i))];
      size_t i2 = validInGate[(rand() % (II + 1 + i))];
      size_t in1 = LITERAL(i1) + (rand() % 2);
      size_t in2 = LITERAL(i2) + (rand() % 2);

      outf << LITERAL(aigId) << " " << in1 << " " << in2 << endl;

   }

   // ioName
   for(size_t i = 0; i < II; ++i){
      outf << "i" << i << " ";
      for(size_t j = 0; j < 6; ++j)
         outf << char(65 + (rand() % 26));
      outf << endl;
   }

   for(size_t i = 0; i < OO; ++i){
      outf << "o" << i << " ";
      for(size_t j = 0; j < 6; ++j)
         outf << char(65 + (rand() % 26));
      outf << endl;
   }

   // comment
   outf << "c" << endl;
   outf << "AAG by Chen Hao Hsu" << endl;

   outf.close();

   return 0;

}
