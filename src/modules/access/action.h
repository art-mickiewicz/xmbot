#ifndef ACCESS_ACTION_H
#define ACCESS_ACTION_H

#include "../modlib.h"

void Do_Devoice(JID *jid, char *room_jid, char *target_nick);
void Do_Kick(JID *jid, char *room_jid, char *target_nick);
void Do_Ban(JID *jid, char *room_jid, char *target_jid);
void Do_Voice(JID *jid, char *room_jid, char *target_nick);
void Do_None(JID *jid, char *room_jid, char *target_jid);
void Do_Moderator(JID *jid, char *room_jid, char *target_nick);
void Do_Admin(JID *jid, char *room_jid, char *target_jid);
void Do_Owner(JID *jid, char *room_jid, char *target_jid);
void Do_Participant(JID *jid, char *room_jid, char *target_nick);

#endif
