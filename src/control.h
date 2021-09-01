/*** Functions working around XMPP stream ***/
#ifndef CONTROL_H
#define CONTROL_H

#include <netinet/in.h>
#include "tools.h"
#include "types.h"

#define T(m) Transmit(sock, m, strlen(m)+1)
#define R(s, t) Receive(sock, s, t)

/** Protocol-specific constants **/
#define XML_HEADER "<?xml version='1.0'?>\n\n"

#define XMPP_STREAM_INIT_BEGIN "<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' to='"
#define XMPP_STREAM_INIT_END_HOST_ "' version='1.0'>\n\n"

#define XMPP_IQ_GET_AUTH_BEGIN "<iq type='get' id='auth_1' to='"
#define XMPP_IQ_GET_AUTH_END_HOST "'><query xmlns='jabber:iq:auth'><username>"
#define XMPP_IQ_GET_AUTH_END_USER_ "</username></query></iq>\n\n"

#define XMPP_IQ_SET_AUTH_BEGIN "<iq type='set' id='auth_2' to='"
#define XMPP_IQ_SET_AUTH_END_HOST "'><query xmlns='jabber:iq:auth'><username>"
#define XMPP_IQ_SET_AUTH_END_USER "</username><digest>"
#define XMPP_IQ_SET_AUTH_END_DIGEST_ "</digest><resource>xmbot</resource></query></iq>\n\n"

#define XMPP_IQ_GET_ROSTER_BEGIN "<iq type='get' id='"
#define XMPP_IQ_GET_ROSTER_END_ID_ "'><query xmlns='jabber:iq:roster'/></iq>\n\n"

#define XMPP_PRESENCE_PRIORITY "<presence><priority>5</priority></presence>\n\n"
#define XMPP_PRESENCE_LOGOUT "<presence type='unavailable'><status>Logged out</status></presence>\n\n"

#define XMPP_PRESENCE_PRIORITY_BEGIN "<presence to='"
#define XMPP_PRESENCE_PRIORITY_END_TO "'><priority>"
#define XMPP_PRESENCE_PRIORITY_END_PRIORITY_ "</priority></presence>\n\n"

#define XMPP_MESSAGE_BEGIN "<message type='"
#define XMPP_MESSAGE_END_TYPE "' id='"
#define XMPP_MESSAGE_END_ID "' to='"
#define XMPP_MESSAGE_END_TO "'><body>"
#define XMPP_MESSAGE_END_BODY_ "</body></message>\n\n"

#define XMPP_IQ_BEGIN "<iq to='"
#define XMPP_IQ_END_TO "' from='"
#define XMPP_IQ_END_FROM "' id='"
#define XMPP_IQ_END_ID "' type='"
#define XMPP_IQ_END_TYPE "'><query xmlns='"
#define XMPP_IQ_END_XMLNS "'>"
#define XMPP_IQ_END_XMLNS_ "'/>"

#define XMPP_IQ_END_QUERY "</query>"
#define XMPP_IQ_END_ "</iq>\n\n"

#define XMPP_IQ_VERSION_BEGIN "<name>"
#define XMPP_IQ_VERSION_END_NAME "</name><version>"
#define XMPP_IQ_VERSION_END_VERSION "</version><os>"
#define XMPP_IQ_VERSION_END_OS_ "</os>"

#define XMPP_ITEM_BEGIN "<item"
#define XMPP_ITEM_END "</item>"

/** Connection control **/
/* The following functions returns not zero as OK */
JID *XMPP_Connect(char *host, in_port_t port, char *hostname, char *username, char *password);
/* Then we need to join some chatroom. Password could be NULL. */
JID *XMPP_MUC_Connect(JID *primjid, char *room, char *domain, char *nick, char *password);
/* Disconnection with MUC JID means in fact leaving chatroom
   Memory of JID will be freed ! */
void XMPP_Disconnect(JID *jid);

/** Chating **/
/* Types: "groupchat", "chat" */
/* To say something you should... */
/* If "recode_flag" is set, message converts to UTF8 */
void XMPP_Send_Msg(JID *jid, char *to, char *type, char *message, char recode_flag);

/** Query **/
void XMPP_Send_IQ_Request(JID *jid, char *to, char *type, char *id);
void XMPP_Send_IQ_Version_Result(JID *jid, char *to, char *id, char *name, char *version, char *os);
void XMPP_Send_IQ_Admin(JID *jid, char *to, char *id, char *nick, char *tjid, char *role, char *affiliation, char *reason);

/** Presence **/
void XMPP_Send_Presence();

/** Misc functions **/
void Destruct_JID(JID *jid); // If needs to destruct, but not disconnect

XMPP_Msg *Construct_XMPP_Msg(char *buff, ssize_t size);
void Destruct_XMPP_Msg(XMPP_Msg *msg);

Timestamp *Construct_Timestamp(char *stamp_str);
void Destruct_Timestamp(Timestamp *t);

XMPP_IQ *Construct_XMPP_IQ(char *buff, ssize_t size);
void Destruct_XMPP_IQ(XMPP_IQ *iq);

XMPP_IQ_version *Construct_XMPP_IQ_version(char *buff, ssize_t size);
void Destruct_XMPP_IQ_version(XMPP_IQ_version *ver);

XMPP_IQ_admin *Construct_XMPP_IQ_admin(char *buff, ssize_t size);
void Destruct_XMPP_IQ_admin(XMPP_IQ_admin *adm);

XMPP_IQ_time *Construct_XMPP_IQ_time(char *buff, ssize_t size);
void Destruct_XMPP_IQ_time(XMPP_IQ_time *t);

XMPP_Presence *Construct_XMPP_Presence(char *buff, ssize_t size);
void Destruct_XMPP_Presence(XMPP_Presence *p);

MUC_Priv *Construct_MUC_Priv(char *buff, ssize_t size);
void Destruct_MUC_Priv(MUC_Priv *mp);

#endif
