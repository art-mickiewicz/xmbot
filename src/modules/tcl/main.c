#include "../modlib.h"

void msg_hdl(JID *jid, XMPP_Msg *msg)
{
	if(msg->from && msg->type && !msg->stamp && msg->body) {
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
	Register_Handler(IQ_EVENT, (event_hdl)iq_hdl);
}
