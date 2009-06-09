/*
 * $Id: mailpart.h 136 2006-09-22 20:06:05Z hubert@u.washington.edu $
 *
 * ========================================================================
 * Copyright 2006 University of Washington
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * ========================================================================
 */

#ifndef PITH_MAILPART_INCLUDED
#define PITH_MAILPART_INCLUDED


#include "../pith/atttype.h"
#include "../pith/state.h"


#define	MIME_MSG(t,s)	((t) == TYPEMESSAGE && (s) && !strucmp((s),"rfc822"))

#define	MIME_DGST(t,s)	((t) == TYPEMULTIPART && (s) && !strucmp((s),"digest"))

/* Is this a message attachment? */
#define	MIME_VCARD(t,s)	((((t) == TYPETEXT || (t) == TYPEAPPLICATION) \
	                   && (s) && !strucmp((s),"DIRECTORY"))       \
			 || ((t) == TYPETEXT                          \
 		   && (s) && !strucmp((s),"X-VCARD")))


#define	MIME_MSG_A(a)	MIME_MSG((a)->body->type, (a)->body->subtype)

/* Is this a digest attachment? */
#define	MIME_DGST_A(a)	MIME_DGST((a)->body->type, (a)->body->subtype)

/* Is this a vCard attachment? */
#define	MIME_VCARD_A(a)	MIME_VCARD((a)->body->type, (a)->body->subtype)


/* exported protoypes */


#endif /* PITH_MAILPART_INCLUDED */
