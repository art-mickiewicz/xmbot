#include "action.h"

void Do_Devoice(JID *jid, char *room_jid, char *target_nick)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, target_nick, 0, "visitor", 0, 0);
}

void Do_Kick(JID *jid, char *room_jid, char *target_nick)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, target_nick, 0, "none", 0, 0);
}

void Do_Ban(JID *jid, char *room_jid, char *target_jid)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, 0, target_jid, 0, "outcast", 0);
}

void Do_Voice(JID *jid, char *room_jid, char *target_nick)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, target_nick, 0, "participant", 0, 0);
}

void Do_None(JID *jid, char *room_jid, char *target_jid)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, 0, target_jid, 0, "none", 0);
}

void Do_Moderator(JID *jid, char *room_jid, char *target_nick)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, target_nick, 0, "moderator", 0, 0);
}

void Do_Admin(JID *jid, char *room_jid, char *target_jid)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, 0, target_jid, 0, "admin", 0);
}

void Do_Owner(JID *jid, char *room_jid, char *target_jid)
{
	XMPP_Send_IQ_Admin(jid, room_jid, 0, 0, target_jid, 0, "owner", 0);
}

/* Eq Do_Voice */
void Do_Participant(JID *jid, char *room_jid, char *target_nick)
{
	Do_Voice(jid, room_jid, target_nick);
}
