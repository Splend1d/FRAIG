# 資料結構與程式設計, 2019 Fall  
## Functionally Reduced And-Inverter Graph (FRAIG)  

### Program Objectives  
1. Parse aag description files and convert them into circuits data structures.
2. Using hash to detect structurally equivalent signals in a circuit.
3. Performing Boolean logic simulations and designing your own data structure to identify functionally equivalent candidate pairs in the circuit.
4. Call Boolean Satisfiability (SAT) solver to prove the functional equivalence.
  
### How to execute
First, generate the executable

	make clean  
	make linux18
	make
then, execute the program by  

`   ./fraig` 

or  

`    ./fraig -F <doFile>`
### List of availible commands and description
See [Homework_6.pdf](https://github.com/Splend1d/FRAIG/Homework_6.pdf) and [FraigProject.pdf](https://github.com/Splend1d/FRAIG/FraigProject.pdf) for details

### Implementation of optimization commands
Example circuit aag and svg files can be found at /demo

### 1. CIRSWeep
Description: Remove the gates that cannot be reached from Primary Outputs.

| Before Sweep | After Sweep |
| :---:  | :---: |
| ![Before Sweep](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Sweep](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) |
### 2. CIROPTimize
Description: Perform trivial circuit optimizations including constant propagation and redundant gate removal. This command can be evoked anytime except when `CIRSIMulate` command is just called. In such case, an error message `Do "CIRFraig" first!!”` will be issued.


| Before Optimize | After Optimize |
| :---:  | :---: |
| ![Before Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) |
### 3. CIRSTRash
Description: Merge the structurally equivalent gates. After the merger, the fanouts of the merged gate will be re-connected to the gate that merges it. Unless the circuit is re-read, or optimization or “fraig” operation has been performed, it does not make sense to perform strash multiple times. If repeated “CIRSTRash” is issued, output an error message: `Error: strash operation has already
been performed!!`

| Before Strash | After Strash |
| :---:  | :---: |
| ![Before Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) |
### 4. CIRSIMulate
Description: Perform circuit simulation to distinguish the functionally different signals and thus collect the FEC pairs/groups. 
* If `-Random` option is specified, perform random simulation until satisfactory. A stopping criteria if devised for the random simulation so that the FEC identification can be as efficient as possible. 
* If the option `-File` is used, read the simulation patterns from the pattern file. If the number of patterns in the pattern file is NOT a multiple of the width of the parallel pattern (e.g. 64), the parallel pattern will be padded with “all-0’s” patterns.
* The `-Output` option designates the name of the log file. After the simulations, the number of input patterns that have been simulated will be reported.

### 5. CIRFRaig
Description: Based on the identified (I)FEC pairs/groups, perform fraig operations on the circuit. All the SAT-proven equivalent gates should be merged together. Sweeping or trivial optimization on the resulted floating gates will not be performed, if these operation are desired, call CIRSWeep and CIROPTimize commands.

