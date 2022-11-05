/* f4seq.cpp/cxx (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.hpp"
#ifdef __TURBOC__
   #pragma hdrstop
#endif  /* __TUROBC__ */

#include "d4data.hpp"

//File4seqRead &File4seqRead::operator>>( Str4 &s )
//{
//   unsigned num_read = read( s.ptr( ), s.len( ) ) ;
//   if( num_read < s.len( ) )
//      s.setLen( num_read ) ;
//   return *this ;
//}

//File4seqWrite& File4seqWrite::operator<<( const long l )
//{
//   char buf[40] ; char *p = &buf[37] ;
//   c4ltoa45( l, buf, 39 ) ;  buf[39] = 0 ;
//   for( int i = 37; buf[i] != ' '; i-- )
//      p-- ;
//   return operator<<( ++p ) ;
//}
