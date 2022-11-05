/* d4modify.c   (c)Copyright Sequiter Software Inc., 1988-1996.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */

#ifndef S4CLIENT

static int createReplacementFile( CODE4 *c4, FILE4 *newFile, const char *oldPathName )
{
   char buf[258] ;
   time_t t ;
   int i, rc, saveFlag, safety, pos ;
   #ifdef S4WINCE
      SYSTEMTIME st ;
   #endif

   saveFlag = c4getErrCreate( c4 ) ;
   safety = c4->safety ;
   c4setErrCreate( c4, 0 ) ;
   c4->safety = 1 ;

   /* use the d4 path to ensure the file gets created in the correct location */
   #ifdef S4MACINTOSH
      pos = 0 ;
      macDirTemp = c4->macDir ;
      macVolTemp = c4->macVol ;
      c4->macDir = d4->file.macSpec.parID ;
      c4->macVol = d4->file.macSpec.vRefNum ;
   #else
      pos = u4namePath( buf, sizeof( buf ) - 14, oldPathName ) ;
   #endif
   strcpy( buf + pos, "TEMP" ) ;
   for ( i = 0 ;; )
   {
      if ( i >= 100 )
      {
         rc = error4( c4, e4create, E80605 ) ;
         break ;
      }
      i++ ;

      #ifndef S4WINCE
         time( &t ) ;
      #else
         GetLocalTime(&st) ;
         t = st.wMilliseconds ;
      #endif
      t %= 10000L ;

      c4ltoa45( t, buf + pos + 4, -4 ) ;
      strcpy( buf + pos + 8, ".tmp" ) ;
      rc = file4create( newFile, c4, buf, 1 ) ;
      if ( rc <= 0 )
         break ;
      u4delayHundredth( 50 ) ;
   }
   #ifdef S4MACINTOSH
      c4->macDir = macDirTemp ;
      c4->macVol = macVolTemp ;
   #endif

   c4setErrCreate( c4, saveFlag ) ;
   c4->safety = safety ;

   return rc ;
}

/* copies the first nFieldsMatch fields from d2 to d1, assumes fields match
   if nFieldsMatch == -1, then it uses the DATA4 info to find mathching fields */
static int d4copy( DATA4 *d1, DATA4 *d2, int nFieldsMatch )
{
   CODE4 *c4 ;
   int areMemosOrNulls, i, numFields ;
   FIELD4 *f1, *f2 ;

   c4 = d1->codeBase ;

   if ( nFieldsMatch == -1 )
      numFields = d4numFields( d1 ) ;
   else
   {
      numFields = d4numFields( d2 ) ;
      #ifndef S4OFF_MEMO
      if ( d2->dataFile->memoFile.file.hand == INVALID4HANDLE ) /* Used to be 0 ? */
      {
      #endif
         areMemosOrNulls = 0 ;
         #ifdef S4FOX
            if ( d4version( d2 ) == 0x30 )  /* maybe null fields */
            {
               for ( i = 0 ; i < numFields ; i++ )
                  if ( d4fieldJ( d2, i+1 )->null == 1 )  /* has a null field */
                  {
                     #ifndef S4OFF_MEMO
                        switch( f4type( d4fieldJ( d2, i+1 ) ) )  /* if memo-type then don't count */
                        {
                           case r4memo:
                           case r4memoBin:
                           case r4gen:
                              continue ;
                        }
                     #endif
                     areMemosOrNulls = 1 ;
                     break ;
                  }
            }
         #endif
      #ifndef S4OFF_MEMO
      }
      else
         areMemosOrNulls = 1 ;
      #endif
   }

   d4top( d2 ) ;
   while ( !d4eof( d2 ) && ( error4code( c4 ) == 0 ) )
   {
      d4appendStart( d1, 0L ) ;
      /* not just good enough to memcpy fields if there are memos or null fields */
      if ( nFieldsMatch == -1 || (areMemosOrNulls == 1) )
      {
         for ( i = 1 ; i <= numFields ; i++ )
         {
            f1 = d4fieldJ( d1, i ) ;
            f2 = d4field( d2, f4name( f1 ) ) ;
            f4memoAssignField( f1, f2 ) ;
         }
      }
      else
         memcpy( d1->record, d2->record, d4recWidth( d2 ) ) ;
      d4append( d1 ) ;
      if ( d4skip( d2, 1L ) != 0 )
         break ;
   }

   if ( error4code( c4 ) != 0 )
      return -1 ;

   return 0 ;
}

/* if doIndexes is false, it means was add field, which did not require
   indexes to be changed, so their file handles are still good. */
static void d4replace( DATA4 *keep, DATA4 *newFiles, int doIndexes )
{
   FILE4LONG pos ;
   #ifndef S4OFF_INDEX
      INDEX4 *idx ;
      #ifdef S4CLIPPER
         TAG4FILE *t4file ;
      #else
         INDEX4FILE *idxFile ;
      #endif
   #endif
   #ifndef S4OFF_MEMO
      char buf[258] ;
      char doRename = 0, nmBuf1[258], nmBuf2[258] ;
   #endif

   #ifndef S4SINGLE
      /* ensure unlock done so partialy closed file does not attempt unlocks */
      d4unlockLow( keep, data4clientId( keep ), 0 ) ;
      d4unlockLow( newFiles, data4clientId( newFiles ), 0 ) ;
   #endif  /* S4SINGLE */

   #ifdef S4CLIPPER
      if ( l4numNodes( &keep->dataFile->tagfiles ) != 0 ) /* has indexes */
   #else
      if ( l4numNodes( &keep->dataFile->indexes ) != 0 ) /* has indexes */
   #endif
   {
      newFiles->dataFile->hasMdxMemo |= 0x01 ;
      file4longAssign( pos, 28, 0 ) ;
      file4writeInternal( &newFiles->dataFile->file, pos, &newFiles->dataFile->hasMdxMemo, 1 ) ;
   }

   /* first we need to flush the new file to disk, and update its header to ensure all is ok */
   #ifndef S4OFF_WRITE
      if ( newFiles->dataFile->fileChanged && newFiles->dataFile->file.isTemp != 1 && newFiles->dataFile->file.isReadOnly == 0 )
      {
         u4yymmdd( &newFiles->dataFile->yy ) ;
         #ifdef S4OFF_MULTI
            dfile4updateHeader( newFiles->dataFile, 1, 1 ) ;
         #else
            if ( newFiles->dataFile->file.lowAccessMode == OPEN4DENY_RW )
               dfile4updateHeader( newFiles->dataFile, 1, 1 ) ;
            else /* only date stamp required */
               dfile4updateHeader( newFiles->dataFile, 1, 0 ) ;
         #endif
      }
      d4flush( newFiles ) ;
   #endif

   file4replace( &keep->dataFile->file, &newFiles->dataFile->file ) ;
   #ifndef S4OFF_INDEX
      if ( doIndexes == 1 )
      {
         /* all indexes in keep are migrated over to newFiles.  It is assumed that newFiles indexes
            if existant deleted/closed those matching held in keep. */
         while ( (idx = (INDEX4 *)newFiles->indexes.lastNode) != 0 )
         {
            l4remove( &newFiles->indexes, idx ) ;
            l4add( &keep->indexes, idx ) ;
            idx->data = keep ;
            #ifndef S4CLIPPER
               idxFile = idx->indexFile ;
               l4remove( &newFiles->dataFile->indexes, idxFile ) ;
               l4add( &keep->dataFile->indexes, idxFile ) ;
               idxFile->dataFile = keep->dataFile ;
            #endif
         }
         #ifdef S4CLIPPER
            while ( (t4file = (TAG4FILE *)l4pop( &newFiles->dataFile->tagfiles)) != 0 )
               l4add( &keep->dataFile->tagfiles, t4file ) ;
         #endif
      }
   #endif /*S4OFF_INDEX */
   #ifndef S4OFF_MEMO
      if ( newFiles->dataFile->memoFile.file.hand != INVALID4HANDLE )
      {
         if ( keep->dataFile->memoFile.file.hand != INVALID4HANDLE )
            file4replace( &keep->dataFile->memoFile.file, &newFiles->dataFile->memoFile.file ) ;
         else  /* new memo file, must have added a memo field, just rename */
         {
            doRename = 1 ;
            u4nameCurrent( nmBuf1, sizeof( nmBuf1 ), newFiles->dataFile->memoFile.file.name ) ;
            strcpy( nmBuf2, keep->dataFile->file.name ) ;
            #ifdef S4MFOX
               #ifdef S4CASE_SEN
                  u4nameExt( nmBuf2, sizeof(nmBuf2), "fpt", 1 ) ;
               #else
                  u4nameExt( nmBuf2, sizeof(nmBuf2), "FPT", 1 ) ;
               #endif
            #else
               #ifdef S4CASE_SEN
                  u4nameExt( nmBuf2, sizeof(nmBuf2), "dbt", 1 ) ;
               #else
                  u4nameExt( nmBuf2, sizeof(nmBuf2), "DBT", 1 ) ;
               #endif
            #endif
         }
      }
      else /* delete the old one if there was one */
         if ( keep->dataFile->memoFile.file.hand != INVALID4HANDLE )
         {
            u4nameCurrent( buf, sizeof( buf ), keep->dataFile->memoFile.file.name ) ;
            file4close( &keep->dataFile->memoFile.file ) ;
            u4remove( buf ) ;
         }
   #endif
   /* at this point, the newFiles structure contains non-valid file handles,
      but all else must be freed up */

   d4close( newFiles ) ;
   #ifndef S4OFF_MEMO
      if ( doRename == 1 )
         u4rename( nmBuf1, nmBuf2 ) ;
   #endif
}

/* assumes file open exclusively, otherwise failures with outside apps may occur */
DATA4 *S4FUNCTION d4fieldsAdd( DATA4 *d4, int nFields, FIELD4INFO *fieldsToAdd )
{
   FILE4 newFile ;
   char buf[258] ;
   FIELD4INFO *fields, *cFields ;
   CODE4 *c4 ;
   DATA4 *newData ;
   int i, oldCompatibility, oldErrFieldName, oldSafety ;
   #ifdef S4CLIPPER
      int hasTags, oldAutoOpen ;
   #endif

   #ifdef E4PARM_LOW
      if ( d4 == 0 || (fieldsToAdd == 0 && nFields != 0 ) || nFields < 0 )
      {
         error4( 0, e4parm, E91102 ) ;
         return 0 ;
      }
   #endif

   #ifdef S4CLIPPER
      if ( d4tagDefault( d4 ) == 0 )
         hasTags = 0 ;
      else
         hasTags = 1 ;
   #endif

   if ( nFields == 0 )  /* no change */
      return d4 ;

   c4 = d4->codeBase ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   if ( d4->dataFile->file.isReadOnly == 1 )
   {
      error4describe( c4, e4write, E80606, d4alias( d4 ), 0, 0 ) ;
      return 0 ;
   }

   #ifndef S4OFF_MULTI
      if ( d4->dataFile->file.lowAccessMode != OPEN4DENY_RW )
      {
         error4describe( c4, e4write, E81306, d4alias( d4 ), 0, 0 ) ;
         return 0 ;
      }
   #endif
   oldErrFieldName = c4->errFieldName ;
   c4->errFieldName = 0 ;
   for ( i = 0 ; i < nFields ; i++ )
      if ( d4field( d4, fieldsToAdd[i].name ) != 0 ) /* duplicate field */
      {
         c4->errFieldName = oldErrFieldName ;
         error4( c4, e4fieldName, E91102 ) ;
         return 0 ;
      }
      else
         error4set( c4, 0 ) ;

   c4->errFieldName = oldErrFieldName ;

   /* we really just want to guarantee a file name in the current directory */
   if ( createReplacementFile( c4, &newFile, d4->dataFile->file.name ) != 0 )
      return 0 ;

   strcpy( buf, newFile.name ) ;
   file4close( &newFile ) ;

   fields = (FIELD4INFO *)u4allocFree( c4, sizeof( FIELD4INFO ) * ( d4numFields( d4 ) + nFields + 1 ) ) ;
   if ( fields == 0 )
   {
      error4( c4, e4memory, E91102 ) ;
      return 0 ;
   }

   cFields = d4fieldInfo( d4 ) ;
   memcpy( fields, cFields, sizeof( FIELD4INFO ) * d4numFields( d4 ) ) ;
   u4free( cFields ) ;
   memcpy( fields + d4numFields( d4 ), fieldsToAdd, sizeof( FIELD4INFO ) * nFields ) ;
   memset( fields + d4numFields( d4 ) + nFields, 0, sizeof( FIELD4INFO ) ) ;

   oldCompatibility = c4->compatibility ;
   if ( d4version( d4 ) == 0x30 )  /* make sure new create that way too */
      c4->compatibility = 30 ;
   oldSafety = c4->safety ;   /* we need to overwrite our just-created file... */
   c4->safety = 0 ;
   newData = d4create( c4, buf, fields, 0 ) ;
   c4->safety = oldSafety ;
   u4free( fields ) ;
   c4->compatibility = oldCompatibility ;
   if ( newData == 0 )
      return 0 ;

   if ( d4copy( newData, d4, d4numFields( d4 ) ) != 0 )
   {
      d4close( newData ) ;
      return 0 ;
   }

   /* delete now because esp. in .NTX index files must be replaced
      cannot delete because tag info goes bad; for clip, maybe just close everyone else */

   /* now rename the data, memo, and index files */
   d4replace( d4, newData, 0 ) ;

   u4nameCurrent( buf, sizeof( buf ), d4->dataFile->file.name ) ;

   d4close( d4 ) ;

   #ifdef S4CLIPPER
      oldAutoOpen = c4->autoOpen ;
      c4->autoOpen = hasTags ;
   #endif

   newData = d4open( c4, buf ) ;

   #ifdef S4CLIPPER
      c4->autoOpen = oldAutoOpen ;
   #endif

   return newData ;
}

typedef struct
{
   int create ;     /* does it need to be created or just opened? */
   char pathName[258] ;
   const char *namePtr ;
   TAG4INFO *tags ;
} I4INFO_EXT ;

static void i4infoExtFree( I4INFO_EXT *info )
{
   int i ;
   char *ptr ;

   if ( info->tags != 0 )
   {
      for ( i = 0 ;; i++ )
      {
         if ( info->tags[i].name == 0 )
            break ;
         if ( info->tags[i].name != 0 )
         {
            u4free( info->tags[i].name ) ;
            info->tags[i].name = 0 ;
         }
         if ( info->tags[i].expression != 0 )
         {
            ptr = (char *)(info->tags[i].expression) ;
            u4free( ptr ) ;
         }
         if ( info->tags[i].filter != 0 )
         {
            ptr = (char *)(info->tags[i].filter) ;
            u4free( ptr ) ;
         }
      }
      u4free( info->tags ) ;
      info->tags = 0 ;
   }
}
#ifndef S4OFF_INDEX
static void i4infoExtInfo( DATA4 *data, I4INFO_EXT *info, INDEX4 *i4, TAG4INFO *tags )
{
   int i, numTags, len ;

   #ifdef E4PARM_LOW
      if ( info == 0 || i4 == 0 )
      {
         error4( 0, e4parmNull, E91102 ) ;
         return ;
      }
   #endif

   for ( numTags = 0 ;; numTags++ )
      if ( tags[numTags].name == 0 )
         break ;

   info->tags = (TAG4INFO *)u4alloc( ( numTags + 1 ) * sizeof( TAG4INFO ) ) ;
   #ifdef S4CLIPPER
      /* clipper is always group file... (i.e. use autoOpen off to avoid) */
      info->namePtr = info->pathName ;
      u4nameCurrent( info->pathName, sizeof( info->pathName ), data->dataFile->file.name ) ;
      u4nameExt( info->pathName, sizeof( info->pathName ), ".CGP", 1 ) ;
   #else
      /* always assign the pathName, even if production index file */
      u4nameCurrent( info->pathName, sizeof( info->pathName ), i4->indexFile->file.name ) ;
      if ( index4isProduction( i4->indexFile ) ) /* mark as not */
         info->namePtr = 0 ;
      else
         info->namePtr = info->pathName ;
   #endif

   for ( i = 0 ; i < numTags ; i++ )
   {
      if ( tags[i].name == 0 )
         break ;
      len = strlen( tags[i].name ) ;
      info->tags[i].name = (char *)u4alloc( len + 1 ) ;
      memcpy( info->tags[i].name, tags[i].name, len ) ;
      len = strlen( tags[i].expression ) ;
      info->tags[i].expression = (char *)u4alloc( len + 1 ) ;
      memcpy( (char *)info->tags[i].expression, tags[i].expression, len ) ;
      if ( tags[i].filter != 0 )
      {
         len = strlen( tags[i].filter ) ;
         if ( len != 0 )
         {
            info->tags[i].filter = (char *)u4alloc( len + 1 ) ;
            memcpy( (char *)info->tags[i].filter, tags[i].filter, len ) ;
         }
      }
      info->tags[i].unique = tags[i].unique ;
      info->tags[i].descending = tags[i].descending ;
   }

   return ;
}
#endif /*S4OFF_INDEX */
/* removes the fields given by names
   also cleans up the tags
   also returns 0 if all fields are removed */
DATA4 *S4FUNCTION d4fieldsRemove( DATA4 **d4, int nFields, char *names[] )
{
   FILE4 newFile ;
   CODE4 *c4 ;
   FIELD4 *field ;
   FIELD4INFO *fields ;
   int fieldCount, i, j, done, oldCompatibility, oldErrFieldName ;
   int oldErrExpr, oldAutoOpen, oldSafety, oldAccessMode ;
   DATA4 *newData ;
   char buf[258] ;
   DATA4FILE *d4file ;
   #ifdef S4SERVER
      int oldSingleClient ;
   #endif
   #ifndef S4OFF_INDEX
      int nTags, numIndexes, needRecreate, idxOn ;
      TAG4INFO *tags = 0 ;
      INDEX4 *idx = 0, *iNext = 0 ;
      EXPR4 *expr = 0 ;
      I4INFO_EXT *indexInfo = 0 ;
      #ifdef S4CLIPPER
         short iTag ;
      #endif
   #endif
   #ifdef S4CLIPPER
      char txbuf[258] ;
   #else
      FILE4LONG fPos ;
   #endif
   #ifndef S4OFF_MEMO
      char hadMemo = 0, memoName[258] ;
   #endif
   #ifdef E4PARM_LOW
      if ( d4 == 0 || *d4 == 0 || (names == 0 && nFields != 0 ) || nFields < 0 )
      {
         error4( 0, e4parm, E91102 ) ;
         return 0 ;
      }
   #endif

   if ( nFields == 0 )  /* no change */
      return *d4 ;

   c4 = (*d4)->codeBase ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   d4file = (*d4)->dataFile ;

   #ifndef S4OFF_MEMO
      hadMemo = d4file->memoFile.file.hand != INVALID4HANDLE ;
      if ( hadMemo )
         u4nameCurrent( memoName, sizeof( memoName ), d4file->memoFile.file.name ) ;
   #endif

   if ( d4file->file.isReadOnly == 1 )
   {
      error4describe( c4, e4write, E80606, d4alias( (*d4) ), 0, 0 ) ;
      return 0 ;
   }
   #ifndef S4OFF_MULTI
      if ( d4file->file.lowAccessMode != OPEN4DENY_RW )
      {
         error4describe( c4, e4write, E81306, d4alias( (*d4) ), 0, 0 ) ;
         return 0 ;
      }
   #endif
   oldErrFieldName = c4->errFieldName ;
   fields = 0 ;
   newData = 0 ;
   oldCompatibility = c4->compatibility ;
   oldErrExpr = c4->errExpr ;

   c4->errFieldName = 0 ;
   c4->errExpr = 0 ;

   fieldCount = d4numFields( (*d4) ) ;
   fields = d4fieldInfo( (*d4) ) ;

   for( ;; )
   {
      for ( i = 0 ; i < nFields ; i++ )
      {
         field = d4field( (*d4), names[i] ) ;
         if ( field == 0 )
         {
            error4( c4, e4fieldName, E91102 ) ;
            break ;
         }

         /* now find the field within the field info and delete */
         for ( j = 0, done = 0 ; j < fieldCount ; j++ )
            if ( strcmp( fields[j].name, f4name( field ) ) == 0 )
            {
               memmove( &(fields[j]), &(fields[j+1]), sizeof( FIELD4INFO ) * (fieldCount - j) ) ;  /* also copy the null entry over */
               fieldCount-- ;
               done = 1 ;
               break ;
            }

         if ( done == 1 )
            continue ;

         /* if here, then error because field not found */
         error4( c4, e4parm, E91102 ) ;
         break ;
      }

      if ( error4code( c4 ) < 0 )
         break ;

      if ( fieldCount == 0 ) /* means we removed all fields */
      {
         d4remove( (*d4) ) ;
         *d4 = 0 ;
         break ;
      }

      /* we really just want to guarantee a file name in the current directory */
      if ( createReplacementFile( c4, &newFile, d4file->file.name ) != 0 )
         break ;
      strcpy( buf, newFile.name ) ;
      file4close( &newFile ) ;

      if ( d4version( (*d4) ) == 0x30 )  /* make sure new create that way too */
         c4->compatibility = 30 ;
      oldSafety = c4->safety ;  /* we must replace our created replacement file */
      c4->safety = 0 ;
      oldAccessMode = c4->accessMode ;
      c4->accessMode = OPEN4DENY_RW ;
      newData = d4create( c4, buf, fields, 0 ) ;
      c4->accessMode = oldAccessMode ;
      c4->safety = oldSafety ;
      if ( newData == 0 )
         break ;

      #ifndef S4OFF_INDEX
         /* now remove any tags associated with the removed fields - just evaluate them, and if
            an error (expression error), then remove
            note that if no changes, then leave alone, else rebuild index from scratch
            with new tag info (we don't have a remove function) */
         tags = 0 ;

         numIndexes = l4numNodes( &(*d4)->indexes ) ;
         if ( numIndexes != 0 )
         {
            indexInfo = (I4INFO_EXT *)u4alloc( sizeof( I4INFO_EXT ) * numIndexes ) ;

            for( idxOn = 0, iNext = (INDEX4 *)l4first( &(*d4)->indexes ) ;; idxOn++ )
            {
               idx = iNext ;
               if ( idx == 0 )
                  break ;
               iNext = (INDEX4 *)l4next( &(*d4)->indexes, idx ) ;
               tags = i4tagInfo( idx ) ;
               if ( tags == 0 )
                  break ;
               for ( nTags = 0 ;; nTags++ )
                  if ( tags[nTags].name == 0 )  /* done */
                     break ;
               for ( i = 0, needRecreate = 0 ;; i++ )
               {
                  if ( tags[i].name == 0 )  /* done */
                     break ;
                  expr = expr4parse( newData, tags[i].expression ) ;
                  if ( expr == 0 )
                  {
                     #ifdef S4CLIPPER
                        TAG4 *tag ;
                        char tbuf[258] ;
                     #endif
                     error4set( c4, 0 ) ;
                     #ifdef S4CLIPPER
                        tag = d4tag( (*d4), tags[i].name ) ;
                        if ( tag == 0 )  /* should be impossible */
                        {
                           error4( c4, e4info, E91102 ) ;
                           return 0 ;
                        }
                        u4nameCurrent( txbuf, sizeof( txbuf ), idx->accessName ) ;
                        u4nameExt( txbuf, sizeof( txbuf ), "CGP", 0 ) ;
                        u4nameCurrent( tbuf, sizeof( tbuf ), tag->tagFile->file.name ) ;
                        t4close( tag ) ;
                        u4remove( tbuf ) ;
                     #endif
                     memcpy( tags + i, tags + i + 1, sizeof( TAG4INFO ) * (nTags - i) ) ;  /* also copy last (null) entry */
                     i-- ;
                     nTags-- ;
                     needRecreate = 1 ;
                  }
                  else
                     expr4free( expr ) ;
               }

               if ( needRecreate == 1 )
               {
                  if ( nTags == 0 )  /* means remove all tags */
                     indexInfo[idxOn].create = 2 ;
                  else
                     indexInfo[idxOn].create = 1 ;

                  i4infoExtInfo( (*d4), &indexInfo[idxOn], idx, tags ) ;

                  #ifndef S4CLIPPER
                     idx->indexFile->dataFile->hasMdxMemo &= (~1) ;
                     file4longAssign( fPos, 28, 0 ) ;
                     file4writeInternal( &idx->indexFile->dataFile->file, fPos, &idx->indexFile->dataFile->hasMdxMemo, 1 ) ;
                     idx->indexFile->dataFile->openMdx = 0 ;
                     #ifdef S4MDX
                        idx->indexFile->header.isProduction = 0 ;
                     #endif
                     i4closeLow( idx ) ;
                  #endif
               }
               else
                  indexInfo[idxOn].create = 0 ;

               if ( tags != 0 )
               {
                  u4free( tags ) ;
                  tags = 0 ;
               }
            }
         }
      #endif  /* S4OFF_INDEX */
      /* now copy contents over. */

      d4copy( newData, (*d4), -1 ) ;

      d4replace( (*d4), newData, 1 ) ;

      u4nameCurrent( buf, sizeof( buf ), d4file->file.name ) ;

      d4close( (*d4) ) ;
      *d4 = 0 ;

      oldAutoOpen = c4->autoOpen ;
      c4->autoOpen = 0 ;
      oldAccessMode = c4->accessMode ;
      c4->accessMode = OPEN4DENY_RW ;
      #ifdef S4SERVER
         oldSingleClient = c4->singleClient ;
         c4->singleClient = OPEN4DENY_RW ;
      #endif
      newData = d4open( c4, buf ) ;
      #ifdef S4SERVER
         c4->singleClient = oldSingleClient ;
      #endif
      c4->accessMode = oldAccessMode ;
      c4->autoOpen = oldAutoOpen ;

      /* now do the indexes */
      #ifndef S4OFF_INDEX
         if ( newData != 0 )
         {
            oldSafety = c4->safety ;
            c4->safety = 0 ;
            for ( i = 0 ; i < numIndexes ; i++ )
            {
               switch( indexInfo[i].create )
               {
                  case 0: /* open */
                     /* open all the tags if clipper */
                     #ifdef S4CLIPPER
                        for ( iTag = 0 ; indexInfo[i].tags[iTag].name != 0 ; iTag++)
                        {
                           if ( t4open( newData, 0, indexInfo[i].tags[iTag].name ) == 0 )
                           {
                              d4close( newData ) ;
                              return 0 ;
                           }
                        }
                     #else
                        if ( i4open( newData, indexInfo[i].namePtr ) == 0 )
                        {
                           d4close( newData ) ;
                           return 0 ;
                        }
                     #endif
                     break ;
                  case 1:
                     oldAccessMode = c4->accessMode ;
                     c4->accessMode = OPEN4DENY_RW ;
                     if ( i4create( newData, indexInfo[i].namePtr, indexInfo[i].tags ) == 0 )
                     {
                        d4close( newData ) ;
                        c4->accessMode = oldAccessMode ;
                        return 0 ;
                     }
                     c4->accessMode = oldAccessMode ;
                     break ;
                  case 2:
                     #ifdef S4CLIPPER
                        u4remove( txbuf ) ;
                     #else
                        u4remove( indexInfo[i].pathName ) ;
                     #endif
                     break ;
                  default:
                     error4( c4, e4info, E91102 ) ;
                     break ;
               }
               i4infoExtFree( &indexInfo[i] ) ;
            }
            u4free( indexInfo ) ;
            c4->safety = oldSafety ;
         }
      #endif
      break ;
   }

   /* recover code here */

   c4->compatibility = oldCompatibility ;
   c4->errFieldName = oldErrFieldName ;
   c4->errExpr = oldErrExpr ;
   if ( fields != 0 )
      u4free( fields ) ;
   #ifndef S4OFF_INDEX
      if ( tags != 0 )
         u4free( tags ) ;
   #endif
   #ifndef S4OFF_MEMO
      if ( hadMemo && newData != 0 ) /* maybe memo file deleted */
        if ( newData->dataFile->memoFile.file.hand == INVALID4HANDLE )
           u4remove( memoName ) ;
   #endif
   return newData ;
}

#endif /* S4CLIENT */
