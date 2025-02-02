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

/* c4trans.c   */

#include "d4all.h"


/* TRANSACTION FILE FORMAT
   -----------------------

   Every entry in the transaction file has the following format:

   [unsigned short int] - transaction entry length (including this length entry)
   [transaction data]
   [transaction header]

   The transaction file can be read either forwards or backwards in this
   fashion.  It is usually read backwards when performing rollbacks, but
   it is useful to read forwards for database history review purposes or
   in case the transaction file itself is corrupt at some point.
*/

#ifndef S4OFF_TRAN
#ifndef S4OFF_WRITE

static void tran4lowCloseDelayed( TRAN4 *trans )
{
   DATA4 *data ;

   for ( ;; )
   {
      data = (DATA4 *)l4pop( &trans->closedDataFiles ) ;
      if ( data == 0 )
         break ;
      l4add( tran4dataList( data->trans ), data ) ;
      d4close( data ) ;
   }
}

static DATA4 *tran4dataFull( TRAN4 *, const long, const long ) ;

#ifdef E4ANALYZE
static int code4transVerify( CODE4TRANS *, int ) ;
#endif  /* E4ANALYZE */

static void code4invalidate( CODE4 * ) ;

#ifdef E4ANALYZE
static int tran4fileVerify( TRAN4FILE *t4, int subs )
{
   int rc ;

   if ( t4 == 0 )
      return error4( 0, e4parmNull, E93801 ) ;

   if ( t4->c4trans == 0 )
      return error4( 0, e4parm, E93801 ) ;

   if ( subs == 1 )
      if ( ( rc = code4transVerify( t4->c4trans, 1 ) ) < 0 )
         return rc ;

   return 0 ;
}
#endif  /* E4ANALYZE */

static int tran4fileInit( TRAN4FILE *t4, CODE4TRANS *c4trans )
{
   t4->c4trans = c4trans ;
   memset( &t4->file, 0, sizeof( FILE4 ) ) ;
   t4->file.hand = INVALID4HANDLE ;
   t4->transId = -1 ;

   #ifdef E4ANALYZE
      return tran4fileVerify( t4, 0 ) ;
   #else
      return 0 ;
   #endif
}

int S4FUNCTION tran4fileAppend( TRAN4FILE *t4, LOG4HEADER *header, void *dta, int doImmediateFlushing )
{
   FILE4LONG filePos ;
   int rc ;
   TRAN4ENTRY_LEN len ;

   #ifdef E4PARM_LOW
      if ( header == 0 )
         return error4( t4->c4trans->c4, e4parmNull, E93801 ) ;
   #endif

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;

      if ( t4->file.hand == INVALID4HANDLE )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif

   filePos = file4lenLow( &t4->file ) ;
   if ( file4longError( filePos ) < 0 )
      return (int)(file4longError( filePos ) ) ;

   len = tran4entryLen( header ) ;
   rc = file4writeInternal( &t4->file, filePos, &len, sizeof( TRAN4ENTRY_LEN ) ) ;
   if ( rc < 0 )
      return rc ;
   file4longAdd( &filePos, sizeof( len ) ) ;

   if ( header->dataLen != 0 )
   {
      #ifdef E4PARM_LOW
         if ( dta == 0 )
            return error4( t4->c4trans->c4, e4parmNull, E93801 ) ;
      #endif
      rc = file4writeInternal( &t4->file, filePos, dta, (unsigned int)header->dataLen ) ;
      if ( rc < 0 )
         return rc ;
      file4longAdd( &filePos, header->dataLen ) ;
   }

   rc = file4writeInternal( &t4->file, filePos, header, sizeof( LOG4HEADER ) ) ;
   if ( rc < 0 )
      return rc ;

   #ifdef S4OFF_OPTIMIZE
      if ( doImmediateFlushing )
         file4flush( &t4->file ) ;
   #else
      t4->needsFlushing = 1 ;
      if ( doImmediateFlushing )
         return tran4fileFlush( t4 ) ;
   #endif

   return 0 ;
}

int S4FUNCTION tran4lowAppend( TRAN4 *t4, void *dta, int doImmediateFlushing )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   if ( t4->dataPos != 0 )
   {
      dta = t4->c4trans->c4->tranData ;
      t4->dataPos = 0 ;
   }

   t4->pos = (unsigned long)-1 ;
   rc = tran4fileAppend( t4->c4trans->transFile, &t4->header, dta, doImmediateFlushing ) ;

   memset( &t4->header, 0, sizeof( LOG4HEADER ) ) ;
   return rc ;
}

static int tran4fileClose( TRAN4FILE *t4 )
{
   int rc, saveRc, saveErr ;

   if ( t4->file.hand == INVALID4HANDLE )
      return 0 ;

   saveRc = 0 ;


   saveErr = error4code( t4->c4trans->c4 ) ;
   error4set( t4->c4trans->c4, 0 ) ;

   rc = file4close( &t4->file ) ;
   if ( rc < 0 && saveRc == 0 )
      saveRc = rc ;

   error4set( t4->c4trans->c4 , saveErr ) ;

   return saveRc ;
}

int tran4fileCreate( TRAN4FILE *t4, const char *name )
{
   char buf[258] ;
   int rc ;
   LOG4HEADER header ;
   CODE4 *c4 ;
   #ifndef S4OFF_MULTI
      int oldExcl ;
   #endif

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   c4 = t4->c4trans->c4 ;

   #ifdef E4PARM_LOW
      if ( name == 0 )
         return error4( c4, e4parmNull, E93801 ) ;
   #endif

   memcpy( buf, name, strlen( name ) + 1 ) ;
   u4nameExt( buf, sizeof( buf ), "log", 0 ) ;
   #ifndef S4OFF_MULTI
      oldExcl = c4->accessMode ;
      c4->accessMode = OPEN4DENY_NONE ;
   #endif
   rc = file4create( &t4->file, c4, buf, 1 ) ;
   #ifndef S4OFF_MULTI
      c4->accessMode = oldExcl ;
   #endif
   if ( rc < 0 )
      return rc ;
   t4->validState = 1;
   memset( &header, 0, sizeof( header ) ) ;
   header.type = TRAN4SHUTDOWN ;
   header.serverDataId = TRAN4VERSION_NUM ;
   rc = tran4fileAppend( t4, &header, "\0", 1 ) ;
   if (rc < 0)
      return(rc);
   #ifdef E4ANALYZE
      return tran4fileVerify( t4, 1 ) ;
   #else
      return 0 ;
   #endif
}

#ifndef S4UTILS
static int tran4fileStatusFile( TRAN4FILE *t4 )
{
   CODE4 *c4 ;
   FILE4LONG filePos, fPos2 ;
   int rc ;
   LOG4HEADER header ;
   TRAN4ENTRY_LEN entryLen;
      #ifndef S4OFF_MULTI
         long loop ;
         int oldNumAttempts ;
      #endif

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->file.hand == INVALID4HANDLE )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif

   c4 = t4->c4trans->c4 ;

      /* first see if the only user, in which case can verify */
      #ifndef S4OFF_MULTI
         oldNumAttempts = c4->lockAttempts ;
             /* can't do overlay locking due to Novell inconsistencies... */
             /* reserve a byte so only one attempt to set up one at a time */
         c4->lockAttempts = -1 ;
         rc = file4lockInternal( &t4->file, TRAN4LOCK_USERS + TRAN4MAX_USERS + 1, 0, 1, 0 ) ;
         c4->lockAttempts = 1 ;
         for ( loop = 0 ; loop <= TRAN4MAX_USERS ; loop++ )
         {
            rc = file4lockInternal( &t4->file, TRAN4LOCK_USERS + loop, 0, 1, 0 ) ;
            if ( rc < 0 || rc == r4locked )
               break ;
            file4unlockInternal( &t4->file, TRAN4LOCK_USERS + loop, 0, 1, 0 ) ;
         }
         c4->lockAttempts = oldNumAttempts ;
         if ( rc  )
         {
            file4unlockInternal( &t4->file, TRAN4LOCK_USERS + TRAN4MAX_USERS + 1, 0, 1, 0 ) ;
            if ( rc == r4locked )
               return 0 ;
            return rc ;
         }
      #endif

   for( ;; )
   {
      filePos = file4lenLow( &t4->file ) ;
      file4longSubtract( &filePos, sizeof( LOG4HEADER ) + sizeof(TRAN4ENTRY_LEN) ) ;
      if ( file4longGetLo( filePos ) < sizeof( LOG4HEADER ) + sizeof(TRAN4ENTRY_LEN) )
      {
         rc = error4( c4, e4trans, E83801 ) ;
         break ;
      }

      file4longAssign( fPos2, 0, 0 ) ;

      rc = file4readAllInternal( &t4->file, fPos2, &entryLen, sizeof( TRAN4ENTRY_LEN ) ) ;
      if ( rc < 0 )
         break ;

      if (entryLen != sizeof(LOG4HEADER) + sizeof(TRAN4ENTRY_LEN))
      {
         rc = error4( c4, e4read, E93801 ) ;
         break ;
      }

      file4longAssign( fPos2, sizeof(TRAN4ENTRY_LEN), 0 ) ;

      rc = file4readAllInternal( &t4->file, fPos2, &header, sizeof( LOG4HEADER ) ) ;
      if ( rc < 0 )
         break ;

      if ( header.type == TRAN4BACKEDUP )
      {
         rc = error4( c4, e4trans, E83815 ) ;
         break ;
      }

      rc = file4readAllInternal( &t4->file, filePos, &entryLen, sizeof( TRAN4ENTRY_LEN ) ) ;
      if ( rc < 0 )
         break ;

      if (entryLen != sizeof(LOG4HEADER) + sizeof(TRAN4ENTRY_LEN))
      {
         rc = error4( c4, e4trans, E83801 ) ;
         break ;
      }

      file4longAdd( &filePos, sizeof(TRAN4ENTRY_LEN) ) ;

      rc = file4readAllInternal( &t4->file, filePos, &header, sizeof( LOG4HEADER ) ) ;
      if ( rc < 0 )
         break ;

      break ;
   }

      file4unlockInternal( &t4->file, TRAN4LOCK_USERS + TRAN4MAX_USERS + 1, 0, 1, 0 ) ;

   if ( rc < 0 )
      return rc ;

   if ( header.type == TRAN4SHUTDOWN )
      return 0 ;

   return error4( c4, e4trans, E83801 ) ;
}
#endif /* S4UTILS */

static int tran4fileOpen( TRAN4FILE *t4, char *name )
{
   int rc, oldOpenError, oldReadOnly ;
   char buf[258] ;
   CODE4 *c4 ;
   #ifndef S4OFF_MULTI
      int oldExcl ;
   #endif

   #ifdef E4PARM_LOW
      if ( name == 0 )
         return error4( t4->c4trans->c4, e4parmNull, E93801 ) ;
   #endif

   c4 = t4->c4trans->c4 ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->file.hand != INVALID4HANDLE )   /* already open */
         return error4( c4, e4struct, E93801 ) ;
   #endif

   memcpy( buf, name, strlen( name ) + 1 ) ;
   u4nameExt( buf, sizeof( buf ), "log", 0 ) ;

   #ifndef S4OFF_MULTI
      oldExcl = c4->accessMode ;
      #ifdef S4UTILS
         c4->accessMode = OPEN4DENY_RW ;
      #else
         c4->accessMode = OPEN4DENY_NONE ;
      #endif
   #endif
   oldReadOnly = c4getReadOnly( c4 ) ;
   c4setReadOnly( c4, 0 ) ;
   oldOpenError = c4->errOpen ;
   c4->errOpen = 0 ;

   rc = file4open( &t4->file, c4, buf, 1 ) ;

   c4->errOpen = oldOpenError ;
   c4setReadOnly( c4, oldReadOnly ) ;
   #ifndef S4OFF_MULTI
      c4->accessMode = oldExcl ;
   #endif
   #ifndef S4UTILS
      if ( rc != r4noOpen && rc >= 0 )
      {
         if ( tran4fileStatusFile( t4 ) )
         {
            t4->validState = 0 ;
            file4close( &t4->file ) ;
            return -1 ;
         }
         else
            t4->validState = 1 ;
      }
   #endif /* S4UTILS */

   if ( rc != 0 )
      return rc ;

   #ifdef E4ANALYZE
      return tran4fileVerify( t4, 1 ) ;
   #else
      return 0 ;
   #endif
}

/* server just increments and checks trans file for last entry */
/* stand-alone uses the userIdNo as an offset to avoid requiring
   continual reads to get next id no.  */
static long tran4fileGetNextTransId( TRAN4FILE *t4, TRAN4 *trans )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

      if ( t4->transId > 0 )
      {
         if ( t4->transId == LONG_MAX )
            t4->transId = t4->c4trans->trans.userIdNo ;
         t4->transId += TRAN4MAX_USERS ;
      }
      else
      {
         if ( tran4bottom( trans ) < 0 )  /* no entries */
            return 0 ;

         for ( ;; )
         {
            if ( trans->header.transId % TRAN4MAX_USERS == trans->userIdNo )
               break ;
            if ( tran4skip( trans, TRAN4BACKWARDS ) != 0 )
               break ;
         }

         if ( trans->header.transId < 0 )
            return e4trans ;

         #if ( __WORDSIZE == 64 )
            if ( trans->header.transId == INT_MAX  || ( trans->header.transId % TRAN4MAX_USERS != t4->c4trans->trans.userIdNo ) )
         #else
            if ( trans->header.transId == LONG_MAX || ( trans->header.transId % TRAN4MAX_USERS != t4->c4trans->trans.userIdNo ) )
         #endif

            t4->transId = t4->c4trans->trans.userIdNo ;
         else
            t4->transId = trans->header.transId + TRAN4MAX_USERS ;
      }
   return t4->transId ;
}

static long tran4getTransId( TRAN4 *t4 )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   if ( t4->transId > 0 )
      return t4->transId ;

   #ifdef E4ANALYZE
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif

   t4->transId = tran4fileGetNextTransId( t4->c4trans->transFile, t4 ) ;
   return t4->transId ;
}

static int tran4fileRead( TRAN4FILE *t4, unsigned long posIn, LOG4HEADER *header, char **data, unsigned int *dataLen )
{
   int rc ;
   FILE4LONG pos ;

   file4longAssign( pos, posIn, 0 ) ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->file.hand == INVALID4HANDLE )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif

   rc = file4readAllInternal( &t4->file, pos, header, sizeof( LOG4HEADER ) ) ;
   if ( rc < 0 )
      return rc ;
   if ( *dataLen < header->dataLen + 1 )
      if ( u4allocAgain( t4->c4trans->c4, data, dataLen, (unsigned int)header->dataLen + 1 ) != 0 )
         return e4memory ;
   if ( header->dataLen > 0L )
   {
      file4longSubtract( &pos, header->dataLen ) ;
      rc = file4readAllInternal( &t4->file, pos, *data, (unsigned int)header->dataLen ) ;
      if ( rc < 0 )
         return rc ;
   }
   else
      (*data)[0] = '\0' ;
   return 0 ;
}

static int tran4read( TRAN4 *t4 )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->pos == (unsigned long)-1 )
         return error4( t4->c4trans->c4, e4info, E83802 ) ;
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4info, E93801 ) ;
   #endif
   return tran4fileRead( t4->c4trans->transFile, t4->pos, &t4->header, &t4->c4trans->c4->tranData, &t4->c4trans->c4->tranDataLen ) ;
}

int S4FUNCTION tran4fileTop( TRAN4FILE *t4, TRAN4 *trans )
{
   int rc ;
   unsigned lenRead ;
   TRAN4ENTRY_LEN len ;
   FILE4LONG pos ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   file4longAssign( pos, 0, 0 ) ;

   lenRead = file4readInternal( &t4->file, pos, &len, sizeof( len ) ) ;
   if ( lenRead != sizeof( len ) )
      return r4bof ;

   trans->pos = len - sizeof( LOG4HEADER ) ;

   rc = tran4read( trans ) ;
   if ( rc < 0 )
   {
      trans->pos = (unsigned long)-1 ;
      return rc ;
   }
   return 0 ;
}

int S4FUNCTION tran4fileBottom( TRAN4FILE *t4, TRAN4 *trans )
{
   int rc ;
   FILE4LONG pos ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   pos = file4lenLow( &t4->file ) ;

   #ifdef E4ANALYZE
      if ( file4longGetHi( pos ) > 0 )   /* < 0, error is ok */
         return error4( 0, e4result, E93801 ) ;
   #endif

   trans->pos = file4longGetLo( pos ) - sizeof( LOG4HEADER ) ;
   if ( trans->pos == (unsigned long)-1 )
      return r4eof ;
   rc = tran4read( trans ) ;
   if ( rc < 0 )
   {
      trans->pos = (unsigned long)-1 ;
      return rc ;
   }
   return 0 ;
}

int S4FUNCTION tran4fileSkip( TRAN4FILE *t4, TRAN4 *trans, const int direction )
{
   int rc ;
   TRAN4ENTRY_LEN len ;
   FILE4LONG tempLen ;

   #ifdef E4PARM_LOW
      if ( t4 == 0 || trans == 0 || ( direction != TRAN4FORWARDS && direction != TRAN4BACKWARDS ) )
         return error4( 0, e4parm, E93801 ) ;
   #endif

   if ( trans->pos == (unsigned long)-1 )   /* empty file ... */
   {
      if ( direction == TRAN4BACKWARDS )
         return r4eof ;
      else
         return r4bof ;
   }

   #ifdef E4ANALYZE
      if ( ( rc = tran4fileVerify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   if ( direction == TRAN4BACKWARDS )
   {
      if ( trans->pos < tran4entryLen( &trans->header ) )
         return r4bof ;
      trans->pos -= tran4entryLen( &trans->header ) ;
      rc = tran4read( trans ) ;
      if ( rc< 0 )
      {
         trans->pos = (unsigned long)-1 ;
         return rc ;
      }
   }
   else
   {
      trans->pos += sizeof( LOG4HEADER ) ;
      tempLen = file4lenLow( &t4->file ) ;
      if ( file4longLessEq( tempLen, trans->pos + sizeof( TRAN4ENTRY_LEN ) ) )
         return r4eof ;
      file4longAssign( tempLen, trans->pos, 0 ) ;
      rc = file4readAllInternal( &t4->file, tempLen, &len, sizeof( TRAN4ENTRY_LEN ) ) ;
      if ( rc < 0 )
         return rc ;

      trans->pos += len - sizeof( LOG4HEADER ) ;

      if ( trans->pos >= file4longGetLo( file4lenLow( &t4->file ) ) )
         return r4eof ;

      rc = tran4read( trans ) ;
      if ( rc< 0 )
      {
         trans->pos = (unsigned long)-1 ;
         return rc ;
      }
   }

   return 0 ;
}

#ifndef S4INLINE
long S4FUNCTION tran4id( TRAN4 *t4 )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.transId ;
}

long S4FUNCTION tran4clientId( TRAN4 *t4 )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.clientId ;
}

unsigned long S4FUNCTION tran4clientDataId( TRAN4 *t4 )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.clientDataId ;
}

unsigned long S4FUNCTION tran4serverDataId( TRAN4 *t4 )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.serverDataId ;
}

int S4FUNCTION tran4type( TRAN4 *t4 )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.type ;
}

unsigned S4FUNCTION tran4len( TRAN4 *t4 )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   return t4->header.dataLen ;
}
#endif  /* S4INLINE */

/* data may be larger than can be contained in memory... new coding reqd */
void *S4FUNCTION tran4getData( TRAN4 *t4, const long pos )
{
   CODE4 *c4 ;

   #ifdef E4ANALYZE
      if ( tran4verify( t4, 1 ) < 0 )
         return 0 ;
      if ( t4->c4trans->transFile == 0 )
      {
         error4( t4->c4trans->c4, e4struct, E93801 ) ;
         return 0 ;
      }
   #endif

   c4 = t4->c4trans->c4 ;

   if ( c4->tranData != 0 )
   {
      if ( c4->tranDataLen < t4->header.dataLen + 1 )
      {
         error4( c4, e4trans, E93801 ) ;
         return 0 ;
      }
      c4->tranData[ t4->header.dataLen ] = 0 ;
   }
   return c4->tranData + pos ;
}

int S4FUNCTION tran4set( TRAN4 *t4
                       , const int status
                       , const long id1, const long id2
                       , const int typ,  const unsigned int dLen
                       , const long clientId, const long serverId )
{
   DATA4 *data ;
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
   #endif

   if ( status == r4active || status == r4partial || typ == TRAN4START || typ == TRAN4ROLLBACK )
   {
      if ( id1 == -1 )
         t4->header.transId = tran4getTransId( t4 ) ;
      else
         t4->header.transId = id1 ;
   }
   else
      t4->header.transId = 0 ;

   #ifdef E4ANALYZE
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif

   if ( t4->header.transId < 0 )
      return error4( t4->c4trans->c4, e4trans, E93801 ) ;

      /* AS 5/14/98 always just use the userIdNo for s/a */
   t4->header.clientId = t4->c4trans->trans.userIdNo ;
   t4->header.type = typ ;
   t4->header.dataLen = dLen ;
   t4->header.clientDataId = clientId ;
   t4->header.serverDataId = serverId ;

   time( &t4->header.time ) ;
   #ifdef __WIN32                /* CJ - This change was needed as 32-bit and 16-bit apps report different values from time().*/
      t4->header.time += 3600;   /* Upon such a time that the CodeUtil is change to 32-bit app the ifdef should be S4WIN16 and */
   #endif                        /* 3600 should be subtracted. If CodeBase nolonger supports 16-bit apps this section is unnecessary. MICROSOFT BUG */

   if ( serverId != 0 && clientId != 0 )  /* ensure a valid data4 before bothering to call (i.e. maybe a non-data-related transaction item) */
   {
      data = tran4dataFull( t4, serverId, clientId ) ;
      if ( data != 0 )   /* probably not a data-related operation (eg. tranAddUser, etc) */
         data->transChanged = 1 ;
   }

   return 0 ;
}

int S4FUNCTION tran4putData( TRAN4 *t4, void *dta, unsigned dLen )
{
   CODE4 *c4 ;

   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->c4trans->transFile == 0 || dLen <= 0 )
         return error4( t4->c4trans->c4, e4info, E93801 ) ;
   #endif

   c4 = t4->c4trans->c4 ;

   if ( t4->dataPos + dLen + 1 > c4->tranDataLen )
      if ( u4allocAgain( c4, &c4->tranData, &c4->tranDataLen, dLen + t4->dataPos + 1 ) != 0 )
         return e4memory ;

   memcpy( c4->tranData + t4->dataPos, dta, dLen ) ;
   t4->dataPos += dLen ;
   return 0 ;
}

#ifndef S4INLINE
int S4FUNCTION tran4bottom( TRAN4 *t4 )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif
   return tran4fileBottom( t4->c4trans->transFile, t4 ) ;
}

int S4FUNCTION tran4top( TRAN4 *t4 )
{
   #ifdef E4ANALYZE
      int rc ;

      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4info, E93801 ) ;
   #endif
   return tran4fileTop( t4->c4trans->transFile, t4 ) ;
}

int S4FUNCTION tran4skip( TRAN4 *t4, int direction )
{
   #ifdef E4ANALYZE
      int rc ;
   #endif

   #ifdef E4PARM_LOW
      if ( direction != TRAN4FORWARDS && direction != TRAN4BACKWARDS )
         return error4( 0, e4parm, E93801 ) ;
   #endif

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( t4, 1 ) ) < 0 )
         return rc ;
      if ( t4->c4trans->transFile == 0 )
         return error4( t4->c4trans->c4, e4struct, E93801 ) ;
   #endif
   return tran4fileSkip( t4->c4trans->transFile, t4, direction ) ;
}
#endif  /* S4INLINE */

int tran4lowUnappend( TRAN4 *trans )
{
   DATA4 *data ;
   int rc ;
   S4LONG recNo ;

  E4PARMLOW( trans, E93801 ) ;

   recNo = *((S4LONG *)(tran4getData( trans, 0L ) ) ) ;

   data = tran4dataFull( trans, tran4serverDataId( trans ), tran4clientDataId( trans ) ) ;
   if ( data == 0 )
      return error4( trans->c4trans->c4, e4name, E93801 ) ;

   #ifndef S4OFF_MULTI
      if ( dfile4lockTestAppend( data->dataFile, tran4clientDataId( trans ), tran4serverDataId( trans ) ) == 0 )
         return error4( trans->c4trans->c4, e4lock, E83804 ) ;
   #endif
   if ( d4recCount( data ) != recNo )
   {
      if ( d4recCount( data ) == recNo - 1 )   /* already unappended */
         return 0 ;
      else
         return error4( trans->c4trans->c4, e4rollback, E83805 ) ;
   }

   memcpy( data->record, tran4getData( trans, sizeof(S4LONG ) ), dfile4recWidth( data->dataFile ) ) ;
   rc = d4unappend( data ) ;
   if ( rc < 0 )
      return error4stack( trans->c4trans->c4, rc, E93801 ) ;

   return d4update( data ) ;
}

int tran4lowUnwrite( TRAN4 *trans )
{
   DATA4 *data ;
   int rc, doSpecial ;
   S4LONG recNo ;
   char *rec, *saveRec = 0;
   CODE4 *c4 ;
   #ifndef S4MEMO_OFF
      int i ;
      unsigned S4LONG ptrLen, pos ;
   #endif

   E4PARMLOW( trans, E93801 ) ;

   c4 = trans->c4trans->c4 ;
   recNo = *((S4LONG *)(tran4getData( trans, 0L ) ) ) ;
   rec = (char *)tran4getData( trans, (long)sizeof(S4LONG ) ) ;

   data = tran4dataFull( trans, tran4serverDataId( trans ), tran4clientDataId( trans ) ) ;
   if ( data == 0 )
      return error4( c4, e4name, E93801 ) ;

   doSpecial = 0 ;
   if ( data->recordChanged == 1 )   /* override flush */
   {
      if ( d4recNo( data ) == recNo )
      {
         /* special, case (automatic rollback) -- save record for user */
         doSpecial = 1 ;
         saveRec = data->record ;
         data->record =(char*) u4allocFree( c4, dfile4recWidth( data->dataFile ) + 1 ) ;
         if ( data->record == 0 )
         {
            /* user will lose changes, due to low memory, but can at least rollback */
            data->record = saveRec ;
            doSpecial = 0 ;
         }
         else
            memcpy( data->record, saveRec, dfile4recWidth( data->dataFile ) ) ;
      }
      data->recordChanged = 0 ;
   }
   rc = 0 ;
   if ( doSpecial == 0 )
      rc = d4go( data, recNo ) ;

   if ( rc == 0 )
   #ifndef S4OFF_MULTI
      if ( dfile4lockTest( data->dataFile, tran4clientDataId( trans ), tran4serverDataId( trans ), recNo ) == 0 )
      {
         /* #ifdef OLEDB5BUILD */ /*commented out so that the standalone DLL will incoporate this change */
            /* with ole-db in isolation level CHAOS, records get unlocked, so we need to re-lock them
               in that instance... therefore no error in this case...
               we will implement as follows -- if we cannot get the lock, then the abort will fail
               for an unspecified reason. */
            if ( dfile4lock( data->dataFile, tran4clientDataId( trans ), tran4serverDataId( trans ), recNo ) != 0 )
         /*#endif */
            rc = error4( c4, e4lock, E83804 ) ;
      }
   #endif

   if ( rc == 0 )
   {
      memcpy( d4record( data ), rec, (size_t)dfile4recWidth( data->dataFile ) ) ;
      #ifndef S4MEMO_OFF
         pos = (long)sizeof(S4LONG ) + 2L * dfile4recWidth( data->dataFile ) ;
         for ( i = 0; i < data->dataFile->nFieldsMemo; i++ )
         {
            ptrLen = *( (unsigned int *)tran4getData( trans, pos ) ) ;
            pos += sizeof( ptrLen ) ;
            if ( ptrLen != 0 )
               f4memoAssignN( data->fieldsMemo[i].field, (char *)tran4getData( trans, pos ), (unsigned int)ptrLen ) ;
            pos += ptrLen ;
            #ifdef S4DATA_ALIGN
               memcpy(&ptrLen, tran4getData(trans, pos), sizeof(ptrLen));
            #else
               ptrLen = *( (unsigned S4LONG *)tran4getData( trans, pos ) ) ;
            #endif
            pos += sizeof( ptrLen ) + ptrLen ;
         }
      #endif
   }

   if ( rc == 0 )
      rc = d4writeLow( data, recNo, 0 ) ;

   if ( rc == 0 )
      rc = d4update( data ) ;

   if ( doSpecial )
   {
      u4free( data->record ) ;
      data->record = saveRec ;
      data->recordChanged = 1 ;
   }

   return rc ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
/* clientId is a unique id for the transactee.  In the single user case, there is only one */
int S4FUNCTION tran4lowStart( TRAN4 *trans, long clientId, int doUnlock )
{
   int rc ;

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_MULTI
         int oldLockAttempts ;
      #endif
   #endif

   CODE4 *c4 ;

   E4PARHIGH( trans, E93801 ) ;

   c4 = trans->c4trans->c4 ;

   if ( trans->c4trans->enabled == 0 )
   {
         if ( c4->logOpen == 0 )
            return error4( c4, e4trans, E83814 ) ;
         rc = code4logOpen( c4, 0, 0 ) ;
      if ( rc < 0 )
         return rc ;
   }

   if ( trans->c4trans->enabled != 1 )
      return error4( c4, e4trans, E83807 ) ;

      if ( trans->currentTranStatus == r4active )   /* already in a transactional state */
         return error4( c4, e4trans, E93801 ) ;

   if ( trans->c4trans->transFile->status != tran4notRollbackOrCommit )
      return error4( c4, e4trans, E83801 ) ;

   #ifndef S4OFF_MULTI
      if ( doUnlock == 1 )
      {
         rc = code4unlock( c4 ) ;
         if ( rc < 0 )
            return rc ;
      }
   #endif

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_MULTI
         oldLockAttempts = c4->lockAttempts ;
         c4->lockAttempts = WAIT4EVER ;
         rc = code4tranLockTransactions( &c4->c4trans, TRAN4LOCK_MULTIPLE ) ;
         c4->lockAttempts = oldLockAttempts ;
         if ( rc < 0 )
            rc = error4( c4, rc, E93801 ) ;
      #endif
      tran4bottom( trans ) ;
   #endif

   rc = tran4set( trans, trans->currentTranStatus, -1L, clientId, TRAN4START, 0, 0L, 0L ) ;
   if ( rc < 0 )
      return rc ;
   if ( tran4lowAppend( trans, "\0", 0 ) != 0 )
      return e4transAppend ;

   trans->currentTranStatus = r4active ;

   #ifndef S4OFF_MULTI
      trans->savedUnlockAuto = trans->unlockAuto ;
      trans->unlockAuto = 0 ;
   #endif

   return 0 ;
}

/*int S4FUNCTION tran4lowRollback( TRAN4 *trans, long id, int doUnlock )*/
int S4FUNCTION tran4lowRollback( TRAN4 *trans, long id, const int doInvalidate )
{
   int done, saveErr, rc ;
   long transId ;
   CODE4 *c4 ;

   E4PARHIGH( trans, E93801 ) ;

   c4 = trans->c4trans->c4 ;
   saveErr = error4set( c4, 0 ) ;
   transId = tran4getTransId( trans ) ;

   #ifdef E4ANALYZE
      if ( transId <= 0 )
         return error4( 0, e4info, E93801 ) ;
   #endif

   #ifdef E4ANALYZE
      if ( trans->c4trans->enabled != 1 )
         return error4( c4, e4rollback, E83807 ) ;
   #endif

   if ( trans->currentTranStatus != r4active )
      return error4( c4, e4transStatus, E83808 ) ;

   if ( trans->c4trans->transFile->status != tran4notRollbackOrCommit )
      return error4( c4, e4trans, E83801 ) ;
   trans->c4trans->transFile->status = tran4rollbackOrCommit ;

   trans->currentTranStatus = r4rollback ;

   /* first rollback all the transactions, then mark a rollback as having occurred */
   rc = tran4bottom( trans ) ;
   if ( rc != 0 )
      return error4stack( c4, rc, E93801 ) ;

   for( done = 0, rc = 0; !done && !rc ; )
   {
      if ( tran4id( trans ) == transId )
      {
         switch( tran4type( trans ) )
         {
            case TRAN4START:
               done = 1 ;
               break ;
            case TRAN4WRITE:
               rc = tran4lowUnwrite( trans ) ;
               break ;
            case TRAN4APPEND:
               rc = tran4lowUnappend( trans ) ;
               break ;
            case TRAN4VOID:   /* transaction connectioning to next was voided, so skip... */
            case TRAN4OPEN:
            case TRAN4OPEN_TEMP:
            case TRAN4CLOSE:
               break ;
            default:
               rc = error4( c4, e4rollback, E83809 ) ;
         }
      }
      if ( !done && !rc )
         rc = tran4skip( trans, TRAN4BACKWARDS ) ;
   }
   if ( rc > 0 )
      rc = 0 ;

   if ( rc == 0 )
   {
      tran4set( trans, trans->currentTranStatus, transId, id, TRAN4ROLLBACK, 0, 0L, 0L ) ;
      if ( tran4lowAppend( trans, "\0", 1 ) != 0 )
         return e4transAppend ;
   }

   trans->currentTranStatus = r4inactive ;
   trans->transId = 0;

   #ifndef S4OFF_MULTI
      trans->unlockAuto = trans->savedUnlockAuto ;
      #ifndef S4OFF_TRAN
         #ifndef S4UTILS
            rc = code4tranUnlockTransactions( &c4->c4trans, TRAN4LOCK_MULTIPLE ) ;
         #endif
      #endif
   #endif

   if ( saveErr != 0 )
      error4set( c4, saveErr ) ;

   if ( rc == 0 )
      trans->c4trans->transFile->status = tran4notRollbackOrCommit ;
   tran4lowCloseDelayed( trans ) ;
      if ( doInvalidate )
         code4invalidate( c4 ) ;
   return rc ;
}

static int tran4lowRemoveKeys( TRAN4 *trans )
{
   DATA4 *data ;
   #ifndef S4OFF_INDEX
      int rc, saveRc ;
      TAG4 *tag ;
      TAG4KEY_REMOVED *removed ;

      saveRc = 0 ;
   #endif

   for ( data = 0 ;; )  /* first do open files */
   {
      data =(DATA4 *) l4next( tran4dataList( trans ), data ) ;
      if ( data == 0 )
         break ;
      #ifndef S4OFF_INDEX
         for ( tag = 0 ;; )
         {
            tag = d4tagNext( data, tag ) ;
            if ( tag == 0 )
               break ;
            for ( removed = 0 ;; )
            {
               removed = (TAG4KEY_REMOVED *)l4first( &tag->removedKeys ) ;
               if ( removed == 0 )
                  break ;
               rc = tfile4remove( tag->tagFile, removed->key, removed->recno ) ;
               if ( rc < 0 )
                  saveRc = rc ;
               l4remove( &tag->removedKeys, removed ) ;
               u4free( removed ) ;
            }
         }
      #endif
   }
   for ( data = 0 ;; )  /* now do closed files */
   {
      data =(DATA4 *) l4next( &trans->closedDataFiles, data ) ;
      if ( data == 0 )
         break ;
      #ifndef S4OFF_INDEX
         for ( tag = 0 ;; )
         {
            tag = d4tagNext( data, tag ) ;
            if ( tag == 0 )
               break ;
            for ( removed = 0 ;; )
            {
               removed = (TAG4KEY_REMOVED *)l4first( &tag->removedKeys ) ;
               if ( removed == 0 )
                  break ;
               rc = tfile4remove( tag->tagFile, removed->key, removed->recno ) ;
               if ( rc < 0 )
                  saveRc = rc ;
               l4remove( &tag->removedKeys, removed ) ;
               u4free( removed ) ;
            }
         }
      #endif
   }
   #ifndef S4OFF_INDEX
      return saveRc ;
   #else
      return 0 ;
   #endif
}

static int tran4updateData( TRAN4 *trans )
{
   DATA4 *data ;
   int rc, saveRc, oldTransStatus ;

   /* changes trans status to force an update */
   oldTransStatus = trans->currentTranStatus ;
   trans->currentTranStatus = r4off ;
   for ( data = 0, saveRc = 0 ;; )
   {
      data = (DATA4 *)l4next( trans->dataList, data ) ;
      if ( data == 0 )
         break ;
      if ( data->dataFile->fileChanged == 1 )
      {
         #ifndef S4OFF_MULTI
            if ( d4lockTestAppend( data ) == 1 )
         #endif
         {
            rc = dfile4updateHeader( data->dataFile, 1, 1 ) ;
            if ( rc < 0 )
               saveRc = rc ;
         }
      }
   }

   trans->currentTranStatus = oldTransStatus ;
   return saveRc ;
}

static int tran4lowFlush( TRAN4 *trans )
{
   DATA4 *dataOn ;
   int rc ;

   for ( dataOn = 0 ;; )
   {
      dataOn = (DATA4 *)l4next( tran4dataList( trans ), dataOn ) ;
      if ( dataOn == 0 )
         break ;
      if ( dataOn->transChanged )  /* if the data file was changed during the transaction... */
      {
         #ifndef S4OFF_MULTI
            if ( d4lockTestAppend( dataOn ) )
         #endif
            dfile4updateHeader( dataOn->dataFile, 1, 1 ) ;
         rc = d4flush( dataOn ) ;
         if ( rc != 0 )
            return rc ;
         dataOn->transChanged = 0 ;
      }
   }

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION tran4lowCommitPhaseOne( TRAN4 *trans, long id )
{
   CODE4 *c4 ;
   int rc ;

   E4PARMLOW( trans, E93801 ) ;

   c4 = trans->c4trans->c4 ;

   if ( trans->c4trans->enabled != 1 )
      return error4( c4, e4parm, E83807 ) ;

   if ( trans->currentTranStatus != r4active )
      return 0 ;

   while ( trans->c4trans->transFile->status != tran4notRollbackOrCommit )  /* means another user is manipulating, wait for them to finish */
   {
      u4delayHundredth( 1 ) ;
      /* return error4( c4, e4trans, E83801 ) ; */
   }

   rc = tran4lowFlush( trans ) ;
   if ( rc != 0 )
      return rc ;

   trans->c4trans->transFile->status = tran4rollbackOrCommit ;

   rc = tran4set( trans, trans->currentTranStatus, -1L, id, TRAN4COMMIT_PHASE_ONE, 0, 0L, 0L ) ;
   if ( rc < 0 )
      return rc ;

   #ifdef S4MULTI_SERVER
      rc = tran4putData( trans, &numServers, sizeof( numServers ) ) ;
      if ( rc < 0 )
         return rc ;
   #endif

   trans->currentTranStatus = r4partial ;

   if ( tran4lowAppend( trans, "\0", 0 ) != 0 )
      return e4transAppend ;

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION tran4lowCommitPhaseTwo( TRAN4 *trans, long id, int doUnlock )
{
   CODE4 *c4 ;
   int rc ;

   E4PARMLOW( trans, E93801 ) ;


   c4 = trans->c4trans->c4 ;

   if ( trans->currentTranStatus != r4partial )
      return error4( trans->c4trans->c4, e4commit, E83811 ) ;

   if ( trans->c4trans->transFile->status != tran4rollbackOrCommit )  /* we should be manipulating */
      return error4( c4, e4trans, E83801 ) ;

   rc = tran4set( trans, trans->currentTranStatus, -1, id, TRAN4COMMIT_PHASE_TWO, 0, 0L, 0L ) ;
   if ( rc < 0 )
      return rc ;
   if ( tran4lowAppend( trans, "\0", 1 ) != 0 )
      return e4transAppend ;

   rc = tran4lowRemoveKeys( trans ) ;
   if ( rc < 0 )
      return error4stack( c4, rc, E93801 ) ;
   trans->currentTranStatus = r4inactive ;
   trans->transId = 0;
   #ifndef S4OFF_MULTI
      trans->unlockAuto = trans->savedUnlockAuto ;
   #endif
   rc = tran4updateData( trans ) ;
   #ifndef S4OFF_MULTI
      if ( rc == 0 && doUnlock )
      {
         if ( code4unlockAuto( trans->c4trans->c4 ) == 1 )
            rc = code4unlock( trans->c4trans->c4 ) ;
      }

      #ifndef S4OFF_TRAN
         #ifndef S4UTILS
               rc = code4tranUnlockTransactions( &c4->c4trans, TRAN4LOCK_MULTIPLE ) ;
         #endif
      #endif
   #endif

   if ( rc == 0 )
      trans->c4trans->transFile->status = tran4notRollbackOrCommit ;

   tran4lowCloseDelayed( trans ) ;

   trans->currentTranStatus = r4inactive ;
   #ifndef S4OFF_MULTI
      trans->unlockAuto = trans->savedUnlockAuto ;
   #endif

   return 0 ;
}



int S4FUNCTION code4tranCommit( CODE4 *c4 )
{
   E4PARHIGH( c4, E93828 ) ;

   if ( code4trans(c4)->currentTranStatus != r4partial )   /* do full commit */
      if ( code4tranCommitPhaseOne( c4 ) != 0 )
         return -1 ;

   return code4tranCommitPhaseTwo( c4, 1 ) ;
}

int S4FUNCTION code4tranCommitSingle( CODE4 *c4 )
{
   E4PARHIGH( c4, E93828 ) ;

   if ( code4tranCommitPhaseOne( c4 ) != 0 )
      return -1 ;

   return code4tranCommitPhaseTwo( c4, 0 ) ;
}

/* places all DATA4's into an invalid state */

static void code4invalidate( CODE4 *c4 )
{
   DATA4 *dataOn ;

   E4PARMLOW( c4, E93827 ) ;

   for ( dataOn = 0 ;; )
   {
      dataOn = (DATA4 *)l4next( c4->c4trans.trans.dataList, dataOn ) ;
      if ( dataOn == 0 )
         break ;
      /* 04/24/96 --> d4blank() replacing memset due to S4FOX binary
         fields having non-blank contents to represent blank */
/*      memset( dataOn->record, ' ', dfile4recWidth( dataOn->dataFile ) ) ; */
      dataOn->recNum = dataOn->recNumOld = -1 ;  /* ensure that d4blank works with lock-enforce on, plus reset record # */
      d4blank( dataOn ) ;
      d4changed( dataOn, 0 ) ;
   }
}


int S4FUNCTION code4tranCommitPhaseOne( CODE4 *c4 )
{
   int saveErr, rc ;

   E4PARHIGH( c4, E93828 ) ;

   saveErr = error4code( c4 ) ;
   if ( saveErr < 0 )
      error4set( c4, 0 ) ;

   rc = tran4lowCommitPhaseOne( &c4->c4trans.trans, 0 ) ;

   #ifndef S4OFF_MULTI
      if ( code4unlockAuto( c4 ) == 1 )
         if ( code4unlock( c4 ) != 0 )
            return error4( c4, e4unlock, E93828 ) ;
   #endif

   if ( saveErr == 0 )
      saveErr = rc ;
   if ( saveErr != 0 )
      error4set( c4, saveErr ) ;

   return rc ;
}


int S4FUNCTION code4tranCommitPhaseTwo( CODE4 *c4, int doUnlock )
{
   int saveErr, rc ;

   E4PARHIGH( c4, E93828 ) ;

   if ( c4->c4trans.trans.currentTranStatus != r4partial )
      return error4( c4, e4transStatus, E83812 ) ;

   saveErr = error4code( c4 ) ;
   if ( saveErr < 0 )
      error4set( c4, 0 ) ;

   rc = tran4lowCommitPhaseTwo( &c4->c4trans.trans, 0, doUnlock ) ;

   if ( saveErr == 0 )
      saveErr = rc ;
   if ( saveErr != 0 )
      error4set( c4, saveErr ) ;

   return rc ;
}


int S4FUNCTION code4tranStart( CODE4 *c4 )
{
   E4PARHIGH( c4, E93829 ) ;

   return tran4lowStart( &c4->c4trans.trans, 0, 0 ) ;
}


int S4FUNCTION code4tranStartSingle( CODE4 *c4 )
{
   E4PARHIGH( c4, E93829 ) ;

   return tran4lowStart( &c4->c4trans.trans, 0, 0 ) ;
}


int S4FUNCTION code4tranRollback( CODE4 *c4 )
{
   int rc ;

   E4PARHIGH( c4, E93830 ) ;

   if ( code4transEnabled( c4 ) != 1 )
      return error4( c4, e4trans, E83807 ) ;

   rc = tran4lowRollback( &c4->c4trans.trans, 0, 1 ) ;
   if ( rc < 0 )
      return rc ;

/*   rc = code4invalidate( c4 ) ;   --> moved to low rollback.*/
/*   if ( rc < 0 )*/
/*      return rc ;*/

   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      return code4unlock( c4 ) ;
   #endif
}

int S4FUNCTION code4transFileEnable( CODE4TRANS *c4trans, const char *logName, const int doCreate )
{
   int rc ;
   CODE4 *c4 ;

   if ( c4trans->enabled == 1 )
   { return 0 ;
   }

   rc = code4tranInitLow( &c4trans->trans, c4trans ) ;
   if ( rc < 0 )
      return rc ;

   #ifdef E4ANALYZE
      if ( ( rc = code4transVerify( c4trans, 1 ) ) < 0 )
         return rc ;
   #endif

   rc = 0 ;

   c4 = c4trans->c4 ;

   if ( c4trans->enabled == 0 )
   {
      if ( logName != 0 )
      {
         if ( c4->transFileName )
            u4free( c4->transFileName ) ;

         c4->transFileName = (char *)u4allocFree( c4, (long)strlen( logName ) + 1L ) ;

         if ( c4->transFileName == 0 )
            rc = e4memory ;

         else
            strcpy( c4->transFileName, logName ) ;
      }

      if( c4->transFileName != 0 )
      {
         rc = tran4fileInit( &c4->transFile, c4trans ) ;
         if ( rc == 0 )
         {
            if ( doCreate == 0 )
            {
                rc = tran4fileOpen( &c4->transFile, c4->transFileName ) ;
            }
            else
            {
                rc = tran4fileCreate( &c4->transFile, c4->transFileName ) ;
            }

            if ( rc == 0 )
            {
               c4trans->enabled = 1 ;
               c4trans->transFile = &c4->transFile ;
   }  }  }  }

   if ( rc )
   {  u4free( c4->transFileName ) ;
      c4->transFileName = 0 ;
   }
   else
   {  c4trans->transFile->status = tran4notRollbackOrCommit ;
   } 
   return( rc );
}

int tran4addUser( TRAN4 *trans, const long clientId, const char *charId, const unsigned short int lenIn )
{
   int rc ;
   short int netIdLen ;
   unsigned short int len ;
   char *netId ;
   CODE4 *c4 ;
   static char defaultUser[] = "PUBLIC" ;

      #ifndef S4OFF_MULTI
         int i, oldNumAttempts ;
      #endif

   len = lenIn ;
   c4 = trans->c4trans->c4 ;

   if ( trans->c4trans->enabled == 1 && code4tranStatus( c4 ) != r4off )
   {
         #ifdef S4OFF_MULTI
            trans->userIdNo = 1 ;   /* only one user */
         #else
            if ( clientId == 0L )  /* need to manually get the id */
            {
               for ( i = 0 ;; i++ )
               {
                  if ( i >= TRAN4MAX_USERS )
                     return error4( c4, e4max, E83816 ) ;
                  oldNumAttempts = c4->lockAttempts ;
                  c4->lockAttempts = 1 ;
                  rc = file4lockInternal( &trans->c4trans->transFile->file, TRAN4LOCK_USERS + i + 1, 0, 1, 0 ) ;
                  c4->lockAttempts = oldNumAttempts ;
                  if ( rc == 0 )
                  {
                     trans->userIdNo = i + 1 ;
                     break ;
                  }
               }
            }
         #endif
      if ( len > sizeof( trans->userId ) )
         len = sizeof( trans->userId ) - 1 ;
      memcpy( trans->userId, charId, len ) ;
      trans->userId[len] = 0 ;
         netId = (char *)0 ;
         netIdLen = 0 ;
      rc = tran4set( trans, trans->currentTranStatus, -1L, clientId, TRAN4INIT, len + netIdLen + sizeof( len ) + sizeof( netIdLen), 0L, 0L ) ;
      if ( rc < 0 )
         return rc ;

      if ( tran4putData( trans, (void *)&netIdLen, sizeof( netIdLen ) ) == e4memory )
         return e4memory ;

      if ( netIdLen != 0 )
         if ( tran4putData( trans, (void *)netId, (unsigned int)netIdLen ) == e4memory )
            return e4memory ;

      if ( tran4putData( trans, (void *)&len, sizeof( len ) ) == e4memory )
         return e4memory ;
      if ( len == 0 )  /* empty char id */
      {
         if ( tran4putData( trans, (void *)defaultUser, strlen( defaultUser ) ) == e4memory )
            return e4memory ;
      }
      else
         if ( tran4putData( trans, (void *)charId, len ) == e4memory )
            return e4memory ;
      if ( tran4lowAppend( trans, 0, 0 ) != 0 )
         return e4transAppend ;
   }
   else
      return e4trans ;  /* must return error to client so it is known that code4tranInit failed */

   return 0 ;
}

#endif /* S4OFF_WRITE */
#endif  /* S4OFF_TRAN */

#ifndef S4OFF_MULTI
void S4FUNCTION tran4freeLocks( CODE4 *c4, SINGLE4DISTANT *toFree )
{
   LOCK4 *lock ;

   E4PARM_HRET( c4, E93828, ) ;

   for ( ;; )
   {
      lock = (LOCK4 *)single4distantToItem( toFree ) ;
      if ( lock == 0 )
         return ;
      single4distantPop( toFree ) ;
      mem4free( c4->lockMemory, lock ) ;
}  }

static void tran4unlock( SINGLE4DISTANT *toAdd, SINGLE4DISTANT *toUnlock )
{
   LOCK4 *lock ;

   for ( ;; )
   {
      lock = (LOCK4 *)single4distantToItem( toUnlock ) ;
      if ( lock == 0 )
         return ;
      single4distantPop( toUnlock ) ;
      single4add( single4distantToSingle( toAdd ), &lock->link ) ;
      lock4unlock( lock ) ;
} }

#endif /* SOFF_MULTI */

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4lock( CODE4 *c4 )
{
   #ifdef S4OFF_MULTI
      return 0 ;
   #else
      LOCK4 *lock ;
      SINGLE4DISTANT locked ;
      int rc = 0 ;
      CODE4TRANS *c4trans ;
       SINGLE4 locks ;
       int saveErr, count, saveUnlockAuto ;
       TRAN4 *trans ;

      #ifdef E4ANALYZE
         if ( ( rc = code4verify( c4, 1 ) ) < 0 )
            return rc ;
      #endif

      c4trans = &c4->c4trans ;
         trans = &c4->c4trans.trans ;
         saveUnlockAuto = code4unlockAuto( c4 ) ;

         if ( saveUnlockAuto == 1 )
         {
            rc = code4unlockDo( tran4dataList( trans ) ) ;
            if ( rc < 0 )
               return error4stack( c4, rc, E91008 ) ;
         }

         code4unlockAutoSet( c4, 0 ) ;

         single4init( &locks ) ;

         single4distantInitIterate( &locked, &locks ) ;
         for ( lock = 0, count = -1 ;; )
         {
            if ( lock == 0 )
            {
               lock = (LOCK4 *)single4next( &trans->locks ) ;
               single4distantInitIterate( &trans->toLock, &trans->locks ) ;
               count++ ;

               if ( lock == 0 )
               {
                  tran4freeLocks( c4, &locked ) ;
                  code4unlockAutoSet( c4, saveUnlockAuto ) ;
                  return 0 ;
               }

               if ( c4->lockAttemptsSingle != WAIT4EVER )
                  if ( count >= c4->lockAttemptsSingle )  /* timed out */
                  {
                     tran4unlock( &trans->toLock, &locked ) ;
                     code4unlockAutoSet( c4, saveUnlockAuto ) ;
                     return r4locked ;
                  }
            }

            switch( lock4lock( lock ) )
            {
               case r4success:
                  assert5( lock == (LOCK4 *)single4distantToItem( &trans->toLock ) ) ; /* ensure the lock we remove is the current lock */
                  single4distantPop( &trans->toLock ) ;
                  single4add( single4distantToSingle( &locked ), &lock->link ) ;
                  lock = (LOCK4 *)single4distantToItem( &trans->toLock ) ;
                  break ;
               case r4locked:
                  if ( c4->lockAttemptsSingle == 1 )  /* we only want one check, we failed, so set lock to 0 to exit out */
                     lock = 0 ;
                  else
                  {
                     single4distantInitIterate( &trans->toLock, single4distantToItem( &trans->toLock ) ) ;
                     lock = (LOCK4 *)single4distantToItem( &trans->toLock ) ;
                  }
                  break ;
               default:
                  saveErr = error4set( c4trans->c4, 0 ) ;
                  tran4unlock( &trans->toLock, &locked ) ;
                  error4set( c4trans->c4, saveErr ) ;
                  code4unlockAutoSet( c4, saveUnlockAuto ) ;
                  return -1 ;
            }
         }
   #endif  /* S4OFF_MULTI */
}

#ifdef E4ANALYZE
static int code4transVerify( CODE4TRANS *c4trans, int subs )
{
   int rc ;

   if ( c4trans == 0 )
      return error4( 0, e4parmNull, E93832 ) ;

   if ( subs == 1 )
   {
      if ( ( rc = code4verify( c4trans->c4, 1 ) ) < 0 )
         return rc ;
   }
   else
      if ( c4trans->c4 == 0 )
         return error4( 0, e4struct, E93832 ) ;

   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
            if ( c4trans->enabled == 1 )
               if ( c4trans->transFile == 0 )
                  return error4( c4trans->c4, e4struct, E93832 ) ;
      #endif
   #endif

   return 0 ;
}

int tran4verify( TRAN4 *t4, int subs )
{
   int rc ;
   #ifndef S4OFF_TRAN
      CODE4 *c4 ;
   #endif

   if ( t4 == 0 )
      return error4( 0, e4parmNull, E93801 ) ;

   if ( t4->c4trans == 0 )
      return error4( 0, e4struct, E93801 ) ;

   #ifndef S4OFF_TRAN
      c4 = t4->c4trans->c4 ;

      if ( ( c4->tranDataLen != 0 && c4->tranData == 0 ) || ( c4->tranDataLen == 0 && c4->tranData != 0 ) )
         return error4( c4, e4struct, E93801 ) ;

      if ( ( t4->dataPos != 0 && c4->tranDataLen == 0 ) )
         return error4( c4, e4struct, E93801 ) ;
   #endif

   if ( subs == 1 )
      if ( ( rc = code4transVerify( t4->c4trans, 1 ) ) < 0 )
         return rc ;

   return 0 ;
}
#endif  /* E4ANALYZE */

int code4tranInitLow( TRAN4 *t4, CODE4TRANS *c4trans )
{
   #ifdef E4PARM_LOW
      if ( t4 == 0 || c4trans == 0 )
         return error4( 0, e4parmNull, E93834 ) ;
   #endif

   t4->c4trans = c4trans ;

   #ifndef S4OFF_WRITE
      t4->transId = -1 ;
      t4->dataPos = 0 ;
      t4->pos = (unsigned long)-1 ;
   #endif

   #ifndef S4OFF_WRITE
      t4->currentTranStatus = r4inactive ;
   #endif

   tran4dataListSet( t4, &(t4->localDataList) ) ;

   #ifdef E4ANALYZE
      return tran4verify( t4, 0 ) ;
   #else
      return 0 ;
   #endif
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
int S4FUNCTION code4transInitUndo( CODE4TRANS *c4trans )
{
   #ifndef S4OFF_WRITE
         CODE4 *c4 ;
         #ifndef S4OFF_TRAN
            int rc, oldError ;
            LOG4HEADER header ;
            #ifndef S4OFF_MULTI
               #ifndef S4UTILS
                  short int i ;
               #endif
                  int oldNumAttempts ;
            #endif
         #endif
   #endif

   E4PARMLOW( c4trans, E93835 ) ;

   #ifdef S4OFF_WRITE
      return 0 ;
   #else
   #endif
}


#ifndef S4OFF_MULTI
#ifndef S4OFF_TRAN
/* ensure only one process is accessing the transaction file */
int S4FUNCTION code4tranLockTransactions( CODE4TRANS *c4trans, long lockByte )
{
   int rc, oldAttempts ;

   #ifdef E4ANALYZE
      if ( code4transVerify( c4trans, 1 ) < 0 )
         return 0 ;
   #endif

      #ifdef S4TESTING
         if ( c4trans->c4->doTransLocking == 0 )
            return 0 ;
      #endif

   if ( lockByte < TRAN4LOCK_BASE )
      return e4parm ;
   if ( c4trans->transFile->fileLocks & ( 1UL << ( lockByte - TRAN4LOCK_BASE ) ) )
      return 0 ;
   oldAttempts = c4trans->c4->lockAttempts ;
   c4trans->c4->lockAttempts = WAIT4EVER ;  /* try forever */
   rc = file4lockInternal( &c4trans->transFile->file, lockByte, 0, 1L, 0 ) ;
   c4trans->c4->lockAttempts = oldAttempts ;
   if ( rc == 0 )
      c4trans->transFile->fileLocks |= ( 1UL << ( lockByte - TRAN4LOCK_BASE ) ) ;
   return rc ;
}

#ifndef S4UTILS
int S4FUNCTION code4tranUnlockTransactions( CODE4TRANS *c4trans, long lockByte )
{
   int rc ;

   #ifdef E4ANALYZE
      if ( code4transVerify( c4trans, 1 ) < 0 )
         return 0 ;
   #endif

      #ifdef S4TESTING
         if ( c4trans->c4->doTransLocking == 0 )
            return 0 ;
      #endif

   if ( lockByte < TRAN4LOCK_BASE )
      return e4parm ;
   if ( !(c4trans->transFile->fileLocks & ( 1UL << ( lockByte - TRAN4LOCK_BASE ) ) ) )
      return e4unlock ;
   rc = file4unlockInternal( &c4trans->transFile->file, lockByte, 0, 1L, 0 ) ;
   if ( rc == 0 )
      c4trans->transFile->fileLocks &= !( 1UL << ( lockByte - TRAN4LOCK_BASE ) ) ;
   return rc ;
}
#endif /* S4UTILS */
#endif /* S4OFF_TRAN */

int tran4lock( TRAN4 *trans )
{
   LOCK4 *lock ;
   SINGLE4 locks ;
   SINGLE4DISTANT locked ;
   int saveErr, rc, saveUnlockAuto ;
   CODE4 *c4 ;

   #ifdef E4ANALYZE
      if ( ( rc = tran4verify( trans, 1 ) ) < 0 )
         return rc ;
   #endif

   c4 = trans->c4trans->c4 ;

   saveUnlockAuto = code4unlockAuto( c4 ) ;

   if ( saveUnlockAuto == 1 )
   {
      rc = code4unlockDo( tran4dataList( trans ) ) ;
      if ( rc < 0 )
         return error4stack( c4, rc, 0L ) ;
   }

   code4unlockAutoSet( c4, 0 ) ;

   single4init( &locks ) ;
   single4distantInitIterate( &locked, &locks ) ;
   single4distantInitIterate( &trans->toLock, &trans->locks ) ;
   lock = (LOCK4 *)single4next( &trans->locks ) ;
   for ( ;; )
   {
      if ( lock == 0 )
      {
         tran4freeLocks( c4, &locked ) ;
         code4unlockAutoSet( c4, saveUnlockAuto ) ;
            return 0 ;
      }

      switch( lock4lock( lock ) )
      {
         case r4success:
            assert5( lock == (LOCK4 *)single4distantToItem( &trans->toLock ) ) ;  /* ensure we are removing the correct lock */
            single4distantPop( &trans->toLock ) ;
            single4add( &locks, &lock->link ) ;
            lock = (LOCK4 *)single4distantToItem( &trans->toLock ) ;
            break ;
         case r4locked:
               single4distantInitIterate( &trans->toLock, single4distantToItem( &trans->toLock ) ) ;
               lock = (LOCK4 *)single4distantToItem( &trans->toLock ) ;
         break ;

         default:
            saveErr = error4set( trans->c4trans->c4, 0 ) ;
            tran4unlock( &trans->toLock, &locked ) ;
            code4unlockAutoSet( c4, saveUnlockAuto ) ;
            error4set( trans->c4trans->c4, saveErr ) ;
            return -1 ;
      }
   }
}
#endif  /* */

static DATA4 *tran4dataFull( TRAN4 *trans, const long serverId, const long clientId )
{
   DATA4 *data ;
   LIST4 *oldList ;

   data = tran4data( trans, serverId, clientId ) ;
   if ( data != 0 )
      return data ;

   oldList = tran4dataList( trans ) ;
   tran4dataListSet( trans, &trans->closedDataFiles ) ;
   data = tran4data( trans, serverId, clientId ) ;
   tran4dataListSet( trans, oldList ) ;

   return data ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
/* are there any active transactions which conflict with the data4? */
int tran4active( CODE4 *c4, DATA4 *data )
{

   #ifndef S4OFF_TRAN
      if ( code4transEnabled( c4 ) )
         if ( code4trans( c4 )->currentTranStatus == r4active )
            return e4transViolation ;

         if ( data->logVal == LOG4TRANS )   /* not logging this datafile, and not a transaction in progress... */
            return 0 ;

      #ifdef S4OFF_MULTI
         return 0 ;
      #else
            if ( d4lockTestFile( data ) == 1 )
               return 0 ;
            if ( data->dataFile->file.lowAccessMode != OPEN4DENY_NONE )
               return 0 ;
            /* in stand-alone, can't check active transactions, so just deny */
            return error4( c4, e4transViolation, E81504 ) ;
      #endif
   #else
      return 0 ;
   #endif  /* S4OFF_TRAN */
}

void S4FUNCTION code4lockClear( CODE4 *c4 )
{
   #ifndef S4OFF_MULTI
   E4PARM_HRET( c4, E91018, ) ;
         single4distantInitIterate( &(c4->c4trans.trans.toLock), &(c4->c4trans.trans.locks ) ) ;
         tran4freeLocks( c4, &(c4->c4trans.trans.toLock) ) ;
   #endif

}
