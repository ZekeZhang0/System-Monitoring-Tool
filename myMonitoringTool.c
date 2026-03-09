#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <utmp.h>

typedef struct {
    int samples;
    long tdelay;
    int show_memory;
    int show_cpu;
    int show_users;
}Settings;

//Initialize default settings for the monitoring tool 
//Sets: samples=20, tdelay=500000, all display flags to 1 (show all)
void initializeSettings(Settings *s){
    (*s).samples = 20;
    (*s).tdelay = 500000;
    (*s).show_memory = 1;
    (*s).show_cpu = 1;
    (*s).show_users = 1;
}

//Determine if a string is a non negative integer. Returns 1 if all characters are digits and are non negative, 0 otherwise
int isNonNegativeInteger(char* str){
    if(*str == '\0'){
        return 0;
    }
    for(int i=0;str[i]!='\0';i++){
        if(!isdigit(str[i])){///////
            return 0;
        }
    }
    return 1;
}

//Read command line arguments and update settings accordingly Handles both positional arguments (samples, tdelay) and flagged arguments 
void readArg(int argc, char **argv, Settings *settings){
    int idx = 1;
    //postional arguments:
    if(idx<argc && isNonNegativeInteger(argv[idx])){
        (*settings).samples = atoi(argv[idx]);
        idx+=1;
    }
    if(idx<argc && isNonNegativeInteger(argv[idx])){
        (*settings).tdelay = strtol(argv[idx],NULL,10);
        idx+=1;
    }
    //Flag: If any of the following occurs, 
    // we will no longer use the default settings.
    int memory_flag = 0;
    int cpu_flag = 0;
    int users_flag = 0;

    while(idx<argc){
        if(strcmp(argv[idx],"--memory")==0){
            memory_flag = 1;
        }
        else if(strcmp(argv[idx],"--cpu")==0){
            cpu_flag = 1;
        }
        else if(strcmp(argv[idx],"--users")==0){
            users_flag = 1;
        }
        else if(strncmp(argv[idx],"--samples=",10)==0){
            long samples_val = strtol(argv[idx]+10,NULL,10);
            (*settings).samples = (int) samples_val;
        }
        else if(strncmp(argv[idx],"--tdelay=",9)==0){
            long tdelay_val = strtol(argv[idx]+9,NULL,10);
            (*settings).tdelay = tdelay_val;
        }
        idx+=1;
    }

    if (memory_flag || cpu_flag || users_flag) {
        (*settings).show_memory = memory_flag;
        (*settings).show_cpu = cpu_flag;
        (*settings).show_users = users_flag;
    }
}

//==============memory=================

//Read current memory usage from system
// Returns 0 on success, -1 on error
int readMemoryUsage(double *used_gb, double *total_gb){
    struct sysinfo info;

    if (sysinfo(&info) != 0){
        return -1;
    }

    double total_bytes = (double)info.totalram * info.mem_unit;
    double free_bytes = (double)info.freeram * info.mem_unit;
    double used_bytes = total_bytes - free_bytes;
    * total_gb = total_bytes / (1024.0 * 1024 * 1024);
    * used_gb = used_bytes / (1024.0 * 1024 * 1024);
    return 0;
}


//Draw graph showing memory usage over time
void drawMemoryGraph(double used_gb, double total_gb, int samples, int graphHeight, int mem_rows[],int current_sample){

    printf(">>> Memory  %.2f GB\n", used_gb);
    for(int row = graphHeight;row>=1;row--){
        if(row==graphHeight){
            printf("%5.0f GB |", total_gb);
        }else{
            printf("         |");
        }

        for(int col = 0;col<samples;col++){
            if(col <= current_sample){
                if(mem_rows[col] == row){
                    printf("#");
                }
                else{
                    printf(" ");
                }
            }
            else{
                printf(" ");
            }
        }

        printf("\n");
    }
    printf("  0 GB  ");
    for(int i = 0;i<(samples+2);i++){
        printf("-");
    }

    printf("\n");

}
/////////////////////////////////////////////////////////////////////////////////////////////

//==============CPU=================
//Read cpu infomation
int readCPU(unsigned long long *idle, unsigned long long *total){
    FILE *fp = fopen("/proc/stat", "r");
    if(!fp){
        return -1;
    }

    char cpu_label[16];

    unsigned long long user, nice, system, idle_time, iowait, irq, softirq, steal;

    
    int returnVal = fscanf(fp, "%15s %llu %llu %llu %llu %llu %llu %llu %llu",
                     cpu_label,
                     &user, &nice, &system, &idle_time, &iowait, &irq, &softirq, &steal);

    fclose(fp);

    if(returnVal < 5){
        return -1;
    }

    unsigned long long idle_all = idle_time + iowait;
    unsigned long long total_all = user + nice + system + idle_all + irq + softirq + steal;
    *idle = idle_all;
    *total = total_all;


    return 0;
}

//calculate usage percent
int getCPUUsage(double *percent, unsigned long long *prev_idle, unsigned long long *prev_total){
    unsigned long long idle, total;

    if(readCPU(&idle, &total) != 0){
        return -1;
    }
    //initialize:
    if(*prev_total == 0){
        *prev_idle = idle;
        *prev_total = total;
        *percent = 0.0;
        return 0;
    }

    unsigned long long idle_diff  = idle  - *prev_idle;
    unsigned long long total_diff = total - *prev_total;

    *prev_idle = idle;
    *prev_total = total;

    if(total_diff == 0){
        *percent = 0.0;
        return 0;
    }

    *percent = ((double)(total_diff - idle_diff) / (double)total_diff) * 100.0;

    if(*percent < 0){
        *percent = 0;
    }
    if(*percent > 100){
        *percent = 100;
    }

    return 0;
}

//
void drawCPUGraph(double percent, int samples, int graphHeight, int cpu_rows[], int current_sample){
    printf(">>> CPU  %.2f %%\n", percent);

    for(int row = graphHeight; row >= 1; row--){
        if(row == graphHeight){
            printf("%6s %% |", "100");
        }else{
            printf("         |");
        }

        for(int col = 0; col < samples; col++){
            if(col <= current_sample){
                if(cpu_rows[col] == row){
                    printf(":");
                }
                else{
                    printf(" ");
                }
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
    }

    printf("  0 %%  ");
    for(int i = 0; i < samples+2; i++){
        printf("-");
    }
    printf("\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////

//==============Current user=================
void printCurrentUsers(void){
    struct utmp *entry;
    setutent();

    printf("----------------------------------------------\n");
    printf("### Sessions/users ###\n");

    while((entry = getutent()) != NULL){

        if(entry->ut_type == USER_PROCESS){
            if(entry->ut_host[0] != '\0'){
                printf("%-10s %-6s (%s)\n", entry->ut_user, entry->ut_line, entry->ut_host);
            }
            else{
                printf("%-10s %-6s ()\n", entry->ut_user, entry->ut_line);
            }
        }
    }

    endutent();
    printf("----------------------------------------------\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
void monitorSystem(Settings *s){
    int graphHeight = 12;

    int mem_rows[s->samples];
    int cpu_rows[s->samples];

    for(int i = 0; i < s->samples; i++){
        mem_rows[i] = 0;
        cpu_rows[i] = 0;
    }

    unsigned long long prev_idle = 0;
    unsigned long long prev_total = 0;

    printf("\033[2J");
    fflush(stdout);

    for(int i = 0; i < s->samples; i++){
        printf("\033[H");
        
        printf("Nbr of samples: %d -- every %ld microSecs (%.3f secs)\n\n", s->samples, s->tdelay, (s->tdelay / 1000000.0));

        // Memory
        if(s->show_memory){
            double used_gb, total_gb;
            if(readMemoryUsage(&used_gb, &total_gb) == 0){
                int bar_row = (int)((used_gb / total_gb) * graphHeight);
                if(bar_row < 1) bar_row = 1;
                if(bar_row > graphHeight) bar_row = graphHeight;

                mem_rows[i] = bar_row;
                drawMemoryGraph(used_gb, total_gb, s->samples, graphHeight, mem_rows, i);
                printf("\n");
            }
            else{
                printf("Memory Error!\n\n");
            }
        }

        //CPU height = 8
        if(s->show_cpu){
            double usage = 0.0;
            if(getCPUUsage(&usage, &prev_idle, &prev_total) == 0){
                int bar_row = (int)((usage / 100.0) * 8);
                if(bar_row < 1){
                    bar_row = 1;
                }
                if(bar_row > 8){
                    bar_row = 8;
                }

                cpu_rows[i] = bar_row;

                drawCPUGraph(usage, s->samples, 8, cpu_rows, i);
                printf("\n");
            }
            else{
                printf("CPU Error!\n\n");
            }
        }

        //Users
        if(s->show_users){
            printCurrentUsers();
        }
            

        fflush(stdout);
        usleep(s->tdelay);
 
    }

    printf("\033[0m\n");

}


///////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){
    Settings s;
    initializeSettings(&s);
    readArg(argc, argv, &s);
    monitorSystem(&s);
    return 0;
}