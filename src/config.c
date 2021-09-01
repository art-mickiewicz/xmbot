#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include "xml-parser.h"

Config *Read_Config(char *path)
{
	int fd;
	ssize_t i, j;
	char *buff = malloc(sizeof(char));
	ssize_t size;
	ssize_t end_index;
	Config *cfg;

	if((fd = open(path, O_RDONLY|O_NONBLOCK)) < 0) {
		perror("ERROR");
		_exit(1);
	}

	i = 0;
	j = 1;
	while((i = read(fd, buff+j-1, 1)) > 0) {
		j += i;
		buff = realloc(buff, j*sizeof(char));
		lseek(fd, j-1, SEEK_SET);
	}
	close(fd);

	size = j;
	end_index = 0;
	cfg = malloc(sizeof(Config));
	
	if(XML_Search(buff, size, &end_index, "client", 0))
		cfg->client = Read_Config_Client(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "modules", 0))
		cfg->modules = Read_Config_Modules(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "common", 0))
		cfg->common = Read_Config_Common(buff+end_index, size-end_index);

	return cfg;
}

Section_Client *Read_Config_Client(char *buff, ssize_t size)
{
	Section_Client *client = malloc(sizeof(Section_Client));
	ssize_t end_index;
	if(XML_Search_Closer(buff, size, &end_index, "client")) size = end_index;
	else return 0;
	if(XML_Search(buff, size, &end_index, "host", 0))
		client->host = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "port", 0))
		client->port = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "hostname", 0))
		client->hostname = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "user", 0))
		client->user = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "password", 0))
		client->password = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "muc", 0))
		client->muc = Read_Config_Client_MUC(buff+end_index, size-end_index);
	return client;
}

Section_Modules *Read_Config_Modules(char *buff, ssize_t size)
{
	ssize_t i = 0;
	ssize_t old_index = 0;
	char *filename = 0;
	ssize_t end_index;

	Section_Modules *modules = malloc(sizeof(Section_Modules));
	if(XML_Search_Closer(buff, size, &end_index, "modules")) size = end_index;
	else return 0;

	modules->quantity = 0;
	modules->load = 0;
	while(filename = XML_Search(buff, size, &end_index, "load", "file")) {
		old_index = end_index;
		if(XML_Search_Closer(buff+end_index, size-end_index, &end_index, "load")) {
			modules->quantity++;
			modules->load = realloc(modules->load, sizeof(Section_Modules_Load) * modules->quantity);
			modules->load[i] = Read_Config_Modules_Load(buff+old_index, end_index);
			modules->load[i].file = filename;
			i++;
		}
		buff += end_index+old_index;
		size -= end_index+old_index;
	}

	return modules;
}

Section_Modules_Load Read_Config_Modules_Load(char *buff, ssize_t size)
{
	Section_Modules_Load load;
	OptVal *optval = 0;
	ssize_t end_index;
	load.pairs = 0;
	while(XML_Search(buff, size, &end_index, "option", 0)) {
		load.pairs++;
		optval = realloc(optval, sizeof(OptVal)*load.pairs);
		optval[load.pairs-1].name = XML_Search(buff, size, 0, "option", "name");
		optval[load.pairs-1].value = XML_Search(buff, size, 0, "option", "value");
		buff+=end_index;
		size-=end_index;
	}

	load.optval = optval;
	return load;
}

Section_Common *Read_Config_Common(char *buff, ssize_t size)
{
	Section_Common *common = malloc(sizeof(Section_Common));
	Logfile *logfile = 0;
	ssize_t end_index;
	ssize_t i = 0;
	ssize_t size2;
	char *buff2;

	if(XML_Search_Closer(buff, size, &end_index, "common")) size = end_index;
	else return 0;
	if(XML_Search(buff, size, &end_index, "encoding", 0))
		common->encoding = XML_Get_Content(buff+end_index, size-end_index);
	size2 = size;
	buff2 = buff;
	while(XML_Search(buff2, size2, &end_index, "logfile", 0)) {
		logfile = realloc(logfile, sizeof(Logfile)*(i+1));
		logfile->type = XML_Search(buff2, size2, 0, "logfile", "type");
		logfile->jid = XML_Search(buff2, size2, 0, "logfile", "jid");
		logfile->prefix = XML_Search(buff2, size2, 0, "logfile", "prefix");
		buff2 += end_index;
		size2 -= end_index;
		logfile->file = XML_Get_Content(buff2, size2);
		XML_Search_Closer(buff2, size2, &end_index, "logfile");
		i++;
		buff2 += end_index;
		size2 -= end_index;
	}
	common->logfile = logfile;
	common->logfiles = i;

	return common;
}

Section_Client_MUC *Read_Config_Client_MUC(char *buff, ssize_t size)
{
	Section_Client_MUC *muc = malloc(sizeof(Section_Client_MUC));
	ssize_t end_index;
	if(XML_Search_Closer(buff, size, &end_index, "muc")) size = end_index;
	else return 0;
	if(XML_Search(buff, size, &end_index, "room", 0))
		muc->room = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "domain", 0))
		muc->domain = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "nick", 0))
		muc->nick = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "password", 0))
		muc->password = XML_Get_Content(buff+end_index, size-end_index);

	return muc;
}
