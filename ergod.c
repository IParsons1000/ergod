/*
 *
 *  (c)2022 Ira Parsons
 *  ergod - the computer ergonomics daemon
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <systemd/sd-daemon.h>

#ifdef USE_SYSLOG
#include <syslog.h>
#endif

#define WAIT_TIME 1200
#define OUTPUT_BIN "/bin/wall"
#define OUTPUT_MSG "Time to look at something distant for 20 seconds!"

void signal_handler(int signo){
	
	if(signo == SIGTERM){
		#ifdef USE_SYSLOG
		syslog(LOG_NOTICE, "Received SIGTERM and terminated");
		closelog();
		#else
		fprintf(stderr, SD_NOTICE "Received SIGTERM and terminated");
		#endif
		exit(EXIT_SUCCESS);
	}
	if(signo == SIGHUP){
		#ifdef USE_SYSLOG
		syslog(LOG_NOTICE, "Received SIGHUP");
		#else
		fprintf(stderr, SD_NOTICE "Received SIGHUP");
		#endif
		sleep(WAIT_TIME);
		return;
		/* Reload the config files. */
	}
	
	return;

}

int main(int argc, char* argv[]){

	#ifdef USE_SYSLOG
	if(argc == 1){
		openlog("ergod", 0, LOG_DAEMON);
		syslog(LOG_NOTICE, "Succesfully began execution");
	}
	#else
	if(argc == 1){
		fprintf(stderr, SD_NOTICE "Succesfully began execution");
	}
	#endif

	int c;
	extern char *optarg;
	extern int optind, optopt;

	while((c = getopt(argc, argv, "vh")) != -1){
		switch(c){
			case 'v':
				if( access("/bin/cowsay", F_OK) == 0 ){
					char* verargs[] = { "/bin/cowsay", "ergod - the computer ergonomics daemon v.0.1\nCopyright (c) 2022 Ira Parsons", 0 };
					execv(verargs[0], verargs);
					break;
				}
				printf("ergod - the computer ergonomics daemon v.0.1\n");
				printf("Copyright (c) 2022 Ira Parsons\n");
				exit(0);
				break;
			case 'h':
				if( access("/bin/cowsay", F_OK) == 0 ){
					char* helpargs[] = { "/bin/cowsay", "Send bug reports to: <irap@cox.net>", 0 };
					execv(helpargs[0], helpargs);
					break;
				}
				printf("Send bug reports to: <irap@cox.net>\n");
				exit(0);
				break;
			case '?':
				exit(0);
				break;
		}
	}

	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	
	char* message[] = { OUTPUT_BIN, OUTPUT_MSG, 0 };
	pid_t pid;

	while(1){
		
		sleep(WAIT_TIME); /* Wait 20 minutes. */
		
		pid = vfork();
		
		if(pid < 0){
			/* Error forking. */
			#ifdef USE_SYSLOG
			syslog(LOG_ERROR, "Failed to fork");
			#else
			fprintf(stderr, SD_ERR "Failed to fork");
			#endif
		}	
		else if(pid > 0){
			int status;
			waitpid(pid, &status, 0);
		}
		else{ /* We are the child: */
			execv(message[0], message); /* Send a message to all users. */
			#ifdef USE_SYSLOG
			syslog(LOG_INFO, "Succesfully notified user");
			#else
			fprintf(stderr, SD_INFO "Succesfully notified user");
			#endif
		};
		
	};

	#ifdef USE_SYSLOG
	closelog();
	#endif
	
	return EXIT_SUCCESS;
	
}
