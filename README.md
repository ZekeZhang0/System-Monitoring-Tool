# Linux System Monitoring Tool

## TITLE -- myMonitoringTool

## Metadata
   * Author: Xiaoran Zhang
   * Date: 2026-2-6
   * Release/Version: 1.0


## Introduction/Rationale
This project is a real-time system monitoring tool for Linux that displays system metrics based on user-specified command-line arguments. The program can graphically display memory usage, CPU utilization, and the information of currently online users. Users can customize the displayed information, the number of samples collected, and the data collection frequency, making it a flexible tool to meet diverse monitoring needs.

## Description of how you solve/approach the problem
First, I implemented command-line argument parsing to support positional arguments (samples and tdelay) and flag arguments (--memory, --cpu, --users, --samples=N, --tdelay=T). Then, I broke the problem down into three separate parts: memory, CPU, and users. I implemented each monitoring metric using Linux's provided interfaces: sysinfo() for memory, /proc/stat for CPU, and utmp for connected users. Finally, I merged all the modules into a single sample loop so that the screen refreshes once per sample, and the display order is independent of the parameter order.

## Implementation
i. The program begins by parsing command line arguments and storing them in a Settings structure. Default values are applied when no arguments are provided. After initialization, the program enters a single loop that repeats for the number of requested samples. During each loop iteration, the program moves the cursor back to the top-left corner and prints the requested monitoring sections (Memory, CPU, Users). Each section reads new data, updates the history arrays for plotting, and prints a graph with a fixed axis size. The program sleeps for the requested delay before moving to the next sample.  

ii. The modules and functions：
- Read Command Line Argument Model
  - readArg(int argc, char **argv, Settings *settings)

- Memory Model
  - readMemoryUsage(double *used_gb, double *total_gb)
  - drawMemoryGraph(double used_gb, double total_gb, int samples, int graphHeight, int mem_rows[],int current_sample)

- CPU Model
  - readCPU(unsigned long long *idle, unsigned long long *total)
  - getCPUUsage(double *percent, unsigned long long *prev_idle, unsigned long long *prev_total)
  -drawCPUGraph(double percent, int samples, int graphHeight, int cpu_rows[], int current_sample)

- Users Model
  - printCurrentUsers(void)

- Monitoring Loop
  - monitorSystem(Settings *s)

iii. Describe the functions in sentences, e.g. this function does this and that and uses this and that sources, system calls, libraries, etc.

- initializeSettings(): Sets up the default monitoring configuration with all metrics enabled, 20 samples, and 500000 microsecond intervals.

- readArg(): This function checks for both positional arguments (samples and tdelay) and flagged arguments (--memory, --cpu, --users, --samples=N, --tdelay=T) via command line arguments. The function updates the Settings struct by applying default values when no inputs are provided, validating numeric values, and enabling or disabling the correct monitoring sections based on the user’s request.

- readMemoryUsage(): This function uses the Linux system call sysinfo() from the library <sys/sysinfo.h> to access system memory statistics. It reads the total RAM and free RAM values from the returned struct sysinfo, converts the values into bytes using the mem_unit field, and then converts bytes into GB. It returns the computed used memory and total memory through the pointer parameters.

- drawMemoryGraph(): Creates a vertical bar graph showing memory usage over time, with the Y-axis scaled to the total available memory.

- readCPU(): Opens and reads the /proc/stat file to extract CPU time breakdowns for different states (user, system, idle, etc.).

- getCPUUsage(): Calculates the percentage of CPU utilization by comparing idle and total CPU time between two consecutive measurements.

- drawCPUGraph(): Creates a vertical bar graph showing CPU utilization percentage over time, with a fixed Y-axis from 0-100%.

- printCurrentUsers(): Iterates through the utmp database entries, filtering for active user processes and displaying their usernames, terminals, and host information.

- monitorSystem(): The main loop that coordinates the entire monitoring process. It checks which metrics should be displayed based on user instructions and only calls the corresponding functions, ensuring the output matches what the user requested.

     

### Include a flow chart or diagram of your program and corresponding functions' calls.


<img width="771" height="571" alt="a1 drawio" src="https://github.com/user-attachments/assets/2c96545f-c083-4a27-a60c-b90ea514ee76" />


## Instructions in how to compile your code
1.Ensure the file is in your current working directory.

2.Use gcc compiler with standard libraries:
- gcc -Wall -o myMonitoringTool myMonitoringTool.c
  
3. Run the Program
- CLA Syntax: ./myMonitoringTool [samples [tdelay]] [--memory] [--cpu] [--users] [--samples=N] [--tdelay=T]
  - --memory: to indicate that only the memory usage should be generated
  - --cpu: to indicate that only the CPU usage should be generated
  - --users: to indicate that a list of users currently connected should be generated
  - --samples=N: if used the value N will indicate how many times the statistics are going to be collected and results will be average and reported based on the N number of repetitions. If not value is indicated the default value will be 20.
  - --tdelay=T: to indicate how frequently to sample in micro-seconds (1 microsec = 10 -6 sec.) If not value is indicated the default value will be 0.5 sec = 500 milisec = 500000 microsec.

       

## Expected Results
When run with no arguments:
./myMonitoringTool

The program prints:

The number of samples and delay frequency

A memory utilization graph in GB

A CPU utilization graph in %

A list of current users connected

The screen refreshes once per sample and graphs update in real time.
### Examples:
./myMonitoringTool
```
Nbr of samples: 20 -- every 500000 microSecs (0.500 secs)

>>> Memory  8.25 GB
   15 GB |                    
         |                    
         |                    
         |                    
         |                    
         |                    
         |####################
         |                    
         |                    
         |                    
         |                    
         |                    
  0 GB  ----------------------

>>> CPU  0.70 %
   100 % |                    
         |                    
         |                    
         |                    
         |                    
         |                    
         |                    
         |::::::::::::::::::::
  0 %  ----------------------

----------------------------------------------
### Sessions/users ###
ebadimea   seat0  (login screen)
abramo44   pts/1  (138.51.8.23)
zhaoyip4   pts/11 (tmux(554052).%1)
zhaoyip4   pts/7  (tmux(554052).%0)
huss1163   pts/10 (tmux(290207).%10)
huss1163   pts/12 (tmux(290207).%6)
teskyeho   pts/16 (tmux(539437).%1)
huss1163   pts/3  (tmux(290207).%4)
huss1163   pts/5  (tmux(290207).%9)
huss1163   pts/6  (tmux(290207).%12)
huss1163   pts/8  (tmux(290207).%2)
abramo44   pts/0  (138.51.9.160)
abelfinn   pts/13 (100.101.139.209)
teskyeho   pts/15 (tmux(539437).%0)
teskyeho   pts/17 (tmux(539437).%2)
abramo44   pts/19 (138.51.9.160)
huss1163   pts/25 (tmux(290207).%8)
abramo44   pts/26 (138.51.9.160)
zha16531   pts/27 (138.51.12.249)
huss1163   pts/22 (tmux(290207).%11)
----------------------------------------------


```
## Test Cases
1. Default run:

./myMonitoringTool
Expected: memory, cpu, and users are printed with default samples=20 and tdelay=500000.

2. Only memory:

./myMonitoringTool --memory
Expected: only memory section is displayed with default samples=20 and tdelay=500000.

3. Only cpu:

./myMonitoringTool --cpu
Expected: only cpu section is displayed with default samples=20 and tdelay=500000.

4. Only users:

./myMonitoringTool --users
Expected: only users section is displayed with default samples=20 and tdelay=500000.

5. Combined flags:

./myMonitoringTool --cpu --users
Expected: CPU section then Users section with default samples=20 and tdelay=500000.

6. Positional arguments:

./myMonitoringTool 10 100000
Expected: samples=10 and tdelay=100000.

7. Only samples

./myMonitoringTool 50
Expected: memory, cpu, and users are printed with samples=50 and tdelay=500000.

## Disclaimers
1. The program is designed for Linux systems that provide /proc/stat and utmp.
2. The program must be executed using the required syntax:

  ./myMonitoringTool  [samples [tdelay]] [--memory] [--cpu] [--users] [--samples=N] [--tdelay=T]

