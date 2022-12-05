/*
 * Author: Kyle Krueger
 * Institution: San Diego State University
 * Course: CS480: Operating Systems
 * Instructor: Dr. Carroll
 * Date: September 22, 2022
 *
 * --------PROGRAM SPECIFICATIONS--------
 * P2 should prompt for input with the character string "%1% "
 * Every line will consist of 0 or more words
 * p2 will skip input lines that have 0 words, and reissue prompt
 * p2 will terminate when it sees an input line whose first word is the EOF marker
 * Metacharacters {>,<,>&, EOF, and trailing &} have special meaning
 * TODO: '<' directs input from right into left
 * '>' directs output from left into right
 * If we attempt to write into an existing file. Print error to stderr "myexistingfile: File exists"
 * TODO: ^^ The same applies for forking
 * '>&' This feeds stderr and stdout to the next file. Should dup2() twice
 * TODO: if the LAST word in the line is '&' p2 will start a new process and execute the command there.
 *      It will also print out the PID of that process in the specific format below. Then continue without waiting for
 *      the completion of the child
 *
 *          ^ The parent will print the argv[0] of the child ("echo), followed by the PID in brackets
 * P2 does not handle signals other than SIGTERM.
 * cd(1) should be included
 *      no parameters, we should do cd $HOME
 *      should only take up to one argument
 *      cd should handle absolute (from home dir)
 *      $HOME can be obtained with
 *  '!!' if it is the first word of the argument should copy the previous command with the subsequent words as
 *
 */
#include "p2.h"


#define BIGSTORAGESIZE  STORAGE * MAXITEM
#define MAXHISTORYSIZE 10
///-CHK Macro-
extern int errno;
#define CHK(x) \
    do{if ((x) == -1) \
        {fprintf(stderr,"In file %s, on line %d: \n",__FILE__,__LINE__); \
               fprintf(stderr,"errno = %d\n",errno);                 \
               exit(1); \
               }       \
   }while(0)
///--Storage Arrays--
char *NEWARGV[MAXITEM]; //Pointers to arguments inside of bigStorage
//char *OLDARGV[MAXITEM];
char *NEWARGVHISTORY[MAXHISTORYSIZE][MAXITEM]; //Stores the history of pointers to commands related to in Bigstoragehistory
char BIGSTORAGEHISTORY[MAXHISTORYSIZE][BIGSTORAGESIZE]; //Stores the history of commands entered by the user
char BIGSTORAGE[BIGSTORAGESIZE]; // Stores words to individual arguments
//char LASTBIGSTORAGE[BIGSTORAGESIZE];
char CWD[STORAGE];
char INDIRECTFILE[STORAGE]; // Will contain the next word after spotting the indirect metachar
char OUTDIRECTFILE[STORAGE];

///--Misc Variables--
int c;
int kidpid;
int size = 0;
//int lastSize = 0;
int sizeHistory[MAXHISTORYSIZE];
int PROCESSEDWORDS = 0;
//int LASTPROCESSEDWORD = 0;
int PROCESSEDWORDSHISTORY[MAXHISTORYSIZE];
char s[STORAGE];//Will store words to be added to bigstorage
int comNum = 0; // Identifier for which command we are on
int pReturn; //<- What parse returns


///--FLAGS--
int CDFLAG = 0;
//int LASTCDFLAG = 0;
int CDFLAGHISTORY[MAXHISTORYSIZE];
int LSFLAG = 0;
//int LASTLSFLAG = 0;
int LSFLAGHISTORY[MAXHISTORYSIZE];
int INDIRECTFLAG = 0;
//int LASTINDIRECTFLAG = 0;
int INDIRECTFLAGHISTORY[MAXHISTORYSIZE];
int OUTDIRECTFLAG = 0;
//int LASTOUTDIRECTFLAG = 0;
int OUTDIRECTFLAGHISTORY[MAXHISTORYSIZE];
int AMPERFLAG = 0;
int BACKGROUNDFLAG = 0;
int DEBUGMODE = 0;
int ARGPROCESSED = 0;
int REPEATEDCOMMAND = 0;
int PIPEFLAG = 0;
int PIPEFLAGHISTORY[MAXHISTORYSIZE];

///-Pipeline Variables-
int filedes[2];
pid_t first, second, pid;

///-Function Identifiers-
int parse();

///Resets current NEWARGV
void resetNEWARGV() {
    //RESETS ALL VARIABLES THAT HAVE TO DO WITH NEWARGV AND ITS OTHER DEPENDENCIES
    int i = 0;
    for (i = 0; i < PROCESSEDWORDS; i++) {
        NEWARGV[i] = NULL;
    }
    PROCESSEDWORDS = 0;
}
///Calls execvp. argvNum determines which element in newargv to start from
void execvpFunct(int argvNum){
    ///Calls execvp, taking in a starting location as input.
    //argvNum should be 0 in any case where all of newargv should be processed. or nonzero in a pipe situation
    if (execvp(NEWARGV[argvNum], NEWARGV) == -1) {
        perror("execvp failed\n");
        exit(-6);
    }
}
///Resets all flags to 0
void resetFlags() {
    //resets all flags
    CDFLAG = 0;
    LSFLAG = 0;
    INDIRECTFLAG = 0;
    OUTDIRECTFLAG = 0;
    AMPERFLAG = 0;
    BACKGROUNDFLAG = 0;
}
///Resets current BIGSTORAGE
void resetBIGSTORAGE() {
    //RESETS ALL VARIABLES THAT HAVE TO DO WITH INDIVIDUAL WORD STORAGE
    int i = 0;
    for (i = 0; i <= size; i++) {
        BIGSTORAGE[i] = (char) NULL;
    }
    for (i = 0; i <= STORAGE; i++) {
        INDIRECTFILE[i] = (char) NULL;
        OUTDIRECTFILE[i] = (char) NULL;
    }

    size = 0;
}
///Resets all current time storage
void resetStorage() {
    //Master reset
    resetNEWARGV();
    resetBIGSTORAGE();
    resetFlags();
}
///Handles sigterm
void sigHandler(int signum) {
    //Handles only sig term (15)
    if (signum == 15) {
        printf("p2 terminated");
        exit(-1);
    } else return;
}
///Copies the passed in command's state
void copyState(int command) {
    //Saves the current command state
    PROCESSEDWORDSHISTORY[command % MAXHISTORYSIZE] = PROCESSEDWORDS;
    strcpy(BIGSTORAGEHISTORY[command % MAXHISTORYSIZE], BIGSTORAGE);
    memmove(NEWARGVHISTORY[command % MAXHISTORYSIZE], NEWARGV, MAXITEM);
    sizeHistory[command % MAXHISTORYSIZE] = size;
    CDFLAGHISTORY[command % MAXHISTORYSIZE] = CDFLAG;
    LSFLAGHISTORY[command % MAXHISTORYSIZE] = LSFLAG;
    INDIRECTFLAGHISTORY[command % MAXHISTORYSIZE] = INDIRECTFLAG;
    OUTDIRECTFLAGHISTORY[command % MAXHISTORYSIZE] = OUTDIRECTFLAG;
}
///Loads the passed in command's state
void loadState(int command) {
    //loads the last command state
    PROCESSEDWORDS = PROCESSEDWORDSHISTORY[command % MAXHISTORYSIZE];
    strcpy(BIGSTORAGE, BIGSTORAGEHISTORY[command % MAXHISTORYSIZE]);
    memmove(NEWARGV, NEWARGVHISTORY[command % MAXHISTORYSIZE], MAXITEM);
    size = sizeHistory[command % MAXHISTORYSIZE];
    CDFLAG = CDFLAGHISTORY[command % MAXHISTORYSIZE];
    LSFLAG = LSFLAGHISTORY[command % MAXHISTORYSIZE];
    INDIRECTFLAG = INDIRECTFLAGHISTORY[command % MAXHISTORYSIZE];
    OUTDIRECTFLAG = OUTDIRECTFLAGHISTORY[command % MAXHISTORYSIZE];
}
///Handles file I/O redirection
void redirectFile(){
    /// - File Input Redirection -
    if (INDIRECTFLAG == 1) {
        int file;
        if ((file = open(INDIRECTFILE, O_RDONLY, 0777)) < 0) {
            fprintf(stderr, "ERROR: UNABLE TO OPEN FILE %s", INDIRECTFILE);
            exit(-2); //TODO: Renumber exits
        }
            //if (DEBUGMODE == 1) printf ("\nFileNum: %d",file);
            //if (DEBUGMODE == 1) printf("\nWe should have an old stdin: %d",stdin);
        else if (dup2(file, STDIN_FILENO) < EXIT_SUCCESS) {
            perror("Dup2 Problem");
            exit(-2);
        } // Redirecting STDIN

        //if (DEBUGMODE == 1) printf("\nWe should have a new stdin: %d",stdin);
        close(file);
        INDIRECTFLAG = 0;

    ///File output redirection
    } else if (OUTDIRECTFLAG == 1) {
        int ofile;
        if (access(OUTDIRECTFILE, F_OK) == 0) fprintf(stderr, "%s: File exists", OUTDIRECTFILE);
        else if ((ofile = open(OUTDIRECTFILE, O_RDWR | O_CREAT, 0777)) < 0) {
            fprintf(stderr, "ERROR: UNABLE TO OPEN FILE %s", OUTDIRECTFILE);
            exit(-3);
        } else if (dup2(ofile, STDOUT_FILENO) < 0) {
            perror("Dup2 problem in OutDirect");
            exit(-4);
        }
        if (AMPERFLAG == 1) {
            printf("We've got a file: %s", OUTDIRECTFILE);
            if (dup2(ofile, STDERR_FILENO) < 0) {
                perror("Dup2 problem in err redirect");
                exit(-5);
            }
        }
        //printf("2\n");
        close(ofile);
        OUTDIRECTFLAG = 0;
        AMPERFLAG = 0;

    }
}
///-Pipe Process-
void pipeFunction(){
    pipe(filedes); // Create the pipe

    //First Child
    CHK(first = fork());
    if (0 == first){
        //First child piping
        //printf("First child reporting\n");
        CHK(dup2(filedes[1],STDOUT_FILENO));
        CHK(close(filedes[0]));
        CHK(close(filedes[1]));
        //First child executing commands
        //printf("First child Should execute commands here / child pid: %d",first);
        execvpFunct(0);

    }
    //Second Child
    CHK(second = fork());
    if(second==0){
        //Second child piping
        //printf("Second Child Reporting\n");
        CHK(dup2(filedes[0],STDIN_FILENO));
        CHK(close(filedes[0]));
        CHK(close(filedes[1]));
        //Second child executing commands
        //printf("Second child Should execute commands here / child pid: %d",second);
        execvpFunct(PIPEFLAG);
    }
    //Parent code here
    //Close the pipe
    CHK(close(filedes[0]));
    CHK(close(filedes[1]));

    for(;;){
        CHK(pid = wait(NULL));
        if (pid == second){
            break;
        }
    }
    //Parent should only be here
    //printf("Parent is done!\n");
    PIPEFLAG = 0;


}
///Handles CD functionality
void cdFlagHandler(){
    //First condition: The only argument is CD
    if (PROCESSEDWORDS == 0) {
        if (DEBUGMODE == 1) printf("We've only got cd\n");
        if (chdir(getenv("HOME")) == -1) fprintf(stderr, "Could not find home");
    }
        //Second condition: There are two arguments, one being CD
    else if (PROCESSEDWORDS == 1) {
        if (chdir(NEWARGV[0]) == -1) {//Checks if it is a full address
            if (chdir(strcat(CWD, NEWARGV[0])) == -1) // Checks if we can branch from our cwd into the new dir
                fprintf(stderr, "The directory: %s does not exist", NEWARGV[1]);
            resetNEWARGV();
        }
    }
        //Fail condition. there are more than 2 arguments
    else {
        if (DEBUGMODE == 1) printf("We done goofed");
        fprintf(stderr, "cd accepts up to 1 argument, %d were found", PROCESSEDWORDS);
    }
    //Print current working dir
    getcwd(CWD, STORAGE);
    if (DEBUGMODE == 1) printf("\n%s\n", CWD);
    CDFLAG = 0; // Lower the flag after processing
}
///Forks a child and executes a command
void executeCommand(){
    fflush(stdout);
    fflush(stderr);
    if (-1 == (kidpid = fork())) {
        perror("Cannot Fork\n");
        exit(-1);
    } else if (kidpid == 0) {
        //printf("1\n");

        ///Executing a single command
        redirectFile();
        execvpFunct(0); // Regular execvp (Only if there is no pipe)

        if (BACKGROUNDFLAG == 1) {
//                if (dup2("/dev/null", STDOUT_FILENO) < 0) {
//                    perror("Dup2 problem in OutDirect");
//                    exit(-7);
            // }
            printf("%s[%ld]\n", NEWARGV[0], (long) getpid());
        }
    }  if (BACKGROUNDFLAG == 0) {
        //We will wait for our background process to finish
//            for(;;){
//                pid_t pid;
//                pid = wait(NULL);
//                if (pid == second){
//                    break;
//                }
//            }
        while (wait(NULL) > 0);
    }
}
///Debugging
void debugPrintNewArgV() {
    printf("%s", NEWARGV[0]);
}
/// Resets dup2's, gets cwd, calls sighandler, prints prompt, and calls parse
void promptAndParse(){
    ///-Housekeeping-
    dup2(STDIN_FILENO, STDIN_FILENO);
    dup2(STDOUT_FILENO, STDOUT_FILENO);
    getcwd(CWD, STORAGE); // Get cwd
    (void) signal(SIGTERM, sigHandler); //Calls the sig handler

    ///-Increment command and print parse-
    comNum++;
    printf("%%%d%% ", comNum);

    ///-Parse and save state-
    pReturn = parse();
    copyState(comNum);
}


int parse() {
    ///--THE PARSING FUNCTIONALITY--
    //Reset all flags, newargv, counters, and bigstorage

    ///-p4 Store previous newargv and bigstorage into history-
//    memmove(BIGSTORAGEHISTORY[comNum%MAXHISTORYSIZE],BIGSTORAGE,BIGSTORAGESIZE);
//    memmove(NEWARGVHISTORY[comNum%MAXHISTORYSIZE],NEWARGV,MAXITEM);
    for (;;) {
        ///--Part one: Taking in a word from getword along with its length-
        c = getword(s);
        if (c == 0) {
            //Check for an ampersand
            if (AMPERFLAG == 1) {
            }
            return 0;
        }  // exit the parse if no words are found on the line
        if (c == -1) return -1; // exit if EOF or end.

        ///-!x Repeat the xth command-
        if (s[0] == '!' && s[1] < 58 && s[1] > 47 && PROCESSEDWORDS == 0) {
            //Note ASCII for 0 is 48 and ASCII for 9 is 57
            //Set newargv and bigstorage to the previous newargv and big storage. then break from the array
            //loadState();
//            memmove(NEWARGV,NEWARGVHISTORY[s[1]%MAXHISTORYSIZE],MAXITEM);
//            memmove(BIGSTORAGE,BIGSTORAGEHISTORY[s[1]%MAXHISTORYSIZE],BIGSTORAGESIZE);
            debugPrintNewArgV();
            loadState(s[1]);
            debugPrintNewArgV();
            break;
        }
            ///-!! Repeat the last command-
        else if (s[0] == '!' && s[1] == '!' && PROCESSEDWORDS == 0) {
            loadState(comNum - 1);
            break;
        }
            ///-!$ Repeat the last word of the last command-
            ///-CD-
        else if (s[0] == 'c' && s[1] == 'd' && c == 2 && PROCESSEDWORDS == 0) {
            CDFLAG = 1;
        }
            ///- < Redirecting input-
        else if (s[0] == '<' && c == 1) { /// - < -
            if (INDIRECTFLAG == 1) fprintf(stderr, "ERROR: Can only redirect one input");
            else {
                INDIRECTFLAG = 1;
                c = getword(s); // Get the next word and check if it is a file name
                memmove(INDIRECTFILE, s, c); // Store the next word as the file name
            }
        }
            ///-> Redirecting Output-
        else if (s[0] == '>') { /// - > - & -
            if (s[1] == '&') AMPERFLAG = 1;
            if (OUTDIRECTFLAG == 1) fprintf(stderr, "ERROR: Can only redirect one output");
            else {
                OUTDIRECTFLAG = 1;
                c = getword(s);
                memmove(OUTDIRECTFILE, s, c);
            }
        }
            /// -| Pipeline-
        else if(s[0] == '|'){
            //Set the pipe flag to where in Newargv our null ptr is, add a null pointer to newargv, continue the loop
            PROCESSEDWORDS++;
            PIPEFLAG = PROCESSEDWORDS;
            NEWARGV[PROCESSEDWORDS] = NULL;
            continue;
        }
            ///- & Background-
        else if (s[0] == '&') {
            //Check to see if the last word is an ampersand
            AMPERFLAG = 1;
        }


            ///--Part two: Processing the words into BIGSTORAGE and setting the pointers in NEWARGV
        else {
            AMPERFLAG = 0; // The last word is no longer an ampersand
            memmove(BIGSTORAGE + size, s, c + 1);
            NEWARGV[PROCESSEDWORDS] = &BIGSTORAGE[size];
            size = size + c + 1; //size says how many chars are in bigstorage
            PROCESSEDWORDS++; // processed words say how many pointers are in newargv

        }


    }
    return 0;

}

int main(int argc, char *argv[]) {
    ///Declarations & setup
    //Necessary setup, including signal catcher and setpgid()
    setpgid(0, 0); //Set the page ID (0,0)
    getcwd(CWD, STORAGE); // Get cwd

    ///--Endless Loop--
    for (;;) {

        ///-Collect Command-
        promptAndParse();

        ///-Checking for exit condition-
        if (pReturn == -1 && size == 0) break;

        ///-FLAGS-
        ///-Pipeline Flag-
        if (PIPEFLAG > 0) pipeFunction();
        ///--CD FLAG--
        if (CDFLAG != 0) cdFlagHandler();

        ///--Single Command Execution--
        if (PIPEFLAG == 0) executeCommand();

        ///Final functions - Copying the contents into the "last" versions
        resetStorage();
    }
    ///-End of p2 Clean and Exit-
    killpg(getpgrp(), SIGTERM);
    exit(0);

}