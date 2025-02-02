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

/* ex69.c */
#include "xbase/d4all.h"


void main( void )
{
   CODE4 cb ;
   DATA4 *data ;
   TAG4 *nameTag, *defaultTag ;

   code4init( &cb ) ;
   data = d4open( &cb, "info" ) ; /* automatically open data & index file.*/
   nameTag = d4tag( data, "INF_NAME" ) ;
   defaultTag = d4tagDefault( data ) ;

   d4tagSelect( data, defaultTag ) ; /* Select the default tag */
   d4seekDouble( data, 32 ) ;    /* Seek using default tag 'PPL_AGE'*/

   d4tagSelect( data, nameTag ) ; /* Select the 'INF_NAME' tag*/
   d4seek( data, "Fred" ) ; /* Seek using 'INF_NAME */

   d4tagSelect( data, NULL ) ;  /* Select record ordering */
   d4seek( data, "ginger" );    /* The seek uses the default tag, which is
                                   INF_AGE, so this seek fails even though
                                   "ginger" is in the data file */

   code4initUndo( &cb ) ;
}
