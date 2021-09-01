#ifndef ACCESS_COMMON_H
#define ACCESS_COMMON_H

#include "types.h"

char *ACL_To_Text(char sw_nums);
void Destruct_ACL(void);
void Destruct_User_Stamp(User_Stamp *us);
void Destruct_Acts(Acts *as);

char Permit(char *jidstr, signed char permited, unsigned int *num);
char Permit_by_Nick(JID *jid, char *nick, signed char permited, unsigned int *num);
void Del_Permit_Item(char *jidstr);

void Fill_Userdata(char *jidstr, char *jid, char *nick, char *role, char *affiliation);
void Fill_Userstamp(char *jidstr, char *client_name, char *client_version, char *client_os);

void Apply_On(JID *jid, char *jidstr);

char CCC(char *cn, char *body);

void Destruct_Uperm_Item(Users_Perm *up, unsigned int i); // To avoid double free

#endif
