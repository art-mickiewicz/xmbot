#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include "control.h"
#include "conveyers.h"
#include "tools.h"
#include "log.h"

extern char *src_encoding;
extern char **loaded_modules;
extern unsigned int loaded_modules_num;
extern time_t start_time;

void Load_Modules(Config *cfg)
{
	void *handle;
	void (*registrator)(OptVal *, ssize_t);
	unsigned int i, num;
	num = 0;
	
	for(i=0; i < cfg->modules->quantity; i++) {
		handle = dlopen(cfg->modules->load[i].file, RTLD_LAZY);
		if(!handle) myerror(dlerror());
		else {
			dlerror();
			*(void **)(&registrator) = dlsym(handle, "Register_Module");
			if(dlerror()) continue;
			num++;
			loaded_modules = realloc(loaded_modules, sizeof(char *)*num);
			loaded_modules[num-1] = cfg->modules->load[i].file;
			(*registrator)(cfg->modules->load[i].optval, cfg->modules->load[i].pairs);
		}
	}
	loaded_modules_num = num;
	printf("%i modules loaded\n", num);
}

int main(int argc, char **argv)
{
	Config *cfg;
	JID *jid;
	in_port_t port;
	JID *jid_muc;

	start_time = time(0);

	puts("Reading config file...");
	cfg = Read_Config("xmbot.xml");
	if(!cfg) {
		myerror("Can't read config");
		return 1;
	}
	if(cfg->common) {
		if(cfg->common->encoding) src_encoding = cfg->common->encoding;
		if(cfg->common->logfiles) {
			puts("Log initialization...");
			Init_Log(cfg);
		}
	}

	if(!cfg->client) {
		myerror("Missed client section");
		return 1;
	}
	if(!cfg->client->host) {
		myerror("Missed host");
		return 1;
	}
	if(!cfg->client->user) {
		myerror("Missed user");
		return 1;
	}
	if(!cfg->client->password) {
		myerror("Missed password");
		return 1;
	}
	
	port = 5222;
	if(cfg->client->port) port = atoi(cfg->client->port);
	puts("Trying to connect server...");
	jid = XMPP_Connect(cfg->client->host, port, cfg->client->hostname, UTF8(cfg->client->user), cfg->client->password);
	if(!jid) {
		myerror("Connection failed");
		return 1;
	}
	printf("Received JID: %s@%s/%s\n", jid->node, jid->domain, jid->resource);

	if(!cfg->client->muc->room) {
		puts("Missed room...");
		puts("Ignoring MUC connection");
	} else {
		jid_muc = XMPP_MUC_Connect(jid, UTF8(cfg->client->muc->room), cfg->client->muc->domain, UTF8(cfg->client->muc->nick), cfg->client->muc->password);
		if(!jid_muc) myerror("Failed muc connection");
		else printf("Received JID: %s@%s/%s\n", jid_muc->node, jid_muc->domain, jid_muc->resource);
	}

	puts("Loading modules...");
	Load_Modules(cfg);
	Root_Event_Handler(jid);
}
