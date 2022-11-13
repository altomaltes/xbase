/* d4declar.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifdef __cplusplus
   extern "C" {
#endif

/* Possibly Unsupported functions */

#ifdef S4NO_ECVT
   char *f4ecvt( double, int, int *, int * ) ;
   char *ecvt( double, int, int*, int* ) ;
#endif
#ifdef S4NO_FCVT
   char *fcvt( double, int, int*, int* ) ;
   char *f4fcvt( double, int, int *, int * ) ;
#endif
#ifdef S4NO_STRNICMP
   int strnicmp(char *, char *, size_t ) ;
#endif
#ifdef S4NO_ATOF
   double c4atof(const char *) ;
#endif
#ifdef S4NO_TOUPPER
   char toupper(char) ;
#endif
/* EXTERNAL FUNCTIONS : */

/* CONVERSIONS */
PUBLIC double S4FUNCTION c4atod( const char S4PTR *, const int ) ;
PUBLIC void S4FUNCTION c4atod2( char S4PTR *, int, double S4PTR *);
PUBLIC int S4FUNCTION c4atoi( const char S4PTR *, const int ) ;
PUBLIC long S4FUNCTION c4atol( const char S4PTR *, const int ) ;
#ifdef S4WINCE
   PUBLIC void S4FUNCTION c4atou(const char *, unsigned short *,  int ) ;
   PUBLIC void S4FUNCTION c4utoa( unsigned short *from) ;
#endif
PUBLIC void S4FUNCTION c4encode( char S4PTR *, const char S4PTR *, char S4PTR *, const char S4PTR * ) ;
PUBLIC void S4FUNCTION c4trimN( char S4PTR *, int ) ;

/* CODE4 */
PUBLIC CODE4 S4PTR *S4FUNCTION code4allocLow( int, const char S4PTR *, long ) ;
PUBLIC DATA4 S4PTR *S4FUNCTION code4data( CODE4 S4PTR *, const char S4PTR * ) ;
PUBLIC const char S4PTR * S4FUNCTION code4indexExtension( CODE4 S4PTR * ) ;
PUBLIC int S4FUNCTION code4initLow( CODE4 S4PTR *, const char S4PTR *, long, long ) ;
PUBLIC int S4FUNCTION code4initUndo( CODE4 S4PTR * ) ;
PUBLIC int S4FUNCTION code4lock( CODE4 S4PTR * ) ;
PUBLIC void S4FUNCTION code4largeOn( CODE4 S4PTR * ) ;
PUBLIC void S4FUNCTION tran4freeLocks( CODE4 S4PTR *, SINGLE4DISTANT S4PTR *t ) ;
PUBLIC void S4FUNCTION code4lockClear( CODE4 S4PTR * ) ;
PUBLIC const char S4PTR *S4FUNCTION code4lockFileName( CODE4 S4PTR * ) ;
PUBLIC long S4FUNCTION code4lockItem( CODE4 S4PTR * ) ;
PUBLIC const char S4PTR *S4FUNCTION code4lockNetworkId( CODE4 S4PTR * ) ;
PUBLIC const char S4PTR *S4FUNCTION code4lockUserId( CODE4 S4PTR * ) ;
PUBLIC int S4FUNCTION code4optStart( CODE4 S4PTR * ) ;
PUBLIC int S4FUNCTION code4optSuspend( CODE4 S4PTR * ) ;
PUBLIC int S4FUNCTION code4unlock( CODE4 S4PTR * ) ;

   #define code4alloc( c4 ) ( code4allocLow( (c4), 0, S4VERSION ) )
   #define code4init( c4 ) ( code4initLow( (c4), 0, S4VERSION, sizeof( CODE4 ) ) )

#ifdef S4UTIL
   PUBLIC int S4FUNCTION code4passwordSet( CODE4 S4PTR *, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;  /* for utility only */
#endif
   PUBLIC int S4FUNCTION code4close( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4connect( CODE4 S4PTR *, const char S4PTR *, const char S4PTR *, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   PUBLIC const char S4PTR *S4FUNCTION code4dateFormat( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4dateFormatSet( CODE4 S4PTR *, const char S4PTR * ) ;
   PUBLIC void S4FUNCTION code4exit( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4flush( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4info( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4infoRetrieve( CODE4 S4PTR *, S4LONG S4PTR *, unsigned short S4PTR *, S4LONG S4PTR *, int S4PTR * ) ;
   PUBLIC char *S4FUNCTION code4serverConfigName( CODE4 S4PTR * ) ;
   PUBLIC const char S4PTR * S4FUNCTION code4serverName( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4optAll( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4logCreate( CODE4 S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   PUBLIC const char S4PTR *S4FUNCTION code4logFileName( CODE4 S4PTR * ) ;
   PUBLIC int S4FUNCTION code4logOpen( CODE4 S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   PUBLIC void S4FUNCTION code4logOpenOff( CODE4 S4PTR * ) ;
   #ifdef S4INLINE
      #ifdef S4OFF_MULTI
         #define code4unlockAuto( c4 ) ( 0 )
         #define code4unlockAutoSet( c4, val ) ( 0 )
      #else
         #define code4unlockAuto( c4 ) ( (c4)->c4trans.trans.unlockAuto )
         #define code4unlockAutoSet( c4, val ) ( (c4)->c4trans.trans.unlockAuto = (val) )
      #endif
   #else
      PUBLIC void S4FUNCTION code4unlockAutoSet( CODE4 *, int ) ;
      int S4FUNCTION code4unlockAuto( CODE4 * ) ;
   #endif

PUBLIC int S4FUNCTION currency4compare( const CURRENCY4 *, const CURRENCY4 * ) ;
PUBLIC int S4FUNCTION date4timeCompare( const long *, const long * ) ;
/* for testing */
PUBLIC int S4FUNCTION currency4add( CURRENCY4 S4PTR *, const CURRENCY4 S4PTR *, const CURRENCY4 S4PTR * ) ;
PUBLIC int S4FUNCTION currency4subtract( CURRENCY4 S4PTR *, const CURRENCY4 S4PTR *, const CURRENCY4 S4PTR * ) ;
PUBLIC int S4FUNCTION currency4multiplyShort( CURRENCY4 S4PTR *, const CURRENCY4 S4PTR *, const short  ) ;
PUBLIC int S4FUNCTION currency4divideShort( CURRENCY4 S4PTR *, const CURRENCY4 S4PTR *, const short ) ;
PUBLIC int S4FUNCTION c4currencyToA( char S4PTR *, int, const CURRENCY4 S4PTR *, int ) ;
PUBLIC int S4FUNCTION c4atoCurrency( CURRENCY4 S4PTR *, const char S4PTR *, int ) ;
PUBLIC void S4FUNCTION f4assignCurrency( FIELD4 S4PTR *, char S4PTR * ) ;
PUBLIC char S4PTR *S4FUNCTION f4currency( const FIELD4 S4PTR *, int ) ;
PUBLIC char S4PTR *S4FUNCTION f4dateTime( const FIELD4 S4PTR * ) ;
PUBLIC void S4FUNCTION f4assignDateTime( FIELD4 S4PTR *, char S4PTR * ) ;

/* DATA4 */
PUBLIC S4CONST char S4PTR * S4FUNCTION d4alias( DATA4 S4PTR * ) ;
PUBLIC void S4FUNCTION d4aliasSet( DATA4 S4PTR *, const char S4PTR * ) ;
PUBLIC int S4FUNCTION d4append( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4appendBlank( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4appendStart( DATA4 S4PTR *, int ) ;
PUBLIC void S4FUNCTION d4blank( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4bof( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4bottom( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4check( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4close( DATA4 S4PTR * ) ;
PUBLIC DATA4 S4PTR * S4FUNCTION d4create( CODE4 S4PTR *, const char S4PTR *, const FIELD4INFO S4PTR *, const TAG4INFO S4PTR * ) ;
PUBLIC DATA4 S4PTR * S4FUNCTION d4createLow( CODE4 S4PTR *, const char S4PTR *, const FIELD4INFO S4PTR *, const TAG4INFO S4PTR * ) ;
PUBLIC DATA4 S4PTR * S4FUNCTION d4createTemp( CODE4 S4PTR *, const FIELD4INFO S4PTR *, const TAG4INFO S4PTR * ) ;
PUBLIC void S4FUNCTION d4delete( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4eof( DATA4 S4PTR * ) ;
PUBLIC FIELD4 S4PTR * S4FUNCTION d4field( DATA4 S4PTR *, const char S4PTR * ) ;
PUBLIC FIELD4 S4PTR * S4FUNCTION d4fieldJ( DATA4 S4PTR *, const int ) ;
PUBLIC int S4FUNCTION d4fieldNumber( DATA4 S4PTR *, const char S4PTR * ) ;
PUBLIC DATA4 S4PTR * S4FUNCTION d4fieldsAdd( DATA4 S4PTR *, int, FIELD4INFO S4PTR * ) ;
PUBLIC DATA4 S4PTR * S4FUNCTION d4fieldsRemove( DATA4 S4PTR * S4PTR *, int, char S4PTR * S4PTR * ) ;
PUBLIC const char S4PTR *S4FUNCTION d4fileName( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4flush( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4freeBlocks( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4go( DATA4 S4PTR *, const long ) ;
PUBLIC int S4FUNCTION d4goBof( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4goEof( DATA4 S4PTR * ) ;
PUBLIC INDEX4 S4PTR * S4FUNCTION d4index( DATA4 S4PTR *, const char S4PTR * ) ;
#ifndef S4COMP_OFF_MULTI
   PUBLIC int S4FUNCTION d4lock( DATA4 S4PTR *, const long ) ;
   PUBLIC int S4FUNCTION d4lockAdd( DATA4 S4PTR *, long ) ;
   PUBLIC int S4FUNCTION d4lockAddAll( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockAddAppend( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockAddFile( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockAll( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockAppendRecord( DATA4 *data ) ;
   PUBLIC int S4FUNCTION d4lockAppend( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockFile( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4lockTest( DATA4 S4PTR *, const long ) ; /* SQL */
   PUBLIC int S4FUNCTION d4lockTestAppendLow( DATA4 S4PTR * ) ;  /* testing only */
#endif
   PUBLIC int S4FUNCTION d4log( DATA4 S4PTR *, const int ) ;
#ifndef S4COMP_OFF_MEMO
   PUBLIC int S4FUNCTION d4memoCompress( DATA4 S4PTR * ) ;
#endif
PUBLIC short int S4FUNCTION d4numFields( DATA4 S4PTR * ) ;
PUBLIC DATA4 S4PTR *S4FUNCTION d4open( CODE4 S4PTR *, const char S4PTR * ) ;
PUBLIC DATA4 S4PTR *S4FUNCTION d4openClone( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4optimizeWrite( DATA4 S4PTR *, const int ) ;
PUBLIC int S4FUNCTION d4pack( DATA4 S4PTR * ) ;
PUBLIC double S4FUNCTION d4position( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4position2( DATA4 S4PTR *, double S4PTR * ) ;
PUBLIC long S4FUNCTION d4recCountDo( DATA4 S4PTR * ) ;
PUBLIC long S4FUNCTION d4recNo( DATA4 S4PTR * ) ;
PUBLIC char S4PTR *S4FUNCTION d4record( DATA4 S4PTR * ) ;
PUBLIC char S4PTR *S4FUNCTION d4recordOld( DATA4 S4PTR * ) ;
PUBLIC unsigned int S4FUNCTION d4recWidth( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4refresh( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4skip( DATA4 S4PTR *, const long ) ;
PUBLIC int S4FUNCTION d4top( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4unappend( DATA4 * ) ;  /* internal only */
PUBLIC int S4FUNCTION d4writeLow( DATA4 S4PTR *, const long, const int ) ;
PUBLIC int S4FUNCTION d4zap( DATA4 S4PTR *, const long, const long ) ;
#ifdef S4OFF_MULTI
   #define d4recCount( d4 ) ( dfile4recCount( ( d4->dataFile ), 0 ) )
#else
   #define d4recCount( d4 ) ( d4recCountDo( d4 ) )
#endif
#ifndef S4COMP_OFF_INDEX
   PUBLIC int S4FUNCTION d4reindex( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4seek( DATA4 S4PTR *, const char S4PTR * ) ;
   PUBLIC int S4FUNCTION d4seekDouble( DATA4 S4PTR *, const double ) ;
   PUBLIC int S4FUNCTION d4seekN( DATA4 S4PTR *, const char S4PTR *, const short ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION d4tag( DATA4 S4PTR *, const char S4PTR * const ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION d4tagDefault( DATA4 S4PTR * ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION d4tagNext( DATA4 S4PTR *, TAG4 S4PTR * ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION d4tagPrev( DATA4 S4PTR *, TAG4 S4PTR * ) ;
   PUBLIC void S4FUNCTION d4tagSelect( DATA4 S4PTR *, TAG4 S4PTR * ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION d4tagSelected( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4tagSync( DATA4 S4PTR *, TAG4 S4PTR * const ) ;
   PUBLIC int S4FUNCTION d4numTags(DATA4 S4PTR *);
#endif /* S4COMP_OFF_INDEX */
#ifndef S4COMP_OFF_MULTI
   #ifndef S4SINGLE
      PUBLIC int S4FUNCTION d4unlockLow( DATA4 *, long, char ) ;
   #endif
      PUBLIC int S4FUNCTION d4unlock( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4unlockRecord( DATA4 *, long ) ;  /* exported for SQL */
#endif
   PUBLIC int S4FUNCTION d4seekNext(       DATA4 S4PTR *, const char S4PTR * ) ;
   PUBLIC int S4FUNCTION d4seekNextDouble( DATA4 S4PTR *, const double ) ;
   PUBLIC int S4FUNCTION d4seekNextN(      DATA4 S4PTR *, const char S4PTR *, const short ) ;
#ifndef S4COMP_OFF_MULTI
   PUBLIC int S4FUNCTION dfile4lockTestFile( DATA4FILE S4PTR *, const long, const long ) ;
#endif
   #ifndef S4COMP_OFF_MULTI
      #define d4lockTestFile( d4 ) ( dfile4lockTestFile( (d4)->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) )
      #ifndef S4OFF_MULTI
         #define d4lockIndex( d4 ) ( dfile4lockIndex( (d4)->dataFile, data4serverId( d4 ) ) )
      #endif
   #endif
   PUBLIC int S4FUNCTION dfile4remove( DATA4FILE S4PTR * ) ;
#ifndef S4COMP_OFF_MULTI
      #define d4lockTestAppend( d4 ) ( dfile4lockTestAppend( (d4)->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) )
#endif
PUBLIC int S4FUNCTION d4deleted( DATA4 S4PTR * ) ;
PUBLIC FIELD4INFO S4PTR * S4FUNCTION d4fieldInfo( DATA4 S4PTR * ) ;
PUBLIC int S4FUNCTION d4remove( DATA4 S4PTR * ) ;
#define d4write( a, b ) ( d4writeLow( a, b, 0 ) )
   PUBLIC int S4FUNCTION d4changed( DATA4 S4PTR *, int ) ;
   PUBLIC int S4FUNCTION d4optimize( DATA4 S4PTR *, const int ) ;
   PUBLIC int S4FUNCTION d4positionSet( DATA4 S4PTR *, const double ) ;
   PUBLIC void S4FUNCTION d4recall( DATA4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4refreshRecord( DATA4 S4PTR * ) ;
#ifndef S4OFF_MULTI
   PUBLIC int S4FUNCTION dfile4lockTestAppend( DATA4FILE S4PTR *, const long, const long ) ;
   PUBLIC int S4FUNCTION dfile4unlockRecord( DATA4FILE *, const long, const long, const long ) ;
#endif /* S4OFF_MULTI */
#ifdef S4OFF_TRAN
   #define d4logStatus( d4 ) ( 0 )
#else
      #define d4logStatus( d4 ) ( (d4)->logVal == 0 ? 0 : 1 )
#endif

/* DATE4 */
PUBLIC          int  S4FUNCTION date4assign( char S4PTR *, const long ) ;
PUBLIC S4CONST char *S4FUNCTION date4cdow( const char S4PTR * ) ;
PUBLIC S4CONST char *S4FUNCTION date4cmonth( const char S4PTR * ) ;
PUBLIC          int  S4FUNCTION date4dow( const char S4PTR * ) ;
PUBLIC         void  S4FUNCTION date4format( const char S4PTR *, char S4PTR *, char S4PTR * ) ;/* 'dt' may be 'result'*/
PUBLIC       double  S4FUNCTION date4formatMdx( const char S4PTR * ) ;
PUBLIC          int  S4FUNCTION date4formatMdx2( const char S4PTR * , double S4PTR * ) ;
PUBLIC         void  S4FUNCTION date4init(       char S4PTR *, const char S4PTR *, char S4PTR * ) ;
PUBLIC          int  S4FUNCTION date4isLeap( const char S4PTR * ) ;
PUBLIC         long  S4FUNCTION date4long(   const char S4PTR * ) ;
PUBLIC         void  S4FUNCTION date4timeNow( char S4PTR * ) ;
PUBLIC         void  S4FUNCTION date4today(    char S4PTR * ) ;
#define date4day( datePtr )    ( (int)c4atol( (datePtr) + 6, 2 ) )
#define date4month( datePtr )  ( (int)c4atol( (datePtr) + 4, 2 ) )
#define date4year( yearPtr )   ( (int)c4atol( (yearPtr), 4 ) )

/* ERROR4 */
PUBLIC int S4FUNCTION error4default( CODE4 S4PTR *, const int, const long ) ;
PUBLIC int S4FUNCTION error4describeDefault( CODE4 S4PTR *, const int, const long, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
PUBLIC int S4FUNCTION error4describeExecute( CODE4 S4PTR *, const int, const long, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
PUBLIC int S4FUNCTION error4file( CODE4 S4PTR *, S4CONST char S4PTR *, const int ) ;
PUBLIC void S4FUNCTION error4hook( CODE4 S4PTR *, int, long, const char S4PTR *, const char S4PTR *, const char S4PTR * ) ;
PUBLIC int S4FUNCTION error4set( CODE4 S4PTR *, const int ) ;
PUBLIC int S4FUNCTION error4set2( CODE4 S4PTR *, const long ) ;
PUBLIC const char S4PTR * S4FUNCTION error4text( CODE4 S4PTR *, const long ) ;
   PUBLIC void S4FUNCTION error4exitTest( CODE4 S4PTR * ) ;
#ifdef E4STACK
   PUBLIC int S4FUNCTION error4stackDefault( CODE4 S4PTR *, const int, const long ) ;
#endif
#ifdef S4COM_PRINT
   const char *s4connectionPrint(const int type ) ;
#endif

/* FIELD4 */
PUBLIC void S4FUNCTION f4assign( FIELD4 S4PTR *, const char S4PTR * ) ;
PUBLIC void S4FUNCTION f4assignChar( FIELD4 S4PTR *, const int ) ;
PUBLIC void S4FUNCTION f4assignDouble( FIELD4 S4PTR *, const double ) ;
PUBLIC void S4FUNCTION f4assignField( FIELD4 S4PTR *, const FIELD4 S4PTR * ) ;
PUBLIC void S4FUNCTION f4assignInt( FIELD4 S4PTR *, const int ) ;
PUBLIC void S4FUNCTION f4assignLong( FIELD4 S4PTR *, const long ) ;
PUBLIC void S4FUNCTION f4assignN( FIELD4 S4PTR *, const char S4PTR *, const unsigned int ) ;
PUBLIC void S4FUNCTION f4assignNull( FIELD4 S4PTR * ) ;
PUBLIC char S4PTR * S4FUNCTION f4assignPtr( FIELD4 S4PTR * ) ;
PUBLIC void S4FUNCTION f4blank( FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4char( const FIELD4 S4PTR * ) ;
PUBLIC DATA4 S4PTR *S4FUNCTION f4data( const FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4decimals( const FIELD4 S4PTR * ) ;
PUBLIC double S4FUNCTION f4double( const FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4double2( const FIELD4 S4PTR *, double S4PTR * ) ;
PUBLIC int S4FUNCTION f4int( const FIELD4 S4PTR * ) ;
PUBLIC unsigned int S4FUNCTION f4len( const FIELD4 S4PTR * ) ;
PUBLIC long S4FUNCTION f4long( const FIELD4 S4PTR * ) ;
#ifndef S4COMP_OFF_MEMO
   PUBLIC int S4FUNCTION f4memoAssign( FIELD4 S4PTR *, const char S4PTR * ) ;
   PUBLIC int S4FUNCTION f4memoAssignN( FIELD4 S4PTR *, const char S4PTR *, const unsigned int ) ;
   PUBLIC int S4FUNCTION f4memoFree( FIELD4 S4PTR * ) ;
   PUBLIC unsigned int S4FUNCTION f4memoLen( FIELD4 S4PTR * ) ;
   PUBLIC unsigned int S4FUNCTION f4memoNcpy( FIELD4 S4PTR *, char S4PTR *, const unsigned int ) ;
   PUBLIC char S4PTR * S4FUNCTION f4memoPtr( FIELD4 S4PTR * ) ;
   PUBLIC S4CONST char S4PTR * S4FUNCTION f4memoStr( FIELD4 S4PTR * ) ;
   void f4memoAssignField( FIELD4 *, FIELD4 * ) ;
#endif
PUBLIC S4CONST char S4PTR * S4FUNCTION f4name( S4CONST FIELD4 S4PTR * ) ;
PUBLIC unsigned int S4FUNCTION f4ncpy( FIELD4 S4PTR *, char S4PTR *, const unsigned int ) ;
PUBLIC int S4FUNCTION f4null( const FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4number( const FIELD4 S4PTR * ) ;
PUBLIC char S4PTR * S4FUNCTION f4ptr( const FIELD4 S4PTR * ) ;
PUBLIC char S4PTR * S4FUNCTION f4str( FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4true( const FIELD4 S4PTR * ) ;
PUBLIC int S4FUNCTION f4type( const FIELD4 S4PTR * ) ;

#ifdef S4CFOX  /*Internal function */
   void S4FUNCTION f4assignNotNull( FIELD4 *field );
#endif

/* FILE4 */
PUBLIC int    S4FUNCTION file4close( FILE4 S4PTR * ) ;
PUBLIC int    S4FUNCTION file4create( FILE4 S4PTR *, CODE4 S4PTR *, S4CONST char S4PTR *, const int ) ;
PUBLIC int    S4FUNCTION file4flush( FILE4 S4PTR * ) ;
PUBLIC FILE4LONG S4FUNCTION file4lenLow( FILE4 S4PTR * ) ;

#ifdef S4FILE_EXTENDED
   int file4lenSetLow( FILE4 S4PTR *, FILE4LONG ) ;
#else
   PUBLIC int S4FUNCTION file4lenSetLow( FILE4 S4PTR *, FILE4LONG ) ;
#endif
#ifndef S4INTERNAL_COMPILE_CHECK
   #ifdef S4FILE_EXTENDED
      PUBLIC int    S4FUNCTION file4lenSet( FILE4 *, long ) ;
   #else
      #define file4lenSet( f4, len ) file4lenSetLow( f4, len )
   #endif
#endif
#ifndef S4INTERNAL_COMPILE_CHECK
   #define file4lock( f4, a, b ) file4lockInternal( (f4), (unsigned long)a, 0, (unsigned long)b, 0 )
#endif
PUBLIC int    S4FUNCTION file4lockInternal( FILE4 S4PTR *, unsigned long, long, unsigned long, long ) ;
#define file4name( f4 ) ( (f4)->name )
PUBLIC int    S4FUNCTION file4open( FILE4 S4PTR *, CODE4 S4PTR *, S4CONST char S4PTR *, const int ) ;
PUBLIC int    S4FUNCTION file4openTest( FILE4 S4PTR * ) ;
PUBLIC int    S4FUNCTION file4optimizeLow( FILE4 S4PTR *, const int, const int, const long, const void S4PTR * ) ;
#define file4optimize( f4, i1, i2 ) ( file4optimizeLow( (f4), (i1), (i2), 0, 0 ) )
PUBLIC int    S4FUNCTION file4optimizeWrite( FILE4 S4PTR *, const int ) ;
int file4readAllInternal( FILE4 *, FILE4LONG, void *, unsigned ) ;
unsigned file4readInternal( FILE4 *, FILE4LONG, void *, unsigned ) ;
#ifndef S4INTERNAL_COMPILE_CHECK
   PUBLIC unsigned int S4FUNCTION file4read( FILE4 S4PTR *, const long, void S4PTR *, const unsigned int ) ;
   PUBLIC int    S4FUNCTION file4readAll( FILE4 S4PTR *, const long, void S4PTR *, const unsigned int ) ;
#endif
PUBLIC int    S4FUNCTION file4readError( FILE4 S4PTR *, FILE4LONG, unsigned, const char S4PTR * ) ;
PUBLIC int    S4FUNCTION file4refresh( FILE4 S4PTR * ) ;
PUBLIC int    S4FUNCTION file4replace( FILE4 S4PTR *, FILE4 S4PTR * ) ;
#ifndef S4INTERNAL_COMPILE_CHECK
   #define file4unlock( f4, a, b ) file4unlockInternal( (f4), (unsigned long)(a), 0, (unsigned long)(b), 0 )
#endif
PUBLIC int    S4FUNCTION file4unlockInternal( FILE4 S4PTR *, unsigned long, long, unsigned long, long ) ;
int file4writeInternal( FILE4 *, FILE4LONG, const void *, unsigned ) ;
#ifndef S4INTERNAL_COMPILE_CHECK
   PUBLIC int    S4FUNCTION file4write( FILE4 S4PTR *, const long, const void S4PTR *, const unsigned int ) ;
#endif
   PUBLIC int    S4FUNCTION file4temp( FILE4 S4PTR *, CODE4 S4PTR *, char *, const int ) ;
   #ifdef E4LOCK_HOOK
      PUBLIC int S4FUNCTION file4lockHook( CODE4 S4PTR *, const char S4PTR *, long, long, int ) ;
   #endif

/* FILE4SEQ_READ */
PUBLIC int S4FUNCTION file4seqReadInitDo( FILE4SEQ_READ *, FILE4 *, FILE4LONG, void *, const unsigned, const int ) ;
#ifndef S4INTERNAL_COMPILE_CHECK
   PUBLIC int S4FUNCTION file4seqReadInit( FILE4SEQ_READ S4PTR *, FILE4 S4PTR *, long, void S4PTR *, const unsigned ) ;
#endif
#ifdef S4ADVANCE_READ
   void file4seqReadInitUndo( const FILE4SEQ_READ * ) ;
#endif
PUBLIC unsigned int S4FUNCTION file4seqRead( FILE4SEQ_READ S4PTR *, void S4PTR *, unsigned ) ;
PUBLIC int    S4FUNCTION file4seqReadAll( FILE4SEQ_READ S4PTR *, void S4PTR *, const unsigned int ) ;

/* FILE4SEQ_WRITE */
int file4seqWriteInitLow( FILE4SEQ_WRITE *seqWrite, FILE4 *file, FILE4LONG startOffset, void *ptr, const unsigned ptrLen ) ;
#ifndef S4INTERNAL_COMPILE_CHECK
   PUBLIC int    S4FUNCTION file4seqWriteInit( FILE4SEQ_WRITE S4PTR *, FILE4 S4PTR *, const long, void S4PTR *, const unsigned int ) ;
#endif
PUBLIC int    S4FUNCTION file4seqWrite( FILE4SEQ_WRITE S4PTR *, const void S4PTR *, const unsigned int ) ;
PUBLIC int    S4FUNCTION file4seqWriteFlush( FILE4SEQ_WRITE S4PTR * ) ;
PUBLIC int    S4FUNCTION file4seqWriteRepeat( FILE4SEQ_WRITE S4PTR *, const long, const char ) ;

/* INDEX4 */
#ifndef S4COMP_OFF_INDEX
   PUBLIC int S4FUNCTION i4close( INDEX4 S4PTR * ) ;
   PUBLIC int S4FUNCTION i4closeLow( INDEX4 S4PTR * ) ;
   PUBLIC INDEX4 S4PTR *S4FUNCTION i4create( DATA4 S4PTR *, const char S4PTR *, const TAG4INFO S4PTR * ) ; /* 0 name -> productn */
   PUBLIC const char S4PTR *S4FUNCTION i4fileName( INDEX4 S4PTR * ) ;
   PUBLIC const char S4PTR *S4FUNCTION t4fileName( TAG4 S4PTR * ) ;
   PUBLIC INDEX4 S4PTR *S4FUNCTION i4open( DATA4 S4PTR *, const char S4PTR * ) ;
   PUBLIC int S4FUNCTION i4reindex( INDEX4 S4PTR * ) ;
   PUBLIC TAG4 S4PTR *S4FUNCTION i4tag( INDEX4 S4PTR *, const char S4PTR * ) ;
   PUBLIC int S4FUNCTION i4tagAdd( INDEX4 S4PTR *, const TAG4INFO S4PTR * ) ;
   PUBLIC TAG4INFO *S4FUNCTION i4tagInfo( INDEX4 * ) ;
   PUBLIC int S4FUNCTION i4tagRemove( TAG4 S4PTR * ) ;
   PUBLIC int S4FUNCTION d4indexesRemove( DATA4 S4PTR * ) ;
   int i4indexRemove( INDEX4 * ) ;
#endif /* S4COMP_OFF_INDEX */

PUBLIC int S4FUNCTION expr4getReturnType( EXPR4 S4PTR *, int ) ;

#ifdef S4JAVA
   PUBLIC int S4FUNCTION java4processMessage( CODE4 *, struct SERVER4CLIENTSt *, short ) ;
#endif /* S4JAVA */

/* LIST4 */
PUBLIC void S4FUNCTION l4addAfter(   LIST4 S4PTR *, void S4PTR *, void S4PTR * ) ;
PUBLIC void S4FUNCTION l4addBefore( LIST4 S4PTR *, void S4PTR *, void S4PTR * ) ;
#define l4init( l4 ) ( (void)( (l4)->selected = 0, (l4)->nLink = 0, (l4)->lastNode = 0 ) )
#define l4numNodes( l4 ) ( (l4)->nLink )
PUBLIC void S4PTR *S4FUNCTION l4prev( const LIST4 S4PTR *, const void S4PTR * ) ;
PUBLIC void S4PTR *S4FUNCTION l4pop( LIST4 S4PTR * ) ;
PUBLIC void S4FUNCTION l4remove( LIST4 S4PTR *, void S4PTR * ) ;
PUBLIC void S4PTR* S4FUNCTION l4firstLow( const LIST4 S4PTR * ) ;  /* Returns 0 if none */
PUBLIC void S4PTR* S4FUNCTION l4lastLow( const LIST4 S4PTR * ) ;   /* Returns 0 if none */
PUBLIC void S4PTR* S4FUNCTION l4nextLow( const LIST4 S4PTR *, const void S4PTR * ) ;  /* Returns 0 if none */
PUBLIC void S4FUNCTION l4addLow( LIST4 S4PTR *, void S4PTR * ) ;
#ifdef E4LINK
   #define l4first( l4 )         ( l4firstLow( l4 ) )
   #define l4last( l4 )          ( l4lastLow( l4 ) )
   #define l4next( list, link )  ( l4nextLow( (list), (link) ) )
   #define l4add( list, item )   ( l4addLow( (list), (item) ) )
#else
   #define l4first( l4 )         ((void *)( ( (l4)->lastNode == 0 ) ? 0 : ( (l4)->lastNode->n ) ))
   #define l4last( l4 )          ((void *)( (l4)->lastNode ))
   #define l4next( list, link )  ((void *)( ( (void *)(link) == (list)->lastNode ) ? 0 : ( (void *)(link) == 0 ) ? l4first( list ) : (void *)(((LINK4 *)(link))->n) ))
   #define l4add( list, item )   ( l4addAfter( (list), (list)->lastNode, (item) ) )
#endif

#define single4init( s4 ) ( (s4)->nextLink = 0 )
#define single4next( s4 ) ( (s4)->nextLink )
#define single4initIterate( s4 ) ( (s4)->nextLink )
#define single4add( s4, s4add ) ( (s4add)->nextLink = (s4)->nextLink, (s4)->nextLink = s4add )

#define single4distantToSingle( s4 ) ( (SINGLE4 *)((s4)->nextLink) )
#define single4distantToItem( s4 ) ( (SINGLE4 *)((s4)->nextLink->nextLink) )
#define single4distantNext( s4 ) ( (s4)->nextLink = (s4)->nextLink->nextLink )
#define single4distantInitIterate( s4, p ) ( (s4)->nextLink = (SINGLE4DISTANT *)(p) )
#define single4distantPop( s4 ) ( (s4)->nextLink->nextLink = (s4)->nextLink->nextLink->nextLink )
#define single4distantPopWithCheck( s4 ) ( (s4)->nextLink ? ( (s4)->nextLink->nextLink ? ( single4distantRemove( s4 ) ) : 0 ) : 0 )

/* MEM4 */
PUBLIC void S4PTR *S4FUNCTION mem4allocDefault( MEM4 S4PTR * ) ;  /* 0 Parm causes 0 return */
PUBLIC void S4PTR *S4FUNCTION mem4alloc2Default( MEM4 S4PTR *, CODE4 S4PTR * ) ;  /* 0 Parm causes 0 return */
PUBLIC int S4FUNCTION mem4checkMemory( void ) ;
PUBLIC void S4PTR *S4FUNCTION mem4createAllocDefault( CODE4 S4PTR *, MEM4 S4PTR * S4PTR *, int, const unsigned int, int, const int ) ;
PUBLIC MEM4 S4PTR *S4FUNCTION mem4createDefault( CODE4 S4PTR *, int, const unsigned int, int, const int ) ;
PUBLIC int S4FUNCTION mem4freeCheck( const int ) ;
PUBLIC int S4FUNCTION mem4freeDefault( MEM4 S4PTR *, void S4PTR * ) ;
PUBLIC void S4FUNCTION mem4release( MEM4 S4PTR * ) ;

/* TAG4 */
#ifndef S4COMP_OFF_INDEX
   PUBLIC char S4PTR *S4FUNCTION t4alias( TAG4 S4PTR * ) ;
   PUBLIC int S4FUNCTION t4close( TAG4 S4PTR * ) ;
   PUBLIC int S4FUNCTION t4uniqueModify( TAG4 S4PTR *, int ) ;
   #define t4open( a, b, c ) t4openLow( (a), (b), (c), 0 )
   PUBLIC TAG4 S4PTR *S4FUNCTION t4openLow( DATA4 S4PTR *, INDEX4 S4PTR *, const char S4PTR *, const char S4PTR * ) ;
   PUBLIC short int S4FUNCTION t4unique( const TAG4 S4PTR * ) ;
   #ifdef S4CLIPPER
      PUBLIC TAG4 S4PTR *S4FUNCTION t4create( DATA4 S4PTR *, const TAG4INFO S4PTR *, INDEX4 S4PTR *, int ) ;
   #endif
      PUBLIC int S4FUNCTION t4uniqueSet( TAG4 S4PTR *, const short ) ;
      PUBLIC S4CONST char S4PTR *S4FUNCTION t4exprLow( TAG4 S4PTR * ) ;
      PUBLIC S4CONST char S4PTR *S4FUNCTION t4filterLow( TAG4 S4PTR * ) ;
         /* 'source' members are constant, so can use defines */
         #define t4expr( t4 )   ( (t4)->tagFile->expr->source )
      #define t4filter( t4 ) ( ( (t4)->tagFile->filter == 0 ? 0 : (t4)->tagFile->filter->source ) )
      PUBLIC unsigned short int S4FUNCTION tfile4isDescending( TAG4FILE * ) ;  /* for SQL */
#endif /* S4COMP_OFF_INDEX */

/* UTIL4 */
PUBLIC int S4FUNCTION u4allocAgainDefault( CODE4 S4PTR *, char S4PTR * S4PTR *, unsigned int S4PTR *, const unsigned int ) ;
PUBLIC void S4PTR *S4FUNCTION u4allocDefault( long ) ;
PUBLIC void S4PTR *S4FUNCTION u4allocErDefault( CODE4 S4PTR *, long ) ;
PUBLIC void S4PTR *S4FUNCTION u4allocFreeDefault( CODE4 S4PTR *, long ) ;
PUBLIC int S4FUNCTION u4freeDefault( void S4PTR * ) ;
PUBLIC void S4FUNCTION u4freeFixedDefault( FIXED4MEM ) ;
PUBLIC int S4FUNCTION u4nameChar( unsigned char ) ;
/* u4nameCurrent for utils */
PUBLIC int S4FUNCTION u4nameCurrent( char S4PTR *, const int, const char S4PTR * ) ;
PUBLIC int S4FUNCTION u4nameCurrentExtended( char S4PTR *, const int, const char S4PTR *, const char S4PTR * ) ;
PUBLIC int S4FUNCTION u4nameExt( char S4PTR *, int, const char S4PTR *, const int ) ;
PUBLIC int S4FUNCTION u4namePiece( char S4PTR *, const unsigned int, const char S4PTR *, const int, const int ) ;
#ifdef S4MACINTOSH
   char *u4getMacPath(CODE4 *c4, char *buf, int buflen ) ;
#endif
PUBLIC unsigned int S4FUNCTION u4ncpy( char S4PTR *, const char S4PTR *, const unsigned int ) ;
PUBLIC int S4FUNCTION u4remove( const char S4PTR * ) ;  /* used for testing */
PUBLIC long S4FUNCTION u4switch( void ) ;  /* used for example start-up verification */
#ifdef S4COMPILE_TEST
   #ifndef S4OFF_WRITE
      PUBLIC void S4FUNCTION u4yymmdd( char S4PTR * ) ;
   #endif
#else
   PUBLIC void S4FUNCTION u4yymmdd( char S4PTR * ) ;
#endif

 #define c4getDoRemove( c4 ) ( (c4)->doRemove )
 #define c4setDoRemove( c4, i ) ( (c4)->doRemove = (i) )
 #define c4getErrCreate( c4 )      ( (c4)->errCreate )
 #define c4setErrCreate( c4, val ) ( (c4)->errCreate = (val) )
 #ifdef S4DLL_BUILD
    #define i4getIndexFile( i4 )     ((i4)->indexFile)
    #define c4getReadLock( c4 )      ( (c4)->readLock )
    #define c4setReadLock( c4, val ) ( (c4)->readLock = (val) )
    #define c4getReadOnly( c4 )      ( (c4)->readOnly )
    #define c4setReadOnly( c4, val ) ( (c4)->readOnly = (val) )
 #else
    #define i4getIndexFile( i4 )     i4getIndexFileDo(i4)
    #define c4getReadLock( c4 )      c4getReadLockDo( (c4) )
    #define c4setReadLock( c4, val ) c4setReadLockDo( (c4), (val) )
    #define c4getReadOnly( c4 )      c4getReadOnlyDo( (c4) )
    #define c4setReadOnly( c4, val ) c4setReadOnlyDo( (c4), (val) )
 #endif

/* functions used to set and get CODE4 flags from outside of a DLL in cases
   where the structures are unknown (eg. index independent program) */
/* cannot be defines */

 PUBLIC int S4FUNCTION c4getAccessMode( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getAutoOpen( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrorCode( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrExpr( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrFieldName( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrGo( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrOpen( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrRelate( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrSkip( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrTagName( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getLockAttempts( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getLockEnforce( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getOptimize( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getOptimizeWrite( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getReadLockDo( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getReadOnlyDo( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getSafety( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getSingleOpen( const CODE4 S4PTR * ) ;
 PUBLIC int S4FUNCTION c4getErrDefaultUnique( const CODE4 S4PTR * ) ;
 #ifndef S4CLIPPER
 PUBLIC INDEX4FILE S4PTR *S4FUNCTION i4getIndexFileDo( const INDEX4 S4PTR * ) ;
 #endif

 PUBLIC const char *S4FUNCTION t4getExprSource( TAG4 S4PTR * ) ;

 PUBLIC void S4FUNCTION c4setAccessMode( CODE4 S4PTR *, char ) ;
 PUBLIC void S4FUNCTION c4setAutoOpen( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrorCode( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrExpr( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrFieldName( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrGo( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrOpen( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrRelate( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrSkip( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setErrTagName( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setLockAttempts( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setLockEnforce( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setOptimize( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setOptimizeWrite( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setReadLockDo( CODE4 S4PTR *, char ) ;
 PUBLIC void S4FUNCTION c4setReadOnlyDo( CODE4 S4PTR *, int ) ;
 PUBLIC void S4FUNCTION c4setSafety( CODE4 S4PTR *, char ) ;
 PUBLIC void S4FUNCTION c4setSingleOpen( CODE4 S4PTR *, short ) ;
 PUBLIC void S4FUNCTION c4setErrDefaultUnique( CODE4 S4PTR *, short ) ;

 #define c4systemPath( c4 ) ( "" )
 #define D4close( d4 ) ( d4close( d4 ) )
 #define D4recCount( d4 ) ( d4recCount( d4 ) )
 #define D4fieldsRemove( d4, n, nm ) ( d4fieldsRemove( (d4), (n), &(nm) ) )
 #define D4appendCurrent( d4 ) ( d4append( d4 ) )
 #define D4writeCurrent( d4, rec ) ( d4write( (d4), (rec) ) )
 #define D4go( d4, rec ) ( d4go( (d4), (rec) ) )
 #define D4remove( d4 ) ( d4remove( d4 ) )
 #define D4indexesRemove( d4 ) ( d4indexesRemove( d4 ) )


/* INTERNAL FUNCTIONS : */


long time4long( const char *, int ) ;

   #ifndef S4OFF_INDEX
      /* B4BLOCK */
      B4BLOCK *b4alloc( TAG4FILE *, const long ) ;
      int b4calcBlanks( const unsigned char *, const int, const unsigned char ) ;
      int b4flush( B4BLOCK * ) ;
      int b4free( B4BLOCK * ) ;
      void b4goEof( B4BLOCK * ) ;
      unsigned char *b4keyKey( B4BLOCK *, const int ) ;
      int b4lastpos( const B4BLOCK * ) ;
      int b4leaf( const B4BLOCK * ) ;
      int b4dataLeaf( void *, TAG4FILE * ) ;
      long b4recNo( const B4BLOCK *, const int ) ;
      int b4remove( B4BLOCK *);
      int b4skip( B4BLOCK *, const long ) ;
      int b4seek( B4BLOCK *, const char *, const int ) ;
      int tfile4unique( TAG4FILE *, const short int ) ;
      #define tfile4keyLen( tagFile ) ( (tagFile)->header.keyLen )

      #ifdef E4INDEX_VERIFY
         int b4verify( B4BLOCK *b4 ) ;
      #endif

      #ifdef S4FOX
         int  b4brReplace(    B4BLOCK *, const unsigned char *, const long ) ;
         int  b4calcDups( const unsigned char *, const unsigned char *, const int ) ;
         int  b4go(           B4BLOCK *, const int ) ;
         int  b4insert(       B4BLOCK *, const void *, const long, const long, const char ) ;
         int  b4insertLeaf(   B4BLOCK *, const void *, const S4LONG ) ;
         int  b4insertBranch( B4BLOCK *, const void *, const S4LONG, const S4LONG, const char ) ;
         void b4leafInit(     B4BLOCK * ) ;
         int  b4leafSeek(     B4BLOCK *, const char *, const int ) ;
         int  b4reindex(      B4BLOCK * ) ;
         int  b4removeLeaf(   B4BLOCK * ) ;
         int  b4rBrseek(      B4BLOCK *, const char *, const int, const long ) ;
         int  b4top(          B4BLOCK * ) ;
         int  tfile4branchSplit( TAG4FILE *, B4BLOCK *, B4BLOCK * ) ;
         int  tfile4init( TAG4FILE *, INDEX4 *, long, unsigned char * ) ;
         int  tfile4leafSplit( TAG4FILE *, B4BLOCK *, B4BLOCK * ) ;
         int  tfile4rSeek( TAG4FILE *, void *, int, long ) ;

         #ifndef S4FOX
            int  S4CALL t4descMemcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
         #endif
         int  x4dupCnt( const B4BLOCK *, const int ) ;
         int  x4putInfo( const B4NODE_HEADER *, void *, const S4LONG, const int, const int ) ;
         S4LONG x4recNo( const B4BLOCK *, const int ) ;
         int  x4trailCnt( const B4BLOCK *, const int ) ;
         B4KEY_DATA S4PTR *b4key( B4BLOCK S4PTR *, const int ) ;
         #define b4numKeys( b4 ) ( (b4)->header.nKeys )
      #else
         #define b4numKeys( b4 ) ( (b4)->nKeys )
         B4KEY_DATA S4PTR *b4key( const B4BLOCK S4PTR *, const int ) ;
      #endif /* S4FOX */

      #ifdef S4MDX
         int b4insert( B4BLOCK *, const void *, const long ) ;
         int tfile4init( TAG4FILE *, INDEX4 *, T4DESC * ) ;
      #endif /* S4MDX */

         PUBLIC int S4FUNCTION tfile4lock( TAG4FILE S4PTR *, const long ) ; /*Move outside the S4CLIPPER to facilitate index independant OLE-DB DLL*/
         PUBLIC int S4FUNCTION tfile4unlock( TAG4FILE S4PTR *, const long ) ;
      #ifdef S4CLIPPER
         int b4append( B4BLOCK *, const long ) ;
         int b4insert( B4BLOCK *, const void *, const long, const long ) ;
         int b4room( const B4BLOCK * ) ;
         int tfile4doVersionCheck( TAG4FILE *, int, int ) ;
         int tfile4close( TAG4FILE *, DATA4FILE * ) ;
         long tfile4extend( TAG4FILE * ) ;
         int tfile4updateHeader( TAG4FILE * ) ;
         void tfile4removeBranch( TAG4FILE *, B4BLOCK * ) ;
         int b4append2( B4BLOCK *, const void *, const long, const long ) ;
         int tfile4getReplaceEntry( TAG4FILE *, B4KEY_DATA *, B4BLOCK * ) ;
         int tfile4shrink( TAG4FILE *, long ) ;
      #endif /* S4CLIPPER */
   #endif /* S4OFF_INDEX */

PUBLIC int S4FUNCTION c4clip( char *, int ) ;
PUBLIC char *S4FUNCTION c4descend( char *, const char *, int ) ; /* exported for OLEDB */
#ifdef S4CLIPPER
   char *c4descendBinary( char *, const char *, int ) ;
#endif
void c4dtoaClipper( double, char *, int, int ) ;
PUBLIC int  S4FUNCTION c4dtoa45( double, char *, int, int ) ; /* need to export for ++ API */
PUBLIC void S4FUNCTION c4lower( char * ) ;
PUBLIC void S4FUNCTION c4ltoa45( long, char *, int ) ;
PUBLIC void S4FUNCTION c4upper( char * ) ;


void code4memStartMaxSet( CODE4 *, const int ) ;
int code4numCodeBase( void ) ;
int code4optRestart( CODE4 * ) ;
int code4unlockDo( LIST4 * ) ;
int code4verify( CODE4 *, int ) ;
PUBLIC long S4FUNCTION code4version( CODE4 S4PTR * ) ;

   #define code4idData( a, b, c ) ( tran4data( code4trans((a)), (b), (c) ) )

#ifdef S4LOCK_HOOK
   int code4lockHook( CODE4 *, const char *, const char *, const char *, long, int ) ;
#endif

/* used for internal testing only */
PUBLIC int S4FUNCTION file4seqWriteDelay( FILE4SEQ_WRITE * ) ;
PUBLIC int S4FUNCTION code4catalogSet( CODE4 S4PTR *, int, int ) ;
PUBLIC int S4FUNCTION code4indexFormat( CODE4 * ) ;
#ifndef S4OFF_TRAN
      int code4transFileEnable( CODE4TRANS *, const char * logName, const int ) ;
#endif

#define dfile4changed( a ) ( (a)->fileChanged = 1 )
int dfile4check( DATA4FILE * ) ;
int dfile4close( DATA4FILE * ) ;
int dfile4closeLow( DATA4FILE * ) ;
int dfile4create( CODE4 *, const char *, const FIELD4INFO *, const TAG4INFO *, DATA4 * * ) ;
DATA4FILE * dfile4data( CODE4 *, const char * ) ;
int dfile4flush( DATA4FILE * ) ;
int dfile4flushData( DATA4FILE * ) ;
int dfile4flushIndex( DATA4FILE * ) ;
int dfile4goData( DATA4FILE *, long, void *, int ) ;
int dfile4lockMemo( DATA4FILE * ) ;
int dfile4memoCompress( DATA4FILE *, DATA4 * ) ;
int dfile4memoUnlock( DATA4FILE * ) ;
DATA4FILE *dfile4open( CODE4 *, DATA4 *, const char *, char * * ) ;
S4CONST char *dfile4name( S4CONST DATA4FILE * ) ;
int dfile4read( DATA4FILE *, long, char *, int ) ;
int dfile4readOld( DATA4FILE *, long ) ;
PUBLIC long S4FUNCTION dfile4recCount( DATA4FILE S4PTR *, const long ) ;  /* exported for single-user version (d4recCount replacement) */
/* #define dfile4recordPosition( d4, rec ) ( (unsigned long)(d4)->headerLen + (unsigned long)(d4)->recWidth * ( (rec) - 1 ) ) */
FILE4LONG dfile4recordPosition( DATA4FILE *, long ) ;
#define dfile4recWidth( d4 ) ((unsigned int)(d4)->recWidth)
int dfile4refresh( DATA4FILE * ) ;
#ifndef S4OFF_INDEX
   int dfile4reindex( DATA4FILE * ) ;
   TAG4FILE *dfile4tag( DATA4FILE *, const char * const ) ;
   TAG4FILE *dfile4tagDefault( DATA4FILE * ) ;
   TAG4FILE *dfile4tagNext( DATA4FILE *, TAG4FILE * ) ;
   TAG4FILE *dfile4tagPrev( DATA4FILE *, TAG4FILE * ) ;
   int dfile4tagSelect( DATA4FILE *, TAG4FILE * ) ;
   TAG4FILE *dfile4tagSelected( DATA4FILE * ) ;
   int dfile4updateIndexes( DATA4FILE * ) ;
#endif /* S4OFF_INDEX */
int dfile4updateHeader( DATA4FILE *, int, int ) ;
int dfile4verify( DATA4FILE *, int ) ;
int dfile4writeData( DATA4FILE *, const long, const char * ) ;
#ifndef S4CLIPPER
   #ifndef S4COMP_OFF_INDEX
      INDEX4FILE * dfile4index( DATA4FILE *, const char * ) ;
   #endif
#endif
#ifndef S4OFF_MULTI
   int dfile4lock( DATA4FILE *, const long, const long, const long ) ;
   int dfile4lockAll( DATA4FILE *, const long, const long ) ;
   int dfile4lockAppend( DATA4FILE *, const long, const long ) ;
   int dfile4lockAppendRecord( DATA4FILE *, const long, const long ) ;
   PUBLIC int S4FUNCTION dfile4lockIndex( DATA4FILE *, const long ) ;
   int dfile4lockTest( DATA4FILE *, const long, const long, const long ) ;
   int dfile4lockTestIndex( DATA4FILE *, const long ) ;
   int dfile4lockTestRecs( DATA4FILE *, const long, const long ) ;
   int dfile4unlockData( DATA4FILE *, const long, const long ) ;
   int dfile4unlockFile( DATA4FILE *, const long, const long ) ;
   int dfile4unlockRecords( DATA4FILE *, const long, const long ) ;
   int dfile4unlockAppend( DATA4FILE *, const long, const long ) ;
   int dfile4unlockRange( DATA4FILE *, const long, long ) ;
      int dfile4lockFile( DATA4FILE *, const long, const long ) ;
#endif /* S4OFF_MULTI */
   int dfile4optimize( DATA4FILE *, const int ) ;
   int dfile4optimizeWrite( DATA4FILE *, const int ) ;
   int dfile4packData( DATA4FILE * ) ;
   int dfile4unappendData( DATA4FILE *, const long, const long ) ;
   int dfile4zapData( DATA4FILE *, long, long ) ;
   #ifndef S4INDEX_OFF
      #ifndef S4OFF_MULTI
         PUBLIC int S4FUNCTION dfile4unlockIndex( DATA4FILE *, const long ) ;
      #endif
   #endif

#ifndef S4OFF_SECURITY
   int d4authorize( DATA4 *, struct SERVER4CLIENTSt * ) ;
#endif
void d4blankLow( DATA4 *, char * ) ;
int d4getTables( DATA4 *data, const char *path ) ;
int d4goData( DATA4 *, long ) ;
int d4goVirtual( DATA4 *, const long, const int, const void *, void * ) ;
int d4read( DATA4 *, const long, char * ) ;
int d4readOld( DATA4 *, const long ) ;
int d4recCountLessEq( DATA4 *, long ) ;
int d4tagUniqueSync( DATA4 * ) ;
int d4tagSyncDo( DATA4 S4PTR *, TAG4 S4PTR * const, int ) ;
#ifndef S4OFF_MULTI
   int d4unlockData( DATA4 * ) ;
   int d4unlockAppend( DATA4 * ) ;
   int d4unlockRecords( DATA4 * ) ;
   int d4unlockFile( DATA4 * ) ;
#endif
#ifndef S4OFF_WRITE
   int d4update( DATA4 * ) ;
   int d4updateRecord( DATA4 *, const int ) ;
#endif
int d4verify( DATA4 *, const int ) ;
#define d4version( d4 ) ( (d4)->dataFile->version )
   int d4packData( DATA4 * ) ;
   int d4writeData( DATA4 *, const long ) ;
   int d4writeKeys( DATA4 *, const long ) ;
   int d4zapData( DATA4 *, const long, const long ) ;
   #ifndef S4OFF_MULTI
      int dfile4registerLocked( DATA4FILE *, const long, int ) ;
      #ifndef S4OFF_MEMO
         int d4validateMemoIds( DATA4 * ) ;
      #endif
   #endif
#ifndef S4OFF_MULTI
      #define d4lockTestIndex( d4 ) ( dfile4lockTestIndex( (d4)->dataFile, 1L ) )
#endif
   #ifndef S4OFF_WRITE
      int d4flushData( DATA4 * ) ;
   #endif

long S4FUNCTION error4number2( const long ) ;
void error4logAppend( CODE4 *c4, int, long, const char *, const char *, const char * ) ;
void error4out( CODE4 *, int, long, const char *, const char *, const char * ) ;
   S4CONST char * e4text( const int ) ;

#ifdef E4FILE_LINE
   extern const char *s4fileName ;
   extern int s4lineNo ;

#endif /* E4FILE_LINE */
   PUBLIC int S4FUNCTION code4lineNo( void ) ;
/*
#ifdef S4CONSOLE
   #define code4lineNoSet( val ) ( s4lineNo = val )
   #define code4fileNameSet( name ) ( s4fileName = name )
#else
*/
   PUBLIC const char *S4FUNCTION code4fileName( void ) ;
   PUBLIC void S4FUNCTION code4lineNoSet( int ) ;
   PUBLIC void S4FUNCTION code4fileNameSet( const char * ) ;
/*
#endif
*/

unsigned file4readLow( FILE4 *, FILE4LONG, void *, unsigned ) ;
int file4writeLow( FILE4 *, FILE4LONG, const void *, unsigned, const int, const int, const int ) ;
int file4tempLow( FILE4 *, CODE4 *, const int, int, const char * ) ;
int file4changeSize( FILE4 *, FILE4LONG ) ;

#ifdef S4WRITE_DELAY
   int file4writeDelay( FILE4 *, unsigned long, const void *, const unsigned, S4DELAY_FUNCTION *, void * ) ;
   int file4writeDelayFlush( FILE4 *, const int ) ;
   #ifndef S4OFF_OPTIMIZE
      int file4writeOpt( FILE4 *, unsigned long, const void *, const unsigned, int doDelay, S4DELAY_FUNCTION *, void * ) ;
   #endif
   #ifdef S4USE_INT_DELAY
      int file4writeDelayMain( void * ) ;
   #else
      void file4writeDelayMain( void * ) ;
   #endif
#else
   #ifndef S4OFF_OPTIMIZE
      int file4writeOpt( FILE4 *, unsigned long, const void *, const unsigned, int doDelay, void *, void * ) ;
   #endif
#endif /* S4WRITE_DELAY */

#ifdef S4READ_ADVANCE
   PUBLIC int S4FUNCTION file4advanceRead( FILE4 *, unsigned, void *, const unsigned, S4ADVANCE_FUNCTION *, void * ) ;
   void file4advanceReadWriteOver( FILE4 *, unsigned long, const unsigned, const void *, const int ) ;
   int file4advanceCancel( FILE4 * ) ;
   void opt4advanceReadBuf( FILE4 *, unsigned long, unsigned ) ;
   #ifdef S4USE_INT_DELAY
      int file4advanceReadMain( void * ) ;
   #else
      void file4advanceReadMain( void * ) ;
   #endif
#endif /* S4READ_ADVANCE */

#ifndef S4OFF_MEMO
   int f4memoFlush( FIELD4 * ) ;
   int f4memoRead( FIELD4 * ) ;       /* Validates memo id's first */
   int f4memoReadLow( FIELD4 * ) ;   /* Assumes the current memo id is valid */
   int f4memoReset( FIELD4 * ) ;      /* Resets to 'Unknown state' */
   int f4memoUpdate( FIELD4 * ) ;
   int f4memoWrite( FIELD4 * ) ;
#endif

#ifndef S4OFF_WRITE
   PUBLIC int S4FUNCTION f4memoSetLen( FIELD4 *, const unsigned int ) ; /* need to export for ++ API */
#endif

#ifndef S4OFF_INDEX
   #ifndef S4CLIPPER
      INDEX4 * index4create( DATA4FILE *, const char *, const TAG4INFO * ) ; /* 0 name -> productn */
      INDEX4FILE * index4open( DATA4 *, const char *, INDEX4 * ) ;
      int index4close( INDEX4FILE * ) ;
   #else
      void index4swapBlockClipper(void *, int, int ) ;
   #endif
    /* #ifdef OLEDB5BUILD */
      PUBLIC int S4FUNCTION tfile4versionCheckFree( TAG4FILE * ) ;
    /* #endif */

      int i4check( INDEX4 * ) ;
      void i4deleteRemoveKeys( INDEX4 * ) ;
      int i4flush( INDEX4 * ) ;
      int i4readBlock( FILE4 *, const long, B4BLOCK *, B4BLOCK * ) ;
      int i4shrink( INDEX4 *, long ) ;  /* Returns a block of disk space */
      int i4unlock( INDEX4 * ) ;
      int i4update( INDEX4 * ) ;
      int i4updateHeader( INDEX4 * ) ;
      PUBLIC int S4FUNCTION i4versionCheck( INDEX4 S4PTR *, const int, const int ) ;
      #ifndef S4OFF_TRAN
         TAG4KEY_REMOVED *t4keyFind( TAG4 *, long, char * ) ;
      #endif
      #ifdef S4CLIPPER
         int i4setup( CODE4 *, DATA4 *, const char *, int ) ;
         int tfile4lockTest( TAG4FILE * ) ;
         int i4lock( INDEX4 * ) ;
         PUBLIC int S4FUNCTION tfile4versionCheck( TAG4FILE S4PTR *, const int, const int ) ;
         #define t4versionCheck( t4, a, b ) ( tfile4versionCheck( (t4)->tagFile, (a), (b) ) )
      #else
         int index4flush( INDEX4FILE * ) ;
         int index4lock( INDEX4FILE *, const long ) ;
         int index4isProduction( INDEX4FILE * ) ;
         int index4lockTest( INDEX4FILE * ) ;
         int index4shrink( INDEX4FILE *, long ) ;  /* Returns a block of disk space */
         int index4unlock( INDEX4FILE *, const long ) ;
         int index4update( INDEX4FILE * ) ;
         int index4updateHeader( INDEX4FILE * ) ;
         int index4versionCheck( INDEX4FILE *, const int ) ;
         long index4extend( INDEX4FILE * ) ;   /* Allocates a block at the end of the file */
         PUBLIC int S4FUNCTION t4versionCheck( TAG4 *, const int, const int ) ;
      #endif  /* S4CLIPPER */
#endif /* S4OFF_INDEX */
   #ifndef S4OFF_MULTI
      int lock4verify( LOCK4 *, const int ) ;
      int lock4lock( LOCK4 * ) ;
      int lock4unlock( LOCK4 * ) ;
   #endif /* S4OFF_MULTI */

PUBLIC int S4FUNCTION l4check( LIST4 * ) ;
int l4seek( const LIST4 *, const void * ) ;

int l4lockCheck( void ) ;
void l4lockRemove( int, long, long ) ;
void l4lockSave( int, long, long ) ;

void mem4init( void ) ;
int mem4reset( void ) ;
#ifdef S4MEM_DBF
   INDEX4 *mem4index( void ) ;
#endif
#ifdef S4SEMAPHORE
   int mem4start( CODE4 * ) ;
   void mem4stop( CODE4 * ) ;
#endif
#ifdef S4MEM_PRINT
   #define mem4alloc( a ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4allocDefault( a ) )
   #define mem4alloc2( a, b ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4alloc2Default( a, b ) )
   #define mem4allocChunk( a ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4allocChunkDefault( a ) )
   #define mem4free( a, b ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4freeDefault( a, b ), b = 0 )
#else
   #define mem4alloc( a ) mem4allocDefault( a )
   #define mem4alloc2( a, b ) mem4alloc2Default( a, b )
   #define mem4allocChunk( a ) mem4allocChunkDefault( a )
   #define mem4free( a, b ) ( mem4freeDefault( a, b ), b = 0 )
#endif
#ifdef S4MEM_PRINT
   #define mem4create( a, b, c, d, e ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4createDefault( a, b, c, d, e ) )
   #define mem4createAlloc( a, b, c, d, e, f ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), mem4createAllocDefault( a, b, c, d, e, f ) )
#else
   #define mem4create( a, b, c, d, e ) mem4createDefault( a, b, c, d, e )
   #define mem4createAlloc( a, b, c, d, e, f ) mem4createAllocDefault( a, b, c, d, e, f )
#endif

#ifndef S4OFF_MEMO
      int memo4fileCheck(     MEMO4FILE * ) ;
      int memo4fileCreate(    MEMO4FILE *, CODE4 *, DATA4FILE *, const char * );
      int memo4fileOpen(      MEMO4FILE *, DATA4FILE *, char * ) ;
      int memo4fileReadPart(  MEMO4FILE *, long , char * *, unsigned int *, unsigned long, const unsigned int, long * ) ;
      int memo4fileWrite(     MEMO4FILE *, long *, const char *, const unsigned int ) ;
      int memo4fileWritePart( MEMO4FILE *, long *, const char *, const long, const long, const unsigned int, const long ) ;
      #ifdef S4MFOX
         int memo4fileDump( MEMO4FILE *, const long, const char *, const unsigned int, const long, const long ) ;
         int memo4fileRead( MEMO4FILE *, long , char * *, unsigned int *, long * ) ;
      #else
         int memo4fileDump( MEMO4FILE *, const long, const char *, const unsigned int, const long ) ;
         int memo4fileRead( MEMO4FILE *, long , char * *, unsigned int * ) ;
         #ifndef S4MNDX
            int memo4fileChainFlush( MEMO4FILE *, MEMO4CHAIN_ENTRY * ) ;
            int memo4fileChainSkip( MEMO4FILE *, MEMO4CHAIN_ENTRY * ) ;
               PUBLIC int S4FUNCTION f4memoCheck( MEMO4FILE S4PTR *, DATA4 S4PTR * ) ;
         #endif /* S4MNDX  */
      #endif /* S4MFOX  */

   int memo4fileLock( MEMO4FILE * ) ;
   int memo4fileUnlock( MEMO4FILE * ) ;
   long memo4lenPart( MEMO4FILE *, long ) ;
#endif /* S4OFF_MEMO */

#ifndef S4OFF_INDEX
      PUBLIC int S4FUNCTION t4uniqueSetLow( TAG4 *, const short, const char ) ;

      /* #ifdef S4HAS_DESCENDING */  /* removed from switch to facilatate index independant code */
      PUBLIC void S4FUNCTION tfile4descending( TAG4FILE *, const unsigned short int ) ;
      /* #endif */
      int t4addCalc( TAG4 *, long ) ; /* Calculates expression and adds */
      #define tfile4alias( t4 ) ( (t4)->alias )
      B4BLOCK *tfile4block( TAG4FILE * ) ;
      PUBLIC int S4FUNCTION tfile4bottom( TAG4FILE * ) ;
      int t4check( TAG4 * ) ;
      PUBLIC long S4FUNCTION tfile4dskip( TAG4FILE *, long ) ;
      int tfile4down( TAG4FILE * ) ;
      int tfile4dump( TAG4FILE *, int, const int ) ;
      int tfile4empty( TAG4FILE * ) ;
      PUBLIC int S4FUNCTION tfile4eof( TAG4FILE S4PTR * ) ;
      PUBLIC int S4FUNCTION tfile4exprKey( TAG4FILE S4PTR *, unsigned char S4PTR * S4PTR * ) ;
      int tfile4freeAll( TAG4FILE * ) ;
      int tfile4freeSaved( TAG4FILE * ) ;
      PUBLIC int S4FUNCTION tfile4go( TAG4FILE *, const unsigned char *, const long, const int ) ;
      int tfile4goEof( TAG4FILE * ) ;
      int tfile4go2( TAG4FILE *, const unsigned char *, const long, const int ) ;
      int tfile4initSeekConv( TAG4FILE *, int ) ;    /* Initialize 'stok' and 'dtok' */
      PUBLIC char * S4FUNCTION tfile4key( TAG4FILE * ) ;
      B4KEY_DATA *tfile4keyData( TAG4FILE * ) ;              /* The current key */
      int tfile4outOfDate( TAG4FILE * ) ;
      int tfile4position2( TAG4FILE *, double * ) ;
      double tfile4position( TAG4FILE * ) ;              /* Returns the position as a percent */
      double tfile4positionDbl( TAG4FILE * ) ;              /* Returns the position as a percent */
      int tfile4positionSet( TAG4FILE *, const double ) ;  /* Positions a percentage */
      PUBLIC long S4FUNCTION tfile4recNo( TAG4FILE * ) ;
      int tfile4removeCurrent( TAG4FILE * ) ;        /* Remove the current key */
      int tfile4remove( TAG4FILE *, const unsigned char *, const long ) ;  /* Remove specified key */
      int tfile4removeCalc( TAG4FILE *, long ) ; /* Calculates expression and removes */
      int tfile4rlBottom( TAG4FILE * ) ;
      int tfile4rlTop( TAG4FILE * ) ;
      PUBLIC int S4FUNCTION tfile4seek( TAG4FILE *, const void *, const int ) ;    /* r4success, r4found, r4after, r4eof */
      #ifdef S4FOX
         #ifdef S4VFP_KEY
            int tfile4vfpKey( TAG4FILE * ) ;
         #endif
         int tfile4setCollatingSeq( TAG4FILE *, const int ) ;
         int tfile4setCodePage( TAG4FILE *, const int ) ;
      #endif
      PUBLIC long S4FUNCTION tfile4skip( TAG4FILE *, long ) ;
      PUBLIC int S4FUNCTION tfile4top( TAG4FILE * ) ;
      int tfile4type( TAG4FILE * ) ;
      int tfile4up( TAG4FILE * ) ;
      int tfile4update( TAG4FILE * ) ;
      int tfile4upToRoot( TAG4FILE * ) ;
      #ifdef S4CLIPPER
         int tfile4add( TAG4FILE *, unsigned char *, const long, short int ) ;  /* Returns r4unique, r4success, r4repeat */
         int tfile4balance( TAG4FILE *, B4BLOCK *, int ) ;
         B4BLOCK *tfile4split( TAG4FILE *, B4BLOCK *, const int ) ;
         int t4reindex( TAG4 * ) ;
         int tfile4flush( TAG4FILE * ) ;
      #else
         int tfile4addDo( TAG4FILE *, const unsigned char *, const long, short int ) ;
         #ifdef S4FOX
            int tfile4add( TAG4FILE *, const unsigned char *, const long, short int ) ;  /* Returns r4unique, r4success, r4repeat */
         #else
            #define tfile4add( t4, k, r, e ) ( tfile4addDo( (t4), (k), (r), (e) ) )
         #endif
         B4BLOCK *tfile4split( TAG4FILE *, B4BLOCK * ) ;
      #endif
#endif /* S4OFF_INDEX */

#ifdef S4TESTING
   #ifdef S4WINDOWS
      PUBLIC void S4FUNCTION u4terminate( void );
   #endif
      void u4setField( const char *, const char *, const long, const char *, const char * ) ;
      int ats4readInfo( char *, void *, const unsigned int ) ;
      PUBLIC void S4FUNCTION ats4setSuiteStatus( const char * ) ;
   PUBLIC void S4FUNCTION u4writeErr( const char S4PTR *, int ) ;
   void u4writeOut( char *, int, long  ) ;
#else
   #ifdef S4TRACK_FILES
      void u4writeErr( const char *, int ) ;
   #else
      PUBLIC void S4FUNCTION u4writeErr( const char S4PTR *, int ) ;
   #endif
#endif

#ifdef S4MAX
   long u4allocated( void ) ;
   long u4max( void ) ;
#endif

#ifdef S4MEM_PRINT
   extern const char *m4fileName ;
   extern int m4lineNo ;

   #ifdef S4CONSOLE
      #define code4memLineNoSet( val ) ( m4lineNo = val )
      #define code4memFileNameSet( val ) ( m4fileName = val )
   #else
      PUBLIC void S4FUNCTION code4memLineNoSet( int ) ;
      PUBLIC void S4FUNCTION code4memFileNameSet( const char S4PTR * ) ;
   #endif
#endif

PUBLIC FIXED4MEM S4FUNCTION u4allocFixedDefault( CODE4 S4PTR *, long ) ;    /* exported for server */
PUBLIC void S4FUNCTION u4delayHundredth( const unsigned int ) ;  /* exported for testing */
void u4delaySec( void ) ;
char *u4environ( char *, const int ) ;
#ifdef S4WINTEL
FILE4LONG u4filelength( HANDLE ) ;
#else
FILE4LONG u4filelength( int ) ;
#endif
void u4nameMake( char *, const int, const char *, const char *, const char * ) ;
PUBLIC void S4FUNCTION u4nameMakeFindDrive( char *, const int, const char *, const char * ) ;
PUBLIC int S4FUNCTION u4namePath( char S4PTR *, const unsigned int, const char S4PTR * ) ;
int u4nameRetExt( char *, const int, const char * ) ;
#ifndef S4NO_RENAME
   int u4rename( const char *, const char * ) ;
#endif

#define u4allocErr( a, b ) ( u4allocEr( (a), (b) ) )

#ifdef S4MEM_PRINT
   #define u4alloc( a ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4allocDefault( a ) )
   #define u4allocEr( a, b ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4allocErDefault( (a), (b) ) )
   #define u4allocFixed( a, b ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4allocFixedDefault( (a), (b) ) )
   #define u4allocFree( a, b ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4allocFreeDefault( (a), (b) ) )
   #define u4allocAgain( a, b, c, d ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4allocAgainDefault( (a), (b), (c), (d) ) )
   #define u4free( a ) ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4freeDefault( a ), (a) = 0 )
   #define u4freeFixed( a ) ( ( code4memFileNameSet( __FILE__ ), code4memLineNoSet( __LINE__ ), u4freeFixedDefault( a ) ) )
#else
   #define u4alloc( a ) ( u4allocDefault( a ) )
   #define u4allocEr( a, b ) ( u4allocErDefault( a, b ) )
   #define u4allocFixed( a, b ) ( u4allocFixedDefault( a, b ) )
   #define u4allocFree( a, b ) ( u4allocFreeDefault( a, b ) )
   #define u4allocAgain( a, b, c, d ) ( u4allocAgainDefault( a, b, c, d ) )
   #define u4free( a ) ( u4freeDefault( a ), a = 0 )
   #define u4freeFixed( a ) ( u4freeFixedDefault( a ) )
#endif

/* MISC */
PUBLIC short S4FUNCTION x4reverseShort( const void S4PTR * ) ;  /* exported for communications DLL */
PUBLIC S4LONG S4FUNCTION x4reverseLong( const void S4PTR * ) ;  /* exported for OLEDB */
double S4FUNCTION x4reverseDouble( const void S4PTR * ) ;

#ifndef S4FOX
   #ifndef S4CLIPPER
      int S4CALL c4bcdCmp(  S4CMP_PARM, S4CMP_PARM, size_t ) ;
      int S4CALL t4cmpDoub( S4CMP_PARM, S4CMP_PARM, size_t ) ;
   #endif
#endif

#ifdef E4ANALYZE_ALL
   int file4writePart( const void *, FILE4 *, long, unsigned ) ;
   int file4cmpPart( CODE4 *, void *, FILE4 *, long, unsigned ) ;
   int file4cmp( FILE4 * ) ;
   int file4partLenSet( FILE4 *, unsigned long ) ;
#endif

/* VISUAL BASIC */
#ifdef S4VBASIC
      PUBLIC int c4parm_check( const void S4PTR *, int, const long ) ;
#endif

#ifndef S4OFF_THREAD
   int semaphore4init( SEMAPHORE4 * ) ;
   int semaphore4initUndo( SEMAPHORE4 * ) ;
   void semaphore4release( SEMAPHORE4 * ) ;
   int semaphore4wait( SEMAPHORE4 *, int ) ;
#endif

#ifdef __cplusplus
   }
#endif

#ifdef S4LANGUAGE
   #ifdef S4GERMAN
      #ifndef S4FOX
         int S4CALL u4memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
      #endif
   #else
      int S4CALL u4memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
   #endif
#else
   #ifdef S4ANSI
      int S4CALL u4memcmp( S4CMP_PARM, S4CMP_PARM, size_t ) ;
   #endif
#endif

   void c4bcdFromA( char *, const char *, const int ) ; /*Removed from S4MDX to facilitate index independent OLEDB*/

#ifdef S4MDX
   void t4strToDateMdx(   char *, const char *, const int ) ;
   void t4noChangeStr(    char *, const char *, const int ) ;
   void c4bcdFromD(       char *, const double ) ;
   void t4noChangeDouble( char *, const double ) ;
#endif

void t4dblToFox(      char *, const double ) ;
void t4curToFox(      char *, const CURRENCY4 * ) ;
void t4intToFox(      char *, const long * ) ;
void t4dateTimeToFox( char *, const long * ) ;
void t4dblToCurFox(   char *, const double ) ;
void t4dblToCur(      char *, const double ) ;

#ifdef S4FOX
      int S4CALL u4keycmp( S4CMP_PARM, S4CMP_PARM, size_t, size_t, size_t, T4VFP * ) ;

   /* collating sequence tables */
   #ifdef S4GENERAL
      extern unsigned char v4general[2][256] ;
      extern compressedChars v4generalComp[4] ;
   #endif

   /* codepage tables */
      extern unsigned char CodePage_1252[128] ;   /* the default */
   #ifdef S4CODEPAGE_437
      extern unsigned char CodePage_437[128] ;
   #endif

   PUBLIC int S4FUNCTION expr4currency( const EXPR4 * ) ;
   void t4dtstrToFox( char *, const char *, const int ) ;
   void t4noChangeStr( char *, const char *, const int ) ;
   void t4strToFox( char *, const char *, const int ) ;
   void t4strToLog( char *, const char *, const int ) ;

   /* TRANSLATING FUNCTION TO SUPPORT VISUAL FOXPRO COLLATING SEQUENCES */
   #ifdef S4VFP_KEY
      int t4strToVFPKey( char *, const char *, const int, const int, T4VFP * ) ;
   #endif
#endif
PUBLIC int S4FUNCTION expr4currency( const EXPR4 * ) ;

#ifdef S4CLIPPER
   void t4strToDoub( char *, const char *, const int ) ;
   void t4noChangeStr( char *, const char *, const int ) ;
   void t4strToClip( char *, const char *, const int ) ;
   void t4dateDoubToStr( char *, const double ) ;
#endif

