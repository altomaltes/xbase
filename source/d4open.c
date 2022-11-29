/* d4open.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifdef S4OFF_MEMO
   extern char f4memoNullChar ;
#endif

#ifndef S4MACINTOSH
   #ifdef __unix__
      #include <sys/stat.h>
      #include <sys/types.h>
   #else
      #ifndef S4WINCE
         #include "sys\stat.h"
      #endif
   #endif
#endif

static DATA4FILE *data4reopen( DATA4FILE *, char ** ) ;

static DATA4 *d4openInit( CODE4 *c4 )
{
   DATA4 *d4 ;
      #ifndef S4OFF_TRAN
         int rc;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E94301 ) )
         return 0 ;
   #endif

   if ( error4code( c4 ) < 0 )
      return 0 ;

   #ifdef E4ANALYZE
      if ( c4->debugInt != 0x5281 )
      {
         error4( 0, e4result, E81301 ) ;
         return 0 ;
      }
   #endif

   #ifndef S4OFF_TRAN
      if ( c4->logOpen )
      {
         rc = code4logOpen( c4, 0, 0 ) ;
         if ( rc < 0 )
            return 0 ;
         else
            error4set( c4, 0 ) ;   /* remove r4open if it already existed */
      }
   #endif

   if ( c4->dataMemory == 0 )
   {
      c4->dataMemory = mem4create( c4, c4->memStartData, sizeof(DATA4), c4->memExpandData, 0 ) ;
      if ( c4->dataMemory == 0 )
      {
         #ifdef E4STACK
            error4stack( c4, e4memory, E94301 ) ;
         #endif
         return 0 ;
      }
   }
   d4 = (DATA4 *)mem4alloc( c4->dataMemory ) ;
   if ( d4 == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4memory, E94301 ) ;
      #endif
      return 0 ;
   }

   #ifdef S4VBASIC
      d4->debugInt = E4DEBUG_INT ;
   #endif
   d4->codeBase = c4 ;

      d4->trans = &c4->c4trans.trans ;
      l4add( tran4dataList( (&(c4->c4trans.trans)) ), d4 ) ;

   return d4 ;
}

static int d4openConclude( DATA4 *d4, const char *name, char *info )
{
   CODE4 *c4 ;
   short iFields, fieldType, loop ;
   unsigned long recOffset ;
   long recWidth, recWidth2 ;
   char *savePtr ;
   char fieldBuf[2] ;
   FIELD4IMAGE *image ;
   #ifdef S4CFOX
      int nullCount ;
   #endif

      #ifdef S4CLIPPER
         #ifndef S4OFF_INDEX
            char nameBuf[258] ;
         #else
            #ifndef S4OFF_TRAN
               #ifndef S4OFF_WRITE
                  char nameBuf[258] ;
               #endif
            #endif
         #endif
      #else
         #ifndef S4OFF_TRAN
            #ifndef S4OFF_WRITE
               char nameBuf[258] ;
            #endif
         #endif
      #endif
      #ifndef S4OFF_WRITE
         #ifndef S4OFF_TRAN
            TRAN4  *trans ;
            int tranCode ;
            #ifndef S4MACINTOSH
               struct stat bufStat ;
            #else
               HParamBlockRec MacFile ;
               Str255 MacName ;
               char macNameBuf[258] ;
            #endif
            S4LONG connectionId, rcl ;
            short rc ;
         #endif
      #endif
      #ifndef S4OFF_INDEX
         INDEX4 *i4 ;
            #ifndef S4CLIPPER
               int oldSingleOpen ;
            #endif
      #endif

   #ifndef S4OFF_MEMO
      int i_memo ;
   #endif

   c4 = d4->codeBase ;

   #ifdef E4ANALYZE
      if ( d4->dataFile->nFields == 0 )
         return error4describe( c4, e4struct, E94301, name, 0, 0 ) ;
   #endif

   u4namePiece( d4->alias, sizeof( d4->alias ), name, 0, 0 ) ;
   d4->alias[ sizeof( d4->alias ) - 1 ] = 0 ;

   recWidth = dfile4recWidth( d4->dataFile ) ;
   #ifdef S4DATA_ALIGN
      #ifdef S4WINCE
         recWidth2 = (recWidth+1) + (4L - (recWidth+1L)%4L)%4L ;
      #else
         recWidth2 = (recWidth +1) + (recWidth+1)%2 ;
      #endif
   #else
      recWidth2 = recWidth +1 ;
   #endif
   if ( ( ( recWidth2 + 50L ) * 2L + (long)sizeof( FIELD4 ) * (long)d4->dataFile->nFields ) > (long)UINT_MAX )  /* try allocating records and fields together, 50 bytes for overhead */
   {
      d4->groupRecordAlloc = (char *)u4allocFree( c4, (recWidth2) * 3L + (long)sizeof( FIELD4 ) * (long)d4->dataFile->nFields ) ;
      if ( d4->groupRecordAlloc != 0 )
      {
         d4->record = d4->groupRecordAlloc ;
         d4->recordOld = d4->groupRecordAlloc + recWidth2 ;
         d4->recordBlank = d4->groupRecordAlloc + 2 * (recWidth2) ;
         d4->fields = (FIELD4 *)(d4->groupRecordAlloc + 3 * ( recWidth2 ) ) ;
      }
   }

   if ( d4->groupRecordAlloc == 0 )
   {
      d4->record = (char *)u4allocFree( c4, recWidth + 1 ) ;
      d4->recordOld = (char *)u4allocFree( c4, recWidth + 1 ) ;
      d4->recordBlank = (char *)u4allocFree( c4, recWidth + 1 ) ;
      d4->fields = (FIELD4 *)u4allocFree( c4, sizeof( FIELD4 ) * (long)d4->dataFile->nFields ) ;
   }

   if ( d4->record == 0 || d4->recordOld == 0 || d4->fields == 0 || d4->recordBlank == 0 )
      return error4stack( c4, e4memory, E94301 ) ;

   recOffset = 1 ;

   #ifdef S4CFOX
      nullCount = 0 ;
   #endif

   if ( !( error4code( c4 ) < 0 ) )
      for ( iFields = 0 ; iFields < d4->dataFile->nFields ; iFields++ )
      {
         image = (FIELD4IMAGE *)( info + iFields * 32 ) ;
         u4ncpy( d4->fields[iFields].name, image->name, sizeof( d4->fields->name ) ) ;

         u4ncpy( fieldBuf, &image->type, 2 ) ;
         c4upper( fieldBuf ) ;
         d4->fields[iFields].type = *fieldBuf ;
         fieldType = d4->fields[iFields].type ;
         #ifdef S4CFOX
            if ( d4version( d4 ) == 0x30 )  /* FOX 3.0 */
            {
               d4->fields[iFields].null = ( image->nullBinary & 0x02 ) ? 1 : 0 ;
               if ( d4->fields[iFields].null == 1 )
               {
                  d4->fields[iFields].nullBit = nullCount ;
                  nullCount++ ;
               }
               if ( image->nullBinary & 0x04 )
                  d4->fields[iFields].binary = 1 ;
               else
               {
                  if ( fieldType == r4memo || fieldType == r4gen )  /* memo fields are also stored binary */
                     d4->fields[iFields].binary = 2 ;
                  else
                     d4->fields[iFields].binary = 0 ;
               }
            }
         #endif

         switch( fieldType )
         {
            #ifdef S4CFOX
               case r4int:
            #endif
            case r4log:
            case r4date:
               d4->fields[iFields].len = image->len ;
               break ;
            case r4double:  /* same as r4bin */
               if ( d4version( d4 ) == 0x30 )  /* double */
               {
                  d4->fields[iFields].len = image->len ;
                  d4->fields[iFields].dec = image->dec ;
               }
               else  /* binary */
                  d4->fields[iFields].len = image->len ;
               break ;
            case r4num:
            case r4float:
            #ifdef S4CFOX
               case r4currency:
               case r4dateTime:
            #endif
               d4->fields[iFields].len = image->len ;
               d4->fields[iFields].dec = image->dec ;
               break ;
            case r4memo:
            case r4gen:
               d4->fields[iFields].len = image->len ;
               break ;
            default:
               d4->fields[iFields].len = image->len + ( image->dec << 8 ) ;
               break ;
         }

         #ifdef S4VBASIC
            d4->fields[iFields].debugInt = E4DEBUG_INT ;
         #endif
         d4->fields[iFields].offset = recOffset ;
         recOffset += d4->fields[iFields].len ;
         d4->fields[iFields].data = d4 ;
      }

   #ifndef S4OFF_MEMO
      if ( d4->dataFile->nFieldsMemo > 0 && !( error4code( c4 ) < 0 ) )
      {
         i_memo = 0 ;

         d4->fieldsMemo = (F4MEMO *)u4allocFree( c4, (long)sizeof(F4MEMO) * d4->dataFile->nFieldsMemo ) ;
         #ifdef E4STACK
            if ( d4->fieldsMemo == 0 )
               error4stack( c4, e4memory, E94301 ) ;
         #endif
         if ( d4->fieldsMemo != 0 )
            for ( iFields = 0 ; iFields < d4->dataFile->nFields ; iFields++ )
            {
               fieldType = d4->fields[iFields].type ;
               if ( fieldType == r4memo || fieldType == r4gen || ( fieldType == r4bin && d4version( d4 ) != 0x30 && c4->oledbSchemaCreate != 1 ) )
               {
                  #ifdef E4ANALYZE
                     if ( i_memo >= d4->dataFile->nFieldsMemo )  /* means we mis-counted somewhere, so didn't allocate enough memory */
                        error4describe( c4, e4struct, E94301, name, 0, 0 ) ;
                  #endif
                  d4->fields[iFields].memo = d4->fieldsMemo+i_memo ;
                  d4->fieldsMemo[i_memo].status = 1 ;
                  d4->fieldsMemo[i_memo].field = d4->fields+iFields ;
                  i_memo++ ;
               }
            }
      }
   #endif

   if ( error4code( c4 ) < 0 )
      return -1 ;

   d4->recNum = d4->recNumOld = -1 ;

   /* set up the blank field buffer before calling d4blank() */
   /* because some field types record blanks as 0, must do field by field */
   savePtr = d4->record ;
   d4->record = d4->recordBlank ;
   d4->record[0] = ' ' ;   /* reset the deleted flag */
   for ( loop = d4numFields( d4 ) ; loop > 0 ; loop-- )
      f4blank( d4fieldJ( d4, loop ) ) ;
   d4->record = savePtr ;

   d4blank( d4 ) ;

   memcpy( d4->recordOld, d4->record, (unsigned)recWidth ) ;
   d4->recordChanged = 0 ;

   d4->record[recWidth] = 0 ;
   d4->recordOld[recWidth] = 0 ;

   #ifndef S4OFF_INDEX
         #ifdef S4CLIPPER
            if ( c4->autoOpen )
            {
               if ( d4->dataFile->userCount > 1 )  /* already open, just set up tags */
               {
                  u4namePiece( nameBuf, sizeof( nameBuf ), name, 0, 0 ) ;
                  if ( i4setup( c4, d4, nameBuf, 1 ) < 0 )
                     return -1 ;
               }
               else
               {
                  i4 = i4open( d4, 0 ) ;
                     if ( i4 == 0 )
                        return -1 ;
               }
            }
         #else
            d4->dataFile->openMdx = 0 ;
            if ( ( d4->dataFile->hasMdxMemo & 0x01 ) && c4->autoOpen )
            {
                  oldSingleOpen = c4->singleOpen ;
                  c4->singleOpen = OPEN4SPECIAL ;
               i4 = i4open( d4, 0 ) ;
                  c4->singleOpen = oldSingleOpen ;
               if ( i4 == 0 )
                  return -1 ;

               #ifdef S4MDX
                  if ( !i4->indexFile->header.isProduction )
                     i4closeLow( i4 ) ;
               #endif

               d4->dataFile->openMdx = 1 ;
            }
         #endif
   #endif  /* S4OFF_INDEX */

      c4->clientDataCount++ ;
      d4->clientId = c4->clientDataCount ;
   #ifndef S4OFF_WRITE
      #ifndef S4OFF_TRAN
            if ( code4transEnabled( c4 ) )
            {
               trans = code4trans( c4 ) ;
                  connectionId = 0L ;
               rc = u4nameCurrent( nameBuf, sizeof( nameBuf ), dfile4name( d4->dataFile ) ) ;
               if ( rc < 0 )
                  return error4stack( c4, rc, E94301 ) ;

               #ifdef S4CASE_SEN
                  rc = u4nameExt( nameBuf, sizeof( nameBuf ), "dbf", 0 ) ;
               #else
                  rc = u4nameExt( nameBuf, sizeof( nameBuf ), "DBF", 0 ) ;
               #endif

               if ( rc < 0 )
                  return error4stack( c4, rc, E94301 ) ;
               #ifdef S4MACINTOSH
                  u4getMacPath( c4, macNameBuf, 256-strlen(nameBuf) ) ;
                  strcat(macNameBuf, nameBuf ) ;
                  strcpy(nameBuf, macNameBuf);
               #endif
               rc = strlen(nameBuf) ;
               if (c4->createTemp == 1)
                  tranCode = TRAN4OPEN_TEMP ;
               else
                  tranCode = TRAN4OPEN ;
               if ( tran4set( trans, trans->currentTranStatus, -1L, connectionId, tranCode,
                    (unsigned)rc + 19, data4clientId( d4 ), data4serverId( d4 ) ) == 0 )
                  {
                     tran4putData( trans, &rc, 2 ) ;
                     tran4putData( trans, nameBuf, (unsigned)rc ) ;
                     rcl = recWidth ;
                     tran4putData( trans, &rcl, 4 ) ;
                     rc = d4numFields( d4 ) ;
                     tran4putData( trans, &rc, 2 ) ;
                     rcl = d4recCount( d4 ) ;
                     tran4putData( trans, &rcl, 4 ) ;
                     #ifndef S4MACINTOSH
                        if ( stat( nameBuf, &bufStat ) != 0 )
                           return -1 ;
                        tran4putData( trans, &bufStat.st_atime, 4 ) ;
                     #else
                        MacFile.fileParam.ioFDirIndex = 0 ;
                        MacFile.fileParam.ioNamePtr = (StringPtr)&d4->dataFile->file.macSpec.name ;
                        /* memcpy( MacName, nameBuf, sizeof(MacName) ) ;*/
                        /* CtoPstr( (char *)MacName ) ;*/  /* convert C string to Pascal string */
                        /* MacFile.fileParam.ioNamePtr = (StringPtr)&MacName ;*/
                        MacFile.fileParam.ioVRefNum = d4->dataFile->file.macSpec.vRefNum ;
                        /* MacFile.fileParam.ioVRefNum = c4->macVol ;*/
                        MacFile.fileParam.ioDirID = d4->dataFile->file.macSpec.parID ;
                        /* MacFile.fileParam.ioDirID = c4->macDir ;*/
                        if (PBHGetFInfoSync((HParmBlkPtr)&MacFile) < 0)
                           return -1 ;
                        tran4putData(trans, &MacFile.fileParam.ioFlMdDat, 4 ) ;
                     #endif
                     tran4putData( trans, &d4->dataFile->yy, 3 ) ;
                     tran4lowAppend( trans, 0, 0 ) ;
                  }
               else
                  return -1 ;
            }
      #endif /* S4OFF_TRAN */
   #endif /* S4OFF_WRITE */


      /* 07/30/96 AS --> previously just check c4 setting.  should instead
         check file read only as well (but still allow read-only override
         for server handling */
      if ( d4->dataFile->file.isReadOnly == 1 )   /* file is read-only attribute, so mark as read-only */
         d4->readOnly = 1 ;
      else
         d4->readOnly = c4getReadOnly( c4 ) ;

   #ifndef S4OFF_TRAN
         if ( code4transEnabled( d4->codeBase ) == 1 )
            d4->logVal = c4->log ;
   #endif

   return 0 ;
}

DATA4 *S4FUNCTION d4open( CODE4 *c4, const char *name )
{
   int rc ;
   char *info ;
   DATA4 *d4 ;

   E4PARM_HRET( c4 && name, E94301, NULL ) ;


   d4 = d4openInit( c4 ) ;
   if ( d4 != 0 )
   {
      d4->dataFile = dfile4open( c4, d4, name, &info ) ;
      if ( d4->dataFile == 0 )
      {
         d4close( d4 ) ;
         return 0 ;
      }

      rc = d4openConclude( d4, name, info ) ;
      if ( rc < 0 )
      {
         d4close( d4 ) ;
         return 0 ;
      }

      #ifdef ACMEDEMO
         if ( strcmp( d4alias( d4 ), "CB6DEMO" ) != 0 )
         {
            code4initUndo( c4 ) ;
            return 0 ;
         }
      #endif
   }

   return d4 ;
}

DATA4 *S4FUNCTION d4openClone( DATA4 *dataOld )
{
   DATA4 *d4 ;
   CODE4 *c4 ;
   int rc ;
   char *info ;
   #ifndef S4OFF_INDEX
      TAG4 *tagNew, *tagOld ;
         #ifdef S4CLIPPER
            TAG4 *t4, *t42 ;
         #else
            INDEX4 *i4 ;
         #endif
   #endif
      int oldSingleOpen ;

   E4PARM_HRET( dataOld, E94301, NULL ) ;

   c4 = dataOld->codeBase ;
   d4 = d4openInit( c4 ) ;
   if ( d4 == 0 )
      return 0 ;
      oldSingleOpen = c4->singleOpen ;
      c4->singleOpen = OPEN4DENY_NONE ;
   d4->dataFile = data4reopen( dataOld->dataFile, &info ) ;
   if ( d4->dataFile == 0 )
   {
         c4->singleOpen = oldSingleOpen ;
      d4close( d4 ) ;
      return 0 ;
   }
   rc = d4openConclude( d4, dfile4name( d4->dataFile ), info ) ;
      c4->singleOpen = oldSingleOpen ;
   if ( rc < 0 )
   {
      d4close( d4 ) ;
      return 0 ;
   }

   /* AS 03/03/97, need to open non-production indexes as well, esp. for relate
      module in client/server */

   #ifndef S4OFF_INDEX
         #ifdef S4CLIPPER
            for ( t4 = 0 ;; )
            {
               t4 = d4tagNext( dataOld, t4 ) ;
               if ( t4 == NULL )
                  break ;
               for( t42 = 0 ;; )   /* see if exists first (i.e. production) */
               {
                  t42 = d4tagNext( d4, t42 ) ;
                  if ( t42 == 0 ) /* open */
                  {
                     i4open( d4, t4->tagFile->file.name ) ;
                     break ;
                  }
                  if ( t42->tagFile == t4->tagFile )  /* don't open */
                     break ;
               }
            }
         #else
            for ( i4 = 0 ;; )
            {
               i4 = (INDEX4 *)l4next( &dataOld->indexes, i4 ) ;
               if ( i4 == NULL )
                  break ;
               if ( !index4isProduction( i4->indexFile ) ) /* not production, so didn't get opened */
                     i4open( d4, i4->accessName ) ;
            }
         #endif /* S4CLIPPER */
      /* now go through all the tags, and set the unique settings to the same as the old data */
      for ( tagOld = 0 ;; )
      {
         tagOld = d4tagNext( dataOld, tagOld ) ;
         if ( tagOld == 0 )
            break ;
         for ( tagNew = 0 ;; )
         {
            tagNew = d4tagNext( d4, tagNew ) ;
            if ( tagNew == 0 )   /* shouldn't happen - means an index tag got lost */
            {
               d4close( d4 ) ;
               error4( c4, e4info, E94301 ) ;
               return 0 ;
            }
            if ( tagNew->tagFile == tagOld->tagFile )
               break ;
         }
         tagNew->errUnique = tagOld->errUnique ;
      }
   #endif /* S4OFF_INDEX */

   return d4 ;
}

#ifndef S4OFF_INDEX
#endif  /* not S4OFF_INDEX */

static DATA4FILE *data4reopen( DATA4FILE *d4, char **info )
{
      #ifndef S4OFF_MULTI
         int rc ;
         #ifndef S4OFF_INDEX
            #ifdef S4CLIPPER
               TAG4FILE *t4file ;
            #else
               INDEX4FILE *i4file ;
               #ifdef E4ANALYZE
                  #ifndef S4CLIPPER
                     unsigned short int nCheck ;
                  #endif
               #endif
            #endif
         #endif
      #endif

      #ifndef S4OFF_TRAN
         DATA4 *data4 ;
         LIST4 *list ;
      #endif
   CODE4 *c4 ;

   if ( d4 == 0 )
      return 0 ;

   c4 = d4->c4 ;
      if ( d4->userCount == 0 )
      {
         #ifndef S4OFF_MULTI
            if ( d4->file.lowAccessMode != c4->accessMode )  /* need to open in updated mode */
            {
               rc = dfile4closeLow( d4 ) ;
               if ( rc != 0 )
                  return 0 ;
               #ifndef S4OFF_INDEX
                  #ifdef S4CLIPPER
                     for ( t4file = 0 ;; )
                     {
                        t4file = (TAG4FILE *)l4next( &d4->tagfiles, t4file ) ;
                        if ( t4file == 0 )
                           break ;
                        rc = tfile4close( t4file, d4 ) ;
                        if ( rc < 0 )
                           return 0 ;
                     }
                  #else
                     if ( d4->indexes.nLink != ((unsigned int)d4->hasMdxMemo & 0x01 ) )
                     {
                        for ( i4file = 0 ;; )
                        {
                           i4file = (INDEX4FILE *)l4next( &d4->indexes, i4file ) ;
                           if ( i4file == 0 )
                              break ;
                           if ( index4isProduction( i4file ) == 1 )
                              continue ;
                           #ifdef E4ANALYZE
                              nCheck = d4->indexes.nLink ;
                           #endif
                           rc = index4close( i4file ) ;
                           #ifdef E4ANALYZE
                              if ( nCheck != d4->indexes.nLink + 1 )
                              {
                                 error4describe( c4, e4result, E91102, dfile4name( d4 ), 0, 0 ) ;
                                 return 0 ;
                              }
                           #endif
                           if ( rc < 0 )
                              return 0 ;
                        }
                     }
                  #endif /* S4CLIPPER */
               #endif /* S4OFF_INDEX */
               d4 = 0 ;
            }
         #endif /* S4OFF_MULTI */
      }
      else
      {
         if ( c4->singleOpen != OPEN4DENY_NONE )   /* only one instance allowed... */
         {
               #ifndef S4OFF_TRAN
                  /* verify that data4 not on the closed data list if within a
                     transaction (which is allowed) */
                  if ( code4tranStatus( c4 ) == r4active )
                  {
                     list = tran4dataList( code4trans( c4 ) ) ;
                     for ( data4 = 0 ;; )
                     {
                        data4 = (DATA4 *)l4next( list, data4 ) ;
                        if ( data4 == 0 )
                           break ;
                        if ( data4->dataFile == d4 )
                        {
                           error4describe( c4, e4instance, E91102, dfile4name( d4 ), 0, 0 ) ;
                           return 0 ;
                        }
                     }
                     #ifdef E4ANALYZE
                        /* ensure that the datafile exists somewhere! */
                        list = &( code4trans( c4 )->closedDataFiles ) ;
                        for ( data4 = 0 ;; )
                        {
                           data4 = (DATA4 *)l4next( list, data4 ) ;
                           if ( data4 == 0 )
                           {
                              error4describe( c4, e4struct, E91102, dfile4name( d4 ), 0, 0 ) ;
                              return 0 ;
                           }
                           if ( data4->dataFile == d4 )
                              break ;
                        }
                     #endif
                  }
                  else
               #endif /* S4OFF_TRAN */
            {
               error4describe( c4, e4instance, E91102, dfile4name( d4 ), 0, 0 ) ;
               return 0 ;
            }
         }
      #ifdef E4ANALYZE
         if ( d4->info == 0 )
         {
            error4describe( c4, e4struct, E91102, dfile4name( d4 ), 0, 0 ) ;
            return 0 ;
         }
      #endif
      /* verify that the desired access level is available in terms of the actual physical open */
      #ifndef S4OFF_MULTI
            /* AS 08/21/97 Also disallow open if file is open in read-only level
               at low-level, but we want it to be open as read-write */
            if ( d4->file.isReadOnly == 1 )
               if ( c4getReadOnly( c4 ) != 1 )
                  error4describe( c4, e4instance, E84307, dfile4name( d4 ), 0, 0 ) ;
         switch( c4->accessMode )
         {
            case OPEN4DENY_NONE:
               break ;
            case OPEN4DENY_RW:
                  if ( d4->file.lowAccessMode != OPEN4DENY_RW )
                  {
                     error4describe( c4, e4instance, E84307, dfile4name( d4 ), 0, 0 ) ;
                     return 0 ;
                  }
               break ;
            case OPEN4DENY_WRITE:
                  if ( d4->file.lowAccessMode == OPEN4DENY_NONE )
                  {
                     error4describe( c4, e4instance, E84307, dfile4name( d4 ), 0, 0 ) ;
                     return 0 ;
                  }
               break ;
            default:
               {
                  error4describe( c4, e4instance, E82502, dfile4name( d4 ), 0, 0 ) ;
                  return 0 ;
               }
         }
      #endif /* S4OFF_MULTI */

      }

   if ( d4 != 0 )
   {
      d4->userCount++ ;
      *info = d4->info ;
      #ifdef E4ANALYZE
         if ( d4->nFields == 0 )
         {
            error4describe( c4, e4struct, E91102, dfile4name( d4 ), 0, 0 ) ;
            return 0 ;
         }
      #endif
      return d4 ;
   }

   return 0 ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
DATA4FILE *dfile4open( CODE4 *c4, DATA4 *data, const char *name, char **info )
{
   int rc ;
   DATA4FILE *d4 ;
   unsigned int count ;
   int iFields ;
   FIELD4IMAGE *image ;
   #ifdef E4MISC
      unsigned fieldDataLen ;
   #endif
      int len2, len3 ;
      #ifndef S4OFF_INDEX
         char indexName[258] ;
      #endif
      FILE4LONG pos, tLen ;
      #ifndef E4MISC
         unsigned fieldDataLen ;
      #endif
      char nameBuf[258] ;
      DATA4HEADER_FULL fullHeader ;
      #ifndef S4OFF_MEMO
         int hasMemo ;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91102 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_LOW
      if ( c4 == 0 || name == 0 )
      {
         error4( c4, e4parmNull, E91102 ) ;
         return 0 ;
      }
         if ( data == 0 )
         {
            error4( c4, e4parmNull, E91102 ) ;
            return 0 ;
         }
   #endif

   if ( error4code( c4 ) < 0 )
      return 0 ;

   #ifdef E4ANALYZE
      if ( c4->debugInt != 0x5281 )
      {
         error4( 0, e4result, E81301 ) ;
         return 0 ;
      }
   #endif

      u4nameCurrent( nameBuf, sizeof( nameBuf ), name ) ;
      u4nameExt( nameBuf, sizeof(nameBuf), "dbf", 0 ) ;

      #ifndef S4CASE_SEN                     /* preserve the case sensitivity for unix */
         c4upper( nameBuf ) ;
      #endif

      d4 = dfile4data( c4, nameBuf ) ;

   if ( d4 != 0 )
   {
      d4 = data4reopen( d4, info ) ;
      if ( error4code( c4 ) < 0 )
         return 0 ;
      if ( d4 != 0 )
         return d4 ;
   }


   if ( c4->data4fileMemory == 0 )
   {
      c4->data4fileMemory = mem4create( c4, c4->memStartDataFile, sizeof(DATA4FILE), c4->memExpandDataFile, 0 ) ;
      if ( c4->data4fileMemory == 0 )
      {
         error4( c4, e4memory, E91102 ) ;
         return 0 ;
      }
   }
   d4 = (DATA4FILE *)mem4alloc( c4->data4fileMemory ) ;
   if ( d4 == 0 )
   {
      error4( c4, e4memory, E91102 ) ;
      return 0 ;
   }

   d4->c4 = c4 ;
   d4->userCount = 1 ;

      #ifndef S4OFF_MEMO
         d4->memoFile.file.hand = INVALID4HANDLE ;
      #endif

      rc = file4open( &d4->file, c4, nameBuf, 1 ) ;

      if ( rc )
      {
         dfile4close( d4 ) ;
         return 0 ;
      }

   l4add( &c4->dataFileList, &d4->link ) ;

      file4longAssign( pos, 0, 0 ) ;
      if ( file4readAllInternal( &d4->file, pos, &fullHeader, sizeof( fullHeader ) ) < 0 )
      {
         dfile4close( d4 ) ;
         return 0 ;
      }

      #ifdef WORDS_BIGENDIAN
         fullHeader.numRecs = x4reverseLong( (void *)&fullHeader.numRecs ) ;
         fullHeader.headerLen = x4reverseShort( (void *)&fullHeader.headerLen ) ;
         fullHeader.recordLen = x4reverseShort( (void *)&fullHeader.recordLen ) ;
      #endif

      #ifdef S4DEMO
         if ( fullHeader.numRecs > 200L)
         {
            error4( c4, e4demo, 0 ) ;
            dfile4close( d4 ) ;
            return 0 ;
         }
      #endif

      if ( fullHeader.recordLen == 0 )  /* divide by zero */
      {
         error4describe( c4, e4data, E83805, nameBuf, dfile4name( d4 ), (char *)0 ) ;
         dfile4close( d4 ) ;
         return 0 ;
      }

      if ( c4->largeFileOffset == 0 )
      {
         /* fullHeader.recordLen is not necessarily accurate with large files */
            /* if the file is opened deny write/exclusively, and this was the
               first open, then verify that the record count matches the file
               length (i.e. to avoid data file corruption) */
            if ( c4->accessMode == OPEN4DENY_WRITE || c4->accessMode == OPEN4DENY_RW )
            {
               pos = file4lenLow( &d4->file ) ;
               file4longSubtract( &pos, fullHeader.headerLen ) ;
               file4longDivide( pos, fullHeader.recordLen ) ;
               if ( ( fullHeader.numRecs != (long)file4longGetLo( pos ) ) || ( file4longGetHi( pos ) != 0 ) )
               {
                  error4describe( c4, e4data, E83805, nameBuf, dfile4name( d4 ), (char *)0 ) ;
                  dfile4close( d4 ) ;
                  return 0 ;
               }
            }

         tLen = file4lenLow( &d4->file ) ;
         file4longSubtract( &tLen, fullHeader.headerLen ) ;
         file4longDivide( tLen, fullHeader.recordLen ) ;
         /* either error or > sizeof( long ), which we don't support (i.e. #recs > sizeof(long)), or just invalid */
         if ( fullHeader.numRecs == -1L || fullHeader.numRecs > ( 1 + (long)file4longGetLo( tLen ) ) || ( file4longGetHi( tLen ) != 0 ))
         {
            error4describe( c4, e4data, E83805, nameBuf, dfile4name( d4 ), (char *)0 ) ;
            dfile4close( d4 ) ;
            return 0 ;
         }
      }

      memcpy( (void *)&d4->version, (void *)&fullHeader.version, (4+(sizeof(S4LONG))+(sizeof(short))) ) ;

      #ifdef S4FOX
         data->codePage = fullHeader.codePage ;
      #endif

      d4->hasMdxMemo = fullHeader.hasMdxMemo ;

      fieldDataLen = fullHeader.headerLen-sizeof(fullHeader) ;
      if ( fullHeader.headerLen <= sizeof(fullHeader) )
      {
         error4describe( c4, e4data, E83805, nameBuf, dfile4name( d4 ), (char *)0 ) ;
         dfile4close( d4 ) ;
         return 0 ;
      }

      d4->info = (char *)u4allocFree( c4, (long)fieldDataLen ) ;
      d4->infoLen = fieldDataLen ;
      d4->headerLen = fullHeader.headerLen ;
      if ( d4->info == 0 )
      {
         #ifdef E4STACK
            error4stack( c4, e4memory, E91102 ) ;
         #endif
         dfile4close( d4 ) ;
         return 0 ;
      }

      file4longAssign( tLen, sizeof( fullHeader ), 0 ) ;
      if ( file4readAllInternal( &d4->file, tLen, d4->info, fieldDataLen ) < 0 )
      {
         error4describe( c4, e4data, E84306, name, 0, 0 ) ;
         dfile4close( d4 ) ;
         return 0 ;
      }

      if ( error4code( c4 ) < 0 )
      {
         dfile4close( d4 ) ;
         return 0 ;
      }

      #ifndef S4OFF_MEMO
         if ( d4->version == 0x30 )  /* visual FP 3.0 */
            hasMemo = fullHeader.hasMdxMemo & 0x02 ;
         else
            hasMemo = d4->version & 0x80 ;
         if ( hasMemo )
         {
            #ifdef S4MFOX
               #ifdef S4CASE_SEN
                  u4nameExt( nameBuf, sizeof(nameBuf), "fpt", 1 ) ;
               #else
                  u4nameExt( nameBuf, sizeof(nameBuf), "FPT", 1 ) ;
               #endif
            #else
               #ifdef S4CASE_SEN
                  u4nameExt( nameBuf, sizeof(nameBuf), "dbt", 1 ) ;
               #else
                  u4nameExt( nameBuf, sizeof(nameBuf), "DBT", 1 ) ;
               #endif
            #endif
            if ( memo4fileOpen( &d4->memoFile, d4, nameBuf ) < 0 )
            {
               dfile4close( d4 ) ;
               return 0 ;
            }
         }
      #endif

   d4->numRecs = -1L ;
   *info = d4->info ;

   /* count the number of fields */
   for ( count = 0 ; d4->info[count] != 0xD ; count += 32 )
      #ifdef E4MISC
         /* if count is > fieldDataLen, then somehow the 0xD got lost, so give error */
         if ( count > fieldDataLen )
         {
            error4describe( c4, e4data, E83805, name, dfile4name( d4 ), 0 ) ;
            return 0 ;
         }
      #endif
     ;
   d4->nFields = (int)( count / 32 ) ;

   #ifdef E4ANALYZE
      if ( d4->nFields == 0 )
      {
         error4describe( c4, e4data, E84309, name, dfile4name( d4 ), 0 ) ;
         return 0 ;
      }
   #endif

   d4->recWidth = 1 ;
   d4->nFieldsMemo = 0 ;

   for ( iFields = 0; iFields < d4->nFields; iFields++ )
   {
      image = (FIELD4IMAGE *)(((char *)*info) + iFields * 32 ) ;

      switch( image->type )
      {
         case r4memo:
         case r4gen:
            d4->nFieldsMemo++ ;
            d4->recWidth += image->len ;
            break ;
         case r4num:
         case r4float:
         case r4log:
         case r4date:
            d4->recWidth += image->len ;
            break ;
         case r5wstr:
         case r4str:
            d4->recWidth += ( image->len + (image->dec << 8) ) ;
            break ;
         case r4double:   /* r4bin and r4double the same */
            if ( d4->version == 0x30 || c4->oledbSchemaCreate == 1 )
            {
               d4->recWidth += image->len ;
            }
            else
            {
               #ifdef S4MDX
                  d4->nFieldsMemo++ ;
                  d4->recWidth += image->len ;
               #else
                  if ( d4->version != 0x30 )  /* 2.5 data files disallowed these fields */
                  {
                     dfile4close( d4 ) ;
                        error4describe( c4, e4data, E80501, name, dfile4name( d4 ), 0 ) ;
                     return 0 ;
                  }
               #endif
            }
            break ;
         #ifdef S4CFOX
            case r4currency:
            case r4int:
            case r4dateTime:
               if ( d4->version != 0x30 )  /* 2.5 data files disallowed these fields */
               {
                  dfile4close( d4 ) ;
                     error4describe( c4, e4data, E80501, name, dfile4name( d4 ), 0 ) ;
                  return 0 ;
               }
               d4->recWidth += image->len ;
               break ;
            case r4system:  /* null-fields/system field */
               if ( ( d4->version != 0x30 ) || ( memcmp( image->name, "_NullFlags", 10 ) != 0 ) )  /* not visual FP 3.0 */
               {
                  dfile4close( d4 ) ;
                     error4describe( c4, e4data, E80501, name, dfile4name( d4 ), 0 ) ;
               }
               d4->recWidth += image->len ;
               break ;
         #endif
         default:
            if ( c4->oledbSchemaCreate == 1 )
            {
               switch( image->type )
               {
                  case r4currency:
                  case r4int:
                  case r4dateTime:
                  #ifdef S5USE_EXTENDED_TYPES
                     case r5i2:
                     case r5ui2:
                     case r5ui4:
                  #endif
                     d4->recWidth += image->len ;
                     break ;
                  default:
                     dfile4close( d4 ) ;
                        error4describe( c4, e4data, E80501, name, dfile4name( d4 ), 0 ) ;
                     return 0 ;
            }  }
            else
            {
               dfile4close( d4 ) ;
                  error4describe( c4, e4data, E80501, name, dfile4name( d4 ), 0 ) ;
               return 0 ;
      }   }   }


      d4->valid = 1 ;   /* valid, so low closes will leave open. */

   #ifndef S4OFF_OPTIMIZE
      file4optimizeLow( &d4->file, c4->optimize, OPT4DBF, d4->recWidth, d4 ) ;
   #endif

   return d4 ;
}

#ifdef S4VB_DOS

DATA4 * d4open_v( CODE4 *c4, char *name )
{
   return d4open( c4, c4str(name) ) ;
}

#endif

