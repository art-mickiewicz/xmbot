/*** Simple XML Parser ***/
#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <sys/types.h>

/** Tag types **/
#define OPEN 1
#define CLOSE 2
#define AUTOCLOSE 3
#define HEADER 4
#define DOCTYPE 5

#define WRONG 0

typedef struct {
	char *key;
	char *value;
} KeyVal;

typedef struct {
	char *tagname;
	KeyVal *keyval;
	ssize_t pairs;
	char type;
} XML_Tag;

/* Parsing first tag from char buffer */
XML_Tag *XML_Get_Tag(char *buff, ssize_t size, ssize_t *end_index);
/* Get key="value" pairs */
KeyVal *XML_Get_KeyVal(char *buff, ssize_t size, ssize_t *counter);
/* Simple function to retrieve text from till '<' or from '>' till '<' */
char *XML_Get_Content(char *buff, ssize_t size);
/* XML_Tag Destructor */
void Destruct_XML_Tag(XML_Tag *tag);
/* XML Search - extra function
	buff, size, end_index like in XML_Get_Tag
	key could be NULL
	tagname could NOT be NULL
	Return value: Value of "key" or NULL
	Return value if "key" is NULL: 1 if tag found, 0 if not found */
char *XML_Search_Tag(char *buff, ssize_t size, ssize_t *end_index, char *tagname, char *key, char is_closer);
#define XML_Search(B, S, E, T, K) XML_Search_Tag(B, S, E, T, K, 0)
#define XML_Search_Closer(B, S, E, T) XML_Search_Tag(B, S, E, T, 0, 1)
/* Wrapper around XML_Search, which returns 1 when matching pair is found,
   if not - return 0 */
char XML_Search_Match(char *buff, ssize_t size, ssize_t *end_index, char *tagname, char *key, char *value);

#endif
