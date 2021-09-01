/*** Some tools... ***/
#ifndef TOOLS_H
#define TOOLS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "types.h"

#define ATTEMP_LIMIT 200
#define myerror(e) fprintf(stderr, "ERROR: %s\n", e)

/* Transmits all data from msg */
char Transmit(int sock, char *msg, ssize_t size); 
/* Receives all data to (char *) */
char *Receive(int sock, ssize_t *size, int timeout);

/* Generate ID for XMPP tag */
char *Gen_Id(int seed);

/* Convert text to utf8 from encoding, specified by
   global variable "src_encoding" and back if in_utf is 1*/
char *UTF8_Convert(char *in, char in_utf);
/* Back convert */
#define UTF8(A) UTF8_Convert(A, 0)
#define fromUTF8(A) UTF8_Convert(A, 1)

/* Allocate memory and collect "num" strings into one */
char *SCollect(unsigned char num, ...);
/* Second arg is target */
void SAppend(unsigned char num, char **target, ...);

/* Allocate memory and return substring from start to end */
char *GetSubStr(char *in, ssize_t start, ssize_t end);

/** String Resolve Functions (Time) **/
#define VAR_NULL 0
#define VAR_YEAR 1
#define VAR_MONTH 2
#define VAR_DAY 3
#define VAR_HOUR 4
#define VAR_MINUTE 5
#define VAR_SECOND 6
/* Main function: returns resolved string with subtituded
   time variables */
char *Resolve_Time(char *f);
/* Read %-string and find apropos type of time var */
char Get_Time_Var_Type(char *buff, ssize_t *end_index);
/* Return time variable by type */
char *Get_Time_Var(char type);

/** String Resolve Functions (Room and nick) **/
#define VAR_ROOM 1
#define VAR_NICK 2
/* Main function: returns resolved string with subtituded
   room and nick variables */
char *Resolve_RoomNick(char *prefix, JID *jid, char *from);
/* Read %-string and find apropos type of var */
char Get_RoomNick_Var_Type(char *buff, ssize_t *end_index);
/* Returns nick or room by type */
char *Get_RoomNick_Var(char type, JID *jid, char *from);

JID *Get_JID_From_String(char *s);

/* Check(by mask) if JID should to log.
   0 - if should not.
   else 1 */
char Check_JID_Mask(char *jidmask, char *jid);

char Check_If_Groupchat(JID *primjid, char *jidstr, char **nick);
/* Returns 1 if context is groupchat
   if nick not NULL - memory allocated and nick returned
   and MUST BE FREED */

char alien_msg(JID *jid, XMPP_Msg *msg);
char alien_iq(JID *jid, XMPP_IQ *iq);
char alien_presence(JID *jid, XMPP_Presence *p);

in_addr_t Get_Addr(char *host);

/* Searches for "mark" in "buff" and returns
   pointer to place just after it */
char *Seek(char *buff, char *mark, char **start);

/* Remove all tag constructions and/or convert spec symbols */
char *Data_Filter(char *buff, char sw_fchar, char sw_ftag);

/* Making data apropriate for web query */
char *Make_W3_Query(char *buff);

#endif
