/* d4remove.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

int S4FUNCTION d4remove( DATA4 *data )
{
   int rc ;
   CODE4 *c4 ;

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E96401 ) ;
   #endif

   /* for c/s, server takes care of determining this info... */
   #ifdef S4STAND_ALONE
      #ifndef S4OFF_TRAN
         if ( code4transEnabled( data->codeBase ) )
            if ( tran4active( data->codeBase, data ) != 0 )
               return error4( data->codeBase, e4transViolation, E81521 ) ;
      #endif
   #endif

   c4 = data->codeBase;
   c4setDoRemove( c4, 1 ) ;
   rc = d4close( data ) ;
   c4setDoRemove( c4, 0 ) ;

   return rc ;
}

#ifdef S4SERVER
int d4clearTables( DATA4FILE *data )
{
   CODE4 *c4 ;
   #ifndef S4OFF_CATALOG
      TAG4 *tag ;
      CATALOG4 *catalog ;
      #ifndef S4OFF_SECURITY
         TAG4 *secTag ;
         DATA4 *table ;
      #endif
   #endif

   c4 = data->c4 ;

   #ifndef S4OFF_CATALOG
      catalog = c4->catalog ;
      if ( catalog != 0 )
         if ( catalog->catalogStatus )
         {
            tag = d4tag( catalog->data, "PATH" ) ;
            if ( tag == 0 )
               return error4( c4, e4info, E81403 ) ;
            d4tagSelect( catalog->data, tag ) ;
            #ifndef S4OFF_SECURITY
               table = c4->server->tableAuth ;
               if ( table != 0 )
               {
                  secTag = d4tag( table, "ALIAS" ) ;
                  if ( secTag == 0 )
                     return error4( c4, e4info, E70220 ) ;
                  d4tagSelect( table, secTag ) ;
               }
            #endif
            for ( ;; )
            {
               if ( error4code( c4 ) != 0 )
                  return -1 ;
               if ( d4seek( catalog->data, data->file.name ) == 0 )
               {
                  #ifndef S4OFF_SECURITY
                     if ( table != 0 )
                     {
                        if ( d4seek( table, cat4alias( catalog ) ) == 0 )
                        {
                           d4delete( table ) ;
                           d4update( table ) ;
                        }
                     }
                  #endif
                  d4delete( catalog->data ) ;
                  d4update( catalog->data ) ;
               }
            }
         }
   #endif

   return 0 ;
}
#endif

int S4FUNCTION dfile4remove( DATA4FILE *data )
{
   int rc ;
   #ifndef S4INDEX_OFF
      #ifdef N4OTHER
         TAG4FILE *tagOn ;
      #else
         INDEX4FILE *indexOn ;
      #endif
   #endif

   #ifdef E4PARM_HIGH
      if ( data == 0 )
         return error4( 0, e4parm_null, E96401 ) ;
   #endif

   if ( data->userCount != 0 )
      return error4( 0, e4parm_null, E86402 ) ;

   #ifdef S4SERVER
      /* remove entries in all tables */
      d4clearTables( data ) ;
   #endif

   #ifndef S4INDEX_OFF
      #ifdef N4OTHER
         for( tagOn = 0;; )
         {
            tagOn = dfile4tagNext( data, tagOn ) ;
            if ( tagOn == 0 )
               break ;

            tagOn->file.isTemp = 1 ;
         }
      #else
         for( indexOn = 0 ;; )
         {
            indexOn = (INDEX4FILE *)l4next( &data->indexes, indexOn ) ;
            if ( indexOn == 0 )
               break ;

            indexOn->file.isTemp = 1 ;
         }
      #endif
   #endif

   data->file.isTemp = 1 ;

   #ifndef S4MEMO_OFF
      if (data->nFieldsMemo )
         data->memoFile.file.isTemp = 1 ;
   #endif

   rc = dfile4closeLow( data ) ;

   return rc ;
}
