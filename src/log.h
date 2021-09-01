/*** Log events ***/
#ifndef LOG_H
#define LOG_H

#include "config.h"
#include "types.h"

/* Write event into file */
void Log_Event(char event_type, void *data, JID *jid); 
/* Set log configuration */
void Init_Log(Config *cfg);

#endif
