#include "xml-parser.h"
#include "tools.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

XML_Tag *XML_Get_Tag(char *buff, ssize_t size, ssize_t *end_index)
{
	XML_Tag *tag;
	char sw = 0;
	char quote = 0;
	char *tag_start; // Points to '<'
	ssize_t tag_size; // As index tag_start points to '>'

	ssize_t starts; // As index of buff points to '<'
	ssize_t i;

	int s; // Shift from begining

	tag = malloc(sizeof(XML_Tag));
	tag->pairs = 0;
	tag->type = WRONG;
	
	if(!buff) return 0;
	if(!size) size = strlen(buff);
	for(i=0; i < size && ! (buff[i]=='>' && sw && !quote); i++)
		if(buff[i] == '<' && !quote) {
			if( i < (size - 3) && !isalnum(buff[i+1]) && isalnum(buff[i+2]) ) {
				if(buff[i+1] == '/') tag->type = CLOSE;
				else if(buff[i+1] == '!') tag->type = DOCTYPE;
				else if(buff[i+1] == '?') tag->type = HEADER;
			} else if( i < (size - 2) && isalnum(buff[i+1])) tag->type = OPEN;
			if(tag->type != WRONG) {
				tag_start = (buff+i);
				starts = i;
				sw = 1;
			}
		} else if(buff[i] == '\"' || buff[i] == '\'') quote = quote?0:1;
	tag_size = i - starts; //Minimal size is 2 (3 x char)
	
	if(end_index) (*end_index) = i;
	if(tag_size < 2 || tag->type == WRONG) return 0;

	if(tag->type == OPEN && tag_start[tag_size-1] == '/') tag->type = AUTOCLOSE;

	tag->tagname = 0;
	i = 1;
	s = (tag->type == CLOSE || tag->type == HEADER || tag->type == DOCTYPE)?1:0; // Where tag name starts
	while(isalnum((tag_start+s)[i]) || (tag_start+s)[i]==':') i++;
	if(i == 1) return 0; // Bad tag name
	tag->tagname = malloc(sizeof(char)*i);
	memcpy(tag->tagname, (tag_start+s+1), i-1);
	tag->tagname[i-1] = '\0';

	if(tag->type == DOCTYPE) ;
	else tag->keyval = XML_Get_KeyVal(tag_start+i+s, tag_size-i-s, &(tag->pairs));

	return tag;
}

KeyVal *XML_Get_KeyVal(char *buff, ssize_t size, ssize_t *counter)
{
	KeyVal *keyval = 0;
	char sw_pair = 0;
	ssize_t i, j;
	*counter = 0;
	
	for(i=0, j=0; i < size; i++) {
		if((buff[i] == '>' || buff[i] == '<') && !sw_pair) break;
		if( (isalnum(buff[i]) || buff[i]==':') && !j) {
			j = i + 1;
		}
		else if(buff[i] == '=' && j && !sw_pair) {
			j--;
			(*counter)++;
			keyval = (KeyVal *)realloc(keyval, sizeof(KeyVal)*(*counter));
			/* (i-j) is quantity of chars in key. +1 is char for '\0' */
			keyval[(*counter)-1].key = (char *)malloc(sizeof(char)*(i-j+1));
			memcpy(keyval[(*counter)-1].key, buff+j, i-j);
			keyval[(*counter)-1].key[i-j] = '\0';
			keyval[(*counter)-1].value = 0;
			j = i + 1;
			sw_pair = 1;
		}
		else if( (buff[i] == '\"' || buff[i] == '\'') && j && j != i && sw_pair && (*counter) > 0) {
			keyval[(*counter)-1].value = (char *)malloc(sizeof(char)*(i-j));
			memcpy(keyval[(*counter)-1].value, buff+j+1, i-j-1);
			keyval[(*counter)-1].value[i-j-1] = '\0';
			j = 0;
			sw_pair = 0;
		}
	}

	if(!(*counter)) return 0;
	return keyval;
}

void Destruct_XML_Tag(XML_Tag *tag)
{
	ssize_t i;

	for(i=0; i<tag->pairs; i++) {
		free(tag->keyval[i].key);
		free(tag->keyval[i].value);
	}
	if(tag->pairs)free(tag->keyval);
	free(tag->tagname);
	free(tag);
}

char *XML_Get_Content(char *buff, ssize_t size)
{
	char *content;
	ssize_t i, j;

	for(i=0, j=0; i < size; i++) {
		if(buff[i]=='>') j=i+1;
		else if(buff[i]=='<') {
			if(!i || i==j) return 0;
			content = (char *)malloc(sizeof(char)*(i-j+1));
			memcpy(content, buff+j, i-j);
			content[i-j] = '\0';
			return content;
		}
	}

	if(i < 1 || buff[i] == '>') return 0;
	content = (char *)malloc(sizeof(char)*(i-j+1));
	memcpy(content, buff+j, i-j);
	content[i-j] = '\0';
	return content;
}

char *XML_Search_Tag(char *buff, ssize_t size, ssize_t *end_index, char *tagname, char *key, char is_closer)
{
	ssize_t ei;
	XML_Tag *tag = 0;
	char sw = 0;
	ssize_t i;

	if(!buff || !tagname)return 0;
	if(key && is_closer)return 0; // Bullshit
	if(end_index) *end_index = 0;

	if(tagname)for(;;) {
		if(tag)Destruct_XML_Tag(tag);
		tag = XML_Get_Tag(buff, size, &ei);
		if(end_index) *end_index+=ei;
		buff += ei;
		size -= ei;
		if(!tag || ( !strcmp(tag->tagname, tagname) && (tag->type == (is_closer)?CLOSE:OPEN || tag->type == (is_closer)?CLOSE:AUTOCLOSE) ) ) {
			break;
		}
	}
	if(tag && !key) {
		Destruct_XML_Tag(tag);
		return 1;
	}
	if(!tag) return 0;

	if(tag && key)for(i=0; i < tag->pairs; i++)
		if(!strcmp(tag->keyval[i].key, key)) {
			sw = 1;
			break;
		}
	if(!sw && key) {
		if(tag)Destruct_XML_Tag(tag);
		return 0;
	}
	if(sw) {
		char *c;
		c = SCollect(1, tag->keyval[i].value);
		Destruct_XML_Tag(tag);
		return c;
	}
}

char XML_Search_Match(char *buff, ssize_t size, ssize_t *end_index, char *tagname, char *key, char *value)
{
	char *tmp;

	if(!buff || !size || !tagname || !key || !value) return 0;
	if(!end_index) {
		ssize_t ei;
		end_index = &ei;
	}
	*end_index = 0;

	for(;;) {
		tmp = XML_Search((buff += *end_index), (size -= *end_index), end_index, tagname, key);
		if(tmp) {
			if(!strcmp(value, tmp)) {
				free(tmp);
				return 1;
			}
		} else if(!size) return 0;
	}
}
