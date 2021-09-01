#include "../modlib.h"

extern char mode_sulci;

char *Body(XMPP_Msg *msg)
{
		if(!mode_sulci || !strncmp(msg->body, "!sulci", 6)) {
			char *body = msg->body;
			body += 7;
			return body;
		}
		return msg->body;
}
