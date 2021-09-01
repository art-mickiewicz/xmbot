/*** All event handlers ***/
#ifndef CONVEYERS_H
#define CONVEYERS_H

#include "types.h"
#include "xml-parser.h"

/* Event types */
#define NULL_EVENT 0
#define MSG_EVENT 1
#define IQ_EVENT 2
#define PRESENCE_EVENT 3

typedef (*event_hdl)(JID *, void *);

/* Handles all incoming socket envents
   and calls other handlers for every
   event type. Next level handlers is called
   conveyers*/
void Root_Event_Handler(JID *jid);

/* Modules registers own handlers thru this function */
void Register_Handler(char event_type, event_hdl hdl);

/* Conveyers.
   If hdl is 0 - conveyer will start.
   If hdl is not 0 - no jid needed. */
/* Third argument specific for each function */
void Conveyer_MSG_EVENT(event_hdl hdl, JID *jid,  XMPP_Msg *msg); 
void Conveyer_IQ_EVENT(event_hdl hdl, JID *jid,  XMPP_IQ *iq); 
void Conveyer_PRESENCE_EVENT(event_hdl hdl, JID *jid,  XMPP_Presence *p); 

#endif
