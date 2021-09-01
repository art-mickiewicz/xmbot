#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "iq.h"
#include "common.h"

Sulci_Query q_ping, q_version, q_time;

void Sulci_Ping_Get(JID *jid, XMPP_Msg *msg)
{
	char *body = Body(msg);
	struct timeval tv;
	if(!strcmp(body, "ping")) {
		S_Q_id(&q_ping, 0);
		gettimeofday(&tv, 0);
		q_ping.stamp[q_ping.num-1] = tv.tv_sec + (double)tv.tv_usec/1000000;
		XMPP_Send_IQ_Request(jid, msg->from, "version", q_ping.id[q_ping.num-1]);
		q_ping.type[q_ping.num-1] = SCollect(1, msg->type);
	} else if(strlen(body) > 5 && isspace(body[4])) {
			char *target = body + 5;
			char *to = 0;
			if(Check_If_Groupchat(jid, msg->from, 0)) {
				JID *tmpjid;
				tmpjid = Get_JID_From_String(msg->from);
				to = SCollect(5, tmpjid->node, "@", tmpjid->domain, "/", target);
				Destruct_JID(tmpjid);
				S_Q_id(&q_ping, 0);
				gettimeofday(&tv, 0);
				q_ping.stamp[q_ping.num-1] = tv.tv_sec + (double)tv.tv_usec/1000000;
				XMPP_Send_IQ_Request(jid, to, "version", q_ping.id[q_ping.num-1]);
				free(to);
				q_ping.sender[q_ping.num-1] = SCollect(1, msg->from);
				q_ping.type[q_ping.num-1] = SCollect(1, msg->type);
			}
	}
}

void Sulci_Ping_Print(unsigned int ind, JID *jid, XMPP_IQ *iq)
{
	struct timeval tv;
	double curtime;
	char restime[20];
	char *out;
	char *nick;

	gettimeofday(&tv, 0);
	curtime = tv.tv_sec + (double)tv.tv_usec/1000000;
	snprintf(restime, 20, "%.3f", curtime-q_ping.stamp[ind]);
	Check_If_Groupchat(jid, iq->from, &nick);
	if(q_ping.sender[ind]) {
		out = SCollect(5, "Понг от ", nick, ": ", restime, " секунды");
		XMPP_Send_Msg(jid, q_ping.sender[ind], q_ping.type[ind], out, 0);
	} else {
		out = SCollect(4, nick, ": Понг от тебя ", restime, " секунды");
		XMPP_Send_Msg(jid, iq->from, q_ping.type[ind], out, 0);
	}
	free(nick);
	free(out);
	S_Q_id(&q_ping, q_ping.id[ind]);
}

void Sulci_Version_Get(JID *jid, XMPP_Msg *msg)
{
	char *body = Body(msg);
	if(!strcmp(body, "version")) {
		S_Q_id(&q_version, 0);
		XMPP_Send_IQ_Request(jid, msg->from, "version", q_version.id[q_version.num-1]);
		q_version.type[q_version.num-1] = SCollect(1, msg->type);
	} else if(strlen(body) > 8 && isspace(body[7])) {
		char *target = body + 8;
		char *to = 0;
		if(Check_If_Groupchat(jid, msg->from, 0)) {
			JID *tmpjid;
			tmpjid = Get_JID_From_String(msg->from);
			to = SCollect(5, tmpjid->node, "@", tmpjid->domain, "/", target);
			Destruct_JID(tmpjid);
			S_Q_id(&q_version, 0);
			XMPP_Send_IQ_Request(jid, to, "version", q_version.id[q_version.num-1]);
			free(to);
			q_version.sender[q_version.num-1] = SCollect(1, msg->from);
			q_version.type[q_version.num-1] = SCollect(1, msg->type);
		}
	}
}

void Sulci_Version_Print(unsigned int ind, JID *jid, XMPP_IQ *iq)
{
	char *out;
	char *nick;
	XMPP_IQ_version *ver;
	ver = (XMPP_IQ_version *)(iq->query);

	Check_If_Groupchat(jid, iq->from, &nick);
	if(q_version.sender[ind]) {
		out = SCollect(8, "У ", nick, " клиент ", ver->name, " ", ver->version, " - ", ver->os);
		XMPP_Send_Msg(jid, q_version.sender[ind], q_version.type[ind], out, 0);
	} else {
		out = SCollect(7, nick, ": У тебя клиент ", ver->name, " ", ver->version, " - ", ver->os);
		XMPP_Send_Msg(jid, iq->from, q_version.type[ind], out, 0);
	}
	free(nick);
	free(out);
	S_Q_id(&q_version, q_version.id[ind]);
}

void Sulci_Time_Get(JID *jid, XMPP_Msg *msg)
{
	char *body = Body(msg);
	if(!strcmp(body, "time")) {
		S_Q_id(&q_time, 0);
		XMPP_Send_IQ_Request(jid, msg->from, "time", q_time.id[q_time.num-1]);
		q_time.type[q_time.num-1] = SCollect(1, msg->type);
	} else if(strlen(body) > 5 && isspace(body[4])) {
		char *target = body + 5;
		char *to = 0;
		if(Check_If_Groupchat(jid, msg->from, 0)) {
			JID *tmpjid;
			tmpjid = Get_JID_From_String(msg->from);
			to = SCollect(5, tmpjid->node, "@", tmpjid->domain, "/", target);
			Destruct_JID(tmpjid);
			S_Q_id(&q_time, 0);
			XMPP_Send_IQ_Request(jid, to, "time", q_time.id[q_time.num-1]);
			free(to);
			q_time.sender[q_time.num-1] = SCollect(1, msg->from);
			q_time.type[q_time.num-1] = SCollect(1, msg->type);
		}
	}
}

void Sulci_Time_Print(unsigned int ind, JID *jid, XMPP_IQ *iq)
{
	char *out;
	char *nick;

	XMPP_IQ_time *tme;
	tme = (XMPP_IQ_time *)(iq->query);
	Check_If_Groupchat(jid, iq->from, &nick);
	if(q_time.sender[ind]) {
		out = SCollect(4, "У ", &nick, " в компе показываются ", tme->display);
		XMPP_Send_Msg(jid, q_time.sender[ind], q_time.type[ind], out, 0);
	} else {
		out = SCollect(3, nick, ": У тебя в компе показываются ", tme->display);
		XMPP_Send_Msg(jid, iq->from, q_time.type[ind], out, 0);
	}
	free(out);
	S_Q_id(&q_time, q_time.id[ind]);
}

/* Sulci_Query Processing */

void S_Q_Init(Sulci_Query *q)
{
	q->num = 0;
	q->id = 0;
	q->sender = 0;
	q->type = 0;
	q->stamp = 0;
}

/* If id is not null - corresponding id elements destroyed */
void S_Q_id(Sulci_Query *q, char *id)
{
	if(!id) {
		q->num++;
		q->id = realloc(q->id, sizeof(char *)*q->num);
		q->sender = realloc(q->sender, sizeof(char *)*q->num);
		q->type = realloc(q->type, sizeof(char *)*q->num);
		q->stamp = realloc(q->stamp, sizeof(double)*q->num);
		q->id[q->num-1] = Gen_Id(0);
		q->sender[q->num-1] = 0;
		q->type[q->num-1] = 0;
		q->stamp[q->num-1] = 0;
	} else if(q->num) {
		unsigned int i;
		for(i=0; i < q->num; i++)
			if(!strcmp(q->id[i], id)) {
				unsigned int j;
				if(q->id[i]) free(q->id[i]);
				if(q->sender[i]) free(q->sender[i]);
				if(q->type[i]) free(q->type[i]);
				for(j = i; j < q->num-1; j++) {
					q->id[j] = q->id[j+1];
					q->sender[j] = q->sender[j+1];
					q->type[j] = q->type[j+1];
					q->stamp[j] = q->stamp[j+1];
				}
				q->num--;
				q->id = realloc(q->id, sizeof(char *)*q->num);
				q->sender = realloc(q->sender, sizeof(char *)*q->num);
				q->type = realloc(q->type, sizeof(char *)*q->num);
				q->stamp = realloc(q->stamp, sizeof(double)*q->num);

				return;
			}
	}
}
