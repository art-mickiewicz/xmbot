#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "control.h"
#include "xml-parser.h"
#include "digest_sha1.h"

JID *XMPP_Connect(char *host, in_port_t port, char *hostname, char *username, char *password)
{
	char i;
	in_addr_t a = 0;
	int sock;
	struct sockaddr_in addr;
	XML_Tag *tag;
	char *msg;
	ssize_t size;
	char *id;
	int seed;
	char *tmp;
	char *outbuff;
	JID *jid = 0;

	if(!host || !username || !password) return 0;
	if(!port) port = 5222;
	if(!hostname) hostname = host;
	
	a = Get_Addr(host);
	if(!a) return 0;

	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("ERROR");
		return 0;
	}

	addr.sin_family = PF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(a);

	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("ERROR");
		return 0;
	}

	T(XML_HEADER);
	outbuff = SCollect(3, XMPP_STREAM_INIT_BEGIN, hostname,
		XMPP_STREAM_INIT_END_HOST_);
	T(outbuff);
	free(outbuff);

	msg = R(&size, 10);
	if(!msg) goto input_error;
	id = XML_Search(msg, size, 0, "stream:stream", "id");
	free(msg);
	if(!id) {
		myerror("Didn't get ID");
		return 0;
	}
	seed = id[strlen(id)-1]*8;

	outbuff = SCollect(5, XMPP_IQ_GET_AUTH_BEGIN, hostname,
		XMPP_IQ_GET_AUTH_END_HOST, username,
		XMPP_IQ_GET_AUTH_END_USER_);
	T(outbuff);
	free(outbuff);

	msg = R(&size, 10);
	if(!msg) goto input_error;
	if(XML_Search(msg, size, 0, "digest", 0)) {
		unsigned char *hdigest;

		tmp = SCollect(2, id, password);
		free(id);
		hdigest = Digest_SHA1((unsigned char *)tmp, strlen(tmp));
		free(tmp);

		outbuff = SCollect(7, XMPP_IQ_SET_AUTH_BEGIN, hostname,
			XMPP_IQ_SET_AUTH_END_HOST, username,
			XMPP_IQ_SET_AUTH_END_USER, hdigest,
			XMPP_IQ_SET_AUTH_END_DIGEST_);
		free(hdigest);
		T(outbuff);
		free(outbuff);
	} else {
		puts("Else occured ;)");
	}
	free(msg);

	msg = R(&size, 10);
	if(!msg) goto input_error;
	tmp = XML_Search(msg, size, 0, "error", "code");
	free(msg);
	if(tmp && !strcmp(tmp, "401")) {
		free(tmp);
		myerror("Authentication failed");
		return(0);
	}

	id = Gen_Id(seed*13);
	outbuff = SCollect(3, XMPP_IQ_GET_ROSTER_BEGIN, id,
		XMPP_IQ_GET_ROSTER_END_ID_);
	T(outbuff);
	free(outbuff);

	msg = R(&size, 10);
	if(!msg) goto input_error;
	tmp = XML_Search(msg, size, 0, "iq", "id");
	if(!tmp || strcmp(id, tmp)) {
		free(msg);
		free(id);
		myerror("Failed ID from server");
		return 0;
	}
	free(tmp);
	free(id);
	tmp = XML_Search(msg, size, 0, "iq", "from");
	free(msg);
	if(!tmp) {
		myerror("Didn't receive JID");
		return 0;
	}

	T(XMPP_PRESENCE_PRIORITY);

	jid = Get_JID_From_String(tmp);
	free(tmp);
	if(jid) {
		jid->_sockfd = sock;
		return jid;
	}

	return 0;
	input_error:
	myerror("Input failed");
	return 0;
}

JID *XMPP_MUC_Connect(JID *primjid, char *room, char *domain, char *nick, char *password)
{
	int sock = primjid->_sockfd;
	char *msg;
	char *jid_string;
	char *outbuff;
	ssize_t size;
	JID *jid;

	if(!primjid) return 0;
	if(!sock || !room) return 0;
	if(!domain) domain = primjid->domain;
	if(!nick) nick = primjid->node;
	if(!domain || !nick) return 0;

	outbuff = SCollect(9, XMPP_PRESENCE_PRIORITY_BEGIN, room, "@", domain,
		"/", nick, XMPP_PRESENCE_PRIORITY_END_TO, "0",
		XMPP_PRESENCE_PRIORITY_END_PRIORITY_);
	T(outbuff);
	free(outbuff);
	
	jid_string = SCollect(5, room, "@", domain, "/", nick);

	while(msg = R(&size, 10)) {
		if(XML_Search_Match(msg, size, 0, "presence", "from", jid_string)) {
			free(jid_string);
			break;
		}
		free(msg);
	}
	if(!msg) {
		myerror("Wrong response");
		free(jid_string);
		free(msg);
		return 0;
	}
	
	jid_string = SCollect(5, primjid->node, "@", primjid->domain, "/",
		primjid->resource);

	if( ! XML_Search_Match(msg, size, 0, "presence", "to", jid_string)) {
		myerror("Wrong response");
		free(jid_string);
		free(msg);
		return 0;
	}

	jid = malloc(sizeof(JID));

	if(! (jid->_muc = Construct_MUC_Priv(msg, size)) ) {
		myerror("MUC connection failed");
		Destruct_JID(jid);
		if(msg) free(msg);
		return 0;
	}
	if(!jid->_muc->role) {
		myerror("Didn't get MUC role");
		Destruct_JID(jid);
		free(msg);
		return 0;
	}
	if(!jid->_muc->affiliation) {
		myerror("Didn't get MUC affiliation");
		Destruct_JID(jid);
		free(msg);
		return 0;
	}
	if(!jid->_muc->jid) jid->_muc->jid = jid_string;
	else free(jid_string);
	
	jid->node = room;
	jid->domain = domain;
	jid->resource = nick;
	jid->_sockfd = 0;
	jid->_parent = primjid;
	primjid->_child = jid;

	outbuff = SCollect(9, XMPP_PRESENCE_PRIORITY_BEGIN, room, "@", domain,
		"/", nick, XMPP_PRESENCE_PRIORITY_END_TO, "5",
		XMPP_PRESENCE_PRIORITY_END_PRIORITY_);
	T(outbuff);
	free(outbuff);

	return jid;
}

void XMPP_Send_Msg(JID *jid, char *to, char *type, char *message, char recode_flag)
{
	int sock = 0;
	char is_groupchat = 0;
	char alt_is_groupchat = 1;
	JID *tmpjid;
	char *outbuff;
	char *id;

	if(!message || !jid) return;
	sock = jid->_sockfd;
	if(!sock || !to) return;
	alt_is_groupchat = Check_If_Groupchat(jid, to, 0);
	if(type && !strcmp("groupchat", type)) is_groupchat = 1;
	if(!type) is_groupchat = alt_is_groupchat;

	outbuff = SCollect(1, XMPP_MESSAGE_BEGIN);
	if(is_groupchat) SAppend(2, &outbuff, "groupchat");
	else SAppend(2, &outbuff, "chat");
	id = Gen_Id(0);
	SAppend(4, &outbuff, XMPP_MESSAGE_END_TYPE, id, XMPP_MESSAGE_END_ID);
	free(id);
	if(!to)	{ SAppend(4, &outbuff, jid->node, "@", jid->domain); }
	else {
		if(is_groupchat) {
			JID *jid2 = Get_JID_From_String(to);
			if(!jid2->resource) SAppend(2, &outbuff, to);
			else if(jid2->node && jid2->domain) {
				SAppend(4, &outbuff, jid2->node, "@",
					jid2->domain);
			}
			Destruct_JID(jid2);
		} else SAppend(2, &outbuff, to);
	}
	SAppend(2, &outbuff, XMPP_MESSAGE_END_TO);
	
	if(recode_flag)	{
		char *msg = UTF8(message);
		if(msg) {
			SAppend(2, &outbuff, msg);
			if(msg != message) free(msg);
		}
	} else SAppend(2, &outbuff, message);
	SAppend(2, &outbuff, XMPP_MESSAGE_END_BODY_);
	T(outbuff);
	free(outbuff);
}

void XMPP_Send_IQ_Request(JID *jid, char *to, char *type, char *id)
{
	int sock = 0;
	char *outbuff = 0;
	char *from = 0;
	char is_groupchat;
	char sw_id_gen = 0;
	JID *tmpjid;

	if(!jid) return;
	sock = jid->_sockfd;
	if(!sock || !to) return;
	is_groupchat = Check_If_Groupchat(jid, to, 0);
	
	if(is_groupchat && jid->_child->_muc)
		from = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
	else
		from = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);
	if(!id) {
		id = Gen_Id(0);
		sw_id_gen = 1;
	}
	outbuff = SCollect(13, XMPP_IQ_BEGIN, to, XMPP_IQ_END_TO, from,
		XMPP_IQ_END_FROM, id, XMPP_IQ_END_ID, "get",
		XMPP_IQ_END_TYPE, "jabber:iq:", type, XMPP_IQ_END_XMLNS_,
		XMPP_IQ_END_);
	if(sw_id_gen) free(id);
	T(outbuff);
	free(from);
	free(outbuff);
}

void XMPP_Send_IQ_Version_Result(JID *jid, char *to, char *id, char *name, char *version, char *os)
{
	int sock = 0;
	char *outbuff = 0;
	char *from = 0;
	char is_groupchat = 1;
	JID *tmpjid;

	if(!jid) return;
	sock = jid->_sockfd;
	if(!sock || !to) return;
	is_groupchat = Check_If_Groupchat(jid, to, 0);
	
	if(is_groupchat && jid->_child->_muc)
		from = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
	else
		from = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);
	outbuff = SCollect(20, XMPP_IQ_BEGIN, to, XMPP_IQ_END_TO, from,
		XMPP_IQ_END_FROM, id, XMPP_IQ_END_ID, "result",
		XMPP_IQ_END_TYPE, "jabber:iq:version", XMPP_IQ_END_XMLNS,
		XMPP_IQ_VERSION_BEGIN, name, XMPP_IQ_VERSION_END_NAME, version,
		XMPP_IQ_VERSION_END_VERSION, os, XMPP_IQ_VERSION_END_OS_,
		XMPP_IQ_END_QUERY, XMPP_IQ_END_);
	T(outbuff);
	free(from);
	free(outbuff);
}

void XMPP_Send_IQ_Admin(JID *jid, char *to, char *id, char *nick, char *tjid, char *role, char *affiliation, char *reason)
{
	int sock = 0;
	char *outbuff = 0;
	char *from = 0;
	char *item = 0;
	char sw_id_gen = 0;
	char is_groupchat = 1;
	JID *tmpjid;

	char *pair_nick, *pair_tjid, *pair_role, *pair_affiliation, *tag_reason;
	pair_nick = pair_tjid = pair_role = pair_affiliation = tag_reason = 0;

	if(!jid) return;
	sock = jid->_sockfd;
	if(!sock || !to) return;
	is_groupchat = Check_If_Groupchat(jid, to, 0);
	
	if(is_groupchat && jid->_child->_muc)
		from = SCollect(5, jid->_child->node, "@", jid->_child->domain, "/", jid->_child->resource);
	else
		from = SCollect(5, jid->node, "@", jid->domain, "/", jid->resource);

	if(!id) {
		id = Gen_Id(0);
		sw_id_gen = 1;
	}

	if(nick) pair_nick = SCollect(3, " nick=\'", nick, "\'");
	if(tjid) pair_tjid = SCollect(3, " jid=\'", tjid, "\'");
	if(role) pair_role = SCollect(3, " role=\'", role, "\'");
	if(affiliation) pair_affiliation = SCollect(3, " affiliation=\'", affiliation, "\'");
	if(reason) tag_reason = SCollect(3, "<reason>", reason, "</reason>");

	outbuff = SCollect(21, XMPP_IQ_BEGIN, to, XMPP_IQ_END_TO, from,
		XMPP_IQ_END_FROM, id, XMPP_IQ_END_ID, "set",
		XMPP_IQ_END_TYPE, "http://jabber.org/protocol/muc#admin",
		XMPP_IQ_END_XMLNS, XMPP_ITEM_BEGIN, pair_nick, pair_tjid,
		pair_role, pair_affiliation, ">", tag_reason, XMPP_ITEM_END,
		XMPP_IQ_END_QUERY, XMPP_IQ_END_);
	T(outbuff);
	free(from);
	if(sw_id_gen) free(id);
	if(pair_nick) free(pair_nick);
	if(pair_tjid) free(pair_tjid);
	if(pair_role) free(pair_role);
	if(pair_affiliation) free(pair_affiliation);
	if(tag_reason) free(tag_reason);
	free(outbuff);
}

void XMPP_Send_Presence()
{
}

void XMPP_Disconnect(JID *jid)
{
	int sock = jid->_sockfd;
	if(sock) {
		T(XMPP_PRESENCE_LOGOUT);
		close(sock);
	}
	Destruct_JID(jid);
}

XMPP_Msg *Construct_XMPP_Msg(char *buff, ssize_t size)
{
	char *tmp;
	ssize_t end_index = 0;
	XMPP_Msg *msg;

	if(!buff || !size) return 0;
	msg = malloc(sizeof(XMPP_Msg));
	msg->from = XML_Search(buff, size, 0, "message", "from");
	msg->to = XML_Search(buff, size, 0, "message", "to");
	msg->type = XML_Search(buff, size, 0, "message", "type");
	msg->id = XML_Search(buff, size, 0, "message", "id");

	if(XML_Search(buff, size, &end_index, "body", 0))
		msg->body = XML_Get_Content(buff+end_index, size-end_index);
	else msg->body = 0;

	/* Checking for timestamp */
	tmp = XML_Search(buff, size, 0, "x", "stamp");
	if(tmp) {
		msg->stamp = Construct_Timestamp(tmp);
		free(tmp);
	} else msg->stamp = 0;
	
	return msg;
}

void Destruct_XMPP_Msg(XMPP_Msg *msg)
{
	if(!msg) return;
	if(msg->from) free(msg->from);
	if(msg->to) free(msg->to);
	if(msg->type) free(msg->type);
	if(msg->id) free(msg->id);
	if(msg->body) free(msg->body);
	if(msg->stamp) Destruct_Timestamp(msg->stamp);
	free(msg);
}

XMPP_IQ *Construct_XMPP_IQ(char *buff, ssize_t size)
{
	XMPP_IQ *iq;
	char *tmp = 0;
	ssize_t end_index = 0;

	if(!buff || !size) return 0;
	iq = malloc(sizeof(XMPP_IQ));
	iq->from = XML_Search(buff, size, 0, "iq", "from");
	iq->to = XML_Search(buff, size, 0, "iq", "to");
	iq->type = XML_Search(buff, size, 0, "iq", "type");
	iq->id = XML_Search(buff, size, &end_index, "iq", "id");

	buff+=end_index;
	size-=end_index;
	if(tmp = XML_Search(buff, size, &end_index, "query", "xmlns")) {
		char i;
		char *tmp2 = tmp;
		for(i = 0; i < 2; tmp2++) {
			if(tmp2 == '\0') break;
			if(*tmp2 == ':' || *tmp2 == '#') i++;
		}
		if(i == 2) iq->iqns = SCollect(1, tmp2);
		else iq->iqns = 0;
		free(tmp);
	} else {
		iq->iqns = 0;
		iq->query = 0;
	}

	if(iq->iqns) {
		if(!(strcmp(iq->iqns, "version")))
			iq->query = Construct_XMPP_IQ_version(buff+end_index, size-end_index);
		else if(!(strcmp(iq->iqns, "time")))
			iq->query = Construct_XMPP_IQ_time(buff+end_index, size-end_index);
		else if(!(strcmp(iq->iqns, "admin")))
			iq->query = Construct_XMPP_IQ_admin(buff+end_index, size-end_index);
	} else iq->query = 0;

	return iq;
}

void Destruct_XMPP_IQ(XMPP_IQ *iq)
{
	if(!iq) return;
	if(iq->from) free(iq->from);
	if(iq->to) free(iq->to);
	if(iq->type) free(iq->type);
	if(iq->id) free(iq->id);
	if(iq->iqns) {
		if(iq->query) {
			if(!strcmp(iq->iqns, "version"))
				Destruct_XMPP_IQ_version((XMPP_IQ_version *)(iq->query));
			if(!strcmp(iq->iqns, "time"))
				Destruct_XMPP_IQ_time((XMPP_IQ_time *)(iq->query));
			if(!strcmp(iq->iqns, "admin"))
				Destruct_XMPP_IQ_admin((XMPP_IQ_admin *)(iq->query));
		}
		free(iq->iqns);
	}
	free(iq);
}

XMPP_IQ_version *Construct_XMPP_IQ_version(char *buff, ssize_t size)
{
	ssize_t end_index;
	XMPP_IQ_version *ver;

	if(!buff || !size) return 0;
	if(XML_Search_Closer(buff, size, &end_index, "query")) size = end_index;
	else return 0;

	ver = malloc(sizeof(XMPP_IQ_version));
	ver->name = 0;
	ver->version = 0;
	ver->os = 0;

	if(XML_Search(buff, size, &end_index, "name", 0))
		ver->name = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "version", 0))
		ver->version = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "os", 0))
		ver->os = XML_Get_Content(buff+end_index, size-end_index);

	return ver;
}

void Destruct_XMPP_IQ_version(XMPP_IQ_version *ver)
{
	if(!ver) return;
	if(ver->name) free(ver->name);
	if(ver->version) free(ver->version);
	if(ver->os) free(ver->os);
	free(ver);
}

XMPP_IQ_admin *Construct_XMPP_IQ_admin(char *buff, ssize_t size)
{
	ssize_t end_index;
	XMPP_IQ_admin *adm;

	if(!buff || !size) return 0;
	if(XML_Search_Closer(buff, size, &end_index, "query")) size = end_index;
	else return 0;

	adm = malloc(sizeof(XMPP_IQ_admin));
	adm->muc = Construct_MUC_Priv(buff, size);
	adm->reason = 0;
	if(XML_Search(buff, size, &end_index, "reason", 0))
		adm->reason = XML_Get_Content(buff+end_index, size-end_index);

	return adm;
}

void Destruct_XMPP_IQ_admin(XMPP_IQ_admin *adm)
{
	if(!adm) return;
	if(adm->reason) free(adm->reason);
	Destruct_MUC_Priv(adm->muc);
	free(adm);
}

XMPP_IQ_time *Construct_XMPP_IQ_time(char *buff, ssize_t size)
{
	ssize_t end_index;
	XMPP_IQ_time *t;
	char *stampstr = 0;

	if(!buff || !size) return 0;
	if(XML_Search_Closer(buff, size, &end_index, "query")) size = end_index;
	else return 0;

	t = malloc(sizeof(XMPP_IQ_time));
	if(XML_Search(buff, size, &end_index, "utc", 0))
		stampstr = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "tz", 0))
		t->tz = XML_Get_Content(buff+end_index, size-end_index);
	if(XML_Search(buff, size, &end_index, "display", 0))
		t->display = XML_Get_Content(buff+end_index, size-end_index);
	if(stampstr) {
		t->utc = Construct_Timestamp(stampstr);
		free(stampstr);
	}

	return t;
}

void Destruct_XMPP_IQ_time(XMPP_IQ_time *t)
{
	if(!t) return;
	Destruct_Timestamp(t->utc);
	if(t->tz) free(t->tz);
	if(t->display) free(t->display);
	free(t);
}

XMPP_Presence *Construct_XMPP_Presence(char *buff, ssize_t size)
{
	XMPP_Presence *p;
	char *tmp = 0;
	ssize_t end_index = 0;

	if(!buff || !size) return 0;
	p = malloc(sizeof(XMPP_Presence));
	p->from = XML_Search(buff, size, 0, "presence", "from");
	p->to = XML_Search(buff, size, 0, "presence", "to");
	p->type = XML_Search(buff, size, &end_index, "presence", "type");

	buff+=end_index;
	size-=end_index;

	if(XML_Search(buff, size, &end_index, "show", 0)) {
		p->show = XML_Get_Content(buff+end_index, size-end_index);
		if(XML_Search(buff, size, &end_index, "show", 0)) {
			buff+=end_index;
			size-=end_index;
		}
	} else p->show = 0;

	if(XML_Search(buff, size, &end_index, "priority", 0)) {
		tmp = XML_Get_Content(buff+end_index, size-end_index);
		if(tmp) {
			p->priority = atoi(tmp);
			free(tmp);
		}
		if(XML_Search(buff, size, &end_index, "priority", 0)) {
			buff+=end_index;
			size-=end_index;
		}
	} else p->priority = 0;

	p->muc = Construct_MUC_Priv(buff, size);

	return p;
}

void Destruct_XMPP_Presence(XMPP_Presence *p)
{
	if(!p) return;
	if(p->from) free(p->from);
	if(p->to) free(p->to);
	if(p->type) free(p->type);
	if(p->show) free(p->show);
	Destruct_MUC_Priv(p->muc);
	free(p);
}

/* Only this format supported YYYYMMDDThh:mm:ss (T - separator) */
Timestamp *Construct_Timestamp(char *stamp_str)
{
	ssize_t len = strlen(stamp_str);
	Timestamp *t;

	if(!stamp_str) return 0;
	if(len != 17) return 0;
	t = malloc(sizeof(Timestamp));
	t->year = GetSubStr(stamp_str, 0, 3);
	t->month = GetSubStr(stamp_str, 4, 5);
	t->day = GetSubStr(stamp_str, 6, 7);
	t->hour = GetSubStr(stamp_str, 9, 10);
	t->minute = GetSubStr(stamp_str, 12, 13);
	t->second = GetSubStr(stamp_str, 15, 16);

	return t;
}

void Destruct_Timestamp(Timestamp *t)
{
	if(!t) return;
	if(t->year) free(t->year);
	if(t->month) free(t->month);
	if(t->day) free(t->day);
	if(t->hour) free(t->hour);
	if(t->minute) free(t->minute);
	if(t->second) free(t->second);
	free(t);
}

void Destruct_JID(JID *jid)
{
	if(!jid) return;
	if(jid->node) free(jid->node);
	if(jid->domain) free(jid->domain);
	if(jid->resource) free(jid->resource);
	Destruct_MUC_Priv(jid->_muc);
	free(jid);
}

MUC_Priv *Construct_MUC_Priv(char *buff, ssize_t size)
{
	MUC_Priv *mp;
	ssize_t end_index;

	if(!buff || !size) return 0;
	if(XML_Search(buff, size, &end_index, "x", 0)) {
		buff+=end_index;
		size-=end_index;
		mp = malloc(sizeof(MUC_Priv));
		mp->role = XML_Search(buff, size, 0, "item", "role");
		mp->affiliation = XML_Search(buff, size, 0, "item", "affiliation");
		mp->jid = XML_Search(buff, size, 0, "item", "jid");
		mp->nick = XML_Search(buff, size, 0, "item", "nick");
	} else mp = 0;

	return mp;
}

void Destruct_MUC_Priv(MUC_Priv *mp)
{
	if(!mp) return;
	if(mp->role) free(mp->role);
	if(mp->affiliation) free(mp->affiliation);
	if(mp->jid) free(mp->jid);
	if(mp->nick) free(mp->nick);
	free(mp);
}
