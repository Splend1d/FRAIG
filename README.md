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
See [Homework_6.pdf](https://github.com/Splend1d/FRAIG/blob/master/Homework_6.pdf) and [FraigProject.pdf](https://github.com/Splend1d/FRAIG/blob/master/FraigProject.pdf) for details

### Implementation of optimization commands
Example circuit aag and svg files can be found at /demo  
The color of an gate in the visualized circuit corresponds to the different types of gates. Yellow denotes CONST0 gate, red denotes PI gates(primary input), gray trapezoid denotes AND gates, aqua denotes PO gates(primary output), and green denotes UNDEF gates(undefined). The circle attach to a gate indicates that the signal is reversed on this connection.

### CIRSWeep
#### Description : 
Remove the gates that cannot be reached from Primary Outputs.
#### Example : 

| Before Sweep | After Sweep |
| :---:  | :---: |
| ![Before Sweep](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Sweep](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-after.svg) |

#### Example Commander Output : 
	
	Sweeping(5)
	Sweeping(6)

#### Algorithm :
1. Build AIG gate DFS **Vector** `_AIGList` (Depth First Search from each PO gate by order, only store AND Gates)  
4 -> (end)
2. Convert the **Vector** `_AIGList` into an **Unordered_Set** `dfsset`
3. For each registered AND Gate, if not in **Unordered_Set** `dfsset`, add to **Unordered_Set** `rmset` to remove it later
4. For each gate in **Unordered_Set** `rmset`, if its fanin gate is not in **Unordered_Set** `rmset`, remove the connecting fanout of the fanin. *(If both gates were to be removed, their interconnection would be removed anyway, so there is no need to deal with fanin and fanouts)*
5. If a fanout of an UNDEF gate is removed, and it has no more fanouts, delete the UNDEF gate.
### CIROPTimize
#### Description: 
Perform trivial circuit optimizations including constant propagation and redundant gate removal. This command can be evoked anytime except when `CIRSIMulate` command is just called. In such case, an error message `Do "CIRFraig" first!!”` will be issued.
#### Example :

| Before Optimize | After Optimize |
| :---:  | :---: |
| ![Before Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/opt-before.svg) | ![After Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/opt-after.svg) |

#### Example Commander Output : 
	
	Opt
	Opt
#### Algorithm :

### CIRSTRash
#### Description :
Merge the structurally equivalent gates. After the merger, the fanouts of the merged gate will be re-connected to the gate that merges it. Unless the circuit is re-read, or optimization or “fraig” operation has been performed, it does not make sense to perform strash multiple times. If repeated “CIRSTRash” is issued, output an error message: `Error: strash operation has already
been performed!!`
#### Example :

| Before Strash | After Strash |
| :---:  | :---: |
| ![Before Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/str-before.svg) | ![After Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/str-after.svg) |

#### Example Commander Output : 
	
	Str
	Str
#### Algorithm :

### CIRSIMulate
#### Description: 
Perform circuit simulation to distinguish the functionally different signals and thus collect the FEC pairs/groups. 
* If `-Random` option is specified, perform random simulation until satisfactory. A stopping criteria if devised for the random simulation so that the FEC identification can be as efficient as possible. 
* If the option `-File` is used, read the simulation patterns from the pattern file. If the number of patterns in the pattern file is NOT a multiple of the width of the parallel pattern (e.g. 64), the parallel pattern will be padded with “all-0’s” patterns.
* The `-Output` option designates the name of the log file. After the simulations, the number of input patterns that have been simulated will be reported.
#### Example :
#### Example Commander Output : 
	
	Sim
	Sim
	
### CIRFRaig
#### Description: 
Based on the identified (I)FEC pairs/groups, perform fraig operations on the circuit. All the SAT-proven equivalent gates should be merged together. Sweeping or trivial optimization on the resulted floating gates will not be performed, if these operation are desired, call CIRSWeep and CIROPTimize commands.
#### Example :

| Before Fraig | After Fraig |
| :---:  | :---: |
| ![Before Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/fr-before.svg) | ![After Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/fr-after.svg) |
#### Example Commander Output : 
	
	Fraig
	Fraig
	
What is worth noting is that the above example cannot be reduced by any of the other commands, whereas CIROPTimize and CIRSTRash are essentially a subset of CIRFRaig, so CIRFRAIG can also do these two types of optimization (however, it is far less efficient).
#### Example :

| Before Fraig | After Fraig |
| :---:  | :---: |
| ![Before Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/opt-before.svg) | ![After Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/opt-after.svg) |
| ![Before Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/str-before.svg) | ![After Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/str-after.svg) |
#### Example Commander Output : 
	
	Fraig
	Fraig
	
All the circuit visuals are generated from [AAG Visualizer](https://byronhsu.github.io/AAG-Visualizer/) developed by Bryon, Hsu and Adrian, Hsu. Be sure to check it out!
