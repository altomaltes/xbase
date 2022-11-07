/* f4info_p.cpp/cxx (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.hpp"
#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */

Data4 Field4::data()
{
   DEBUG4PTR(field==0, 61204L)
   return Data4(field->data) ;
}

Field4info::Field4info( Code4 &code )
{
   codeBase = &code ;
   size = 0 ;
   length = 0 ;
   field = NULL ;
}

Field4info::Field4info( Data4 d )
{
   DEBUG4VOID(d.data == 0, E60282)
   size = 0 ;
   length = 0 ;
   field = NULL ;
   codeBase = (Code4 *) d.data->codeBase ;
   add( d ) ;
}

const FIELD4INFO * Field4info::operator[] ( int index )
{
   FIELD4INFO *t ;

   DEBUG4PTR(field == 0, E60280)
   if (index > size || index < 0)
   {
      t=NULL;
   }
   else
   {
      t=&field[index];
   }
   return t ;
}

int Field4info::add( Data4 d )
{
   int i ;
   for( i = 1 ; i <= d4numFields( d.data) ; i++ )
   {
      FIELD4 * f ;
      f = d4fieldJ( d.data, i ) ;
      if( add( f4name( f ), (char)f4type( f ), (int)f4len( f ), f4decimals(f) ) < 0 )
         return -1 ;
   }
   return 0 ;
}

int Field4info::add(const char *name, char type, int len , int dec )
{

   Str4ten st_name( name ) ;
   st_name.upper( ) ;
   st_name.trim( ) ;
   if( u4allocAgain( codeBase, (char**)&field, &length, (size+2)*sizeof(FIELD4INFO)) != 0 )
      return -1 ;

   if( (field[size].name = (char *) u4allocEr( codeBase, 11 )) == 0 )
      return -1 ;

   u4ncpy( field[size].name, st_name.ptr( ), 11) ;

   field[size].type = (short int) type ;
   field[size].len = (unsigned short int) len ;
   field[size].dec = (unsigned short int) dec ;

   size++ ;
   return 0 ;
}

int Field4info::add( Field4 fp )
{
   return add( fp.name(), fp.type(), fp.len(), fp.decimals() ) ;
}

int Field4info::del( const char *name )
{
   DEBUG4INT(codeBase == 0, E60281)
   Str4ten st_name( name ) ;
   st_name.upper() ;
   st_name.trim() ;
   for( int i = 0 ; i < size ; i++ )
   {
      if( memcmp( field[i].name, st_name.ptr(), st_name.len() ) == 0 )
      {
          del( i ) ;
          return 0 ;
      }
   }
   codeBase->error( e4parm, E60281, name ) ;
   return -1 ;
}

int Field4info::del( int index )
{
   DEBUG4INT(codeBase == 0, E60281)
   if( index >= size || index < 0 )
   {
      codeBase->error( e4parm, E60281 ) ;
      return -1 ;
   }

   u4free( field[index].name ) ;
   memcpy( field+index, field+index+1, sizeof(FIELD4INFO) * (size-index) ) ;
   size-- ;
   return 0 ;
}

void Field4info::free( )
{
   for( int i = size-1 ; i >= 0 ; i-- )
      u4free( field[i].name ) ;
   size = 0 ;
   if( field )
   {
      u4free( field ) ;
      field = 0 ; length = 0 ;
   }
}

Field4info::~Field4info()
{
   free( ) ;
}

