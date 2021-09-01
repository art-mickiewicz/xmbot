#include "../modlib.h"
#include <unistd.h>
#include "parser.h"
#include "cmd.h"
#include "common.h"

Access_List *acl;
Users_Perm *uperm;
char *acl_filename;
char *acl_owner;
char ACL_Enabled = 1;

void msg_hdl(JID *jid, XMPP_Msg *msg)
{
	if(!jid->_child || !jid->_child->_muc) return;
	if(msg->from && msg->type && !msg->stamp && msg->body &&
		CCC("!acl", msg->body)) {
		char *body = msg->body + 5;
		signed char result = -1;
		if(!strcmp(body, "on")) result = ACmd_On(msg);
		else if(!strcmp(body, "off")) result = ACmd_Off(msg);
		else if(!strcmp(body, "read")) result = ACmd_Read(jid, msg);
		else if(!strcmp(body, "write")) result = ACmd_Write(jid, msg);
		else if(CCC("add", body)) result = ACmd_Add(jid, msg);
		else if(CCC("del", body)) result = ACmd_Del(jid, msg);
		else if(!strcmp(body, "print")) result = ACmd_Print(jid, msg);
		else if(CCC("do", body)) result = ACmd_Do(jid, msg);
		else if(CCC("apply", body)) result = ACmd_Apply(jid, msg);
		else if(!strcmp(body, "help")) ACmd_Help(jid, msg);

		if(!result) XMPP_Send_Msg(jid, msg->from, msg->type, "/me [ACCESS: OK]", 0);
		else if(result == 1) XMPP_Send_Msg(jid, msg->from, msg->type, "/me [ACCESS: FAILED]", 0);
		else if(result == 2) XMPP_Send_Msg(jid, msg->from, msg->type, "/me [ACCESS: PERMISSION DENIED]", 0);
	}
}

void iq_hdl(JID *jid, XMPP_IQ *iq)
{
	if(!jid->_child || !jid->_child->_muc) return;
	if(iq->from && iq->iqns && iq->query) {
		if(!strcmp(iq->iqns, "version")) {
			XMPP_IQ_version *ver = iq->query;
			Fill_Userstamp(iq->from, ver->name, ver->version, ver->os);
			Apply_On(jid, iq->from);
		}
	}
}

void p_hdl(JID *jid, XMPP_Presence *p)
{
	if(!p || !p->from || !jid->_child || !jid->_child->_muc) return;
	if(p->muc) {
		if(p->type && !strcmp(p->type, "unavailable")) Del_Permit_Item(p->from);
		else if(ACL_Enabled) {
			char *muc_nick = 0;
			char al_sw = 0;
			if(p->muc->nick) muc_nick = p->muc->nick;
			else {
				JID *tmpjid;
				tmpjid = Get_JID_From_String(p->from);
				muc_nick = SCollect(1, tmpjid->resource);
				Destruct_JID(tmpjid);
				al_sw = 1;
			}
			Fill_Userdata(p->from, p->muc->jid, muc_nick, p->muc->role, p->muc->affiliation);
			if(al_sw) free(muc_nick);
			Apply_On(jid, p->from);
			XMPP_Send_IQ_Request(jid, p->from, "version", 0);
			sleep(1); /* Maybe server kills flooders */
		}
		if(Check_JID_Mask(acl_owner, p->muc->jid) || !strcmp(p->from, acl_owner)) Permit(p->from, 1, 0);
	}
}

Register_Module(OptVal *optval, ssize_t pairs)
{
	if(pairs) {
		unsigned int i;
		for(i=0; i < pairs; i++) {
			if(!strcmp(optval[i].name, "acl")) acl_filename = optval[i].value;
			else if(!strcmp(optval[i].name, "owner")) acl_owner = optval[i].value;
		}
		if(acl_filename) acl = Read_ACL(acl_filename, 0);
		else return;
		Register_Handler(MSG_EVENT, (event_hdl)msg_hdl);
		Register_Handler(PRESENCE_EVENT, (event_hdl)p_hdl);
		Register_Handler(IQ_EVENT, (event_hdl)iq_hdl);
	}
}
