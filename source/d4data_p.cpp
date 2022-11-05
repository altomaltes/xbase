/* d4data_p.cpp/cxx (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.hpp"
#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */

#ifndef S4CLIENT
//int Code4::optAll()
//{
//   int rc ;

//   for( Data4 data = Data4((DATA4 *)l4first( &this->dataFileList )); data.isValid();
//        data = Data4((DATA4 *)l4next( &this->dataFileList, data.data)) )
//   {
//      if( (rc = data.lockAll()) != 0 )  return rc ;
//      data.optimize( 1 ) ;
//      data.optimizeWrite( 1 ) ;
//   }
//   if( errorCode < 0 )  return -1 ;
//   if( optStart() < 0 )
//      return error( e4memory ) ;
//   return 0 ;
//}
#endif

Data4 Index4::data()
{
   DEBUG4PTR(index==0, 60704L)
   return Data4(index->data) ;
}

void Field4memo::changed()
{
   DEBUG4VOID(field == 0, E60523)
   field->data->recordChanged = 1 ;
   #ifndef S4OFF_MEMO
      if ( field->memo != 0 )
         field->memo->isChanged =  1 ;
   #endif
}

Field4memo::Field4memo() : Field4()
{
}


Field4memo::Field4memo( Data4& data, int j ) : Field4( data, j )
{
}

Field4memo::Field4memo( Data4& data, const char *name ) : Field4( data, name )
{
}

Field4memo::Field4memo( Field4 f )
{
   field = f.field ;
}


void Tag4::init( Data4 d, const char *name )
{
   if( name )
      tag = d4tag( d.dat(), name ) ;
   else
   {
      tag = d4tagSelected( d.dat() ) ;

      if( ! tag )
         tag = d4tagNext( d.dat(), NULL ) ;
   }
}

#ifndef S4OFF_REPORT
REPORT4 * S4FUNCTION  report4retrieve(Code4 &cb, char *file_name,
                      int open_files, char *pathname)
{

   char *buf, *name_buf;
   REPORT4  *retvalue;

   buf = (char *) u4allocFree( (CODE4 *) &cb, 2048 );
   if( !buf )
      return NULL;

   name_buf = (char *) u4allocFree( (CODE4 *) &cb, 512 );
   if( !name_buf )
   {
      u4free( buf );
      return NULL;
   }
   retvalue = report4retrieve2( (CODE4 *) &cb, file_name, open_files, pathname, buf, name_buf  );
   u4free( buf );
   u4free( name_buf );

   return retvalue;

}

RELATE4 * S4FUNCTION relate4retrieve( Code4 &cb, char *file_name,
                      int open_files, char *pathname )
{
   char *buf, *name_buf;
   RELATE4* retvalue;

   buf = (char *) u4allocFree( (CODE4 *) &cb, 2048 );
   if( !buf )
      return (RELATE4*)NULL;

   name_buf = (char *) u4allocFree( (CODE4 *) &cb, 512 );
   if( !name_buf )
   {
      u4free( buf );
      return (RELATE4*)NULL;
   }

   retvalue = relate4retrieve2( (CODE4 *) &cb, file_name, open_files, pathname, buf, name_buf  );

   u4free( buf );
   u4free( name_buf );

   return retvalue;

}

int S4FUNCTION relate4save(Relate4set &relSet, char *file_name,
                      int save_paths )
{
   char *buf, *name_buf;
   int retvalue;

   buf = (char *) u4allocFree( relSet.relate->codeBase, 2048 );
   if( !buf )
      return -1;

   name_buf = (char *) u4allocFree( relSet.relate->codeBase, 512 );
   if( !name_buf )
   {
      u4free( buf );
      return -1;
   }

   retvalue = relate4save2( relSet.relate, file_name, save_paths, buf, name_buf  );

   u4free( buf );
   u4free( name_buf );

   return retvalue;

}
#endif /* NOT S4OFF_REPORT */

#ifndef S4OFF_ENFORCE_LOCK
int Field4::lockCheck( )
{
   DEBUG4INT(field == 0, E60201)
   if( field->data->codeBase->accessMode!=OPEN4DENY_NONE)
     return r4success;
   if( field->data->codeBase->lockEnforce == FALSE || field->data->recNum <= 0L )
      return r4success ;
   // Check to see if the record is locked.
   if( d4lockTest( field->data, d4recNo( field->data ) ))
      return r4success ;
   return error4( field->data->codeBase, e4lock, E60201 ) ;

}
#endif
