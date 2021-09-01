#include <stdio.h>
#include "conveyers.h"
#include "tools.h"
#include "common.h"
#include "log.h"
#include "control.h"

char **loaded_modules;
unsigned int loaded_modules_num;
time_t start_time;

void Root_Event_Handler(JID *jid)
{
	int sock;
	ssize_t size;
	ssize_t end_index;
	char *buff;
	char *oldbuff;

	if(!jid) return;
	if(jid->_sockfd) sock = jid->_sockfd;
	else if(jid->_parent && jid->_parent->_sockfd) sock = jid->_parent->_sockfd;
	else {
		myerror("No socket");
		return;
	}

	for(;;) {
		buff = R(&size, 10);
		oldbuff = buff;
		if(!buff) continue;
		while(size)
			if(XML_Search(buff, size, &end_index, "message", 0)) {
				XMPP_Msg *msg;
				ssize_t old_end = end_index;
				XML_Search_Closer(buff+old_end, size-old_end, &end_index, "message");
				end_index+=old_end;
				msg = Construct_XMPP_Msg(buff, end_index);
				if(msg) {
					if(alien_msg(jid, msg)) Conveyer_MSG_EVENT(0, jid, msg);
					else Destruct_XMPP_Msg(msg);
				}
				size -= end_index;
				buff += end_index;
			} else if(XML_Search(buff, size, &end_index, "iq", 0)) {
				XMPP_IQ *iq;
				ssize_t old_end = end_index;
				XML_Search_Closer(buff+old_end, size-old_end, &end_index, "iq");
				end_index+=old_end;
				iq = Construct_XMPP_IQ(buff, end_index);
				if(iq) {
					if(alien_iq(jid, iq)) Conveyer_IQ_EVENT(0, jid, iq);
					else Destruct_XMPP_IQ(iq);
				}
				size -= end_index;
				buff += end_index;
			} else if(XML_Search(buff, size, &end_index, "presence", 0)) {
				XMPP_Presence *p;
				ssize_t old_end = end_index;
				XML_Search_Closer(buff+old_end, size-old_end, &end_index, "presence");
				end_index+=old_end;
				p = Construct_XMPP_Presence(buff, end_index);
				if(p) {
					if(alien_presence(jid, p)) Conveyer_PRESENCE_EVENT(0, jid, p);
					else Destruct_XMPP_Presence(p);
				}
				size -= end_index;
				buff += end_index;
			} else break;
		/* Other events here... */
		free(oldbuff);
	}
}

void Register_Handler(char event_type, event_hdl hdl)
{
	switch(event_type) {
	case NULL_EVENT:
		return;
	case MSG_EVENT:
		Conveyer_MSG_EVENT(hdl, 0, 0);
		break;
	case IQ_EVENT:
		Conveyer_IQ_EVENT(hdl, 0, 0);
		break;
	case PRESENCE_EVENT:
		Conveyer_PRESENCE_EVENT(hdl, 0, 0);
		break;
	}
}

void Conveyer_MSG_EVENT(event_hdl hdl, JID *jid, XMPP_Msg *msg)
{
	static event_hdl *vhdl;
	static ssize_t count = 0;

	if(!hdl && !jid) return;
	if(hdl) {
		count++;
		vhdl = (event_hdl *)realloc(vhdl, count * sizeof(event_hdl));
		vhdl[count-1] = hdl;
	} else {
		ssize_t i;
		Log_Event(MSG_EVENT, msg, jid);
		if(msg && msg->body && !msg->stamp && !strcmp("!xmbot", msg->body)) {
			char *stat_screen;
			char modnum[10];
			time_t now = time(0);
			time_t total_days, total_hours, total_minutes, total_seconds;
			time_t days, hours, minutes, seconds;
			char c_days[10], c_hours[3], c_minutes[3], c_seconds[3];
			unsigned int n;

			total_seconds = now-start_time;
			seconds = total_seconds%60;
			total_minutes = (total_seconds-seconds)/60;
			minutes = total_minutes%60;
			total_hours = (total_minutes-minutes)/60;
			hours = total_hours%24;
			days = (total_hours-hours)/24;

			snprintf(modnum, 10, "%i", loaded_modules_num);
			snprintf(c_days, 10, "%i", days);
			snprintf(c_hours, 3, "%i", hours);
			snprintf(c_minutes, 3, "%i", minutes);
			snprintf(c_seconds, 3, "%i", seconds);

			stat_screen = SCollect(14, "\n[", VERSION,
				"]\nUptime: ", c_days, " days ", c_hours,
				" hours ", c_minutes, " minutes ", c_seconds,
				" seconds", "\n", modnum, " modules loaded");

			for(n = 0; n < loaded_modules_num; n++) {
				char *tmp = loaded_modules[n];
				ssize_t len = strlen(tmp);
				tmp += len;
				while(len-- && *(--tmp) != '/');
				tmp++;
				SAppend(3, &stat_screen, "\n", tmp);
			}

			XMPP_Send_Msg(jid, msg->from, msg->type, stat_screen, 0);
			free(stat_screen);
		}
		
		for(i=0; i < count; i++)
			vhdl[i](jid, msg);
		if(count && msg) Destruct_XMPP_Msg(msg);
	}
}

void Conveyer_IQ_EVENT(event_hdl hdl, JID *jid, XMPP_IQ *iq)
{
	static event_hdl *vhdl;
	static ssize_t count = 0;

	if(!hdl && !jid) return;
	if(hdl) {
		count++;
		vhdl = (event_hdl *)realloc(vhdl, count * sizeof(event_hdl));
		vhdl[count-1] = hdl;
	} else {
		ssize_t i;

		if(iq && iq->type && iq->iqns && iq->id && iq->from && !strcmp(iq->type, "get") && !strcmp(iq->iqns, "version")) {
			XMPP_Send_IQ_Version_Result(jid, iq->from, iq->id, CLIENT_NAME, CLIENT_VERSION, CLIENT_OS);
		}
		
		for(i=0; i < count; i++)
			vhdl[i](jid, iq);
		if(count && iq) Destruct_XMPP_IQ(iq);
	}
}

void Conveyer_PRESENCE_EVENT(event_hdl hdl, JID *jid, XMPP_Presence *p)
{
	static event_hdl *vhdl;
	static ssize_t count = 0;

	if(!hdl && !jid) return;
	if(hdl) {
		count++;
		vhdl = (event_hdl *)realloc(vhdl, count * sizeof(event_hdl));
		vhdl[count-1] = hdl;
	} else {
		ssize_t i;

		for(i=0; i < count; i++)
			vhdl[i](jid, p);
		if(count && p) Destruct_XMPP_Presence(p);
	}
}
