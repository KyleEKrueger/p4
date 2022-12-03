/* getword.h - header file for the getword() function used in
   CS480 Fall 2022
   San Diego State University
*/

#include <stdio.h>
#include <string.h>
#include <strings.h>

#define STORAGE 255
      /* This is one more than the max wordsize that getword() can handle */

int getword(char *w);
/* (Note: the preceding line is an ANSI C prototype statement for getword().
    It will work fine with edoras' gcc or cc compiler.)

* The getword() function gets one word from the input stream.
* It returns -1 if end-of-file is encountered (or if the word is "end.");
* otherwise, it returns the number of characters in the word
* (but a newline returns 0)
*
* INPUT: a pointer to the beginning of a character string
* OUTPUT: -1 or the number of characters in the word
* SIDE EFFECTS: bytes beginning at address w will be overwritten.
*   Anyone using this routine should have w pointing to an
*   available area at least STORAGE bytes long before calling getword().

Upon return, the string pointed to by w contains the next word in the line
from stdin. A "word" is a string containing one of the following metacharacters,
or a string consisting of non-metacharacters delimited by blanks or metacharacters.

The metacharacters are "<", ">", ">&", ">>", ">>&", "|", and "&".
The last word on a line may be terminated by the newline character OR by
end-of-file.  Word collection is "greedy": getword() always tries to read
the largest word that does not violate the rules.  For example, >>>& is
parsed as ">>" ">&" (rather than ">" ">>&", or ">" ">" ">" "&", etc.)

getword() skips leading blanks, so if getword() is called and there are no
more words on the line, then w points to an empty string. All strings,
including the empty string, will be delimited by a zero-byte (eight 0-bits),
as per the normal C convention (this delimiter is not 'counted' when determining
the length of the string that getword will report as a return value).

The backslash character "\" is special, and may change the behavior of
the character that directly follows it on the input line.  When "\" precedes a
metacharacter, the next metacharacter symbol is treated like most other characters.
(That is, the symbol will be part of a word rather than a word delimiter.)

Thus, three calls applied to the input
Null&void
will return 4,1,4 and produce the strings "Null", "&", "void", respectively.

However, the first call to getword() applied to the input
Null\&void
returns 9 and produces the string "Null&void".
Note that the '\' is NOT part of the resulting string!

Similarly, "\<" is treated as the [non-meta]character "<", "\>" is ">",
"\>>&" becomes TWO words (">" and ">&"), etc.  "\\" represents the (non-special)
character "\".  The combination "\ " should be treated as " ", and therefore
allow a space to be embedded in a word:
Null\ void
returns 9 and produces the string "Null void".
(A backslash preceding any other character should simply be ignored; in
particular, a backslash before a newline will not affect the meaning
of that newline.)

The integer that getword() returns is usually the length of the resultant
string to which w points. There are two exceptions to this:
(1) If the rest of the line consists of zero or more blanks followed by
end-of-file, then w still points to an empty string, but the returned
integer is (-1).
(2) If the word collected is "end.", then w still points to "end.",
but -1 (rather than the expected 4) is returned.

Example: Suppose the input line were
Hi there&
(Assume there are trailing blanks, followed by EOF.)
Four calls to getword(w) would return 2,5,1,-1 and fill each of the
areas pointed to by w with the strings "Hi", "there", "&", and "",
respectively.

Example: Suppose the input line were
Hi there&
(Assume there are two trailing blanks, followed by a newline character.)
Four calls to getword(w) would return 2,5,1,0 and fill each of the
areas pointed to by w with the strings "Hi", "there", "&", and "",
respectively.  (If EOF followed the newline, then a fifth call would
produce "" and return -1.)

Note that we would obtain exactly the same results if the input line had been
    Hi   there  &
(This example has leading blanks and a newline right after the ampersand.)

If the word scanned is longer than STORAGE-1, then getword() constructs the
string consisting of the first STORAGE-1 bytes only. (As usual, a zero-byte
is appended. The next getword() call will begin with the rest of that word.)

Useful manpages to consider are those for ungetc() and getchar().

*/

