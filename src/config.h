/*** Parsing configuration file ***/
#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>

/** Full config file structure **/
typedef struct {
	char *name;
	char *value;
} OptVal;

typedef struct {
	char *room;
	char *domain;
	char *nick;
	char *password;
} Section_Client_MUC;
typedef struct {
	char *host;
	char *port;
	char *hostname;
	char *user;
	char *password;
	Section_Client_MUC *muc;
} Section_Client;

typedef struct {
	char *file;
	OptVal *optval;
	ssize_t pairs;
} Section_Modules_Load;
typedef struct {
	Section_Modules_Load *load;
	ssize_t quantity;
} Section_Modules;

typedef struct {
	char *type;
	char *jid;
	char *prefix;
	char *file;
} Logfile;
typedef struct {
	char *encoding;
	Logfile *logfile;
	ssize_t logfiles;
} Section_Common;

typedef struct {
	Section_Client *client;
	Section_Modules *modules;
	Section_Common *common;
} Config;

/** Parsing functions **/

Config *Read_Config(char *path);
Section_Client *Read_Config_Client(char *buff, ssize_t size);
Section_Client_MUC *Read_Config_Client_MUC(char *buff, ssize_t size);
Section_Modules *Read_Config_Modules(char *buff, ssize_t size);
Section_Modules_Load Read_Config_Modules_Load(char *buff, ssize_t size);
Section_Common *Read_Config_Common(char *buff, ssize_t size);

#endif
