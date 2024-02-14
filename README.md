# Pipelining Hazards and Timing Diagram
This project takes in up to 4 lines of MIPS code, identifies any pipelining hazards, and prints the timing diagram with and without a data forwarding unit.
Written by Hannah Ashton in September 2023 using Code::Blocks for Syracuse's CSE 661 Advanced Computer Architecture course.

<img width="390" alt="image" src="https://github.com/hcharise/Pipeline-Stalls-and-Stages/assets/110205350/2b7d69d3-a193-47a3-8f31-9f4a895845cf">


# Overall Structure
The main function uses a loop to allow the user to repeatedly (if desired) enter sets of instructions and print their hazards and pipelining diagrams. Within each of these loops, the program asks how many instructions there will be, then passes the input into the appropriate functions to print the instructions without a solution (showing where the hazards exist), with stalling but no forwarding, and with stalling and forwarding. The program then asks if the user would like to repeat this for a new set of instructions.

The program converts the set of 1-4 MIPS instructions (any combination of lw, sw, add, and sub) into a 2-dimensional matrix in order to easily identify the operations and instructions.


# Program Functions and Descriptions

## Unavailable Queue and Variable
The **enqueue function** is used to keep track of the registers from the most recent instructions that are currently unavailable. The "unavailable queue" is an array of 2 strings that moves every string over/out for each instruction in a first-in, first-out queue structure. This means that each register added to the queue will stay unavailable for two complete stages before being released to become available, since any use of this register in the next two instructions will result in a hazard.

Once a forwarding unit is in use, a register is only unavailable for the next _single_ instruction, since anything beyond that point can have the data forwarded to it. For this reason, a separate unavailable variable is used in the solutions with forwarding; this variable is then replaced or cleared after the next instruction, making the previously unavailable register unavailable. Note that only the "lw" instruction results in an unavailable register with forwarding since the result is not available until the Memory stage, whereas add and sub results are available after the Execute stage, and sw has no result in the registers.

## Printing the Hazards with No Solution
In order to print the data hazards that exist without any solution/forwarding, the **check_queue** function searches the unavailable queue for the registers required in the current instruction's Decode stage. If the necessary registers are in the unavailable queue, this triggers the hazard message to be printed.


## Printing the Solution with Stalling, but No Forwarding
The solution with stalling still utilizes the enqueue function to update the unavailable queue, since without forwarding, the results still require 2 stages to be available for use. The **print_stages_nf** and **print_stages2_nf** functions print the timing diagram (F, D, X, M, W, and S as needed), using the check_queue function to determine when a stall is needed (ie, a register is currently in the unavailable queue). The lw instructions use the print_stages_nf function, which takes in only one parameter since only one register must be available for use in order to execute a load. The sw, add, and sub instructions use the print_stages2_nf function that takes in two parameters since two registers must be available for use in order to execute these operations.

The current_stage variable is used simply to determine how many spaces to put in before each instruction's Fetch stage since these must be staggered.

## Printing the Solution with Stalling and Forwarding
As discussed above, the unavailable queue is not necessary once forwarding is enabled, since registers are only unavailable for one stage. Instead, with forwarding, we use the unavailable variable to recognize any registers awaiting the result of a lw. The **print_stages_wf** and **print_stages2_wf** functions, similar to without forwarding, print the timing diagram (F, D, X, M, W, and S as needed). To determine when a stall is needed, these functions simply compare the registers being used with the register in the unavailable variable. There are two functions again to represent the difference between lw instructions, which must only check one register as a parameter, and the sw, add, and sub instructions that require two registers to be checked and passed as parameters.
