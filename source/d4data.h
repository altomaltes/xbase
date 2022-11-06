/* d4data.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

typedef int S4CALL S4CMP_FUNCTION( S4CMP_PARM, S4CMP_PARM, size_t) ;

/* typedef required to work around VC++ 1.50 bug in definition of Sort4.assignCmp() */
typedef int (S4CALL *S4CMP_FUNCTION_PTR)( S4CMP_PARM, S4CMP_PARM, size_t ) ;
typedef short BOOL4  ;

#ifdef __SC__
   typedef void _cdecl C4DTOK( char S4PTR *, const double ) ;
   typedef void _cdecl C4STOK( char S4PTR *, const char S4PTR *, const int ) ;
#else
   typedef void C4DTOK( char S4PTR *, const double ) ;
   typedef void C4STOK( char S4PTR *, const char S4PTR *, const int ) ;
#endif

#ifdef S4FOX
   typedef unsigned char translatedChars[256] ;
   typedef unsigned char compressedChars[2] ;
#endif

#ifdef __cplusplus
   #ifndef S5OLE_DB
      #define C4PLUS_PLUS
   #endif
#endif

struct DATA4St ;
struct FIELD4St ;
struct DATA4FILESt ;
#ifdef S4COMPILE_TEST
   #ifndef S4OFF_MEMO
      struct F4MEMOSt ;
   #endif
   #ifndef S4OFF_INDEX
      struct INDEX4St ;
      struct TAG4St ;
      struct TAG4FILESt ;
      #ifndef S4CLIPPER
         struct INDEX4FILESt ;
      #endif
      #ifdef S4FOX
         struct T4VFPSt ;
      #endif
   #endif
   #ifndef S4OFF_TRAN
      struct S4CLASS TRAN4FILESt ;
      struct S4CLASS CODE4TRANSSt ;
   #endif
#else
   struct F4MEMOSt ;
   struct INDEX4St ;
   struct TAG4St ;
   struct TAG4FILESt ;
   struct S4CLASS TRAN4FILESt ;
   struct S4CLASS CODE4TRANSSt ;
   #ifndef S4CLIPPER
      struct INDEX4FILESt ;
   #endif
   #ifdef S4FOX
      struct T4VFPSt ;
   #endif
#endif

#ifdef S4CBPP
   class S4CLASS FILE4 ;
#else
   struct FILE4St ;
#endif
#ifdef S4CBPP
   class S4CLASS CODE4 ;
#else
   struct CODE4St ;
#endif

#ifdef S4CBPP
   class S4CLASS Code4 ;
   typedef int ERROR4FUNCTION( Code4 S4PTR *, short int, long ) ;
   typedef int ERROR4DESCRIBE_FUNCTION( Code4 S4PTR *, short int, long, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   typedef int ERROR4STACK( Code4 S4PTR *, short int, long ) ;
   typedef int ERROR4SET( Code4 S4PTR *, short int ) ;
#else
   typedef int ERROR4FUNCTION( struct CODE4St S4PTR *, short int, long ) ;
   typedef int ERROR4DESCRIBE_FUNCTION( struct CODE4St S4PTR *, short int, long, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   typedef int ERROR4STACK( struct CODE4St S4PTR *, short int, long ) ;
   typedef int ERROR4SET( struct CODE4St S4PTR *, short int ) ;
#endif

#ifdef C4PLUS_PLUS
   #ifdef S4WINDOWS
      #ifdef __WIN32
         #ifdef _MSC_VER
            typedef struct _declspec(dllexport) l4linkSt
         #else
            typedef struct _export l4linkSt
         #endif
      #else
         typedef struct _export l4linkSt
      #endif
   #else
      typedef struct l4linkSt
   #endif
#else
typedef struct l4linkSt
#endif
{
   struct l4linkSt S4PTR *n, S4PTR *p ;
} LINK4 ;

#ifdef S5OLE
   typedef struct S5CLASS
#else
   #ifdef S4CBPP
      typedef struct S4CLASS
   #else
      typedef struct
   #endif
#endif
{
   S4CONV( LINK4 S4PTR *lastNode, LINK4 S4PTR *last_node ) ; /* the last Link */
   LINK4 S4PTR *selected ;
   S4CONV( unsigned short int nLink, unsigned short int n_link ) ; /* The number of links in the list */
} LIST4 ;

typedef struct s4singleSt
{
   struct s4singleSt *nextLink ;
} SINGLE4 ;

typedef struct s4singleDistantSt
{
   struct s4singleDistantSt *nextLink ;
} SINGLE4DISTANT ;

typedef struct
{
   LINK4 link ;
   double  data ;  /* Make sure it is on a boundry good for at least a double  */
} Y4CHUNK ;

typedef struct
{
   LINK4 link ;

   LIST4 chunks ;      /* Chunks of pieces */
   LIST4 pieces ;      /* A list of available memory pieces */

   int unitStart;    /* The starting # of entries for the Memory Type */
   unsigned   unitSize ;   /* The size of each allocated piece */
   int unitExpand ; /* The expansion # of entries for the Memory Type */
   int nRepeat ;    /* The number of times entry returned for 'new' */
                    /* If nRepeat is '-1', it is a temporary entry. */
   int nUsed ;      /* The number of entries used */
   #ifdef S4DOS    /* no memory sharing under DOS, so can use a CODE4 for limited memory */
      #ifdef S4CBPP
         CODE4 S4PTR    *codeBase ;
      #else
         struct CODE4St *codeBase ;
      #endif
   #endif
}  MEM4 ;

#ifndef S4OFF_OPTIMIZE
typedef struct
{
   LIST4 list ;

   unsigned short int minLink ;
   unsigned long maxTime ;
   unsigned long minTime ;
   char currentPrioCount ;    /* should this list be scanned for avail blocks? */
} OPT4LIST ;

typedef struct
{
   LINK4 link ;
   LINK4 lruLink ;
   char changed ;
   unsigned len ;
   void S4PTR *data ;
   unsigned long readTime ;
   unsigned long accessTime ;
   double hitCount ;
   OPT4LIST *optList ;       /* the optimization list the file resides on */

   /* these next 2 elements must match the OPT4CMP structure in o4opt.h */
   #ifdef S4CBPP
      FILE4 S4PTR *file ;
   #else
      struct FILE4St *file ;
   #endif
   long pos ;
} OPT4BLOCK ;

#ifdef S4CBPP
   typedef struct S4CLASS
#else
   typedef struct
#endif
{
   char blockPower ;
   char forceCurrent ;    /* switch forces a read of current contents */
   char numShift ;

   unsigned char oldMode ;
   unsigned char doUpdate ;
   unsigned char checkCount ;
   unsigned char dummyChar ;

   unsigned int minLink ;
   int numBuffers ;

   unsigned maxBlocks ;
   unsigned writeBlockCount ; /* is the buffer full? */

   unsigned long blockSize ;
   unsigned long bufferSize ;
   unsigned long hashTrail ;       /* where last optimized file ended */
   unsigned long mask ;
   unsigned long numBlocks ;
   unsigned long numLists ;
   unsigned long readStartPos ;
   unsigned long writeStartPos ;
   unsigned long writeCurPos ;

   unsigned long readTimeCount ;
   unsigned long accessTimeCount ;
   unsigned int minAccessTimeVariation ;  /* access times of less than this value will not be updated == 1% of blocks */

   char S4PTR *readBuffer ;
   char S4PTR *writeBuffer ;
   char S4PTR *writeBufferActual ;  /* writeBuffer just points to the actual buffer */

   void S4PTR* S4PTR* buffers ;

   OPT4BLOCK S4PTR *blocks ;

   LIST4 S4PTR *lists ;
   OPT4LIST S4PTR *prio[OPT4NUM_LISTS] ;

   LIST4 avail ;
   OPT4LIST dbfLo ;
   OPT4LIST dbfHi ;
   OPT4LIST indexLo ;
   OPT4LIST indexHi ;
   OPT4LIST other ;

   LIST4 optFiles ;

   #ifdef __WIN32
      #ifdef S4WRITE_DELAY
         LIST4 delayAvail ;   /* extra blocks to allow efficient delay-writing */
         char S4PTR *delayWriteBuffer ;
         char S4PTR *delayLargeBuffer ;
         CRITICAL_SECTION critical4optWrite ;
         int delayLargeBufferAvail ;
         int writeBufferActualAvail ;
      #else
         LIST4 space1 ;   /* extra blocks to allow efficient delay-writing */
         char S4PTR *space2 ;
         char S4PTR *space3 ;
         CRITICAL_SECTION space4 ;
         int space5 ;
         int space6 ;
      #endif
   #endif

   #ifdef __WIN32
      #ifdef S4ADVANCE_READ
         char S4PTR *advanceLargeBuffer ;
         CRITICAL_SECTION critical4optRead ;
         #ifdef S4CBPP
            FILE4 S4PTR *advanceReadFile ;
         #else
            struct FILE4St S4PTR *advanceReadFile ;
         #endif
         int advanceLargeBufferAvail ;
         unsigned long advanceLargePos ;
         unsigned int advanceLargeLen ;
      #else
         char S4PTR *space7 ;
         CRITICAL_SECTION space8 ;
         #ifdef S4CBPP
            FILE4 S4PTR *space9 ;
         #else
            struct FILE4St S4PTR *space9 ;
         #endif
         int space10 ;
         unsigned long space11 ;
         unsigned int space12 ;
      #endif
   #endif

   #ifdef S4CBPP
      FILE4 S4PTR    *writeFile ;
      FILE4 S4PTR    *readFile ;
   #else
      struct FILE4St S4PTR *writeFile ;        /* which file has the write buffer? */
      struct FILE4St S4PTR *readFile ;        /* which file has the write buffer? */
   #endif
} OPT4 ;

#endif /* S4OFF_OPTIMIZE */

/* temporarily allow everything to work if S4FILE_EXTENDED not defined (for development purposes only) */
#ifdef S4FILE_EXTENDED
   #ifndef S464BIT
   typedef struct
   {
      unsigned long longLo ;  /* must be unsigned, can be up to 4 gig */
      #ifdef __WIN32
         /* for 64 bit file addressing (files > 4 GIG) */
         long longHi ;  /* signed value, negative could be error or that the whole value is negative (-1 means longLo is negative) */
      #endif
   } FILE4LONGPIECE ;

   typedef union
   {
      FILE4LONGPIECE piece ;
      DWORDLONG dLong ;
   } FILE4LONG ;

      #define file4longAssign( f2, longLoIn, longHiIn ) ( ((f2).piece.longLo) = (longLoIn), ((f2).piece.longHi) = (longHiIn) )
      #define file4longAssignLong( f1, f2 ) ( (f1) = (f2))    // JACS
      #define file4longCmp( f1, f2 ) ( ( (f1).piece.longLo == (f2).piece.longLo && (f1).piece.longHi == (f2).piece.longHi ) ? 0 : 1 )
      #define file4longGetLo( f1 ) ( (f1).piece.longLo )
      #define file4longGetHi( f1 ) ( (f1).piece.longHi )
      #define file4longGreater( f1, val ) ( (f1).piece.longLo > (val) || (f1).piece.longHi > 0 )
      #define file4longGreaterEqZero( f1 ) ( (f1).piece.longLo >= 0 && (f1).piece.longHi >= 0 )
      #define file4longGreaterLong( f1, f2 ) ( ((f1).piece.longHi > (f2).piece.longHi ) || ( (f1).piece.longHi == (f2).piece.longHi && (f1).piece.longLo > (f2).piece.longLo ) )
      #define file4longGreaterZero( f1 ) ( ((f1).piece.longLo > 0 && (f1).piece.longHi >= 0 ) ||((f1).piece.longHi > 0))
      #define file4longLess( f1, val ) ( (f1).piece.longLo < (val) && (f1).piece.longHi == 0 )
      #define file4longLessEq( f1, val ) ( (f1).piece.longLo <= (val) && (f1).piece.longHi == 0 )
      #define file4longLessEqLong( f1, f2 ) ( ((f1).piece.longHi < (f2).piece.longHi)|| ( ((f1).piece.longHi == (f2).piece.longHi) && ((f1).piece.longHi <= (f2).piece.longHi)) )
      #define file4longSetLo( f1, val ) ( (f1).piece.longLo = val )
      #define file4longMultiply( f1, f2 ) ( (f1).dLong *= (f2))
      #define file4longDivide( f1, f2 ) ( (f1).dLong /= (f2))
      #define file4longSubtract( f1, val ) ( (*(f1)).dLong -= (val) )
      #define file4longSubtractLong( f1, f2 ) ( (*(f1)).dLong - (*(f2)).dLong )
      #define file4longSubtractLongLong( f1, f2 ) ( (*(f1)).dLong - (*(f2)).dLong )
   /*   #define file4longSubtractLongLong( FILE4LONG *f1, FILE4LONG f2 ) */

      #define file4longAdd( f1, val ) ( (*(f1)).dLong += (val) )
      #define file4longAddLong( f1, f2 ) ( (*(f1)).dLong += (*(f2)).dLong )
      #define file4longAssignError( f2 ) ( (f2).piece.longHi = -1 )
      #define file4longError( f2 ) ( (unsigned long)(f2).piece.longHi )
      #define file4longGetHiAddress( f1 ) (&((f1).piece.longHi))
      #define file4longCheckError( f1 ) ( ((f1).piece.longLo == -1 ) ? ( ( GetLastError() != NO_ERROR ) ? ( (f1).piece.longHi = -1 ) : 0 ) : 0 )
   #else
      #define FILE4LONG long

      #define file4longAdd( f1, val ) ( *(f1) += (val) )
      #define file4longAddLong( f1, f2 ) ( (*(f1)) += (*(f2)) )
      #define file4longAssign( f2, longLoIn, longHiIn ) ((f2) = (longLoIn) + (longHiIn<<32))
      #define file4longAssignError( f2 ) ( (f2) = (unsigned long)-1L )
      #define file4longAssignLong( f1, f2 ) ( (f1) = (f2) )
      #define file4longCheckError( f1 )
      #define file4longCmp( f1, f2 ) ( (f1) == (f2)  ? 0 : 1 )
      #define file4longError( f2 ) ( f2 )
      #define file4longGetHi( f1 ) ( f1 >> 32 )
      #define file4longGetHiAddress( f1 ) (0)
      #define file4longGetLo( f1 ) ( f1 & 0x00000000FFFFFFFF )
      #define file4longGreater( f1, val ) ( (f1) > (val) )
      #define file4longGreaterLong( f1, f2 ) ( (f1) > (f2) )
      #define file4longGreaterZero( f1 ) ( (f1) > 0 )
      #define file4longLess( f1, val ) ( (f1) < (val) )
      #define file4longLessEq( f1, val ) ( (f1) <= (val) )
      #define file4longMultiply( f1, f2 ) ( (f1) *= (f2) )
      #define file4longSetLo( f1, val ) ( (f1) = (val) )
      #define file4longSubtract( f1, val ) ( *(f1) -= (val) )
      #define file4longSubtractLong( f1, f2 ) ( (f1) - (f2) )
      #define file4longDivide( f1, f2 ) ( f1 /= (f2))
   #endif
#else
   #define FILE4LONG unsigned long

   #define file4longAdd( f1, val ) ( *(f1) += (val) )
   #define file4longAddLong( f1, f2 ) ( (*(f1)) += (*(f2)) )
   #define file4longAssign( f2, longLoIn, longHiIn ) ((f2) = (longLoIn))
   #define file4longAssignError( f2 ) ( (f2) = (unsigned long)-1L )
   #define file4longAssignLong( f1, f2 ) ( (f1) = (f2) )
   #define file4longCheckError( f1 )
   #define file4longCmp( f1, f2 ) ( (f1) == (f2)  ? 0 : 1 )
   #define file4longError( f2 ) ( f2 )
   #define file4longGetHi( f1 ) ( 0 )
   #define file4longGetHiAddress( f1 ) (0)
   #define file4longGetLo( f1 ) ( f1 )
   #define file4longGreater( f1, val ) ( (f1) > (val) )
   #define file4longGreaterLong( f1, f2 ) ( (f1) > (f2) )
   #define file4longGreaterZero( f1 ) ( (f1) > 0 )
   #define file4longLess( f1, val ) ( (f1) < (val) )
   #define file4longLessEq( f1, val ) ( (f1) <= (val) )
   #define file4longMultiply( f1, f2 ) ( (f1) *= (f2) )
   #define file4longSetLo( f1, val ) ( (f1) = (val) )
   #define file4longSubtract( f1, val ) ( *(f1) -= (val) )
   #define file4longSubtractLong( f1, f2 ) ( (f1) - (f2) )
   #define file4longDivide( f1, f2 ) ( f1 /= (f2))
#endif /* S4FILE_EXTENDED */

   /* structure whose size, when added to a FILE4LONG, will be 10 */
typedef struct FILE4LONG_EXTENDSt
{
   char space[10 - sizeof( FILE4LONG )] ;
} FILE4LONG_EXTEND ;

#ifndef S4INTERNAL_COMPILE_CHECK
   #define file4len( f4 ) ( file4longGetLo( file4lenLow( f4 ) ) )
#endif

#ifdef S4CBPP
class S4CLASS FILE4
{
public:
#else
typedef struct FILE4St
{
#endif
   #ifndef S4OFF_OPTIMIZE
      LINK4 link ;           /* set to 0 if file not optimized */
   #else
      LINK4 space2b ;
   #endif
   /* accessMode and isReadOnly both on indicate 'r' attribute on file */
   /* isReadOnly only on indicates user access is limited to read only */
   /* if accessMode, will do full bufferring, if isReadOnly, will avoid */
   /* performing any disk writes */

   char doAllocFree ;
   char isTemp ;             /* True if it is a temporary file */

   #ifdef __WIN32
      HANDLE hand ;
   #else
      #ifdef S4WIN16
         HFILE hand ;
      #else
         int hand ;
      #endif
   #endif

   int isReadOnly ;          /* True if file is read only */

   char S4PTR *nameBuf ;

   S4CONST char S4PTR *name ;

   #ifdef S4MACINTOSH
      struct FSSpec macSpec ;
   #endif

   #ifdef S4CBPP
      CODE4 S4PTR *codeBase ;
   #else
      S4CONV( struct CODE4St *codeBase, struct CODE4St *code_base ) ;
   #endif

   #ifndef S4OFF_MULTI
      int lowAccessMode ;       /* open access level */
   #else
      int space25 ;
   #endif

   #ifdef S4FILE_EXTENDED
      char isLong ;
   #else
      char space26 ;
   #endif
   #ifndef S4OFF_OPTIMIZE
      char fileCreated ;     /* false if the file has not been created yet - i.e. if a memory-only file */
      char writeBuffer ;    /* buffer writes where possible */
      long hashInit ;
      FILE4LONG len ;            /* internal if optimized */
      FILE4LONG_EXTEND space0 ;
      char type ;           /* dbf, index, other */
      char bufferWrites ;   /* are writes being bufferred? */
      int  doBuffer ;       /* is the file bufferring on? */
      long expectedReadSize ;  /* record length */
      double hitCountAdd ;
      const void *ownerPtr ;        /* points to DATA4FILE/INDEX4FILE which holds file handle */
   #else
      char  space1 ;
      char  space2 ;
      long  space3 ;
      FILE4LONG  space4 ;
      FILE4LONG_EXTEND space4a;
      char  space5 ;
      char  space6 ;
      int   space7 ;
      long  space11 ;
      double space12 ;
      const void *space13 ;
   #endif

   #ifdef S4MULTI_THREAD
      CRITICAL_SECTION critical4file ;
   #else
      #ifdef __WIN32
         CRITICAL_SECTION space29 ;
      #endif
   #endif

   #ifdef S4WRITE_DELAY
      LIST4 delayWriteFileList ;
   #else
      LIST4 space28 ;
   #endif

   #ifdef S4READ_ADVANCE
      LIST4 advanceReadFileList ;
      char *advanceReadBuf ;        /* a buffer for advanced-reading */
      int advanceReadBufStatus ;    /* the status of the advance-read buffer */
                                    /* AR4EMPTY, AR4IN_USE, AR4FULL */
      unsigned advanceReadBufPos ;   /* we don't support > 4 gigs for advance reading */
      unsigned advanceReadBufLen ;
   #else
      LIST4 space30 ;
      char *space31 ;
      int space32 ;
      unsigned space33 ;
      unsigned space34 ;
   #endif

   #ifdef E4ANALYZE_ALL
      char dupName[L_tmpnam] ;
      int hasDup ;
      int inUse ;
/*
   #else
      this caused a problem because L_tmpnam differs between
      Borland and Microsoft compilers
      char space8[L_tmpnam] ;
      int space9 ;
      int space10 ;
*/
   #endif
#ifdef S4CBPP
} ;
#else
} FILE4 ;
#endif

#ifdef S4WRITE_DELAY
typedef void S4PTR * S4DELAY_PARM ;
typedef void S4CALL S4DELAY_FUNCTION( S4DELAY_PARM ) ;

typedef struct
{
   LINK4 link ;
   LINK4 fileLink ;    /* for the given file, all links on delay-chain */
   FILE4 *file ;
   const char *data ;
   unsigned pos ;   /* we don't support files > 4 gigs for delay-writing */
   unsigned len ;
   int usageFlag ;
   int status ;

   S4DELAY_FUNCTION *completionRoutine ;
   void *completionData ;   /* routine-specific data for use in completion routine */
} FILE4WRITE_DELAY ;
#endif

#ifdef S4READ_ADVANCE
typedef void S4PTR * S4ADVANCE_PARM ;
typedef void S4CALL S4ADVANCE_FUNCTION( S4ADVANCE_PARM ) ;

typedef struct
{
   LINK4 link ;
   LINK4 fileLink ;    /* for the given file, all links on delay-chain */
   FILE4 *file ;
   char *data ;
   unsigned pos ;   /* we don't support files > 4 gigs for delay-writing */
   unsigned len ;
   int usageFlag ;
   unsigned status ;

   S4ADVANCE_FUNCTION *completionRoutine ;
   void *completionData ;   /* routine-specific data for use in completion routine */
} FILE4ADVANCE_READ ;
#endif

typedef struct
{
   /* structure must remain fixed positions */
   S4LONG transId ;
   S4LONG clientId ;
   S4LONG clientDataId ;
   S4LONG serverDataId ;
   short int type ;
   unsigned S4LONG dataLen ;
   time_t    time ;
} LOG4HEADER ;

typedef struct S4CLASS TRAN4St
{
   /* transaction capabilities for a client */
   struct CODE4TRANSSt *c4trans ;
   LIST4 *dataList ;        /* A list of open data files. */
   LIST4 localDataList ;
   char dateFormat[LEN4DATE_FORMAT];          /* Longest is September 15, 1990 */

   #ifndef S4OFF_WRITE
      int currentTranStatus ;  /* is there a current transaction? */
   #else
      int space1 ;
   #endif
   #ifndef S4OFF_MULTI
      short unlockAuto ;        /* automatic unlocking on or off? */
      short savedUnlockAuto ;
      SINGLE4DISTANT toLock ;
      SINGLE4 locks ;           /* list of Lock4's */
   #else
      short space2 ;
      short space3 ;
      SINGLE4 space4 ;
   #endif

   #ifndef S4OFF_WRITE
      /* given transaction details */
      long       transId ;
      unsigned long pos ;       /* used during examination processing */
      LOG4HEADER header ;
      unsigned   dataPos ;
/*      char      *charId ;*/
   #else
      long       space6 ;
      long       space7 ;    /* used during examination processing */
      LOG4HEADER space8 ;
      char      *space9 ;    /* used during transaction writing and examination processing */
      unsigned   space10 ;
      unsigned   space11 ;
      char      *space12 ;
   #endif

   #ifdef S4STAND_ALONE
      int userIdNo ;    /* used to determine the id of user */
   #endif

   LIST4 closedDataFiles ;  /* during a transaction */

      char userId[LEN4USERID+1] ;
      char netId[LEN4NETID+1] ;
} TRAN4 ;

typedef struct S4CLASS CODE4TRANSSt
{
   #ifdef S4CBPP
      CODE4 S4PTR *c4 ;
   #else
      struct CODE4St *c4 ;
   #endif

   #ifndef S4OFF_TRAN
      int enabled ;
      #ifndef S4OFF_WRITE
            struct TRAN4FILESt *transFile ;
      #else
         struct TRAN4FILESt *space1 ;
      #endif
   #else
      int space2 ;
      struct TRAN4FILESt *space1 ;
   #endif
      TRAN4 trans ;
} CODE4TRANS ;

// AS 6/10.98 cannot be enumerator because Borland Builder mismatches the DLL
//enum TRAN4FILE_STATUS
//{
//   tran4notRollbackOrCommit,
//   tran4rollbackOrCommit
//} ;

typedef struct TRAN4FILESt
{
   long   transId ;
   long   validState ;    /* true/false switch */
   unsigned long fileLocks ;   /* bit encoded, bit i == byte TRAN4LOCK_BASE+i locked */
   FILE4  file ;
   int status ;

   #ifndef S4OFF_OPTIMIZE
      #ifndef S4OFF_WRITE
         #ifndef S4OFF_TRAN
            int needsFlushing ;
         #endif
      #endif
   #endif

   CODE4TRANS *c4trans ;
} TRAN4FILE ;

#ifndef S4OFF_CATALOG
typedef struct CATALOG4St
{
   struct DATA4St *data ;
   int valid ;
   int catalogAdd ;
   int catalogStatus ;
   struct FIELD4St *ownerFld, *aliasFld, *pathNameFld, *typeFld, *readOnlyFld,
                    *indexOpenFld, *createFld, *openModeFld, *logFld ;
   struct TAG4St *aliasTag, *pathTag ;
} CATALOG4 ;
#endif

#ifndef S4OFF_THREAD
typedef struct SEMAPHORE4St
{
   LINK4 link ;   /* Allows the SEMAPHORE4 to be on a list of SEMAPHORE3s */
   #ifdef __WIN32
      HANDLE handle ; /* Handle to the semaphore itself */
   #endif
} SEMAPHORE4 ;
#endif

#ifndef S4STAND_ALONE

#define C4ADDRESS struct sockaddr_in
#define C4NETID struct in_addr

#ifndef S4OFF_THREAD

struct INTER4St ;
struct SIGNAL4ROUTINESt ;

typedef void S4CALL COMPLETION4ROUTINE( struct INTER4St *, struct SIGNAL4ROUTINESt * ) ;

typedef struct SIGNAL4ROUTINESt
{
   LINK4 link ;  /* Allows the SIGNAL4ROUTINE to be on a list */
   HANDLE handle ;
   /* routine to call when the handle is signalled
      the routine is called with the SIGNAL4ROUTINE parameter as the 2nd
      parameter. */
   COMPLETION4ROUTINE *routine ;
   void *data ;   /* a pointer to some additional data the routine may need */

   /* pointer to the signal structure associated with the routine.
      may be either an EVENT4 or a SEMAPHORE4 */
   void *signalStructure ;

   int inArray ; /* indicates whether or not this SIGNAL4ROUTINE is in an SIGNAL4ARRAY */
   int arrayPosition ; /* sometimes set to current position in array */
} SIGNAL4ROUTINE ;

typedef struct
{
   HANDLE *handleArray;     /* the handle array to wait on */
   SIGNAL4ROUTINE **signalRoutineArray ; /* The signal array corresponding to the handle array */
   int numActiveElements ;
   int permenants ;
   int maxElements ;
} SIGNAL4ROUTINE_ARRAY ;

typedef struct
{
   LINK4 link ;       /* allows the EVENT4 to be on a list of EVENT4s*/

   #ifdef __WIN32
      HANDLE handle ; /* a handle to the event itself */
   #endif
} EVENT4 ;

typedef struct LIST4MUTEXSt
{
   LIST4 list ;
   HANDLE mutex ;
   char isValid ;
} LIST4MUTEX ;

typedef struct INTER4St
{
   #ifdef S4CBPP
      CODE4 S4PTR *cb ;
   #else
      struct CODE4St *cb ;
   #endif
   /* A list of NET4MESSAGEs which worker threads have requested to be */
   /* asynchronously read, and for which the inter thread needs to */
   /* perform (but has not yet performed) the asynchronous read */
   LIST4MUTEX readsRequested ;

   /* Semaphore count is equal to the number of reads requested ( i.e the */
   /* number of links in readRequested above ) */
   SEMAPHORE4 readsRequestedSemaphore ;

   /* readsRequestedRoutine is the routine structure which results in the */
   /* appropriate routine being calles every time the readsRequestedSemaphore*/
   /* above gets signalled */
   SIGNAL4ROUTINE readsRequestedRoutine ;

   /* a list of NET4MESSAGEs which worker threads have requested to be */
   /* asynchronously written, and for which the inter thread needs */
   /* to perform (but has not yet performed) the asynchronous write */
   LIST4MUTEX writesRequested ;

   /*Semaphore count is equal to the number of writes requested (i.e. the*/
   /*number of links in writesRequested above )  */
   SEMAPHORE4 writesRequestedSemaphore ;

   /* writesRequestedRoutine is the routine which results in the */
   /* appropriate routine being called every time the writesRequestedSemaphore */
   /* above gets signalled */
   SIGNAL4ROUTINE writesRequestedRoutine ;

   /* a list of CONNECT4THREADs which worker threads have finished servicing */
   /* and for which the communications thread needs to put back into the */
   /* pool of connections which are not being serviced by a worker thread */
   LIST4MUTEX completesRequested ;

   /* Semaphore count is equal to the number of completes requested (i.e the
      number of links in completesRequested); */
   SEMAPHORE4 completesRequestedSemaphore ;

   /* completesRequestedRoutine is the routine structure which results in the
      appropriate routine being called every time the
      completesRequestedSemaphore above gets signalled */
   SIGNAL4ROUTINE completesRequestedRoutine ;

   /* This event gets set when the inter thread is to be shut down
      i.e. called form code4initUndo() */
   EVENT4 shutdownThreadEvent ;

   /* routine to call when the thread should be shut down */
   SIGNAL4ROUTINE shutdownThreadRoutine ;

   /* the communications thread waits on all of the signals in the array.
      When signalled, the signalArray is used to directly call a function
      which was previously associated with the event. This is more
      optimal than using a switch statement to determine which function to
      call based on the signalled event or semaphore */
   SIGNAL4ROUTINE_ARRAY signalArray ;

   EVENT4 *finishedShutdownThreadPtr ;
   MEM4 S4PTR *signalRoutineMemory ; /* For SIGNAL4ROUTINES */
} INTER4 ;
#endif /*!S4OFF_THREAD */


typedef struct
{
   unsigned char tcpAddress[4] ;
} TCP4ADDRESS ;


struct CONNECT4St ;
struct CONNECT4BUFFERSt ;

#ifndef S4OFF_THREAD
typedef struct CONNECT4THREADSt
{
   LINK4 link ;

   #ifdef S4CBPP
      CODE4 S4PTR *cb ;
   #else
      struct CODE4St *cb ;
   #endif
   INTER4 *inter ;/* pointer to interchange thread info */

   struct CONNECT4BUFFERSt *connectBuffer ;

   /*List of NET4MESSAGEs which ReadFile()s completed */
   LIST4MUTEX readMessageCompletedList ;

   /* Equal to # elements on readMessageCompletedList, released by the
      communications thread when a read request has been completed */
   SEMAPHORE4 readMessageCompletedListSemaphore ;

   /* writes in asynchronous WriteFile(), used usually by communications
      thread. Kept here in case a cancel is requested (eg. blastDone)
      list of outstanding SIGNAL4ROUTINEs */
   LIST4 writingSignalsList ;

   /*These next 2 lists are required in case we need to shut down the
     connection, so we can remove the events */

   LIST4 readSignalsList ;   /*list of SIGNAL4ROUTINES waiting for data arrival */
   /* List of NET4MESSAGES which could not be performed due to too many
      already outstanding messages on the socket */
   LIST4 writeMessageOutstandingList ;

   LIST4MUTEX writeMessageCompletedList ; /* writes completed */

   /* Set when interchange thread is finished servicing a
      com4threadWriteCompleted() (i.e. when a write is complete)
      semaphore count equal to number links in writeMessageCompletedList
      above */
   SEMAPHORE4 writeMessageCompletedListSemaphore ;

   /* Set when communications thread is finished
      inter4threadCompleteRequest() */
   SEMAPHORE4 completedSemaphore ;

} CONNECT4THREAD ;

typedef struct
{
   LINK4 link ;
   #ifdef __WIN32
      OVERLAPPED overlap ; /* also used as a LINK4 when on the avail list */
   #endif
   CONNECT4THREAD *connectThread ;  /* Conenction associated with this message */
   long messageLen ;
      /* message array length will be equal to CODE4.readMessageBufferLen,*/
      /* which is configurable in the server configuration file and will */
      /* have a #define default for the client (S4READ_MESSAGE_DEFAULT_LEN*/
   long messageBufferLen ;    /* Length of the buffer */
   char message[1] ;
} NET4MESSAGE ;
#endif /*!S4OFF_THREAD */


typedef struct CONNECT4BUFFERSt
{
   LINK4 link ;
   #ifdef S4CBPP
      CODE4 S4PTR *cb ;
   #else
      struct CODE4St *cb ;
   #endif

   int connected ;
   CONNECT4LOW *connectLow ;

   /* If CONNECT4 is null, it means that this CONNECT4THREAD is not */
   /* bound to a connection which requires a worker thread (i.e. it is an */
   /* auxillary connection) */
   struct CONNECT4St *connect ;

   int communicationError ;
   #ifndef S4OFF_THREAD
      CONNECT4THREAD connectThread ;

      int canShareWriteBuffers ;
      int maxWritesOutstanding ;
      int advanceReads ;
      int readSize ;
      short type ;
      short id ;

      NET4MESSAGE *workingReadMessage ;
      long workingReadMessagePos ;
      NET4MESSAGE *messageReadArray ;
      NET4MESSAGE *workingWriteMessage ;
   #else
      char *workingWriteBuffer ;
      unsigned long workingWriteLen ;
      unsigned long workingWritePos ;
   #endif
} CONNECT4BUFFER ;

typedef struct CONNECT4St
{
   LINK4 link ;
   #ifdef S4CBPP
      CODE4 S4PTR *cb ;
   #else
      struct CODE4St *cb ;
   #endif

   /* sturcture used to communicate with the communications thread */
   CONNECT4BUFFER connectBuffer ;

   /* some identification method used by the client to identify the server
      side of the connection is required...
      just use the pointer from the server. Then on the server side,
      skip through a list of connections (to ensure memory integrity */
   /* void *id ; */
      C4ADDRESS address ;
      short addrLen ;
      long clientId ;
   /* int connected ; */ /* Is this NOT in the process of disconnecting?*/
} CONNECT4 ;

typedef struct PACKET4St
{
   short type ;
   short status ;   /* function rc */

   S4LONG int dataLen ;
   S4LONG errCode2 ;
   S4LONG clientDataId ;   /* which client-data4 id is being referred to? */
   S4LONG serverDataId ;   /* which server-data4 id is being referred to? */

   short readLock ;
   short unlockAuto ;
/*   short requestLockedInfo ; */
/*   short numRepeat ; */

   S4LONG delayHundredthsIn ;
} PACKET4 ;

typedef struct CONNECTION4St
{
   LINK4 link ;

   CONNECT4 *connect ;

   int connected ;
   unsigned short dataLen ;

   short connectionFailure ;    /* true if a communications failure occurs */

   #ifdef S4CBPP
      CODE4 S4PTR *cb ;
   #else
      struct CODE4St *cb ;
   #endif

   PACKET4 packet ;

   char *buffer ;
   long bufferLen ;
   long curBufferLen ;

   #ifdef E4ANALYZE
      short initUndone ;
   #endif
} CONNECTION4 ;

#endif /* S4STAND_ALONE */


#ifdef S4CBPP
class S4CLASS CODE4
{
public:
#else
typedef struct CODE4St
{
#endif
   /* documented members... (except ifdef'ed ones) */

   S4CONV( int autoOpen, int auto_open ) ;               /* Automatic production index file opening */
   int createTemp ;                                      /* create files as temporary ? */
   S4CONV( short errDefaultUnique, short default_unique_error ) ; /* e4unique, r4unique, r4uniqueContinue */
   S4CONV( int errExpr, int expr_error ) ;
   S4CONV( int errFieldName, int field_name_error ) ;
   S4CONV( int errOff, int off_error ) ;                 /* Show error messages? */
   S4CONV( int errOpen, int open_error ) ;               /* Do 'file4open' error ? */
   short log ;
   S4CONV( int memExpandData, int mem_expand_data ) ;          /* Expanding data allocation */
   S4CONV( unsigned memSizeBuffer, unsigned mem_size_buffer ) ;/* Pack, Zap */
   S4CONV( unsigned memSizeSortBuffer, unsigned mem_size_sort_buffer ) ; /* The default file buffer size when sorting */
   S4CONV( unsigned memSizeSortPool, unsigned mem_size_sort_pool ) ;   /* The default pool size for sorting */
   S4CONV( unsigned memStartData, unsigned mem_start_data ) ;      /* Initial data allocation */
   char safety ;                                          /* File create with safety ? */
   long timeout ;
   short int compatibility ;                                   /* FoxPro compatibility with... 25, 26, 30 */
   #ifdef __WIN32 /* documented member */
      void S4PTR *hWnd;            /* For use under Microsoft Windows */
   #else
      unsigned hWnd ;              /* For use under Microsoft Windows */
   #endif

   /* undocumented members... */

   int s4cr2 ;                                            /* used by CodeReporter */

   int initialized ;                                      /* only allow a single initUndo */
   int numericStrLen ;                                    /* the default length for clipper index files */
   int decimals ;                                         /* the default # decimals for clipper index files */

   int memExpandDataFile ;
   unsigned memStartDataFile ;

   unsigned exprBufLen ;        /* for exprWorkBuf */
   unsigned storedKeyLen ;      /* for storedKeu */
   unsigned bufLen ;            /* for fieldBuffer */

   char S4PTR *exprWorkBuf ;    /* used by expression parsing */
   char S4PTR *storedKey ;      /* used by the expr4key() function */
   char S4PTR *fieldBuffer ;
/*   char S4PTR *version ;   I don't think this is used anywhere anymore... */

   char indexExtension[4] ;

   MEM4 S4PTR *bitmapMemory ;
   MEM4 S4PTR *dataMemory ;
   S4CONV( MEM4 S4PTR *calcMemory, MEM4 S4PTR *calc_memory ) ;
   MEM4 S4PTR *data4fileMemory ;
   MEM4 S4PTR *relateDataListMemory ;  /* for R4DATA_LIST */
   MEM4 S4PTR *relateListMemory ;  /* for RELATE4LIST */
   MEM4 S4PTR *relateMemory ;
   MEM4 S4PTR *relationMemory ;
   MEM4 S4PTR *dataListMemory ;  /* for DATA4LIST */

   LIST4 dataFileList ;         /* A list of open data files. */

   FILE4 *errorLog ;             /* log file for error messages */

   int collatingSequence ;   /* the collating sequence to use when creating a VFP tag */
   int codePage ;            /* the codepage to use when creating a database file */

   int didAlloc ;  /* true if code4alloc() called, in which case we auto-free it */

   char oledbSchemaCreate ;
   long largeFileOffset ;

   #ifdef S4COMPILE_TEST
      #ifndef S4OFF_MULTI
         S4CONV( char accessMode, char exclusive ) ;             /* how should files be opened? */
         int memExpandLock ;       /* Expanding lock memory allocation */
         unsigned memStartLock ;   /* Initial lock memory allocation record locks */
         int lockAttemptsSingle ;                               /* How many times to attempt each lock in a group lock */
         unsigned int lockDelay ;
         int fileFlush ;  /* force hard file flush during write */
            S4CONV( char readLock, char read_lock ) ;               /* Do lock when reading database ? */
         S4CONV( int lockAttempts, int lock_attempts ) ;        /* How many times to attempt locks. */
         MEM4 S4PTR *lockMemory ;
         MEM4 S4PTR *lockLinkMemory ;
      #endif
      #ifndef S4OFF_OPTIMIZE
         S4CONV( int optimizeWrite, int optimize_write ) ;
         S4CONV( unsigned memSizeBlock, unsigned mem_size_block ) ;  /* Block size (bytes) for memo and index files */
         int optimize ;            /* should files be automatically bufferred? */
         int memMaxPercent ;
         S4CONV( long memStartMax, long mem_start_max ) ;
      #endif
      #ifndef S4OFF_INDEX
         #ifdef S4OFF_OPTIMIZE
            S4CONV( unsigned memSizeBlock, unsigned mem_size_block ) ;  /* Block size (bytes) for memo and index files */
         #endif
         S4CONV( int errTagName, int tag_name_error ) ;
         S4CONV( int memExpandBlock, int mem_expand_block ) ;        /* Expanding block memory allocation */
         S4CONV( int memExpandIndex, int mem_expand_index ) ;        /* Expanding index allocation */
         S4CONV( int memExpandTag, int mem_expand_tag ) ;            /* Expanding tag allocation */
         S4CONV( unsigned memStartBlock, unsigned mem_start_block ) ;     /* Initial block memory allocation for index files */
         S4CONV( unsigned memStartIndex, unsigned mem_start_index ) ;     /* Initial index file allocation */
         S4CONV( unsigned memStartTag, unsigned mem_start_tag ) ;       /* Initial tag allocation */
         int memExpandTagFile ;
         unsigned memStartTagFile ;
         MEM4 S4PTR *indexMemory ;
         MEM4 S4PTR *index4fileMemory ;
         MEM4 S4PTR *tagMemory ;
         MEM4 S4PTR *tagFileMemory ;
      #endif
      #ifndef S4OFF_MEMO
         S4CONV( unsigned memSizeMemo, unsigned mem_size_memo ) ;
         S4CONV( unsigned memSizeMemoExpr, unsigned mem_size_memo_expr ) ;
      #endif
      #ifndef S4OFF_TRAN
         int doTrans ;                                         /* Maintain a transaction file? */
      #endif
   #else
      S4CONV( char accessMode, char exclusive ) ;             /* how should files be opened? */
      int memExpandLock ;       /* Expanding lock memory allocation */
      unsigned memStartLock ;   /* Initial lock memory allocation record locks */
      int lockAttemptsSingle ;                               /* How many times to attempt each lock in a group lock */
      unsigned int lockDelay ;
      int fileFlush ;  /* force hard file flush during write */
         S4CONV( char readLock, char read_lock ) ;               /* Do lock when reading database ? */
      S4CONV( int lockAttempts, int lock_attempts ) ;        /* How many times to attempt locks. */
      MEM4 S4PTR *lockMemory ;
      MEM4 S4PTR *lockLinkMemory ;
      S4CONV( int optimizeWrite, int optimize_write ) ;
      int optimize ;            /* should files be automatically bufferred? */
      int memMaxPercent ;
      S4CONV( long memStartMax, long mem_start_max ) ;
      S4CONV( unsigned memSizeBlock, unsigned mem_size_block ) ;  /* Block size (bytes) for memo and index files */
      S4CONV( int errTagName, int tag_name_error ) ;
      S4CONV( int memExpandBlock, int mem_expand_block ) ;        /* Expanding block memory allocation */
      S4CONV( int memExpandIndex, int mem_expand_index ) ;        /* Expanding index allocation */
      S4CONV( int memExpandTag, int mem_expand_tag ) ;            /* Expanding tag allocation */
      S4CONV( unsigned memStartBlock, unsigned mem_start_block ) ;     /* Initial block memory allocation for index files */
      S4CONV( unsigned memStartIndex, unsigned mem_start_index ) ;     /* Initial index file allocation */
      S4CONV( unsigned memStartTag, unsigned mem_start_tag ) ;       /* Initial tag allocation */
      int memExpandTagFile ;
      unsigned memStartTagFile ;
      MEM4 S4PTR *indexMemory ;
      MEM4 S4PTR *index4fileMemory ;
      MEM4 S4PTR *tagMemory ;
      MEM4 S4PTR *tagFileMemory ;
      S4CONV( unsigned memSizeMemo, unsigned mem_size_memo ) ;
      S4CONV( unsigned memSizeMemoExpr, unsigned mem_size_memo_expr ) ;
      int doTrans ;                                         /* Maintain a transaction file? */
   #endif

   #ifdef S4MACINTOSH
      long macDir ;
      int macVol ;
   #endif

   #ifdef E4ANALYZE
      int debugInt ;               /* used to check structure integrity (set to 0x5281) */
   #else
      #ifdef S4VBASIC
         int debugInt ;
      #endif
   #endif

   #ifndef S4CLIPPER
      unsigned memStartIndexFile ; /* Initial index file allocation */
      int memExpandIndexFile ;  /* Expanding index file allocation */
   #endif

   #ifndef S4DOS
      #ifdef __WIN32
         void S4PTR *hInst ;
         void S4PTR *hInstLocal ;
      #else
         unsigned hInst ;
         unsigned hInstLocal ;
      #endif
/*      char protocol[LEN4PROTOCOL+1] ; */
   #endif

   #ifndef S4OFF_WRITE
      char *transFileName ;
   #endif

   #ifndef S4OFF_CATALOG
         CATALOG4 S4PTR *catalog ;
   #endif

   #ifndef S4OFF_TRAN
      char *tranData ;   /* buffer used to hold transaction entries */
      unsigned int tranDataLen ;  /* length of transaction entry buffer */
   #endif

   #ifdef S4MNDX
      char *memoUseBuffer ;
   #endif

   #ifndef S4OFF_OPTIMIZE
      OPT4 opt ;
      int hasOpt, doOpt, hadOpt ;
   #endif

   int doRemove ;

      S4CONV( int errCreate, int create_error ) ;           /* Do 'file4create' error ? */
      S4CONV( int errorCode, int error_code ) ;
      S4CONV( int readOnly, int read_only ) ;
      long errorCode2 ;
      /* singleOpen can take 0,1, or 2, but only 0 and 1 are documented -OPEN4 modes */
      short singleOpen ;      /* only one instance of a DATA4 allowed (as opposed to exclusive which is access for client) */
      S4CONV( int errGo, int go_error ) ;                   /* Do 'd4go' error ? */
      S4CONV( int errRelate, int relate_error ) ;         /* do relate4terminate error when no match and relate4terminate selected */
      S4CONV( int errSkip, int skip_error ) ;           /* Do 'DataIndex::skip' error ? */
      int lockEnforce ;         /* are pre-locks required for record modifications? */

      char *lockedNetName ;
      char *lockedNetNameBuf ;
      char *lockedUserName ;
      char *lockedUserNameBuf ;
      char *lockedFileName ;
      char *lockedFileNameBuf ;
      unsigned int lockedFileNameLen ;
      unsigned int lockedUserNameLen ;
      unsigned int lockedNetNameLen ;
      long lockedLockItem ;

      CODE4TRANS c4trans ;
      S4CONV( MEM4 S4PTR *totalMemory, MEM4 S4PTR *total_memory ) ;
      S4CONV( LIST4 calcList, LIST4 calc_list ) ;
      S4CONV( LIST4 totalList, LIST4 total_list ) ;
      S4CONV( int numReports, int num_reports ) ;
      short pageno ;
      long clientDataCount ;

      int doIndexVerify ;       /* for internal purposes only at this point */
      struct RELATION4St S4PTR *currentRelation ;
      char savedKey[I4MAX_KEY_SIZE + 2 * sizeof(S4LONG)] ;       /* used by i4remove.c, i4tag.c and i4addtag.c, i4versionCheck, t4versionCheck */

   #ifdef S4OPTIMIZE_STATS
      struct DATA4St *statusDbf ;
      struct FIELD4St *typeFld, *fileNameFld, *offsetFld, *lengthFld ;
   #endif

   #ifdef S4STAND_ALONE
      #ifndef S4OFF_TRAN
         #ifndef S4OFF_WRITE
            TRAN4FILE transFile ;
            int logOpen ;
         #endif
      #endif

      #ifdef S4TESTING
         /* allow sequential handling of transaction file by not doing locking
            i.e. for same program to have multiple transactions */
         int doTransLocking ;
      #endif
   #else
      MEM4 S4PTR *connectBufferMemory ;
      MEM4 S4PTR *connectLowMemory ;
      MEM4 S4PTR *writeMemory ; /*Memory for NET4MESSAGE for writes Memory */
      #ifdef S4DISTRIBUTED
         LIST4 servers ;       /* list of connected servers */
      #endif

      int ver4 ;

      int readMessageBufferLen ;  /*Buffer size for communications reading */
      int readMessageNumBuffers ; /*# of buffers per connection for reading*/
      int writeMessageNumOutstanding ; /*# of outstanding writes per connection*/
      int writeMessageBufferLen ; /*buffer size for communications writing */

      #ifndef S4OFF_THREAD
         LIST4 eventsAvail ;        /* List of available EVENT4 objects */
         MEM4 S4PTR *eventMemory ;
         LIST4MUTEX writeBuffersAvail ; /* list of unused NET4MESSAG for writes */
         LIST4MUTEX connectBufferListMutex ;  /* list of CONNECT4BUFFERs */
      #endif

      #ifdef S4COMM_THREAD
         INTER4 *inter ;
         unsigned long interThreadHandle ;
      #endif
   #endif

   #ifdef S4WRITE_DELAY
      int delayWritesEnabled ;
      CRITICAL_SECTION critical4delayWriteList ;
      LIST4 delayWriteList ;
      MEM4 S4PTR *delayWriteMemory ;
      long uninitializeDelayWrite ;   /* request to uninitialize delay-writes */
      HANDLE initUndoDelayWrite ;    /* event for delay-write thread to notify main
                                       thread that uninitialization is complete */
      HANDLE pendingWriteEvent ;    /* event to notify delay-write thread that
                                       there is an action to perform */
   #else
      /* AS microsoft compiler 5.0 uses write-delay, others don't.  Must
         still access DLL ok, so pad out here */
      int dummy1 ;
      #ifdef __WIN32
         CRITICAL_SECTION dummy2 ;
      #endif
      LIST4 dummy3 ;
      MEM4 S4PTR *dummy4 ;
      long dummy5 ;   /* request to uninitialize delay-writes */
      HANDLE dummy6 ;    /* event for delay-write thread to notify main
                                       thread that uninitialization is complete */
      HANDLE dummy7 ;    /* event to notify delay-write thread that
                                       there is an action to perform */
   #endif

   #ifdef S4READ_ADVANCE
      int advanceReadsEnabled ;
      CRITICAL_SECTION critical4advanceReadList ;
      LIST4 advanceReadList ;
      MEM4 S4PTR *advanceReadMemory ;
      HANDLE initUndoAdvanceRead ;
      long uninitializeAdvanceRead ;
      HANDLE pendingReadEvent ;
   #else
      int dummy8 ;
      #ifdef __WIN32
         CRITICAL_SECTION dummy9 ;
      #endif
      LIST4 dummy10 ;
      MEM4 S4PTR *dummy11 ;
      HANDLE dummy12 ;
      long dummy13 ;
      HANDLE dummy14 ;
   #endif

   /*CJ- made change to facilate security issues for distribution of CodeBase DLLs */
   char *copyright;

#ifdef S4CBPP
} ;
#else
} CODE4 ;
#endif

#ifndef S4OFF_MEMO
typedef struct
{
   FILE4     file ;
   short      blockSize ;               /* Bytes */
   struct DATA4FILESt  S4PTR *data ;
   #ifndef S4OFF_MULTI
      int fileLock ;                  /* True if file is locked */
   #else
      int space1 ;
   #endif
} MEMO4FILE ;
#endif /* S4OFF_MEMO */

#ifdef S4CBPP
class S4CLASS FILE4SEQ_READ
{
public:
#else
typedef struct
{
#endif
   FILE4 S4PTR *file ;

   FILE4LONG pos ;          /* The next position to read from */
   FILE4LONG_EXTEND pad ;   /* pad for variations in FILE4LONG size */
   char S4PTR *buffer ;
   unsigned nextReadLen ;
   unsigned total ;     /* Total buffer length */
   unsigned working ;   /* Temporary working buffer length (to help align write) */
   unsigned avail ;     /* # of bytes currently available */
   #ifdef S4ADVANCE_READ
      int doAdvance ;
      char S4PTR *buf1 ;
      char S4PTR *buf2 ;
      unsigned int buf1status, buf2status ;
      int buf1avail ;
      int buf2avail ;
   #endif
#ifdef S4CBPP
} ;
#else
} FILE4SEQ_READ ;
#endif

#ifdef S4CBPP
class S4CLASS FILE4SEQ_WRITE
{
public:
#else
typedef struct
{
#endif
   FILE4 S4PTR *file ;

   FILE4LONG pos ;          /* The next position to read from */
   FILE4LONG_EXTEND pad ;   /* pad for variations in FILE4LONG size */
   char S4PTR *buffer ;
   unsigned total ;     /* Total buffer length */
   unsigned working ;   /* Temporary working buffer length (to help align write) */
   unsigned avail ;     /* # of bytes left in working buffer */
   #ifdef S4WRITE_DELAY
      char S4PTR *buf1 ;
      char S4PTR *buf2 ;
      unsigned bufLen ;
      unsigned buf2len ;
      int buf1avail ;
      int buf2avail ;
   #endif
#ifdef S4CBPP
} ;
#else
} FILE4SEQ_WRITE ;
#endif

typedef struct  /* Data File Format */
{
   char name[11] ;
   char type ;
   S4LONG offset ;    /* field's offset into record buffer (for vfp 3.0) */
   unsigned char len ;
   unsigned char dec ;
   #ifdef CLIENT_OR_FOX
      char nullBinary ;    /* for FOX 3.0 0x02 == allows null fields, 0x04 == binary field */
      char filler2[12] ;
   #else
      char filler2[13] ;
   #endif
   char hasTag ;
} FIELD4IMAGE ;

typedef struct
{
   unsigned short int lo[4] ;
/*   signed short int hi ;     significant bit in this field */
} CURRENCY4 ;

typedef struct FIELD4St /* Internal Structure and Field Routines. */
{
   char name[11] ;
   unsigned short int len ;
   unsigned short int dec ;
   short int type ;
   unsigned long offset ;   /* codebase allows long */
   struct DATA4St S4PTR *data ;

   /* #ifdef CLIENT_OR_FOX  */  /*These members are used in OLEDB*/
      char null ;
      unsigned short int nullBit ;   /* bit which indicates whether field contents currently are null */
      char binary ;
   /*#endif  */
   #ifndef S4OFF_MEMO
      struct F4MEMOSt S4PTR *memo ;
   #endif
   #ifdef S4VBASIC
      short int debugInt ;         /* used to check structure integrity (set to 0x5281) */
   #else
      short int space1 ;
   #endif
} FIELD4 ;

#ifndef S4OFF_MEMO
typedef struct F4MEMOSt
{
   S4CONV( int isChanged, int is_changed ) ;
   int status ;           /* 0 - Current contents, 1 - Unknown */
   char  S4PTR *contents ;
   unsigned int len ;
   S4CONV( unsigned int lenMax, unsigned int len_max ) ;
   FIELD4  S4PTR *field ;
} F4MEMO ;
#endif

typedef struct  /* Creating Data File */
{
   char S4PTR *name ;
   short int type ;
   unsigned short int len ;
   unsigned short int dec ;
   unsigned short int nulls ;  /* are nulls allowed? */
} FIELD4INFO ;


typedef struct
{
   /* Database Header Information */
   char     version ;
   char     yy ;             /* Last Update */
   char     mm ;
   char     dd ;
   S4LONG   numRecs ;
   unsigned short headerLen; /* Header Length, Indicates start of data */
   unsigned short recordLen;
   char     zero[16] ;
   char     hasMdxMemo ;    /* 0x01 for has mdx, in fox 0x02 for has memo */
   char     codePage ;
   char     zero2[2] ;
} DATA4HEADER_FULL ;

typedef struct DATA4FILESt
{
   LINK4 link ;

   S4CONV( unsigned int recWidth, unsigned int record_width ) ;

   unsigned short int infoLen ;

   int nFields ;         /* The number of data fields in the database */
   int nFieldsMemo ;     /* The number of memo fields in the database */

   long minCount ;
   long userCount ;      /* number of DATA4's that are using this DATA4FILE */

   CODE4 S4PTR *c4 ;
   char S4PTR *info ;

      char S4PTR *record ;

   #ifndef S4OFF_WRITE
      int doDate ;    /* does the date need to be updated on unlock/close? */
   #endif

   /**** the next set of lines must remain in order as they are a file view ****/
   /* Database Header Information */
   char version ;
      char     yy ;             /* Last Update */
      char     mm ;
      char     dd ;
   S4LONG   numRecs ;
   unsigned short headerLen ; /* Header Length, Indicates start of data */
   /**** the previous set of lines must remain in order as they are a file view ****/

      void S4PTR *space6 ;
      time_t space7 ;
      struct DATA4St S4PTR *space9 ;
      struct DATA4St S4PTR *space10 ;

      FILE4    file ;
      char hasMdxMemo ;        /* Has an MDX and/or a memo file attached to it */

      int fileChanged ;   /* True if the file has been changed since */
                                /* the header has been updated. */
      short    blockSize ;
      #ifndef S4OFF_MEMO
         MEMO4FILE   memoFile ;   /* Memo file handle */
      #endif

      int tranStatus ;   /* is this data4file using transactions? */
      char valid ;    /* if valid, a close on a datafile will not low-close it */
      #ifndef S4OFF_MULTI
         SINGLE4 lockedRecords ; /* list of LOCK4LINK's in client version, LOCK4's in stand-alone/server */
         long fileServerLock ;    /* which data holds file lock */
         long fileClientLock ;    /* which data holds file lock */
         long appendClientLock ;  /* True if the file is locked for appending */
         long appendServerLock ;  /* True if the file is locked for appending */

         long tempServerLock ;    /* for lock registering */
         long tempClientLock ;    /* for lock registering */
      #endif

   #ifndef S4OFF_OPTIMIZE
      int hiPrio ;   /* used to determine which priority lru list to put block data on and advance group-reads */
                     /* '1' if d4seek, -1 if d4skip */
   #endif

   char openMdx ;       /* does the datafile have an open production file attatched to it */

   #ifdef S4CLIPPER
      LIST4 tagfiles ;
      unsigned long indexLocked ;
   #else
      LIST4    indexes ;        /* list of INDEX4FILE;s */
      unsigned long space99 ;
   #endif
} DATA4FILE ;

typedef struct DATA4St
{
   LINK4 link ;

   int bofFlag ;
   int eofFlag ;
   int readOnly ;
   S4CONV( int recordChanged, int record_changed ) ; /* T/F */
   S4LONG count ;                /* a fairly current record count on the database */
   S4CONV( S4LONG recNum, S4LONG rec_num ) ;               /* Record number; -1 unknown; 0 for append */
   S4LONG recNumOld ;            /* Record number, -1 none present; 0 for append */

   char alias[LEN4DATA_ALIAS+1] ;

   S4CONV( CODE4 S4PTR *codeBase, CODE4 S4PTR *code_base ) ;

   TRAN4 S4PTR *trans ;

   FIELD4  S4PTR *fields ;       /* An array of field pointers */

   LIST4 indexes ;               /* list of INDEX4's */

   struct TAG4St *tagSelected ;

   int    codePage ;

   DATA4FILE *dataFile ;
   /* CJ- The DATA4FILE member of this structure must be at this point for
      CodeReporter to work.  Do not place any new members before this point.
      The next four allocations also must stay together so place new members
      after the declaration of "recordBlank" */

   char S4PTR *groupRecordAlloc ;   /* if the current and old record allocations
                                       allocated at the same time? */
   char S4PTR *record ;          /* Data allocated with 'u4alloc' */
   char S4PTR *recordOld ;       /* Data allocated with 'u4alloc' */
                                 /* Extra byte added for temporary CTRL_Z */

   char S4PTR *recordBlank ;  /* To allow fast-blanking of binary fields */

   #ifndef S4OFF_MEMO
      S4CONV( F4MEMO S4PTR *fieldsMemo, F4MEMO S4PTR *fields_memo ) ;    /* A list of fields to be flushed */
      #ifndef S4OFF_MULTI
         char memoValidated ; /* Can we be sure memo id #'s are up to date. */
      #endif
   #endif

   #ifndef S4OFF_TRAN
         int logVal ;
      char transChanged ;
   #endif

   #ifdef S4VBASIC
      int debugInt ;      /* used to check structure integrity (set to 0x5281) */
   #endif

      long clientId ;
} DATA4 ;

typedef void S4OPERATOR(void) ;

/* NOTE: IT IS CRITICAL THAT THE FOLLOWING STRUCTURE'S MEMBERS ARE NOT
   RE-ORDERED. */
typedef struct E4INFOSt
{
   short int fieldNo ;   /* field no in data structure -- since FIELD4 may be transient */
   FIELD4 S4PTR *fieldPtr ;
   int localData ;       /* true if the fieldPtr points to a local data4 */
   char S4PTR *p1 ;
   int len ;         /* Length */
   int numEntries ; /* Number of entries in sub-expression */
   int numParms ;
   int resultPos ;  /* The position in the result array for result. */
   int i1 ;          /* Could be constant position. 'i1' and 'resultPos'
                       and 'functionI' and 'function'
                       must be at last position due to memcmp() in e4isTag() */
   int functionI ;
   S4OPERATOR S4PTR *function ;
} E4INFO ;

typedef struct e4exprSt
{
   E4INFO S4PTR *info ;
   int infoN ;
   S4CONST char S4PTR *source ;
   char S4PTR *constants ;
   int len, type ;
   struct TAG4FILESt S4PTR *tagPtr ;
   #ifdef S4FOX
      struct T4VFPSt S4PTR *vfpInfo ;
   #else
      void S4PTR *space1 ;
   #endif
   DATA4 S4PTR *data ;
   DATA4FILE S4PTR *dataFile ;
   S4CONV( CODE4 S4PTR *codeBase, CODE4 S4PTR *code_base ) ;

   unsigned int lenEval ;     /* This is the length of the buffer needed for evaluation. */
   int numParms ;    /* This is the # of parameter positions used in evaluation. */
   char hasTrim ;    /* special case for key evaluation */
   #ifdef S4CLIPPER
      int keyDec, keyLen ;   /* used for CLIPPER version */
   #endif
} EXPR4 ;

#ifdef S4CLIPPER
   typedef struct
   {
      S4LONG  pointer ;    /* =0L if record, not pointer */
      S4LONG  num ;
      unsigned char  value[1] ;  /* The key size is variable */
   } B4KEY_DATA ;
#else
   typedef struct
   {
      S4LONG num ;
      unsigned char  value[1] ;  /* The key size is variable */
   } B4KEY_DATA ;
#endif

#ifdef S4FOX
/* the following structure is used only on the leaf nodes of the tree structure */
typedef struct
{
   short            freeSpace ;        /* # bytes available in node */
   unsigned char    recNumMask[4] ;      /* record number mask */
   unsigned char    dupByteCnt ;      /* duplicate byte mask count */
   unsigned char    trailByteCnt ;    /* Trailing byte mask count */
   unsigned char    recNumLen ;       /* # bits used for record number */
   unsigned char    dupCntLen ;       /* # bits used for duplicate count */
   unsigned char    trailCntLen ;     /* # bits used for trail count */
   unsigned char    infoLen ;          /* # bytes for holding record number, */
} B4NODE_HEADER ;

typedef struct
{
   short      nodeAttribute ;    /* 0=index, 1=root, 2=leaf */
   short      nKeys ;            /* Block Image starts here */
   S4LONG     leftNode ;         /* -1 if not present */
   S4LONG     rightNode ;        /* -1 if not present */
} B4STD_HEADER ;

typedef struct
{
   LINK4 link ;
   struct TAG4FILESt  S4PTR *tag ;

   int changed ;
   S4LONG  fileBlock ;     /* Identifies block within index file */
   int keyOn ;         /* The current key within the block */
   int curTrailCnt ;  /* current value used for seeking */
   int curDupCnt ;    /* current value used for seeking */
   int dupPos ;        /* bit offset into the info for the duplicate data */
   int trailPos ;      /* bit offset into the info for the trail data */
   int recPos ;        /* bit offset into the info for the record # data */
   char  *curPos ;       /* current position into the data (starts at end) */

   int builtOn ;       /* the 'current' key value (i.e. key really 'on') */
   char  S4PTR *builtPos ;     /* position where built on */
   B4KEY_DATA S4PTR *builtKey ;

   B4STD_HEADER header ;
   B4NODE_HEADER nodeHdr ;    /* only if the block is a leaf */
   char  data[1] ;        /* the remaining data */
} B4BLOCK ;

#endif  /* ifdef S4FOX  */

#ifndef S4FOX

#ifdef S4CLIPPER
   typedef struct
   {
      short          sign ;
      short          version ;
      S4LONG         root ;          /* Root Block */
      S4LONG         eof ;           /* First Free Block Pointer */
      short          groupLen ;     /* Key Length + 2*sizeof(long) */
      short          keyLen ;       /* Key Length */
      short          keyDec ;       /* Number of Decimals in Key */
      short          keysMax ;      /* Maximum # of keys per block;  <= 100 */
      short          keysHalf ;     /* Maximum # of keys per half block */

 /*   char           expression[256];   The index expression corresponding to the database. */
 /*   short          unique   ;         TRUE if Unique */
   } I4IND_HEAD_WRITE;
#endif

#ifdef S4CLIPPER
   typedef struct
   {
      short      oldVersion ;
      int headerOffset ;
      S4LONG     virtualEof ;   /* The next available file block */
      short      sign ;
      short      version ;
      S4LONG     root ;          /* Root Block */
      S4LONG     eof ;           /* First Free Block Pointer */
      short      groupLen ;     /* Key Length + 2*sizeof(long) */
      short      keyLen ;       /* Key Length */
      short      keyDec ;       /* Number of Decimals in Key */
      short      keysMax ;      /* Maximum # of keys per block;  <= 100 */
      short      keysHalf ;     /* Maximum # of keys per half block */
   /* char       expression[256] ;   The index expression corresponding to the database. */
      short      unique   ;      /* TRUE if Unique */
      short      descending ;     /* The descending flag corresponding to the index file */
   /* char       filter[256] ;   The filter(for) expression corresponding to the database. */
   }  T4HEADER ;
#else
   typedef struct
   {
      char   two ;                /* Version number (currently 2) */
      char   yymmdd[3] ;          /* Date of last reindex */
      char   dataName[12] ;      /* Name of associated data file */
      char   dummy1[4] ;          /* extra 4 bytes for data-names-not used in DOS */
      short  blockChunks ;       /* Block Size 1 to 32 (512 byte chunks) */
      short  blockRw ;           /* Block Read/Write Size in bytes */
      char   isProduction ;      /* 1 if production index, else 0 */
      char   numSlots ;          /* number possible tags (48) */
      short  slotSize ;          /* number bytes/tag slot (32) */
      short  numTags ;
      short  dummy2 ;
      S4LONG eof ;
      S4LONG freeList ;          /* start of the free list */
      char   zero[4] ;
      char   createDate[3];      /* not used by CodeBase */
      char   blank ;
   }  I4HEADER ;
#endif     /*   ifdef S4CLIPPER  */
#endif     /*   ifndef S4FOX  */

#ifndef S4FOX
typedef struct
{
   LINK4 link ;
   struct TAG4FILESt  S4PTR *tag ;

   S4LONG  fileBlock ;  /* Identifies block within index file */
   int changed ;
   int keyOn ;      /* The current key within the block */

   short      nKeys ; /* Block Image starts here */
   #ifdef S4CLIPPER
      short pointers[( B4BLOCK_SIZE / 2 - 1 )] ;
      B4KEY_DATA S4PTR *data ;
   #else
      char       dummy[6] ;
      B4KEY_DATA info ;
   #endif
} B4BLOCK ;

typedef struct
{
   S4LONG headerPos ;          /* Header position (in 512 byte chunks) */
   char  tag[10] ;
   short x1000 ;               /* used for dBASE/SQL expression type - dBASE only allowed for CBPP 1.0x */
   char  leftChld ;
   char  rightChld ;
   char  parent ;
   char  x2 ;
   char  indexType ;
   char  zeros[11] ;
} T4DESC ;
#endif     /* ifndef  S4FOX  */

#ifndef S4CLIPPER
typedef struct
{
#ifdef S4FOX
   S4LONG         root ;            /* -1 means unknown */
   S4LONG         freeList ;       /* start of the free list (-1 if none) */
   unsigned S4LONG version ;        /* used multi-user only */
   short          keyLen ;         /* Key Length */
   unsigned char  typeCode;        /* 0x01 Uniq; 0x08 For Clause; 0x32 Compact; 0x80 Compound */
   unsigned char  signature ;       /* unused */

/* char           dummy2[482] ;        unused */

   char           sortSeq[8] ;    /* collating sequence (ie GENERAL) */
   char           dummy3[4] ;
   short          descending   ;    /* 1 = descending, 0 = ascending */
   short          filterPos ;      /* not used, == to exprLen */
   short          filterLen ;      /* length of filter clause */
   short          exprPos ;        /* not used, == to 0  */
   short          exprLen ;        /* length of expression */
/* char           exprPool[512] ;  expression and filter pool */
#else
   S4LONG         root ;            /* -1 means unknown */
   char           dummy1[4] ;
   char           typeCode;        /* 0x10 Normal; 0x58 Uniq,Desc; 0x50 Uniq; 0x18 Desc */
   char           type ;            /* N,D, or C (F is type N)  */
   char           dummy2[2] ;
   short          keyLen ;
   short          keysMax ;        /* Maximum # of keys per block;  <= 100  */
   short          isDate ;       /* 1 if Date Key */
   short          groupLen ;       /* keyLen plus 4 (MDX); plus 8 (NDX) */
   unsigned char  version ;
   char           dummy4 ;
   short          unique   ;        /* 0x4000 (TRUE)if Unique */

   /* Note, 'exprKey[220]' comes after 'unique' and */
   /*       'exprFilter[220]' comes at position 0x2FA */
#endif
}  T4HEADER ;
#endif /* S4CLIPPER */

#ifdef S4FOX   /* info on codepage and collating sequence */
typedef struct T4VFPSt
{
   int              codePage ;    /* codepage of database file */
   int              sortType ;    /* collating seq of tag in index file */
   translatedChars  *tablePtr ;   /* points to translation table */
   compressedChars  *compPtr;     /* points to compressed char table */
   unsigned char    *cpPtr ;      /* points to codepage table */
} T4VFP ;
#endif /* S4FOX */

typedef struct TAG4FILESt
{
   LINK4   link ;
   #ifndef S4CLIPPER
      struct INDEX4FILESt S4PTR *indexFile ;
   #else
      void S4PTR *space1 ;
   #endif
   char    alias[LEN4TAG_ALIAS+1] ;
   CODE4 S4PTR *codeBase ;
   int debugInt ;  /* used to check structure integrity (set to 0x5281) */

   #ifdef S4CLIPPER
      int userCount ;
   #else
      int space2 ;
   #endif

      EXPR4   S4PTR  *expr ;
      EXPR4   S4PTR  *filter ;
      S4CMP_FUNCTION *cmp ;
      C4STOK S4PTR   *stok ;        /* Conversion for 'seek' */
      C4DTOK S4PTR   *dtok ;        /* Conversion for 'seek' */
      #ifdef S4MDX
         char         hadKeys ;
      #else
         char         space3 ;
      #endif
      char            hasKeys ;

      LIST4           blocks ;
      LIST4           saved ;

      #ifdef S4FOX
         char         pChar ;
         T4VFP     vfpInfo ;
      #else
         char         space4 ;
         char         space5 ;     /* modify appropriately when above workaround is combined */
      #endif

      #ifdef S4CLIPPER
         S4LONG       checkEof ;     /* used for debug purposes to verify eof length */
      #else
         S4LONG       space6 ;
      #endif

      #ifdef S4CLIPPER
         FILE4 file ;
         S4LONG fileLocked ;
         MEM4 S4PTR *blockMemory ;
         #ifndef S4OFF_OPTIMIZE
            struct TAG4FILESt *readBlockTag ;    /* used for optimization -- is a block-read request occurring? */
         #endif
      #else
         FILE4        space7 ;
         S4LONG       space8 ;
         void S4PTR  *space9 ;
      #endif

      T4HEADER        header ;
      S4LONG          headerOffset ;     /* Offset in file to the tag's header info. */
      int rootWrite ;        /* True if 'header.root' needs to be written */
      int keyDec ;
      #ifdef S4FOX
         MEM4 *builtKeyMemory ;
      #endif
      #ifdef S4MDX
         S4LONG       changed ;
      #endif
      #ifdef __unix__
         int keyType ;
      #endif
} TAG4FILE ;

typedef struct TAG4St
{
   LINK4   link ;
   struct  INDEX4St S4PTR *index ;
   TAG4FILE *tagFile ;
   /* this uniqueError MUST be 0 if the tag is not unique */
   short int errUnique ; /* Is rewriting a unique key an error ? */
   #ifndef S4OFF_TRAN
      int isValid ;
   #endif
      #ifndef S4OFF_TRAN
         LIST4 removedKeys ;     /* list of temporarily removed keys for r4unique and e4unique cases */
      #endif
      int added, removed ;             /* was an entry added, removed */
} TAG4 ;

typedef struct
{
   char S4PTR  *name ;
   S4CONST char S4PTR *expression ;
   S4CONST char S4PTR *filter ;
   short int unique ;
   unsigned short int descending ;
} TAG4INFO ;

#ifndef S4CLIPPER
typedef struct INDEX4FILESt
{
   LINK4  link ;

   LIST4  tags ;
   int userCount ;
   CODE4 S4PTR *codeBase ;
   DATA4FILE S4PTR *dataFile ;

      FILE4  file ;
      MEM4 S4PTR *blockMemory ;
      long fileLocked ;

      #ifdef S4FOX
         TAG4FILE S4PTR *tagIndex ;    /* the tags are a tag in the index file! */
         long  eof ;
         unsigned long versionOld ;
         unsigned long versionReadUnlocked ;   /* used to know latest unlocked version read (not guaranteed for writing) */
      #else
         TAG4FILE S4PTR *space2 ;
         long space3 ;
         unsigned long space4 ;
      #endif

      #ifdef S4MDX
         I4HEADER header ;
         long  changed ;
      #endif

      #ifndef S4OFF_OPTIMIZE
         TAG4FILE *readBlockTag ;    /* used for optimization -- is a block-read request occurring? */
      #endif
   int isValid ;
} INDEX4FILE ;
#endif

typedef struct INDEX4St
{
   LINK4  link ;
   LIST4  tags ;

   DATA4 S4PTR *data ;
   CODE4 S4PTR *codeBase ;

   #ifndef S4OFF_TRAN
      int isValid ;
   #endif
   #ifndef S4CLIPPER
      INDEX4FILE *indexFile ;
   #else
      long space4;
   #endif
      char accessName[LEN4PATH+1] ;
      #ifdef S4CLIPPER
         FILE4 file ;
/*         char alias[LEN4TAG_ALIAS+1] ;*/
         char S4PTR *path ;
         long  versionOld ;
      #else
         FILE4 space1 ;
         char S4PTR *space2 ;
         long space3 ;
      #endif
} INDEX4 ;

#ifndef S4OFF_TRAN
typedef struct
{
   LINK4 link ;
   long recno ;
   unsigned char key[1] ;   /* of variable length, but marked as '1' for a place holder */
} TAG4KEY_REMOVED ;
#endif

/* Memo File Structures */
typedef struct
{
   #ifdef S4MFOX
      S4LONG  nextBlock ;  /* Memo Entry 1,2, ... */
      char  usused[2] ;
      short blockSize ;  /* Bytes */
   #else
      #ifdef S4MNDX
         S4LONG  nextBlock ;  /* Memo Entry 1,2, ... */
      #else
         S4LONG  nextBlock ;  /* Memo Entry 1,2, ... */
         S4LONG  zero ;
         char  fileName[8] ;
         short zero2 ;
         short x102 ;
         short blockSize ;  /* Bytes */
         short zero3 ;
      #endif
   #endif
} MEMO4HEADER ;

#ifndef S4MNDX
#ifndef S4MFOX
typedef struct
{
   S4LONG  next ;     /* The next free block area */
   S4LONG  num ;      /* The number of free blocks in the free block area */
   int toDisk ;    /* TRUE if this information needs to be written to disk */
   S4LONG  blockNo ; /* The current block number */
} MEMO4CHAIN_ENTRY ;
#endif /*  ifndef S4MFOX  */

typedef struct
{
   #ifdef S4MFOX
      S4LONG type ;         /* 0 for picture, 1 for text, 2 for OLE -- picture not supported */
      S4LONG numChars ;    /* Including the 'MemoBlock' */
   #else
      short minusOne ;    /* '-1' for dBASE IV */
      short startPos ;
      S4LONG  numChars ;    /* Including the 'MemoBlock' */
   #endif
} MEMO4BLOCK ;
#endif  /*  ifndef S4MNDX  */

typedef struct
{
   unsigned  char  sigDig ;  /* The number of significant digits; 52 is zero */
   unsigned  char  digitInfo ; /* contains one, len and sign */
   unsigned  char  bcd[10] ;
}  C4BCD ;

#ifndef S4OFF_MULTI
/* for a single lock */
typedef struct
{
   long recNum ;    /* if appropriate */
   long clientId ;
   long serverId ;
   short int type ;
   char blank[2] ;
} LOCK4ID ;

typedef struct LOCK4St
{
   SINGLE4 link ;
      DATA4FILE *data ;

   LOCK4ID id ;
} LOCK4 ;

#endif

#ifdef S4FILE_EXTENDED
   #ifdef S4FOX
      typedef struct
      {
         char SearchString[12];
         int doLargeLocking;
         int previousCode4init;
      }C4STAMP;
   #endif
#endif

#ifdef S4USE_LOW_MEMORY
typedef union
{
   unsigned long block ;
   struct
   {
      unsigned short protect ;
      unsigned short real ;
   } ptr ;
} FIXED4MEM ;
#else
typedef void * FIXED4MEM ;
#endif

#ifdef S4TESTING
typedef struct
{
   unsigned char T4SUITE[255] ;
   unsigned char T4RECNO[100] ;
} ATS4RECINFO ;
#endif

