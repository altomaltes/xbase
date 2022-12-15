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

#include "xbase.fox/d4all.h"



void main( void )
{
   CODE4 cb ;
   DATA4 *info, *data;

   code4init( &cb );
   info = d4open( &cb, "info" ) ;
   data = d4open( &cb, "data" ) ;

   d4lock( info, 1L ) ;
   d4lockAll( data ) ;

   code4unlock( &cb ) ;  /*unlocks all open files.*/

   code4initUndo( &cb ) ;
}
