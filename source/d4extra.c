/* d4extra.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifdef E4ANALYZE
#ifdef P4ARGS_USED
   #pragma argsused
#endif

int code4verify( CODE4 *c4, int subs )
{
   if ( !c4 )   return error4( 0, e4parmNull, E93901 ) ;

   if ( error4code( c4 ) < 0 )
      return error4code( c4 ) ;

   return 0 ;
}

int dfile4verify( DATA4FILE *d4, int subs )
{
   int rc ;

   if ( !d4     )  return error4( 0, e4parmNull, E91102 ) ;
   if ( !d4->c4 )  return error4( 0,   e4struct, E91102 ) ;

   if ( subs == 1 )
      if ( ( rc = code4verify( d4->c4, 1 ) ) < 0 )
         return rc ;

   return 0 ;
}

#ifndef S4SINGLE
int lock4verify( LOCK4 *lock, const int subs )
{
   int rc ;

   if ( lock->data == 0 )
      return error4( 0, e4parmNull, E93903 ) ;

   if ( subs == 1 )
      if ( ( rc = dfile4verify( lock->data, 1 ) ) < 0 )
         return rc ;

/*
   if ( lock->link.n == 0 || lock->link.p == 0 )
      return error4( lock->data->c4, e4struct, E93903 ) ;
*/

   return 0 ;
}
#endif  /* S4SINGLE */
#endif  /* E4ANALYZE */

#ifndef S4SINGLE
int lock4lock( LOCK4 *lock )
{
   int rc ;

#ifdef E4ANALYZE
   if ( ( rc = lock4verify( lock, 1 ) ) < 0 )
      return rc ;
#endif

   switch( lock->id.type )
   {
      case LOCK4ALL:    rc = dfile4lockAll(    lock->data, lock->id.clientId, lock->id.serverId ) ;      break ;
      case LOCK4APPEND: rc = dfile4lockAppend( lock->data, lock->id.clientId, lock->id.serverId ) ;         break ;
      case LOCK4FILE:   rc = dfile4lockFile(   lock->data, lock->id.clientId, lock->id.serverId ) ;         break ;
      case LOCK4RECORD: rc = dfile4lock(       lock->data, lock->id.clientId, lock->id.serverId, lock->id.recNum ) ;         break ;
      default:          rc = error4( lock->data->c4, e4lock, E83901 ) ;         break ;
   }

   return rc ;
}

int lock4unlock( LOCK4 *lock )
{
   int rc, oldUnlockAuto ;

#ifdef E4ANALYZE
   if ( ( rc = lock4verify( lock, 1 ) ) < 0 )
      return rc ;
#endif

   oldUnlockAuto = code4unlockAuto( lock->data->c4 ) ;
   code4unlockAutoSet( lock->data->c4, 1 ) ;

   switch ( lock->id.type )
   {
      case LOCK4ALL:
         #ifndef N4OTHER
            #ifndef S4MEMO_OFF
               dfile4memoUnlock( lock->data ) ;
            #endif
         #endif
         #ifndef S4INDEX_OFF
            dfile4unlockIndex( lock->data, lock->id.serverId ) ;
         #endif
         rc = dfile4unlockFile( lock->data, lock->id.clientId, lock->id.serverId ) ;
      break ;

      case LOCK4APPEND:
         rc = dfile4unlockAppend( lock->data, lock->id.clientId, lock->id.serverId ) ;
      break ;

      case LOCK4FILE:
         rc = dfile4unlockFile( lock->data, lock->id.clientId, lock->id.serverId ) ;
      break ;

      case LOCK4RECORD:
         rc = dfile4unlockRecord( lock->data, lock->id.clientId, lock->id.serverId, lock->id.recNum ) ;
      break ;

      default:
         rc = error4( lock->data->c4, e4info, E93801 ) ;
         break ;
   }

   code4unlockAutoSet( lock->data->c4, oldUnlockAuto ) ;

   return rc ;
}
#endif  /* S4SINGLE */
