#include "../modlib.h"
#include <string.h>
#include <stdio.h>
#include "search.h"
#include "iq.h"

extern Sulci_Query q_ping, q_version, q_time;

char mode_sulci;

void msg_hdl(JID *jid, XMPP_Msg *msg)
{
	if(msg->from && msg->type && !msg->stamp && msg->body &&
		( mode_sulci || (!strncmp(msg->body, "!sulci", 6) && strlen(msg->body) > 7 && isspace(msg->body[6]))) ) {
		char *body = msg->body;
		if(!strcmp(msg->body, "!sulci on")) {
			mode_sulci = 1;
			XMPP_Send_Msg(jid, msg->from, msg->type, "/me [SULCI MODE ON]", 0);
		} else if(!strcmp(msg->body, "!sulci off")) {
			mode_sulci = 0;
			XMPP_Send_Msg(jid, msg->from, msg->type, "/me [SULCI MODE OFF]", 0);
		}
		if(!mode_sulci || !strncmp(msg->body, "!sulci", 6)) body += 7;

		if(!strncmp(body, "ping", 4)) Sulci_Ping_Get(jid, msg);
		else if(!strncmp(body, "version", 7)) Sulci_Version_Get(jid, msg);
		else if(!strncmp(body, "time", 4)) Sulci_Time_Get(jid, msg);
		else if(!strncmp(body, "google", 6)) Sulci_Google_Print(jid, msg);
	}
}

void iq_hdl(JID *jid, XMPP_IQ *iq)
{
	if(iq->from && iq->iqns && iq->query) {
		if(!strcmp(iq->iqns, "version")) {
			unsigned int c;
			c = q_ping.num;
			while(c-- > 0)
				if(q_ping.id[c] && !strcmp(iq->id, q_ping.id[c])) {
					Sulci_Ping_Print(c, jid, iq);
					return;
				}
			c = q_version.num;
			while(c-- > 0)
				if(q_version.id[c] && !strcmp(iq->id, q_version.id[c])) {
					Sulci_Version_Print(c, jid, iq);
					return;
				}
		} else if(!strcmp(iq->iqns, "time")) {
			unsigned int c;
			c = q_time.num;
			while(c-- > 0)
				if(q_time.id[c] && !strcmp(iq->id, q_time.id[c])) Sulci_Time_Print(c, jid, iq);
		}
	}
}

Register_Module(OptVal *optval, ssize_t pairs)
{
	S_Q_Init(&q_ping);
	S_Q_Init(&q_version);
	S_Q_Init(&q_time);
	Register_Handler(MSG_EVENT, (event_hdl)msg_hdl);
	Register_Handler(IQ_EVENT, (event_hdl)iq_hdl);
}
