/* d4defs.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifndef D4DEFS_INC
#define D4DEFS_INC

#ifdef S4WINCE
   #define __WIN32
#endif

#ifdef S4OFF_MULTI
   #ifdef OLEDB5BUILD
      /* ole-db requires multi-user */
      #undef OLEDB5BUILD
   #endif
#endif

#ifdef OLEDB5BUILD
   #ifndef __cplusplus
      #error must build OLEDB as C++ (compiler build says compile is C, not C++)
   #endif
   #ifdef S4STATIC
      #undef S4STATIC
   #endif
   #ifdef S4DOS
      #undef S4DOS
   #endif
   #ifndef __WIN32
      #define __WIN32
      #include <windows.h>
   #endif
      #ifndef S4CONSOLE
         #define S4CONSOLE
      #endif
   #ifndef S4DLL
      #define S4DLL
   #endif
   #define S4OLEDB_OR_NOT_SERVER
#endif

   #ifndef S4OLEDB_OR_NOT_SERVER
      #define S4OLEDB_OR_NOT_SERVER
   #endif

#ifndef S4DLL_BUILD
   #ifndef S4LIB_BUILD
      #define S4EXE_BUILD
   #endif
#endif

#ifdef __BORLANDC__
   #if __BORLANDC__ < 0x500
      #ifndef S4OFF_THREAD
         #define S4OFF_THREAD
      #endif
   #endif
#endif

#ifdef _MSC_VER
   #if _MSC_VER < 1000
      #ifndef S4OFF_THREAD
         #define S4OFF_THREAD
      #endif
   #endif
#endif

#ifdef __cplusplus
   /* only use CodeBase++ ::true() functions on older compilers
      otherwise use ::isTrue() replacment */

   #ifdef __BORLANDC__
      #if __BORLANDC__ < 0x500
         #define S4USE_TRUE
      #endif
   #endif

   #ifdef _MSC_VER
      #if _MSC_VER < 1010
         #define S4USE_TRUE
      #endif
   #endif
#endif

/*
#ifdef _MSC_VER
   #if _MSC_VER > 999
      #define S4USE_INT_DELAY
   #endif
#endif
*/

#ifndef S4INLINE
   #define S4INLINE
#endif

#ifndef __unix__
   #ifndef S4MACINTOSH
      #ifndef S4OS2
         #ifndef _MSC_VER
            #define P4ARGS_USED
         #endif
      #endif
   #endif
#endif

#ifdef _MSC_VER
   #ifdef _WIN32
      #ifndef __WIN32
         #error Must compile with __WIN32 defined
      #endif
   #else
      #ifdef __WIN32
         #error Can not compile with __WIN32 defined
      #endif
   #endif
#endif

#ifdef __TURBOC__
   #ifdef __WIN32__
      #ifndef __WIN32
         #error Must compile with __WIN32 defined
      #endif
   #endif
#endif

/* catalog files not supported in version 6.0 */
#define S4OFF_CATALOG


#ifdef S4UTILS
   #ifndef S4OFF_ENFORCE_LOCK
      #define S4OFF_ENFORCE_LOCK
   #endif
#endif

/* equivalency defines should go first... */

#ifdef S4OFF_INDEX
   #ifndef S4INDEX_OFF
      #define S4INDEX_OFF
   #endif
#endif

#ifdef S4INDEX_OFF
   #ifndef S4OFF_INDEX
      #define S4OFF_INDEX
   #endif
#endif

#ifdef S4OPTIMIZE_OFF
   #ifndef S4OFF_OPTIMIZE
      #define S4OFF_OPTIMIZE
   #endif
#endif

#ifdef S4OFF_OPTIMIZE
   #ifndef S4OPTIMIZE_OFF
      #define S4OPTIMIZE_OFF
   #endif
#endif

#ifdef S4OFF_MEMO
   #ifndef S4MEMO_OFF
      #define S4MEMO_OFF
   #endif
#endif

#ifdef S4MEMO_OFF
   #ifndef S4OFF_MEMO
      #define S4OFF_MEMO
   #endif
#endif

#ifdef S4OFF_MULTI
   #ifndef S4SINGLE
      #define S4SINGLE
   #endif
#endif

#ifdef S4SINGLE
   #ifndef S4OFF_MULTI
      #define S4OFF_MULTI
   #endif
#endif

/* end of equivalency defines */

#ifndef S4DISTRIBUTED
   #define S4DISTRIBUTED
#endif

   #ifndef S4OFF_CATALOG
      #define S4OFF_CATALOG
   #endif
   #ifndef S4OFF_SECURITY
      #define S4OFF_SECURITY
   #endif

#ifdef S4OFF_WRITE
   #ifndef S4OFF_TRAN
      #define S4OFF_TRAN
   #endif
#endif

#ifdef S4FOX
      #define CLIENT_OR_FOX
#endif

   #ifndef S4MDX
      #ifndef S4FOX
         #ifndef S4CLIPPER
            #ifndef S4NDX
               #error - Must compile with one of the indexing options (S4MDX, S4FOX, OR S4CLIPPER)
            #endif
         #endif
      #endif
   #endif

#ifndef S4WIN16
   #ifndef __WIN32
      #ifndef S4OS2
         #ifndef __unix__
            #ifndef S4MACINTOSH
               #ifndef S4PASCAL_WIN
                  #ifndef S4DOS
                     #error NO OPERATING SYSTEM SELECTED (S4WIN16/__WIN32/S4DOS/__unix__/...)
                  #endif
               #endif
            #endif
         #endif
      #endif
   #endif
#endif

#ifndef __unix__
  #ifndef S4MACINTOSH
     #define S4WINTEL   /*This will include DOS and OS2 also */
  #endif
#endif

#ifdef S4WIN16
   #ifndef S4WINDOWS
      #define S4WINDOWS
   #endif
#endif

#ifdef __WIN32
   #ifndef S4CONSOLE
      #ifndef S4WINDOWS
         #define S4WINDOWS
      #endif
   #endif
   #ifdef __BORLANDC__
      /* borland's atof() causes problems... */
      #define S4NO_ATOF
   #endif
   #ifndef S4SEMAPHORE
      #define S4SEMAPHORE
   #endif
   #ifdef S4DLL
      #ifndef S4OFF_THREAD
         #ifndef S4OFF_WRITE_DELAY
            #ifndef S4WRITE_DELAY
               #define S4WRITE_DELAY
            #endif
         #endif
         #ifndef S4OFF_READ_ADVANCE
            #ifndef S4READ_ADVANCE
               #define S4READ_ADVANCE
            #endif
         #endif
      #endif
   #endif
   #ifdef S4DLL_BUILD
      #ifndef S4OFF_THREAD
         #ifndef S4OFF_WRITE_DELAY
            #ifndef S4WRITE_DELAY
               #define S4WRITE_DELAY
            #endif
         #endif
         #ifndef S4OFF_READ_ADVANCE
            #ifndef S4READ_ADVANCE
               #define S4READ_ADVANCE
            #endif
         #endif
      #endif
   #endif
#endif

#ifdef S4OFF_WRITE_DELAY
   #ifdef S4WRITE_DELAY
      #undef S4WRITE_DELAY
   #endif
#endif

#ifdef S4OFF_READ_ADVANCE
   #ifdef S4READ_ADVANCE
      #undef S4READ_ADVANCE
   #endif
#endif

#ifdef S4READ_ADVANCE
   #ifndef S4ADVANCE_READ
      #define S4ADVANCE_READ
   #endif
#endif

#ifdef S4WRITE_DELAY
   #define S4MULTI_THREAD
#else
   #ifdef S4READ_ADVANCE
      #define S4MULTI_THREAD
   #endif
#endif


#ifdef S4OS2
   #ifndef S4CONSOLE
      #ifndef S4OS2PM
         #ifndef S4WINDOWS
            #define S4WINDOWS
         #endif
      #endif
   #endif
#endif

#ifdef S4DOS
   #ifndef S4CONSOLE
      #define S4CONSOLE
   #endif
#endif

#ifdef __unix__
   #ifndef S4CONSOLE
      #define S4CONSOLE
   #endif
#endif

   #ifndef S4OFF_TRAN
      #define S4STAND_ALONE_TRANS
   #endif
   #ifdef __unix__
      #define S4OFF_COMMUNICATIONS
   #endif
   #ifdef S4OS2
      #define S4OFF_COMMUNICATIONS
   #endif
   /* #ifndef S4UTILS */ /* switch not needed anymore unless CodeUtil connects to the server */
      #ifndef S4OFF_COMMUNICATIONS
         #define S4OFF_COMMUNICATIONS
      #endif
   /* #endif */

#ifndef S4MACINTOSH
   #ifndef S4WINDOWS
      #ifndef S4OS2PM
         #ifndef S4CODE_SCREENS
            #ifndef S4CONSOLE
               #error - Must compile with an output option( S4WINDOWS, S4OS2PM, S4CODE_SCREENS or S4CONSOLE )
            #endif
         #endif
      #endif
   #endif
#endif

#ifdef S4WINDOWS
#ifdef S4CONSOLE
   #error - Both S4WINDOWS and S4CONSOLE switches set - only one is allowed.
#endif
#ifdef S4OS2PM
   #error - Both S4WINDOWS and S4OS2PM switches set - only one is allowed.
#endif
#endif

#ifdef S4CONSOLE
#ifdef S4OS2PM
   #error - Both S4CONSOLE and S4OS2PM switches set - only one is allowed.
#endif
#endif



#ifdef S4MDX
   #ifdef S4FOX
     #error - Both S4MDX and S4FOX switches set - only one is allowed.
   #endif
   #ifdef S4CLIPPER
     #error - Both S4MDX and S4CLIPPER switches set - only one is allowed.
   #endif
   #ifdef S4NDX
     #error - Both S4MDX and S4NDX switches set - only one is allowed.
   #endif
#endif

#ifdef S4FOX
   #ifdef S4CLIPPER
     #error - Both S4FOX and S4CLIPPER switches set - only one is allowed.
   #endif
   #ifdef S4NDX
     #error - Both S4FOX and S4NDX switches set - only one is allowed.
   #endif
#endif

#ifdef S4CLIPPER
   #ifdef S4NDX
     #error - Both S4CLIPPER and S4NDX switches set - only one is allowed.
   #endif
#endif
/*   OS2 2.0 SUPPORT */
#ifdef __OS2__       /* Watcom 386, Borland C++ for OS/2 */
   #define S4OS2
#endif
#ifdef __IBMC__  /* IBM C SET/2 */
   #define S4OS2
   #define S4IBMOS2
#endif
#ifdef __IBMCPP__  /* IBM C++ SET/2 */
   #define S4OS2
   #define S4IBMOS2
#endif

#ifdef __OS2__
   #ifndef S4OS2
      #define S4OS2
   #endif
   typedef unsigned HANDLE;
   #ifdef __BORLANDC__
      #if __BORLANDC__ == 0x400
         #define S4MEMCMP
      #endif
   #endif
#endif

#ifdef S4OS2
   #ifndef __WATCOMC__
      #define INCL_DOSSEMAPHORES
      #ifndef S4OS2PM
         #define INCL_NOPMAPI    /* don't include PM.H */
      #endif
      #include <os2.h>
   #endif
#endif

#ifndef S4OS2
   #ifdef S4WINTEL
      #ifndef _SIZE_T_DEFINED_     /* Watcom */
         #ifndef _SIZE_T_DEFINED      /* Microsoft, Metaware */
            #ifndef _SIZE_T              /* Borland  */
               typedef unsigned size_t ;
               #define _SIZE_T_DEFINED_           /* Used to resolve Watcom 386 warning */
               #define _SIZE_T
            #endif
         #endif
      #endif
   #endif
#endif

#ifdef S4DOS
   typedef unsigned HANDLE ;
#endif

#ifdef __unix__
   #define S4DIR '/'
   #define INVALID4HANDLE -1
#else
   #ifdef S4MACINTOSH
      #define S4DIR ':'
      #define INVALID4HANDLE -1
   #else
      #define S4DIR '\\'
      #ifdef S4DOS
         #define INVALID4HANDLE (-1)
      #else
         #define INVALID4HANDLE INVALID_HANDLE_VALUE
      #endif
   #endif
#endif

#ifndef _A_RDONLY
   #define _A_RDONLY 0x01                  /* Read-only attribute  */
#endif

#ifdef __HIGHC__           /* align structures to word alignment, Metaware */
  #pragma Align_members(1)
  #pragma Off(Char_default_unsigned)
#endif

#ifdef __SC__
   #pragma SC align 1      /* align structures to word alignment, Symantec */
#endif

#ifdef __WATCOMC__
   #pragma off(unreferenced)
#endif

/* Visual FoxPro Support */
#ifdef S4FOX
   #ifdef S4GENERAL
      #ifndef S4VFP_KEY
         #define S4VFP_KEY
      #endif
   #endif
   #ifdef S4LANGUAGE
      #error - International Language Support is done via Collating Sequences
   #endif
#else
   #ifdef S4GENERAL
      #error - S4GENERAL switch should only be set with S4FOX.
   #endif
   #ifdef S4CODEPAGE_437
      #error - CodePage Support is only compatible with S4FOX.
   #endif
   #ifdef S4CODEPAGE_1252
      #error - CodePage Support is only compatible with S4FOX.
   #endif
#endif

                           /* Foreign language support */
#ifndef S4FOX   /* VFP uses codepages and collating sequences */
   #ifdef S4GERMAN
      #define S4LANGUAGE
   #endif

   #ifdef S4FRENCH
      #define S4LANGUAGE
      #define S4VMAP
   #endif

   #ifdef S4SWEDISH
      #define S4LANGUAGE
      #define S4VMAP
   #endif

   #ifdef S4FINNISH
      #define S4LANGUAGE
      #define S4VMAP
   #endif

   #ifdef S4ANSI
      #define S4VMAP
   #endif
#endif

#ifdef S4DLL_BUILD
   #define __DLL__ 1
#endif

#ifdef __unix__
   #define S4ERRNO           /* use global variable, 'errno' */
   #define S4NO_DUP          /* use if dup() not found/not required */
/* #define S4LSEEK        */ /* use if lseek() cannot seek past EOF */
   #define S4CASE_SEN
   #define S4STRUCT_PAD
   #define S4NO_INET_ATON

   #define S4OFF_REPORT
   #define S4OFF_THREAD
   #ifndef S4OFF_REPORT
      #error - CodeReporter not supported under Unix. Set S4OFF_REPORT.
   #endif
   #ifndef S4OFF_THREAD
      #error - Multithreading not supported under Unix. Set S4OFF_THREAD.
   #endif
   #ifndef S4OFF_THREAD
      #define __unix___THREADS
   #endif
#endif

#ifdef S4MACINTOSH
   #define S4BYTEORDER_3210
   #define S4BYTE_SWAP
   #define S4NO_FCVT
   #define S4NO_ECVT
   #define S4NO_FILELENGTH
   #define S4NO_STRUPR
   #define S4NO_STRLWR
   #define S4NO_STRNICMP
   #define S4CASE_SEN
   #define S4STRUCT_PAD
   #ifndef S4OFF_REPORT
      #error - CodeReporter not supported under Macintosh. Set S4OFF_REPORT.
   #endif
   #ifndef S4OFF_THREAD
      #error - Multithreading not supported under Macintosh. Set S4OFF_THREAD
   #endif
   long MAClseek(int, long, int, int) ;
#endif

#ifdef S4WINCE
   #define S4NO_ATOF
   #define ecvt(a,b,c,d) _ecvt( (a),(b),(c),(d) )
   #define fcvt(a,b,c,d) _fcvt( (a),(b),(c),(d) )
   #define S4NO_STRLWR
   #define S4NO_STRUPR
   #define S4NO_STRNICMP
   #define S4NO_TOUPPER
   #define S4UNICODE
   #ifndef S4OFF_REPORT
      #error - CodeReporter not supported under Windows CE. Set S4OFF_REPORT.
   #endif
   #ifndef S4OFF_MULTI
      #error - Multiuser is not supported under Windows CE. Set S4OFF_MULTI.
   #endif
   #ifndef S4OFF_OPTIMIZE
      #error - Optimization is not supported under Windows CE. Set S4OFF_OPTIMIZE.
   #endif
   #ifndef S4OFF_TRAN
      #error - Transaction Processing is not supported under Windows CE. Set S4OFF_TRAN.
   #endif
#endif

#ifdef S4BYTEORDER_3210
   #define S4DO_BYTEORDER
#endif

#ifdef S4BYTEORDER_2301
   #define S4DO_BYTEORDER
#endif

#ifdef S4MEMCMP
   #ifdef __cplusplus
      extern "C" {
   #endif
      int c4memcmp(const void *, const void *, size_t) ;
   #ifdef __cplusplus
   }
   #endif
#else
   #define c4memcmp memcmp
#endif

#ifdef S4FOX
   #define u4memcmp c4memcmp
#else
   #ifndef S4LANGUAGE
      #ifndef S4ANSI
         #define u4memcmp c4memcmp
      #endif
   #endif
#endif

#ifdef S4NO_MEMMOVE
   #ifdef __cplusplus
      extern "C" {
   #endif
   void *c4memmove(void *, const void *, size_t) ;
   #ifdef __cplusplus
      }
   #endif
#else
   #define c4memmove memmove
#endif

#ifdef __WIN32
   #define S4NO_FILELENGTH
// JACS   #define INVALID_HANDLE_VALUE (HANDLE)-1    /* for Windows NT CreateFile */
#else
   #ifdef S4WIN16
      #define INVALID_HANDLE_VALUE (HANDLE)-1
   #endif
   #ifdef __TURBOC__
      #ifndef __DLL__
         #ifdef S4DLL
            #ifdef S4CBPP
               #define S4CLASS huge
            #endif
            #define S4FUNCTION far pascal
         #endif
      #endif
   #endif

   #ifdef _MSC_VER
      #if _MSC_VER == 600
         #define S4NO_NEGATIVE_LOCK
      #endif
      #ifndef __DLL__
         #ifdef S4DLL
            #ifdef S4CBPP
               #define S4CLASS
            #endif
            #define S4FUNCTION far pascal
         #endif
      #endif
   #endif

   #ifdef __ZTC__
      #ifdef S4DLL
         #ifndef __DLL__
            #ifdef S4CBPP
               #define S4CLASS
            #endif
            #define S4FUNCTION far pascal
         #endif
      #endif
   #endif
#endif

#ifdef S4PASCAL_DOS
   #define S4PASCAL
#endif

#ifdef S4PASCAL_WIN
   #define S4PASCAL
#endif

#ifdef __DLL__
   #ifdef __WIN32
      #ifndef S4DLL
         #define S4DLL
      #endif
      #ifdef _MSC_VER
         #ifdef S4CBPP
            #define S4CLASS __declspec(dllexport)
         #endif
         #define PUBLIC __declspec(dllexport)
         #define S4FUNCTION __stdcall
      #else
         #ifdef S4CBPP
            #define S4CLASS _export
         #endif
         #define S4FUNCTION __stdcall _export
      #endif
   #else
      #ifdef S4OS2
         #ifdef S4CBPP
            #define S4CLASS _export
         #endif
         #define S4FUNCTION _export
      #else
         #ifndef S4DLL
            #define S4DLL
         #endif
         #ifdef S4CBPP
            #define S4CLASS _export
         #endif
         #define S4FUNCTION far pascal _export
      #endif
   #endif
#endif

#ifdef S4STATIC
#ifdef S4DLL
   #error - Both S4STATIC and S4DLL switches set - only one is allowed.
#endif
#endif

#ifndef S4DLL_BUILD
   #ifdef S4DLL
      #ifdef __WIN32
         #ifdef _MSC_VER
            #define S4FUNCTION __stdcall
         #endif
         #ifdef __BORLANDC__
            #define S4FUNCTION __stdcall _export
         #endif
      #endif
   #endif
#endif

#ifndef S4FUNCTION
   #define S4FUNCTION
#endif

#ifndef S4CLASS
   #define S4CLASS
#endif

#ifndef PUBLIC
   #define PUBLIC
#endif

#ifdef S4DLL
   #ifdef __WIN32
      #define S4CALL _cdecl
   #else
      #ifdef S4OS2
         #define S4CALL
      #else
         #define S4CALL far _cdecl _export
      #endif
   #endif
#else
   #ifdef _MSC_VER
      #define S4CALL _cdecl S4FUNCTION
   #else
      #ifdef __ZTC__
         #define S4CALL _cdecl S4FUNCTION
      #else
         #define S4CALL S4FUNCTION
      #endif
   #endif
#endif

#ifdef S4WINDOWS
   typedef unsigned int UINT ;
#endif

#ifndef TRUE
   #define TRUE 1
#endif
#ifndef FALSE
   #define FALSE 0
#endif

#ifdef S4WINDOWS
   #ifdef __TURBOC__
      #if __TURBOC__ == 0x297         /* Borland C++ 2.0 */
         #define M4PRINT sprintf
      #else
         #define M4PRINT wsprintf
      #endif
   #else
      #define M4PRINT wsprintf
   #endif
#else
   #define M4PRINT sprintf            /* DOS */
#endif

#ifndef __WIN32
   #ifdef S4DLL
      #ifndef S4OS2
         #define S4PTR far
      #endif
   #endif
#endif

#ifndef S4PTR
   #define S4PTR
#endif

#ifdef S4NDX
   #define N4OTHER
   /* default is to use dBASE III+/Clipper memo file formats if using .NDX index file formats */
   #define S4MNDX
#endif

#ifdef S4CLIPPER
   #define N4OTHER
   /* default is to use dBASE III+/Clipper memo file formats if using .NTX index file formats */
   #define S4MNDX
   #define S4HAS_DESCENDING
#endif

#ifdef S4FOX
   /* default is to use foxpro memo file formats if using foxpro index file formats */
   #define S4MFOX
   #define S4HAS_DESCENDING
#endif

#ifdef __unix__
   #ifdef S4NDX
      #error:  dBase III support not allowed.
   #endif
#endif

#ifdef S4MDX
   #define S4MMDX
#endif

#ifdef S4MNDX
   #define   MEMO4SIZE   0x200
#endif

#ifdef S4WINTEL
   typedef  const void S4PTR *  S4CMP_PARM ;
#endif

#ifdef S4DOS
   #ifndef S4LOW_MEMORY
      #define S4LOW_MEMORY
   #endif
#endif

#ifdef S4OFF_MULTI
   #ifndef S4OFF_ENFORCE_LOCK
      #define S4OFF_ENFORCE_LOCK
   #endif
#endif

#ifdef S4TEST
   #ifndef S4TESTING
      #define S4TESTING
   #endif
   #ifndef E4HOOK
      #define E4HOOK
   #endif
   #ifndef S4NO_OUT
      #define S4NO_OUT
   #endif
#endif

/*
#ifdef S4TESTING
   #ifndef S4COMPILE_TEST
      #define S4COMPILE_TEST
   #endif
#endif
*/

#ifdef S4COMPILE_TEST
   #ifdef S4OFF_MULTI
      #define S4COMP_OFF_MULTI
   #endif
   #ifdef S4OFF_MEMO
      #define S4COMP_OFF_MEMO
   #endif
   #ifdef S4OFF_INDEX
      #define S4COMP_OFF_INDEX
   #endif
#endif

#ifdef E4DEBUG
   #ifndef E4ANALYZE
      #define E4ANALYZE
   #endif
   #ifndef E4PARM_HIGH
      #define E4PARM_HIGH
   #endif
   #ifndef E4MISC
      #define E4MISC
   #endif
   #ifndef S4TESTING
      #ifndef E4STOP_CRITICAL
         #define E4STOP_CRITICAL
      #endif
   #endif
#endif

#ifdef E4OFF
   #ifndef E4OFF_ERROR
      #define E4OFF_ERROR
   #endif
   #ifndef E4ERROR_OFF
      #define E4ERROR_OFF
   #endif
   #ifndef E4OFF_STRING
      #define E4OFF_STRING
   #endif
#endif

#ifdef E4ANALYZE_ALL
   #ifndef E4ANALYZE
      #define E4ANALYZE
   #endif
   #ifndef E4INDEX_VERIFY
      #define E4INDEX_VERIFY
   #endif
   #ifndef E4LOCK
      #define E4LOCK
   #endif
#endif

#ifdef E4ANALYZE
   #ifndef E4LINK
      #define E4LINK
   #endif
   #ifndef E4PARM_LOW
      #define E4PARM_LOW
   #endif
#endif

#ifdef S4OFF_INLINE
   #undef S4OFF_INLINE
#endif

#define WAIT4EVER -1
   #ifdef S4TESTING
      #define ATS_FILENAME_REC "RECINFO.ATS"
      #define ATS_FILENAME_CS "CSINFO.ATS"
   #endif

#define TYPE4TEMP 'T'
#define TYPE4PERMANENT 'P'
#define TYPE4SCHEMA 'S'

#ifdef S4DLL
   #define sort4assignCmp(s4,f)  (s4)->cmp = (S4CMP_FUNCTION S4PTR *) MakeProcInstance((FARPROC) f, (HINSTANCE)(s4)->codeBase->hInst)
#else
   #define sort4assignCmp(s4,f)  (s4)->cmp = (S4CMP_FUNCTION S4PTR *) (f)
#endif

/*#define TRAN4INACTIVE  110*/
/*#define TRAN4ACTIVE    120*/
/*#ifdef S4DISTRIBUTED*/
/*   #define TRAN4PARTIAL  130*/
/*#endif*/
/* #define TRAN4ROLLBACK  7 -- defined in c4trans.h */

#define S4NUM_STRING_TYPES 16

#ifndef S4SINGLE
   #define CON4LOCK                 100
   #define CON4UNLOCK               200
#endif
#define CON4WRITE                   300
#define CON4GO                      400
#define CON4SKIP                    500
#define CON4SEEK                    600
#define CON4SEEK_DBL                650
#define CON4START                   700
#define CON4COMMIT_PHASE_ONE        800
#define CON4COMMIT_PHASE_TWO        900
#define CON4COMMIT_BOTH_PHASES      950
#define CON4ROLLBACK               1000
#define CON4OPEN                   1100
#define CON4CLOSE                  1200
#define CON4RECCOUNT               1300
#ifndef S4SINGLE
   #define CON4LOCK_CONFIRM        1400
   #define CON4LOCK_GROUP          1500
#endif
/* #define CON4ABORT               1600 */
#define CON4CONNECT                1700
#define CON4DISCONNECT             1800
/* #define CON4DISCONNECTED        1900 */
#define CON4PACK                   2000
#define CON4ZAP                    2100
#define CON4CREATE                 2200
#define CON4CANCEL                 2300
#define CON4RELATE_INIT            2400
#define CON4RELATE_TOP             2500
#define CON4RELATE_BOTTOM          2600
#define CON4RELATE_DO              2700
#define CON4RELATE_DO_ONE          2800
#define CON4RELATE_FREE            2900
/* #define CON4RELATE_CHANGED      2950 */
#ifndef S4SINGLE
   #define CON4RELATE_LOCK         3000
   #define CON4RELATE_UNLOCK       3100
#endif
#define CON4RELATE_SKIP            3200
#define CON4INDEX_CREATE           3300
#define CON4INDEX_OPEN             3400
#define CON4INDEX_CLOSE            3500
#define CON4POSITION               3600
#define CON4POSITION_SET           3700
#define CON4REINDEX                3800
#define CON4CHECK                  3900
#define CON4TOP                    4000
#define CON4BOTTOM                 4100
#define CON4APPEND                 4200
#define CON4MEMO_COMPRESS          4300
#define CON4MEMO                   4400
#define CON4INFO                   4500
#define CON4UNIQUE_SET             4600
/*#define CON4PASSWORD             4700 */
#define CON4TRANS_INIT             4800
#define CON4RELATE_OPT             4900
#define CON4SYSTEM                 5000
#define CON4TAG_SYNCH              5100
#define CON4DATE_FORMAT            5200
#define CON4TRAN_EOF               5300
#define CON4TRAN_EOF_HALT          5400
#define CON4TRAN_RESTART           5500
#define CON4INDEX_FORMAT           5600
#define CON4INDEX_INFO             5700
#define CON4INDEX_REINDEX          5800
#define CON4ACK                    5900
#define CON4INDEX_FNAME            6000
#define CON4DATA_FNAME             6100
#define CON4ADD_TAG                6200
#define CON4CATALOG                6300
#define CON4REMOVE                 6400
#define CON4PASSWORD_SET           6500
#define CON4RESTART                6600
#define CON4CALC_CREATE            6700
#define CON4CALC_RESET             6800
#define CON4TAG_OPEN               6900
#define CON4CRASH                  7000
#define CON4CREATE_AUX_CONNECTION  7100
#define CON4CONFIG_NAME            7200
#define CON4SHUTDOWN               7300
#define CON4CONNECT_ACCEPT_NEW     7400
#define CON4CONNECT_CUT_ALL        7500
#define CON4CONNECT_CUT            7600
#define CON4CLOSE_FILES            7700
#define CON4TAG_FNAME              7800

#define STREAM4START_COUNT        10000

#define STREAM4DISCONNECT         10000
#define STREAM4UNLOCK_ALL         10010
#define STREAM4LOCKED_INFO        10020
#define STREAM4RECONNECT          10030
#define STREAM4UNLOCK_DATA        10040
#define STREAM4PING               10050
#define STREAM4STATUS             10060
#define STREAM4TABLES             10070
#define STREAM4CURRENT_DIRECTORY  10080
#define STREAM4DIRECTORY          10090

#define STREAM4BLAST_TEST_WRITE   11100
#define STREAM4BLAST_TEST_READ    11200

#define MSG5DB_SESSION_DATA_CLOSE     12000
#define MSG5DB_SESSION_DATA_OPEN      12100
#define MSG5DB_SESSION_DATA_CREATE    12200
#define MSG5DB_ROW_REQUEST_ARRAY      12400
#define MSG5DB_REQUEST_DEFERRED_FIELD 12500
#define MSG5DB_ROW_REQUEST_SEQUENTIAL 12600
#define MSG5DB_INDEX_REQUEST_KEY      12700
#define MSG5DB_INDEX_ROW_REQUEST      12800
#define MSG5DB_INDEX_ROW_REQUEST_KEYS 12900
#define MSG5DB_COLUMN_INFO            13000
#define MSG5DB_UPDATE_FIELDSET        13100
#define MSG5DB_RECCOUNT               13200
#define MSG5DB_SESSION_INDEX_CLOSE    13300
#define MSG5DB_SESSION_INDEX_OPEN     13400
#define MSG5DB_INDEX_COLUMN_INFO      13500
#define MSG5DB_INDEX_UPDATE_FIELDSET  13600
#define MSG5DB_SESSION_SCHEMA_OPEN    13700
#define MSG5DB_CURRENT_DIRECTORY      13800
#define MSG5DB_SET_RESTRICTIONS       13900
#define MSG5DB_ADD_DIRECTORY          14000
#define MSG5DB_TAG_SELECT             14100
#define MSG5DB_SCHEMA_SEEK            14200
#define MSG5DB_SCHEMA_REQUEST_SEQ     14300
#define MSG5DB_SCHEMA_GET_NEXT        14400
#define MSG5DB_FIND_OR_ADD_ENTRY      14500
#define MSG5DB_ADD_TAG                14600
#define MSG5DB_INDEX_REMOVE           14700
#define MSG5DB_TBL_REMOVE_INDEXES     14800
#define MSG5DB_TBL_REMOVE             14900
#define MSG5DB_SESSION_DATA_SEEK      15000
#define MSG5DB_SESSION_DATA_WRITE     15100
#define MSG5DB_SESSION_DATA_WRITE_DONE 15200
#define MSG5LEN                       15300
#define MSG5DB_SESSION_DATA_DELETE    15400
#define MSG5DB_SESSION_DATA_DEFAULTS  15500
#define MSG5DB_TBL_WRITE_RESULT_INC   15600
#define MSG5DB_INDEX_SET_RANGE        15700
#define MSG5DB_SESSION_ISO_LEVEL      15800
#define MSG5DB_SESSION_IN_TRANSACTION 15900
#define MSG5DB_ADD_COLUMN             16000
#define MSG5DB_REMOVE_COLUMN          16100


#ifdef S4JAVA
   #define JAVA4SINGLE_OPEN_SETTING 0

   /* same as in c4comws.h */
   #ifdef E4OFF_STRING
      #define S4SOCK_BASE_ERROR_NUM   88000L
   #else
      #define S4SOCK_BASE_ERROR_DEF   E88004
   #endif

   #define java4lockAll    -2
   #define java4lockAppend  0
   #define java4lockFile   -1

   #define java4blankField       0
   #define java4nullField       -1
   #define java4defaultFieldLen -2
   #define java4shortLen        -3
   #define java4longLen         -4
   #define java4noChange        -5
   #define java4doubleField     -6

   #define java4lenVariable      0

   #define CON4ACK            5900

   /* start count is used to decide when to process a message (i.e. if > this value */
   #define JAVA4START_COUNT   20000

   #define JAVA4LOCK_GROUP    20100
   #define JAVA4UNLOCK        20200
   #define JAVA4WRITE         20300
   #define JAVA4GO            20400
   #define JAVA4SKIP          20500
   #define JAVA4SEEK_N        20600
   #define JAVA4SEEK_DBL      20650
   #define JAVA4OPEN          21100
   #define JAVA4CLOSE         21200
   #define JAVA4RECCOUNT      21300
   #define JAVA4CONNECT       21700
   #define JAVA4DISCONNECT    21800
   #define JAVA4PACK          22000
   #define JAVA4CREATE        22200
   #define JAVA4INDEX_CREATE  23300
   #define JAVA4INDEX_OPEN    23400
   #define JAVA4POSITION      23600
   #define JAVA4POSITION_SET  23700
   #define JAVA4REINDEX       23800
   #define JAVA4TOP           24000
   #define JAVA4BOTTOM        24100
   #define JAVA4APPEND        24200
   #define JAVA4ACCESS_MODE_SET 28000
   #define JAVA4READ_LOCK_SET  28300
   #define JAVA4STATUS_CODE   29000
   #define JAVA4STATUS_FIELDS 29100
   #define JAVA4READ_ONLY_SET  28400
   #define JAVA4REGISTER_FIELD 28600
   #define JAVA4SAFETY_SET    28700
   #define JAVA4SELECT_TAG    28900
   #define JAVA4DEFAULT_UNIQUE_SET 28200
   #define JAVA4SELECT_DATA   28800
   #define JAVA4BLANK         28100
   #define JAVA4RECNO         28500
   #define JAVA4UNLOCK_AUTO_SET 29200
#endif /* S4JAVA */

#define TIME4OUT 5

#ifndef S4SINGLE
   #define LOCK4OFF       0
   #define LOCK4ALL       1
   #define LOCK4DATA      2
   #define LOCK4APPEND   10
   #define LOCK4FILE     20
   #define LOCK4RECORD   30
   #define LOCK4INDEX    40
#endif

#define LEN4ACCOUNT_ID   20
#define LEN4PASSWORD     20
#define LEN4GARBAGE     518
#define LEN4DATA_ALIAS   32
#define LEN4TABLE_NAME LEN4DATA_ALIAS + 4
#define LEN4TAG_ALIAS    10
#define LEN4PATH        250
#define LEN4DATE_FORMAT  20
#define LEN4PROTOCOL    128
#define LEN4USER_ID      32

#define LEN4USERID       10
#define LEN4NETID        20

#define LOG4ALWAYS 2
#define LOG4ON     1
#define LOG4TRANS  0

#ifdef S4FOX
   #define LEN4HEADER_WR 0x10
#endif

#define OPEN4DENY_NONE  0
#define OPEN4DENY_RW    1
#define OPEN4DENY_WRITE 2
#define OPEN4SPECIAL    3    /* for internal use only */

#define OPT4EXCLUSIVE  -1
#define OPT4OFF         0
#define OPT4ALL         1

/* used for the # of lists in the priority chain */
#define OPT4NUM_LISTS       5

#define AUTH4ALLOW 'Y'
#define AUTH4DISALLOW ' '

#define PROT4DEFAULT 0

#ifdef S4MULTI_THREAD
   #define r4queued 1
   #define r4inUse  2
   #define r4finished 3
   #define r4canceled 4
#endif

#define r4quit   9

#define r4restart  1
#define r4shutdown 2

#define r4complete 2
#define r4down 1
#define r4same 0

/* Integer Return Codes */
#define r4off              -2
#define r4success           0
#define r4found             1     /* Primary Key Match */
#define r4after             2
#define r4eof               3
#define r4bof               4
#define r4entry             5     /* No index file entry or no record (go) */
#define r4noRecords         6
#define r4delay             7
#define r4descending       10
#define r4candidate        15
#define r4unique           20     /* Key is not unique, do not write/append */
#define r4uniqueContinue   25 /* Key is not unique, write/append anyway */
/* #define r4keep             30 no longer used */
/* #define r4ignore           40 no longer used */
#define r4locked           50
#define r4noCreate         60     /* Could not create file */
#define r4noOpen           70     /* Could not open file */
#define r4noTag            80     /* DataIndex::seek, with no default tag */
#define r4terminate        90     /* no relation match with terminate set */
#define r4exit            100     /* a function is requesting program termination */
#define r4inactive        110     /* transactional state */
#define r4partial         115     /* transactional state */
#define r4active          120     /* transactional state */
#define r4rollback        130     /* transactional state */
#define r4authorize       140     /* lacks authorization rights to perform action */
#define r4connected       150
#define r4logOn           160
#define r4logOpen         170
#define r4logOff          180
#define r4null            190
#define r4cdx             200
#define r4mdx             201
#define r4ntx             202
#define r4ndx             203
#define r4unknown         204
#define r4done            210
#define r4pending         215
#define r4deleted         220
#define r4timeout         225
#define r4invalid         230
#define r4schema          240
#define r4open            245
#define r4blankTcpAddress 250
#define r4errNetwork      260  /* network not properly setup */
#define r4hostNotFound    270  /* host name cannot be resolved to address */
#define r4hostUnavailable 280  /* host exists, but can't be reached */
#define r4numSockets      290  /* number of sockets is maxed out */
#define r4connectTimeOut  300  /* socket timed out waiting for connection */
#define r4noServerOnHost  305  /* host available, but no server running */
#define r4noConnect       310  /* error connecting, reason unknown */

/* backward compatibility redefinitions */
#define r4no_records      r4noRecords
#define r4unique_continue r4uniqueContinue
#define r4no_create       r4noCreate
#define r4no_open         r4noOpen
#define r4no_tag          r4noTag


/* collating sequence support */
#define sort4machine 0
#define sort4general 1

/* codepage support */
#define cp0      0
#define cp437    1
#define cp850    2
#define cp1252   3


/* General Disk Access Errors */
#define e4close    -10
#define e4create   -20
#define e4len      -30
#define e4lenSet   -40
#define e4lock     -50
#define e4open     -60
#define e4permiss  -61
#define e4access   -62
#define e4numFiles -63
#define e4fileFind -64
/* e4exclusive means that exclusive access was required but file not open in that mode */
#define e4exclusive -65
#define e4instance -69
#define e4read     -70
#define e4remove   -80
#define e4rename   -90
#define e4unlock  -110
#define e4write   -120

/* Database Specific Errors */
#define e4data      -200
#define e4fieldName -210     /* Invalid field name */
#define e4fieldType -220
#define e4recordLen -230
#define e4append    -240
#define e4seek      -250

/* Index File Specific Errors */
#define e4entry      -300     /* Tag entry not located */
#define e4index      -310
#define e4tagName    -330
#define e4unique     -340     /* Key is not unique */
#define e4tagInfo    -350     /* tag information is invalid */
#define e4candidate  -360     /* key is not unique/non-null */

/* Expression Errors */
#define e4commaExpected  -400
#define e4complete       -410
#define e4dataName       -420
#define e4lengthErr      -422
#define e4notConstant    -425
#define e4numParms       -430
#define e4overflow       -440 /* Overflow while evaluating expression */
#define e4rightMissing   -450
#define e4typeSub        -460
#define e4unrecFunction  -470
#define e4unrecOperator  -480
#define e4unrecValue     -490
#define e4unterminated   -500
#define e4tagExpr        -510 /* Expression is invalid for use in a tag - eg. DAT4 pointers */

/* Optimization Errors */
#define e4opt         -610
#define e4optSuspend  -620
#define e4optFlush    -630

/* thread management and communications errors */
#define e4event       -650
#define e4outstanding -660
#define e4signal      -670
#define e4semaphore   -680

/* Relation Errors */
#define e4relate      -710
#define e4lookupErr   -720
#define e4relateRefer -730   /* relation referred to does not exist or not initialized */

/* Report Errors */
#define e4report           -810
#define e4styleCreate      -811
#define e4styleSelect      -812
#define e4styleIndex       -813
#define e4areaCreate       -814
#define e4groupCreate      -815
#define e4groupExpr        -816
#define e4totalCreate      -817
#define e4objCreate        -818
#define e4repWin           -819
#define e4repOut           -820
#define e4repSave          -821
#define e4repRet           -822
#define e4repData          -823
/* backward compatibility redefinitions */
#define e4style_create     e4styleCreate
#define e4style_select     e4styleSelect
#define e4style_Index      e4styleIndex
#define e4area_create      e4areaCreate
#define e4group_create     e4groupCreate
#define e4group_expr       e4groupExpr
#define e4total_create     e4totalCreate
#define e4obj_create       e4objCreate
#define e4rep_win          e4repWin
#define e4rep_out          e4repOut
#define e4rep_save         e4repSave
#define e4rep_ret          e4repRet
#define e4rep_data         e4repData

/* Critical Errors
*/
#define e4info     -910  /* Unexpected information in internal variable */
#define e4memory   -920  /* Out of memory */
#define e4parm     -930  /* Unexpected parameter */
#define e4parmNull -935  /* Unexpected parameter - null input */
#define e4demo     -940  /* Exceeded maximum record number for demo */
#define e4result   -950  /* Unexpected result */
#define e4verify   -960
#define e4struct   -970

/* Library Errors */
/* call to library function calls not supported */
#define e4notIndex     -1010  /* S4OFF_INDEX */
#define e4notMemo      -1020  /* S4OFF_MEMO */
#define e4notRename    -1030  /* S4NO_RENAME */
#define e4notWrite     -1040  /* S4OFF_WRITE */
#define e4notClipper   -1050  /* S4CLIPPER */
#define e4notLock      -1060  /* S4LOCK_HOOK */
/* #define e4notHook      -1070  E4HOOK */
#define e4notSupported -1090  /* generally not supported (maybe due to server set-up) */
#define e4version      -1095  /* version mismatch */

/* MEMO errors */
#define e4memoCorrupt -1110
#define e4memoCreate  -1120

/* transaction errors */
#define e4transViolation -1200
#define e4trans       -1210
#define e4rollback    -1220
#define e4commit      -1230
#define e4transAppend -1240
#define e4transStatus -1250

/* communications errors */
#define e4corrupt     -1300
#define e4connection  -1310
#define e4socket      -1320
#define e4net         -1330
#define e4loadlib     -1340
#define e4timeOut     -1350
#define e4message     -1360
#define e4packetLen   -1370
#define e4packet      -1380
#define e4connect     -1390

/* miscellaneous errors */
#define e4max               -1400
#define e4codeBase          -1410
#define e4name              -1420
#define e4authorize         -1430
#define e4invalidUserId     -1440
#define e4invalidPassword   -1450
#define e4invalidTcpAddress -1460
#define e4connectDenied     -1470
#define e4invalidLicence    -1480


/* e4packet means the packet is corrupted */

/* all server-specific error >2100, not only e4server returned to client */

#define e4server      -2100
#define e4config      -2110
/* #define e4cat         -2120  -- not used currently */

#define E4DEMO_MAX 200

/* garbage between expression and filter is length: */
#ifdef S4FOX
   #define I4MULTIPLY       1
   #define B4DO_BACK_LEVEL  3
   #define I4MAX_KEY_SIZE 240
   #define I4MAX_KEY_SIZE_COMPATIBLE 240
   #define F4MAX_NUMERIC   20
   #define F4MAX_DECIMAL   19
   #define F4DECIMAL_OFFSET 1
#else
   #define I4MULTIPLY     512
   #define F4DECIMAL_OFFSET 2
#endif
#ifdef S4CLIPPER
   #define F4MAX_NUMERIC   19
   #define F4MAX_DECIMAL   15
   #define I4MAX_KEY_SIZE 338
   #define I4MAX_KEY_SIZE_COMPATIBLE 338
#endif
#ifdef S4MDX
   #define F4MAX_NUMERIC   20
   #define F4MAX_DECIMAL   18
   /* for MDX the verify size is what is physically allowed to be dBASE
      compatible.  However, (eg. oledb) uses larger key sizes, which work
      but are incompatible */
   #define I4MAX_KEY_SIZE_COMPATIBLE 102
   #define I4MAX_KEY_SIZE 240
#endif
#ifndef I4MAX_KEY_SIZE
   #define I4MAX_KEY_SIZE 102
   #define I4MAX_KEY_SIZE_COMPATIBLE 102
#endif

#define E4ACCURACY     1.0e-13
#define E4ACCURACY_DIGITS  15

/* if S4NO_NEGATIVE_LOCK is defined, there is no dBASE IV compatibility */

   #ifndef S4SINGLE
      #ifdef N4OTHER
         #define L4LOCK_POS     1000000000L
      #endif
      #ifdef S4FOX
         #define L4LOCK_POS_OLD 0x40000000L
         #define L4LOCK_POS     0x7FFFFFFEL
      #endif
      #ifdef S4MDX
         #ifdef S4NO_NEGATIVE_LOCK
            #define L4LOCK_POS_OLD 1000000000L
            #define L4LOCK_POS     2000000000L
         #else
            #define L4LOCK_POS_OLD 0x40000000L
            #define L4LOCK_POS     0xEFFFFFFFL
         #endif
      #endif
   #endif

#ifdef S4MDX
   #define I4MAX_EXPR_SIZE 220
#endif
#ifdef S4NDX
   #define I4MAX_EXPR_SIZE 220
#endif
#ifdef S4FOX
   #define I4MAX_EXPR_SIZE 255
#endif
#ifdef S4CLIPPER
   #define I4MAX_EXPR_SIZE 255
#endif

#ifdef N4OTHER
   #ifdef S4NDX
      #define B4BLOCK_SIZE 512
   #else
      #ifdef S4CLIPPER
         #define B4BLOCK_SIZE 1024
      #endif
   #endif
#endif

#ifdef S4FOX
#define B4BLOCK_SIZE 512
#endif

#define CONNECTION4BUFFER_LEN 4096

#define E4DEBUG_INT 0x5281

#ifdef S4COMNB
   #define S4SERVER_NAME_SIZE 16
#endif

#ifdef S4COMFILE
   #define S4SERVER_NAME_SIZE 80
#endif

#define S4MAX_SERVER_NAME_SIZE 80


   #define S4CONST const

#define even4up( a ) ( (a) + ( ( (a) / 2 ) * 2 != (a) ) )
#define q4oddUp( a ) ( (a) + 2 * ( (((a)/4)*4) == (a) ) )
#define q4up( a ) ( (a) + 2 * ( (((a)/4)*4) != (a) ) )
#define quad4oddUp(a)  q4oddUp(even4up(a)) /* returns 'a' so that 'a' mod 4 = 2 */
#define quad4up(a) q4up(even4up(a) )  /* returns 'a' so that 'a' mod 4 = 0 */


#ifndef S4CONV
   #define S4CONV( a, b )  a
#endif

#ifdef S464BIT
   #define S4LONG int
   #define S4UNSIGNED_LONG unsigned int
#else
   #define S4LONG long
   #define S4UNSIGNED_LONG unsigned long
#endif

#ifdef S4READ_ADVANCE
   #define AR4EMPTY  1
   #define AR4SET    2
   #define AR4FULL   3
#endif

#ifdef __WIN32
   #ifdef S4FILE_EXTENDED
      #ifdef S4FOX
         #ifdef _MSC_VER
            #if _MSC_VER < 1100
               #undef S4FILE_EXTENDED
            #endif
         #endif
      #else
         #undef S4FILE_EXTENDED
      #endif
   #else
      #ifdef S4FOX
         #ifdef _MSC_VER
            #if _MSC_VER >= 1100  /* only for Visual C++ 5 and higher */
               #define S4FILE_EXTENDED
            #endif
         #else
            #define S4FILE_EXTENDED
         #endif
      #endif
   #endif
   #ifdef __BORLANDC__
      #if __BORLANDC__ < 0x500
         #ifdef S4FILE_EXTENDED
            #undef S4FILE_EXTENDED
         #endif
      #endif
   #endif
#else   /* we only support extended length files with WIN 32 and 64-bit UNIX machines*/
   #ifndef S464BIT
      #ifdef S4FILE_EXTENDED
         #undef S4FILE_EXTENDED
      #endif
   #else
      #ifdef S4FOX
         #ifndef S4FILE_EXTENDED
            #define S4FILE_EXTENDED
         #endif
      #else
         #ifdef S4FILE_EXTENDED
            #undef S4FILE_EXTENDED
         #endif
      #endif
   #endif
#endif

#ifdef S4FILE_EXTENDED
   /* Need enough room for both a large starting offset and room to grow, use middle value */
   /* this value is the hi-byte offset used for locking when using large files */
   #define S4LARGE_FILE_OFFSET 0x3FFFFFFF
#endif

#ifdef S4SPEED_TEST
   #ifdef E4ANALYZE
      #undef E4ANALYZE
   #endif
   #ifdef E4DEBUG
      #undef E4DEBUG
   #endif
   #ifdef E4LINK
      #undef E4LINK
   #endif
   #ifdef E4PARM_HIGH
      #undef E4PARM_HIGH
   #endif
   #ifdef E4PARM_LOW
      #undef E4PARM_LOW
   #endif
   #ifdef E4MISC
      #undef E4MISC
   #endif
   #ifdef E4OFF_STRING
      #undef E4OFF_STRING
   #endif
   #ifdef E4INDEX_VERIFY
      #undef E4INDEX_VERIFY
   #endif
   #ifdef E4LOCK
      #undef E4LOCK
   #endif
      #ifdef S4OFF_OPTIMIZE
         #error should not compile with both S4OFF_OPTIMIZE and S4SPEED_TEST
      #endif
      #ifndef S4OFF_TRAN
         #error should not compile without S4OFF_TRAN and S4SPEED_TEST
      #endif
   #ifdef S4OFF_THREAD
      #error should not compile with both S4OFF_THREAD and S4SPEED_TEST
   #endif
#endif

/* use Bool5 instead because it allows porting (esp. communications) to know
   whether or not bytes need to be swapped.
   0 is false, all else is true */
#define Bool5 unsigned char

#define false5 0
#define true5  1

#ifdef S4TRACK_FILES
   #define S4TRACK_FILES_OR_SERVER
#endif

#ifndef OLEDB5BUILD
#define assert5( p )  ( (p) ? 0 : error4( 0, e4result, E99999 ) )
#define assert5always( p ) ( (p) ? 0 :  error4( 0, e4result, E99999 ) )
#endif

#define tran4notRollbackOrCommit 10
#define tran4rollbackOrCommit    20

#endif /* D4DEFS_INC */
