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


