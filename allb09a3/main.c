/**
 * Program: System Monitoring Tool
 * Description: This C program will report different metrics of the utilization of a given system.
 *              The interruption signals' behaviour has been modified for Ctrl-Z and Ctrl-C. For the former, 
 *              it will just ignore it as the program should not be run in the background while running interactively.
 *              For the latter, the program will ask the user whether it wants to quit or not the program.
 * 
 * Accepted Command Line Arguments
 * => Directly running the program will print all memory, user, cpu, system information we can provide with default sample=10 and tdelay =1.
 * => Running with command line arguments (order dose not matter):
 *      -> --system : to indicate that only the system usage should be generated
 *      -> --user : to indicate that only the users usage should be generated
 *      -> --graphic : to include graphical output in the cases where a graphical outcome includes: all memory, user, cpu, system information
 *      -> --sequential : to indicate that the information will be output sequentially without needing to "refresh" the screen, do not
 *          combine with any other command line argument
 *      -> --samples=N : if used the value N will indicate how many times the statistics are going to be collected and results will
 *          be average and reported based on the N number of repetitions
 *      -> --tdelay=T : to indicate how frequently to sample in seconds
 *              
 * 
 * Author: David Qu
 * Organization: University of Toronto Scarborough
 * Course Code: CSCB09-Winter 
 * Date Created: April 1st, 2024
 * Last Modified: April 8th, 2024
 * 
 * StudentID: 1007653585
 * School email: davidsz.qu@mail.utoronto.ca
 *
 */

#include "stats_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <utmpx.h>
#include <ctype.h>
#include <signal.h>

void sigtstp_handler(int signum){
    printf("\nCtrl-Z (SIGTSTP) reveived. Ignoring...\n");
}

void sigint_handler(int signum){
    char choice;
    printf("\nCtrl-C (SIGINT) received. Do you want to quit? (Y/N): ");
    scanf("%c", &choice);

    if (choice == 'y' || choice == 'Y'){
        printf("Exiting...\n");
        exit(0);
    }
    else {
        printf("Continuing...\n");
    }

}

int main(int argc, char *argv[]) {
    int samples = 10, tdelay = 1; // Default values
    int system_flag = 0, user_flag = 0, sequential_flag = 0, graphics_flag = 0 ;

    signal(SIGTSTP, sigtstp_handler);
    signal(SIGINT, sigint_handler);

    // Parse command-line arguments
    parse_arguments(argc, argv, &samples, &tdelay, &system_flag, &user_flag, &sequential_flag, &graphics_flag);

    // Validate arguments and decide on the action
    if (system_flag && graphics_flag == 0) {
       if(sequential_flag != 1){
		    gather_memory_info(samples, tdelay, user_flag);
        }
    }
    
    if (user_flag && graphics_flag == 0) {
        gather_user_info();
        gather_system_info_noCore();
    }
	
	if(sequential_flag && graphics_flag == 0){
		gather_sequential_info(samples,tdelay);
	}

    if (graphics_flag){
        gather_graphics_info(samples,tdelay,system_flag, user_flag);
    }

    return 0;
}