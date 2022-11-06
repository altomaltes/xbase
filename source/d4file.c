/* d4file.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

/* fromDisk set to 1 if ensure that a disk read is done, instead of a buffer read */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
int dfile4read( DATA4FILE *data, long recNum, char *ptr, int fromDisk )
{
      unsigned len ;

      #ifdef E4PARM_LOW
         if ( data == 0 || recNum <= 0 || ptr == 0 )
            return error4( 0, e4parm_null, E91102 ) ;
      #endif

      if ( error4code( data->c4 ) < 0 )
         return e4codeBase ;

      #ifndef S4OPTIMIZE_OFF
         /* make sure read from disk unless file locked, etc. */
         if ( fromDisk )
            if ( data->file.doBuffer )
               data->c4->opt.forceCurrent = 1 ;
      #endif

      len = file4readInternal( &data->file, dfile4recordPosition( data, recNum ), ptr, data->recWidth ) ;
      #ifndef S4OPTIMIZE_OFF
         if ( fromDisk )
            if ( data->file.doBuffer )
               data->c4->opt.forceCurrent = 0 ;
      #endif

      if ( error4code( data->c4 ) < 0 )
         return error4code( data->c4 ) ;

      if ( len != data->recWidth )
         return r4entry ;

      return 0 ;
}

/* set serverId to -2 to get the actual count if possible for example,
   b4leafInit needs to know how many may potentially exist */
#ifdef P4ARGS_USED
   #pragma argsused
#endif
long S4FUNCTION dfile4recCount( DATA4FILE *data, const long serverId )
{
      unsigned len ;
      FILE4LONG pos ;
   S4LONG tmpCount ;

   #ifdef E4PARM_HIGH
      /* PARM_HIGH because called directly in S4OFF_MULTI case */
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   if ( error4code( data->c4 ) < 0 )
      return e4codeBase ;

   /* client checks current count in d4recCount */
      if ( data->numRecs >= 0L )
      {
         #ifndef S4SINGLE
            if ( serverId == -2L )
            {  return data->numRecs ;
            }

            if ( dfile4lockTestAppend( data, 0L, serverId ) != 1 )
            {  return data->minCount ;
            }
            else
         #endif
               return data->numRecs ;
      }

      file4longAssign( pos, 4, 0 ) ;
      len = file4readInternal( &data->file, pos, &tmpCount, sizeof(S4LONG) ) ;
      #ifdef S4BYTE_SWAP
         tmpCount = x4reverseLong((void *)&tmpCount) ;
      #endif
      if ( tmpCount < 0L || len != sizeof( S4LONG ) )
         return -1L ;

      #ifndef S4SINGLE
         if ( dfile4lockTestAppend( data, 0L, 0L ) )
      #endif  /* S4SINGLE */
            data->numRecs = tmpCount ;

      /* this minCount is used as the actual record count in instances where
         transactions are taking place and the append bytes are locked, and
         data handles of other datafiles are performing the access */
      data->minCount = tmpCount ;    /* used for multi-user ensured sequencing */

   return tmpCount ;
}

#ifndef S4INLINE
unsigned long dfile4recordPosition( DATA4FILE *data, const long rec )
{
   #ifdef E4PARM_LOW
      if ( data == 0 || rec <= 0L )
         return error4( 0, e4parm, E91102 ) ;
   #endif

   return (unsigned long)data->headerLen + (unsigned long)data->recWidth * ( rec - 1 ) ;
}

unsigned int dfile4recWidth( DATA4FILE *data )
{
   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   return (unsigned int)data->recWidth ;
}
#endif /* !S4INLINE */

S4CONST char *dfile4name( S4CONST DATA4FILE *data )
{
   #ifdef E4PARM_LOW
      if ( data == 0 )
      {
         error4( 0, e4parm_null, E93205 ) ;
         return 0 ;
      }
   #endif
      return data->file.name ;
}

FILE4LONG dfile4recordPosition( DATA4FILE *d4, long rec )
{
   FILE4LONG val ;

   file4longAssign( val, (unsigned long)(d4)->recWidth, 0 ) ;
   file4longMultiply( val, ( (rec) - 1 ) ) ;
   file4longAdd( &val, (d4)->headerLen ) ;

   return val ;
}

#ifndef S4OFF_WRITE
int dfile4updateHeader( DATA4FILE *data, int doTimeStamp, int doCount )
{
   FILE4LONG pos ;
   unsigned len ;

   if ( code4trans( data->c4 )->currentTranStatus == r4active || code4trans( data->c4 )->currentTranStatus == r4rollback )   /* delay to avoid append rollback problems */
      return 0 ;

   #ifdef E4PARM_LOW
      if ( data == 0 )
         return error4( 0, e4parm_null, E91102 ) ;
   #endif

   #ifdef E4ANALYZE
      #ifndef S4SINGLE
            /* note that the server doesn't need it locked since that is a data level, not
               a data4file level lock for server --> can't make this check */
            if  ( doCount && ( dfile4lockTestAppend( data, 0, 0 ) == 0 ) )
               return error4( data->c4, e4info, E83201 ) ;
      #endif
   #endif

   len = 4 + ( sizeof(S4LONG) ) + ( sizeof( short ) ) ;
   if ( doTimeStamp )
   {
      data->doDate = 1 ;
      file4longAssign( pos, 0, 0 ) ;
   }
   else
   {
      file4longAssign( pos, 4, 0 ) ;
      len -= 4 ;
   }

   if ( !doCount || data->numRecs < 0 )
      len -= (sizeof( data->numRecs ) + sizeof( data->headerLen ) ) ;

   #ifdef S4BYTE_SWAP
      data->numRecs = x4reverseLong( (void *)&data->numRecs ) ;
      data->headerLen = x4reverseShort( (void *)&data->headerLen ) ;
   #endif
      if ( file4writeInternal( &data->file, pos, (char *)&data->version + file4longGetLo( pos ), len ) < 0 )
         return -1 ;
   #ifdef S4BYTE_SWAP
      data->numRecs = x4reverseLong( (void *)&data->numRecs ) ;
      data->headerLen = x4reverseShort( (void *)&data->headerLen ) ;
   #endif

   if ( doCount )
      data->minCount = data->numRecs ;
   data->fileChanged = 0 ;
   return 0 ;
}
#endif  /* !S4OFF_WRITE */
