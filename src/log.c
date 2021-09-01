#include <stdio.h>
#include "log.h"
#include "conveyers.h"
#include "control.h"

Logfile *logfile;
ssize_t logquan;

void Log_Event(char event_type, void *data, JID *jid)
{
	FILE *file;
	ssize_t i;

	if(!logfile || !logquan) return;

	switch(event_type) {
	case MSG_EVENT:
		for(i=0; i < logquan; i++) {
			if( !strcmp(logfile[i].type, "msg") && !((XMPP_Msg *)data)->stamp && ((XMPP_Msg *)data)->from && ((XMPP_Msg *)data)->body && Check_JID_Mask(logfile[i].jid, ((XMPP_Msg *)data)->from) ) {
				char *filename = Resolve_Time(logfile[i].file);
				char *prefix;
				char *entry;
				char *output;
				ssize_t cnt;
				ssize_t len;
				if( ! (file = fopen(filename, "a"))) {
					perror("ERROR");
					free(filename);
					continue;
				}
				free(filename);
				prefix = 0;
				if(logfile[i].prefix) {
					char *tmp_prefix = 0;
					tmp_prefix = Resolve_Time(logfile[i].prefix);
					prefix = Resolve_RoomNick(tmp_prefix, jid, ((XMPP_Msg *)data)->from);
					free(tmp_prefix);
				}
				entry = fromUTF8( ((XMPP_Msg *)data)->body );
				output = 0;
				if(prefix) {
					output = SCollect(2, prefix, entry);
					free(prefix);
				} else output = entry;
				cnt = 0;
				len = strlen(output);
				while( cnt = fwrite( output, 1, len-cnt, file) ) {
					fseek(file, cnt, SEEK_CUR);
				}
				fseek(file, 1, SEEK_CUR);
				fwrite( "\n", 1, 1, file);
				fclose(file);
				free(output);
			}	
		}
		break;
	}
	
}

void Init_Log(Config *cfg)
{
	if(!cfg || !cfg->common || !cfg->common->logfile) return;
	else {
		logfile = cfg->common->logfile;
		logquan = cfg->common->logfiles;
	}
}
