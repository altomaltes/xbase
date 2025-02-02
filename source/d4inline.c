/* d4inline.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"


#ifndef S4INLINE

/*
#ifndef S4SINGLE
int trans4unlockAuto( TRAN4 *t4 )
{
   return t4->unlockAuto ;
}

void trans4unlockAutoSet( TRAN4 *t4, int val )
{
   t4->unlockAuto = val ;
}
#endif
*/

#ifndef S4SINGLE
int code4unlockAutoSave( CODE4 *c4 )
{
   return c4->c4trans.trans.savedUnlockAuto ;
}

int S4FUNCTION code4unlockAuto( CODE4 *c4 )
{
   return c4->c4trans.trans.unlockAuto ;
}

PUBLIC void S4FUNCTION code4unlockAutoSet( CODE4 *c4, int val )
{
   c4->c4trans.trans.unlockAuto = val ;
}
#endif  /* S4SINGLE */

#ifndef S4OFF_WRITE
#ifndef S4OFF_TRAN
int S4FUNCTION code4tranStatus( CODE4 *c4 )
{
   return c4->c4trans.trans.currentTranStatus ;
}

PUBLIC int S4FUNCTION code4tranStatusSet( CODE4 *c4, const int val )
{
   return c4->c4trans.trans.currentTranStatus = val ;
}
#endif
#endif


#ifndef S4OFF_TRAN
int code4transEnabled( CODE4 *c4 )
{
      return ( c4->c4trans.enabled && code4tranStatus( c4 ) != r4rollback && code4tranStatus( c4 ) != r4off ) ;
}
#endif  /* S4OFF_TRAN */

TRAN4 *code4trans( CODE4 *c4 )
{
   return &c4->c4trans.trans ;
}

#ifndef S4OFF_TRAN
unsigned short int tran4entryLen( LOG4HEADER *header )
{
   return sizeof( LOG4HEADER ) + header->dataLen + sizeof( TRAN4ENTRY_LEN ) ;
}
#endif

int tran4dataListSet( TRAN4 *t4, LIST4 *list )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
         return error4( 0, e4parmNull, E93839 ) ;
   #endif
   t4->dataList = list ;
   return 0 ;
}

LIST4 *S4FUNCTION tran4dataList( TRAN4 *t4 )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 )
      {
         error4( 0, e4parmNull, E93838 ) ;
         return 0 ;
      }
   #endif
   return t4->dataList ;
}
#endif   /* S4INLINE */
