#ifndef ACCESS_TYPES_H
#define ACCESS_TYPES_H

#define DEVOICE 1
#define KICK 2
#define BAN 3
#define VOICE 4
#define NONE 5
#define MODERATOR 6
#define ADMIN 7
#define OWNER 8
#define PARTICIPANT 9

typedef struct {
	char *nick;
	char *jid;
	char *client_name;
	char *client_version;
	char *client_os;
} User_Stamp;

typedef struct {
	unsigned int num;
	char *action;		// Stores constants
	unsigned int *next_at;	// Value after action of this index
} Acts;

typedef struct {
	unsigned int lines;
	User_Stamp **stamp;
	Acts **act;
} Access_List;

/* ACL control permissions */
typedef struct {
	unsigned int items;
	User_Stamp **stamp;
	MUC_Priv **mp;
	char **mucjid;
	char *is_permited;
} Users_Perm;

#endif
