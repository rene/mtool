/**
 * mtool: A simple tool to measure system resources
 * Copyright (C) 2015 Renê de Souza Pinto. All rights reserved.
 *
 * Author: Renê S. Pinto
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include "mtool.h"


/** SIGALRM callback */
void cb_sigalrm(int status);

/** SIGCHLD callback */
void cb_sigchld(int status);

/** Read stat[m] files (acquire a sample) */
void read_stat();

/** Child PID */
static pid_t child;

volatile char chdone;

volatile unsigned int samples_count = 0;
volatile struct pid_stats st_samples[STAT_SAMPLES];
char filename1[MAX_FNAME], filename2[MAX_FNAME], filename3[MAX_FNAME];

/**
 * main
 */
int main(int argc, const char *argv[])
{
	char *program;
	char **args;
	int i, status;
	struct rusage child_stats;
	struct tms cld_cpu;
	struct sigaction act;
	struct sigaction act2;

	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = cb_sigalrm;

	memset(&act2, 0, sizeof(struct sigaction));
	act2.sa_handler = cb_sigchld;


	if (argc < 2) {
		printf("Usage:\n\t%s <program_path>\n\n", argv[0]);
		return EXIT_FAILURE;
	} else {
		program = (char*)argv[1];
	}

	chdone = 0;

	if ((child = fork()) != 0) {
		/* Open child's /proc stat file */
		sprintf(filename1, "/proc/%u/stat", child);
		sprintf(filename2, "/proc/%u/statm", child);
		sprintf(filename3, "/proc/%u/io", child);
	
		printf("%s ", program);
		for (i = 2; i < argc; i++) {
			printf("%s ", argv[i]);
		}
		printf("\n");

		/* Program the timer */
		samples_count = 0;
		sigaction(SIGCHLD, &act2, NULL);
		sigaction(SIGALRM, &act, NULL);
		alarm(ALARM_TIME);

		/* now, let's wait for child's death */
		while(!chdone) {
			sleep(2);
		}
		if (waitpid(child, &status, 0) == child) {
			/* Stop alarms */
			alarm(0);
	
			/* Ok, show statistics */
			if (getrusage(RUSAGE_CHILDREN, &child_stats) == 0) {
				/* Show statistics */
				printf("%ld \n", child_stats.ru_maxrss);        /* maximum resident set size */
				printf("%ld \n", child_stats.ru_minflt);        /* page reclaims (soft page faults) */
				printf("%ld \n", child_stats.ru_majflt);        /* page faults (hard page faults) */
				printf("%ld \n", child_stats.ru_inblock);       /* block input operations */
				printf("%ld \n", child_stats.ru_oublock);       /* block output operations */
				printf("%ld \n", child_stats.ru_nvcsw);         /* voluntary context switches */
				printf("%ld \n", child_stats.ru_nivcsw);        /* involuntary context switches */
			} else {
				perror("getrusage()");
				exit(EXIT_FAILURE);
			}
		} else {
			/* Oops... */
			perror("wait()");
			exit(EXIT_FAILURE);
		}
		
	} else {
		/* Child */
		/* Redirect child's output to standard error */
		close(STDOUT_FILENO);
		dup(STDERR_FILENO);

		/* do execv */
		args = (char**)calloc(argc, sizeof(char*));
		if (args == NULL) {
			perror("malloc()");
			exit(EXIT_FAILURE);
		} else {
			args[0] = program;
			for (i = 1; i < argc; i++) {
				args[i] = (char*)argv[i+1];
			}
		}
		execv(program, args);

		/* On success, should never reaches here... */
		perror("execv()");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}


/**
 * SIGALRM callback
 */
void cb_sigalrm(int status)
{
	read_stat();
	alarm(ALARM_TIME);
}


/**
 * SIGCHLD callback
 */
void cb_sigchld(int status)
{
	chdone = 1;
}

/**
 * read_stat
 */
void read_stat()
{
	char format1[256], format2[256], comm[MAX_COMM_LEN + 1], line[256];
	struct pid_stats pst;
	int thread_nr;
	size_t len;
	FILE *fp1, *fp2, *fp3;
	unsigned long utime, stime;
	unsigned long long rbytes, wbytes;

	if (samples_count >= STAT_SAMPLES) {
		kill(child, SIGKILL);
		return;
	}
	
	if ((fp1 = fopen(filename1, "r")) == NULL) {
		perror("fopen()");
		exit(EXIT_FAILURE);
	}
	if ((fp2 = fopen(filename2, "r")) == NULL) {
		fclose(fp1);
		perror("fopen()");
		exit(EXIT_FAILURE);
	}
	if ((fp3 = fopen(filename3, "r")) == NULL) {
		fclose(fp1);
		fclose(fp2);
		perror("fopen()");
		exit(EXIT_FAILURE);
	}

	sprintf(format1, "%%*d (%%%ds %%*c %%*d %%*d %%*d %%*d %%*d %%*u %%lu %%lu"
		" %%lu %%lu %%lu %%lu %%lu %%lu %%*d %%*d %%u %%*u %%*d %%lu %%lu"
		" %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u"
		" %%*u %%u %%*u %%*u %%*u %%lu %%lu\\n", MAX_COMM_LEN);
/**
minflt %lu  (10)  The  number  of minor faults the process has made which
            have not required loading a memory page from disk.

cminflt %lu (11) The number of minor faults that the process's waited-for
            children have made.

majflt %lu  (12)  The  number  of major faults the process has made which
            have required loading a memory page from disk.

cmajflt %lu (13) The number of major faults that the process's waited-for
            children have made.

utime %lu   (14)  Amount  of time that this process has been scheduled in
            user   mode,   measured   in   clock   ticks    (divide    by
            sysconf(_SC_CLK_TCK)).   This includes guest time, guest_time
            (time spent running a virtual CPU, see below), so that appli‐
            cations  that  are  not  aware of the guest time field do not
            lose that time from their calculations.

stime %lu   (15) Amount of time that this process has been  scheduled  in
            kernel   mode,   measured   in   clock   ticks   (divide   by
            sysconf(_SC_CLK_TCK)).

cutime %ld  (16) Amount of time that this process's  waited-for  children
            have  been  scheduled  in  user mode, measured in clock ticks
            (divide by sysconf(_SC_CLK_TCK)).  (See also times(2).)  This
            includes  guest  time, cguest_time (time spent running a vir‐
            tual CPU, see below).

cstime %ld  (17) Amount of time that this process's  waited-for  children
            have  been  scheduled in kernel mode, measured in clock ticks
            (divide by sysconf(_SC_CLK_TCK)).

processor %d (since Linux 2.2.8)
            (39) CPU number last executed on.

guest_time %lu (since Linux 2.6.24)
            (43)  Guest time of the process (time spent running a virtual
            CPU for a guest operating system), measured  in  clock  ticks
            (divide by sysconf(_SC_CLK_TCK)).

cguest_time %ld (since Linux 2.6.24)
            (44)  Guest time of the process's children, measured in clock
            ticks (divide by sysconf(_SC_CLK_TCK)).	
*/

	fscanf(fp1, format1, comm,
	       &pst.minflt, &pst.cminflt, &pst.majflt, &pst.cmajflt,
	       &pst.utime,  &pst.stime, &pst.cutime, &pst.cstime,
	       &thread_nr, &pst.vsz, &pst.rss, &pst.processor,
	       &pst.gtime, &pst.cgtime);

	st_samples[samples_count].utime = pst.utime;
	st_samples[samples_count].stime = pst.stime;

	
/**
size       (1) total program size
           (same as VmSize in /proc/[pid]/status)
resident   (2) resident set size
           (same as VmRSS in /proc/[pid]/status)
share      (3) shared pages (i.e., backed by a file)
text       (4) text (code)
lib        (5) library (unused in Linux 2.6)
data       (6) data + stack
dt         (7) dirty pages (unused in Linux 2.6)
*/

	sprintf(format2, "%%*lu %%*lu %%*lu %%*lu %%*lu %%lu %%*lu\\n");
	fscanf(fp2, format2, &pst.data_stack_size);

	strncpy(pst.comm, comm, MAX_COMM_LEN);
	pst.comm[MAX_COMM_LEN - 1] = '\0';

	/* Remove trailing ')' */
	len = strlen(pst.comm);
	if (len && (pst.comm[len - 1] == ')')) {
		pst.comm[len - 1] = '\0';
	}

/**
 IO Statistics

*/
	fseek(fp3, 0, SEEK_SET);
	while(fgets(&line[0], 256, fp3) != NULL) {
		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(&line[12], "%llu", &pst.read_bytes);
		}
		else if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(&line[13], "%llu", &pst.write_bytes);
		}
		/*else if (!strncmp(line, "cancelled_write_bytes:", 22)) {
			sscanf(&line[23], "%llu", &pst.cancelled_write_bytes);
		}*/
	}

	st_samples[samples_count].write_bytes = pst.write_bytes;
	st_samples[samples_count].read_bytes  = pst.read_bytes;

	if (samples_count > 0) {
		utime = pst.utime - st_samples[samples_count - 1].utime;
		stime = pst.stime - st_samples[samples_count - 1].stime;

		rbytes = pst.read_bytes  - st_samples[samples_count - 1].read_bytes;
		wbytes = pst.write_bytes - st_samples[samples_count - 1].write_bytes;
	} else {
		utime  = pst.utime;
		stime  = pst.stime;
		rbytes = pst.read_bytes;
		wbytes = pst.write_bytes;
	}

	pst.pid = child;

	printf("%lu %lu %lu %llu %llu \n", utime, stime, pst.data_stack_size, rbytes, wbytes);

	samples_count++;

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
}

