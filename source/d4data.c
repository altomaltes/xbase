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


#include "d4all.h"

#ifndef S4INLINE
unsigned long S4FUNCTION data4serverId( DATA4 *d4 )
{
   return d4->clientId ;
}

unsigned long data4clientId( DATA4 *d4 )
{
   return d4->clientId ;
}


#endif  /* S4INLINE */

S4CONST char *S4FUNCTION d4alias( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93301 ) )
         return (char *) NULL;
   #endif  /* S4VBASIC */

   E4PARM_HRET( data, E93301, NULL ) ;

   return data->alias ;
}

void S4FUNCTION d4aliasSet( DATA4 *data, const char * newAlias )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93302 ) )
         return ;
   #endif  /* S4VBASIC */

   E4PARM_HRET( data && newAlias, E93302, ) ;

   u4ncpy( data->alias, newAlias, sizeof( data->alias ) ) ;
   #ifndef S4CASE_SEN
      c4upper( data->alias ) ;
   #endif


   return ;
}

void d4blankLow( DATA4 *data, char *record )
{
   if ( data->recordBlank != 0 )
      memcpy( record, data->recordBlank, (unsigned int)dfile4recWidth( data->dataFile ) ) ;
   else
      memset( record, ' ', (unsigned int)dfile4recWidth( data->dataFile ) ) ;

   return ;
}

void S4FUNCTION d4blank( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93303 ) )
         return ;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93303 ) ;

      #ifndef S4OFF_ENFORCE_LOCK
         if ( data->codeBase->lockEnforce && data->recNum > 0L )
            if ( d4lockTest( data, data->recNum ) != 1 )
            {
               error4( data->codeBase, e4lock, E93303 ) ;
               return ;
            }
      #endif

   d4blankLow( data, data->record ) ;

   data->recordChanged = 1 ;

   return ;
}

int S4FUNCTION d4bof( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93304 ) )
         return -1;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93304 ) ;

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   return data->bofFlag ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION d4delete( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93305 ) )
         return ;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93305 ) ;
   #ifdef E4MISC
      if ( data->record[0] != ' ' && data->record[0] != '*' )
      {
         error4( data->codeBase, e4info, E83301 ) ;
         return ;
      }
   #endif

   if ( data->record[0] != '*' )
   {
         #ifndef S4OFF_ENFORCE_LOCK
            if ( data->codeBase->lockEnforce && data->recNum > 0L )
               if ( d4lockTest( data, data->recNum ) != 1 )
               {
                  error4( data->codeBase, e4lock, E93305 ) ;
                  return ;
               }
         #endif
        data->record[0] = '*' ;
      data->recordChanged = 1 ;
   }

   return ;
}
#endif

#ifdef S4OLEDB_OR_NOT_SERVER
int S4FUNCTION d4deleted( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93306 ) )
         return -1;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93306 ) ;
   #ifdef E4MISC
      if ( data->record[0] != ' ' && data->record[0] != '*' )
         return error4( data->codeBase, e4info, E83301 ) ;
   #endif

   return *data->record != ' ' ;
}
#endif /* S4OLEDB_OR_NOT_SERVER */

int S4FUNCTION d4eof( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93307 ) )
         return -1;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93307 ) ;

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   return data->eofFlag ;
}

const char *S4FUNCTION d4fileName( DATA4 *data )
{
  E4PARM_HRET( data, E93305, NULL ) ;

  return data->dataFile->file.name ;
}

#ifndef S4OFF_TRAN
#ifndef S4OFF_WRITE
/* can change LOG4ON-->LOG4TRANS or LOG4TRANS-->LOG4ON */
int S4FUNCTION d4log( DATA4 *data, const int logging )
{
   int oldVal ;

   E4PARHIGH( data, E93319 ) ;

   if ( code4transEnabled( data->codeBase ) == 0 )
   {
      if ( logging == -1 )
         return r4logOff ;
      else
         return error4( data->codeBase, e4trans, E83807 ) ;
   }

   if ( data->logVal == LOG4ALWAYS )
      return r4logOn ;

   oldVal = ( data->logVal == 0 ) ? 0 : 1 ;

   if ( logging != -1 )
      if ( logging )
      {
         if ( data->logVal == LOG4TRANS )
            data->logVal = LOG4ON ;
      }
      else
         if ( data->logVal == LOG4ON )
            data->logVal = LOG4TRANS ;

   return oldVal ;
}
#endif /* S4OFF_WRITE */
#endif /* S4OFF_TRAN */

short int S4FUNCTION d4numFields( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93308 ) )
         return -1;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93308 ) ;

   #ifdef CLIENT_OR_FOX
      if ( data->fields[data->dataFile->nFields-1].type == '0' )   /* null flags field */
         return data->dataFile->nFields - 1 ;
   #endif

   return data->dataFile->nFields ;
}

/*  currently this function is not used...
int d4read( DATA4 *data, const long recNum, char *ptr )
{
   #ifdef E4PARM_LOW
      if ( data == 0 || recNum <= 0 || ptr == 0 )
         return error4( 0, e4parm, E93309 ) ;
   #endif

   return dfile4read( data->dataFile, recNum, ptr, 0 ) ;
}
*/

int d4readOld( DATA4 *data, const long recNum )
{
   int rc ;

   #ifdef E4PARM_LOW
      if ( data == 0 || recNum <= 0 )
         return error4( 0, e4parm, E93310 ) ;
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   if ( recNum <= 0 )
   {
      data->recNumOld = recNum ;
      d4blankLow( data, data->recordOld ) ;
   }

   if ( data->recNumOld == recNum )
      return 0 ;

   data->recNumOld = -1 ;
   rc = dfile4read( data->dataFile, recNum, data->recordOld, 1 ) ;
   if ( rc < 0 )
      return -1 ;
   if ( rc > 0 )
      d4blankLow( data, data->recordOld ) ;
   data->recNumOld = recNum ;

   return 0 ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION d4recall( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93311 ) )
         return ;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93311 ) ;

   #ifdef E4MISC
      if ( data->record[0] != ' ' && data->record[0] != '*' )
      {
         error4( data->codeBase, e4info, E83301 ) ;
         return ;
      }
   #endif

   if ( *data->record != ' ' )
   {
         #ifndef S4OFF_ENFORCE_LOCK
            if ( data->codeBase->lockEnforce && data->recNum > 0L )
               if ( d4lockTest( data, data->recNum ) != 1 )
               {
                  error4( data->codeBase, e4lock, E93311 ) ;
                  return ;
               }
         #endif
      *data->record = ' ' ;
      data->recordChanged = 1 ;
   }

   return ;
}
#endif

/* returns true if the input count is <= record count */
int d4recCountLessEq( DATA4 *data, long count )
{
   long count2 ;

   count2 = data->count ;
   if ( count2 > data->dataFile->minCount ) /* transaction rollback possibility */
      count2 = data->dataFile->minCount ;
   if ( count <= count2 )
      return 1 ;
   data->count = d4recCount( data ) ;
   if ( data->count < 0 )
      return error4( data->codeBase, (short int)data->count, E93318 ) ;
   if ( count <= data->count )
      return 1 ;
   return 0 ;
}

#ifndef S4OFF_MULTI
/* note that S4OFF_MULTI version just calls dfile4recCount since checking of
   locks and serverId are not required */

long S4FUNCTION d4recCountDo( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93312 ) )
         return -1L ;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93312 ) ;

   if ( data->dataFile->numRecs >= 0L )
   {
      #ifndef S4OFF_MULTI
         if ( d4lockTestAppend( data ) != 1 )
            return data->dataFile->minCount ;
         else
      #endif
         return data->dataFile->numRecs ;
   }

   return dfile4recCount( data->dataFile, data4serverId( data ) ) ;
}
#endif

long S4FUNCTION d4recNo( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E93313 ) )
         return -1L;
   #endif  /* S4VBASIC */

   E4PARHIGH( data, E93313 ) ;

   return data->recNum ;
}

char *S4FUNCTION d4record( DATA4 *data )
{
   E4PARHIGH( data, E93314 ) ;

   return data->record ;
}

char *S4FUNCTION d4recordOld( DATA4 *data )
{
   E4PARM_HRET( data, E93314, NULL ) ;


   return data->recordOld ;
}

unsigned int S4FUNCTION d4recWidth( DATA4 *data )
{
   E4PARM_HRET( data, E93314, 0 ) ;

   return dfile4recWidth( data->dataFile ) ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int d4verify( DATA4 *d4, const int subs )
{
   #ifndef S4OFF_WRITE
      #ifdef E4ANALYZE
         int rc ;
      #endif
   #endif

   if ( d4 == 0 )
      return error4( d4->codeBase, e4parmNull, E93317 ) ;

   if ( d4->trans == 0 )
      return error4( 0, e4struct, E93317 ) ;

   #ifndef S4OFF_WRITE
      #ifdef E4ANALYZE
         if ( subs == 1 )
            if ( ( rc = tran4verify( d4->trans, 1 ) ) < 0 )
               return rc ;
      #endif
   #endif

   if ( d4->link.n == 0 || d4->link.p == 0 )
      return error4( d4->codeBase, e4struct, E93317 ) ;

      if ( d4->dataFile == 0 )
         return error4( d4->codeBase, e4struct, E93317 ) ;

      #ifdef E4ANALYZE
         if ( subs == 1 )
            return dfile4verify( d4->dataFile, 0 ) ;
      #endif

   return 0 ;
}

#ifdef S4VB_DOS

char * d4alias_v( DATA4 *d4 )
{
   return v4str( d4alias(d4) ) ;
}

void d4aliasSet( DATA4 *d4, const char *alias )
{
   d4aliasSet( d4, c4str(alias) ) ;
}

#endif
