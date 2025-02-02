/* d4zap.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
int S4FUNCTION d4zap( DATA4 *d4, const long r1, const long r2 )
{
   int rc ;
   CODE4 *c4 ;

   if ( r2 < r1 )   /* simple no records to remove -- get before parm check */
      return 0 ;

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
         return error4( 0, e4parmNull, E94604 ) ;
      if ( r1 < 1 || r2 < 1 )
         return error4( d4->codeBase, e4parm, E94604 ) ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E94604 ) )
         return -1 ;
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( d4->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;

      #ifndef S4SINGLE
         rc = d4lockAll( d4 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
         if ( rc )
            return rc ;
      #endif

      #ifndef S4OFF_TRAN
         if ( code4transEnabled( c4 ) )
            if ( tran4active( c4, d4 ) != 0 )
               return error4( d4->codeBase, e4transViolation, E81503 ) ;
      #endif

      rc = d4zapData( d4, r1, r2 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      if ( rc == 0 )
      {
         if ( d4recCount( d4 ) == 0 )
            d4->bofFlag = d4->eofFlag = 1 ;
         else
            d4->bofFlag = d4->eofFlag = 0 ;

         #ifndef S4INDEX_OFF
            rc = d4reindex( d4 ) ;
         #endif
         dfile4updateHeader( d4->dataFile, 1, 1 ) ;
      }

      return rc ;
}

int d4zapData( DATA4 *data, const long startRec, const long endRec )
{
   int rc ;
   #ifndef S4OFF_TRAN
         TRAN4 *trans ;
         long connectionId ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parmNull, E94605 ) ;
      if ( startRec < 1 || endRec < 1 )
         return error4( data->codeBase, e4parm, E94605 ) ;
   #endif

   #ifndef S4SINGLE
         rc = d4lockFile( data ) ;
      if ( rc )
         return rc ;
   #endif

   rc = d4update( data ) ;   /* returns -1 if error4code( codeBase ) < 0 */
   if ( rc )
      return rc ;

   #ifndef S4OFF_TRAN
         if ( code4transEnabled( data->codeBase ) )
         {
            trans = code4trans( data->codeBase ) ;
               connectionId = 0L ;
            rc = tran4set( trans, trans->currentTranStatus, -1L, connectionId, TRAN4ZAP,
                 2 * sizeof(S4LONG), data4clientId( data ), data4serverId( data ) ) ;
            if ( rc < 0 )
               return rc ;
            rc = tran4putData( trans, (void *)&startRec, sizeof( startRec ) ) ;
            if ( rc < 0 )
               return rc ;
            rc = tran4putData( trans, (void *)&endRec, sizeof( endRec ) ) ;
            if ( rc < 0 )
               return rc ;
            rc = tran4lowAppend( trans, 0, 1 ) ;
            if ( rc < 0 )
               return rc ;
         }
   #endif  /* S4OFF_TRAN */

   rc = dfile4zapData( data->dataFile, startRec, endRec ) ;

   if ( rc < 0 )
      return rc ;

   data->recNum = -1 ;
   data->recNumOld = -1 ;
   d4blankLow( data, data->record ) ;

   return rc ;
}

int dfile4zapData( DATA4FILE *d4, long startRec, long endRec )
{
   long curCount, iRec ;
   char *rdBuf, *wrBuf, *record ;
   FILE4SEQ_READ rd ;
   FILE4LONG len ;
   FILE4SEQ_WRITE wr ;
   unsigned  bufSize ;
   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_LOW
      if ( d4 == 0 )
         return error4( 0, e4parmNull, E91102 ) ;
      if ( startRec < 0 || endRec < 0 )
         return error4( d4->c4, e4parm, E91102 ) ;
   #endif

   c4 = d4->c4 ;
   d4->fileChanged = 1 ;

   #ifdef E4ANALYZE
      rdBuf = wrBuf = record = 0 ;
      if ( c4 == 0 )
         return error4( 0, e4parm, E91102 ) ;
   #endif

   if ( startRec == 0 )
      startRec = 1 ;

   curCount = dfile4recCount( d4, -2L ) ;
   if ( curCount < 0 )
      return -1 ;
   if ( startRec > curCount )
      return 0 ;
   if ( endRec < startRec )
      return 0 ;
   if ( endRec > curCount )
      endRec = curCount ;

   rdBuf = wrBuf = 0 ;
   bufSize = c4->memSizeBuffer ;

   record = (char *)u4allocFree( c4, (long)d4->recWidth ) ;
   if ( record == 0 )
      return error4stack( c4, e4memory, E91102 ) ;

   for ( ; bufSize > d4->recWidth ; bufSize -= 0x800 )
   {
      rdBuf = (char *)u4allocFree( c4, (long)bufSize ) ;
      if ( rdBuf == 0 )
         continue ;

      wrBuf = (char *)u4allocFree( c4, (long)bufSize ) ;
      if ( wrBuf )
         break ;

      u4free( rdBuf ) ;
      rdBuf = 0 ;
   }
   file4seqReadInitDo( &rd, &d4->file, dfile4recordPosition( d4, endRec + 1 ), rdBuf, (rdBuf == 0) ? 0 : bufSize, 1 ) ;
   file4seqWriteInitLow( &wr, &d4->file, dfile4recordPosition( d4, startRec ), wrBuf, (wrBuf == 0) ? 0 : bufSize ) ;

   for ( iRec= endRec+1L; iRec <= curCount; iRec++ )
   {
      file4seqReadAll( &rd, record, d4->recWidth ) ;
      file4seqWrite( &wr, record, d4->recWidth ) ;
   }

   file4seqWrite( &wr, "\032", 1 ) ;
   rc = file4seqWriteFlush( &wr ) ;
   #ifdef S4ADVANCE_READ
      file4seqReadInitUndo( &rd ) ;
   #endif
   u4free( rdBuf ) ;
   u4free( wrBuf ) ;
   u4free( record ) ;

   if ( rc < 0 )
      return -1 ;

   d4->numRecs = curCount - ( endRec - startRec + 1 ) ;
   d4->minCount = curCount - ( endRec - startRec + 1 ) ;

   len = dfile4recordPosition( d4, d4->numRecs + 1L ) ;
   file4longAdd( &len, 1 ) ;
   return file4lenSetLow( &d4->file, len ) ;
}
#endif  /* S4OFF_WRITE */
