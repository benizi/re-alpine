#if	!defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: word.c 154 2006-09-29 18:00:57Z hubert@u.washington.edu $";
#endif

/*
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

/*
 * Program:	Word at a time routines
 *
 * The routines in this file implement commands that work word at a time.
 * There are all sorts of word mode commands. If I do any sentence and/or
 * paragraph mode commands, they are likely to be put in this file.
 */

#include	"headers.h"


int fpnewline(UCS *quote);


/* Word wrap on n-spaces. Back-over whatever precedes the point on the current
 * line and stop on the first word-break or the beginning of the line. If we
 * reach the beginning of the line, jump back to the end of the word and start
 * a new line.  Otherwise, break the line at the word-break, eat it, and jump
 * back to the end of the word.
 * Returns TRUE on success, FALSE on errors.
 */
int
wrapword(void)
{
    register int cnt;			/* size of word wrapped to next line */
    register int bp;			/* index to wrap on */
    register int first = -1;
    int wid, ww;

    if(curwp->w_doto <= 0)		/* no line to wrap? */
      return(FALSE);

    wid = 0;
    for(bp = cnt = 0; cnt < llength(curwp->w_dotp) && !bp; cnt++){
	if(ucs4_isspace(lgetc(curwp->w_dotp, cnt).c)){
	    first = 0;
	    if(lgetc(curwp->w_dotp, cnt).c == TAB){
	      ++wid;
	      while(wid & 0x07)
		++wid;
	    }
	    else
	      ++wid;
	}
	else{
	    ww = wcellwidth((UCS) lgetc(curwp->w_dotp, cnt).c);
	    wid += (ww >= 0 ? ww : 1);
	    if(!first)
	      first = cnt;
	}

	if(first > 0 && wid > fillcol)
	  bp = first;
    }

    if(!bp)
      return(FALSE);

    /* bp now points to the first character of the next line */
    cnt = curwp->w_doto - bp;
    curwp->w_doto = bp;

    if(!lnewline())			/* break the line */
      return(FALSE);

    /*
     * if there's a line below, it doesn't start with whitespace 
     * and there's room for this line...
     */
    if(!(curbp->b_flag & BFWRAPOPEN)
       && lforw(curwp->w_dotp) != curbp->b_linep 
       && llength(lforw(curwp->w_dotp)) 
       && !ucs4_isspace(lgetc(lforw(curwp->w_dotp), 0).c)
       && (llength(curwp->w_dotp) + llength(lforw(curwp->w_dotp)) < fillcol)){
	gotoeol(0, 1);			/* then pull text up from below */
	if(lgetc(curwp->w_dotp, curwp->w_doto - 1).c != ' ')
	  linsert(1, ' ');

	forwdel(0, 1);
	gotobol(0, 1);
    }

    curbp->b_flag &= ~BFWRAPOPEN;	/* don't open new line next wrap */
					/* restore dot (account for NL)  */
    if(cnt && !forwchar(0, cnt < 0 ? cnt-1 : cnt))
      return(FALSE);

    return(TRUE);
}


/*
 * Move the cursor backward by "n" words. All of the details of motion are
 * performed by the "backchar" and "forwchar" routines. Error if you try to
 * move beyond the buffers.
 */
int
backword(int f, int n)
{
        if (n < 0)
                return (forwword(f, -n));
        if (backchar(FALSE, 1) == FALSE)
                return (FALSE);
        while (n--) {
                while (inword() == FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
                while (inword() != FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
        }
        return (forwchar(FALSE, 1));
}

/*
 * Move the cursor forward by the specified number of words. All of the motion
 * is done by "forwchar". Error if you try and move beyond the buffer's end.
 */
int
forwword(int f, int n)
{
        if (n < 0)
                return (backword(f, -n));
        while (n--) {
#if	NFWORD
                while (inword() != FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
#endif
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
#if	NFWORD == 0
                while (inword() != FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
#endif
        }
	return(TRUE);
}

int
ucs4_isalnum(UCS c)
{
    return((c && c <= 0x7f && isalnum((unsigned char) c))
	   || (c >= 0xA0 && !SPECIAL_SPACE(c)));
}

int
ucs4_isalpha(UCS c)
{
    return((c && c <= 0x7f && isalpha((unsigned char) c))
	   || (c >= 0xA0 && !SPECIAL_SPACE(c)));
}

int
ucs4_isspace(UCS c)
{
    return(c < 0xff && isspace((unsigned char) c) || SPECIAL_SPACE(c));
}

#ifdef	MAYBELATER
/*
 * Move the cursor forward by the specified number of words. As you move,
 * convert any characters to upper case. Error if you try and move beyond the
 * end of the buffer. Bound to "M-U".
 */
int
upperword(int f, int n)
{
        register int    c;
	CELL            ac;

	ac.a = 0;
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        while (n--) {
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
                while (inword() != FALSE) {
                        c = lgetc(curwp->w_dotp, curwp->w_doto).c;
                        if (c>='a' && c<='z') {
                                ac.c = (c -= 'a'-'A');
                                lputc(curwp->w_dotp, curwp->w_doto, ac);
                                lchange(WFHARD);
                        }
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
        }
        return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert characters to lower case. Error if you try and move over the end of
 * the buffer. Bound to "M-L".
 */
int
lowerword(int f, int n)
{
        register int    c;
	CELL            ac;

	ac.a = 0;
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        while (n--) {
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
                while (inword() != FALSE) {
                        c = lgetc(curwp->w_dotp, curwp->w_doto).c;
                        if (c>='A' && c<='Z') {
                                ac.c (c += 'a'-'A');
                                lputc(curwp->w_dotp, curwp->w_doto, ac);
                                lchange(WFHARD);
                        }
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
        }
        return (TRUE);
}

/*
 * Move the cursor forward by the specified number of words. As you move
 * convert the first character of the word to upper case, and subsequent
 * characters to lower case. Error if you try and move past the end of the
 * buffer. Bound to "M-C".
 */
int
capword(int f, int n)
{
        register int    c;
	CELL	        ac;

	ac.a = 0;
	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        while (n--) {
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                }
                if (inword() != FALSE) {
                        c = lgetc(curwp->w_dotp, curwp->w_doto).c;
                        if (c>='a' && c<='z') {
			    ac.c = (c -= 'a'-'A');
			    lputc(curwp->w_dotp, curwp->w_doto, ac);
			    lchange(WFHARD);
                        }
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        while (inword() != FALSE) {
                                c = lgetc(curwp->w_dotp, curwp->w_doto).c;
                                if (c>='A' && c<='Z') {
				    ac.c = (c += 'a'-'A');
				    lputc(curwp->w_dotp, curwp->w_doto, ac);
				    lchange(WFHARD);
                                }
                                if (forwchar(FALSE, 1) == FALSE)
                                        return (FALSE);
                        }
                }
        }
        return (TRUE);
}

/*
 * Kill forward by "n" words. Remember the location of dot. Move forward by
 * the right number of words. Put dot back where it was and issue the kill
 * command for the right number of characters. Bound to "M-D".
 */
int
delfword(int f, int n)
{
        register long   size;
        register LINE   *dotp;
        register int    doto;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        dotp = curwp->w_dotp;
        doto = curwp->w_doto;
        size = 0L;
        while (n--) {
#if	NFWORD
		while (inword() != FALSE) {
			if (forwchar(FALSE,1) == FALSE)
				return(FALSE);
			++size;
		}
#endif
                while (inword() == FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
#if	NFWORD == 0
                while (inword() != FALSE) {
                        if (forwchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
#endif
        }
        curwp->w_dotp = dotp;
        curwp->w_doto = doto;
        return (ldelete(size, kinsert));
}

/*
 * Kill backwards by "n" words. Move backwards by the desired number of words,
 * counting the characters. When dot is finally moved to its resting place,
 * fire off the kill command. Bound to "M-Rubout" and to "M-Backspace".
 */
int
delbword(int f, int n)
{
        register long   size;

	if (curbp->b_mode&MDVIEW)	/* don't allow this command if	*/
		return(rdonly());	/* we are in read only mode	*/
        if (n < 0)
                return (FALSE);
        if (backchar(FALSE, 1) == FALSE)
                return (FALSE);
        size = 0L;
        while (n--) {
                while (inword() == FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
                while (inword() != FALSE) {
                        if (backchar(FALSE, 1) == FALSE)
                                return (FALSE);
                        ++size;
                }
        }
        if (forwchar(FALSE, 1) == FALSE)
                return (FALSE);
        return (ldelete(size, kinsert));
}
#endif	/* MAYBELATER */

/*
 * Return TRUE if the character at dot is a character that is considered to be
 * part of a word. The word character list is hard coded. Should be setable.
 */
int
inword(void)
{
    return(curwp->w_doto < llength(curwp->w_dotp)
	   && ucs4_isalnum(lgetc(curwp->w_dotp, curwp->w_doto).c));
}


/*
 * Return number of quotes if whatever starts the line matches the quote string
 */
int
quote_match(UCS *q, LINE *l, UCS *buf, int buflen)
{
    register int i, n, j, qb;

    *buf = '\0';
    if(*q == '\0')
      return(1);

    qb = (ucs4_strlen(q) > 1 && q[ucs4_strlen(q)-1] == ' ') ? 1 : 0;
    for(n = 0, j = 0; ;){
	for(i = 0; j <= llength(l) && qb ? q[i+1] : q[i]; i++, j++)
	  if(q[i] != lgetc(l, j).c)
	    return(n);

	n++;
	if((!qb && q[i] == '\0') || (qb && q[i+1] == '\0')){
	    if(ucs4_strlen(buf) + ucs4_strlen(q) + 1 < buflen){
		ucs4_strncat(buf, q, buflen-ucs4_strlen(q));
		buf[buflen-1] = '\0';
		if(qb && (j > llength(l) || lgetc(l, j).c != ' '))
		  buf[ucs4_strlen(buf)-1] = '\0';
	    }
	}
	if(j > llength(l))
	  return(n);
	else if(qb && lgetc(l, j).c == ' ')
	  j++;
    }
    return(n);  /* never reached */
}


/* Justify the entire buffer instead of just a paragraph */
int
fillbuf(int f, int n)
{
    int i, lastflagsave;
    LINE *eobline;
    REGION region;

    if(curbp->b_mode&MDVIEW){		/* don't allow this command if	*/
	return(rdonly());		/* we are in read only mode	*/
    }
    else if (fillcol == 0) {		/* no fill column set */
	mlwrite_utf8("No fill column set", NULL);
	return(FALSE);
    }

    if((lastflag & CFFILL) && (lastflag & CFFLBF)){
	/* no use doing a full justify twice */
	thisflag |= (CFFLBF | CFFILL);
	return(TRUE);
    }

    /* record the pointer of the last line */
    if(gotoeob(FALSE, 1) == FALSE)
      return(FALSE);

    eobline = curwp->w_dotp;		/* last line of buffer */
    if(!llength(eobline))
      eobline = lback(eobline);

    /* and back to the beginning of the buffer */
    gotobob(FALSE, 1);

    thisflag |= CFFLBF; /* CFFILL also gets set in fillpara */

    if(!Pmaster)
      sgarbk = TRUE;
    
    curwp->w_flag |= WFMODE;

    /*
     * clear the kill buffer, that's where we'll store undo
     * information, we can't do the fill buffer because
     * fillpara relies on its contents
     */
    kdelete();
    curwp->w_doto = 0;
    getregion(&region, eobline, llength(eobline));

    /* Put full message in the kill buffer for undo */
    if(!ldelete(region.r_size, kinsert))
      return(FALSE);

    /* before yank'ing, clear lastflag so we don't just unjustify */
    lastflag &= ~(CFFLBF | CFFILL);

    /* Now in kill buffer, bring back text to use in fillpara */
    yank(FALSE, 1);

    gotobob(FALSE, 1);

    /* call fillpara until we're at the end of the buffer */
    while(curwp->w_dotp != curbp->b_linep)
      if(!(fillpara(FALSE, 1)))
	return(FALSE);
    
    return(TRUE);
}


/*
 * Fill the current paragraph according to the current fill column
 */
int
fillpara(int f, int n)
{
    int	    i, j, c, qlen, word[NSTRING], same_word,
	    spaces, word_len, word_ind, line_len, qn, ww;
    UCS     line_last;
    UCS    *qstr, qstr2[NSTRING];
    LINE   *eopline;
    REGION  region;

    if(curbp->b_mode&MDVIEW){		/* don't allow this command if	*/
	return(rdonly());		/* we are in read only mode	*/
    }
    else if (fillcol == 0) {		/* no fill column set */
	mlwrite_utf8("No fill column set", NULL);
	return(FALSE);
    }
    else if(curwp->w_dotp == curbp->b_linep) /* don't wrap! */
      return(FALSE);

    /* record the pointer to the line just past the EOP */
    if(gotoeop(FALSE, 1) == FALSE)
      return(FALSE);

    eopline = curwp->w_dotp;		/* first line of para */

    /* and back to the beginning of the paragraph */
    gotobop(FALSE, 1);

    /* determine if we're justifying quoted text or not */
    qstr = (glo_quote_str
	    && quote_match(glo_quote_str, 
			   curwp->w_dotp, qstr2, NSTRING)
	    && *qstr2) ? qstr2 : NULL;
    qlen = qstr ? ucs4_strlen(qstr) : 0;

    /* let yank() know that it may be restoring a paragraph */
    thisflag |= CFFILL;

    if(!Pmaster)
      sgarbk = TRUE;
    
    curwp->w_flag |= WFMODE;

    /* cut the paragraph into our fill buffer */
    fdelete();
    curwp->w_doto = 0;
    getregion(&region, eopline, llength(eopline));
    if(!ldelete(region.r_size, finsert))
      return(FALSE);

    /* Now insert it back wrapped */
    spaces = word_len = word_ind = line_len = same_word = 0;

    /* Beginning with leading quoting... */
    if(qstr){
	i = 0;
	while(qstr[i]){
	  ww = wcellwidth(qstr[i]);
	  line_len += (ww >= 0 ? ww : 1);
	  linsert(1, qstr[i++]);
	}

	line_last = ' ';			/* no word-flush space! */
    }

    /* ...and leading white space */
    for(i = qlen; (c = fremove(i)) == ' ' || c == TAB; i++){
	linsert(1, line_last = c);
	line_len += ((c == TAB) ? (~line_len & 0x07) + 1 : 1);
    }

    /* then digest the rest... */
    while((c = fremove(i++)) > 0){
	switch(c){
	  case '\n' :
	    i += qlen;				/* skip next quote string */
	    if(!spaces)
	      spaces++;
	    same_word = 0;
	    break;

	  case TAB :
	  case ' ' :
	    spaces++;
	    same_word = 0;
	    break;

	  default :
	    if(spaces){				/* flush word? */
		if((line_len - qlen > 0)
		   && line_len + word_len + 1 > fillcol
		   && ((ucs4_isspace(line_last))
		       || (linsert(1, ' ')))
		   && (line_len = fpnewline(qstr)))
		  line_last = ' ';	/* no word-flush space! */

		if(word_len){			/* word to write? */
		    if(line_len && !ucs4_isspace(line_last)){
			linsert(1, ' ');	/* need padding? */
			line_len++;
		    }

		    line_len += word_len;
		    for(j = 0; j < word_ind; j++)
		      linsert(1, line_last = word[j]);

		    if(spaces > 1 && strchr(".?!:;\")", line_last)){
			linsert(2, line_last = ' ');
			line_len += 2;
		    }

		    word_len = word_ind = 0;
		}

		spaces = 0;
	    }

	    if(word_ind + 1 >= NSTRING){
		/* Magic!  Fake that we output a wrapped word */
		if((line_len - qlen > 0) && !same_word++){
		    if(!ucs4_isspace(line_last))
		      linsert(1, ' ');
		    line_len = fpnewline(qstr);
		}

		line_len += word_len;
		for(j = 0; j < word_ind; j++)
		  linsert(1, word[j]);

		word_len = word_ind = 0;
		line_last = ' ';
	    }

	    word[word_ind++] = c;
	    ww = wcellwidth(c);
	    word_len += (ww >= 0 ? ww : 1);

	    break;
	}
    }

    if(word_len){
	if((line_len - qlen > 0) && (line_len + word_len + 1 > fillcol)){
	    if(!ucs4_isspace(line_last))
	      linsert(1, ' ');
	    (void) fpnewline(qstr);
	}
	else if(line_len && !ucs4_isspace(line_last))
	  linsert(1, ' ');

	for(j = 0; j < word_ind; j++)
	  linsert(1, word[j]);
    }

    /* Leave cursor on first char of first line after paragraph */
    curwp->w_dotp = lforw(curwp->w_dotp);
    curwp->w_doto = 0;

    return(TRUE);
}


/*
 * fpnewline - output a fill paragraph newline mindful of quote string
 */
int
fpnewline(UCS *quote)
{
    int len;

    lnewline();
    for(len = 0; quote && *quote; quote++){
	int ww;

	ww = wcellwidth(*quote);
	len += (ww >= 0 ? ww : 1);
	linsert(1, *quote);
    }

    return(len);
}
