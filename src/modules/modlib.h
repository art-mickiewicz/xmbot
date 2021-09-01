#ifndef MODLIB_H
#define MODLIB_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*** Types ***/
#ifndef TYPES_H
#define TYPES_H

/** Full config file structure **/
typedef struct {
	char *name;
	char *value;
} OptVal;

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

/*** All event handlers ***/
#ifndef CONVEYERS_H
#define CONVEYERS_H

/* Event types */
#define NULL_EVENT 0
#define MSG_EVENT 1
#define IQ_EVENT 2
#define PRESENCE_EVENT 3

typedef (*event_hdl)(JID *, void *);

/* Modules registers own handlers thru this function */
void Register_Handler(char event_type, event_hdl hdl);

#endif

/*** Some tools... ***/
#ifndef TOOLS_H
#define TOOLS_H

#define T(m) Transmit(sock, m, strlen(m)+1)
#define R(s, t) Receive(sock, s, t)

/* Transmits all data from msg */
char Transmit(int sock, char *msg, ssize_t size); 
/* Receives all data to (char *) */
char *Receive(int sock, ssize_t *size, int timeout);

/* Generate ID for XMPP tag */
char *Gen_Id(int seed);

/* Convert text to utf8 from encoding, specified by
   global variable "src_encoding" and back if in_utf is 1*/
char *UTF8_Convert(char *in, char in_utf);
/* Back convert */
#define UTF8(A) UTF8_Convert(A, 0)
#define fromUTF8(A) UTF8_Convert(A, 1)

/* Allocate memory and collect "num" strings into one */
char *SCollect(unsigned char num, ...);
/* Second arg is target */
void SAppend(unsigned char num, ...);

/* Allocate memory and return substring from start to end */
char *GetSubStr(char *in, ssize_t start, ssize_t end);

/* Main function: returns resolved string with subtituded
   time variables */
char *Resolve_Time(char *f);

/* Main function: returns resolved string with subtituded
   room and nick variables */
char *Resolve_RoomNick(char *prefix, JID *jid, char *from);

/* Check(by mask) if JID should to log.
   0 - if should not.
   else 1 */
char Check_JID_Mask(char *jidmask, char *jid);

char Check_If_Groupchat(JID *primjid, char *jidstr, char **nick);
/* Returns 1 if context is groupchat
   if nick not NULL - memory allocated and nick returned
   and MUST BE FREED */

JID *Get_JID_From_String(char *s);
in_addr_t Get_Addr(char *host);

/* Searches for "mark" in "buff" and returns
   pointer to place just after it */
char *Seek(char *buff, char *mark, char **start);
/* Remove all tag constructions and/or convert spec symbols */
char *Data_Filter(char *buff, char sw_fchar, char sw_ftag);
/* Making data apropriate for web query */
char *Make_W3_Query(char *buff);

#endif

/* End Of Modlib */
#endif
