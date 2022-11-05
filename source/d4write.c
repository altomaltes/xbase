/* d4write.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TURBOC__ */
#endif  /* S4UNIX */

#ifndef S4OFF_WRITE
#ifdef S4CLIENT

int S4FUNCTION d4writeLow( DATA4 *d4, const long recIn, const int unlock )
{
   int rc ;
   CODE4 *c4 ;
   CONNECTION4 *connection ;
   CONNECTION4WRITE_INFO_IN *info ;
   CONNECTION4WRITE_INFO_OUT *out ;
   long rec ;
   #ifndef S4OFF_MEMO
      CONNECTION4MEMO *memo ;
      F4MEMO *mfield ;
      short i ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E92601 ) )
         return 0 ;
   #endif  /* S4VBASIC */

   if ( recIn == -1 )
      rec = d4recNo( d4 ) ;
   else
      rec = recIn ;

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
         return error4( 0, e4parm_null, E92601 ) ;
      if ( rec == 0 || rec < -1 || d4->codeBase == 0 )
         return error4( d4->codeBase, e4parm, E92601 ) ;
      #ifdef E4ANALYZE
         if ( d4->dataFile == 0 )
            return error4( d4->codeBase, e4parm, E92601 ) ;
         if ( d4->dataFile->connection == 0 )
            return error4( d4->codeBase, e4parm, E92601 ) ;
      #endif
   #endif

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   #ifdef E4MISC
      if ( d4->record[0] != ' ' && d4->record[0] != '*' )
         return error4( c4, e4info, E83301 ) ;
   #endif

   if ( d4->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;

   /* ensure the record is being written to a valid position */
   if ( rec > d4recCount( d4 ) + d4lockTestAppend( d4 ) )
      return error4describe( c4, e4write, E82601, d4alias( d4 ), 0, 0 ) ;

   connection = d4->dataFile->connection ;
   connection4assign( connection, CON4WRITE, data4clientId( d4 ), data4serverId( d4 ) ) ;
   connection4addData( connection, NULL, sizeof( CONNECTION4WRITE_INFO_IN ), (void **)&info ) ;
   #ifndef S4OFF_MEMO
      for ( i = 0 ; i < d4->dataFile->nFieldsMemo ; i++ )
      {
         mfield = d4->fieldsMemo + i ;
         if ( mfield->isChanged == 1 )
            info->numMemoFields++ ;
      }
   #endif
   info->numMemoFields = htons(info->numMemoFields) ;
   info->recNo = htonl(rec) ;
   info->unlock = unlock ;
   connection4addData( connection, d4->record, dfile4recWidth( d4->dataFile ), NULL ) ;
   #ifndef S4OFF_MEMO
      for ( i = 0 ; i < d4->dataFile->nFieldsMemo ; i++ )
      {
         mfield = d4->fieldsMemo + i ;
         if ( mfield->isChanged == 1 )
         {
            connection4addData( connection, NULL, sizeof( CONNECTION4MEMO ), (void **)&memo ) ;
            memo->fieldNum = htons(i) ;
            memo->memoLen = htonl(mfield->len) ;
            if ( mfield->len > 0 )
               connection4addData( connection, mfield->contents, mfield->len, NULL ) ;
         }
      }
   #endif
   rc = connection4repeat( connection ) ;
   if ( rc == r4locked )
      return rc ;
   if ( rc < 0 )
      return connection4error( connection, c4, rc, E92601 ) ;
   if ( connection4len( connection ) != sizeof( CONNECTION4WRITE_INFO_OUT ) )
      return error4( c4, e4packetLen, E92601 ) ;
   out = (CONNECTION4WRITE_INFO_OUT *)connection4data( connection ) ;
   if ( out->recordLocked )
      d4localLockSet( d4, d4recNo( d4 ) ) ;

   if ( rc > 0 )  /* eg. r4entry or r4locked */
      return rc ;
   #ifndef S4OFF_MEMO
      for ( i = 0 ; i < d4->dataFile->nFieldsMemo ; i++ )
      {
         mfield = d4->fieldsMemo + i ;
         mfield->isChanged = 0 ;
      }
   #endif
   d4->recordChanged = 0 ;
   return 0 ;
}

#else
static int d4unwriteKeys( DATA4 *, const long ) ;

int S4FUNCTION d4writeLow( DATA4 *d4, const long recIn, const int unlock )
{
   long rec ;
   int rc, finalRc, old ;
   CODE4 *c4 ;
   #ifndef S4OFF_TRAN
      int hasTran ;
      unsigned long len ;
      TRAN4 *trans = 0 ;
      S4LONG connectionId = 0L ;
      S4LONG recNo ;
      #ifndef S4OFF_MEMO
         unsigned int ptrLen ;
         char *ptr, *tempRecord ;
         #ifdef S4MFOX
            long type ;
         #endif
         S4LONG entry ;
         unsigned S4LONG tempLong ;
         unsigned S4LONG zero = 0L ;
      #endif
   #endif
   #ifndef S4OFF_MEMO
      int i ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E92601 ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( d4 == 0 )
         return error4( 0, e4parm_null, E92601 ) ;
      if ( recIn < -1 || recIn == 0 || d4->codeBase == 0 )
         return error4( d4->codeBase, e4parm, E92601 ) ;
   #endif

   #ifdef E4MISC
      if ( d4->record[0] != ' ' && d4->record[0] != '*' )
         return error4( d4->codeBase, e4info, E83301 ) ;
   #endif
   if ( recIn == -1 )
      rec = d4recNo( d4 ) ;
   else
      rec = recIn ;

   c4 = d4->codeBase ;
   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   if ( d4->readOnly == 1 )
      return error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;

   old = d4->recordChanged ;
   d4->recordChanged = 0 ;

   /* set lock before transaction handling since cannot otherwise rollback */
   #ifndef S4OFF_MULTI
      #ifdef S4SERVER
         rc = dfile4lock( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ), rec ) ;
      #else
         rc = d4lock( d4, rec ) ;
      #endif
      if ( rc )
      {
         d4->recordChanged = old ;
         return rc ;
      }
   #endif  /* S4OFF_MULTI */

   /* ensure the record is being written to a valid position */
   if ( d4bof( d4 ) || d4eof( d4 ) ||
   #ifdef S4OFF_MULTI
      ( rec > d4recCount( d4 ) + 1 ) )
   #else
      ( rec > d4recCount( d4 ) + dfile4lockTestAppend( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) ) )
   #endif
   {
      d4->recordChanged = old ;
      return error4( c4, e4write, E82601 ) ;
   }

   #ifndef S4OFF_TRAN
      hasTran = 0 ;
      if ( d4->logVal != LOG4TRANS )
         if ( code4transEnabled( c4 ) )
            if ( ( code4tranStatus( c4 ) == r4inactive ) )  /* start a mini-transaction */
            {
               rc = code4tranStartSingle( c4 ) ;
               if ( rc != 0 )
               {
                  d4->recordChanged = old ;
                  return rc ;
               }
               hasTran = 1 ;
            }
   #endif
   /* 0. Validate memo id's */
   /* 1. Update Keys */
   /* 2. Update Memo Information */
   /* 3. Update Data FILE4 */

   #ifndef S4OFF_MEMO
      #ifndef S4OFF_MULTI
         if ( d4->dataFile->nFieldsMemo > 0 )
            if ( ( rc = d4validateMemoIds( d4 ) ) != 0 )
            {
               d4->recordChanged = old ;
               #ifndef S4OFF_TRAN
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
               #endif
               return rc ;
            }
      #endif  /* S4OFF_MULTI */
   #endif  /* S4OFF_MEMO */

   #ifndef S4OFF_TRAN
      if ( code4transEnabled( c4 ) )
         if ( code4tranStatus( c4 ) == r4active )
         {
            trans = code4trans( c4 ) ;
            #ifndef S4STAND_ALONE
               connectionId = c4->currentClient->id ;
            #endif
            recNo = d4recNo( d4 ) ;
            rc = tran4set( trans, trans->currentTranStatus, -1L, connectionId, TRAN4WRITE,
                 sizeof( recNo ) + 2 * dfile4recWidth( d4->dataFile ), data4clientId( d4 ), data4serverId( d4 ) ) ;
            if ( rc < 0 )
            {
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
               d4->recordChanged = old ;
               return 0 ;
            }
            if ( tran4putData( trans, &recNo, sizeof( recNo ) ) == e4memory )
            {
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
               d4->recordChanged = old ;
               return 0 ;
            }
            if ( d4readOld( d4, rec ) < 0 )
            {
               d4->recordChanged = old ;
               return -1 ;
            }
            if ( tran4putData( trans, d4->recordOld, dfile4recWidth( d4->dataFile ) ) == e4memory )
            {
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
               d4->recordChanged = old ;
               return 0 ;
            }
            if ( tran4putData( trans, d4record( d4 ), dfile4recWidth( d4->dataFile ) ) == e4memory )
            {
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
               d4->recordChanged = old ;
               return 0 ;
            }
            len = trans->header.dataLen ;
            #ifndef S4OFF_MEMO
               /* First cycle through the fields to be flushed */
               ptr = 0 ;
               ptrLen = 0 ;

               for ( i = 0; i < d4->dataFile->nFieldsMemo; i++ )
               {
                  if ( d4->fieldsMemo[i].isChanged == 1 )
                  {
                     tempRecord = d4->record ;
                     d4->record = d4->recordOld ;
                     entry = f4long( d4->fieldsMemo[i].field ) ;
                     d4->record = tempRecord ;

                     if ( entry == 0 )
                     {
                        if ( tran4putData( trans, &zero, sizeof( zero ) ) == e4memory )
                        {
                           rc = e4memory ;
                           break ;
                        }
                        len += sizeof( zero ) ;
                     }
                     else
                     {
                        #ifdef S4MFOX
                           rc = memo4fileRead( &d4->dataFile->memoFile, entry, &ptr, &ptrLen, &type ) ;
                        #else
                           rc = memo4fileRead( &d4->dataFile->memoFile, entry, &ptr, &ptrLen ) ;
                        #endif
                        if ( rc < 0 )
                           break ;
                        tempLong = ptrLen;
                        if ( tran4putData( trans, &tempLong, sizeof( tempLong ) ) == e4memory )
                        {
                           rc = e4memory ;
                           break ;
                        }
                        len += sizeof( tempLong ) ;
                        if ( ptrLen > 0 )
                        {
                           if ( tran4putData( trans, ptr, ptrLen ) == e4memory )
                           {
                              rc = e4memory ;
                              break ;
                           }
                           len += ptrLen ;
                        }
                     }
                     tempLong = d4->fieldsMemo[i].len;
                     if ( tran4putData( trans, &tempLong, sizeof( tempLong ) ) == e4memory )
                     {
                        rc = e4memory ;
                        break ;
                     }
                     len += sizeof( tempLong ) ;
                     if ( d4->fieldsMemo[i].len )
                     {
                        if ( tran4putData( trans, d4->fieldsMemo[i].contents, d4->fieldsMemo[i].len ) == e4memory )
                        {
                           rc = e4memory ;
                           break ;
                        }
                        len += d4->fieldsMemo[i].len ;
                     }
                  }
                  else
                  {
                     if ( tran4putData( trans, &zero, sizeof( zero ) ) == e4memory )
                     {
                        rc = e4memory ;
                        break ;
                     }
                     if ( tran4putData( trans, &zero, sizeof( zero ) ) == e4memory )
                     {
                        rc = e4memory ;
                        break ;
                     }
                     len += 2 * sizeof( zero ) ;
                  }
               }

               u4free( ptr ) ;
               ptr = 0 ;
               ptrLen = 0 ;
               if ( rc < 0 )
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  d4->recordChanged = old ;
                  return error4stack( c4, (short)rc, E92601 ) ;
               }
               trans->header.dataLen = (unsigned short int)len ;
            #endif  /* S4OFF_MEMO */

            #ifdef S4OFF_OPTIMIZE
               rc = tran4lowAppend( trans, 0, 1 ) ;
            #else
               rc = tran4lowAppend( trans, 0, ( ( d4->dataFile->file.bufferWrites == 1 && d4->dataFile->file.doBuffer == 1) ? 0 : 1 ) ) ;
            #endif
            if ( rc != 0 )
            {
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
               d4->recordChanged = old ;
               return rc ;
            }
         }
   #endif /* S4OFF_TRAN */

   #ifndef S4INDEX_OFF
      rc = d4writeKeys( d4, rec ) ;
   #endif
   d4->recordChanged = old ;
   #ifndef S4INDEX_OFF
      if ( rc )
      {
         #ifndef S4OFF_TRAN
            if ( hasTran )
               code4tranRollbackSingle( c4 ) ;
         #endif
         return rc ;
      }
   #endif

   finalRc = 0 ;

   #ifndef S4OFF_MEMO
      /* First cycle through the fields to be flushed */
      for ( i = 0; i < d4->dataFile->nFieldsMemo; i++ )
      {
         rc = f4memoUpdate( d4->fieldsMemo[i].field) ;
         if ( rc < 0 )
         {
            #ifndef S4OFF_TRAN
               if ( hasTran )
                  code4tranRollbackSingle( c4 ) ;
            #endif
            return error4stack( c4, (short)rc, E92601 ) ;
         }
         if ( rc > 0 )
            finalRc = rc ;
      }
   #endif  /* S4OFF_MEMO */

   rc = d4writeData( d4, rec ) ;
   if ( rc < 0 )
      d4unwriteKeys( d4, rec ) ;
   #ifndef S4OFF_TRAN
      if ( rc < 0 )
         if ( code4transEnabled( c4 ) )
            if ( code4tranStatus( c4 ) == r4active )
            {
               rc = tran4set( trans, trans->currentTranStatus, -1L,
                    connectionId, TRAN4VOID, (unsigned int)0, data4clientId( d4 ), data4serverId( d4 ) ) ;
               if ( rc < 0 )
               {
                  if ( hasTran )
                     code4tranRollbackSingle( c4 ) ;
                  return rc ;
               }
               #ifdef S4OFF_OPTIMIZE
                  return tran4lowAppend( trans, "\0", 1 ) ;
               #else
                  return tran4lowAppend( trans, "\0", ( ( d4->dataFile->file.bufferWrites == 1 && d4->dataFile->file.doBuffer == 1) ? 0 : 1 ) ) ;
               #endif
            }
   #else
      if ( rc < 0 )
         return rc ;
   #endif

   #ifndef S4OFF_TRAN
      if ( hasTran )
         code4tranCommitSingle( c4 ) ;
   #endif

   if ( unlock && code4unlockAuto( c4 ) != 0 ) /* unlock records (unless entire file is locked)... */
   {
      #ifdef S4SERVER
         if ( dfile4lockTestFile( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) == 0 )
            return dfile4unlockData( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ) ) ;
      #else
         #ifndef S4OFF_TRAN
            if ( code4transEnabled( c4 ) )
               if ( code4tranStatus( c4 ) == r4active )
                  return 0 ;
         #endif
         #ifndef S4OFF_MULTI
            if ( d4lockTestFile( d4 ) == 0 )
            {
               rc = d4unlockLow( d4, data4clientId( d4 ), 0 ) ;
               if ( rc == r4active )  /* just a transactional notification */
                  return 0 ;
               return rc ;
            }
         #endif
      #endif
   }

   return finalRc ;
}

int d4writeData( DATA4 *data, const long rec )
{
   #ifndef S4OFF_MULTI
      int rc ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E92602 ) ;
      if ( rec < 1 || data->codeBase == 0 )
         return error4( data->codeBase, e4parm, E92602 ) ;
   #endif

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_MULTI
      #ifdef S4SERVER
         rc = dfile4lock( data->dataFile, data4clientId( data ), data4serverId( data ), rec ) ;
      #else
         rc = d4lock( data, rec ) ;
      #endif
      if ( rc )
         return rc ;
   #endif  /* S4OFF_MULTI */

   data->recordChanged = 0 ;
   return dfile4writeData( data->dataFile, rec, data->record ) ;
}

int dfile4writeData( DATA4FILE *d4, const long rec, const char *record )
{
   #ifdef E4PARM_LOW
      if ( d4 == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
      if ( rec < 1 || d4->c4 == 0 )
         return error4( d4->c4, e4parm, E91102 ) ;
   #endif

   if ( error4code( d4->c4 ) < 0 )
      return e4codeBase ;

   d4->fileChanged = 1 ;
   return file4writeInternal( &d4->file, dfile4recordPosition(d4, rec), record, d4->recWidth ) ;
}

#ifndef S4OFF_TRAN
/* search the removed list for the specified keys/char combination.
   if recno is 0, then the matching character entry is returned */
TAG4KEY_REMOVED *t4keyFind( TAG4 *tag, long recno, char *key )
{
   TAG4KEY_REMOVED *found ;

   for ( found = 0 ; ; )
   {
      found =(TAG4KEY_REMOVED *)l4next( &tag->removedKeys, found ) ;
      if ( found == 0 )
         break ;
      if ( recno == 0 )
      {
         if ( c4memcmp( key, found->key, (unsigned int)tag->tagFile->header.keyLen ) == 0 )
            return found ;
      }
      else
         if ( found->recno == recno )
            if ( c4memcmp( key, found->key, (unsigned int)tag->tagFile->header.keyLen ) == 0 )
               return found ;
   }

   return 0 ;
}
#endif

int d4writeKeys( DATA4 *d4, const long rec )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      unsigned char newKeyBuf[I4MAX_KEY_SIZE] ;
      unsigned char *tempPtr ;
      char *saveRecBuffer ;
      unsigned char *oldKey ;
      int rc, rc2, saveError, keyLen, oldKeyAdded, addNewKey ;
      TAG4 *tagOn ;
      TAG4FILE *tagFileOn ;
      CODE4 *c4 ;
      #ifndef S4OFF_MULTI
         int indexLocked ;
      #endif
      #ifndef S4OFF_TRAN
         TAG4KEY_REMOVED *removed ;
      #endif
      #ifdef S4FOX
         int newKeyLen ;
      #endif

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm_null, E92604 ) ;
         if ( rec < 1 || d4->codeBase == 0 )
            return error4( d4->codeBase, e4parm, E92604 ) ;
      #endif

      c4 = d4->codeBase ;
      d4->bofFlag = d4->eofFlag = 0 ;

      #ifdef S4CB51
         #ifndef S4OFF_MULTI
            #ifdef S4SERVER
               rc = dfile4lock( d4->dataFile, data4clientId( d4 ), data4serverId( d4 ), rec, d4 ) ;
            #else
               rc = d4lock( d4, rec ) ;
            #endif
            if ( rc )
               return rc ;
         #endif  /* S4OFF_MULTI */
      #endif

      #ifdef S4CLIPPER
         if ( d4->dataFile->tagfiles.nLink > 0 )
      #else
         if ( d4->dataFile->indexes.nLink > 0 )
      #endif
      {
         if ( d4readOld( d4, rec ) < 0 )
            return -1 ;
         if ( u4memcmp( d4->recordOld, d4->record, dfile4recWidth( d4->dataFile )) == 0 )
            return 0 ;
      }

      saveRecBuffer = d4->record ;
      rc = 0 ;
      #ifndef S4OFF_MULTI
         #ifdef S4SERVER
            indexLocked = dfile4lockTestIndex( d4->dataFile, data4serverId( d4 ) ) ? 2 : 0 ;  /* 2 means was user locked */
         #else
            indexLocked = d4lockTestIndex( d4 ) ? 2 : 0 ;  /* 2 means was user locked */
         #endif
      #endif

      for( tagOn = 0 ;; )
      {
         tagOn = d4tagNext( d4, tagOn ) ;
         if ( tagOn == 0 )
            break ;

         tagFileOn = tagOn->tagFile ;
         oldKeyAdded = addNewKey = 1 ;
         tagOn->added = tagOn->removed = 0 ;

         rc2 = expr4context( tagFileOn->expr, d4 ) ;
         if ( rc2 < 0 )
         {
            rc = rc2 ;
            break ;
         }
         if ( tagFileOn->filter != 0 )
         {
            rc2 = expr4context( tagFileOn->filter, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
         }
         keyLen = tfile4exprKey( tagFileOn, &tempPtr ) ;
         if ( keyLen < 0 )
         {
            rc = -1 ;
            break ;
         }
         #ifdef E4ANALYZE
            if ( keyLen != tagFileOn->header.keyLen || keyLen > I4MAX_KEY_SIZE)
               return error4( c4, e4index, E92604 ) ;
         #endif

         memcpy( (void *)newKeyBuf, tempPtr, (unsigned int)keyLen ) ;
         #ifdef S4FOX
            newKeyLen = keyLen ;
         #endif

         if ( tagFileOn->filter )
            addNewKey = expr4true( tagFileOn->filter ) ;

         d4->record = d4->recordOld ;

         rc2 = expr4context( tagFileOn->expr, d4 ) ;
         if ( rc2 < 0 )
         {
            rc = rc2 ;
            break ;
         }
         if ( tagFileOn->filter != 0 )
         {
            rc2 = expr4context( tagFileOn->filter, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
         }
         if ( tagFileOn->filter )
            oldKeyAdded = expr4true( tagFileOn->filter ) ;
         keyLen = tfile4exprKey( tagOn->tagFile, &oldKey ) ;

         d4->record = saveRecBuffer ;

         if ( keyLen < 0 )
         {
            rc = keyLen ;
            break ;
         }
         if ( oldKeyAdded == addNewKey )
            #ifdef S4FOX
               if ( u4keycmp( (void *)newKeyBuf, oldKey, (unsigned int)keyLen, (unsigned int)newKeyLen, 0, &tagFileOn->vfpInfo ) == 0 )
            #else
               if ( u4memcmp( (void *)newKeyBuf, oldKey, (unsigned int)keyLen ) == 0 )
            #endif
                continue ;

         #ifndef S4OFF_MULTI
            if ( indexLocked == 0 )
            {
               indexLocked = 1 ;
               #ifdef S4SERVER
                  rc = dfile4lockIndex( d4->dataFile, data4serverId( d4 ) ) ;
               #else
                  rc = d4lockIndex( d4 ) ;
               #endif
               if ( rc )
                  break ;
            }
         #endif  /* S4OFF_MULTI */

         if ( oldKeyAdded )
         {
            tagOn->removed = 1 ;
            #ifndef S4OFF_TRAN
               if ( code4tranStatus( c4 ) == r4active && ( t4unique( tagOn ) == r4unique ||
                    t4unique( tagOn ) == e4unique
                    #ifdef S4FOX
                       || t4unique( tagOn ) == r4candidate
                    #endif
                    ) )  /* save the entry due to transactions */
               {
                  removed = (TAG4KEY_REMOVED *)u4allocFree( c4, (long)sizeof( LINK4 ) + (long)sizeof(S4LONG) + tagFileOn->header.keyLen ) ;
                  if ( removed == 0 )
                  {
                     rc = e4memory ;
                     break ;
                  }
                  removed->recno = rec ;
                  memcpy( removed->key, oldKey, (unsigned int)tagFileOn->header.keyLen ) ;
                  l4addBefore( &tagOn->removedKeys, l4first( &tagOn->removedKeys ), removed ) ;
               }
               else
               {
            #endif
               rc2 = expr4context( tagFileOn->expr, d4 ) ;
               if ( rc2 < 0 )
               {
                  rc = rc2 ;
                  break ;
               }
               if ( tagFileOn->filter != 0 )
               {
                  rc2 = expr4context( tagFileOn->filter, d4 ) ;
                  if ( rc2 < 0 )
                  {
                     rc = rc2 ;
                     break ;
                  }
               }

               if ( tfile4remove( tagFileOn, oldKey, rec ) < 0 )
               {
                  rc = -1 ;
                  break ;
               }
            #ifndef S4OFF_TRAN
               }
            #endif
         }

         #ifndef S4OFF_TRAN
            if ( code4tranStatus( c4 ) == r4rollback && ( t4unique( tagOn ) == r4unique ||
                 t4unique( tagOn ) == e4unique
                 #ifdef S4FOX
                    || t4unique( tagOn ) == r4candidate
                 #endif
                 ) )  /* remove the removal due to transactions */
            {
               removed = t4keyFind( tagOn, rec,(char *)newKeyBuf ) ;
               if ( removed != 0 )  /* means the record really was not deleted, so just remove from the list of to-be-removed */
               {
                  l4remove( &tagOn->removedKeys, removed ) ;
                  u4free( removed ) ;
                  addNewKey = 0 ;
               }
               /* else: removed is null when, within the same transaction,
                  a record alteration is made to replace the key that was to
                  be removed.  At that point, the key was actually removed,
                  so it must now be added back */
            }
         #endif
         if ( addNewKey )
         {
            tagOn->added = 1 ;
            rc2 = expr4context( tagFileOn->expr, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
            if ( tagFileOn->filter != 0 )
            {
               rc2 = expr4context( tagFileOn->filter, d4 ) ;
               if ( rc2 < 0 )
               {
                  rc = rc2 ;
                  break ;
               }
            }
            #ifndef S4OFF_TRAN
               /* if a unique tag, first check if the record may have instead been pre-deleted */
               if ( t4unique( tagOn ) != 0 )
               {
                  removed = t4keyFind( tagOn, 0L, (char *)newKeyBuf ) ;
                  if ( removed != 0 )   /* re-adding a key that was removed, so really remove this entry from the tag file first */
                  {
                     if ( tfile4remove( tagFileOn, removed->key, removed->recno ) < 0 )
                     {
                        rc = -1 ;
                        break ;
                     }
                     l4remove( &tagOn->removedKeys, removed ) ;
                     u4free( removed ) ;
                  }
               }
            #endif
            rc = tfile4add( tagFileOn, newKeyBuf, rec, t4unique( tagOn ) ) ;
            if ( rc == r4unique || rc == e4unique )
            {
               saveError = error4set( c4, 0 ) ;

               #ifndef S4OFF_TRAN
                  if ( code4tranStatus( c4 ) == r4active && ( t4unique( tagOn ) == r4unique ||
                       t4unique( tagOn ) == e4unique
                       #ifdef S4FOX
                          || t4unique( tagOn ) == r4candidate
                       #endif
                       ) )  /* just remove from the removed list */
                  {
                     removed =(TAG4KEY_REMOVED *)l4first( &tagOn->removedKeys ) ;
                     if ( removed == 0 )
                     {
                        error4( c4, e4info, E92604 ) ;
                        error4set( c4, 0 ) ;
                        rc = e4info ;
                        break ;
                     }
                     l4remove( &tagOn->removedKeys, removed ) ;
                     u4free( removed ) ;
                  }
                  else
               #endif
               if ( oldKeyAdded )
               {
                  rc2 = expr4context( tagFileOn->expr, d4 ) ;
                  if ( rc2 < 0 )
                  {
                     rc = rc2 ;
                     break ;
                  }
                  if ( tagFileOn->filter != 0 )
                  {
                     rc2 = expr4context( tagFileOn->filter, d4 ) ;
                     if ( rc2 < 0 )
                     {
                        rc = rc2 ;
                        break ;
                     }
                  }
                  if ( tfile4add( tagFileOn, (unsigned char *)oldKey, rec, t4unique( tagOn ) ) < 0 )
                  {
                     rc = -1 ;
                     break ;
                  }
               }

               /* Remove the keys which were just added */
               for(;;)
               {
                  tagOn = d4tagPrev( d4, tagOn ) ;
                  if ( tagOn == 0 )
                     break ;
                  tagFileOn = tagOn->tagFile ;

                  if ( tagOn->added )
                  {
                     d4->record = saveRecBuffer ;

                     rc2 = expr4context( tagFileOn->expr, d4 ) ;
                     if ( rc2 < 0 )
                     {
                        rc = rc2 ;
                        break ;
                     }
                     if ( tagFileOn->filter != 0 )
                     {
                        rc2 = expr4context( tagFileOn->filter, d4 ) ;
                        if ( rc2 < 0 )
                        {
                           rc = rc2 ;
                           break ;
                        }
                     }

                     rc2 = tfile4removeCalc( tagFileOn, rec ) ;
                     if ( rc2 < 0 )
                     {
                        rc = rc2 ;
                        break ;
                     }
                  }

                  if ( tagOn->removed )
                  {
                     d4->record = d4->recordOld ;

                     rc2 = expr4context( tagFileOn->expr, d4 ) ;
                     if ( rc2 < 0 )
                     {
                        rc = rc2 ;
                        break ;
                     }
                     if ( tagFileOn->filter != 0 )
                     {
                        rc2 = expr4context( tagFileOn->filter, d4 ) ;
                        if ( rc2 < 0 )
                        {
                           rc = rc2 ;
                           break ;
                        }
                     }
                     rc2 = t4addCalc( tagOn, rec ) ;
                     if ( rc2 < 0 )
                     {
                        d4->record = saveRecBuffer ;
                        rc = rc2 ;
                        break ;
                     }
                  }
               }

               d4->record = saveRecBuffer ;

               error4set( c4, (short)saveError ) ;
               if ( saveError < 0 )
                  rc = saveError ;
               break ;
            }
            if ( rc < 0 )   /* can't generate e4unique, so just set to -1 */
            {
               rc = -1 ;
               break ;
            }
            rc = 0 ;
         }
      }
      #ifndef S4OFF_MULTI
         if ( indexLocked == 1 )
            dfile4unlockIndex( d4->dataFile, data4serverId( d4 ) ) ;
      #endif

      d4->recNumOld = -1 ;
      return rc ;
   #endif  /* S4OFF_INDEX */
}

static int d4unwriteKeys( DATA4 *d4, const long rec )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      unsigned char newKeyBuf[I4MAX_KEY_SIZE] ;
      char *saveRecBuffer ;
      unsigned char *oldKey, *tempPtr ;
      int rc2, rc, keyLen, oldKeyAdded, addNewKey ;
      #ifndef S4OFF_MULTI
         int indexLocked ;
      #endif
      TAG4 *tagOn ;
      TAG4FILE *tagFileOn ;
      #ifndef S4OFF_TRAN
         TAG4KEY_REMOVED *removed ;
      #endif
      #ifdef S4FOX
         int newKeyLen ;
      #endif

      #ifdef E4PARM_HIGH
         if ( d4 == 0 )
            return error4( 0, e4parm_null, E92605 ) ;
         if ( rec < 1 || d4->codeBase == 0 )
            return error4( d4->codeBase, e4parm, E92605 ) ;
      #endif

      #ifdef N4OTHER
         if ( d4->dataFile->tagfiles.nLink > 0 )
      #else
         if ( d4->dataFile->indexes.nLink > 0 )
      #endif
         if ( u4memcmp( d4->recordOld, d4->record, dfile4recWidth( d4->dataFile )) == 0 )
            return 0 ;

      saveRecBuffer = d4->record ;

      rc = 0 ;
      #ifndef S4OFF_MULTI
         #ifdef S4SERVER
            indexLocked = dfile4lockTestIndex( d4->dataFile, data4serverId( d4 ) ) ? 2 : 0 ;  /* 2 means was user locked */
         #else
            indexLocked = d4lockTestIndex( d4 ) ? 2 : 0 ;  /* 2 means was user locked */
         #endif
      #endif

      for( tagOn = 0 ;; )
      {
         tagOn = d4tagNext( d4, tagOn ) ;
         if ( tagOn == 0 )
            break ;

         tagFileOn = tagOn->tagFile ;
         rc2 = expr4context( tagFileOn->expr, d4 ) ;
         if ( rc2 < 0 )
         {
            rc = rc2 ;
            break ;
         }
         if ( tagFileOn->filter != 0 )
         {
            rc2 = expr4context( tagFileOn->filter, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
         }

         oldKeyAdded = addNewKey = 1 ;

         keyLen = tfile4exprKey( tagFileOn, &tempPtr ) ;
         if ( keyLen < 0 )
         {
            rc = keyLen ;
            break ;
         }
         #ifdef E4ANALYZE
            if ( keyLen != tagFileOn->header.keyLen || keyLen > I4MAX_KEY_SIZE)
               return error4( d4->codeBase, e4index, E92605 ) ;
         #endif

         memcpy( (void *)newKeyBuf, tempPtr, (unsigned int)keyLen ) ;
         #ifdef S4FOX
            newKeyLen = keyLen ;
         #endif

         if ( tagFileOn->filter )
            addNewKey = expr4true( tagFileOn->filter ) ;

         d4->record = d4->recordOld ;

         rc2 = expr4context( tagFileOn->expr, d4 ) ;
         if ( rc2 < 0 )
         {
            rc = rc2 ;
            break ;
         }
         if ( tagFileOn->filter != 0 )
         {
            rc2 = expr4context( tagFileOn->filter, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
         }

         if ( tagFileOn->filter )
            oldKeyAdded = expr4true( tagFileOn->filter ) ;
         keyLen = tfile4exprKey( tagFileOn, &oldKey ) ;

         d4->record = saveRecBuffer ;

         if ( keyLen < 0 )
         {
            rc = keyLen ;
            break ;
         }
         if ( oldKeyAdded == addNewKey )
            #ifdef S4FOX
               if ( u4keycmp( (void *)newKeyBuf, oldKey, (unsigned int)keyLen, (unsigned int)newKeyLen, 0, &tagFileOn->vfpInfo ) == 0 )
            #else
               if ( u4memcmp( (void *)newKeyBuf, oldKey, (unsigned int)keyLen ) == 0 )
            #endif
                continue ;

         #ifndef S4OFF_MULTI
            if ( indexLocked == 0 )
            {
               indexLocked = 1 ;
               #ifdef S4SERVER
                  rc = dfile4lockIndex( d4->dataFile, data4serverId( d4 ) ) ;
               #else
                  rc = d4lockIndex( d4 ) ;
               #endif
               if ( rc )
                  break ;
            }
         #endif  /* S4OFF_MULTI */

         if ( oldKeyAdded )
         {
            #ifndef S4OFF_TRAN
               if ( code4tranStatus( d4->codeBase ) == r4active && ( t4unique( tagOn ) == r4unique ||
                    t4unique( tagOn ) == e4unique
                    #ifdef S4FOX
                       || t4unique( tagOn ) == r4candidate
                    #endif
                    ) )  /* save the entry due to transactions */
               {
                  for ( removed = 0 ;; )
                  {
                     removed = (TAG4KEY_REMOVED *)l4next( &tagOn->removedKeys, removed ) ;
                     /* if a reindex occurs, then the list will be empty,
                        so follow regular procedures.  Otherwise the key should
                        be on the removed list */
                     if ( removed == 0 )
                        break ;
                     if ( c4memcmp( removed->key, oldKey, (unsigned int)tagFileOn->header.keyLen ) == 0 && ( removed->recno == rec ) )
                     {
                        l4remove( &tagOn->removedKeys, removed ) ;
                        u4free( removed ) ;
                        break ;
                     }
                  }
                  if ( rc < 0 )
                     break ;
               }
               else
               {
            #endif
               rc2 = expr4context( tagFileOn->expr, d4 ) ;
               if ( rc2 < 0 )
               {
                  rc = rc2 ;
                  break ;
               }
               if ( tagFileOn->filter != 0 )
               {
                  rc2 = expr4context( tagFileOn->filter, d4 ) ;
                  if ( rc2 < 0 )
                  {
                     rc = rc2 ;
                     break ;
                  }
               }
               if ( tfile4add( tagFileOn, oldKey, rec, t4unique( tagOn ) ) < 0 )
               {
                  rc = -1 ;
                  break ;
               }
            #ifndef S4OFF_TRAN
               }
            #endif
         }

         #ifndef S4OFF_TRAN
            if ( code4tranStatus( d4->codeBase ) == r4rollback && ( t4unique( tagOn ) == r4unique ||
                 t4unique( tagOn ) == e4unique
                 #ifdef S4FOX
                    || t4unique( tagOn ) == r4candidate
                 #endif
                 ) )  /* remove the removal due to transactions */
            {
               rc = -1 ;
               break ;
            }
            else
         #endif
         if ( addNewKey )
         {
            rc2 = expr4context( tagFileOn->expr, d4 ) ;
            if ( rc2 < 0 )
            {
               rc = rc2 ;
               break ;
            }
            if ( tagFileOn->filter != 0 )
            {
               rc2 = expr4context( tagFileOn->filter, d4 ) ;
               if ( rc2 < 0 )
               {
                  rc = rc2 ;
                  break ;
               }
            }
            rc = tfile4remove( tagFileOn, newKeyBuf, rec ) ;
            if ( rc == r4unique || rc == e4unique )
            {
               rc = -1 ;
               break ;
            }
            if ( rc < 0 )
            {
               rc = -1 ;
               break ;
            }
            rc = 0 ;
         }
      }
      #ifndef S4OFF_MULTI
         if ( indexLocked == 1 )
            dfile4unlockIndex( d4->dataFile, data4serverId( d4 ) ) ;
      #endif

      d4->recNumOld = -1 ;
      return rc ;
   #endif  /* S4OFF_INDEX */
}
#endif  /* S4CLIENT */
#endif  /* S4OFF_WRITE */
