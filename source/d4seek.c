/* d4seek.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#ifndef S4INDEX_OFF

#ifdef S4FOX
/* because of possibilities of null, key may need to have a character
   indicater added */
static int tfile4stok( TAG4FILE *t4, char *buf, const char *str, int len )
{
   char *ptr = buf ;
   int hasNull = 0 ;

   if ( t4->indexFile->dataFile->version == 0x30 )
      if ( expr4nullLow( t4->expr, 0 ) )
      {
         if ( len == 0 )   /* seek for null */
         {
            *ptr = 0 ;
            return 1 ;
         }
         *ptr = (char)0x80 ;
         ptr++ ;
         hasNull = 1 ;
      }

   #ifdef S4VFP_KEY
      if ( tfile4type( t4 ) == r4str && tfile4vfpKey( t4 ) )
      {
         if ( len*2 > t4->header.keyLen )
         {
            #ifdef E4ANALYZE
               if ( (t4->header.keyLen-hasNull)%2 == 1 )
                  return error4( 0, e4index, E82107 ) ;
            #endif
               len = (t4->header.keyLen-hasNull) / 2 ;
         }
         len = t4strToVFPKey( ptr, str, len, t4->header.keyLen, &t4->vfpInfo ) ;
      }
      else
         (*t4->stok)( ptr, str, len ) ;
   #else
      (*t4->stok)( ptr, str, len ) ;
   #endif

   return len + hasNull ;
}

static void tfile4dtok( TAG4FILE *t4, char *buf, const double dkey )
{
   if ( t4->indexFile->dataFile->version == 0x30 )
      if ( expr4nullLow( t4->expr, 0 ) )
      {
         buf[0] = (char)0x80 ;
         (*t4->dtok)( buf+1, dkey ) ;
         return ;
      }

   (*t4->dtok)( buf, dkey ) ;
   return ;
}
#else
   #define tfile4stok( t4, buf, str, len ) ((*((t4)->stok))( (buf), (str), (len) ), len)
   #define tfile4dtok( t4, buf, str ) (*((t4)->dtok))( (buf), (str) )
#endif /* S4FOX */

/* function to ensure the integrity of the seeks return value */
/* in single user mode, exclusive, or if file is locked, the data is assumed up to date, and not checked */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
static int d4seekCheck( DATA4 *data, TAG4FILE *tag, const int rc, const char *buf, const int l )
{
   int rc2, len ;
   #ifndef S4SINGLE
      int skipped ;
      unsigned char *dbfKey ;
   #endif

   #ifndef S4SINGLE
      if ( rc == r4locked )
         return r4locked ;
      skipped = 0 ;
   #endif

   len = l ;

   if ( tfile4eof( tag ) )
      return d4goEof( data ) ;

   if ( len > tag->header.keyLen )
      len = tag->header.keyLen ;

   if ( d4recCountLessEq( data, tfile4recNo( tag ) ) == 0 )  /* past eof */
      while ( d4recCountLessEq( data, tfile4recNo( tag ) ) == 0 )  /* past eof */
      {
         if ( tfile4skip( tag, 1L ) == 0 )  /* eof */
            return d4goEof( data ) ;
         #ifndef S4SINGLE
            skipped = 1 ;
         #endif
      }

   #ifndef S4SINGLE
         if ( d4lockTestFile( data ) )
      {
   #endif
      #ifndef S4OFF_OPTIMIZE
         data->dataFile->hiPrio = 1 ;
      #endif
      rc2 = d4go( data, tfile4recNo( tag ) ) ;
      #ifndef S4OFF_OPTIMIZE
         data->dataFile->hiPrio = 0 ;
      #endif
      if ( rc2 )
         return rc2 ;
      return rc ;
   #ifndef S4SINGLE
      }
      for( ;; )
      {
         if ( d4recCountLessEq( data, tfile4recNo( tag ) ) != 0 )  /* valid spot */
         {
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = 1 ;
            #endif
            rc2 = d4go( data, tfile4recNo( tag ) ) ;
            #ifndef S4OFF_OPTIMIZE
               data->dataFile->hiPrio = 0 ;
            #endif
            if ( rc2 )
               return rc2 ;

            if ( expr4context( tag->expr, data ) < 0 )
               return -1 ;
            if ( tfile4exprKey( tag, &dbfKey ) < 0 )
               return -1 ;
            #ifdef S4FOX
               if ( !u4keycmp( tfile4key( tag ), dbfKey, (unsigned)expr4keyLen( tag->expr ), (unsigned)tag->header.keyLen, 0, &tag->vfpInfo ) )   /* matched */
            #else
               if ( !(*tag->cmp)( tfile4key( tag ), dbfKey, (unsigned)expr4keyLen( tag->expr ) ) )  /* matched */
            #endif
            {
               if ( skipped )
               {
                  #ifdef S4FOX
                     rc2 = u4keycmp( dbfKey, buf, (unsigned)len, (unsigned)expr4keyLen( tag->expr ), 0, &tag->vfpInfo ) ;
                  #else
                     rc2 = (*tag->cmp)( dbfKey, buf, (unsigned)expr4keyLen( tag->expr ) ) ;
                  #endif
                  if ( rc2 == 0 )   /* worked */
                     return rc2 ;
                  if ( rc2 > 0 )
                     return r4after ;
                  /* other wise, if < 0, can't return r4after, so go down and skip next */
               }
               else
                  return rc ;
            }
         }

         /* try next record */
         if ( tfile4skip( tag, 1L ) == 0 )  /* eof */
            return d4goEof( data ) ;
         if ( error4code( data->codeBase ) < 0 )
            return -1 ;
         skipped = 1 ;
      }
   #endif
}

int S4FUNCTION d4seek( DATA4 *data, const char *str )
{
   #ifdef E4PARM_HIGH
      if ( data == 0 || str == 0 )
         return error4( 0, e4parm_null, E92907 ) ;
   #endif

      return d4seekN( data, str, (short)strlen( str ) ) ;
}

#ifndef S4CB51
int S4FUNCTION d4seekNext( DATA4 *data, const char *str )
{
   #ifdef E4PARM_HIGH
      if ( data == 0 || str == 0 )
         return error4( 0, e4parm_null, E92908 ) ;
   #endif

      return d4seekNextN( data, str, (const short)strlen( str ) ) ;
}

int S4FUNCTION d4seekNextN( DATA4 *data, const char *str, const short l )
{
   CODE4 *c4 ;
   int rc ;
      #ifndef S4OFF_MULTI
         int oldErrGo ;
      #endif
      int len, rc2, saveGo ;
      TAG4 *tag ;
      TAG4FILE *tfile ;
      char buf[I4MAX_KEY_SIZE] ;
      unsigned char *dbfKey ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92905 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 || str == 0 )
         return error4( 0, e4parm_null, E92905 ) ;
   #endif

      len = l ;
      c4 = data->codeBase ;
      if ( c4 == 0 )
         return e4info ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      tag = d4tagDefault( data ) ;
      if ( tag == 0 )
         return r4noTag ;

      #ifndef S4OFF_WRITE
         /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      #endif

      tfile = tag->tagFile ;

      #ifdef __unix__
         #ifdef S4MDX
            switch ( tfile->keyType )
            {
               case r4num:
                  c4bcdFromA( buf, str, len ) ;
                  break ;
               case r4date:
                  t4strToDateMdx( buf, str, len ) ;
                  break ;
               case r4str:
                  t4noChangeStr( buf, str, len ) ;
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4FOX
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
                  t4strToFox( buf, str, len ) ;
                  break ;
               case r4date:
               case r4dateDoub:
                  t4dtstrToFox( buf, str, len ) ;
                  break ;
               case r4str:
                  t4noChangeStr( buf, str, len ) ;
                  break ;
               case r4log:
                  t4strToLog( buf, str, len ) ;
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4CLIPPER
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
                  t4strToClip( buf, str, len ) ;
                  break ;
               case r4date:
               case r4dateDoub:
               case r4str:
                  t4noChangeStr( buf, str, len ) ;
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
      #else
         #ifdef S4FOX
            /* fox version requires to see if null */
            if ( d4version( data ) == 0x30 )
               if ( expr4context( tag->tagFile->expr, data ) < 0 )
                  return -1 ;
         #endif
         len = tfile4stok( tag->tagFile, buf, str, len ) ;
      #endif

      if ( tfile4type( tfile ) != r4str && tfile4type( tfile ) != r5wstr && tfile4type( tfile ) != r4charBin )
         len = tfile->header.keyLen ;
      else
         if ( len <= 0 )
            len = strlen( str ) ;

      t4versionCheck( tag, 0, 0 ) ;

      if ( tfile4recNo( tfile ) != data->recNum )
      {
         #ifndef S4OFF_MULTI
            if ( d4lockTest( data, data->recNum ) == 0 )  /* ensure latest from disk */
            {
                  oldErrGo = data->codeBase->errGo ;
                  data->codeBase->errGo = 0 ;
               #ifndef S4OFF_OPTIMIZE
                  data->dataFile->hiPrio = 1 ;
               #endif
               saveGo = d4go( data, data->recNum ) ;
               #ifndef S4OFF_OPTIMIZE
                  data->dataFile->hiPrio = 0 ;
               #endif
                  data->codeBase->errGo = oldErrGo ;
               if ( saveGo < 0 )
                  return saveGo ;
            }
            else
         #endif
            saveGo = 0 ;

         if ( saveGo != r4entry )
         {
            if ( expr4context( tfile->expr, data ) < 0 )
               return -1 ;

            tfile4exprKey( tfile, &dbfKey ) ;

            rc = tfile4go( tfile, dbfKey, data->recNum, 0 ) ;
         }
      }
      else
      {
         saveGo = 0 ;
         if ( expr4context( tfile->expr, data ) < 0 )
            return -1 ;
         rc = tfile4exprKey( tfile, &dbfKey ) ;
      }

      if ( rc < 0 )
         return rc ;

      if ( len > tfile->header.keyLen )
         len = tfile->header.keyLen ;

      if ( saveGo == r4entry )  /* at eof or bof, so present record not match, so regular seek */
      {
         rc = tfile4seek( tfile, buf, len ) ;
         return d4seekCheck( data, tfile, rc, buf, len ) ;  /* return a valid value */
      }

      /* first check where the datafile currently is in relation to the
         seeked-for item */
      #ifdef S4FOX
         rc = u4keycmp( buf, dbfKey, len, (unsigned)expr4keyLen( tfile->expr ), 0, &tag->tagFile->vfpInfo ) ;
      #else
         rc = (*tfile->cmp)( buf, dbfKey, (unsigned)len ) ;
      #endif

      if ( rc != 0 )  /* present record not match, so regular seek */
      {
         rc = tfile4seek( tfile, buf, len ) ;
         return d4seekCheck( data, tfile, rc, buf, len ) ;  /* return a valid value */
      }

      rc = (int)tfile4dskip( tfile, 1L ) ;
      if ( rc == 0 )   /* on a valid entry, but it is last entry, so r4entry returned */
         return r4entry ;
      if ( rc < 0 )
         return rc ;
      else
         rc = 0 ;

      /* need to check the key against the seek key to see whether or not
         we have gone too far */
      #ifdef S4FOX
         /* fox version returns # matching bytes, so subtract to see if valid value */
         rc = u4keycmp( tfile4key( tfile ), buf, (unsigned)len, (unsigned)tfile->header.keyLen, 0, &tfile->vfpInfo ) ;
      #else
         rc = (*tfile->cmp)( buf, tfile4key( tfile ), (unsigned)len ) ;
      #endif
      rc2 = d4seekCheck( data, tfile, rc, buf, len ) ;  /* return a valid value */
      if ( rc != 0 )
         return r4entry ;
      return rc2 ;
}
#endif /* S4CB51 */

int S4FUNCTION d4seekN( DATA4 *data, const char *str, const short l )
{
      TAG4 *tag ;
      TAG4FILE *tfile ;
      int rc, len ;
      CODE4 *c4 ;
      char buf[I4MAX_KEY_SIZE] ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92903 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 || str == 0 )
         return error4( 0, e4parm_null, E92903 ) ;
   #endif

      len = l ;
      c4 = data->codeBase ;
      if ( c4 == 0 )
         return e4info ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      tag = d4tagDefault( data ) ;
      if ( tag == 0 )
         return r4noTag ;

      #ifndef S4OFF_WRITE
         /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      #endif

      tfile = tag->tagFile ;

      #ifdef S4FOX
         if ( d4version( data ) == 0x30 && l == 0 )  /* means seek for .NULL. */
         {
            len = tfile->header.keyLen ;
            memset( buf, 0, len ) ;
         }
         else
      #endif
      {  /* for S4FOX */
         #ifdef __unix__
            #ifdef S4MDX
               switch ( tfile->keyType )
               {
                  case r4num:
                     c4bcdFromA( buf, str, len ) ;
                     break ;
                  case r4date:
                     t4strToDateMdx( buf, str, len ) ;
                     break ;
                  case r4str:
                     if ( len > tfile->header.keyLen )
                        len = tfile->header.keyLen ;
                     t4noChangeStr( buf, str, len ) ;
                     break ;
                  default:
                     return error4( data->codeBase, e4index, E82901 ) ;
               }
            #endif
            #ifdef S4FOX
               switch ( tfile->keyType )
               {
                  case r4num:
                  case r4numDoub:
                     t4strToFox( buf, str, len ) ;
                     break ;
                  case r4date:
                  case r4dateDoub:
                     t4dtstrToFox( buf, str, len ) ;
                     break ;
                  case r4str:
                     if ( len > tfile->header.keyLen )
                        len = tfile->header.keyLen ;
                     t4noChangeStr( buf, str, len ) ;
                     break ;
                  case r4log:
                     t4strToLog( buf, str, len ) ;
                     break ;
                  default:
                     return error4( data->codeBase, e4index, E82901 ) ;
               }
            #endif
            #ifdef S4CLIPPER
               switch ( tfile->keyType )
               {
                  case r4num:
                  case r4numDoub:
                     t4strToClip( buf, str, tfile->header.keyLen ) ;
                     break ;
                  case r4date:
                  case r4dateDoub:
                  case r4str:
                     if ( len > tfile->header.keyLen )
                        len = tfile->header.keyLen ;
                     t4noChangeStr( buf, str, len ) ;
                     break ;
                  default:
                     return error4( data->codeBase, e4index, E82901 ) ;
               }
            #endif
         #else
            #ifdef S4CLIPPER
               switch ( tfile4type( tfile ) )
               {
                  case r4num:
                  case r4numDoub:
                     if ( tfile4stok( tfile, buf, str, tfile->header.keyLen ) < 0 )
                        return -1 ;
                     break ;
                  default:
                     if ( tfile4stok( tfile, buf, str, len ) < 0 )
                        return -1 ;
                     break ;
               }
            #else
               #ifdef S4FOX
                  /* fox version requires to see if null */
                  if ( d4version( data ) == 0x30 )
                     if ( expr4context( tag->tagFile->expr, data ) < 0 )
                        return -1 ;
               #endif
               len = tfile4stok( tfile, buf, str, len ) ;
            #endif
         #endif

         if ( tfile4type( tfile ) != r4str && tfile4type( tfile ) != r5wstr )
            len = tfile->header.keyLen ;
         else
            if ( len <= 0 )
               len = strlen( str ) ;
      }  /* for S4FOX */

      t4versionCheck( tag, 0, 0 ) ;
      rc = tfile4seek( tfile, buf, len ) ;

      return d4seekCheck( data, tfile, rc, buf, len ) ;  /* return a valid value */
}

int S4FUNCTION d4seekDouble( DATA4 *data, const double dkey )
{
      TAG4 *tag ;
      TAG4FILE *tfile ;
      CODE4 *c4 ;
      int rc ;
      char buf[I4MAX_KEY_SIZE] ;
      #ifdef S4CLIPPER
         int len ;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92903 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E92904 ) ;
   #endif

      c4 = data->codeBase ;
      if ( c4 == 0 )
         return e4info ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      tag = d4tagDefault( data ) ;
      if ( tag == 0 )
         return r4noTag ;

      #ifndef S4OFF_WRITE
         /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      #endif

      tfile = tag->tagFile ;

      #ifdef S4CLIPPER
         if ( tfile->dtok == 0 )
         {
            len = tfile->header.keyLen ;
            c4dtoa45( dkey, buf, len, tfile->header.keyDec ) ;
            if ( buf[0] == '*' )  /* unknown overflow result */
               return -1 ;
            c4clip( buf, len ) ;
         }
         else
      #else
         if ( tfile->dtok == 0 )
            return error4( data->codeBase, e4seek, E82902 ) ;
      #endif

      #ifdef __unix__
         #ifdef S4MDX
            switch ( tfile->keyType )
            {
               case r4num:
                  c4bcdFromD( buf, dkey ) ;
                  break ;
               case r4date:
                  t4noChangeDouble( buf, dkey ) ;
                  break ;
               case r4str:
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4FOX
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
               case r4date:
               case r4dateDoub:
                  t4dblToFox( buf, dkey ) ;
                  break ;
               case r4str:
               case r4log:
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4CLIPPER
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
               case r4str:
                  break ;
               case r4date:
               case r4dateDoub:
                  t4dateDoubToStr( buf, dkey ) ;
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
      #else
         #ifdef S4FOX
            /* fox version requires to see if null */
            if ( d4version( data ) == 0x30 )
               if ( expr4context( tag->tagFile->expr, data ) < 0 )
                  return -1 ;
         #endif
         tfile4dtok( tfile, buf, dkey ) ;
      #endif

      t4versionCheck( tag, 0, 0 ) ;
      rc = tfile4seek( tfile, buf, tfile->header.keyLen ) ;

      return d4seekCheck( data, tfile, rc, buf, tfile->header.keyLen ) ;  /* return a valid value */
}

#ifndef S4CB51
int S4FUNCTION d4seekNextDouble( DATA4 *data, const double dkey )
{
   CODE4 *c4 ;
   int rc ;
      #ifndef S4OFF_MULTI
         int oldErrGo ;
      #endif
      int rc2, saveGo ;
      TAG4 *tag ;
      TAG4FILE *tfile ;
      char buf[I4MAX_KEY_SIZE] ;
      unsigned char *dbfKey ;
      #ifdef S4CLIPPER
         int len ;
      #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92909 ) )
         return 0 ;
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E92909 ) ;
   #endif

      c4 = data->codeBase ;
      if ( c4 == 0 )
         return e4info ;
      if ( error4code( c4 ) < 0 )
         return -1 ;

      tag = d4tagDefault( data ) ;
      if ( tag == 0 )
         return r4noTag ;

      #ifndef S4OFF_WRITE
         /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
         rc = d4updateRecord( data, 0 ) ;
         if ( rc )
            return rc ;
      #endif

      tfile = tag->tagFile ;
      #ifdef S4CLIPPER
         if ( tfile->dtok == 0 )
         {
            len = tfile->header.keyLen ;
            c4dtoa45( dkey, buf, len, tfile->header.keyDec ) ;
            if ( buf[0] == '*' )  /* unknown overflow result */
               return -1 ;
            c4clip( buf, len ) ;
         }
         else
      #else
         if ( tfile->dtok == 0 )
            return error4( data->codeBase, e4seek, E82902 ) ;
      #endif

      #ifdef __unix__
         #ifdef S4MDX
            switch ( tfile->keyType )
            {
               case r4num:
                  c4bcdFromD( buf, dkey ) ;
                  break ;
               case r4date:
                  t4noChangeDouble( buf, dkey ) ;
                  break ;
               case r4str:
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4FOX
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
               case r4date:
               case r4dateDoub:
                  t4dblToFox( buf, dkey ) ;
                  break ;
               case r4str:
               case r4log:
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
         #ifdef S4CLIPPER
            switch ( tfile->keyType )
            {
               case r4num:
               case r4numDoub:
               case r4str:
                  break ;
               case r4date:
               case r4dateDoub:
                  t4dateDoubToStr( buf, dkey ) ;
                  break ;
               default:
                  return error4( data->codeBase, e4index, E82901 ) ;
            }
         #endif
      #else
         #ifdef S4FOX
            /* fox version requires to see if null */
            if ( d4version( data ) == 0x30 )
               if ( expr4context( tag->tagFile->expr, data ) < 0 )
                  return -1 ;
         #endif
         tfile4dtok( tfile, buf, dkey ) ;
      #endif

      t4versionCheck( tag, 0, 0 ) ;

      if ( tfile4recNo( tfile ) != data->recNum )
      {
         #ifndef S4OFF_MULTI
            if ( d4lockTest( data, data->recNum ) == 0 )  /* ensure latest from disk */
            {
                  oldErrGo = data->codeBase->errGo ;
                  data->codeBase->errGo = 0 ;
               #ifndef S4OFF_OPTIMIZE
                  data->dataFile->hiPrio = 1 ;
               #endif
               saveGo = d4go( data, data->recNum ) ;
               #ifndef S4OFF_OPTIMIZE
                  data->dataFile->hiPrio = 0 ;
               #endif
                  data->codeBase->errGo = oldErrGo ;
               if ( saveGo < 0 )
                  return saveGo ;
            }
            else
         #endif
            saveGo = 0 ;

         if ( saveGo != r4entry )
         {
            if ( expr4context( tfile->expr, data ) < 0 )
               return -1 ;
            rc = tfile4exprKey( tfile, &dbfKey ) ;
            if ( rc < 0 )
               return rc ;

            rc = tfile4go( tfile, dbfKey, data->recNum, 0 ) ;
         }
      }
      else
      {
         saveGo = 0 ;
         if ( expr4context( tfile->expr, data ) < 0 )
            return -1 ;
         rc = tfile4exprKey( tfile, &dbfKey ) ;
      }

      if ( rc < 0 )
         return rc ;

      if ( saveGo == r4entry )  /* at eof or bof, so present record not match, so regular seek */
      {
         rc = tfile4seek( tfile, buf, tfile->header.keyLen ) ;
         return d4seekCheck( data, tfile, rc, buf, tfile->header.keyLen ) ;  /* return a valid value */
      }

      /* first check where the datafile currently is in relation to the
         seeked-for item */
      #ifdef S4FOX
         rc = u4keycmp( dbfKey, buf, (unsigned)tfile->header.keyLen, (unsigned)expr4keyLen( tfile->expr ), 0, &tfile->vfpInfo ) ;
      #else
         rc = (*tfile->cmp)( buf, dbfKey, (unsigned)tfile->header.keyLen ) ;
      #endif

      if ( rc != 0 )  /* present record not match, so regular seek */
      {
         rc = tfile4seek( tfile, buf, tfile->header.keyLen ) ;
         return d4seekCheck( data, tfile, rc, buf, tfile->header.keyLen ) ;  /* return a valid value */
      }

      rc = (int)tfile4dskip( tfile, 1L ) ;
      if ( rc == 0 )
         return r4after ;
      if ( rc < 0 )
         return rc ;
      else
         rc = 0 ;

      /* need to check the key against the seek key to see whether or not
         we have gone too far */
      #ifdef S4FOX
         rc = u4keycmp( tfile4key( tfile ), buf, tfile->header.keyLen, (unsigned)tfile->header.keyLen, 0, &tfile->vfpInfo ) ;
      #else
         rc = (*tfile->cmp)( buf, tfile4key( tfile ), tfile->header.keyLen ) ;
      #endif
      rc2 = d4seekCheck( data, tfile, rc, buf, tfile->header.keyLen ) ;  /* return a valid value */
      if ( rc != 0 )
         return r4entry ;
      return rc2 ;
}
#endif  /* S4CB51 */

#endif  /* S4INDEX_OFF */

#ifdef S4VB_DOS

int d4seek_v( DATA4 *data, const char *seek )
{
   return d4seek( data, c4str(seek) ) ;
}

#endif
