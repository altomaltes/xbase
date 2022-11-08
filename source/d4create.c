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


/* d4create.c */

#include "d4all.h"

#ifndef S4OFF_WRITE

DATA4 *S4FUNCTION d4createTemp( CODE4 *c4, const FIELD4INFO *fieldData, const TAG4INFO *tagInfo )
{
   int oldTemp ;
   DATA4 *data ;

#ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91401 ) )
         return 0 ;
#endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( c4 == 0 || fieldData == 0 )
      {
         error4( c4, e4parmNull, E91401 ) ;
         return 0 ;
      }
   #endif

   oldTemp = c4->createTemp ;
   c4->createTemp = 1 ;
   data = d4create( c4, (char *)0, fieldData, tagInfo ) ;
   c4->createTemp = oldTemp ;

   if ( data == 0 )
   {
      #ifdef E4STACK
         error4stack( c4, e4data, E91401 ) ;
      #endif
      return 0 ;
   }

   return data ;
}

#ifdef P4ARGS_USED
   #pragma argsused
#endif
static void d4createClose( CODE4 *c4, DATA4 *d4, int doDelete )
{
   #ifndef S4OFF_TRAN
      int oldStatus ;
   #endif
   #ifdef S4CLIPPER
      TAG4FILE *tagFile ;
   #endif

   if ( d4 == 0 )
      return ;

   if ( doDelete == 1 )
   {
      d4->dataFile->file.isTemp = 1 ;   /* delete the file on close */

      /* and delete any index files */
      #ifdef S4CLIPPER
         for ( tagFile = 0 ;; )
         {
            tagFile = (TAG4FILE *)l4next( &d4->dataFile->tagfiles, tagFile ) ;
            if ( tagFile == 0 )
               break ;
            tagFile->file.isTemp = 1 ;
         }
      #endif
   }

   #ifdef S4OFF_TRAN
      d4close( d4 ) ;
   #else
      oldStatus = code4tranStatus( c4 ) ;
      code4tranStatusSet( c4, r4off ) ;
      d4close( d4 ) ;
      code4tranStatusSet( c4, oldStatus ) ;
   #endif
}

DATA4 *S4FUNCTION d4createLow( CODE4 *c4, const char *name, const FIELD4INFO *fieldData, const TAG4INFO *tagInfo )
{
      int oldReadOnly ;
      #ifndef S4OFF_TRAN
         int saveStatus ;
      #endif
   char nameBuf[258] ;
   int rc, i, oldAutoOpen ;
   DATA4FILE *d4 = NULL ;
   DATA4 *data = NULL ;
   TAG4 *tag = NULL ;

   if ( error4code( c4 ) < 0 )
      return 0 ;

   #ifndef S4OFF_TRAN
      if ( c4->logOpen )   /* open now so that can turn off during create */
      {
         rc = code4logOpen( c4, 0, 0 ) ;
         if ( rc < 0 )
            return 0 ;
         else
            error4set( c4, 0 ) ;   /* remove r4open if it already existed */
      }
   #endif

   /* AS 06/04/97 ole-db allows creation of permanent files without fixed file names */
   if ( c4->createTemp == 1 || name == NULL )
   {
      #ifndef S4OFF_CATALOG
         #ifndef CAT4CREATE
            if ( cat4avail( c4->catalog ) == 1 )
               c4->catalog->valid = 0 ;
         #endif
      #endif
   }
   else
   {
         u4nameCurrent( nameBuf, sizeof( nameBuf ), name ) ;
         u4nameExt( nameBuf, sizeof( nameBuf ), "dbf", 0 ) ;
         #ifndef S4CASE_SEN
            c4upper(nameBuf);
         #endif
         d4 = dfile4data( c4, nameBuf ) ;
      if ( d4 != 0 )
      {
         if ( c4getErrCreate( c4 ) == 0 )
            error4set( c4, r4noCreate ) ;
         else
           error4describe( c4, e4create, E81304, name, (char *)0, (char *)0 ) ;
         return 0 ;
      }
   }

      if ( c4->createTemp == 1 )
      {
         data = 0 ;
         rc = dfile4create( c4, name, fieldData, tagInfo, &data ) ;
         if ( rc < 0 )
         {
            d4createClose( c4, data, 1 ) ;
            return 0 ;
         }
      }
      else
      {
         #ifndef S4OFF_TRAN
            saveStatus = code4tranStatus( c4 ) ;
            code4tranStatusSet( c4, r4off ) ;
         #endif
         oldReadOnly = c4getReadOnly( c4 ) ;
         c4setReadOnly( c4, 0 ) ;
         if ( name == 0 )
         {
            rc = dfile4create( c4, name, fieldData, tagInfo, &data ) ;
            if ( rc < 0 )
            {
               d4createClose( c4, data, 1 ) ;
               return 0 ;
            }
            u4nameCurrent( nameBuf, sizeof( nameBuf ), data->dataFile->file.name ) ;
            d4close( data ) ;
         }
         else
            rc = dfile4create( c4, name, fieldData, tagInfo, 0 ) ;
         c4setReadOnly( c4, oldReadOnly ) ;
         #ifndef S4OFF_TRAN
            if ( saveStatus != 0 )   /* which may have resulted in transaction enabling when file created, so leave */
               code4tranStatusSet( c4, saveStatus ) ;
         #endif
         if ( rc != 0 )
            return 0 ;
         oldAutoOpen = c4->autoOpen ;
         if ( tagInfo == 0 )
            c4->autoOpen = 0 ;
         else
         {
            if ( tagInfo[0].name == 0 )
               c4->autoOpen = 0 ;
            else /* make sure on */
               c4->autoOpen = 1 ;
         }
         if ( name == NULL )
            data = d4open( c4, nameBuf ) ;
         else
            data = d4open( c4, name ) ;
         c4->autoOpen = oldAutoOpen ;
         if ( data == 0 )
            return 0 ;
      }

      /* set the unique settings... */
      if ( tagInfo != 0 )
         for ( i = 0 ; tagInfo[i].name != 0 ; i++ )
         {
            tag = d4tag( data, tagInfo[i].name ) ;
            if ( tag == 0 )
            {
               error4describe( data->codeBase, e4name, E81406, d4alias( data ), tagInfo[i].name, 0 ) ;
               return data ;
            }
            tag->errUnique = tagInfo[i].unique ;
         }

      return data ;
}

DATA4 *S4FUNCTION d4create( CODE4 *c4, const char *name, const FIELD4INFO *fieldData, const TAG4INFO *tagInfo )
{

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91401 ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef E4PARM_HIGH
      if ( c4 == 0 || fieldData == 0 )
      {
         error4( c4, e4parmNull, E91401 ) ;
         return 0 ;
      }
      /* #ifndef OLEDB5BUILD */ /* commented out for compatablity with OLEDB and library users */
         /* AS 06/04/97 ole-db allows creation of permanent files without fixed file names */
         if ( name == 0 && c4->createTemp != 1 )
         {
            error4( c4, e4parmNull, E91401 ) ;
            return 0 ;
         }
      /* #endif */
   #endif

   #ifdef E4ANALYZE
      if ( c4->debugInt != 0x5281 )
      {
         error4( c4, e4info, E81301 ) ;
         return 0 ;
      }
   #endif

   return (d4createLow(c4,name,fieldData,tagInfo));

}

#ifndef S4OFF_MEMO
   #ifndef CREATE4MEMO_EXT
      #ifdef S4MFOX
         #define CREATE4MEMO_EXT "fpt"
      #else
         #define CREATE4MEMO_EXT "mdx"
      #endif
   #endif

int dfile4create( CODE4 *c4, const char *name, const FIELD4INFO *fieldData, const TAG4INFO *tagInfo, DATA4 **temp )
{
   unsigned int nFlds ;
   int i, rc, oldAutoOpen, tempFreeSet, oldCreateTemp ;
   #ifndef S4OFF_MULTI
      int oldAccessMode ;
   #endif
   #ifndef S4OFF_MEMO
      int hasMemo ;
   #endif
   S4LONG calcRecordLen, lheaderLen ;
   char buf[258], buffer[0x800] ;
   FILE4SEQ_WRITE seqWrite ;
   DATA4 *data ;
   DATA4HEADER_FULL createHeader ;
   DATA4FILE *dfile ;
   FIELD4IMAGE createFieldImage ;
   FILE4 file ;
   char *tempName ;
   unsigned int len ;
   FILE4LONG pos ;
   #ifdef S4CLIPPER
      #ifndef S4OFF_INDEX
         char nameBuf[258] ;
      #endif
   #endif
   #ifdef E4MISC
      int dec ;
   #endif
   #ifndef S4OFF_MEMO
      int isMemo ;
      MEMO4FILE m4file ;
   #endif
   #ifndef S4OFF_TRAN
      int oldStatus = 0 ;
   #endif
   #ifdef S4FOX
      int hasNulls = 0 ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E91102 ) )
         return -1 ;
   #endif  /* S4VBASIC */

   E4PARMLOW( c4 && fieldData, E91102 ) ;
/*    if ( name == 0 && c4->createTemp != 1 )
         return error4( c4, e4parmNull, E91102 ) ; */

   if ( error4code( c4 ) < 0 )
      return e4codeBase ;

   error4set( c4, 0 ) ;  /* get rid of any positive code */

   if ( name != 0 )
   {
      u4nameCurrent( buf, sizeof( buf ), name ) ;
      u4nameExt( buf, sizeof(buf), "dbf", 0 ) ;
      /* an unused dfile may be removed to allow re-create */
      #ifndef S4CASE_SEN
         c4upper( buf ) ;
      #endif
      dfile = dfile4data( c4, buf ) ;
      if ( dfile != 0 )
      {
         rc = dfile4closeLow( dfile ) ;
         if ( rc < 0 )
            return error4stack( c4, (short)rc, E91102 ) ;
         dfile = dfile4data( c4, buf ) ;
         if ( dfile != 0 )   /* datafile already open -- can't create */
            return error4describe( c4, e4create, E81304, buf, (char *)0, (char *)0 ) ;
      }
   }

   #ifndef S4OFF_MEMO
      isMemo = 0 ;
   #endif
   calcRecordLen = 1L ;
   nFlds = 0 ;

   for ( ; fieldData[nFlds].name ; nFlds++ )
   {
      #ifdef S4FOX
         if ( c4->compatibility == 30 )
            if ( fieldData[nFlds].nulls == r4null )
               hasNulls++ ;
      #endif

      #ifndef S4OFF_MEMO
         if ( fieldData[nFlds].type == r4memo || fieldData[nFlds].type == r4gen
         #ifdef S4FOX
             || fieldData[nFlds].type == r4memoBin
         #else
             || (fieldData[nFlds].type == r4bin && c4->oledbSchemaCreate != 1 )
         #endif
            )
            isMemo = 1 ;
      #endif
      switch( fieldData[nFlds].type )
      {
         case r4num:
         case r4float:
         case r4str:
         #ifndef S4FOX
            case r4bin:
         #endif
            calcRecordLen += fieldData[nFlds].len ;
            break ;
         case r4memo:
         case r4gen:
            #ifdef S4OFF_MEMO
               #ifdef E4MISC
                  return error4( c4, e4notMemo, E91102 ) ;
               #endif
            #else
               #ifdef S4FOX
                  if ( c4->compatibility == 30 )
                     calcRecordLen += 4 ;
                  else
               #endif
                  calcRecordLen += 10 ;
               break ;
            #endif
         case r4date:
            calcRecordLen += 8 ;
            break ;
         case r4log:
            calcRecordLen += 1 ;
            break ;
         #ifdef S4FOX
            case r4memoBin:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += 4 ;
               break ;
            case r4charBin:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += fieldData[nFlds].len ;
               break ;
            case r4currency:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += 8 ;
               break ;
            case r4dateTime:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += 8 ;
               break ;
            case r4double:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += 8 ;
               break ;
            case r4int:
               if ( c4->compatibility != 30 )
                  return error4( c4, e4data, E81404 ) ;
               calcRecordLen += 4 ;
               break ;
         #endif
         default:
            if ( c4->oledbSchemaCreate == 1 )
            {
               switch( fieldData[nFlds].type )
               {
                  case r5wstr:
                     calcRecordLen += fieldData[nFlds].len ;
                     break ;
                  #ifdef S5USE_EXTENDED_TYPES
                     case r5i2:
                     case r5ui2:
                        calcRecordLen += 2 ;
                        break ;
                  #endif
                  case r4int:
                  #ifdef S5USE_EXTENDED_TYPES
                     case r5ui4:
                  #endif
                     calcRecordLen += 4 ;
                     break ;
                  case r4memoBin:
                     calcRecordLen += 4 ;
                     break ;
                  case r4charBin:
                     calcRecordLen += fieldData[nFlds].len ;
                     break ;
                  case r4currency:
                     calcRecordLen += 8 ;
                     break ;
                  case r4dateTime:
                     calcRecordLen += 8 ;
                     break ;
                  case r4double:
                     calcRecordLen += 8 ;
                     break ;
                  default:
                     return error4( c4, e4data, E81404 ) ;
               }
            }
            else
               return error4( c4, e4data, E81404 ) ;
            break ;
      }
   }

   if ( calcRecordLen >= USHRT_MAX ) /* Increment for deletion flag. */
   {
      if ( c4->largeFileOffset == 0 )  /* allow if largeFileOffset != 0 */
      {
         if ( name == 0 )
            return error4( c4, e4recordLen, E81407 ) ;
         else
            return error4describe( c4, e4recordLen, E91102, name, 0, 0 ) ;
      }
   }

   lheaderLen = (long)nFlds * 32 + 34 ;
   #ifdef S4FOX
      if ( c4->compatibility == 30 )
         if ( hasNulls > 0 )  /* extra field for null settings */
         {
            lheaderLen += 32 ;
            calcRecordLen += (hasNulls+7)/8 ;
         }
   #endif

   if ( lheaderLen >= USHRT_MAX )
   {
      if ( name == 0 )
         return error4( c4, e4recordLen, E81408 ) ;
      else
         return error4describe( c4, e4create, E81405, name, 0, 0 ) ;
   }

   tempFreeSet = 0 ;
   tempName = 0 ;

   if ( name != 0  )
   {
      if ( c4->createTemp == 1 )  /* take care of setting as temp later */
      {
         c4->createTemp = 0 ;
         rc = file4create( &file, c4, buf, 1 ) ;
         c4->createTemp = 1 ;
      }
      else
         rc = file4create( &file, c4, buf, 1 ) ;
   }
   else
   {
      oldCreateTemp = c4->createTemp ;
      c4->createTemp = 0 ;
      rc = file4tempLow( &file, c4, 0, oldCreateTemp, (char *)(oldCreateTemp ? NULL : "DBF") ) ;  /* need to set as non-remove to get name input */
      c4->createTemp = oldCreateTemp ;
      if ( rc == 0 )
      {
         if ( oldCreateTemp == 1 )
            file.isTemp = 1 ;
         if ( file.name != 0 )
         {
            len = strlen( file.name ) ;
            if ( len > sizeof( buf ) + 1 )
               len = sizeof( buf ) - 1 ;
            memcpy( buf, file.name, len ) ;
            buf[len] = 0 ;
            tempName = file.nameBuf ;
         }
         #ifdef E4DEBUG
            else
            {
               file4close( &file ) ;
               error4( c4, e4info, E91102 ) ;
            }
         #endif
         tempFreeSet = 1 ;
      }
   }

   if ( rc < 0 )   /* file creation failed */
      return error4stack( c4, (short)rc, E91102 ) ;

   if ( rc != 0 )   /* maybe r4noCreate */
      return rc ;

   file4longAssign( pos, 0, 0 ) ;

   rc = file4seqWriteInitLow( &seqWrite, &file, pos, buffer, sizeof(buffer) ) ;
   if ( rc < 0 )
   {
      file.isTemp = 1 ;   /* force the file to not be created */
      file4close( &file ) ;
      return error4stack( c4, (short)rc, E91102 ) ;
   }

   /* Write the header */
   memset( (void *)&createHeader, 0, sizeof(createHeader) ) ;
   #ifdef S4FOX
      if ( c4->compatibility == 30 )  /* 3.0 file */
      {
         createHeader.version = 0x30 ;
         #ifndef S4OFF_MEMO
            if ( isMemo )
               createHeader.hasMdxMemo |= 0x02 ;
         #endif
      }
      else
      {
         #ifndef S4OFF_MEMO
            if ( isMemo )
               createHeader.version = (char)0xF5 ;
            else
         #endif
            createHeader.version = (char)0x03 ;
      }
   #else
      #ifndef S4OFF_MEMO
         if ( isMemo )
            #ifdef S4MNDX
               createHeader.version = (char)0x83 ;
            #endif  /* S4MNDX */
            #ifdef S4MMDX
               createHeader.version = (char)0x8B ;
            #endif  /* S4MMDX */
         else
      #endif /* S4OFF_MEMO */
         createHeader.version = (char)0x03 ;
   #endif /* S4FOX */

   u4yymmdd( &createHeader.yy ) ;
   createHeader.headerLen = (unsigned short)(32*(nFlds+1) + 1) ;
   #ifdef S4FOX
      if ( c4->compatibility == 30 )  /* 3.0 file */
      {
         if ( hasNulls > 0 )  /* extra field for null settings */
            createHeader.headerLen += 32 ;
         createHeader.headerLen += 263 ;    /* visual fox reserves an extra 263 bytes */
      }
   #endif
   createHeader.recordLen = (unsigned short)calcRecordLen ;

   #ifdef S4FOX
      createHeader.codePage = (char)c4->codePage ;   /* write codepage stamp */
   #endif

   #ifdef S4BYTE_SWAP
      createHeader.numRecs = x4reverseLong( (void *)&createHeader.numRecs ) ;
      createHeader.headerLen = x4reverseShort( (void *)&createHeader.headerLen ) ;
      createHeader.recordLen = x4reverseShort( (void *)&createHeader.recordLen ) ;
   #endif  /* S4BYTE_SWAP */

   rc = file4seqWrite( &seqWrite, (char *) &createHeader, sizeof(createHeader) ) ;
   if ( rc < 0 )
   {
      file4seqWriteFlush( &seqWrite ) ;
      file.isTemp = 1 ;   /* force the file to not be created */
      file4close( &file ) ;
      return error4stack( c4, (short)rc, E91102 ) ;
   }

   #ifdef S4FOX
      calcRecordLen = 1L ;
   #endif
   for ( i = 0; i < (int)nFlds; i++ )
   {
      memset( (void *)&createFieldImage, 0, sizeof(createFieldImage) ) ;
      u4ncpy( createFieldImage.name, fieldData[i].name, sizeof(createFieldImage.name));
      c4trimN( createFieldImage.name, sizeof(createFieldImage.name) ) ;
      c4upper( createFieldImage.name ) ;

      createFieldImage.type = (char)fieldData[i].type ;
      c4upper( &createFieldImage.type ) ;
      #ifdef S4FOX
         createFieldImage.offset = (short)calcRecordLen ;
         #ifdef S4BYTE_SWAP
            createFieldImage.offset = x4reverseLong( (void *)&createFieldImage.offset ) ;
         #endif
         calcRecordLen += fieldData[i].len ;
         if ( fieldData[i].nulls == r4null )
            createFieldImage.nullBinary |= 0x02 ;
      #endif

      switch( createFieldImage.type )
      {
         case r4str:
            createFieldImage.len = (unsigned char)(fieldData[i].len & 0xFF) ;
            createFieldImage.dec = (unsigned char)(fieldData[i].len>>8) ;
            break ;
         #ifndef S4OFF_MEMO
            case r4memo:
            case r4gen:
            #ifndef S4FOX
               case r4bin:
            #endif
               #ifdef S4FOX
                  if ( c4->compatibility == 30 )  /* 3.0 file */
                     createFieldImage.len = 4 ;
                  else
                     createFieldImage.len = 10 ;
               #else
                  createFieldImage.len = 10 ;
               #endif
               createFieldImage.dec = 0 ;
               break ;
         #endif
         case r4date:
            createFieldImage.len = 8 ;
            createFieldImage.dec = 0 ;
            break ;
         case r4log:
            createFieldImage.len = 1 ;
            createFieldImage.dec = 0 ;
            break ;
         case r4num:
         case r4float:
            createFieldImage.len = (unsigned char)fieldData[i].len ;
            createFieldImage.dec = (unsigned char)fieldData[i].dec ;
            #ifdef E4MISC
               len = fieldData[i].len ;
               dec = fieldData[i].dec ;
               if ( ( len > F4MAX_NUMERIC || len < 1 || len <= 2 && dec != 0 || dec < 0 ) ||
                    ( (unsigned int)dec > len - F4DECIMAL_OFFSET  && dec > 0 ) || ( dec > F4MAX_DECIMAL ) )
                  rc = error4( c4, e4data, E81404 ) ;
            #endif
            break ;
         #ifdef S4FOX
            case r4charBin:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.type = r4str ;
                  createFieldImage.len = (unsigned char)(fieldData[i].len & 0xFF) ;
                  createFieldImage.dec = (unsigned char)(fieldData[i].len>>8) ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
            case r4memoBin:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.type = r4memo ;
                  createFieldImage.dec = 0 ;
                  createFieldImage.len = 4 ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
            case r4currency:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.len = 8 ;
                  createFieldImage.dec = 4 ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
            case r4dateTime:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.len = 8 ;
                  createFieldImage.dec = 0 ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
            case r4int:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.len = 4 ;
                  createFieldImage.dec = 0 ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
            case r4double:
               if ( c4->compatibility != 30 )
                  rc = error4( c4, e4data, E81404 ) ;
               else
               {
                  createFieldImage.len = 8 ;
                  createFieldImage.dec = (unsigned char)fieldData[i].dec ;
                  createFieldImage.nullBinary |= 0x04 ;
               }
               break ;
         #endif
         default:
            if ( c4->oledbSchemaCreate == 1 )
            {
               switch( createFieldImage.type )
               {
                  case r5wstr:
                     createFieldImage.len = (unsigned char)(fieldData[i].len & 0xFF) ;
                     createFieldImage.dec = (unsigned char)(fieldData[i].len>>8) ;
                     #ifdef S4FOX
                        createFieldImage.nullBinary |= 0x04 ;
                     #endif
                     break ;
                  case r4charBin:
                     createFieldImage.type = r4str ;
                     createFieldImage.len = (unsigned char)(fieldData[i].len & 0xFF) ;
                     createFieldImage.dec = (unsigned char)(fieldData[i].len>>8) ;
                     break ;
                  case r4memoBin:
                     createFieldImage.type = r4memo ;
                     createFieldImage.dec = 0 ;
                     createFieldImage.len = 4 ;
                     break ;
                  case r4currency:
                     createFieldImage.len = 8 ;
                     createFieldImage.dec = 4 ;
                     break ;
                  case r4dateTime:
                     createFieldImage.len = 8 ;
                     createFieldImage.dec = 0 ;
                     break ;
                  #ifdef S5USE_EXTENDED_TYPES
                     case r5i2:
                     case r5ui2:
                        createFieldImage.len = 2 ;
                        createFieldImage.dec = 0 ;
                        break ;
                  #endif
                  #ifdef S5USE_EXTENDED_TYPES
                     case r5ui4:
                  #endif
                  case r4int:
                     createFieldImage.len = 4 ;
                     createFieldImage.dec = 0 ;
                     break ;
                  case r4double:
                     createFieldImage.len = 8 ;
                     createFieldImage.dec = (unsigned char)fieldData[i].dec ;
                     break ;
                  default:
                     rc = error4( c4, e4data, E81404 ) ;
               }
            }
            else
               rc = error4( c4, e4data, E81404 ) ;
      }

      if ( rc == 0 )
      {
         rc = file4seqWrite( &seqWrite, &createFieldImage, sizeof(createFieldImage) ) ;
         if ( rc < 0 )
            break ;
      }
   }

   #ifdef S4FOX
      if ( rc == 0 )
         if ( hasNulls > 0 ) /* need to add the special field */
         {
            memset( (void *)&createFieldImage, 0, sizeof(createFieldImage) ) ;
            u4ncpy( createFieldImage.name, "_NullFlags", sizeof(createFieldImage.name));
            c4trimN( createFieldImage.name, sizeof(createFieldImage.name) ) ;

            createFieldImage.offset = (short)calcRecordLen ;
            createFieldImage.type = r4system ;
            createFieldImage.nullBinary = 0x05 ;
            createFieldImage.len = (hasNulls+7)/8 ;  /* 1 byte for every 8 nulls */
            createFieldImage.dec = 0 ;
            rc = file4seqWrite( &seqWrite, &createFieldImage, sizeof(createFieldImage) ) ;
         }
   #endif

   if ( rc == 0 )
   {
      #ifdef S4FOX
         if ( c4->compatibility == 30 )  /* 3.0 file */
         {
            rc = file4seqWriteRepeat( &seqWrite, 1, '\015' ) ;
            if ( rc == 0 )
               rc = file4seqWriteRepeat( &seqWrite, 263, '\0' ) ;
         }
         else
            rc = file4seqWrite( &seqWrite, "\015\032", 2 ) ;
      #else
         rc = file4seqWrite( &seqWrite, "\015\032", 2 ) ;
      #endif
   }

   file4seqWriteFlush( &seqWrite ) ;

   if ( rc < 0 )   /* force the file to not be created */
      file.isTemp = 1 ;
   else
      if ( tempFreeSet == 1 )
      {
         file.doAllocFree = 0 ;
         file.isTemp = 0 ;
      }

   file4close( &file ) ;

   if ( rc < 0 )
      return error4stack( c4, (short)rc, E91102 ) ;

   #ifndef S4OFF_MEMO
      hasMemo = 0 ;
      #ifdef S4FOX
         if ( c4->compatibility == 30 )
         {
            if ( createHeader.hasMdxMemo & 0x02 )
               hasMemo = 1 ;
         }
         else
            if ( createHeader.version & 0x80 )
               hasMemo = 1 ;
      #else
         if ( createHeader.version & 0x80 )
            hasMemo = 1 ;
      #endif

      if ( hasMemo )
      {
         oldCreateTemp = c4->createTemp ;
         c4->createTemp = 0 ;
         if ( name == 0 )
            rc = memo4fileCreate( &m4file, c4, 0, tempName ) ;
         else
         {
            u4nameExt( buf, sizeof(buf), CREATE4MEMO_EXT, 1 ) ;
            rc = memo4fileCreate( &m4file, c4, 0, buf ) ;
         }
         c4->createTemp = oldCreateTemp ;
         if ( rc == 0 )
            file4close( &m4file.file ) ;
      }
   #endif

   if ( rc < 0 )
   {
      if ( tempFreeSet == 1 && tempName != 0 )
         u4free( tempName ) ;
      return error4stack( c4, (short)rc, E91102 ) ;
   }

   oldAutoOpen = c4->autoOpen ;

   #ifndef S4OFF_MULTI
      oldAccessMode = c4->accessMode ;
      c4->accessMode = OPEN4DENY_RW ;
   #endif

   if ( tagInfo == 0 )
      c4->autoOpen = 0 ;
   else
   {
      if ( tagInfo[0].name == 0 )
         c4->autoOpen = 0 ;
   }
   #ifdef S4CLIPPER
      c4->autoOpen = 0 ;
   #endif

   #ifndef S4OFF_TRAN
      if ( name == 0 || c4->createTemp == 1 )
      {
         oldStatus = code4tranStatus( c4 ) ;
         code4tranStatusSet( c4, r4off ) ;
      }
   #endif

   if ( name == 0 )
      data = d4open( c4, tempName ) ;
   else
      data = d4open( c4, name ) ;

   if ( tempFreeSet == 1 && tempName != 0 )
      u4free( tempName ) ;

   if ( name == 0 || c4->createTemp == 1 )
   {
      #ifndef S4OFF_TRAN
         code4tranStatusSet( c4, oldStatus ) ;
      #endif

      if ( data != 0 )
      {
         if ( tempFreeSet == 1 )
            if ( data->dataFile->file.name != 0 )
               data->dataFile->file.doAllocFree = 1 ;
         if ( c4->createTemp == 1 )
            data->dataFile->file.isTemp = 1 ;
         #ifndef S4OFF_TRAN
            data->logVal = LOG4TRANS ;
         #endif
         #ifndef S4OFF_MEMO
            if ( createHeader.version & 0x80 )
               if ( c4->createTemp == 1 )
               {
                  #ifdef E4ANALYZE
                     if ( data->dataFile->memoFile.file.hand == INVALID4HANDLE )
                        error4( 0, e4struct, E91102 ) ;
                  #endif
                  data->dataFile->memoFile.file.isTemp = 1 ;
               }
         #endif
      }
   }

   c4->autoOpen = oldAutoOpen ;
   if ( data == 0 )
      error4( c4, e4open, E91102 ) ;
   else
   {
      #ifdef S4OFF_INDEX
         #ifdef E4MISC
            if ( tagInfo )
               error4( c4, e4notIndex, E91102 ) ;
         #endif
      #else
         if ( tagInfo )
            #ifdef S4CLIPPER
               if ( name == 0 )
               {
                  if ( i4create( data, 0, tagInfo ) == 0 )
                     if ( error4code( c4 ) != r4noCreate && error4code( c4 ) > 0 )  /* if == r4noCreate, just pass through */
                        error4( c4, e4create, E91102 ) ;
                }
               else
               {
                  /* 03/06/96 AS --> fix #25 changes.60 */
                  u4namePiece( nameBuf, sizeof( nameBuf ), name, 1, 0 ) ;
                  if ( i4create( data, nameBuf, tagInfo ) == 0 )
                     if ( error4code( c4 ) != r4noCreate && error4code( c4 ) > 0 )  /* if == r4noCreate, just pass through */
                        error4( c4, e4create, E91102 ) ;
               }
            #else
               if ( i4create( data, 0, tagInfo ) == 0 )
                  if ( error4code( c4 ) != r4noCreate && error4code( c4 ) > 0 )  /* if == r4noCreate, just pass through */
                     error4( c4, e4create, E91102 ) ;
            #endif
      #endif
   }

   #ifndef S4OFF_MULTI
      c4->accessMode = oldAccessMode ;
   #endif
   rc = error4code( c4 ) ;
   if ( rc != 0 || data == 0 )
      d4createClose( c4, data, 1 ) ;
   else
   {
      if ( ( c4->createTemp != 1 && name != 0 ) || temp == 0 )  /* if temp is NULL must close even if temporary */
      {
         dfile = data->dataFile ;
         d4createClose( c4, data, 0 ) ;
      }
      else
         *temp = data ;
   }

   return error4code( c4 ) ;
}
#endif  /*  */
#endif  /* S4OFF_WRITE */

#ifdef S4VB_DOS

DATA4 *d4create_v ( CODE4 *c4 , char *name, FIELD4INFO *f4, TAG4INFO *t4 )
{
   return d4create( c4, c4str(name), f4, t4 ) ;
}

DATA4 *d4createData ( CODE4 *c4, char *name, FIELD4INFO *f4 )
{
   return d4create( c4, c4str(name), f4, 0 ) ;
}

#endif
