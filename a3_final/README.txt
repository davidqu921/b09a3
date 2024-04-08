Solving obstacles:
 1. First problem was left from the assignment1: the printing order. In my assignment1, printing is liner which means the number of cores and total cpu usage section is always printed after all memory usages were loaded and printed, however in the demo video, the printing for Memory usage section and CPU usage section should be printed simultaneously, this order is more clear when running "--graphics" function. To achieve the result of printing simultaneously, I used "printf("\e[1;1H\e[2J");" to clear the console first and then print an overall updated version after tdelay seconds. This solved the order of printing technically, but I realized that the result can vary depends on the terminal we used, when we redirect the result to a text file, it is more clear to see the code is doing its job correctly.
 
 2. The second problem to solve is the method we calculate Total CPU Usage, the provided
 function in Assignment3 is different from mine, the provided function required the T1, U1 (cpu utilization time from previous sampling time slot), and T2, U2 (cpu utilization time in current sampling time slot). Therefore, I rewrite my functions: getTotalCPUUsage() and readCPUTimes() to be able to store previous cpu utilization time records. 

3. The third problem is happened during the implementation of my first problem's solution, since
for each iteration we need to clear the consle and reprint every thing, we need a place to store the history memory usages and cpu usage from previous sampling times, otherwise we may always print the memory/cpu usages for current time slot. Most importantly, I need to be able to access this storing place and modify some data from my helper functions. Therefore, I decided to use a dynamic memory location to store these records for safe, it is easy to malloc() enough space since we know the number of samples.

4. For the concurrency: 
Overall, the code achieves concurrency by leveraging the fork() system call to create multiple processes that execute concurrently. Communication between processes is facilitated using pipes, allowing data to be passed between the parent and child processes. The parent process waits for child processes to finish using the wait() system call before continuing its execution.

5. The last problem I need to obstacle is to employ forking and piping to communicate with the main process. I rewrite gather_system_info(), gather_user_info(), gather_sequential_info() so that a child process will be created to implement those function independently and redirect their output back to parent pocess when they get called by themselves or from main operating functions: gather_memory_info() and gather_graphic-info().

An overview of the functions:
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

Guide to run:
This C program will report different metrics of the utilization of a given system.
The interruption signals' behaviour has been modified for Ctrl-Z and Ctrl-C. For the former, it will just ignore it as the program should not be run in the background while running interactively. For the latter, the program will ask the user whether it wants to quit or not the program.

Accepted Command Line Arguments
  => Directly running the program will print all memory, user, cpu, system information we can provide with default samples=10 and tdelay=1.
  => Running with command line arguments (order dose not matter):
       -> --system : to indicate that only the system usage should be generated
       -> --user : to indicate that only the users usage should be generated
       -> --graphic : to include graphical output in the cases where a graphical outcome includes: all memory, user, cpu, system information
       -> --sequential : to indicate that the information will be output sequentially without needing to "refresh" the screen, do not
           combine with any other command line argument
       -> --samples=N : if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions
       -> --tdelay=T : to indicate how frequently to sample in seconds

Example: ./myprog // print all memory, user, cpu, system information we can provide with default samples=10 and tdelay=1.
	 ./myprog 5 2 // print all memory, user, cpu, system information we can provide with samples=5 and tdelay=2.
 	 ./myprogram --samples=5 --tdelay=2 // print all memory, user, cpu, system information we can provide with samples=5 and tdelay=2.
 	 ./myprog --samples=5 --tdelay=2 --system // print memory, cpu, system information with samples=5 and tdelay=2.
 	 ./myprog --user // print user and system information in current time.
 	 ./myprog 5 2 --sequential // print each time slot's memory usage and cpu usage seperately in a connected sequence with samples=5 and tdelay=2.
 	 ./myprog 5 2 --graphics // print memory usage graph and cpu usage graph along with user and system information in samples=5 and tdelay=2. If any additional flag is given then we will not print the information for ungiven flag, otherwise --graphics is default to print everything we can provided just act like demo video.


