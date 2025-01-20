/*
 *
 *  (c)2022-2025 Ira Parsons
 *  ergod - the computer ergonomics daemon
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <systemd/sd-daemon.h>

#ifdef USE_SYSLOG
#include <syslog.h>
#endif

char *CONFIGPATH = "/etc/ergod.conf";
char *CONFIGRINT = "REMINDERINT";
char *CONFIGRBIN = "REMINDERBIN";
char *CONFIGRMSG = "REMINDERMSG";

struct config_s;

struct config_s {
	int reminderinterval;
	char *reminderbinary;
	char *remindermessage;
};

struct config_s config;

void parseconfig(void); /* Parse the configuration file. */

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
		syslog(LOG_NOTICE, "Received SIGHUP and reloaded configuration file");
		#else
		fprintf(stderr, SD_NOTICE "Received SIGHUP and reloaded configuration file");
		#endif
		parseconfig();
		return;
		/* Reload the config files. */
	}
	
	return;

}

int main(int argc, char* argv[]){

	if(argc == 1){
	#if defined(USE_SYSLOG)
		openlog("ergod", 0, LOG_DAEMON);
		syslog(LOG_NOTICE, "Succesfully began execution");
	#else
		fprintf(stderr, SD_NOTICE "Succesfully began execution");
	#endif
	}

	register int c;
	extern char *optarg;
	extern int optind, optopt;

	while((c = getopt(argc, argv, "vh")) != -1){
		switch(c){
			case 'v':
				if( access("/bin/cowsay", F_OK) == 0 ){
					char* verargs[] = { "/bin/cowsay", "ergod - the computer ergonomics daemon v.0.2\nCopyright (c) 2022-2025 Ira Parsons", 0 };
					execv(verargs[0], verargs);
					break;
				}
				printf("ergod - the computer ergonomics daemon v.0.2\n");
				printf("Copyright (c) 2022-2025 Ira Parsons\n");
				exit(0);
				break;
			case 'h':
				if( access("/bin/cowsay", F_OK) == 0 ){
					char* helpargs[] = { "/bin/cowsay", "Usage: ergod [-vh]\n\nSee also: ergod(1)", 0 };
					execv(helpargs[0], helpargs);
					break;
				}
				printf("Usage: ergod [-vh]\nSee also: ergod(1)\n");
				exit(0);
				break;
			case '?':
				exit(0);
				break;
		}
	}

	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);

	config.reminderinterval = 1200;
	config.reminderbinary = "/bin/wall";
	config.remindermessage = "Time to look away from the screen for 20 seconds!";
	parseconfig();
	
	char* reminderargs[] = { config.reminderbinary, config.remindermessage, 0 };
	pid_t pid;

	for(;;){
		
		sleep(config.reminderinterval); /* Wait 20 minutes. */
		
		pid = vfork();
		
		if(pid < 0){
			/* Error forking. */
			#ifdef USE_SYSLOG
			syslog(LOG_PERROR, "Failed to fork");
			#else
			fprintf(stderr, SD_ERR "Failed to fork");
			#endif
		}	
		else if(pid > 0){
			int status;
			waitpid(pid, &status, 0);
		}
		else{ /* We are the child: */
			execv(reminderargs[0], reminderargs); /* Send a message to all users. */
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

void parseconfig(){
	
	if(access(CONFIGPATH, F_OK) != 0){ /* If the file doesn't exist. */
		#ifdef USE_SYSLOG
		syslog(LOG_INFO, "No configuration file found");
		#else
		fprintf(stderr, SD_INFO "No configuration file found");
		#endif
		return;
	}
	
	/* Otherwise: */
	
	FILE *configfile = fopen(CONFIGPATH, "r");
	rewind(configfile);
	
	#ifdef USE_SYSLOG
	syslog(LOG_INFO, "Opened configuration file");
	#else
	fprintf(stderr, SD_INFO "Opened configuration file");
	#endif

	char *workingentry = malloc(128);
	char *workingvalue = malloc(256);
	char *line = malloc(128+256);

	while(fgets(line, 128+256, configfile)){
		char *ptr = strchr(line, '=');
		if(ptr != NULL){
			*ptr++ = 0;
			strcpy(workingentry, line);
			strcpy(workingvalue, ptr);
			#ifdef USE_SYSLOG
			syslog(LOG_INFO, "Configuration option detected: %s=%s", workingentry, workingvalue);
			#else
			fprintf(stderr, SD_INFO "Configuration option detected: %s=%s", workingentry, workingvalue);
			#endif
			if(workingentry != NULL && !strcmp(workingentry, CONFIGRINT)){
				config.reminderinterval = atoi(workingvalue);
			};
			if(workingentry != NULL && !strcmp(workingentry, CONFIGRBIN)){
				config.reminderbinary = strdup(workingvalue);
			};
			if(workingentry != NULL && !strcmp(workingentry, CONFIGRMSG)){
				config.remindermessage = strdup(workingvalue);
			};
		};
	};

	free(workingentry);
	free(workingvalue);
	free(line);

	#ifdef USE_SYSLOG
	syslog(LOG_INFO, "Finished parsing configuration file");
	#else
	fprintf(stderr, SD_INFO "Finished parsing configuration file");
	#endif

	fclose(configfile);
	
}
