/* c4trans.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifndef S4OFF_TRAN
   #define TRAN4ENTRY_LEN unsigned S4LONG
#endif  /* S4OFF_TRAN */

#ifdef __cplusplus
   extern "C" {
#endif

#ifdef S4SERVER
   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
         #define S4TRAN_DEFINED
         #ifdef S4INLINE
            #define code4tranStart( c4 )        ( tran4lowStart( &((c4)->currentClient->trans), (c4)->currentClient->id, 0 ) )
            #define code4tranStartSingle( c4 )  ( tran4lowStart( &((c4)->currentClient->trans), (c4)->currentClient->id, 0 ) )
            #define code4tranCommitPhaseOne( c4 ) (tran4lowCommitPhaseOne( &(c4)->currentClient->trans, (c4)->currentClient->id ))
            #define code4tranCommitPhaseTwo( c4, d ) (tran4lowCommitPhaseTwo( &(c4)->currentClient->trans, (c4)->currentClient->id, d ) )
            #define code4tranRollback( c4 )     ( tran4lowRollback( &((c4)->currentClient->trans), (c4)->currentClient->id, 1 ) )
            #define code4tranRollbackSingle( c4 )     ( tran4lowRollback( &((c4)->currentClient->trans), (c4)->currentClient->id, 0 ) )
            #define code4tranStatus( c4 ) ( (c4)->currentClient->trans.currentTranStatus )
            #define code4tranStatusSet( c4, val ) ( (c4)->currentClient->trans.currentTranStatus = (val) )
         #else
            S4EXPORT int S4FUNCTION code4tranCommitPhaseOne( CODE4 S4PTR * ) ;
            S4EXPORT int S4FUNCTION code4tranCommitPhaseTwo( CODE4 S4PTR *, int ) ;
            S4EXPORT int S4FUNCTION code4tranStart( CODE4 S4PTR * ) ;
            S4EXPORT int S4FUNCTION code4tranStartSingle( CODE4 S4PTR * ) ;
            S4EXPORT int S4FUNCTION code4tranRollback( CODE4 S4PTR * ) ;
            S4EXPORT int S4FUNCTION code4tranStatus( CODE4 * ) ;
            S4EXPORT int S4FUNCTION code4tranStatusSet( CODE4 *, const int val ) ;
         #endif
      #endif
   #endif
   #ifdef S4TRAN_DEFINED
      #undef S4TRAN_DEFINED
   #else
      #define code4tranStart( c4 ) ( 0 )
      #define code4tranRollback( c4 ) ( 0 )
      #define code4tranStatus( c4 ) ( r4inactive )
   #endif
#else
   #ifdef __cplusplus
      extern "C" {
   #endif
   /* exported for utilities */
   S4EXPORT int S4FUNCTION code4tranCommitPhaseOne( CODE4 S4PTR * ) ;
   S4EXPORT int S4FUNCTION code4tranStart( CODE4 S4PTR * ) ;
   S4EXPORT int S4FUNCTION code4tranRollback( CODE4 S4PTR * ) ;
   S4EXPORT int S4FUNCTION code4tranCommitPhaseOne( CODE4 S4PTR * ) ;
   S4EXPORT int S4FUNCTION code4tranCommitPhaseTwo( CODE4 S4PTR *, int ) ;
   S4EXPORT int S4FUNCTION code4tranInit( CODE4 S4PTR * ) ;
   #ifdef S4OFF_TRAN
      #ifdef S4CLIENT
         #define code4tranStatus( c4 ) ( (c4)->c4trans.trans.currentTranStatus )
      #else
         #define code4tranStatus( c4 ) ( 0 )
      #endif
   #else
      #ifndef S4OFF_WRITE
         S4EXPORT int S4FUNCTION code4tranStartSingle( CODE4 S4PTR * ) ;
         #ifdef S4INLINE
            #define code4tranStatus( c4 ) ( (c4)->c4trans.trans.currentTranStatus )
            #define code4tranStatusSet( c4, val ) ( (c4)->c4trans.trans.currentTranStatus = (val) )
         #else
            S4EXPORT int S4FUNCTION code4tranStatusSet( CODE4 *, const int val ) ;
         #endif
      #endif
   #endif
   #ifndef S4INLINE
      #ifdef S4CLIENT
         int code4tranInit2( CODE4 *, const char *, const char * ) ;
         void code4tranInitUndo( CODE4 * ) ;
      #endif
   #endif
   #ifdef S4STAND_ALONE
      S4EXPORT int S4FUNCTION code4tranInit2( CODE4 S4PTR *, const char S4PTR *, const char S4PTR * ) ;
      void code4tranInitUndo( CODE4 * ) ;
   #endif
   #ifdef __cplusplus
      }
   #endif
#endif  /* S4SERVER */

S4EXPORT int S4FUNCTION code4tranCommit( CODE4 S4PTR * ) ;
S4EXPORT int S4FUNCTION code4tranCommitSingle( CODE4 S4PTR * ) ;

#ifndef S4OFF_WRITE
   #ifndef S4OFF_TRAN
      #ifndef S4OFF_OPTIMIZE
         #define tran4fileFlush( t4 ) ( (t4)->needsFlushing ? file4flush( &((t4)->file) ) : 0 )
      #endif
   #endif

      #ifndef S4OFF_TRAN
         /* #define TRAN4CREATE    1  Not supported*/
         #define TRAN4OPEN             1
         #define TRAN4OPEN_TEMP        2
         #define TRAN4CLOSE            3
         #define TRAN4START            4
         #define TRAN4COMMIT_PHASE_ONE 5
         #define TRAN4COMMIT_PHASE_TWO 6
         #define TRAN4ROLLBACK         7
         #define TRAN4WRITE            8
         #define TRAN4APPEND           9
         #define TRAN4VOID            10
         /* #define TRAN4CREATE_TEMP    11  Not Supported*/
         #define TRAN4PACK            12
         #define TRAN4ZAP             13
         #define TRAN4INIT            15
         #define TRAN4SHUTDOWN        16
         #define TRAN4BACKEDUP        17
         #define TRAN4INIT_UNDO       18

         #define E4_LOG_IFS "Log File - invalid file status"
         #define TRAN4FORWARDS  1
         #define TRAN4BACKWARDS 2
         #define TRAN4LOCK_BASE      (1000000000L)
         #define TRAN4LOCK_SERVER    TRAN4LOCK_BASE+0
         #define TRAN4LOCK_MULTIPLE  TRAN4LOCK_BASE+1
         #define TRAN4LOCK_BACKUP    TRAN4LOCK_BASE+2
         #define TRAN4LOCK_RESTORE   TRAN4LOCK_BASE+3
         #define TRAN4LOCK_FIX       TRAN4LOCK_BASE+4
         #define TRAN4VERSION_NUM 2      /*CJ- changed so older version of CodeUtil would report the correct error*/

         #ifdef S4STAND_ALONE
            #define TRAN4LOCK_USERS     TRAN4LOCK_BASE+1000
            #define TRAN4MAX_USERS      1000
         #endif
      #endif  /* S4OFF_TRAN */

   #ifdef __cplusplus
      extern "C" {
   #endif
   #ifndef S4OFF_TRAN
         int  tran4fileCreate( TRAN4FILE *, const char * ) ;
         int  tran4active( CODE4 *, DATA4 * ) ;
         int  tran4addUser( TRAN4 *, const long, const char *, const unsigned short int ) ;
         /* log file examination functionality */
         S4EXPORT int  S4FUNCTION tran4fileAppend( TRAN4FILE S4PTR *, LOG4HEADER S4PTR *, void S4PTR *, int ) ;  /* a null pointer means use internal value */
         S4EXPORT int  S4FUNCTION tran4fileBottom( TRAN4FILE S4PTR *, TRAN4 S4PTR * ) ;
         S4EXPORT int  S4FUNCTION tran4fileSkip( TRAN4FILE S4PTR *, TRAN4 S4PTR *, const int ) ;
         S4EXPORT int  S4FUNCTION tran4fileTop( TRAN4FILE S4PTR *, TRAN4 S4PTR * ) ;
         int  code4tranInitUndoLow( TRAN4 *, const long ) ;
         S4EXPORT int  S4FUNCTION tran4set(  TRAN4 S4PTR *, const int, const long, const long, const int, const unsigned int, const long, const long ) ;
         S4EXPORT int  S4FUNCTION tran4putData(  TRAN4 S4PTR *, void *, unsigned ) ;
         /* log file examination functionality */
         #ifndef S4INLINE
            S4EXPORT int  S4FUNCTION tran4bottom( TRAN4 S4PTR * ) ;
            S4EXPORT unsigned long S4FUNCTION tran4clientDataId( TRAN4 S4PTR * ) ;
            S4EXPORT long S4FUNCTION tran4clientId( TRAN4 S4PTR * ) ;
            S4EXPORT long S4FUNCTION tran4id( TRAN4 S4PTR * ) ;
            S4EXPORT unsigned S4FUNCTION tran4len( TRAN4 S4PTR * ) ;
            S4EXPORT unsigned long S4FUNCTION tran4serverDataId( TRAN4 S4PTR * ) ;
            S4EXPORT int  S4FUNCTION tran4skip( TRAN4 S4PTR *, int ) ;
            S4EXPORT int  S4FUNCTION tran4top( TRAN4 S4PTR * ) ;
            S4EXPORT int  S4FUNCTION tran4type( TRAN4 S4PTR * ) ;
         #endif
         S4EXPORT void * S4FUNCTION tran4getData( TRAN4 S4PTR *, const long pos  ) ;
   #endif  /* S4OFF_TRAN */

   #ifndef S4OFF_TRAN
         S4EXPORT int S4FUNCTION tran4lowAppend( TRAN4 S4PTR *, void S4PTR *, int ) ;
         S4EXPORT int S4FUNCTION tran4lowCommitPhaseOne( TRAN4 S4PTR *, long ) ;
         S4EXPORT int S4FUNCTION tran4lowCommitPhaseTwo( TRAN4 S4PTR *, long, int ) ;
         S4EXPORT int S4FUNCTION tran4lowRollback( TRAN4 S4PTR *, long, const int ) ;
         S4EXPORT int S4FUNCTION tran4lowStart( TRAN4 S4PTR *, long, int ) ;
         int tran4lowUnappend( TRAN4 * ) ;
         int tran4lowUnwrite( TRAN4 * ) ;
   #endif  /* S4OFF_TRAN */
   #ifdef __cplusplus
      }
   #endif
#endif /* S4OFF_WRITE */

   int tran4closeAll( struct TRAN4St * ) ;
   #ifndef S4SINGLE
      #ifdef S4SERVER
         int tran4lock( TRAN4 * ) ;
      #endif
      S4EXPORT int  S4FUNCTION code4tranLockTransactions( CODE4TRANS S4PTR *, long ) ;
      S4EXPORT int  S4FUNCTION code4tranUnlockTransactions( CODE4TRANS S4PTR *, long ) ;
   #endif  /* S4SINGLE */

DATA4 *tran4data( struct TRAN4St *, const long, const long ) ;
DATA4 *tran4dataName( struct TRAN4St *, const char *, const long, const int ) ;

#ifdef E4ANALYZE
   int  tran4verify( TRAN4 *, int ) ;
#endif  /* E4ANALYZE */

int code4tranInitLow( TRAN4 *, CODE4TRANS * ) ;
int code4transInit( CODE4TRANS *, CODE4 * ) ;
S4EXPORT int S4FUNCTION code4transInitUndo( CODE4TRANS S4PTR * ) ;

#ifdef __cplusplus
   }
#endif
