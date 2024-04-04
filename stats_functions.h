#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <utmpx.h>
#include <ctype.h>

int isAllDigit(char str[]);

void parse_arguments(int argc, char *argv[], int *samples, int *tdelay, int *system_flag, int *user_flag, int *sequential_flag, int *graphics_flag);

void convert_seconds_to_hms(int seconds, char *buffer);

void convert_seconds_to_dhms(int seconds, char *buffer);

int getCPUCoreCount();

double getTotalCPUUsage();

void gather_system_info();

void gather_system_info_noCore();

void gather_memory_info(int samples,int tdelay);

void gather_user_info();

void print_each_sequential(int iterationTime, int samples);

void gather_sequential_info(int samples,int tdelay);

void print_memory_graph(double previous, double current);

void print_cpu_graph(double usage);

void display_usage();

void gather_graphics_info(int samples,int tdelay);

void display_usage();

