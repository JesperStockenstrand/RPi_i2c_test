#include<stdio.h>	//printf
#include<string.h>	//memset
#include<errno.h>	//errno
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
    //int i_cpu_load;
    FILE *fp;

    fp = fopen("/proc/stat","r"); 
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3],&a[4],&a[5],&a[6]);
    fclose(fp);
    
	/*
	usleep(500000);
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3],&b[4],&b[5],&b[6]);
    fclose(fp);

	*/

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
				//printf("Default interface is : %s \n" , p);
				break;
			}
		}
	}
    
    //which family do we require , AF_INET or AF_INET6
    int fm = AF_INET;
    struct ifaddrs *ifaddr, *ifa;
	int family , s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) 
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	//Walk through linked list, maintaining head pointer so we can free list later
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
					printf("getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}
				
				//printf("address: %s", host);
                return(host);
			}
			//printf("\n");
		}
	}

	freeifaddrs(ifaddr);
	
	return("0.0.0.0");
}



int main(void)
{
	cpu_interval = 0;
    int mem_load;
	int display_mode = mode_CPU; //Init to CPU
	char tmp_value[20];
	mem_tot = mem_total()/1024;
    for(;;) {
    		int cpu_l;
		//switch case display_mode
		switch (display_mode) {
		
			case mode_CPU:
				//tmp_value = " ";	
				cpu_l = cpu_load();
				//printf("cpu: %d%%\n", cpu_l);
				sprintf(tmp_value,"         %d%%",cpu_l);
				//tmp_chars = "        ";
				//tmp_line = strcat(tmp_chars,tmp_value);
				//tmp_line = strcat(tmp_line,"%");
				lcd_line("      CPU LOAD  ");
				lcd_line(tmp_value);
				lcd_line("   ");
				lcd_line(">CPU<[MEM][NET][UPT]"); 
				break;
				
			case mode_MEM:
				mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
				//printf("mem: %d/%d %d%%\n",mem_used(),mem_tot,mem_load);
				lcd_line("     MEM USAGE      ");
				sprintf(tmp_value,"       %d/%dMb",mem_used(),mem_tot);
				lcd_line(tmp_value);
				sprintf(tmp_value,"         %d%%",mem_load);
				lcd_line(tmp_value);
				lcd_line("[CPU]>MEM<[NET][UPT]");
				break;
				
			case mode_NET:
				//printf("ip: %s\n", net_address());
				lcd_line("    IP ADDRESS      ");
				sprintf(tmp_value,"    %s", net_address());
				lcd_line(tmp_value);
				lcd_line("      ");
				lcd_line("[CPU][MEM]>NET<[UPT]");
				break;
				
			case mode_UPT:
				//printf("uptime: %d seconds\n", uptime());
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
	
	} //loop
    return(0);
}

