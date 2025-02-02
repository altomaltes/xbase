/* d4pack.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
int S4FUNCTION d4pack( DATA4 *d4 )
{
      int rc ;
   CODE4 *c4 ;

   C4PARMDATA( d4, E94601, -1 );

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
               return error4( c4, e4transViolation, E81518 ) ;
      #endif

      rc = d4packData( d4 ) ;  /* returns -1 if error4code( codeBase ) < 0 */
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

int d4packData( DATA4 *d4 )
{
   int rc ;
      #ifndef S4OFF_TRAN
         TRAN4 *trans ;
         long connectionId ;
      #endif

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
         return error4( 0, e4parmNull, E94602 ) ;
   #endif

   rc = d4update( d4 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
   if ( rc )
      return rc ;

   #ifndef S4SINGLE
         rc = d4lockFile( d4 ) ;
      if ( rc )
         return rc ;
   #endif

      #ifndef S4OFF_TRAN
         if ( code4transEnabled( d4->codeBase ) )
         {
            trans = code4trans( d4->codeBase ) ;
               connectionId = 0L ;
            if (  tran4set( trans, trans->currentTranStatus, -1L, connectionId, TRAN4PACK,
                 0, data4clientId( d4 ), data4serverId( d4 ) ) == 0 )
               tran4lowAppend( trans, "\0", 1 ) ;
         }
      #endif

   rc = dfile4packData( d4->dataFile ) ;

   d4->recNum = -1 ;
   d4->recNumOld = -1 ;
   memset( d4->record, ' ', dfile4recWidth( d4->dataFile ) ) ;

   return rc ;
}

int dfile4packData( DATA4FILE *d4 )
{
   char *rdBuf, *wrBuf, *record ;
   FILE4SEQ_READ   rd ;
   FILE4SEQ_WRITE  wr ;
   long newCount, curCount, iRec ;
   int  rc ;
   unsigned bufSize ;
   CODE4 *c4 ;
   FILE4LONG len ;

   #ifdef E4PARM_LOW
      if ( d4 == 0 )
         return error4( 0, e4parmNull, E91102 ) ;
   #endif

   c4 = d4->c4 ;

   d4->fileChanged = 1 ;

   rdBuf = wrBuf = 0 ;
   bufSize = c4->memSizeBuffer ;

   for ( ; bufSize > d4->recWidth; bufSize -= 0x800 )
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

   newCount = 0L ;
   curCount = dfile4recCount( d4, -2L ) ;

   record = (char *)u4allocFree( c4, (long)d4->recWidth ) ;
   if ( record == 0 )
      return -1 ;

   file4seqReadInitDo( &rd, &d4->file, dfile4recordPosition( d4, 1L ), rdBuf, ( rdBuf == 0 ) ? 0 : bufSize, 1 ) ;
   file4seqWriteInitLow( &wr, &d4->file, dfile4recordPosition( d4, 1L ), wrBuf, ( wrBuf == 0 ) ? 0 : bufSize) ;

   for ( iRec= 1L; iRec <= curCount; iRec++ )
   {
      file4seqReadAll( &rd, record, d4->recWidth ) ;
      if ( record[0] == ' ' )
      {
         file4seqWrite( &wr, record, d4->recWidth ) ;
         newCount++ ;
      }
   }

   u4free( record ) ;

   file4seqWrite( &wr, "\032", 1 ) ;
   rc = file4seqWriteFlush( &wr ) ;
   #ifdef S4ADVANCE_READ
      file4seqReadInitUndo( &rd ) ;
   #endif
   u4free( rdBuf ) ;
   u4free( wrBuf ) ;
   if ( rc < 0 )
      return -1 ;

   d4->numRecs = newCount ;
   d4->minCount = newCount ;

   len = dfile4recordPosition( d4, newCount + 1L ) ;
   file4longAdd( &len, 1 ) ;
   return file4lenSetLow( &d4->file, len ) ;
}

#endif  /* S4OFF_WRITE */
