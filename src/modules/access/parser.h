#ifndef ACCESS_PARSER_H
#define ACCESS_PARSER_H

#include "types.h"

Access_List *Read_ACL(char *filename, char *stat);
char **Parse_Line(char *line);
void Add_Context(Access_List *acl, char **fields);
Acts *Get_Acts(char *actstr);

#endif
