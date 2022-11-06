/* d4all.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifndef D4ALL_INC
#define D4ALL_INC

/**********************************************************************/
/**********            USER SWITCH SETTINGS AREA            ***********/

/* CodeBase configuration */
   #define S4STAND_ALONE

/* Index File compatibility options */
/* #define S4CLIPPER */
   #define S4FOX
/* #define S4MDX     */

#ifdef S4FOX
   /* FoxPro collating sequence support (select none, some or all) */
   #define S4GENERAL       /* Supports German FoxPro 2.5a and Visual FoxPro with general collating sequences */

   /* FoxPro codepage support (select none, some or all) */
   #define S4CODEPAGE_437   /* U.S. MS-DOS CodePage */
   #define S4CODEPAGE_1252  /* WINDOWS ANSI CodePage */
#endif

/* Output selection (alternatives to default) */
/* #define S4CODE_SCREENS */
/* #define S4CONSOLE */

/* Specify Library Type (choose one) */
   #define S4STATIC
/* #define S4DLL     */
/* #define S4DLL_BUILD */

/* Choose Operating System */
//   #define S4DOS
/* #define S4OS2        */   /* for OS/2 */
/* #define S4WIN16      */   /* for Windows 3.x */
/* #define S4WINCE      */   /* for Windows CE */
/* #define __unix__       */   /* requires CodeBase Portability version */
/* #define S4MACINTOSH  */   /* requires CodeBase Mac version */
/* #define S4PASCAL_WIN */   /* requires CodePascal version */

/* Alterable CodeBase Global Defines */
#define DEF4SERVER_ID "localhost"
#define DEF4PROCESS_ID "23165"

/* General Configuration Options */
/* #define S4CB51         */
/* #define S4LOCK_HOOK    */
/* #define S4MAX          */
/* #define S4SAFE         */
/* #define S4TIMEOUT_HOOK */

/* Error Configuration Options */
/* #define E4ANALYZE    */
/* #define E4DEBUG      */
/* #define E4HOOK       */
/* #define E4LINK       */
/* #define E4MISC       */
/* #define E4OFF        */
/* #define E4OFF_STRING */
   #define E4PARM_HIGH
   #define E4PAUSE
/* #define E4STOP       */
   #define E4STOP_CRITICAL
/* #define E4MAC_ALERT 4444 */

/* Library Reducing Switches */
/* #define S4OFF_INDEX    */
/* #define S4OFF_MEMO     */
/* #define S4OFF_MULTI    */
/* #define S4OFF_OPTIMIZE */
/* #define S4OFF_REPORT   */
/* #define S4OFF_THREAD   */
/* #define S4OFF_TRAN     */
/* #define S4OFF_WRITE    */

/* Spoken Language Switches */
/* #define S4ANSI         */
/* #define S4DICTIONARY   */
/* #define S4FINNISH      */
/* #define S4FRENCH       */
/* #define S4GERMAN       */
/* #define S4NORWEGIAN    */
/* #define S4SCANDINAVIAN */
/* #define S4SWEDISH      */

/**********************************************************************/

#ifdef _MSC_VER
   #if _MSC_VER >= 900
      #pragma pack(push,1)
   #else
      #pragma pack(1)
   #endif
#else
   #ifdef __BORLANDC__
      #pragma pack(1)
   #endif
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#ifndef S4WINCE
   #include <stdio.h>
   #include <time.h>
#endif

#ifdef __unix__
   #include "p4port.h"
#else
   #ifdef S4MACINTOSH
      #include "p4port.h"
   #else
      #include <stdarg.h>
      #ifndef S4WINCE
         #include <io.h>
      #endif
      #ifdef S4OS2
         #include <os2.h>
         #include <direct.h>
      #else
         #ifndef S4WINCE
            #include <dos.h>
         #endif
      #endif
   #endif
#endif


#ifdef S4WIN16
   #include <windows.h>
#else
   #ifdef __WIN32
      #include <windows.h>
   #else
      #ifdef S4WINCE
         #include <windows.h>
      #endif
   #endif
#endif

#include "d4defs.h"
#include "d4data.h"
#include "d4declar.h"
#include "d4inline.h"
#include "f4flag.h"
#include "e4expr.h"
#include "s4sort.h"
#include "e4string.h"
#include "e4error.h"

#include "o4opt.h"

#include "c4trans.h"

#ifdef OLEDB5BUILD
   #include "oledb5.hpp"
#endif

#include "r4relate.h"

#ifdef S4VBX
   #ifndef S4CONTROLS
   #endif
   #include "ctrl4vbx.h"
#else
   #ifdef S4CONTROLS
      #include "ctrl4.h"
   #endif
#endif

#include "r4report.h"

#ifdef S4CODE_SCREENS
   #include "w4.h"
#endif

#ifdef _MSC_VER
   #if _MSC_VER >= 900
      #pragma pack(pop)
   #else
      #pragma pack()
   #endif
#else
   #ifdef __BORLANDC__
      #pragma pack()
   #endif
#endif


#define S4VERSION 6401

#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */


#endif /* D4ALL_INC */
