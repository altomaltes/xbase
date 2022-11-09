/* d4top.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

int S4FUNCTION d4top( DATA4 *data )
{
   int rc ;
   CODE4 *c4 ;
   #ifndef S4OFF_INDEX
      long recno ;
      TAG4 *tag ;
      TAG4FILE *tagFile ;
   #endif
      int saveFlag ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E92301 ) )
         return -1;
   #endif  /* S4VBASIC */

      E4PARHIGH( data, E92301 ) ;

   c4 = data->codeBase ;
   if ( error4code( c4 ) < 0 )
      return -1 ;

   #ifndef S4OFF_INDEX
      tag = data->tagSelected ;

      if ( tag == 0 )
      {
   #endif
      if ( d4recCount( data ) > 0L )
      {
            saveFlag = c4->errGo ;
            c4->errGo = 0 ;
         rc = d4go( data, 1L ) ;
            c4->errGo = saveFlag ;
         return rc ;
      }
   #ifndef S4OFF_INDEX
      }
      else
      {
         tagFile = tag->tagFile ;
         #ifndef S4OFF_WRITE
            /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
            rc = d4updateRecord( data, 0 ) ;
            if ( rc )
               return rc ;
         #endif
         t4versionCheck( tag, 0, 0 ) ;
         rc = tfile4top( tagFile ) ;
         if ( rc )
            return rc ;
         if ( !tfile4eof( tagFile ) )
         {
            recno = tfile4recNo( tagFile ) ;
            if ( recno < 0 )
               return (int)recno ;
            rc = d4go( data, recno ) ;
            #ifndef S4OFF_MULTI
               #ifndef S4OFF_TRAN
                  if ( rc == 0 )
                     if ( code4transEnabled( c4 ) )
                        if ( t4unique( tag ) != 0 )
                              if ( !d4lockTestFile( data ) )
                              rc = d4tagSyncDo( data, tag, 1 ) ;
               #endif
            #endif
            return rc ;
         }
      }
   #endif

   data->bofFlag = 1 ;
   return d4goEof( data ) ;
}


int S4FUNCTION d4goBof( DATA4 *data )
{
   int rc ;

      E4PARHIGH( data, E93104 ) ;

   if ( error4code( data->codeBase ) < 0 )
      return e4codeBase ;

   #ifndef S4OFF_WRITE
      /* AS 04/22/97 causes problems with t4seek, unlock reset to 0 */
      rc = d4updateRecord( data, 0 ) ;
      if ( rc )
         return rc ;
   #endif

   rc = d4top( data ) ;
   data->bofFlag = 1 ;
   if ( rc < 0 )
      return rc ;

   return r4bof ;
}
