#include "../modlib.h"

void msg_hdl(JID *jid, XMPP_Msg *msg)
{
	if(msg->from && msg->type && !msg->stamp && msg->body) {
		if(!strncmp(msg->body, "!say", 4) && strlen(msg->body) > 5 && isspace(msg->body[4])) {
			char *rest = msg->body + 5;
			XMPP_Send_Msg(jid, msg->from, "groupchat", rest, 0);
		}
	}
}

void iq_hdl(JID *jid, XMPP_IQ *iq)
{
	if(iq->from && iq->iqns && iq->query) {
	}
}

Register_Module(OptVal *optval, ssize_t pairs)
{
	Register_Handler(MSG_EVENT, (event_hdl)msg_hdl);
//	Register_Handler(IQ_EVENT, (event_hdl)iq_hdl);
}
