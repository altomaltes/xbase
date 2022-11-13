/* e4error.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifdef S4WINTEL
   #ifndef S4WINDOWS
      #include <conio.h>
   #endif  /* not S4WINDOWS */
#endif  /* S4WINTEL */

#ifdef S4OS2PM
   #define  E4MSGBOXID 9513
#endif

#ifdef S4VB_DOS
  #define V4ERROR  1
  #define V4SEVERE 2
#endif

#ifdef S4OS2PM
   #ifndef E4ERROR_OFF
      #ifndef E4OFF_STRING
         extern ERROR4INFO_ARRAY *error4array ;
      #endif
   #endif
#endif
int S4FUNCTION error4file( CODE4 *c4, S4CONST char *name, const int overwrite )
{
   int rc, oldOpenError, oldCreateError, oldSafety ;

   if ( c4->errorLog != 0 )
      return -1 ;

   c4->errorLog = (FILE4 *)u4allocEr( c4, sizeof( FILE4 ) ) ;
   if ( c4->errorLog == 0 )
      return e4memory ;
   c4->errorLog->hand = INVALID4HANDLE ;

   oldSafety = c4->safety ;
   if ( overwrite == 0 )
   {
      oldOpenError = c4->errOpen ;
      c4->errOpen = 0 ;
      rc = file4open( c4->errorLog, c4, name, 1 ) ;
      c4->errOpen = oldOpenError ;
      if ( rc != r4noOpen )  /* if open failed, try create */
         return rc ;
      c4->safety = 1 ; /* file shouldn't exist */
   }
   else
      c4->safety = 0 ; /* ignore setting for overwrite */

   oldCreateError = c4getErrCreate( c4 ) ;
   c4setErrCreate( c4, 0 ) ;
   rc = file4create( c4->errorLog, c4, name, 1 ) ;
   c4setErrCreate( c4, oldCreateError ) ;
   c4->safety = oldSafety ;

   return rc ;
}

static void error4logAppendNewLine( FILE4 *errorLog )
{
   FILE4LONG len ;

   len = file4lenLow( errorLog ) ;
   file4writeInternal( errorLog, len, "\r\n", 2 ) ;
}

void error4logAppend( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{
   int slen ;
   char num[11] ;
   const char *ptr ;
   FILE4 *errorLog ;
   FILE4LONG pos ;

   if ( c4->errorLog == 0 )
      return ;

   if ( c4->errorLog->hand == INVALID4HANDLE )
      return ;

   /* in case one of the file writes fails, avoid endless error loop */
   errorLog = c4->errorLog ;
   c4->errorLog = 0 ;

   memset( num, 0, sizeof( num ) ) ;
   c4ltoa45( (long)errCode1, num, sizeof( num ) - 1 ) ;

   pos = file4lenLow( errorLog ) ;
   file4writeInternal( errorLog, pos, num, strlen( num ) ) ;

   error4logAppendNewLine( errorLog ) ;

   ptr = e4text( errCode1 ) ;
   if ( ptr != 0 )
   {
      pos = file4lenLow( errorLog ) ;
      file4writeInternal( errorLog, pos, ptr, strlen( ptr ) ) ;
      error4logAppendNewLine( errorLog ) ;
   }

   memset( num, 0, sizeof( num ) ) ;
   c4ltoa45( error4number2( errCode2 ), num, sizeof( num ) - 1 ) ;
   pos = file4lenLow( errorLog ) ;
   file4writeInternal( errorLog, pos, num, strlen( num ) ) ;
   error4logAppendNewLine( errorLog ) ;

   #ifndef E4OFF_STRING
      ptr = error4text( c4, errCode2 ) ;
      if ( ptr != 0 )
      {
         pos = file4lenLow( errorLog ) ;
         file4writeInternal( errorLog, pos, ptr, strlen( ptr ) ) ;
         error4logAppendNewLine( errorLog ) ;
      }
   #endif

   #ifdef E4FILE_LINE
      if ( s4fileName != 0 )
      {
         pos = file4lenLow( errorLog ) ;
         file4writeInternal( errorLog, pos, code4fileName(), strlen( code4fileName() ) ) ;
         memset( num, 0, sizeof( num ) ) ;
         c4ltoa45( code4lineNo(), num, sizeof( num ) - 1 ) ;
         pos = file4lenLow( errorLog ) ;
         file4writeInternal( errorLog, pos, num, strlen( num ) ) ;
         error4logAppendNewLine( errorLog ) ;
      }
   #endif

   if ( desc1 != 0 )
   {
      slen = strlen( desc1 ) ;
      pos = file4lenLow( errorLog ) ;
      file4writeInternal( errorLog, pos, desc1, slen ) ;
      error4logAppendNewLine( errorLog ) ;
   }
   if ( desc2 != 0 )
   {
      slen = strlen( desc2 ) ;
      pos = file4lenLow( errorLog ) ;
      file4writeInternal( errorLog, pos, desc2, slen ) ;
      error4logAppendNewLine( errorLog ) ;
   }
   if ( desc3 != 0 )
   {
      slen = strlen( desc3 ) ;
      pos = file4lenLow( errorLog ) ;
      file4writeInternal( errorLog, pos, desc3, slen ) ;
      error4logAppendNewLine( errorLog ) ;
   }
   error4logAppendNewLine( errorLog ) ;
   error4logAppendNewLine( errorLog ) ;

   c4->errorLog = errorLog ;
}

#ifdef S4CONSOLE
#ifdef E4PAUSE
static void error4pause( void )
{
  #ifndef S4WINTEL
      getchar() ;
   #else
      getch() ;
   #endif
}
#endif
#endif

#ifdef S4WINDOWS
   #ifdef __TURBOC__
      #if __TURBOC__ == 0x297     /* if Borland C++ 2.0 */
         #ifdef __cplusplus
            extern "C"{ void FAR PASCAL FatalAppExit(WORD,LPSTR) ; }
         #else
            void FAR PASCAL FatalAppExit(WORD,LPSTR) ;
         #endif  /* __cplusplus */
      #endif  /* __TUROBC__ == 0x297 */
   #endif  /* __TUROBC__ */

   #ifndef __SC__
   #ifdef __ZTC__
      #ifdef __cplusplus
         extern "C"{ void FAR PASCAL FatalAppExit(unsigned short,LPSTR) ; }
      #else
         void FAR PASCAL FatalAppExit(unsigned short,LPSTR) ;
      #endif  /* __cplusplus */
   #endif  /* __ZTC__ */
   #endif  /* __SC__ */

   #ifdef _MSC_VER
      #if _MSC_VER == 600
         #ifdef __cplusplus
            extern "C"{ void FAR PASCAL FatalAppExit(WORD,LPSTR) ; }
         #else
            void FAR PASCAL FatalAppExit(WORD,LPSTR) ;
         #endif  /* __cplusplus */
      #endif  /* _MSC_VER == 600 */
   #endif  /* _MSC_VER */
#endif  /* S4WINDOWS */

#ifndef E4ERROR_OFF
S4CONST char *bad4data = "Invalid or Unknown Error Code" ;

#ifndef S4LANGUAGE
ERROR4DATA e4errorData[] =
{
 /* General Disk Access Errors
 */
  { e4close,          "Closing File" }
 ,{ e4create,         "Creating File" }
 ,{ e4len,            "Determining File Length" }
 ,{ e4lenSet,         "Setting File Length" }
 ,{ e4lock,           "Locking File" }
 ,{ e4open,           "Opening File" }
 ,{ e4permiss,        "Permission Error Opening File" }
 ,{ e4access,         "Access Error Opening File" }
 ,{ e4numFiles,       "File Handle Count Overflow Error Opening File" }
 ,{ e4fileFind,       "File Find Error Opening File" }
 ,{ e4instance,       "Duplicate Instance Found Error Opening File" }
 ,{ e4read,           "Reading File" }
 ,{ e4remove,         "Removing File" }
 ,{ e4rename,         "Renaming File" }
 ,{ e4seek,           "Seeking to File Position" }
 ,{ e4unlock,         "Unlocking File" }
 ,{ e4write,          "Writing to File" }

 /* Database Specific Errors
  */
 ,{ e4data,          "File is not a Data File" }
 ,{ e4fieldName,     "Unrecognized Field Name" }
 ,{ e4fieldType,     "Unrecognized Field Type" }
 ,{ e4recordLen,     "Record Length is too Large" }
 ,{ e4append,        "Record Append Attempt Past End of File" }
 ,{ e4seek,          "Attempt to perform a d4seekDouble() on non-numeric tag" }

/* Index File Specific Errors
 */
 ,{ e4entry,          "Tag Entry Missing" }
 ,{ e4index,          "Not a Correct Index File" }
 ,{ e4tagName,        "Tag Name not Found" }
 ,{ e4unique,         "Unique Key Error" }
 ,{ e4tagInfo,        "Tag Information Invalid" }
 ,{ e4candidate,      "Candidate Key Error" }

 /* Expression Evaluation Errors
 */
 ,{ e4commaExpected, "Comma or Bracket Expected" }
 ,{ e4complete,      "Expression not Complete" }
 ,{ e4dataName,      "Data File Name not Located" }
 ,{ e4lengthErr,     "IIF() Needs Parameters of Same Length" }
 ,{ e4notConstant,   "SUBSTR() and STR() need Constant Parameters" }
 ,{ e4numParms,      "Number of Parameters is Wrong" }
 ,{ e4overflow,      "Overflow while Evaluating Expression" }
 ,{ e4rightMissing,  "Right Bracket Missing" }
 ,{ e4typeSub,       "Sub-expression Type is Wrong" }
 ,{ e4unrecFunction, "Unrecognized Function" }
 ,{ e4unrecOperator, "Unrecognized Operator" }
 ,{ e4unrecValue,    "Unrecognized Value"        }
 ,{ e4unterminated,  "Unterminated String"       }
 ,{ e4tagExpr,       "Expression Invalid for Tag"}

/* Optimization Errors
 */
 ,{ e4opt,            "Optimization Error"               }
 ,{ e4optSuspend,     "Optimization Removal Failure"     }
 ,{ e4optFlush,      "Optimization File Flushing Failure"}

/* Relation Errors
 */
 ,{ e4lookupErr,      "Matching Slave Record Not Located"}
 ,{ e4relate,         "Relation Error"}
 ,{ e4relateRefer,    "Relation Referred to Does Not Exist or is Not Initialized"}

 /* Report Errors
 */
 ,{ e4report,         "Report Error"}
 ,{ e4styleCreate,   "Error Creating Style"}
 ,{ e4styleSelect,   "Error Selecting Style"}
 ,{ e4styleIndex,    "Error Finding Style"}
 ,{ e4areaCreate,    "Error Creating Area"}
 ,{ e4groupCreate,   "Error Creating Group"}
 ,{ e4groupExpr,     "Error Setting Group Reset-Expression"}
 ,{ e4totalCreate,   "Error Creating Total"}
 ,{ e4objCreate,     "Error Creating Object"}
 ,{ e4repWin,        "Error In Windows Output"}
 ,{ e4repOut,        "Error In Report Output"}
 ,{ e4repSave,       "Error Saving Report"}
 ,{ e4repRet,        "Error Retrieving Report"}
 ,{ e4repData,       "Error In Sending Report to Data File"}

 /* Critical Errors
  */
 ,{ e4info,           "Unexpected Information"}
 ,{ e4memory,         "Out of Memory"}
 ,{ e4parm,           "Unexpected Parameter"}
 ,{ e4parmNull,      "Null Input Parameter unexpected"}
 ,{ e4demo,           "Exceeded Maximum Record Number for Demonstration"}
 ,{ e4result,         "Unexpected Result"}
 ,{ e4verify,         "Structure Verification Failure"}
 ,{ e4struct,         "Data Structure Corrupt or not Initialized" }

/* Not Supported Errors
 */
 ,{ e4notIndex,       "Function unsupported: library compiled with S4OFF_INDEX" }
 ,{ e4notMemo,        "Function unsupported: library compiled with S4OFF_MEMO" }
 ,{ e4notRename,      "Function unsupported: library compiled with S4NO_RENAME" }
 ,{ e4notWrite,       "Function unsupported: library compiled with S4OFF_WRITE" }
 ,{ e4notClipper,     "Function unsupported: library not compiled with S4CLIPPER" }
 ,{ e4notLock,        "Function unsupported: library not compiled with S4LOCK_HOOK" }
/* ,{ e4notHook,        "Function unsupported: library not compiled with E4HOOK" } */
 ,{ e4notSupported,   "Function unsupported" }
 ,{ e4version,        "Application/Library version mismatch" }

 /* MEMO errors
  */
 ,{ e4memoCorrupt,    "Memo File Corrupt" }
 ,{ e4memoCreate,     "Error Creating Memo File" }

 /* transaction errors
  */
 ,{ e4transViolation, "Transaction Violation Error" }
 ,{ e4trans,          "Transaction Error" }
 ,{ e4rollback,       "Transaction Rollback Failure" }
 ,{ e4commit,         "Transaction Commit Failure" }
 ,{ e4transAppend,    "Error Appending Information to Log File" }
 ,{ e4transStatus,    "Transaction state confliction" }  /* eg. attempt to commit when no active transaction */

 /* communications errors
  */
 ,{ e4corrupt,        "Communication Information Corrupt" }
 ,{ e4connection,     "Connection Failure" }
 ,{ e4socket,         "Socket Failure" }
 ,{ e4net,            "Network Failure" }
 ,{ e4loadlib,        "Failure Loading Communication DLL" }
 ,{ e4timeOut,        "Network Timed Out" }
 ,{ e4message,        "Communication Message Corrupt" }
 ,{ e4packetLen,      "Communication Packet Length Mismatch" }
 ,{ e4packet,         "Communication Packet Corrupt" }
 ,{ e4connect,        "system-level communication failure" }

 /* miscellaneous errors
  */
 ,{ e4max,            "CodeBase Capabilities Exceeded (system maxed out)" }
 ,{ e4codeBase,       "CodeBase in an Unacknowledged Error State" }
 ,{ e4name,           "Name not Found error" }
 ,{ e4authorize,      "Authorization Error (access denied)" }
 ,{ e4invalidUserId,  "Invalid User ID Authorization Error (access denied)" }
 ,{ e4invalidPassword, "Invalid Password Authorization Error (access denied)" }
 ,{ e4invalidTcpAddress, "Invalid TCP Address access Authorization Error (access denied)" }
 ,{ e4connectDenied,  "Server is not accepting any new connections at this time - contact System Administrator for details" }
 ,{ e4invalidLicence, "LICENCE NOTICE: " }

/* all server-specific error >2100, not only e4server returned to client
 */
 ,{ e4server,         "Server Failure" }
 ,{ e4config,         "Server Configuration Failure" }
/* ,{ e4cat,            "Catalog Failure" }  -- not used currently */

, { 0, 0 }
} ;

#endif  /* not S4LANGUAGE */

#ifdef S4LANGUAGE
#ifdef S4GERMAN

S4CONST char *bad4data = "Invalid or Unknown Error Code" ;

ERROR4DATA e4errorData[] =
{
   /* Allgemeine Fehler beim Diskzugriff  (General Disk Access Errors) */
  { e4create,         "Anlegen einer Datei" }
 ,{ e4open,           "™ffnen einer Datei" }
 ,{ e4read,           "Lesen einer Datei" }
 ,{ e4seek,           "Suchen einer Position in der Datei " }
 ,{ e4write,          "Schreiben einer Datei" }
 ,{ e4close,          "Schlieáen einer Datei" }
 ,{ e4remove,         "L”schen einer Datei" }
 ,{ e4lock,           "Locken einer Datei" }
 ,{ e4unlock,         "Freigeben einer Datei" }
 ,{ e4len,            "Festlegen der L„nge einer Datei" }
 ,{ e4lenSet,        "Einstellen der L„nge einer Datei" }
 ,{ e4rename,         "Umnennen einer Datei" }

   /* Datenbank spezifische Fehler (Database Specific Errors) */
 ,{ e4data,           "Datei is keiner DatenBank" }
 ,{ e4recordLen,     "Datensatzl„nge zu groá" }
 ,{ e4fieldName,     "Unbekannter Feldname" }
 ,{ e4fieldType,     "Feldtyp" }

   /* Indexdatei spezifische Fehler  (Index File Specific Errors) */
 ,{ e4index,          "Datei is keine Indexdatei" }
 ,{ e4entry,          "Indexdatei is veraltet" }
 ,{ e4unique,         "Schulsel ist schon einmal vorhanden" }
 ,{ e4tagName,       "Name des 'Tag'"}

   /* Fehler bei der Bewertung von Ausdrcken   (Expressions Evaluation Errors) */
 ,{ e4commaExpected, "\",\" oder \")\" erwartet" }
 ,{ e4complete,       "Ausdruck ist nich vollst„ndig" }
 ,{ e4dataName,      "Keine offene Datenbank" }
 ,{ e4numParms,      "Ungltige Anzahl von Parametern im Ausdruck"}
 ,{ e4overflow,       "šberlauf bei der Auswertung eines Ausdrucks" }
 ,{ e4rightMissing,  "Rechte Klammer im Ausdruck fehlt" }
 ,{ e4unrecFunction, "Unbekannte Funktion im Ausdruck" }
 ,{ e4unrecOperator, "Unbekannter Operator im Ausdruck" }
 ,{ e4unrecValue,    "Unbekannter Wert im Ausdruck"}
 ,{ e4unterminated,   "Nicht abgeschlossene Zeichenkette im Ausdruck"}

   /* Optimization Errors */
 ,{ e4opt,            "Optimization Error"}   /*!!!GERMAN*/
 ,{ e4optSuspend,     "Optimization Removal Failure"}      /*!!!GERMAN*/
 ,{ e4optFlush,      "Optimization File Flushing Failure"} /*!!!GERMAN*/

   /* Relation Errors */
 ,{ e4lookupErr,     "Matching Slave Record Not Located"}

   /* Kritische Fehler  (Critical Errors) */
 ,{ e4memory,         "Kein Speicher mehr verfgbar"}
 ,{ e4info,           "Unerwartete Information" }
 ,{ e4parm,           "Unerwarteter Parameter"}
 ,{ e4parmNull,      "Null Input Parameter unexpected"}
 ,{ e4demo,           "Exceeded Maximum Record Number for Demonstration"} /*!!!GERMAN*/
 ,{ e4result,         "Unerwartetes Ergebnis"}
   { 0, 0 },
} ;

#endif  /* S4GERMAN  */

#ifdef S4FRENCH

ERROR4DATA e4errorData[] =
{
   /* General Disk Access Errors */
 ,{ e4create,         "En cr‚ant le fichier" }
 ,{ e4open,           "En engageant le fichier" }
 ,{ e4read,           "En lisant le fichier" }
 ,{ e4seek,           "En se pla‡ant dans le fichier" }
 ,{ e4write,          "En ‚crivant dans le fichier" }
 ,{ e4close,          "En lib‚rant le fichier" }
 ,{ e4remove,         "En effa‡ant le fichier" }
 ,{ e4lock,           "En bloquant le fichier" }
 ,{ e4unlock,         "En d‚bloquant le fichier" }
 ,{ e4len,            "En d‚terminant la longueur du fichier" }
 ,{ e4lenSet,        "Mise … jour de la longueur du fichier" }
 ,{ e4rename,         "D‚nomination du fichier" }

   /* Database Specific Errors */
 ,{ e4data,           "Le fichier n'est pas une base de donn‚es:" }
 ,{ e4recordLen,     "La fiche est trop grande" }
 ,{ e4fieldName,     "Champ inconnu" }
 ,{ e4fieldType,     "Type de champ inconnu" }

   /* Index File Specific Errors */
 ,{ e4index,          "Ce n'est pas un fichier d'indice" }
 ,{ e4entry,          "Le fichier d'indice n'est pas … jour" }
 ,{ e4unique,         "La clef n'est pas unique" }
 ,{ e4tagName,       "L'article d‚sign‚ par l'indice n'existe pas" }

   /* Expression Evaluation Errors */
 ,{ e4commaExpected, "\",\" ou \")\" manquant dans l'expression" }
 ,{ e4complete,       "Expression incomplŠte" }
 ,{ e4dataName,      "La base r‚f‚r‚e dans l'expression n'est pas pr‚sente" }
 ,{ e4numParms,      "Nombre ill‚gal de critŠres dans l'expression"}
 ,{ e4overflow,       "L'expression donne un r‚sultat trop grand" }
 ,{ e4rightMissing,  "ParenthŠse manquante dans l'expression" }
 ,{ e4typeSub,       "Un paramŠtre est de la mauvaise sorte" }
 ,{ e4unrecFunction, "L'expression contient une fonction inconnue" }
 ,{ e4unrecOperator, "L'expression contient un op‚rateur inconnu" }
 ,{ e4unrecValue,    "L'expression contient une valeur inconnue"}
 ,{ e4unterminated,   "Apostrophe manquante dans l'expression"}

   /* Optimization Errors */
 ,{ e4opt,            "Optimization Error"}
 ,{ e4optSuspend,     "Optimization Removal Failure"}
 ,{ e4optFlush,      "Optimization File Flushing Failure"}

   /* Relation Errors */
 ,{ e4lookupErr,     "Matching Slave Record Not Located"}

   /* Critical Errors */
 ,{ e4memory,         "Plus de m‚moire disponible" }
 ,{ e4info,           "Information inexpect‚e"}
 ,{ e4parm,           "ParamŠtre inexpect‚"}
 ,{ e4parmNull,      "Null Input Parameter unexpected"}
 ,{ e4demo,           "Au maximum d'articles dans la version de d‚monstration" }
 ,{ e4result,         "R‚sultat inexpect‚"}
   { 0, 0 },
} ;

#endif  /* S4FRENCH */

#ifdef S4SCANDINAVIAN

ERROR4DATA e4errorData[] =
{
   /* General Disk Access Errors */
  { e4create,         "Creating File" }
 ,{ e4open,           "Opening File" }
 ,{ e4read,           "Reading File" }
 ,{ e4seek,           "Seeking to File Position" }
 ,{ e4write,          "Writing to File" }
 ,{ e4close,          "Closing File" }
 ,{ e4remove,         "Removing File" }
 ,{ e4lock,           "Locking File" }
 ,{ e4unlock,         "Unlocking File" }
 ,{ e4len,            "Determining File Length" }
 ,{ e4lenSet,        "Setting File Length" }
 ,{ e4rename,         "Renaming File" }

   /* Database Specific Errors */
 ,{ e4data,           "File is not a Data File" }
 ,{ e4recordLen,     "Record Length is too Large" }
 ,{ e4fieldName,     "Unrecognized Field Name" }
 ,{ e4fieldType,     "Unrecognized Field Type" }

   /* Index File Specific Errors */
 ,{ e4index,          "Not a Correct Index File" }
 ,{ e4entry,          "Tag Entry Missing" }
 ,{ e4unique,         "Unique Key Error" }
 ,{ e4tagName,       "Tag Name not Found" }

   /* Expression Evaluation Errors */
 ,{ e4commaExpected, "Comma or Bracket Expected" }
 ,{ e4complete,       "Expression not Complete" }
 ,{ e4dataName,      "Data File Name not Located" }
 ,{ e4lengthErr,     "IIF() Needs Parameters of Same Length" }
 ,{ e4notConstant,   "SUBSTR() and STR() need Constant Parameters" }
 ,{ e4numParms,      "Number of Parameters is Wrong" }
 ,{ e4overflow,       "Overflow while Evaluating Expression" }
 ,{ e4rightMissing,  "Right Bracket Missing" }
 ,{ e4typeSub,       "Sub-expression Type is Wrong" }
 ,{ e4unrecFunction, "Unrecognized Function" }
 ,{ e4unrecOperator, "Unrecognized Operator" }
 ,{ e4unrecValue,    "Unrecognized Value"}
 ,{ e4unterminated,   "Unterminated String"}

   /* Optimization Errors */
 ,{ e4opt,            "Optimization Error"}
 ,{ e4optSuspend,     "Optimization Removal Failure"}
 ,{ e4optFlush,      "Optimization File Flushing Failure"}

   /* Relation Errors */
 ,{ e4relate,         "Relation Error"}
 ,{ e4lookupErr,     "Matching Slave Record Not Located"}

   /* Report Errors */
 ,{ e4report,         "Report Error"}

   /* Critical Errors */
 ,{ e4memory,         "Out of Memory"}
 ,{ e4info,           "Unexpected Information"}
 ,{ e4parm,           "Unexpected Parameter"}
 ,{ e4parmNull,      "Null Input Parameter unexpected"}
 ,{ e4demo,           "Exceeded Maximum Record Number for Demonstration"}
 ,{ e4result,         "Unexpected Result"}
   { 0, 0 },
} ;
#endif  /* S4SCANDINAVIAN */
#ifdef S4SWEDISH
ERROR4DATA e4errorData[] =
{
   /* General Disk Access Errors */
  { e4create,         "Creating File" }
 ,{ e4open,           "Opening File" }
 ,{ e4read,           "Reading File" }
 ,{ e4seek,           "Seeking to File Position" }
 ,{ e4write,          "Writing to File" }
 ,{ e4close,          "Closing File" }
 ,{ e4remove,         "Removing File" }
 ,{ e4lock,           "Locking File" }
 ,{ e4unlock,         "Unlocking File" }
 ,{ e4len,            "Determining File Length" }
 ,{ e4lenSet,        "Setting File Length" }
 ,{ e4rename,         "Renaming File" }

   /* Database Specific Errors */
 ,{ e4data,           "File is not a Data File" }
 ,{ e4recordLen,     "Record Length is too Large" }
 ,{ e4fieldName,     "Unrecognized Field Name" }
 ,{ e4fieldType,     "Unrecognized Field Type" }

   /* Index File Specific Errors */
 ,{ e4index,          "Not a Correct Index File" }
 ,{ e4entry,          "Tag Entry Missing" }
 ,{ e4unique,         "Unique Key Error" }
 ,{ e4tagName,       "Tag Name not Found" }

   /* Expression Evaluation Errors */
 ,{ e4commaExpected, "Comma or Bracket Expected" }
 ,{ e4complete,       "Expression not Complete" }
 ,{ e4dataName,      "Data File Name not Located" }
 ,{ e4lengthErr,     "IIF() Needs Parameters of Same Length" }
 ,{ e4notConstant,   "SUBSTR() and STR() need Constant Parameters" }
 ,{ e4numParms,      "Number of Parameters is Wrong" }
 ,{ e4overflow,       "Overflow while Evaluating Expression" }
 ,{ e4rightMissing,  "Right Bracket Missing" }
 ,{ e4typeSub,       "Sub-expression Type is Wrong" }
 ,{ e4unrecFunction, "Unrecognized Function" }
 ,{ e4unrecOperator, "Unrecognized Operator" }
 ,{ e4unrecValue,    "Unrecognized Value"}
 ,{ e4unterminated,   "Unterminated String"}

   /* Optimization Errors */
 ,{ e4opt,            "Optimization Error"}
 ,{ e4optSuspend,     "Optimization Removal Failure"}
 ,{ e4optFlush,      "Optimization File Flushing Failure"}

   /* Relation Errors */
 ,{ e4relate,         "Relation Error"}
 ,{ e4lookupErr,     "Matching Slave Record Not Located"}

   /* Report Errors */
 ,{ e4report,         "Report Error"}

   /* Critical Errors */
 ,{ e4memory,         "Out of Memory"}
 ,{ e4info,           "Unexpected Information"}
 ,{ e4parm,           "Unexpected Parameter"}
 ,{ e4parmNull,      "Null Input Parameter unexpected"}
 ,{ e4demo,           "Exceeded Maximum Record Number for Demonstration"}
 ,{ e4result,         "Unexpected Result"}
   { 0, 0 },
} ;
#endif
#endif  /* S4LANGUAGE */
#else
   S4CONST char *bad4data = "Invalid or Unknown Error Code" ;
#endif  /* S4ERROR_OFF */

#ifndef S4INLINE
int S4FUNCTION error4code( CODE4 *c4 )
{
   E4PARMLOW( c4, E96602 ) ;
      return c4->errorCode ;
}
#endif

int S4FUNCTION error4set( CODE4 *c4, const int newErrCode )
{
   int oldErrCode ;

   E4PARMLOW( c4, E96601 ) ;

   oldErrCode = error4code( c4 ) ;
      c4->errorCode = newErrCode ;
   return oldErrCode ;
}

int S4FUNCTION error4set2( CODE4 *c4, const long newErrCode2 )
{
   int oldErrCode2 ;

   E4PARMLOW( c4, E96601 ) ;


   oldErrCode2 = error4code2( c4 ) ;
      c4->errorCode2 = newErrCode2 ;
   return oldErrCode2 ;
}

S4CONST char *e4text( const int errCode )
{
   #ifndef E4ERROR_OFF
      int i ;

      for ( i = 0 ; (int)e4errorData[i].errorNum != 0 ; i++ )
         if ( e4errorData[i].errorNum == errCode )
            return e4errorData[i].errorData ;
   #endif

   return bad4data ;   /* errCode not matched */
}

#ifdef S4OS2PM

void error4out( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{
   int pos = 0 , descNumber = 1 ;
   const char *ptr, *errPtr ;
   HAB  e4hab ;
   HMQ  e4hmq ;
   char errorStr[100] ;
   #ifndef E4ERROR_OFF
      #ifndef E4OFF_STRING
         const char *tPtr ;
      #endif
   #endif

   if ( c4 != 0 )
      error4set( c4, errCode1 ) ;

   #ifndef E4ERROR_OFF
      if ( c4 != 0 )
         if ( c4->errOff )
            return ;

      if (errCode1 != 0)
      {
         strcpy( errorStr, E4_ERROR ) ;
         strcat( errorStr, " #: " ) ;
         c4ltoa45( errCode1, (char *)errorStr+9, 5 ) ;
         pos = 13 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         strcpy( errorStr + pos, E4_ERROR ) ;
         strcat( errorStr + pos, " #: " ) ;
         errorStr[strlen(errorStr)] = ' ' ;
         c4ltoa45( error4number2( errCode2 ), (char *)errorStr + pos + 9, 6 ) ;
         pos += 16 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         errPtr = e4text( errCode1 ) ;
         if ( errPtr != 0 )
         {
            strcpy( errorStr+pos, errPtr ) ;
            pos += strlen( errPtr ) ;
            errorStr[pos++] = '\r\n' ;
         }
      }

      #ifndef E4OFF_STRING
         tPtr = error4text( c4, errCode2 ) ;
         if ( tPtr != 0 )
         {
            strcpy( errorStr + pos, tPtr ) ;
            pos += strlen( tPtr ) ;
            errorStr[pos++] = '\r' ;
            errorStr[pos++] = '\n' ;
         }
      #endif

      ptr = desc1 ;
      while ( (ptr != (char *) 0) && (descNumber <= 3 ) )
      {
         if ( strlen(desc1)+pos+3 >= sizeof(errorStr) )
            break ;
         strcpy( errorStr + pos, ptr ) ;
         pos += strlen(ptr) ;
         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;
         if ( descNumber++ == 1 )
            ptr = desc2 ;
         else
            ptr = desc3 ;
      }

      #ifdef E4FILE_LINE
         if ( code4fileName() != 0 )
            if ( strlen( code4fileName() )+pos+8 < sizeof(errorStr) )
            {
               strcpy( errorStr + pos, code4fileName() ) ;
               pos += strlen( code4fileName() ) ;
               errorStr[pos++] = ' ' ;
               c4ltoa45( code4lineNo(), (char *)errorStr+pos, 6 ) ;
               pos += 5 ;
               errorStr[pos++] = '\r' ;
               errorStr[pos++] = '\n' ;
            }
      #endif

      errorStr[pos] = 0 ;

      /* In case the application has done no PM Initialization, set up an
         instance to allow for the error output to occur */

      e4hab = WinInitialize(0) ;
      if ( e4hab == NULLHANDLE )
         return ;

      e4hmq = WinCreateMsgQueue(e4hab, 0) ;

      if ( e4hmq == NULLHANDLE )
      {
         WinTerminate(e4hab) ;
         return ;
      }

      /* And print out the error via a desktop message box */
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, errorStr, "Error", E4MSGBOXID, MB_OK | MB_MOVEABLE | MB_CUACRITICAL ) ;
      WinDestroyMsgQueue( e4hmq );
      WinTerminate(e4hab) ;
   #endif  /* E4ERROR_OFF */
}

#endif  /* S4OS2PM */

#ifdef S4WINDOWS

/* S4WINDOWS */
void error4out( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{
   #ifndef E4ERROR_OFF
      char errorStr[257] ;
      const char *ptr, *errPtr ;
      #ifdef S4WINCE
         unsigned short errUStr[257] ;
      #endif
   #endif
   int pos=0,  descNumber = 1 ;
   #ifdef S4TESTING
   #else
      #ifndef S4WINCE
         WORD wType ;
      #endif
   #endif
   #ifndef E4ERROR_OFF
      #ifndef E4OFF_STRING
         const char *tPtr ;
      #endif
   #endif

   if ( c4 != 0 )
      error4set( c4, errCode1 ) ;

   #ifndef E4ERROR_OFF
      if ( c4 != 0 )
         if ( c4->errOff )
            return ;

      if (errCode1 != 0)
      {
         memset( errorStr, ' ', sizeof( errorStr ) - 1 ) ;

         strcpy( errorStr, E4_ERROR ) ;
         strcat( errorStr, " #: " ) ;
         c4ltoa45( (long)errCode1, (char *)errorStr+9, 5 ) ;
         pos = 31 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         strcpy( errorStr + pos, E4_ERROR ) ;
         strcat( errorStr + pos, " #: " ) ;
         errorStr[strlen(errorStr)] = ' ' ;
         c4ltoa45( error4number2( errCode2 ), (char *)errorStr + pos + 9, 6 ) ;
         pos += 16 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         errPtr = e4text( errCode1 ) ;
         if ( errPtr != 0 )
         {
            strcpy( errorStr + pos, errPtr ) ;
            pos += strlen( errPtr ) ;
            errorStr[pos++] = '\r' ;
            errorStr[pos++] = '\n' ;
         }

      #ifndef E4OFF_STRING
         tPtr = error4text( c4, errCode2 ) ;
         if ( tPtr != 0 )
         {
            strcpy( errorStr + pos, tPtr ) ;
            pos += strlen( tPtr ) ;
            errorStr[pos++] = '\r' ;
            errorStr[pos++] = '\n' ;
         }
      #endif

      }

      ptr = desc1 ;
      while ( (ptr != (char *) 0) && (descNumber <= 3 ) )
      {
         if ( strlen( desc1 ) + pos + 3 >= sizeof( errorStr ) )
            break ;
         strcpy( errorStr+pos, ptr ) ;
         pos += strlen(ptr) ;
         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;
         if ( descNumber++ == 1 )
            ptr = desc2 ;
         else
            ptr = desc3 ;
      }

      #ifdef E4FILE_LINE
         if ( s4fileName != 0 )
            if ( strlen(s4fileName)+pos+8 < sizeof(errorStr) )
            {
               strcpy( errorStr + pos, s4fileName ) ;
               pos += strlen( s4fileName ) ;
               errorStr[pos++] = ' ' ;
               c4ltoa45( s4lineNo, (char *)errorStr+pos, 6 ) ;
               pos += 6 ;
               errorStr[pos++] = '\r' ;
               errorStr[pos++] = '\n' ;
            }
      #endif

      errorStr[pos] = 0 ;

      #ifdef S4TESTING
         u4writeErr( errorStr, 1 ) ;
      #endif

      #ifndef S4WINCE
         OemToAnsi( errorStr, errorStr ) ;
      #endif

      #ifdef S4TESTING
      #else
         #ifndef S4WINCE
            wType = MB_OK | MB_ICONSTOP ;
            if ( errCode1 == e4memory )
               wType |= MB_SYSTEMMODAL ;
            if ( MessageBox( 0, errorStr, E4_ERROR_COD, wType ) == 0 )
               FatalAppExit( 0, E4_MEMORY_ERR ) ;
         #else
            c4atou(errorStr, errUStr, 257) ;
            MessageBox( 0, (const unsigned short *)errUStr, E4_ERROR_COD, MB_OK | MB_ICONSTOP ) ;
         #endif
      #endif
   #endif
}

#endif  /* S4WINDOWS  */

#ifdef S4VB_DOS

/* S4VB_DOS */
void error4out( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{

}
#endif /* S4VB_DOS */


#ifndef S4CONSOLE
#ifdef S4MACINTOSH
/* S4MACINTOSH */
void error4out( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{
   #ifdef S4TESTING
      char errorStr[257] ;
      const char *errPtr ;
      int pos=0 ;
   #endif
   #ifndef E4ERROR_OFF
      #ifndef E4OFF_STRING
         char const *tPtr ;
      #endif
   #endif
   char const * ptr ;
   int descNumber = 1 ;
   Str255 macStr, macStra="\p", macStrb="\p", macStrc="\p";
   short itemHit ;
   AlertTHndl hALRT ;
   AlertTPtr  pALRT ;

   if ( c4 != 0 )
      error4set( c4, errCode1 ) ;

   #ifndef E4ERROR_OFF
      if ( c4 != 0 )
         if ( c4->errOff )
            return ;

      if (errCode1 != 0)
      {
         sprintf( (char *)&macStr, "  %d, %ld \r", errCode1, error4number2( errCode2 ) ) ;
         ptr = e4text( errCode1 ) ;
         if ( ptr != 0 )
            sprintf( (char *)&macStr + strlen((char *)macStr), "%s\r",e4text( errCode1 ) ) ;
         #ifndef E4OFF_STRING
            tPtr = error4text( c4, errCode2 ) ;
            if ( tPtr != 0 )
               sprintf( (char *)&macStr+strlen((char *)macStr), "%s",error4text( c4, errCode2 ) ) ;
         #endif
         CtoPstr( (char *)&macStr );  /* convert C string to Pascal string */
      }

      if (desc1 != (char*)0 )
      {
         strcpy((char *)&macStra, desc1 );
         CtoPstr((char *)&macStra) ;
      }
      #ifndef E4FILE_LINE
         if (desc2 != (char*)0 )
         {
            strcpy((char *)&macStrb, desc2 );
            CtoPstr((char *)&macStrb) ;
         }

         if (desc3 != (char*)0 )
         {
            strcpy((char *)&macStrc, desc3 );
            CtoPstr((char *)&macStrc) ;
         }
      #else     /*since we can only support 4 lines total, we'll have to combine two*/
         if (desc2 != (char*)0 )
         {
            strcpy((char *)&macStrb, desc2 );
            if (desc3 != (char*)0 )
            {
                strcat((char *)&macStrb,"\r");
                strcat((char *)&macStrb,"desc3");
            }
            CtoPstr((char *)&macStrb) ;
         }
         else if (desc3 != (char*)0 )
         {
            strcpy((char *)&macStrb,"desc3");
            CtoPstr((char *)&macStrb) ;
         }

         if ( s4fileName != 0 )
         {
            sprintf( (char *)&macStrc, "File: %s Line:%d\r\r\r", s4fileName, s4lineNo ) ;
            CtoPstr( (char *)&macStrc );  /* convert C string to Pascal string */
         }
      #endif
      ParamText(macStr, macStra, macStrb, macStrc ) ;
      itemHit = StopAlert( E4MAC_ALERT, 0 ) ;
      ParamText(0,0,0,0);


   #endif  /* E4ERROR_OFF */
}
#endif
#endif


#ifdef S4CONSOLE

/* S4CONSOLE */
void error4out( CODE4 *c4, int errCode1, long errCode2, const char *desc1, const char *desc2, const char *desc3 )
{
   #ifdef S4TESTING
      char errorStr[257] ;
      const char *errPtr ;
      int pos=0 ;
   #endif
   #ifndef E4ERROR_OFF
      #ifndef E4OFF_STRING
         char const *tPtr ;
      #endif
   #endif
   char const * ptr ;
   int descNumber = 1 ;

   if ( c4 != 0 )
      error4set( c4, errCode1 ) ;

   #ifndef E4ERROR_OFF
      if ( c4 != 0 )
         if ( c4->errOff )
            return ;

   #ifdef S4TESTING
      if (errCode1 != 0)
      {
         memset( errorStr, ' ', sizeof( errorStr ) - 1 ) ;

         strcpy( errorStr, E4_ERROR ) ;
         strcat( errorStr, " #: " ) ;
         c4ltoa45( (long)errCode1, (char *)errorStr+9, 5 ) ;
         pos = 31 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         strcpy( errorStr + pos, E4_ERROR ) ;
         strcat( errorStr + pos, " #: " ) ;
         errorStr[strlen(errorStr)] = ' ' ;
         c4ltoa45( error4number2( errCode2 ), (char *)errorStr + pos + 9, 6 ) ;
         pos += 16 ;

         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;

         errPtr = e4text( errCode1 ) ;
         if ( errPtr != 0 )
         {
            strcpy( errorStr + pos, errPtr ) ;
            pos += strlen( errPtr ) ;
            errorStr[pos++] = '\r' ;
            errorStr[pos++] = '\n' ;
         }

      #ifndef E4OFF_STRING
         tPtr = error4text( c4, errCode2 ) ;
         if ( tPtr != 0 )
         {
            strcpy( errorStr + pos, tPtr ) ;
            pos += strlen( tPtr ) ;
            errorStr[pos++] = '\r' ;
            errorStr[pos++] = '\n' ;
         }
      #endif

      }

      ptr = desc1 ;
      while ( (ptr != (char *) 0) && (descNumber <= 3 ) )
      {
         if ( strlen( desc1 ) + pos + 3 >= sizeof( errorStr ) )
            break ;
         strcpy( errorStr+pos, ptr ) ;
         pos += strlen(ptr) ;
         errorStr[pos++] = '\r' ;
         errorStr[pos++] = '\n' ;
         if ( descNumber++ == 1 )
            ptr = desc2 ;
         else
            ptr = desc3 ;
      }

      #ifdef E4FILE_LINE
         if ( s4fileName != 0 )
            if ( strlen(s4fileName)+pos+8 < sizeof(errorStr) )
            {
               strcpy( errorStr + pos, s4fileName ) ;
               pos += strlen( s4fileName ) ;
               errorStr[pos++] = ' ' ;
               c4ltoa45( s4lineNo, (char *)errorStr+pos, 6 ) ;
               pos += 6 ;
               errorStr[pos++] = '\r' ;
               errorStr[pos++] = '\n' ;
            }
      #endif

      errorStr[pos] = 0 ;

      u4writeErr( errorStr, 1 ) ;
   #else
      if (errCode1 != 0)
      {
         fprintf( stderr, E4_ERROR_NUM ) ;
         fprintf( stderr, "  %d, %ld \r\n", errCode1, error4number2( errCode2 ) ) ;
         ptr = e4text( errCode1 ) ;
         if ( ptr != 0 )
            fprintf( stderr, "%s\r\n",e4text( errCode1 ) ) ;
         #ifndef E4OFF_STRING
            tPtr = error4text( c4, errCode2 ) ;
            if ( tPtr != 0 )
               fprintf( stderr, "%s\r\n",error4text( c4, errCode2 ) ) ;
         #endif
      }

      ptr = desc1 ;
      while ( (ptr != (char *) 0) && (descNumber <= 3 ) )
      {
         fprintf( stderr, "\r\r\n" ) ;
         fprintf( stderr, ptr ) ;
         if ( descNumber++ == 1 )
            ptr = desc2 ;
         else
            ptr = desc3 ;
      }

      fprintf( stderr, "\r\r\n" ) ;
      #ifdef E4FILE_LINE
         if ( s4fileName != 0 )
            fprintf( stderr, "File: %s Line:%d\r\r\n", s4fileName, s4lineNo ) ;
      #endif
   #endif  /* S4TESTING */

   #endif  /* E4ERROR_OFF */
}
#endif

#ifdef E4FILE_LINE
   /* use globals to hold the current file name and line number */
   const char *s4fileName = 0 ;
   int s4lineNo = 0 ;

   int  S4FUNCTION code4lineNo( void )
   {
      return s4lineNo ;
   }

   const char *S4FUNCTION code4fileName( void )
   {
      return s4fileName ;
   }

   void S4FUNCTION code4lineNoSet( int val )
   {
      s4lineNo = val ;
   }

   void S4FUNCTION code4fileNameSet( const char *ptr )
   {
      s4fileName = ptr ;
   }
#endif

#ifndef E4FILE_LINE
/*int S4FUNCTION e4( CODE4 *c4, int errCode, const char *desc )*/
/*{*/
/*   return error4describe( c4, errCode, 0L, desc, 0, 0 ) ;*/
/*}*/
#endif

#ifdef E4STACK
int S4FUNCTION error4stackDefault( CODE4 *c4, const int errCode1, const long errCode2 )
{
   return error4describeExecute( c4, errCode1, errCode2, 0, 0, 0 ) ;
}
#endif

int S4FUNCTION error4default( CODE4 *c4, const int errCode1, const long errCode2 )
{
   return error4describeExecute( c4, errCode1, errCode2, 0, 0, 0 ) ;
}

int S4FUNCTION error4describeDefault( CODE4 *c4, const int errCode1, const long errCode2, const char *s1, const char *s2, const char *s3 )
{
   return error4describeExecute( c4, errCode1, errCode2, s1, s2, s3 ) ;
}

int S4FUNCTION error4describeExecute( CODE4 *c4
                                    , const int errCode1, const long errCode2
                                    , const char *s1, const char *s2, const char *s3 )
{
   #ifdef E4HOOK
         if ( c4 != 0 )
         {
            error4set(  c4, errCode1 );
            error4set2( c4, errCode2 );
         }

      error4hook( c4, errCode1, errCode2, s1, s2, s3 ) ;
      return errCode1 ;
   #else
      /* display error local to operating system */
      if ( c4 != 0 )
         if ( c4->errorLog != 0 )
            if ( c4->errorLog->hand != INVALID4HANDLE )
               error4logAppend( c4, errCode1, errCode2, s1, s2, s3 ) ;
      error4out( c4, errCode1, errCode2, s1, s2, s3 ) ;
      if ( c4 != 0 )
      {
         #ifndef E4ERROR_OFF
            #ifdef S4CONSOLE
               #ifdef E4PAUSE
                  if ( c4->errOff == 0 )
                     error4pause() ;
               #endif
            #endif
         #endif
         error4set(  c4, errCode1 ) ;
         error4set2( c4, errCode2 ) ;
      }

      #ifdef E4STOP
         code4exit( c4 ) ;
      #endif

      #ifdef E4STOP_CRITICAL
         if ( c4 == 0 )
            code4exit( c4 ) ;
         switch ( errCode1 )
         {
            case e4info:
            case e4memory:
            case e4parm:
            case e4parmNull:
            case e4result:
            case e4struct:
               code4exit( c4 ) ;
         }
      #endif

      if ( errCode1 == e4demo )
         code4exit( c4 ) ;

      return errCode1 ;
   #endif
}

void S4FUNCTION error4exitTest( CODE4 *c4 )
{
   if ( c4 == 0 )
      return ;
   if ( error4code( c4 ) < 0 )
      code4exit( c4 ) ;
}

