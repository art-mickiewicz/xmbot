#ifndef SULCI_IQ_H
#define SULCI_IQ_H

#include "../modlib.h"

/* Sulci Query Stack */
typedef struct {
	unsigned int num;
	char **id;
	char **sender;
	char **type;
	double *stamp;
} Sulci_Query;

void Sulci_Ping_Get(JID *jid, XMPP_Msg *msg);
void Sulci_Ping_Print(unsigned int ind, JID *jid, XMPP_IQ *iq);
void Sulci_Version_Get(JID *jid, XMPP_Msg *msg);
void Sulci_Version_Print(unsigned int ind, JID *jid, XMPP_IQ *iq);
void Sulci_Time_Get(JID *jid, XMPP_Msg *msg);
void Sulci_Time_Print(unsigned int ind, JID *jid, XMPP_IQ *iq);

void S_Q_Init(Sulci_Query *q);
void S_Q_id(Sulci_Query *q, char *id);

#endif
