#ifndef ACCESS_CMD_H
#define ACCESS_CMD_H

#include "../modlib.h"

char ACmd_On(XMPP_Msg *msg);
char ACmd_Off(XMPP_Msg *msg);
char ACmd_Read(JID *jid, XMPP_Msg *msg);
char ACmd_Write(JID *jid, XMPP_Msg *msg);
char ACmd_Add(JID *jid, XMPP_Msg *msg);
char ACmd_Del(JID *jid, XMPP_Msg *msg);
char ACmd_Print(JID *jid, XMPP_Msg *msg);
void ACmd_Help(JID *jid, XMPP_Msg *msg);
char ACmd_Do(JID *jid, XMPP_Msg *msg);
char ACmd_Apply(JID *jid, XMPP_Msg *msg);

#endif
