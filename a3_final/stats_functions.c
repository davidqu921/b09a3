#include "stats_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <utmpx.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

// Function prototypes
void parse_arguments(int argc, char *argv[], int *samples, int *tdelay, int *system_flag, int *user_flag, int *sequential_flag, int *graphics_flag);
void gather_system_info();
void gather_memory_info(int samples,int tdelay, int user_flag);
void gather_user_info();
void gather_sequential_info(int samples,int tdelay);
void display_usage();
void gather_graphics_info(int samples,int tdelay, int system_flag, int user_flag);
void gather_system_info_noCore();


int isAllDigit(char str[]){
    // check if a command line argument input is numeric
    int isInt = 1;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!isdigit(str[i])) {
            isInt = 0; // Not an integer
            break;
        }
    }
    return isInt;
}

void parse_arguments(int argc, char *argv[], int *samples, int *tdelay, int *system_flag, int *user_flag, int *sequential_flag, int *graphics_flag) {
    // check the command line argumets and collet all flags from the user's input
    if (argc == 1){
		*user_flag = 1;
		*system_flag = 1;
	}
	else{
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--system") == 0) {
				*system_flag = 1;
			} else if (strcmp(argv[i], "--user") == 0) {
				*user_flag = 1;
			} else if (strcmp(argv[i], "--sequential") == 0) {
				*sequential_flag = 1;
			} else if (strncmp(argv[i], "--samples=", 10) == 0) {
                *samples = atoi(argv[i] + 10);
                *system_flag = 1;
			} else if (strncmp(argv[i], "--tdelay=", 9) == 0) {
				*tdelay = atoi(argv[i] + 9);
                *system_flag = 1;
			} 
            else if (strcmp(argv[i], "--graphics") == 0) {
				*graphics_flag = 1;
			}
            else if (isAllDigit(argv[i]) == 1){
                if (*samples == 10){
                    *samples = strtol(argv[i], NULL, 10);
                    *system_flag = 1;
                    *user_flag = 1;
                }
                else if (*tdelay == 1){
                    *tdelay = strtol(argv[i], NULL, 10);
                }
            }
            else {
				printf("Unknown argument: %s\n", argv[i]);
				display_usage();
				exit(EXIT_FAILURE);
			}
		}
	}	
}

void convert_seconds_to_hms(int seconds, char *buffer) {
    // Calculate hours, minutes, and seconds
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int seconds_remaining = seconds % 60;

    // Write the formatted time to the buffer
    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds_remaining);
}

void convert_seconds_to_dhms(int seconds, char *buffer) {
    int days = seconds / (24 * 3600);
    seconds = seconds % (24 * 3600);
    int hours = seconds / 3600;
    seconds = seconds % 3600;
    int minutes = seconds / 60;
    seconds = seconds % 60;

    // Assuming buffer is large enough to hold the resulting string
    sprintf(buffer, "%d days %02d:%02d:%02d", days, hours, minutes, seconds);
}

// Function to get the number of CPU cores
int getCPUCoreCount() {
    FILE *fp = popen("nproc", "r");
    
    if (fp == NULL) {
        perror("Error opening pipe");
        exit(EXIT_FAILURE);
    }

    int coreCount;
    fscanf(fp, "%d", &coreCount);

    if (pclose(fp) == -1) {
        perror("Error closing pipe");
        exit(EXIT_FAILURE);
    }

    return coreCount;
}

void readCPUTimes(unsigned long long *cpu_sum, unsigned long long *cpu_no_idle) {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return;
    }

    char buffer[512];
    int cpu_count = 0;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strncmp(buffer, "cpu", 3) == 0) {
            if (cpu_count > 0) {
                // Extract CPU times for each CPU (excluding the first line which aggregates all CPUs)
                unsigned long long user, nice, system, idle, iowait, irq, softirq;
                sscanf(buffer, "cpu%*d %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait, &irq ,&softirq);

                // Sum all CPU times (excluding idle time)
                *cpu_sum += user + nice + system + idle + iowait + irq + softirq;
                // Sum idle time
                *cpu_no_idle += user + nice + system + iowait + irq + softirq;
            }
            cpu_count++;
        }
    }

    fclose(fp);
}

// Function to get the total CPU usage
double getTotalCPUUsage(unsigned long long *cpu_last_sum, unsigned long long *cpu_last_no_idle) {

    unsigned long long cpu_sum = 0;
    unsigned long long cpu_no_idle = 0;

    readCPUTimes(&cpu_sum, &cpu_no_idle);

    unsigned long long cpu_delta = cpu_sum - *cpu_last_sum;
    unsigned long long cpu_idle_delta = cpu_no_idle - *cpu_last_no_idle;

    *cpu_last_sum = cpu_sum;
    *cpu_last_no_idle = cpu_no_idle;

    double cpu_usage = 100.0 * (double) cpu_idle_delta / cpu_delta;

    return cpu_usage;
}

/*
void gather_system_info() {
    // Gather and display system information here
    unsigned long long cpu_last_sum = 0;
    unsigned long long cpu_last_no_idle = 0;
    long int time_in_seconds;
	int core_num;
	double percentage;
	char time_str[10];
	char day_str[50];
	struct utsname uname_data;
    uname(&uname_data);
	struct sysinfo info;
    sysinfo(&info);
	
	// Get core_num and percentage.
	core_num = getCPUCoreCount();
	percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);

	printf("Number of cores: %d\n", core_num);
	printf("total cpu use = %.2f%%\n", percentage);
	printf("---------------------------------------\n");
	printf("### System Information ###\n");
    printf("System Name = %s\n", uname_data.sysname);
    printf("Machine Name = %s\n", uname_data.nodename);
    printf("Version = %s\n", uname_data.version);
    printf("Release = %s\n", uname_data.release);
    printf("Architecture = %s\n", uname_data.machine);
	
	time_in_seconds = info.uptime;
	convert_seconds_to_hms(time_in_seconds,time_str);
	convert_seconds_to_dhms(time_in_seconds,day_str);
	
    printf("System running since last reboot: %s (%s)\n",day_str,time_str);
    printf("---------------------------------------\n");
    
}
*/

void gather_system_info() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close write end of pipe after duplication

        // Gather and display system information
        unsigned long long cpu_last_sum = 0;
        unsigned long long cpu_last_no_idle = 0;
        long int time_in_seconds;
        int core_num;
        double percentage;
        char time_str[10];
        char day_str[50];
        struct utsname uname_data;
        uname(&uname_data);
        struct sysinfo info;
        sysinfo(&info);
        
        // Get core_num and percentage.
        core_num = getCPUCoreCount();
        percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);

        printf("Number of cores: %d\n", core_num);
        printf("total cpu use = %.2f%%\n", percentage);
        printf("---------------------------------------\n");
        printf("### System Information ###\n");
        printf("System Name = %s\n", uname_data.sysname);
        printf("Machine Name = %s\n", uname_data.nodename);
        printf("Version = %s\n", uname_data.version);
        printf("Release = %s\n", uname_data.release);
        printf("Architecture = %s\n", uname_data.machine);
        
        time_in_seconds = info.uptime;
        convert_seconds_to_hms(time_in_seconds, time_str);
        convert_seconds_to_dhms(time_in_seconds, day_str);
        
        printf("System running since last reboot: %s (%s)\n", day_str, time_str);
        printf("---------------------------------------\n");

        // Exit the child process
        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        // Wait for the child process to complete
        wait(NULL);

        // Read data from the read end of the pipe
        char buffer[4096];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Close read end of pipe after reading
        close(pipefd[0]);

        // Print the system information received from the child process
        printf("%.*s", (int)bytes_read, buffer);
    }
}

/*
void gather_system_info_noCore(){
    // Gather and display system information here
    long int time_in_seconds;
	char time_str[10];
	char day_str[50];
	struct utsname uname_data;
    uname(&uname_data);
	struct sysinfo info;
    sysinfo(&info);
	
	printf("### System Information ###\n");
    printf("System Name = %s\n", uname_data.sysname);
    printf("Machine Name = %s\n", uname_data.nodename);
    printf("Version = %s\n", uname_data.version);
    printf("Release = %s\n", uname_data.release);
    printf("Architecture = %s\n", uname_data.machine);
	
	time_in_seconds = info.uptime;
	convert_seconds_to_hms(time_in_seconds,time_str);
	convert_seconds_to_dhms(time_in_seconds,day_str);
	
    printf("System running since last reboot: %s (%s)\n",day_str,time_str);
    printf("---------------------------------------\n");
}
*/

void gather_system_info_noCore() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close write end of pipe after duplication

        // Execute the function's code
        long int time_in_seconds;
        char time_str[10];
        char day_str[50];
        struct utsname uname_data;
        uname(&uname_data);
        struct sysinfo info;
        sysinfo(&info);

        printf("### System Information ###\n");
        printf("System Name = %s\n", uname_data.sysname);
        printf("Machine Name = %s\n", uname_data.nodename);
        printf("Version = %s\n", uname_data.version);
        printf("Release = %s\n", uname_data.release);
        printf("Architecture = %s\n", uname_data.machine);

        time_in_seconds = info.uptime;
        convert_seconds_to_hms(time_in_seconds, time_str);
        convert_seconds_to_dhms(time_in_seconds, day_str);

        printf("System running since last reboot: %s (%s)\n", day_str, time_str);
        printf("---------------------------------------\n");

        // Exit the child process
        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        // Wait for the child process to complete
        wait(NULL);

        // Read data from the read end of the pipe
        char buffer[4096];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Close read end of pipe after reading
        close(pipefd[0]);

        // Print the system information received from the child process
        printf("%.*s", (int)bytes_read, buffer);
    }
}

/*
int get_user_num(){
    struct utmpx *ut;
    int count = 0;
    // Open the utmpx file for reading
    setutxent();
    // Read each entry in the utmpx file
    while ((ut = getutxent()) != NULL) {
        // Check if the entry represents a user process
        if (ut->ut_type == USER_PROCESS) {
            count ++;
        }
    }
    // Close the utmpx file
    endutxent();
    return count;
}
*/

/*
void gather_user_info() {
    struct utmpx *ut;
    int count = 0;
    // Open the utmpx file for reading
    setutxent();

	printf("### Sessions/users ###\n");
    // Read each entry in the utmpx file
    while ((ut = getutxent()) != NULL) {
        // Check if the entry represents a user process
        if (ut->ut_type == USER_PROCESS) {
            // Print information for the user
            printf("%-13s %s (%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
        }
    }
    printf("---------------------------------------\n");

    // Close the utmpx file
    endutxent();
}
*/

void gather_user_info() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } 
    else if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close write end of pipe after duplication

        // Execute the function's code
        struct utmpx *ut;
        int count = 0;
        // Open the utmpx file for reading
        setutxent();

	    printf("### Sessions/users ###\n");
        // Read each entry in the utmpx file
        while ((ut = getutxent()) != NULL) {
            // Check if the entry represents a user process
            if (ut->ut_type == USER_PROCESS) {
                // Print information for the user
                printf("%-13s %s (%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
            }
        }
        printf("---------------------------------------\n");

        // Close the utmpx file
        endutxent();

        // Exit the child process
        exit(EXIT_SUCCESS);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        // Wait for the child process to complete
        wait(NULL);

        // Read data from the read end of the pipe
        char buffer[4096];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Close read end of pipe after reading
        close(pipefd[0]);

        // Print the system information received from the child process
        printf("%.*s", (int)bytes_read, buffer);
    }
}

void print_each_sequential(int iterationTime, int samples){
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end

        // Redirect stdout to the write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); // Close write end of pipe after duplication

        // Execute the function's code
        unsigned long long cpu_last_sum = 0;
        unsigned long long cpu_last_no_idle = 0;
        int core_num;
	    double percentage;
        struct sysinfo info;
	    sysinfo(&info);
    
        printf(">>> iteration %d\n", iterationTime);
	    printf("Memory usage: %ld kilobytes\n", (info.totalram-info.freeram)/1024);
	    printf("---------------------------------------\n");
	    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
	
        // Main loop for sampling 
        for (int i = 0; i < samples; i++) {
       
            if (i == iterationTime){
                sysinfo(&info);
                printf("%.2lf GB / %.2lf GB  -- %.2lf GB / %.2lf GB\n", (info.totalram-info.freeram)/(1024.0 * 1024 * 1024),info.totalram/(1024.0 * 1024 * 1024),
                    (info.totalswap + info.totalram - info.freeram -info.freeswap)/(1024.0 * 1024 * 1024),(info.totalswap + info.totalram)/(1024.0 * 1024 * 1024));
            }

            else if(i != iterationTime){
                printf("\n");
            }

        }
	    printf("---------------------------------------\n");
    
        gather_user_info();
    
        // Get core_num and percentage.
	    core_num = getCPUCoreCount();
	    percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);
	
	    printf("Number of cores: %d\n", core_num);
	    printf("total cpu use = %.2f%%\n", percentage);   
        
        // Exit the child process
        exit(EXIT_SUCCESS);
    } 
    else { // Parent process
        close(pipefd[1]); // Close unused write end

        // Wait for the child process to complete
        wait(NULL);

        // Read data from the read end of the pipe
        char buffer[4096];
        ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer));
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        // Close read end of pipe after reading
        close(pipefd[0]);

        // Print the system information received from the child process
        printf("%.*s", (int)bytes_read, buffer);
    }    
}

/*
void print_each_sequential(int iterationTime, int samples){
    unsigned long long cpu_last_sum = 0;
    unsigned long long cpu_last_no_idle = 0;
    int core_num;
	double percentage;
    struct sysinfo info;
	sysinfo(&info);
    
    printf(">>> iteration %d\n", iterationTime);
	printf("Memory usage: %ld kilobytes\n", (info.totalram-info.freeram)/1024);
	printf("---------------------------------------\n");
	printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot) \n");
	
    // Main loop for sampling 
    for (int i = 0; i < samples; i++) {
       
        if (i == iterationTime){
            sysinfo(&info);
            printf("%.2lf GB / %.2lf GB  -- %.2lf GB / %.2lf GB\n", (info.totalram-info.freeram)/(1024.0 * 1024 * 1024),info.totalram/(1024.0 * 1024 * 1024),
                (info.totalswap + info.totalram - info.freeram -info.freeswap)/(1024.0 * 1024 * 1024),(info.totalswap + info.totalram)/(1024.0 * 1024 * 1024));
        }

        else if(i != iterationTime){
            printf("\n");
        }

    }
	printf("---------------------------------------\n");
    
    gather_user_info();
    
    // Get core_num and percentage.
	core_num = getCPUCoreCount();
	percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);
	
	printf("Number of cores: %d\n", core_num);
	printf("total cpu use = %.2f%%\n", percentage);

}
*/

void gather_sequential_info(int samples,int tdelay){
    
    int jelly;
    for ( jelly = 0; jelly < samples ; jelly++)
    {
        print_each_sequential(jelly,samples);
        
        if (jelly < (samples - 1)) {
            sleep(tdelay);
        }
    }
    gather_system_info_noCore();  // The last iteration will print System Information
}


// Function to store graphical representation for memory into buffer
void store_memory_graph(char *str, double previous, double current) {
    char temp_buf[512];
    int change = (current - previous) * 100; // Assuming change is a percentage
    if (change < 0) {
        for (int i = 0; i > change ; i--){
            strcat(str, ":");
        }
        sprintf(temp_buf, "@ %.2f (%.2f)\n", current - previous, current);
        strcat(str, temp_buf);
    } 
    else if(change == 0){
        sprintf(temp_buf, "* %.2f (%.2f)\n", current - previous, current);
        strcat(str, temp_buf);
    }
    else {
        for (int i = 0; i < change; i++){
            strcat(str, "#");
        }
        sprintf(temp_buf, "* %.2f (%.2f)\n", current - previous, current);
        strcat(str, temp_buf);
    }
}

// Function to store graphical representation for CPU usage into buffer
void store_cpu_graph(char *str, double usage) {
    char temp_buf[512];
    int bars = (int) usage; // Assuming each bar represents 10% CPU usage
    strcat(str, "|||");
    for (int i = 0; i < bars; i++) {
        strcat(str, "|");
    }
    sprintf(temp_buf, " %.2f\n", usage);
    strcat(str, temp_buf);
}

void add_newline(char **array_of_string, int samples){
    for (int i = 0; i < samples; i++){
        strcpy(array_of_string[i], "\n");
    }
}


void store_user_info(char * user_buf){
    char temp_buf[512];
    struct utmpx *ut;
    int count = 0;
    // Open the utmpx file for reading
    setutxent();

    strcpy(user_buf, "### Sessions/users ###\n");
    // Read each entry in the utmpx file
    while ((ut = getutxent()) != NULL) {
        // Check if the entry represents a user process
        if (ut->ut_type == USER_PROCESS) {
            // Print information for the user
            sprintf(temp_buf, "%-13s %s (%s)\n", ut->ut_user, ut->ut_line, ut->ut_host);
            strcat(user_buf, temp_buf);
        }
    }
    strcat(user_buf, "---------------------------------------\n");

    // Close the utmpx file
    endutxent();

}


void gather_memory_info(int samples, int tdelay, int user_flag){
    char print_buf[2024];
    char temp_buf[512];
    char * user_buf = malloc(sizeof(char) * 2024);
    unsigned long long cpu_last_sum = 0;
    unsigned long long cpu_last_no_idle = 0;

    // Gather and display system information here
    int core_num;
	double percentage;
	struct sysinfo info;
    sysinfo(&info);

    char **memory_lines_buf = malloc(sizeof(char *) * samples);
    for (int i = 0; i < samples; i++){
        memory_lines_buf[i] = malloc(sizeof(char) * 512);
    }
    add_newline(memory_lines_buf, samples);

    double previous_memory_usage = (double)(info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024);

    sprintf(temp_buf, "Nbr of samples: %d -- every %d secs\n", samples, tdelay);
    strcpy(print_buf, temp_buf);
    sprintf(temp_buf, "Memory usage: %ld kilobytes\n", (info.totalram - info.freeram) / 1024);
    strcat(print_buf, temp_buf);
    strcat(print_buf, "---------------------------------------\n");
    strcat(print_buf, "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

    for (int i = 0; i < samples; i++) {
        printf("\e[1;1H\e[2J");
        //system("clear");
        printf("%s", print_buf);

        sysinfo(&info);
        double current_memory_usage = (double)(info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024);

        sprintf(temp_buf, "%.2lf GB / %.2lf GB  -- %.2lf GB / %.2lf GB\n", 
                (info.totalram - info.freeram) / (1024.0 * 1024 * 1024), 
                info.totalram / (1024.0 * 1024 * 1024),
                (info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024),
                (info.totalswap + info.totalram) / (1024.0 * 1024 * 1024));
        strcpy(memory_lines_buf[i], temp_buf);

        for (int i = 0; i < samples; i++){
            printf("%s", memory_lines_buf[i]);
        }

        printf("---------------------------------------\n");

        
        if (user_flag == 1){
            printf("%s", user_buf);
        }

        core_num = getCPUCoreCount();

	    percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);
	
	    printf("Number of cores: %d\n", core_num);
	    printf("total cpu use = %.2f%%\n", percentage);
        printf("---------------------------------------\n");

        if (i < samples - 1) {
            sleep(tdelay);
        }

    }
    gather_system_info_noCore();

    free(user_buf); 
    for (int i = 0; i < samples; i++){
        free(memory_lines_buf[i]);
    }
    free(memory_lines_buf); 
}


void gather_graphics_info(int samples,int tdelay, int system_flag, int user_flag){
    char print_buf[2024];
    char temp_buf[512];
    char * user_buf = malloc(sizeof(char) * 2024);
    unsigned long long cpu_last_sum = 0;
    unsigned long long cpu_last_no_idle = 0;
    
    store_user_info(user_buf);

    // Gather and display system information here
	int core_num;
	double percentage;
	struct sysinfo info;
    sysinfo(&info);
    double percentage_arr[samples];


    char **memory_lines_buf = malloc(sizeof(char *) * samples);
    for (int i = 0; i < samples; i++){
        memory_lines_buf[i] = malloc(sizeof(char) * 512);
    }
    add_newline(memory_lines_buf, samples);

    char **cpu_usage_buf = malloc(sizeof(char *) * samples);
    for (int i = 0; i < samples; i++){
        cpu_usage_buf[i] = malloc(sizeof(char) * 512);
    }
    add_newline(cpu_usage_buf, samples);

    
    double previous_memory_usage = (double)(info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024);

    sprintf(temp_buf, "Nbr of samples: %d -- every %d secs\n", samples, tdelay);
    strcpy(print_buf, temp_buf);
    sprintf(temp_buf, "Memory usage: %ld kilobytes\n", (info.totalram - info.freeram) / 1024);
    strcat(print_buf, temp_buf);
    strcat(print_buf, "---------------------------------------\n");
    strcat(print_buf, "### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

    for (int i = 0; i < samples; i++) {
        printf("\e[1;1H\e[2J");
        //system("clear");
        printf("%s", print_buf);

        sysinfo(&info);
        double current_memory_usage = (double)(info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024);

        sprintf(temp_buf, "%.2lf GB / %.2lf GB  -- %.2lf GB / %.2lf GB   |", 
                (info.totalram - info.freeram) / (1024.0 * 1024 * 1024), 
                info.totalram / (1024.0 * 1024 * 1024),
                (info.totalswap + info.totalram - info.freeram - info.freeswap) / (1024.0 * 1024 * 1024),
                (info.totalswap + info.totalram) / (1024.0 * 1024 * 1024));
        strcpy(memory_lines_buf[i], temp_buf);
        // Print graphical representation
        if (i == 0){
            sprintf(temp_buf, "o %.2f (%.2f)\n", current_memory_usage - previous_memory_usage, current_memory_usage);
            strcat(memory_lines_buf[i], temp_buf);
    
        }
        else{
            store_memory_graph(memory_lines_buf[i], previous_memory_usage, current_memory_usage);
            previous_memory_usage = current_memory_usage;
        }

        for (int i = 0; i < samples; i++){
            printf("%s", memory_lines_buf[i]);
        }

        printf("---------------------------------------\n");

        if (system_flag == 0 || user_flag == 1){
            printf("%s", user_buf);
        }

        core_num = getCPUCoreCount();


	    percentage = getTotalCPUUsage(&cpu_last_sum, &cpu_last_no_idle);
        percentage_arr[i] = percentage;
	
	    printf("Number of cores: %d\n", core_num);
	    printf("total cpu use = %.2f%%\n", percentage);

        sprintf(temp_buf, "%*c", 9, ' ');
        strcpy(cpu_usage_buf[i], temp_buf);

        store_cpu_graph(cpu_usage_buf[i], percentage_arr[i]);

        for (int i = 0; i < samples; i++){
            printf("%s", cpu_usage_buf[i]);
        }

        printf("---------------------------------------\n");

        gather_system_info_noCore();

        if (i < samples - 1) {
            sleep(tdelay);
        }

    }

    free(user_buf); 
    for (int i = 0; i < samples; i++){
        free(memory_lines_buf[i]);
    }
    free(memory_lines_buf); 

    for (int i = 0; i < samples; i++){
        free(cpu_usage_buf[i]);
    }
    free(cpu_usage_buf); 
}

void display_usage() {
    printf("Usage: ./program_name [--system] [--user] [--sequential] [--samples=N] [--tdelay=T]\n");
}


