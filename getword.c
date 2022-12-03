/*
 * Program: p1
 * Author: Kyle Krueger
 * Instructor Name: Dr. Carroll
 * Institution: San Diego State University
 * Course: CS480
 * Date of completion: September 7th, 2022
 * Due Date: September 12th, 2022
 */
#include <stdio.h>
#include "getword.h"

#define METACHARACTERCOUNT  5

int getword(char *w) {
    ///Declarations
    int iochar;
    char BACKSLASH = 92;
    int charCount = 0;
    int i;
    int inputChar = 0;
    int metaStack[4];
    int metaCount = 0;
    char metaCharacters[METACHARACTERCOUNT] = "<>&|";


    //Metacharacters: <, >, >&, >>, >>&, |, &
    //Backslash: a '\' should consider the next metacharacter as a regular character and treat is as such

    ///Main Loop
    while ((iochar = getchar()) != EOF) {
        ///Overflow Handling
        //Check if there is space for a new char, empty the array if there is not enough storage
        if (charCount == STORAGE - 1) {
            ungetc(iochar, stdin);
            return charCount;
        }

        //If the character count is 0, then null out the array
        if (charCount == 0) {
            //bzero empties the arrays
            bzero(w, STORAGE);
        }
        ///Exit Condition
        //Check if there is an "end" keyword
        if (charCount == 4) {
            if (w[0] == 'e' && w[1] == 'n' && w[2] == 'd' && w[3] == '.') {
                return -1;
            }
        }
        //Check to see if the char is a '\', and skip the special character checks if it is.
        ///Backslash Check
        if (iochar != BACKSLASH) {
            ///Space and Newline Handling
            // Check and see if the leading character is a space, and skip the character if it is
            if ((iochar == ' ' || iochar == '\n')) {
                if (charCount != 0) {
                    //If the character is a space, then return char count
                    ungetc(iochar, stdin);
                    return charCount;
                } else if (iochar == '\n') {
                    return charCount;
                }
                continue;
            }
            ///Metacharacter Handling
            /*
             * Meta characters are handled using an array to store the metacharacters that have been
             * previously seen, and then comparing the last seen meta character to the possible
             * metacharacters that can be created from it.
             */

            //Check to see if the character is a metacharacter
            for (i = 0; i < METACHARACTERCOUNT; i++) {
                if (iochar == metaCharacters[i]) {
                    //check to see if we need to handle returning the previous word
                    if (metaCount > 0 || charCount == 0) {
                        //Checks if there are previously seen metacharacters;
                        if (metaCount == 0) {
                            //If there are no prviously seen metacharacters, we will check if it is a one length metacharacter ('<', '|', or '&')
                            if (iochar == '<' || iochar == '|' || iochar == '&') {
                                //These metacharacters do not have any other metacharacters that can be created from them
                                w[charCount] = iochar;
                                charCount++;
                                return charCount;
                            } else {
                                //These metacharacters could possibly be a longer metacharacter, so we will get the next input.
                                metaStack[0] = iochar;
                                metaCount++;
                                break;
                            }
                        } else if (metaCount == 1) {
                            if ((metaStack[0] == '>' && iochar == '&')) {
                                //This metacharacter is only 2 in length
                                w[charCount] = iochar;
                                charCount++;
                                return charCount;
                            } else if (metaStack[0] == '>' && iochar == '>') {
                                //This metacharacter could be more than 2 in length, check what the next character is.
                                metaStack[1] = iochar;
                                metaCount++;
                                break;
                            } else {
                                //We had a metacharacter that could have been a length of more than one, but was not.
                                ungetc(iochar, stdin);
                                return charCount;
                            }
                        } else if (metaCount == 2) {
                            //The only valid metacharacter here should be the '>>&' metacharacter
                            if (iochar == '&') {
                                w[charCount] = iochar;
                                charCount++;
                                return charCount;
                            } else {
                                ungetc(iochar, stdin);
                                return charCount;
                            }
                        }
                    }
                        //Metacharacter has been found on the tail of a non-metacharacter
                    else {
                        //First unget the found meta character
                        ungetc(iochar, stdin);
                        //Now return the previous word, and the next loop will process the metacharacters
                        return charCount;
                    }
                }
            }
        } else {
            ///Backslash Handling
            //Backslash detected, treat the next character as a normal character no matter what
            iochar = getchar();
            ///Newline Handling
            if (iochar == '\n') {
                if (charCount != 0) {
                    //If the character is a space, then return char count
                    ungetc(iochar, stdin);
                    return charCount;
                }
                return charCount;

            }
            ///Character After a Backslash Handling
            w[charCount] = iochar;
            charCount++;
            continue;
        }
        ///Normal Character Handling
        w[charCount] = iochar;
        charCount++;

    }

}
