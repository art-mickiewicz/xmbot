#include "search.h"
#include "common.h"

#define GOOGLE_START "</table> <p class=g><a class=l href=\""
#define GOOGLE_END_LINK "\">"
#define GOOGLE_END_TITLE "</a><table cellpadding=0 cellspacing=0 border=0><tr><td class=j><font size=-1>"
#define GOOGLE_END_DATA_ "<br>"

void Sulci_Google_Print(JID *jid, XMPP_Msg *msg)
{
	char *body = Body(msg);
	if(strlen(body) > 7 && isspace(body[6])) {
		char *query = body + 7;
		char *out;
		out = Get_Google_Result(query);
		if(out) {
			XMPP_Send_Msg(jid, msg->from, 0, out, 0);
			free(out);
		} else {
			XMPP_Send_Msg(jid, msg->from, 0, "Гугль сломался", 0);
		}
	}
}

char *Get_Google_Result(char *query)
{
	in_addr_t a;
	struct sockaddr_in addr;
	char *w3query;
	char *result;
	char *out;
	char *plainout;
	char *qstr;
	int sock;
	ssize_t size;

	char *link, *title, *data;
	char *n;

	a = Get_Addr("google.com");
	if(!a) return 0;
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) return 0;
	addr.sin_family = PF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = htonl(a);
	if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) return 0;
	w3query = Make_W3_Query(query);
	qstr = SCollect(3, "GET /search?q=", w3query, "&start=0&ie=utf-8&oe=utf-8 HTTP/1.0\n\n");
	free(w3query);
	T(qstr);
	free(qstr);
	result = R(&size, 20);
	if(!result) return 0;

	link = Seek(result, GOOGLE_START, 0);
	if(!link) {
		free(result);
		return 0;
	}
	title = Seek(link, GOOGLE_END_LINK, &n);
	if(!title) {
		free(result);
		return 0;
	}
	*n = '\0';
	data = Seek(title, GOOGLE_END_TITLE, &n);
	if(!data) {
		free(result);
		return 0;
	}
	*n = '\0';
	n = 0;
	Seek(data, GOOGLE_END_DATA_, &n);
	if(!n) {
		free(result);
		return 0;
	}
	*n = '\0';

	out = SCollect(6, "\n*", title, "*\n", link, "\n", data);
	plainout = Data_Filter(out, 1, 1);
	free(out);
	free(result);

	return plainout;
}
