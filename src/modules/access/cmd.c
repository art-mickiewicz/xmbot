#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "cmd.h"
#include "action.h"
#include "common.h"
#include "parser.h"

#define END_OF_FILE 0x0A

extern Access_List *acl;
extern Users_Perm *uperm;
extern char *acl_filename;
extern char ACL_Enabled;

char ACmd_On(XMPP_Msg *msg)
{
	if(Permit(msg->from, -1, 0)) {
		ACL_Enabled = 1;
		return 0;
	}
	return 2;
}

char ACmd_Off(XMPP_Msg *msg)
{
	if(Permit(msg->from, -1, 0)) {
		ACL_Enabled = 0;
		return 0;
	}
	return 2;
}

char ACmd_Read(JID *jid, XMPP_Msg *msg)
{
	char stat = 0;

	if(!Permit(msg->from, -1, 0)) return 2;
	Destruct_ACL();
	if(acl_filename) acl = Read_ACL(acl_filename, &stat);
	else return 1;

	return stat;
}

char ACmd_Write(JID *jid, XMPP_Msg *msg)
{
	int fd;
	char *buff;
	ssize_t len;
	ssize_t c;
	ssize_t i = 0;

	if(!Permit(msg->from, -1, 0)) return 2;
	if(!acl) return 1;

	if((fd = open(acl_filename, O_WRONLY|O_CREAT)) < 0) {
		perror("ERROR");
		return 1;
	}
	buff = ACL_To_Text(0);
	if(!buff) return 1;
	len = strlen(buff);
	buff[len] = END_OF_FILE;
	while(len >= 0 && (c = write(fd, buff+i, 1)) > 0) {
		len -= c;
		i += c;
		lseek(fd, i, SEEK_SET);
	}
	free(buff);

	return 0;
}

char ACmd_Add(JID *jid, XMPP_Msg *msg)
{
	if(!Permit(msg->from, -1, 0)) return 2;
	return 0;
}

char ACmd_Del(JID *jid, XMPP_Msg *msg)
{
	if(!Permit(msg->from, -1, 0)) return 2;
	return 0;
}

char ACmd_Print(JID *jid, XMPP_Msg *msg)
{
	char *out;

	if(!Permit(msg->from, -1, 0)) return 2;
	
	out = SCollect(1, "\nACTION|Nick|Jid|Client|Version|OS\n");
		
	if(!acl) SAppend(2, &out, "[EMPTY]");
	else {
		char *acltxt = ACL_To_Text(1);
		SAppend(2, &out, acltxt);
		free(acltxt);
	}

	XMPP_Send_Msg(jid, msg->from, msg->type, out, 0);
	free(out);

	return -1;
}

void ACmd_Help(JID *jid, XMPP_Msg *msg)
{
	char help_screen[] =
		"\nACL line format:\n"
		"ACTION|Nick|Jid|Client|Version|OS\n"
		"\nActions:\n"
		"DEVOICE KICK BAN\n"
		"VOICE MODERATOR\n"
		"NONE ADMIN OWNER PARTICIPANT\n"
		"\nCommands:\n"
		"!acl on                - Enable access control\n"
		"!acl off               - Disable access control\n"
		"!acl read              - Reread access file\n"
		"!acl write             - Write current acl into file\n"
		"!acl add [ACL LINE]    - Add rule\n"
		"!acl del [LINE]        - Remove rule by number or itself\n"
		"!acl print             - Print current acl\n"
		"!acl do [ACTN] [NICK]  - Perform action\n"
		"!acl apply [NICK]      - Pass user thru acl\n"
		"!acl help              - This screen";
	XMPP_Send_Msg(jid, msg->from, msg->type, help_screen, 0);
}

char ACmd_Do(JID *jid, XMPP_Msg *msg)
{
	char *body;
	char *room_jid;
	unsigned int num = 0;

	if(!Permit(msg->from, -1, 0)) return 2;

	body = msg->body + (5+3);
	room_jid = SCollect(3, jid->_child->node, "@", jid->_child->domain);
	if(CCC("devoice", body)) {
		body += 8;
		Do_Devoice(jid, room_jid, body);
	} else if(CCC("kick", body)) {
		body += 5;
		Do_Kick(jid, room_jid, body);
	} else if(CCC("ban", body)) {
		body += 4;
		Permit_by_Nick(jid, body, -1, &num); if(!num) return 1; num--;
		Do_Ban(jid, room_jid, uperm->stamp[num]->jid);
	} else if(CCC("voice", body)) {
		body += 6;
		Do_Voice(jid, room_jid, body);
	} else if(CCC("none", body)) {
		body += 5;
		Permit_by_Nick(jid, body, -1, &num); if(!num) return 1; num--;
		Do_None(jid, room_jid, uperm->stamp[num]->jid);
	} else if(CCC("moderator", body)) {
		body += 10;
		Do_Moderator(jid, room_jid, body);
	} else if(CCC("admin", body)) {
		body += 6;
		Permit_by_Nick(jid, body, -1, &num); if(!num) return 1; num--;
		Do_Admin(jid, room_jid, uperm->stamp[num]->jid);
	} else if(CCC("owner", body)) {
		body += 6;
		Permit_by_Nick(jid, body, -1, &num); if(!num) return 1; num--;
		Do_Owner(jid, room_jid, uperm->stamp[num]->jid);
	} else if(CCC("participant", body)) {
		body += 12;
		Do_Participant(jid, room_jid, body);
	} else {
		free(room_jid);
		return 1;
	}

	free(room_jid);
	return -1;
}

char ACmd_Apply(JID *jid, XMPP_Msg *msg)
{
	char *body;
	char *jidstr;

	if(!Permit(msg->from, -1, 0)) return 2;
	if(!jid || !jid->_child || !jid->_child->_muc) return 1;
	body = msg->body + (5+6);
	jidstr = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", body);	
	Apply_On(jid, jidstr);
	free(jidstr);
	return -1;
}
