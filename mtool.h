/**
 * A very easy tool to measure system resources
 *
 * Author: Renê S. Pinto
 */ 

#ifndef MTOOL_H

	#define MTOOL_H

	#define ALARM_TIME 1
	#define MAX_FNAME  256

	#define STAT_SAMPLES 30

	/** 
	 * struct from pidstat
	 * (C) 2007-2013 by Sebastien Godard (sysstat <at> orange.fr)
	 */
	#define MAX_COMM_LEN	128
	#define MAX_CMDLINE_LEN	128

	struct pid_stats {
		unsigned long long read_bytes			__attribute__ ((aligned (8)));
		unsigned long long write_bytes			__attribute__ ((packed));
		unsigned long long cancelled_write_bytes	__attribute__ ((packed));
		unsigned long long total_vsz			__attribute__ ((packed));
		unsigned long long total_rss			__attribute__ ((packed));
		unsigned long long total_stack_size		__attribute__ ((packed));
		unsigned long long total_stack_ref		__attribute__ ((packed));
		unsigned long long total_threads		__attribute__ ((packed));
		unsigned long long total_fd_nr			__attribute__ ((packed));
		unsigned long      minflt			__attribute__ ((packed));
		unsigned long      cminflt			__attribute__ ((packed));
		unsigned long      majflt			__attribute__ ((packed));
		unsigned long      cmajflt			__attribute__ ((packed));
		unsigned long      utime			__attribute__ ((packed));
		unsigned long      cutime			__attribute__ ((packed));
		unsigned long      stime			__attribute__ ((packed));
		unsigned long      cstime			__attribute__ ((packed));
		unsigned long      gtime			__attribute__ ((packed));
		unsigned long      cgtime			__attribute__ ((packed));
		unsigned long      vsz				__attribute__ ((packed));
		unsigned long      rss				__attribute__ ((packed));
		unsigned long      nvcsw			__attribute__ ((packed));
		unsigned long      nivcsw			__attribute__ ((packed));
		unsigned long      stack_size			__attribute__ ((packed));
		unsigned long      stack_ref			__attribute__ ((packed));
		/* If pid is null, the process has terminated */
		unsigned int       pid				__attribute__ ((packed));
		/* If tgid is not null, then this PID is in fact a TID */
		unsigned int       tgid				__attribute__ ((packed));
		unsigned int       rt_asum_count		__attribute__ ((packed));
		unsigned int       rc_asum_count		__attribute__ ((packed));
		unsigned int       uc_asum_count		__attribute__ ((packed));
		unsigned int       tf_asum_count		__attribute__ ((packed));
		unsigned int       sk_asum_count		__attribute__ ((packed));
		unsigned int       processor			__attribute__ ((packed));
		unsigned int       flags			__attribute__ ((packed));
		unsigned int       uid				__attribute__ ((packed));
		unsigned int       threads			__attribute__ ((packed));
		unsigned int       fd_nr			__attribute__ ((packed));

		unsigned long      data_stack_size	__attribute__ ((packed));
		char               comm[MAX_COMM_LEN];
		char               cmdline[MAX_CMDLINE_LEN];
	};

#endif

