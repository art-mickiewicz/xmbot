/*** Types ***/
#ifndef TYPES_H
#define TYPES_H

/* Structure: MUC Privileges */
typedef struct {
	char *role;
	char *affiliation;
	char *jid;
	char *nick;
} MUC_Priv;
/* Structure: JabberID node@domain/resource */
struct _JID {
	char *node;
	char *domain;
	char *resource;
	int _sockfd;
	struct _JID *_parent;
	struct _JID *_child;
	MUC_Priv *_muc;
};
typedef struct _JID JID;
/* Structure: Timestampr
   XMPP timestamp looks like stamp="20060302T14:34:53"*/
typedef struct {
	char *year;
	char *month;
	char *day;
	char *hour;
	char *minute;
	char *second;
} Timestamp;
/* Structure: XMPP Message
   Needs destructor...
   "to" and "body" - could not be NULL*/
typedef struct {
	char *from;
	char *to;
	char *type;
	char *id;
	char *body;
	Timestamp *stamp;
} XMPP_Msg;

/* Structure: XMPP IQ
   "from", "to", "type", "id" is like in XMPP_Msg
   "iqns" is for example "version" from this tag <query xmlns="jabber:iq:version">
   "query" is some structure which type depends on "iqns"*/
typedef struct {
	char *from;
	char *to;
	char *type;
	char *id;
	char *iqns;
	void *query;
} XMPP_IQ;

typedef struct {
	char *name;
	char *version;
	char *os;
} XMPP_IQ_version;

typedef struct {
	Timestamp *utc;
	char *tz;
	char *display;
} XMPP_IQ_time;

typedef struct {
	MUC_Priv *muc;
	char *reason;
} XMPP_IQ_admin;

/* Structure: XMPP IQ
   "from", "to", is like in XMPP_Msg
   "type" is something like state. For example "unavaliable".
   "show" is something like state too. But it has more humanity purpose. */
typedef struct {
	char *from;
	char *to;
	char *type;
	int priority;
	char *show;
	MUC_Priv *muc;
} XMPP_Presence;

#endif
