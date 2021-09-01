#include "../modlib.h"
#include <stdlib.h>
#include "common.h"

extern Access_List *acl;
extern Users_Perm *uperm;
extern char *acl_owner;

char *ACL_To_Text(char sw_nums)
{
	char *out = 0;
	unsigned int i, j;
	char linenum[6];
	char *ACTIONS[] = { "", "DEVOICE", "KICK", "BAN", "VOICE", "NONE",
		"MODERATOR", "ADMIN", "OWNER", "PARTICIPANT" };

	for(i=0; i<acl->lines; i++) {
		if(i) SAppend(2, &out, "\n");
		if(sw_nums) {
			snprintf(linenum, 6, "%i", i);
			SAppend(3, &out, linenum, ":  ");
		}
		for(j=0; j < acl->act[i]->num; j++) {
			SAppend(2, &out, ACTIONS[acl->act[i]->action[j]]);
			if(acl->act[i]->next_at[j]) {
				snprintf(linenum, 6, "%i", acl->act[i]->next_at[j]);
				SAppend(2, &out, linenum);
			}
		}
		SAppend(2, &out, "|");
		if(acl->stamp[i]->nick) SAppend(2, &out, acl->stamp[i]->nick);
		SAppend(2, &out, "|");
		if(acl->stamp[i]->jid) SAppend(2, &out, acl->stamp[i]->jid);
		SAppend(2, &out, "|");
		if(acl->stamp[i]->client_name) SAppend(2, &out, acl->stamp[i]->client_name);
		SAppend(2, &out, "|");
		if(acl->stamp[i]->client_version) SAppend(2, &out, acl->stamp[i]->client_version);
		SAppend(2, &out, "|");
		if(acl->stamp[i]->client_os) SAppend(2, &out, acl->stamp[i]->client_os);
	}

	return out;
}

/* If jid not found - added
   0 - not permited
   1 - permited
   -1 - return current */
char Permit(char *jidstr, signed char permited, unsigned int *num)
{
	unsigned int i;
	if(!jidstr) return 0;
	if(uperm && uperm->items) {
		for(i = 0; i < uperm->items; i++)
			if(!strcmp(uperm->mucjid[i], jidstr)) {
				if(permited == -1) {
					if(num) *num = i+1;
					return uperm->is_permited[i];
				}
				uperm->is_permited[i] = permited;
				return 0;
			}
	}
	if(permited == -1) return 0;
	if(!uperm) {
		uperm = malloc(sizeof(Users_Perm));
		uperm->items = 0;
		uperm->mucjid = 0;
		uperm->stamp = 0;
		uperm->mp = 0;
		uperm->is_permited = 0;
	}
	uperm->items++;
	uperm->mucjid = realloc(uperm->mucjid, sizeof(char *)*uperm->items);
	uperm->is_permited = realloc(uperm->is_permited, sizeof(char)*uperm->items);
	uperm->stamp = realloc(uperm->stamp, sizeof(User_Stamp *)*uperm->items);
	uperm->mp = realloc(uperm->mp, sizeof(MUC_Priv *)*uperm->items);

	uperm->mucjid[uperm->items-1] = SCollect(1, jidstr);
	uperm->is_permited[uperm->items-1] = permited;
	uperm->stamp[uperm->items-1] = malloc(sizeof(User_Stamp));
	uperm->stamp[uperm->items-1]->nick = 0;
	uperm->stamp[uperm->items-1]->jid = 0;
	uperm->stamp[uperm->items-1]->client_name = 0;
	uperm->stamp[uperm->items-1]->client_version = 0;
	uperm->stamp[uperm->items-1]->client_os = 0;
	uperm->mp[uperm->items-1] = malloc(sizeof(MUC_Priv));
	uperm->mp[uperm->items-1]->role = 0;
	uperm->mp[uperm->items-1]->affiliation = 0;
	uperm->mp[uperm->items-1]->jid = 0;
	uperm->mp[uperm->items-1]->nick = 0;

	return 0;
}

void Fill_Userdata(char *jidstr, char *jid, char *nick, char *role, char *affiliation)
{
	unsigned int i;
	if(!jidstr) return;
	if(!uperm) Permit(jidstr, 0, 0);
	for(i = 0; i < uperm->items; i++)
		if(!strcmp(uperm->mucjid[i], jidstr)) {
			uperm->mp[i]->jid = jid ? SCollect(1, jid) : 0;
			uperm->mp[i]->nick = nick ? SCollect(1, nick) : 0;
			uperm->mp[i]->role = role ? SCollect(1, role) : 0;
			uperm->mp[i]->affiliation = affiliation ? SCollect(1, affiliation):0;
			uperm->stamp[i]->nick = uperm->mp[i]->nick;
			uperm->stamp[i]->jid = uperm->mp[i]->jid;
			return;
		}
	Permit(jidstr, 0, 0);
	Fill_Userdata(jidstr, jid, nick, role, affiliation);
}

void Fill_Userstamp(char *jidstr, char *client_name, char *client_version, char *client_os)
{
	unsigned int i;
	if(!jidstr) return;
	if(!uperm) Permit(jidstr, 0, 0);
	for(i = 0; i < uperm->items; i++)
		if(!strcmp(uperm->mucjid[i], jidstr)) {
			uperm->stamp[i]->client_name =  client_name ? SCollect(1, client_name) : 0;
			uperm->stamp[i]->client_version = client_version ? SCollect(1, client_version) : 0;
			uperm->stamp[i]->client_os = client_os ? SCollect(1, client_os) : 0;
			return;
		}
	Permit(jidstr, 0, 0);
	Fill_Userstamp(jidstr, client_name, client_version, client_os);
}

/* When user leaves chatroom */
void Del_Permit_Item(char *jidstr)
{
	unsigned int i, j;
	if(!jidstr || !uperm || !uperm->items) return;
	for(i=0; i < uperm->items; i++)
		if(!strcmp(uperm->mucjid[i], jidstr)) {
			Destruct_Uperm_Item(uperm, i);
			for(j = i; j < uperm->items-1; j++) {
				uperm->mucjid[j] = uperm->mucjid[j+1];
				uperm->is_permited[j] = uperm->is_permited[j+1];
				uperm->stamp[j] = uperm->stamp[j+1];
				uperm->mp[j] = uperm->mp[j+1];
			}
			uperm->items--;
			uperm->mucjid = realloc(uperm->mucjid, sizeof(char *)*uperm->items);
			uperm->stamp = realloc(uperm->stamp, sizeof(User_Stamp *)*uperm->items);
			uperm->mp = realloc(uperm->mp, sizeof(MUC_Priv *)*uperm->items);
			uperm->is_permited = realloc(uperm->is_permited, sizeof(char)*uperm->items);
			return;
		}
}

/* Destructors */

void Destruct_ACL(void)
{
	unsigned int i;
	if(!acl || !acl->lines || !acl->stamp || !acl->act) return;
	for(i=0; i < acl->lines; i++) {
		Destruct_User_Stamp(acl->stamp[i]);
		Destruct_Acts(acl->act[i]);
	}
	acl->lines = 0;
	free(acl);
}

void Destruct_User_Stamp(User_Stamp *us)
{
	if(!us) return;
	if(us->nick) free(us->nick);
	if(us->jid) free(us->jid);
	if(us->client_name) free(us->client_name);
	if(us->client_version) free(us->client_version);
	if(us->client_os) free(us->client_os);
	free(us);
}

void Destruct_Acts(Acts *as)
{
	if(!as) return;
	if(as->action) free(as->action);
	if(as->next_at) free(as->next_at);
	as->num = 0;
	free(as);
}

char CCC(char *cn, char *body)
{
	ssize_t len;

	if(!cn || !body) return 0;
	len = strlen(cn);
	if(!strncmp(cn, body, len) && strlen(body) > len+1 && isspace(body[len]))
		return 1;
	return 0;
}

void Apply_On(JID *jid, char *jidstr)
{
	unsigned int i, num = 0;
	char *room_jid;
	char match;
	if(!jidstr || !acl) return;
	Permit(jidstr, -1, &num);
	if(!num) return;
	num--; /* Num in Permit() filled with line number - not real index */

	for(i = 0; i < acl->lines; i++) {
		match = 1;
		if(acl->stamp[i]->nick && ( !uperm->stamp[num]->nick ||
			(uperm->stamp[num]->nick && (strcmp(acl->stamp[i]->nick, uperm->stamp[num]->nick))
			) ) ) match = 0;
		if(acl->stamp[i]->jid && ( !uperm->stamp[num]->jid ||
			(uperm->stamp[num]->jid && (strcmp(acl->stamp[i]->jid, uperm->stamp[num]->jid))
			) ) ) match = 0;
		if(acl->stamp[i]->client_name && ( !uperm->stamp[num]->client_name ||
			(uperm->stamp[num]->client_name && (strcmp(acl->stamp[i]->client_name, uperm->stamp[num]->client_name))
			) ) ) match = 0;
		if(acl->stamp[i]->client_version && ( !uperm->stamp[num]->client_version ||
			(uperm->stamp[num]->client_version && (strcmp(acl->stamp[i]->client_version, uperm->stamp[num]->client_version))
			) ) ) match = 0;
		if(acl->stamp[i]->client_os && ( !uperm->stamp[num]->client_os ||
			(uperm->stamp[num]->client_os && (strcmp(acl->stamp[i]->client_os, uperm->stamp[num]->client_os))
			) ) ) match = 0;
		if(match) break;
	}
	if(!match) return;
	i--;

	room_jid = SCollect(3, jid->_child->node, "@", jid->_child->domain);
	if(acl->act[i]->action[0] == DEVOICE) {
		Do_Devoice(jid, room_jid, uperm->stamp[num]->nick);
	} else if(acl->act[i]->action[0] == KICK) {
		Do_Kick(jid, room_jid, uperm->stamp[num]->nick);
	} else if(acl->act[i]->action[0] == BAN) {
		Do_Ban(jid, room_jid, uperm->stamp[num]->jid);
	} else if(acl->act[i]->action[0] == VOICE) {
		Do_Voice(jid, room_jid, uperm->stamp[num]->nick);
	} else if(acl->act[i]->action[0] == NONE) {
		Do_None(jid, room_jid, uperm->stamp[num]->jid);
	} else if(acl->act[i]->action[0] == MODERATOR) {
		Do_Moderator(jid, room_jid, uperm->stamp[num]->nick);
	} else if(acl->act[i]->action[0] == ADMIN) {
		Do_Admin(jid, room_jid, uperm->stamp[num]->jid);
	} else if(acl->act[i]->action[0] == OWNER) {
		Do_Owner(jid, room_jid, uperm->stamp[num]->jid);
	} else if(acl->act[i]->action[0] == PARTICIPANT) {
		Do_Participant(jid, room_jid, uperm->stamp[num]->nick);
	}
	free(room_jid);
}

char Permit_by_Nick(JID *jid, char *nick, signed char permited, unsigned int *num)
{
	char *jidstr;
	char ret = 0;
	if(!jid || !jid->_child || !jid->_child->_muc || !nick) return 0;
	jidstr = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", nick);
	ret = Permit(jidstr, permited, num);
	free(jidstr);	

	return ret;
}

void Destruct_Uperm_Item(Users_Perm *up, unsigned int i)
{
	free(uperm->mucjid[i]);
	if(uperm->stamp[i]) {
		User_Stamp *us = uperm->stamp[i];
		if(us->client_name) free(us->client_name);
		if(us->client_version) free(us->client_version);
		if(us->client_os) free(us->client_os);
		free(us);
	}
	Destruct_MUC_Priv(uperm->mp[i]);
}
