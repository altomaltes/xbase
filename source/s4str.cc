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

/* s4str.cpp/cxx (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.hpp"

#include <math.h>
#include <ctype.h>

static char v4buffer[257] ;

#ifdef E4DEBUG
/* Ensure the strings do not overlap */
void s4asser_no_overlap( Str4& s1, Str4& s2 )
{
   if ( s1.maximum() == 0  ||  s2.maximum() == 0 )  return ;

   unsigned s1_end_pos =  s1.maximum()-1 ;
   unsigned s2_end_pos =  s2.maximum()-1 ;

   long  s1_ptr =  long( s1.ptr() ) ;
   long  s2_ptr =  long( s2.ptr() ) ;

   if ( s1_ptr == 0  ||  s2_ptr == 0 )
      error4( 0, e4parm_null, E61038 ) ;

   long  s1_end_ptr =  s1_ptr + s1_end_pos ;
   long  s2_end_ptr =  s2_ptr + s2_end_pos ;

   if ( s1_ptr     >= s2_ptr  && s1_ptr     <= s2_end_ptr      ||
        s1_end_ptr >= s2_ptr  && s1_end_ptr <= s2_end_ptr      ||
        s2_ptr     >= s1_ptr  && s2_ptr     <= s1_end_ptr      ||
        s2_end_ptr >= s1_ptr  && s2_end_ptr <= s1_end_ptr )
      error4( 0, e4parm_null, E61038 ) ;
//    e4severe( e4parm, "Overlapping Strings Detected" ) ;
}
#endif

//Tag4 Index4::tag( char *name )
//{
//   return Tag4( index->data, name ) ;
//}

Str4::operator char()
{
   if ( len() < 1 || ptr() == 0 )  return 0 ;
   return  *ptr() ;
}

Str4::operator double()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 )
         error4( 0, e4parm_null, E61002 ) ;
   #endif
   #ifdef __TURBOC__
      double d ;
      c4atod2( ptr(), len(), &d ) ;
      return d ;
   #else
     return c4atod( ptr(), len()) ;
   #endif
}

Str4::operator int()
{
   char *p =  ptr() ;
   #ifdef E4PARM_HIGH
      if( p == 0 )
         error4( 0, e4parm_null, E61003 ) ;
   #endif
   int i =  len() ;
   int   r =  (int) c4atol( p,i ) ;
   return r ;
}

Str4::operator long()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 )
         error4( 0, e4parm_null, E61004 ) ;
   #endif
   return c4atol( ptr(), len()) ;
}

int Str4::operator==( Str4& s )
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 || s.ptr() == 0 )
         error4( 0, e4parm_null, E61005 ) ;
   #endif
   unsigned l =  len() ;
   if ( l != s.len() )  return 0 ;

   if ( memcmp( ptr(), s.ptr(), l) == 0 )
      return 1 ;
   else
      return 0 ;
}

int Str4::operator< ( Str4& s )
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 || s.ptr() == 0 )
         error4( 0, e4parm_null, E61006 ) ;
   #endif
   unsigned cmp_len ;
   int      result ;
   unsigned l1 =  len() ;
   unsigned l2 =  s.len() ;

   cmp_len =  (l1<l2) ? l1 : l2 ;
   result =  memcmp( ptr(), s.ptr(), cmp_len ) ;
   if ( result == 0 )  return  l1 < l2 ;
   return result < 0 ;
}

int Str4::operator> ( Str4& s )
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 || s.ptr() == 0 )
         error4( 0, e4parm_null, E61008 ) ;
   #endif
   unsigned cmp_len ;
   int      result ;
   unsigned l1 =  len() ;
   unsigned l2 =  s.len() ;

   cmp_len =  (l1<l2) ? l1 : l2 ;
   result =  memcmp( ptr(), s.ptr(), cmp_len ) ;
   if ( result == 0 )  return  l1 > l2 ;
   return result > 0 ;
}

char& Str4::operator[] ( int index )
{
   #ifdef E4PARM_HIGH
      if( index > (int)len() )
         error4( 0, e4parm, E61010 ) ;
   #endif
   return ptr()[index] ;
}

int Str4::add( Str4& str_from )
{
   return insert( str_from, len() ) ;
}

int Str4::add( char *ptr )
{
   Str4ptr p( ptr ) ;
   return insert( p, len() ) ;
}

int Str4::at( Str4& search_str )
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 || search_str.ptr() == 0 )
         error4( 0, e4parm_null, E61015 ) ;
   #endif
   unsigned len_search =  search_str.len() ;
   if ( len_search == 0 )  return 0 ;

   unsigned str_len  =  len() ;
   if ( str_len < len_search )  return -1 ;

   unsigned last_try =  str_len - len_search ;
   char *str_ptr =  ptr() ;
   char *search_ptr =  search_str.ptr() ;

   for ( unsigned i = 0; i <= last_try; i++ )
      if ( str_ptr[i]  == *search_ptr )
      {
        unsigned j ;
         for ( j = 1; j < len_search; j++ )
            if ( str_ptr[i+j] != search_ptr[j] )  break ;
         if ( j >= len_search )  return int(i) ;
      }

   return -1 ;
}

int Str4::insert( Str4& str_from, unsigned pos )
{
   #ifdef E4DEBUG
      s4asser_no_overlap( *this, str_from ) ;
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
         return error4( 0, e4lock, E61022 ) ;
   #endif

   changed() ;

   unsigned  start_to_len =  len() ;
   #ifdef E4DEBUG
      if ( pos > start_to_len )
         error4( 0, e4parm, E61022 ) ;
   #endif
   unsigned  move_len =  start_to_len-pos ;

   unsigned  from_len =  str_from.len() ;
   long      want_to_len =  long(start_to_len) +from_len ;
   int rc ;
   if ( want_to_len > UINT_MAX )
      rc =  -1 ;
   else
      rc =  setLen( unsigned(want_to_len) ) ;

   unsigned to_len = len() ;

   if ( rc != 0 )
   {
      #ifdef E4DEBUG
         if ( to_len < start_to_len )
            error4( 0, e4info, E61022 ) ;
      #endif
      move_len  -=  unsigned( want_to_len - to_len ) ;

      unsigned space_avail =  to_len-pos ;
      if ( space_avail < from_len ) from_len =  space_avail ;
   }

   char *to_ptr =  ptr() ;
   char *to_pos =  to_ptr+pos ;
   memmove( to_pos+from_len, to_pos, move_len ) ;
   memcpy( to_pos, str_from.ptr(), from_len ) ;

   if ( to_len < maximum() )
      to_ptr[to_len] =  0 ;
   return rc ;
}

int Str4::assign( const char *from_ptr )
{
   return assign( from_ptr, strlen(from_ptr) ) ;
}

int Str4::assign( const char *from_ptr,const unsigned from_len )
{
   #ifdef E4PARM_HIGH
      if( from_ptr == 0 )
         error4( 0, e4parm_null, E61012 ) ;
   #endif
   #ifdef E4DEBUG
      Str4len str(from_ptr, from_len) ;
      s4asser_no_overlap( *this, str ) ;
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
         return error4( 0, e4lock, E61012 ) ;
   #endif

   changed() ;

   int rc = setLen( from_len ) ;
   unsigned to_len, curLen ;
   to_len =  curLen =  len() ;
   if ( to_len >= from_len )
   {
      set( ' ' ) ;
      to_len =  from_len ;
      rc = r4success;
   }

   char *p = ptr() ;
   memcpy( p, from_ptr, to_len ) ;
   if ( curLen < maximum() )  p[curLen] =  0 ;
   return rc ;
}

int Str4::assign(const Str4& str_from )
{
   const char *t=str_from.ptr1();
   const int tl=str_from.len1();
   return assign(t,tl);
   //   return assign( str_from.ptr(), str_from.len() ) ;
}

void Str4::assignDouble( double d, int new_len, int dec )
{
   char *p =  ptr() ;

   #ifdef E4PARM_HIGH
      if( p == 0 ){
        error4( 0, e4parm_null, E61013 ) ;
        return;
        }
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success ){
          error4( 0, e4lock, E61013 ) ;
          return;
          }
   #endif

   changed() ;

   if ( new_len >= 0 )
      setLen( new_len ) ;

   unsigned l = len() ;

   if ( dec < 0 )  dec =  decimals() ;
   c4dtoa45( d, p, l, dec ) ;
   if ( l < maximum() )
      p[l] =  r4success ;
}

void Str4::assignLong( long lval, int new_len, int zeros_in_front )
{
   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
      {
         error4( 0, e4lock, E61014 ) ;
         return;
      }
   #endif

   changed() ;

   if ( new_len >= 0 )
      setLen(new_len) ;

   char *p =  ptr() ;

   #ifdef E4PARM_HIGH
      if( p == 0 ){
         error4( 0, e4parm_null, E61014 ) ;
         return;
         }
   #endif

   int l = (int) len() ;

   if ( zeros_in_front )
      c4ltoa45( lval, p, -l ) ;
   else
      c4ltoa45( lval, p, l ) ;

   if ( l < (int)maximum() )
      p[l] =  0 ;
}

int Str4::encode( char *from, char *t_to, char *t_from )
{
   #ifdef E4DEBUG
      s4asser_no_overlap( *this, Str4len( from, (unsigned) strlen(t_from)) ) ;
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
         return error4( 0, e4lock, E61019 ) ;
   #endif

   changed() ;

   if( len() < strlen( t_to ) )
   {
      if( setLen( strlen( t_to)) < 0 )
         return -1;
   }
   c4encode( ptr(), from, t_to, t_from ) ;
   return 0 ;
}

char *Str4::endPtr()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 ){
         error4( 0, e4parm_null, E61020 ) ;
         return NULL;
         }
   #endif

   unsigned pos =  len() ;
   if ( pos == 0 ) return 0 ;
   pos-- ;
   return ptr()+pos ;
}

const char *Date4::format( char *pict ) const
{
   #ifdef E4PARM_HIGH
      if( pict== 0 ){
         error4( 0, e4parm_null, E61115 ) ;
         return NULL;
         }
   #endif

   date4format( ptr1(), v4buffer, pict ) ;
   return v4buffer ;
}

const Str4len Str4::left( unsigned want_len ) const
{
   return substr( 0, want_len ) ;
}

unsigned Str4::len()
{
   if ( ptr() == 0 ) return 0 ;
   return strlen(ptr()) ;
}

unsigned Str4::len1() const
{
   if ( ptr1() == 0 ) return 0 ;
   return (unsigned const)strlen(ptr1()) ;
}
void Str4::lower()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 )
         error4( 0, e4parm_null, E61025 ) ;
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
         error4( 0, e4lock, E61025 ) ;
   #endif

   changed() ;
   #ifdef S4WINDOWS
      #ifdef S4ANSI
         AnsiLower( ptr( ) ) ;
      #else
      unsigned str_len = len() ;
      char    *str_ptr =  ptr() ;
      for ( unsigned i = 0; i < str_len; i++ )
         str_ptr[i] =  tolower( str_ptr[i] ) ;
      #endif
   #else
      unsigned str_len = len() ;
      char    *str_ptr =  ptr() ;
      for ( unsigned i = 0; i < str_len; i++ )
         str_ptr[i] =  tolower( str_ptr[i] ) ;
   #endif
   return ;
}

unsigned Str4::ncpy( char *to, unsigned to_len )
{
   unsigned l = len() ;

   if ( to_len <= l )
   {
      if ( to_len-- > 0 )
      {
         memcpy( to, ptr(), to_len ) ;
         to[to_len] =  0 ;
         return to_len ;
      }
      return (unsigned) 0 ;
   }
   else
   {
      memcpy( to, ptr(), l ) ;
      to[l] =  0 ;
      return l ;
   }
}

int  Str4::replace( Str4& str, unsigned pos )
{
   #ifdef E4DEBUG
      s4asser_no_overlap( *this, str ) ;
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
         return error4( 0, e4lock, E61028 ) ;
   #endif

   changed() ;

   unsigned curLen =  len() ;
   #ifdef E4DEBUG
      if ( pos > curLen )
         return error4( 0, e4parm, E61028 ) ;
   #endif
   unsigned copy_len =  str.len() ;
   long to_final_len =  long(copy_len) + pos ;
   int rc = r4success ;
   if ( to_final_len > UINT_MAX )
      rc =  -1 ;
   else
   {
      if ( to_final_len > (long)curLen )
      {
         rc =  setLen( unsigned(to_final_len) ) ;
         curLen =  len() ;
      }
   }
   if ( rc < 0 )
      copy_len =  curLen - pos ;

   char *p = ptr() ;
   memcpy( p+pos, str.ptr(), copy_len ) ;
   if ( curLen < maximum() )  p[curLen] =  0 ;
   return rc ;
}

const Str4len Str4::right( unsigned want_len ) const
{
   unsigned curLen =  len1() ;
   if ( want_len > curLen )  want_len =  curLen ;
   return substr( curLen - want_len, want_len ) ;
}


void Str4::set( int chr_value )
{
   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success )
      {
         error4( 0, e4lock, E61030 ) ;
         return;
      }
   #endif

   char *p =  ptr() ;
   if (p)
   {
      changed() ;

      unsigned l =  len() ;
      memset( p, chr_value, l ) ;
      if ( l < maximum() )
         p[l] =  0 ;
      return ;
   }
}

const char *Str4::str()
{
   ncpy( v4buffer, sizeof(v4buffer) ) ;
   return v4buffer ;
}

void Str4::trim()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 ){
          error4( 0, e4parm_null, E61035 ) ;
          return;
          }
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success ){
         error4( 0, e4lock, E61035 ) ;
         return;
         }
   #endif

   changed() ;

   char *p =  ptr() ;
   for ( unsigned i = len();; )
   {
      if ( i == 0 )  break ;
      i-- ;

      if ( p[i] == ' ' || p[i] == 0 )
         p[i] =  0 ;
      else
      {
         setLen(i+1) ;
         return ;
      }
   }
   setLen(0) ;
   return ;
}

#ifdef S4USE_TRUE
int Str4::true()
#else
int Str4::isTrue()
#endif
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 )
         return error4( 0, e4parm_null, E61035 ) ;
   #endif

   char ch =  char(*this) ;
   if ( ch == 'Y' || ch == 'y' || ch == 'T' || ch == 't' )
      return 1 ;
   else
      return 0 ;
}

void Str4::upper()
{
   #ifdef E4PARM_HIGH
      if( ptr() == 0 ){
         error4( 0, e4parm_null, E61037 ) ;
         return;
         }
   #endif

   #ifndef S4OFF_ENFORCE_LOCK
      if( lockCheck( ) != r4success ){
         error4( 0, e4lock, E61037 ) ;
         return;
         }
   #endif

   changed() ;
   #ifdef S4WINDOWS
      #ifdef S4ANSI
         AnsiUpper( ptr( ) ) ;
      #else
      unsigned str_len = len() ;
      char    *str_ptr =  ptr() ;
      for ( unsigned i = 0; i < str_len; i++ )
         str_ptr[i] =  toupper( str_ptr[i] ) ;
      #endif
   #else
      unsigned str_len = len() ;
      char    *str_ptr =  ptr() ;
      for ( unsigned i = 0; i < str_len; i++ )
         str_ptr[i] =  toupper( str_ptr[i] ) ;
   #endif
   return ;
}

const Str4len Str4::substr( unsigned pos, unsigned result_len ) const
{
   const char *t;
   Str4len nel(0,0);
   #ifdef E4PARM_HIGH
      if( ptr1() == 0 ){
         error4( 0, e4parm_null, E61034 ) ;
         return nel;
         }
   #endif
   unsigned my_len =  len1() ;

   if ( (unsigned long)pos + result_len > (unsigned long)my_len )
   {
      if ( pos > my_len )
         result_len =  0 ;
      else
         result_len =  my_len - pos ;
   }
   t=ptr1()+pos;
   return Str4len( t, result_len ) ;
}

Str4large::Str4large()
{
   curLen = 0;
   buf[0] = 0;
   buf[255] = 0;
}

Str4large::Str4large(const char *p )
{
   curLen = 0;
   buf[255] = 0;
   assign(p);
}

Str4large::Str4large( Str4 &s )
{
   curLen = 0;
   buf[255] = 0;
   assign(s);
}

int Str4max::setLen( unsigned new_len )
{
   if ( new_len <= maxLen )
   {
      curLen =  new_len ;
      if ( curLen < maxLen )
         p[curLen] =  0 ;
      return 0 ;
   }
   if ( setMax( new_len )  < 0 )
   {
      setMax( maxLen ) ;
      return -1 ;
   }
   return setLen( new_len ) ;
}

int  Str4ten::setLen( unsigned new_len )
{
   if ( new_len <= 10 )
   {
      curLen =  new_len ;
      buf[curLen] = 0 ;
      return 0 ;
   }
   setLen( 10 ) ;
   return -1 ;
}

int Str4large::setLen( unsigned new_len )
{
   if ( new_len <= 255 )
   {
      curLen =  new_len ;
      buf[curLen] = 0 ;
      return 0 ;
   }
   setLen( 255 ) ;
   return -1 ;
}

Str4char::Str4char( char ch )
{
   c = ch ;
}

Date4::Date4()
{
   set( ' ' ) ;
   dt[8] = 0 ;
}

Date4::Date4( long l )
{
   date4assign(dt, l) ;
   dt[8] = 0 ;
}

Date4::Date4( const char *p )
{
   u4ncpy( dt, p, sizeof(dt) ) ;
}

Date4::Date4(const char *p, char *pict )
{
   date4init( dt, p, pict ) ;
}

const char *Field4::str()
{

  switch(field->type)
  {
     case r4int:
        {
          S4LONG rcLong;
          int len;

          #ifdef S4BYTE_SWAP
             rcLong=x4reverseLong(ptr());
          #else
             rcLong=*((long *)ptr());
          #endif
          len=(rcLong==0)?1:(int)(log10(fabs((double)rcLong))+(rcLong<0?2:1));
          c4ltoa45(rcLong,v4buffer,len);
          v4buffer[len]=0;
        }
        break;
     case r4double:
        if (code4indexFormat(field->data->codeBase)==r4cdx)
        {
          double rcDouble;
          int len;
          rcDouble=*((double *)ptr());
          len=(int)((rcDouble==0)?2:(log10(fabs(rcDouble))+(rcDouble<0?3:2)))+decimals();
          c4dtoa45(rcDouble,v4buffer,len,decimals());
          v4buffer[len]=0;
          break;
        }
     default :
        ncpy( v4buffer, sizeof(v4buffer) ) ;
        break;
  }
  return (v4buffer) ;
}
