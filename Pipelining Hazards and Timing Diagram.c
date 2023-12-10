/*
Hannah Ashton
CSE 661
September 14th, 2023

Final Exam
MIPS Pipelining Stages/Stalls
*/

/*
-- FEEDBACK!!! --
lw s0, 0(s2)
sw s0, 0(s3)
Results in a stall for the lw when forwarding, but this does not make any sense.....!!
*/

/*
SAMPLE TESTS

** USE ME **
lw $t0, 0($s2)
add $s2, $s2, $a4
add $t0, $t0, $a5
sw $t0, 0($s4)

lw $r0, 0($r0)
sub $r1, $r0, $r1
sw $t0, 10($r2)
add $t0, $t1, $t0

lw $t1, 0($t2)
lw $t2, 12($t1)
lw $t3, ($t1)
lw $t4, 3($t3)

lw R2, 0(A0)
add R1, R1, A0
sw R1, 0(R2)
add R2, R2, A2
*/


#include <stdio.h>
#include <string.h>

char unavailables[2][7] = {'NULL', 'NULL'}; // holds registers that are awaiting a write back (w/o forwarding)
char * unavailable = "NULL"; // holds the register that can't yet be forwarded (w/ forwarding)
int current_stage = 0; // used for staggering the pipelining stages for each instruction
int last_instruct = 0; // used with forwarding to track if the previous instruction stalled, meaning current one must too

// Adds register to the unavailable queue when it is not usable by other instructions, awaiting write back
int enqueue(char * reg) {
    // Move existing items in the queue over a spot, FIFO
    for (int i = 1; i > 0; i--) {
         strcpy(unavailables[i], unavailables[i - 1]);
    }
    // Add new register to queue
    strcpy(unavailables[0], reg);

    return 0;
}

// Returns 0 if reg is available (not in queue), returns 1 if reg is unavailable (in queue)
int check_queue(char * reg) {
    // Loop through queue to search for reg match
    for (int i = 0; i < 2; i++) {
        if (strcmp(unavailables[i], reg) == 0) {
            return 1;
        }
    }
    return 0;
}

// Prints the pipelining stages (with stalls as needed) without forwarding unit
// Takes a single parameter, reg1, to determine if it is available for use
int print_stages_nf(char * reg1) {
    char * null = "NULL";

    // Prints spaces for staggering instructions, then prints F
    for (int i = 0; i < current_stage; i++) {
        printf("  ");
    }
    printf("F ");

    // Increments staggering count for next instruction
    current_stage++;

    // Prints S repeatedly while needed register is unavailable
    while (check_queue(reg1) == 1) {
        printf("S ");
        enqueue(null);
        current_stage++;
    }

    // Prints rest of stages
    printf("D X M W");

    return 0;
}

// Prints the pipelining stages (with stalls as needed) without forwarding unit
// Takes two parameters, reg1 and reg2, to determine if they are both available for use
int print_stages2_nf(char * reg1, char * reg2) {
    char * null = "NULL";

    // Prints spaces for staggering instructions, then prints F
    for (int i = 0; i < current_stage; i++) {
        printf("  ");
    }
    printf("F ");

    // Increments staggering count for next instruction
    current_stage++;

    // Prints S repeatedly while either of needed registers are unavailable
    while (check_queue(reg1) == 1 || check_queue(reg2) == 1) {
        printf("S ");
        enqueue(null);
        current_stage++;
    }

    // Prints rest of stages
    printf("D X M W");

    return 0;
}

// Prints the pipelining stages (with stalls as needed) with forwarding unit
// Takes a single parameter, reg1, to determine if it is available for use
int print_stages_wf(char * reg1) {

    // Prints spaces for staggering instructions, then prints F
    if (last_instruct == 1) {
        current_stage--;
    }
    for (int i = 0; i < current_stage; i++) {
        printf("  ");
    }
    printf("F ");

    // Checks if previous instruction stalled, thus stalling this instruction
    if (last_instruct == 1) {
        printf("S D ");
        last_instruct = 0;
        current_stage++;
    } else {
        printf("D ");
    }

    // Increments staggering count for next instruction
    current_stage++;

    // Prints S if needed register is unavailable (only 1 stall needed with forwarding)
    if (strcmp(unavailable, reg1) == 0) {
        printf("S ");
        current_stage++;
        unavailable = "NULL";
        last_instruct = 1;
    }
    unavailable = "NULL";

    // Prints rest of stages
    printf("X M W");

    return 0;
}


// Prints the pipelining stages (with stalls as needed) with forwarding unit
// Takes two parameters, reg1 and reg2, to determine if they are both available for use
int print_stages2_wf(char * reg1, char * reg2) {

    // Prints spaces for staggering instructions, then prints F
    if (last_instruct == 1) {
        current_stage--;
    }
    for (int i = 0; i < current_stage; i++) {
        printf("  ");
    }
    printf("F ");

    // Checks if previous instruction stalled, thus stalling this instruction
    if (last_instruct == 1) {
        printf("S D ");
        last_instruct = 0;
        current_stage++;
    } else {
        printf("D ");
    }

    // Increments staggering count for next instruction
    current_stage++;

    // Prints S if needed registers are unavailable (only 1 stall needed with forwarding)
    if (strcmp(unavailable, reg1) == 0 || strcmp(unavailable, reg2) == 0) {
        printf("S ");
        current_stage++;
        unavailable = "NULL";
        last_instruct = 1;
    }

    unavailable = "NULL";


    // Prints rest of stages
    printf("X M W");

    return 0;
}

// Takes MIPS instructions from user, then shows data hazards and prints pipelining with and without forwarding.
int main() {
    char repeat = 'Y'; // Loop to repeat full program
    char instructions[4][4][7]; // Multi-dim array to hold instructions, each word/reg is its own element
    int imax; // Number of instructions to be taken in, used for looping
    int fflush(FILE *stream); // Clears input after getting instructions from user
    char offsets[4][4] = {NULL, NULL, NULL, NULL}; // Array to hold offsets for lw and sw instructions

    // Loops to repeat full program if running repeatedly
    while (repeat == 'Y') {

        // Print directions
        printf("How many instructions are there?\n");
        scanf("%d", &imax);

        printf("Enter the %d instructions, each on a new line.\n", imax);

        // Take in instructions
        for (int i = 0; i < imax; i++) {
            // Get instruction (add, sub, lw, sw)
            scanf("%s ", instructions[i][0]);

            // Depending on instruction, takes in correct number of registers
            // Lw/sw format
            if (strcmp(instructions[i][0], "lw") == 0 || strcmp(instructions[i][0], "sw") == 0) {
                scanf("%3[^,], %s", instructions[i][1], instructions[i][2]);
                if (instructions[i][2][0] != '(') {
                    char * offset = strtok(instructions[i][2], "(");
                    char * reg = strtok(NULL, ")");
                    strcpy(offsets[i], offset);
                    strcpy(instructions[i][2], reg);
                } else{
                    *instructions[i][2] = *instructions[i][2] + 1;
                    char * reg = strtok(instructions[i][2], ")");
                    strcpy(offsets[i], "");
                    strcpy(instructions[i][2], reg);
                }
            } else {
                // Add/sub format
                scanf("%3[^,], %3[^,], %s", instructions[i][1], instructions[i][2], instructions[i][3]);
            }
            fflush(stdin);
        }

        // PRINTING PIPELINING
        printf("\n--- PIPELINING ---\n");

        // Print Sequence, w/o solution
        // LW - second reg must be available to start, first register is unavailable until WB completes // lw $r0, 0($r1)
        // ADD/SUB - second and third must be available to start, first register is unavailable until WB completes // add $r0, $r1, $r2
        // SW - first reg must be available to start, all registers are available immediately // sw $r0, 0($r1)

        printf("No Solution:\n");
        for (int i = 0; i < imax; i++) {
            printf("   ");
            // Print lw format
            if (strcmp(instructions[i][0], "lw") == 0) {
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                // Check for data hazard with second reg
                if (check_queue(instructions[i][2]) == 1) {
                    printf(" << Data hazard with %s!", instructions[i][2]);
                }
                enqueue(instructions[i][1]);
            // Print sw format
            } else if(strcmp(instructions[i][0], "sw") == 0) {
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                // Check for data hazard with first reg
                if (check_queue(instructions[i][1]) == 1) {
                    printf(" << Data hazard with %s!", instructions[i][1]);
                }
                // Check for data hazard with second reg
                if (check_queue(instructions[i][2]) == 1) {
                    printf(" << Data hazard with %s!", instructions[i][2]);
                }
                // Nothing to enqueue, since sw does not update registers
            // Prints add and sub format
            } else {
                printf("%-3s %-3s, %-3s, %-3s \t", instructions[i][0], instructions[i][1], instructions[i][2], instructions[i][3]);
                // Check for data hazard with second reg
                if (check_queue(instructions[i][2]) == 1) {
                    printf(" << Data hazard with %s!", instructions[i][2]);
                }
                // Check for data hazard with third reg
                if (check_queue(instructions[i][3]) == 1) {
                    printf(" << Data hazard with %s!", instructions[i][3]);
                }
                // Adds first reg to unavailable, since register must be updated
                enqueue(instructions[i][1]);
            }
            printf("\n");
        }

        // Reset queue and current_stage
        for (int i = 0; i < 2; i++) {
            strcpy(unavailables[i], "NULL");
        }
        current_stage = 0;

        // Print Sequence, w/ stalling
        printf("Stalling, No Forwarding:\n");
        for (int i = 0; i < imax; i++) {
            printf("   ");
            // Print lw format
            if (strcmp(instructions[i][0], "lw") == 0) {
                // Add first register to unavailable queue
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                print_stages_nf(instructions[i][2]); // Second reg must be available, otherwise stall
                enqueue(instructions[i][1]); // First reg is unavailable after lw
            // Print sw format
            } else if(strcmp(instructions[i][0], "sw") == 0) {
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                print_stages2_nf(instructions[i][1], instructions[i][2]); // First and second reg's must be available, otherwise stall
            // Print add and sub format
            } else {
                printf("%-3s %-3s, %-3s, %-3s \t", instructions[i][0], instructions[i][1], instructions[i][2], instructions[i][3]);
                print_stages2_nf(instructions[i][2], instructions[i][3]); // Second and third reg's must be available, otherwise stall
                enqueue(instructions[i][1]); // First register is unavailable after add/sub
            }
            printf("\n");
        }

        // Resent current stage and unavailable
        current_stage = 0;
        unavailable = "NULL";

        // Print Sequence, w/ forwarding
        printf("Stalling, With Forwarding:\n");
        for (int i = 0; i < imax; i++) {
            printf("   ");
            // Print lw format
            if (strcmp(instructions[i][0], "lw") == 0) {
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                print_stages_wf(instructions[i][2]); // Second reg must be available, otherwise stall
                unavailable = instructions[i][1]; // First reg is unavailable after lw
            // Print sw format
            } else if(strcmp(instructions[i][0], "sw") == 0) {
                printf("%-3s %-3s, %-2s(%-3s)  \t", instructions[i][0], instructions[i][1], offsets[i], instructions[i][2]);
                print_stages_wf(instructions[i][2]); // First and second reg's must be available, otherwise stall
            // Print add and sub format
            } else {
                printf("%-3s %-3s, %-3s, %-3s \t", instructions[i][0], instructions[i][1], instructions[i][2], instructions[i][3]);
                print_stages2_wf(instructions[i][2], instructions[i][3]); // Second and third reg's must be available, otherwise stall
                // With forwarding, all registers are available by next stage
            }
            printf("\n");
        }

        // Check for repeating program
        printf("\nRepeat for another set of instructions? (Y/N)\n"); // NOT WORKING WITH Y
        scanf(" %c%*c", &repeat);

    } // end repeat while loop

    return 0;

} // end main()
