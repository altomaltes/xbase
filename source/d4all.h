/* d4all.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifndef D4ALL_INC
#define D4ALL_INC

#include "d4config.h"

/**********************************************************************/
/**********            USER SWITCH SETTINGS AREA            ***********/

/* CodeBase configuration */

/* Index File compatibility options */

#ifdef S4FOX
   /* FoxPro collating sequence support (select none, some or all) */
   #define S4GENERAL       /* Supports German FoxPro 2.5a and Visual FoxPro with general collating sequences */

/* FoxPro codepage support (select none, some or all) 
 */
   #define S4CODEPAGE_437   /* U.S. MS-DOS CodePage */
   #define S4CODEPAGE_1252  /* WINDOWS ANSI CodePage */
#endif

/* Output selection (alternatives to default) */
/* #define S4CODE_SCREENS */
/* #define S4CONSOLE */

/* Specify Library Type (choose one) */
/* #define S4DLL     */
/* #define S4DLL_BUILD */

/* Error Configuration Options
 */

/* #define E4MAC_ALERT 4444 */

/* Library Reducing Switches
 */
/* #define S4OFF_REPORT   */
/* #define S4OFF_THREAD   */

/**********************************************************************/

#ifdef _MSC_VER
   #if _MSC_VER >= 900
      #pragma pack( push,1)
   #else
      #pragma pack( 1 )
   #endif
#else
   #ifdef __BORLANDC__
      #pragma pack( 1 )
   #endif
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifndef S4WINCE
   #include <stdio.h>
   #include <time.h>
#endif

#ifndef __unix__
   #ifdef S4MACINTOSH
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

  #ifndef TURBOC_STKLEN
    #define TURBOC_STKLEN 10000
  #define  

   extern unsigned _stklen = TURBOC_STKLEN;

#endif  /* __TUROBC__ */


#endif /* D4ALL_INC */
