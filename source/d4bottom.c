/* *********************************************************************************************** */
/* Copyright (C) 1999-2015 by Sequiter, Inc., 9644-54 Ave, NW, Suite 209, Edmonton, Alberta Canada.*/
/* This program is free software: you can redistribute it and/or modify it under the terms of      */
/* the GNU Lesser General Public License as published by the Free Software Foundation, version     */
/* 3 of the License.                                                                               */
/*                                                                                                 */
/* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;       */
/* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.       */
/* See the GNU Lesser General Public License for more details.                                     */
/*                                                                                                 */
/* You should have received a copy of the GNU Lesser General Public License along with this        */
/* program. If not, see <https://www.gnu.org/licenses/>.                                           */
/* *********************************************************************************************** */

/* revisited by altomaltes@gmail.com
 */

/* d4bottom.c  */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifdef S4CLIENT

int S4FUNCTION d4bottom( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
   #endif
   int rc ;
   CONNECTION4 *connection ;
   CONNECTION4BOTTOM_INFO_IN *info ;
   CONNECTION4BOTTOM_INFO_OUT *out ;
   CODE4 *c4 ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E91201 ) ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E91201 ) )
         return -1 ;
   #endif

   c4 = data->codeBase ;

   #ifdef S4OFF_WRITE
      if ( error4code( c4 ) < 0 )
         return e4codeBase ;
   #else
      rc = d4updateRecord( data, 0 ) ;   /* returns -1 if error4code( codeBase ) < 0 */
      if ( rc )
         return rc ;
   #endif

   connection = data->dataFile->connection ;
   #ifdef E4ANALYZE
      if ( connection == 0 )
         return error4( c4, e4connection, E91201 ) ;
   #endif

   connection4assign( connection, CON4BOTTOM, data4clientId( data ), data4serverId( data ) ) ;
   connection4addData( connection, NULL, sizeof( CONNECTION4BOTTOM_INFO_IN ), (void **)&info ) ;
   #ifdef S4INDEX_OFF
      info->usesTag = 0 ;
   #else
      tag = data->tagSelected ;
      if ( tag == 0 )
         info->usesTag = 0 ;
      else
      {
         info->usesTag = 1 ;
         memcpy( info->tagName, tag->tagFile->alias, LEN4TAG_ALIAS  ) ;
         info->tagName[LEN4TAG_ALIAS] = 0 ;
      }
   #endif
   rc = connection4repeat( connection ) ;
   data->recNum = -1 ;
   switch( rc )
   {
      case r4eof:
         data->bofFlag = 1 ;
         return d4goEof( data ) ;
      case r4success:
         break ;
      default:
         if ( rc < 0 )
            connection4error( connection, c4, rc, E91201 ) ;
         return rc ;
   }

   if ( connection4len( connection ) != (long)sizeof( CONNECTION4BOTTOM_INFO_OUT ) + (long)dfile4recWidth( data->dataFile ) )
      return error4( c4, e4packetLen, E91201 ) ;

   out = (CONNECTION4BOTTOM_INFO_OUT *)connection4data( connection ) ;

   /* now copy the data into the record */
   memcpy( data->record, ((char *)out) + sizeof( CONNECTION4BOTTOM_INFO_OUT ), dfile4recWidth( data->dataFile ) ) ;

   data->bofFlag = out->bofFlag ;
   data->eofFlag = out->eofFlag ;

   data->recNum = ntohl(out->recNo) ;
   if ( out->recordLocked )
   {
      d4localLockSet( data, data->recNum ) ;
      memcpy( data->recordOld, data->record, dfile4recWidth( data->dataFile ) ) ;
      data->recNumOld = data->recNum ;
      data->memoValidated = 1 ;
   }

   return 0 ;
}

#else

int S4FUNCTION d4bottom( DATA4 *data )
{
   long rec ;
   CODE4 *c4 ;
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
      long recno ;
      int rc ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E91201 ) )
         return -1;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E91201 ) ;
   #endif

   c4 = data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifndef S4INDEX_OFF
      tag = data->tagSelected ;

      if ( tag == 0 )
      {
   #endif
      rec = d4recCount( data ) ;  /* updates the record, returns -1 if error4code( codeBase ) < 0 */
      if ( rec > 0L )
         return d4go( data, rec ) ;
      if ( rec < 0 )
         return (int)rec ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         #ifndef S4OFF_WRITE
            /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
            rc = d4updateRecord( data, 0 ) ;
            if ( rc )
               return error4stack( data->codeBase, (short)rc, E91201 ) ;
         #endif
         t4versionCheck( tag, 0, 0 ) ;
         rc = tfile4bottom( tag->tagFile ) ;
         if ( rc )
            return error4stack( data->codeBase, (short)rc, E91201 ) ;
         if ( !tfile4eof( tag->tagFile ) )
         {
            recno = tfile4recNo( tag->tagFile ) ;
            if ( recno < 0 )
               return error4stack( data->codeBase, (short)recno, E91201 ) ;
            #ifndef S4SINGLE
               if ( recno > data->dataFile->minCount )   /* ensure correct sequencing for multi-user */
               {
                  if ( recno > d4recCount( data ) )
                  {
                     rc = (int)tfile4skip( tag->tagFile, -1L ) ;
                     if ( rc < 0 )
                        return error4stack( data->codeBase, (short)rc, E91201 ) ;
                     if ( rc == 0 )
                     {
                        data->bofFlag = 1 ;
                        return d4goEof( data ) ;
                     }
                     recno = tfile4recNo( tag->tagFile ) ;
                     if ( recno <= 0 || recno > data->dataFile->minCount )
                        return error4stack( data->codeBase, -1, E91201 ) ;
                  }
               }
            #endif
            rc = d4go( data, recno ) ;
            #ifndef S4OFF_MULTI
               #ifndef S4OFF_TRAN
                  if ( rc == 0 )
                     if ( code4transEnabled( c4 ) )
                        if ( t4unique( tag ) != 0 )
                           #ifdef S4SERVER
                              if ( !dfile4lockTestFile( data->dataFile, data4clientId( data ), data4serverId( data ) ) )
                           #else
                              if ( !d4lockTestFile( data ) )
                           #endif
                              rc = d4tagSyncDo( data, tag, -1 ) ;
               #endif
            #endif
            return rc ;
         }
      }
   #endif

   data->bofFlag = 1 ;
   return d4goEof( data ) ;
}

#endif /* S4CLIENT */

