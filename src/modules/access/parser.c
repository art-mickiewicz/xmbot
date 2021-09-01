#include "../modlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "parser.h"

Access_List *Read_ACL(char *filename, char *stat)
{
	int fd;
	char *buff, *oldbuff;
	char **tmp;
	char *startline;
	ssize_t i, j;
	Access_List *acl;

	if((fd = open(filename, O_RDONLY|O_NONBLOCK)) < 0) {
		perror("ERROR");
		if(stat) *stat = 1;
		return 0;
	}

	i = 0;
	j = 1;
	buff = malloc(sizeof(char));
	while((i = read(fd, buff+j-1, 1)) > 0) {
		j += i;
		buff = realloc(buff, j*sizeof(char));
		lseek(fd, j-1, SEEK_SET);
	}
	close(fd);
	oldbuff = buff;

	acl = malloc(sizeof(Access_List));
	acl->lines = 0;
	acl->stamp = 0;
	acl->act = 0;

	startline = buff;
	for(;j > 0; j--) {
		if(*buff == '\n' || *buff == '\0') {
			*buff = '\0';
			tmp = Parse_Line(startline);
			Add_Context(acl, tmp);
			if(j > 1) startline = buff+1;
		}
		buff++;
	}

	free(oldbuff);

	if(!acl->lines) {
		free(acl);
		return 0;
	}

	if(stat) *stat = 0;
	return acl;
}

/* Allocation occured */
char **Parse_Line(char *line)
{
	ssize_t size;
	ssize_t i;
	char **out;
	char *null;
	char *prev;

	if(!line) return 0;
	out = malloc(sizeof(char *)*6);

	for(i=0; i < 6; i++) {
		prev = line;
		if(line = Seek(line, "|", &null)) *null = '\0';
		if(prev && *prev != '\0') out[i] = SCollect(1, prev);
		else out[i] = 0;
	}

	return out;
}

/* Freeing or construction occured */
void Add_Context(Access_List *acl, char **fields)
{
	Acts *acts = 0;

	if(!fields[0]) return;
	if(acts = Get_Acts(fields[0])) {
		acl->lines++;
		acl->act = realloc(acl->act, sizeof(Acts *)*acl->lines);
		acl->act[acl->lines-1] = acts;
		acl->stamp = realloc(acl->stamp, sizeof(User_Stamp *)*acl->lines);
		acl->stamp[acl->lines-1] = malloc(sizeof(User_Stamp));
		acl->stamp[acl->lines-1]->nick = fields[1];
		acl->stamp[acl->lines-1]->jid = fields[2];
		acl->stamp[acl->lines-1]->client_name = fields[3];
		acl->stamp[acl->lines-1]->client_version = fields[4];
		acl->stamp[acl->lines-1]->client_os = fields[5];
	}
}

Acts *Get_Acts(char *actstr)
{
	Acts *acts;
	char *start;
	char *digstart;
	char curact;
	char tmp;
	ssize_t c;
	unsigned int mpx;
	
	if(!actstr || !isalpha(actstr[0])) return 0;
	acts = malloc(sizeof(Acts));
	acts->num = 0;
	acts->action = 0;
	acts->next_at = 0;
	
	for(;;) {
		start = actstr;
		while(isalpha(*actstr)) {
			*actstr = toupper(*actstr);
			actstr++;
		}
		tmp = *actstr;
		*actstr = '\0';
		if(!strcmp(start, "DEVOICE")) curact = DEVOICE;
		else if(!strcmp(start, "KICK")) curact = KICK;
		else if(!strcmp(start, "BAN")) curact = BAN;
		else if(!strcmp(start, "VOICE")) curact = VOICE;
		else if(!strcmp(start, "NONE")) curact = NONE;
		else if(!strcmp(start, "MODERATOR")) curact = MODERATOR;
		else if(!strcmp(start, "ADMIN")) curact = ADMIN;
		else if(!strcmp(start, "OWNER")) curact = OWNER;
		else if(!strcmp(start, "PARTICIPANT")) curact = PARTICIPANT;
		else break;
		*actstr = tmp;
		
		acts->num++;
		acts->action = realloc(acts->action, sizeof(char)*acts->num);
		acts->next_at = realloc(acts->next_at, sizeof(unsigned int)*acts->num);

		acts->action[acts->num-1] = curact;
		acts->next_at[acts->num-1] = 0;
		
		if(isdigit(*actstr)) {
			digstart = actstr;
			c = 0;
			while(isdigit(*actstr)) {
				actstr++;
				c++;
			}
			mpx = 1;
			while(c-- > 0) {
				acts->next_at[acts->num-1] += (digstart[c]-'0')*mpx;
				mpx *= 10;
			}
		} else break;
	}

	if(!acts->num) {
		free(acts);
		return 0;
	}

	return acts;
}
