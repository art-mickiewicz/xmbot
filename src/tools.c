#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <poll.h>
#include <time.h>
#include <iconv.h>
#include "tools.h"
#include "control.h"

char *src_encoding;

char Transmit(int sock, char *msg, ssize_t size)
{
	u_int16_t i = 0;
	ssize_t idx = 0;
	ssize_t jdx = 0;

	if(!size) size = strlen(msg);
	while( size-idx > idx && i <= ATTEMP_LIMIT) {
		jdx = send(sock, msg+idx, size-idx, 0);
		if(jdx > 0) idx += jdx;
		i++;
	}
	return (i <= ATTEMP_LIMIT)?1:0;
}

char *Receive(int sock, ssize_t *size, int timeout)
{
	char *msg = 0;
	char c;
	int poll_result;
	ssize_t i = 1;
	struct pollfd pfd;

	msg = 0;
	pfd.fd = sock;
	pfd.events = POLLIN;
	if(timeout < 0) poll_result = poll(&pfd, 1, -1);
	else poll_result = poll(&pfd, 1, timeout*1000); 
	if(!poll_result) {
		return 0;
	} else if (poll_result < 0) {
		perror("ERROR");
		return 0;
	}
	
	while(recv(sock, &c, sizeof(char), 0) > 0 && c != '\0') {
		msg = (char *)realloc(msg, sizeof(char)*i);
		msg[i-1] = c;
		i++;
	}
	if(msg) {
		msg = (char *)realloc(msg, sizeof(char)*(i+1));
		msg[i] = '\0';
	}
	if(size) (*size) = i - 1;
	return msg;
}

char *Gen_Id(int seed)
{
	int id;
	char *sid;

	if(seed) srand(seed);
	id = 1 + (int)(16777216 * (rand() / (RAND_MAX + 1.0)));

	sid = malloc(sizeof(char)*7);
	sprintf(sid, "%06x", id);
	sid[6] = '\0';

	return sid;
}

char *UTF8_Convert(char *in, char in_utf)
{
	iconv_t cd;
	ssize_t i, size;
	ssize_t state;
	ssize_t inbytesleft, outbytesleft;
	char *out;
	char *output;

	if(!strcmp(src_encoding, "UTF-8")) return in;
	if(!in_utf) cd = iconv_open("UTF-8", src_encoding);
	else cd = iconv_open(src_encoding, "UTF-8");
	if(cd == (iconv_t)(-1)) {
		perror("ERROR");
		return 0;
	}

	size = (strlen(in)+1);
	inbytesleft = size;
	outbytesleft = size*6;
	out = (char *)malloc(sizeof(char)*size*6); // UTF-8 maximum 6 bytes per char (In fact 4)
	memset(out, '\0', size);

	output = out;
	while( (state = iconv(cd, (char **)&in, &inbytesleft, (char **)&output, &outbytesleft)) > 0 )
	if(state == -1) {
			perror("ERROR");
			return 0;
	}

	iconv_close(cd);
	return out;
}

char *SCollect(unsigned char num, ...)
{
	char *out = 0;
	ssize_t size = 0; //size of out without '\0'
	va_list a;
	
	if(!num) return 0;
	out = malloc(sizeof(char));
	out[0] = '\0';
	va_start(a, num);

	while(num--) {
		char *c = va_arg(a, char *);
		if(c) {
			size += strlen(c);
			out = realloc(out, sizeof(char)*(size+1));
			strcat(out, c);
		}
	}

	va_end(a);
	return out;
}

void SAppend(unsigned char num, char **target, ...)
{
	ssize_t size = 0; //size of out without '\0'
	va_list a;
	
	if(num < 2 || !target) return;
	if(!*target) {
		*target = malloc(sizeof(char));
		**target = '\0';
	}

	size += strlen(*target);
	num--;
	va_start(a, target);

	while(num--) {
		char *c = va_arg(a, char *);
		if(c) {
			size += strlen(c);
			*target = realloc(*target, sizeof(char)*(size+1));
			strcat(*target, c);
		}
	}

	va_end(a);
}

char *GetSubStr(char *in, ssize_t start, ssize_t end)
{
	char *out = malloc((end-start+2)*sizeof(char));
	ssize_t i;
	for(i = 0; i < (end-start+1); i++)
		out[i] = (in+start)[i];
	out[i] = '\0';
	return out;
}

char Get_Time_Var_Type(char *buff, ssize_t *end_index)
{
	ssize_t i, j;
	char bk;
	char evar = 0;
	char chance[] = { 0, 1, 1, 1, 1, 1, 1 };
	const char *strtime[] = { "", "year", "month", "day", "hour", "minute", "second" };

	i = j = 0;
	if(!buff || buff[0] != '%') return 0;
	while(isalnum(buff[++i]))
		for(j=1; j < 7; j++)
			if(chance[j]) {
				if(strtime[j][i-1]!=buff[i]) chance[j] = 0;
				else if(strtime[j][i]=='\0') {
					evar = j;
					break;
				}
			}

	if(end_index) *end_index = i-1;
	return evar;
}

char *Get_Time_Var(char type)
{
	char *var = 0;

	time_t t;
	struct tm tms;
	time(&t);
	tms = *localtime(&t);
	switch(type) {
	case VAR_YEAR:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%y", &tms);
		break;
	case VAR_MONTH:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%m", &tms);
		break;
	case VAR_DAY:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%d", &tms);
		break;
	case VAR_HOUR:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%H", &tms);
		break;
	case VAR_MINUTE:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%M", &tms);
		break;
	case VAR_SECOND:
		var = malloc(sizeof(char)*3);
		strftime(var, 3, "%S", &tms);
		break;
	}

	return var;
}

char *Resolve_Time(char *f)
{
	ssize_t len = strlen(f);
	ssize_t i, j=0;
	char evar = 0;
	char *out = 0;
	char *tmpout = 0;
	for(i=0; i < len; i++) {
		if( (!i && f[i]=='%') || ((i>0)?f[i-1]!='\\':1 && f[i] == '%' && (i<len-1)?isalnum(f[i+1]):0) )
			if(evar = Get_Time_Var_Type(f+i, &j)) {
				char *var = Get_Time_Var(evar);
				if(var) {
					char *begin = GetSubStr(f, 0, i-1);
					tmpout = SCollect(3, begin, var, f+i+j+1);
					free(var);
					free(begin);
					out = Resolve_Time(tmpout);
					free(tmpout);
					if(out) return out;
					break;
				}
			}
	}

	out = SCollect(1, f);
	return out;
}

char Get_RoomNick_Var_Type(char *buff, ssize_t *end_index)
{
	ssize_t i, j;
	char bk;
	char evar = 0;
	char chance[] = { 0, 1, 1 };
	const char *strtime[] = { "", "room", "nick" };

	if(!buff || buff[0] != '%') return 0;

	i = j = 0;
	while(isalnum(buff[++i]))
		for(j=1; j < 3; j++)
			if(chance[j]) {
				if(strtime[j][i-1]!=buff[i]) chance[j] = 0;
				else if(strtime[j][i]=='\0') {
					evar = j;
					break;
				}
			}

	if(end_index) *end_index = i-1;
	return evar;
}

char *Get_RoomNick_Var(char type, JID *jid, char *from)
{
	char *var = 0;
	JID *cjid;

	if(!jid || !from) return 0;

	cjid = Get_JID_From_String(from);
	if(!cjid || !cjid->node || !cjid->domain) return 0;
	if(jid->_child && jid->_child->node && jid->_child->domain && !strcmp(cjid->node, jid->_child->node) && !strcmp(cjid->domain, jid->_child->domain) ) {
		if(type == VAR_ROOM) var = SCollect(1, cjid->node);
		else if(type == VAR_NICK) {
			if(cjid->resource)
				var = SCollect(1, cjid->resource);
			else {
				var = malloc(sizeof(char));
				*var = '\0';
			}
		}
	} else {
		if(type == VAR_NICK) var = SCollect(1, cjid->node);
		else if(type == VAR_ROOM) {
			var = malloc(sizeof(char));
			*var = '\0';
		}
	}

	Destruct_JID(cjid);
	return var;
}

char *Resolve_RoomNick(char *prefix, JID *jid, char *from)
{
	ssize_t i, j=0, len;
	char evar;
	char *var = 0;
	char *out = 0;
	char *tmpout = 0;

	len = strlen(prefix);
	for(i=0; i < len; i++) {
		if( (!i && prefix[i]=='%') || ((i>0)?prefix[i-1]!='\\':1 && prefix[i] == '%' && (i<len-1)?isalnum(prefix[i+1]):0) )
			if(evar = Get_RoomNick_Var_Type(prefix+i, &j)) {
				var = Get_RoomNick_Var(evar, jid, from);
				if(var) {
					char *begin = GetSubStr(prefix, 0, i-1);
					tmpout = SCollect(3, begin, var, prefix+i+j+1);
					free(var);
					free(begin);
					out = Resolve_RoomNick(tmpout, jid, from);
					free(tmpout);
					if(out)	return out;
					break;
				}
			}
	}

	out = SCollect(1, prefix);
	return out;
}

char Check_JID_Mask(char *jidmask, char *jid)
{
	char result = 1;
	JID *Jidmask, *Jid;

	if(!jidmask || !jid) return 1;
	Jidmask = Get_JID_From_String(jidmask);
	if(!Jidmask) return 0;
	Jid = Get_JID_From_String(jid);
	if(!Jid) {
		Destruct_JID(Jidmask);
		return 0;
	}
	if(Jidmask->node && Jid->node && strcmp(Jidmask->node, Jid->node)) result = 0;
	else if(Jidmask->domain && Jid->domain && strcmp(Jidmask->domain, Jid->domain)) result = 0;
	else if(Jidmask->resource && Jid->resource && strcmp(Jidmask->resource, Jid->resource)) result = 0;

	Destruct_JID(Jidmask);
	Destruct_JID(Jid);
	return result;
}

char Check_If_Groupchat(JID *primjid, char *jidstr, char **nick)
{
	JID *jid;

	if(!jidstr || !primjid) return 0;
	jid = Get_JID_From_String(jidstr);
	if(!jid) return 0;
	if(primjid->_child)
		if(!strcmp(primjid->_child->domain, jid->domain)) {
			if(nick) *nick = SCollect(1, jid->resource);
			Destruct_JID(jid);
			return 1;
		}
	if(!strcmp(primjid->domain, jid->domain)) {
		if(nick) *nick = SCollect(1, jid->node);
		Destruct_JID(jid);
		return 0;
	}

	Destruct_JID(jid);
	return 0;
}

char alien_msg(JID *jid, XMPP_Msg *msg)
{
	char *tmp;
	if(!jid || !msg || !msg->from) return 0;
	tmp = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);
	if(!strcmp(tmp, msg->from)) {
		free(tmp);
		return 0;
	}
	free(tmp);
	if(jid->_child) {
		tmp = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
		if(!strcmp(tmp, msg->from)) {
			free(tmp);
			return 0;
		}
		free(tmp);
	}

	return 1;
}

char alien_iq(JID *jid, XMPP_IQ *iq)
{
	char *tmp;
	if(!jid || !iq || !iq->from) return 0;
	tmp = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);
	if(!strcmp(tmp, iq->from)) {
		free(tmp);
		return 0;
	}
	free(tmp);
	if(jid->_child) {
		tmp = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
		if(!strcmp(tmp, iq->from)) {
			free(tmp);
			return 0;
		}
		free(tmp);
	}

	return 1;
}

char alien_presence(JID *jid, XMPP_Presence *p)
{
	char *tmp;
	if(!jid || !p || !p->from) return 0;
	tmp = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);
	if(!strcmp(tmp, p->from)) {
		free(tmp);
		return 0;
	}
	free(tmp);
	if(jid->_child) {
		tmp = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
		if(!strcmp(tmp, p->from)) {
			free(tmp);
			return 0;
		}
		free(tmp);
	}

	return 1;
}

in_addr_t Get_Addr(char *host)
{
	char i;
	struct hostent *h;
	in_addr_t a = 0;

	if(!host) return 0;
	
	h = gethostbyname(host);
	if(!h) {
		perror("ERROR");
		return 0;
	}

	if(h->h_length == 4)
		for(i=0; i < 4; i++)
			a += (in_addr_t)((h->h_addr_list[0][i])&0xFF)<<((4-(i+1))*8);
	else {
		myerror("Bad address");
		return 0;
	}

	return a;
}

char *Seek(char *buff, char *mark, char **start)
{
	ssize_t i = 0;
	char *s;

	while(buff && *buff != '\0') {
		i = 0;
		while(mark[i] != '\0' && *buff != '\0' && *buff == mark[i]) {
			if(!i) s = buff;
			if(mark[i+1] == '\0') {
				if(start) *start = s;
				return ++buff;
			}
			buff++;
			i++;
		}
		buff++;
	}

	return 0;
}

char *Data_Filter(char *buff, char sw_fchar, char sw_ftag)
{
	char *out = 0;
	char sw_tag = 0;
	ssize_t i = 0;

	if(!buff) return 0;
	while(buff && *buff != '\0') {
		if(sw_ftag && *buff == '<' && !sw_tag) sw_tag = 1;
		else if(sw_ftag && *buff == '>' && sw_tag) sw_tag = 0;
		else if(sw_fchar && *buff == '&') {
			out = realloc(out, sizeof(char)*(i+5));
			memcpy(out+i, "&amp;", 5);
			i += 5;
		} else if(!sw_tag) {
			out = realloc(out, sizeof(char)*(i+1));
			out[i++] = *buff;
		}
		buff++;
	}

	if(i) {
		out = realloc(out, sizeof(char)*(i+1));
		out[i] = '\0';
		return out;
	}
	return 0;
}

char *Make_W3_Query(char *buff)
{
	char *out = 0;
	size_t i = 0;
	char c[4];
	c[0] = '%';
	c[3] = '\0';

	if(!buff) return 0;
	while(buff && *buff != '\0') {
		if((unsigned char)(*buff) > 0x7f || *buff == '&' || *buff == '?' || *buff == '+') {
			snprintf(c+1, 3, "%02X", (unsigned char)(*buff));
			out = realloc(out, sizeof(char)*(i+3));
			memcpy(out+i, c, 3);
			i += 3;
		} else {
			out = realloc(out, sizeof(char)*(i+1));
			if(*buff == ' ') out[i++] = '+';
			else out[i++] = *buff;
		}
		buff++;
	}

	if(i) {
		out = realloc(out, sizeof(char)*(i+1));
		out[i] = '\0';
		return out;
	}
	return 0;
}

JID *Get_JID_From_String(char *s)
{
	JID *jid;
	unsigned int i, size;
	unsigned int at_idx, sl_idx;
	unsigned int rborder;
	char elements = 0;

	if(!s || !strlen(s)) return 0;
	
	jid = malloc(sizeof(JID));
	jid->node = 0;
	jid->domain = 0;
	jid->resource = 0;
	jid->_sockfd = 0;
	jid->_muc = 0;
	jid->_parent = 0;
	jid->_child = 0;

	at_idx = sl_idx = 0;
	size = strlen(s);
	for(i=0; i < size && elements < 2; i++)
		switch(s[i]) {
		case '@':
			at_idx = i;
			elements++;
			break;
		case '/':
			sl_idx = i;
			elements++;
			break;
		}
	if(sl_idx && at_idx && sl_idx < at_idx) return 0;
	if(sl_idx == size || at_idx == size) return 0;
	if(at_idx) {
		jid->node = malloc(sizeof(char)*(at_idx+1));
		memcpy(jid->node, s, at_idx);
		jid->node[at_idx] = '\0';
	}
	if(sl_idx) {
		jid->resource = malloc(sizeof(char)*(size-sl_idx));
		memcpy(jid->resource, s+sl_idx+1, size-sl_idx-1);
		jid->resource[size-sl_idx-1] = '\0';
	}
	rborder = sl_idx?sl_idx:size;
	jid->domain = malloc(sizeof(char)*(rborder-at_idx));
	memcpy(jid->domain, s+at_idx+1, rborder-at_idx-1);
	jid->domain[rborder-at_idx-1] = '\0';

	return jid;
}
