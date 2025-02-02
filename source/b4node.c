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

/* b4node.c  (c)Copyright Sequiter Software Inc., 1988-2001.  All rights reserved. */

#include "d4all.h"

#if  !defined( S4OFF_INDEX )
   #ifndef S4CLIPPER
      void b4nodeGetFilePosition( INDEX4FILE *i4, const B4NODE node, FILE4LONG *outPos )
      {
         /* LY July 7/03 : change from 0 to 0L for Linux compiler */
         file4longAssign( *outPos, b4node( node ), 0L ) ;
         file4longMultiply( *outPos, i4multiplier( i4 ) ) ;
      }
   #endif /* S4CLIPPER */



   #ifndef S4CLIPPER
      void b4nodeSetFromFilePosition( INDEX4FILE *i4, B4NODE *node, FILE4LONG filePos )
      {
         assert5( i4multiplier( i4 ) != 0 ) ;
         file4longDivide( filePos, i4multiplier( i4 ) ) ;
         assert5( file4longGetHi( filePos ) == 0 ) ;
         b4node( *node ) = file4longGetLo( filePos ) ;
      }
   #endif /* S4CLIPPER */



   #ifdef S4FOX
      void b4nodeSubtractBlocks( B4NODE *node, INDEX4FILE *i4, int numBlocksToSubtract )
      {
         b4node( *node ) -= ( numBlocksToSubtract * i4blockSize( i4 ) / i4multiplier( i4 )  ) ;
      }
   #endif /* S4FOX */



   #ifndef S4CLIPPER
      void b4nodeAddBlocks( B4NODE *node, INDEX4FILE *i4, int numBlocksToAdd )
      {
         b4node( *node ) += ( numBlocksToAdd * i4blockSize( i4 ) / i4multiplier( i4 ) ) ;
      }
   #endif /* S4CLIPPER */



   #ifdef S4FOX
      void b4getFilePosition( B4BLOCK *b4, FILE4LONG *outPos )
      {
         b4nodeGetFilePosition( b4->tag->indexFile, b4->fileBlock, outPos ) ;
      }
   #endif /* S4FOX */
#endif /*  !OFF_INDEX */
