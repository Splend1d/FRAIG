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

int main(int argc, char** argv)
{
   if(argc != 4){
      cerr << "Usage : ./ptnGen <OUT_FILE_NAME> <INPUT_SIZE> <#_of_PATTERN>" << endl;
      return -1;
   }

   ofstream outf;
   outf.open(argv[1], ios::out);

   size_t inputSize  = atoi(argv[2]);
   size_t numPattern = atoi(argv[3]);

   srand((unsigned)time(NULL));

   for(size_t i = 0; i < numPattern; ++i){
      for(size_t j = 0; j < inputSize; ++j)
         outf << (rand() % 2);
      outf << endl;
   }

   outf.close();

   return 0;

}
