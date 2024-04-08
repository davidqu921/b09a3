#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <utmpx.h>
#include <ctype.h>

// check if a command line argument input is numeric
int isAllDigit(char str[]);

// check the command line argumets and collet all flags from the user's input
void parse_arguments(int argc, char *argv[], int *samples, int *tdelay, int *system_flag, int *user_flag, int *sequential_flag, int *graphics_flag);

// Convert seconds to hours:minutes:seconds
void convert_seconds_to_hms(int seconds, char *buffer);

// Convert seconds to days and hours:minutes:seconds
void convert_seconds_to_dhms(int seconds, char *buffer);

// Function to get the number of CPU cores
int getCPUCoreCount();

// Function to get the total CPU usage
double getTotalCPUUsage();

// Gather and display user information here
void gather_system_info();

// Gather only information of operating system
void gather_system_info_noCore();

// Gather the memory usage for Physical and Virtual (Used/Total)
void gather_memory_info(int samples,int tdelay, int user_flag);

// Gather and display user information here
void gather_user_info();

// Get the total number of users
int get_user_num();

// Print all assorted information for each time slot in a sequence
void print_each_sequential(int iterationTime, int samples);

// Gather the memory, user, cpu, system information for a single time slot
void gather_sequential_info(int samples,int tdelay);

// Store the memory usage graph into a buffer
void store_memory_graph(char *str, double previous, double current);

// Store the cpu usage graph into a buffer
void store_cpu_graph(char *str, double usage) ;

// Warning for unacceptable arguments, and remind the acceptable ones
void display_usage();

// Gather and print the graphics for memory usage and cpu usage, if no additional flag,
// print the users and system information as well, if there is one more spcific flag has
// been given, then we just print the spcific required one except "user_flag", since this 
// function is aim to printing some graphs.
void gather_graphics_info(int samples,int tdelay, int system_flag, int user_flag);

