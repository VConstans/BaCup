#ifndef _ARGUMENT_H_
#define _ARGUMENT_H_


struct argument
{
	char* destination;
	char* source;
	pthread_mutex_t mut_scanner;
	pthread_cond_t cond_scanner;
	pthread_mutex_t mut_analyser;
	pthread_cond_t cond_analyser;

};

#endif
