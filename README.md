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

| Before Optimize | After Optimize |
| :---:  | :---: |
| ![Before Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Optimize](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) |
### 3. CIRSTRash

| Before Strash | After Strash |
| :---:  | :---: |
| ![Before Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) | ![After Strash](https://github.com/Splend1d/FRAIG/blob/master/demo/sw-before.svg) |
### 4. CIRSIMulate
### 5. CIRFRaig
