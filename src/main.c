/* RPi_i2c_test - Used together with schematics at http://raspify.stockenstrand.com  
 * Copyright (C) 2013 Jesper Stockenstrand
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.	
 */

#include<stdio.h>    
#include<string.h>    
#include<errno.h>    
#include<sys/socket.h>
#include<netdb.h>
#include<ifaddrs.h>
#include<stdlib.h>
#include<unistd.h>

#define mode_CPU 1
#define mode_MEM 2
#define mode_NET 3
#define mode_UPT 4

int mem_tot;
long double b[7];
int cpu_interval;
int i_cpu_load;

int cpu_load() {
    if (cpu_interval == 10) {
    long double a[7],loadavg;
        FILE *fp;

        fp = fopen("/proc/stat","r"); 
        fscanf(fp,"%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6]);
        fclose(fp);

        loadavg = ((b[0]+b[1]+b[2]+b[4]+b[5]+b[6]) - (a[0]+a[1]+a[2]+a[4]+a[5]+a[6]))
             / ((b[0]+b[1]+b[2]+b[3]+b[4]+b[5]+b[6]) - (a[0]+a[1]+a[2]+a[3]+a[4]+a[5]+a[6]));
        i_cpu_load = (int)(loadavg * 100);
        
        b[0] = a[0];
        b[1] = a[1];
        b[2] = a[2];
        b[3] = a[3];
        b[4] = a[4];
        b[5] = a[5];
        b[6] = a[6];
    
        cpu_interval = 1;
    } else {
        cpu_interval++;
    }
    return(i_cpu_load);
}

int mem_total() {
    long double a;
    int i_mem_total;
    FILE *fp;

    fp = fopen("/proc/meminfo","r"); 
    fscanf(fp,"%*s %Lf",&a);
    fclose(fp);
    i_mem_total = (int)a;
    return(i_mem_total);
}

int mem_used() {
    long double a;
    int i_mem_used;
    char c;
    
    FILE *fp;

    fp = fopen("/proc/meminfo","r"); 
    do
        c = fgetc(fp);
    while (c != '\n');
    fscanf(fp,"%*s %Lf",&a);
    fclose(fp);
    i_mem_used = mem_tot - ((int)a/1024);
    return(i_mem_used);
}

int uptime() {
    long double a;
    int i_uptime;
       
    FILE *fp;

    fp = fopen("/proc/uptime","r"); 
    fscanf(fp,"%Lf",&a);
    fclose(fp);
    i_uptime = (int)a;
    return(i_uptime);
}

char* net_address()
{
    FILE *f;
    char line[100] , *p , *c;
    
    f = fopen("/proc/net/route" , "r");
    
    while(fgets(line , 100 , f))
    {
        p = strtok(line , " \t");
        c = strtok(NULL , " \t");
        
        if(p!=NULL && c!=NULL)
        {
            if(strcmp(c , "00000000") == 0)
            {
                break;
            }
        }
    }
    
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
    int family , s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        family = ifa->ifa_addr->sa_family;

        if(strcmp( ifa->ifa_name , p) == 0)
        {
            if (family == fm) 
            {
                s = getnameinfo( ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6) , host , NI_MAXHOST , NULL , 0 , NI_NUMERICHOST);
                
                if (s != 0) 
                {
                    exit(EXIT_FAILURE);
                }
                
                return(host);
            }
        }
    }

    freeifaddrs(ifaddr);
    
    return("0.0.0.0");
}



int main(void)
{
    cpu_interval = 0;
    int mem_load;
    int display_mode = mode_CPU; 
    char tmp_value[20];
    mem_tot = mem_total()/1024;
    
    lcd_line("    RPi i2c test");
    lcd_line("    Copyright (C)");
    lcd_line("        2013");
    lcd_line("Jesper Stockenstrand");
    usleep(5000000);
    for(;;) {
        int cpu_l;
            
        switch (display_mode) {
        
            case mode_CPU:
                cpu_l = cpu_load();
                sprintf(tmp_value,"         %d%%",cpu_l);
                lcd_line("      CPU LOAD  ");
                lcd_line(tmp_value);
                lcd_line("   ");
                lcd_line(">CPU<[MEM][NET][UPT]"); 
                break;
                
            case mode_MEM:
                mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
                lcd_line("     MEM USAGE      ");
                sprintf(tmp_value,"       %d/%dMb",mem_used(),mem_tot);
                lcd_line(tmp_value);
                sprintf(tmp_value,"         %d%%",mem_load);
                lcd_line(tmp_value);
                lcd_line("[CPU]>MEM<[NET][UPT]");
                break;
                
            case mode_NET:
                lcd_line("    IP ADDRESS      ");
                sprintf(tmp_value,"    %s", net_address());
                lcd_line(tmp_value);
                lcd_line("      ");
                lcd_line("[CPU][MEM]>NET<[UPT]");
                break;
                
            case mode_UPT:
                lcd_line("       UP TIME      ");
                sprintf(tmp_value,"     %d sec",uptime());
                lcd_line(tmp_value);
                lcd_line("     ");
                lcd_line("[CPU][MEM][NET]>UPT<");
                break;
                
            default:
                display_mode = mode_CPU;
                
        }
        
        switch (checkButton()) {
        
            case 1:
                display_mode = mode_CPU;
                break;
                
            case 2:
                display_mode = mode_MEM;
                break;
                
            case 3:
                display_mode = mode_NET;
                break;
            
            case 4:
                display_mode = mode_UPT;
                break;
                
        } 

            
        usleep(10000);
    
    } 
    return(0);
}

