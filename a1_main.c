/**
 * Program: System Monitoring Tool
 * Description: This C program will report different metrics of the utilization of a given system.
 * 
 * Author: David Qu
 * Date Created: Feburary 1, 2024
 * Last Modified: Feburary 4, 2024
 * 
 * StudentID: 1007653585
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

void sigstp_handler(int signum){
    printf("\nCtrl-Z (SIGTSTP) reveived. ignoring\n");
}

void sigint_handler(int signum){
    char choice;
    printf("\nCtrl-C (SIGINT) received. Do you want to quit? (y/n): ");
    

}

int main(int argc, char *argv[]) {
    int samples = 10, tdelay = 1; // Default values
    int system_flag = 0, user_flag = 0, sequential_flag = 0, graphics_flag = 0 ;

    // Parse command-line arguments
    parse_arguments(argc, argv, &samples, &tdelay, &system_flag, &user_flag, &sequential_flag, &graphics_flag);
	
    // Validate arguments and decide on the action
    if (system_flag && graphics_flag == 0) {
       if(sequential_flag != 1){
		    gather_memory_info(samples,tdelay);
			    if(user_flag == 1){
				    gather_user_info();
				    user_flag = 0;
			    }
		    gather_system_info();
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
        gather_graphics_info(samples,tdelay);
    }

    return 0;
}