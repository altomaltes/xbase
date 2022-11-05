/* l4link.c   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#include "d4all.h"

#define  LINK_PTR(p)  ((LINK4 *)p)

#ifdef E4LINK
void S4FUNCTION l4addLow( LIST4 *list, void *item )
{
   #ifdef E4LINK
      if ( list == 0 || item == 0 )
         error4( 0, e4parm_null, E96201 ) ;
      else
   #endif
   l4addAfter( list, list->lastNode, item ) ;
}
#endif

void S4FUNCTION l4addAfter( LIST4 *list, void *anchor, void *item )
{
   #ifdef S4DATA_ALIG2
      LINK4  *temp ;
   #endif

   #ifdef E4LINK
      if ( list == 0 || item == 0 )
      {
         error4( 0, e4parm_null, E96202 ) ;
         return ;
      }
      if ( l4seek( list, item ) == 1 )
      {
         error4( 0, e4info, E96202 ) ;
         return ;
      }
      if ( list->lastNode != 0 )
      {
         if ( anchor == 0 )
         {
            error4( 0, e4parm_null, E96202 ) ;
            return ;
         }
         if ( l4seek( list, anchor ) == 0 )
         {
            error4( 0, e4info, E96202 ) ;
            return ;
         }
      }
   #endif

   if ( list->lastNode == 0 )
   {
      list->lastNode = (LINK4 *)item ;
      LINK_PTR(item)->p = (LINK4 *)item ;
      LINK_PTR(item)->n = (LINK4 *)item ;
   }
   else
   {
      #ifdef S4DATA_ALIG2
         memcpy( &LINK_PTR(item)->p, &anchor, sizeof(LINK4 *)  );
         memcpy( &LINK_PTR(item)->n, &LINK_PTR(anchor)->n, sizeof(LINK4 *) ) ;
         memcpy( &temp, &LINK_PTR(anchor)->n, sizeof(LINK4 *) ) ;
         memcpy( &LINK_PTR(temp)->p, &item, sizeof(LINK4 *) ) ;
         memcpy( &LINK_PTR(anchor)->n, &item, sizeof(LINK4 *) ) ;
      #else
         LINK_PTR(item)->p = (LINK4 *)anchor ;
         LINK_PTR(item)->n = LINK_PTR(anchor)->n ;
         LINK_PTR(anchor)->n->p= (LINK4 *)item ;
         LINK_PTR(anchor)->n = (LINK4 *)item ;
      #endif /* !S4DATA_ALIG2 */
      if ( anchor == (void *)list->lastNode )
         list->lastNode = (LINK4 *)item ;
   }

   list->nLink++ ;
   #ifdef E4LINK
      l4check( list ) ;
   #endif
}

void S4FUNCTION l4addBefore( LIST4 *list, void *anchor, void *item )
{
   #ifdef E4LINK
      if ( list == 0 || item == 0 )
      {
         error4( 0, e4parm_null, E96203 ) ;
         return ;
      }
      if ( l4seek( list, item ) == 1 )
      {
         error4( 0, e4info, E96203 ) ;
         return ;
      }
      if ( list->lastNode != 0 )
      {
         if ( anchor == 0 )
         {
            error4( 0, e4parm_null, E96202 ) ;
            return ;
         }
         if ( l4seek( list, anchor ) == 0 )
         {
            error4( 0, e4info, E96203 ) ;
            return ;
         }
      }
   #endif

   if ( list->lastNode == 0 )
   {
      list->lastNode = (LINK4 *)item ;
      LINK_PTR(item)->p = (LINK4 *)item ;
      LINK_PTR(item)->n = (LINK4 *)item ;
   }
   else
   {
      LINK_PTR(item)->n = (LINK4 *)anchor ;
      LINK_PTR(item)->p = LINK_PTR(anchor)->p ;
      LINK_PTR(anchor)->p->n= (LINK4 *)item ;
      LINK_PTR(anchor)->p = (LINK4 *)item ;
   }

   list->nLink++ ;
   #ifdef E4LINK
      l4check( list ) ;
   #endif
}

#ifdef E4LINK
static void l4verifySelected( LIST4 *list )
{
   if ( list != NULL )
      if ( list->selected != NULL )
         if ( l4seek( list, list->selected ) == 0 )
            error4( 0, e4info, E96201 ) ;
}

int S4FUNCTION l4check( LIST4 *list )
{
   /* Check the Linked List */
   LINK4 *onLink ;
   unsigned int i ;

   if ( list == 0 )
      return error4( 0, e4parm_null, E96204 ) ;

   onLink = list->lastNode ;
   if ( onLink == 0 )
   {
      if ( list->nLink != 0 )
         return error4( 0, e4info, E86201 ) ;
   }
   else
      if ( list->nLink == 0 )
         return error4( 0, e4info, E86201 ) ;

   for ( i = 1; i <= list->nLink; i++ )
   {
      if ( onLink->n->p != onLink  ||  onLink->p->n != onLink )
         return error4( 0, e4info, E86201 ) ;

      onLink = onLink->n ;

      if ( i == list->nLink || onLink == list->lastNode )
         if ( i != list->nLink || onLink != list->lastNode )
            return error4( 0, e4info, E86201 ) ;
   }

   #ifdef E4LINK
      l4verifySelected( list ) ;
   #endif
   return 0 ;
}
#endif

#ifdef E4LINK
void *S4FUNCTION l4firstLow( const LIST4 *list )
{
   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E96205 ) ;
         return 0 ;
      }
   #endif

   if ( list->lastNode == 0 )
      return 0 ;
   return (void *)list->lastNode->n ;
}

void *S4FUNCTION l4lastLow( const LIST4 *list )
{
   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E96206 ) ;
         return 0 ;
      }
   #endif

   return (void *)list->lastNode ;
}

void *S4FUNCTION l4nextLow( const LIST4 *list, const void *link )
{
   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E96207 ) ;
         return 0 ;
      }
   #endif

   if ( link == (void *)list->lastNode )
      return 0 ;
   if ( link == 0 )
      return l4first( list ) ;

   return (void *)(LINK_PTR(link)->n) ;
}
#endif

void *S4FUNCTION l4pop( LIST4 *list )
{
   LINK4 *p ;

   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E96208 ) ;
         return 0 ;
      }
   #endif

   p = list->lastNode ;
   l4remove( list, list->lastNode ) ;
   return (void *)p ;
}

void *S4FUNCTION l4prev( const LIST4 *list, const void *link )
{
   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E96209 ) ;
         return 0 ;
      }
   #endif

   if ( link == 0 )
       return (void *)list->lastNode ;
   if ( (void *)list->lastNode->n == link )
      return 0 ;

   return (void *)(LINK_PTR(link)->p) ;
}

void S4FUNCTION l4remove( LIST4 *list, void *item )
{
   #ifdef S4DATA_ALIG2
      LINK4   *temp, *temp2;
   #endif

   if ( item == 0 )
      return ;

   #ifdef E4LINK
      if ( list == 0 )
      {
         error4( 0, e4parm_null, E86202 ) ;
         return ;
      }
      /* Make sure the link being removed is on the linked list ! */
      if ( l4seek( list, item ) == 0 )
      {
         error4( 0, e4parm_null, E86202 ) ;
         return ;
      }
   #endif

   if ( list->selected == item )
   {
      list->selected = (LINK4 *)(LINK_PTR(item)->p) ;
      if ( list->selected == item )
         list->selected = 0 ;
   }
   #ifdef S4DATA_ALIG2
      memcpy( &temp, &LINK_PTR(item)->p, sizeof(LINK4 *) );
      memcpy( &LINK_PTR(temp)->n, &LINK_PTR(item)->n, sizeof(LINK4 *) );
      memcpy( &temp, &LINK_PTR(item)->n, sizeof(LINK4 *) );
      memcpy( &LINK_PTR(temp)->p, &LINK_PTR(item)->p, sizeof(LINK4 *) );
      if ( item == (void *)list->lastNode )
      {
         memcpy(&temp, &((LIST4 *)list)->lastNode, sizeof(LIST4 *) ) ;
         memcpy(&temp2, &LINK_PTR(temp)->p, sizeof(LINK4 *) ) ;
         if (temp == temp2)
            list->lastNode = 0 ;
         else
            list->lastNode = temp2 ;
      }
   #else
      LINK_PTR(item)->p->n = LINK_PTR(item)->n ;
      LINK_PTR(item)->n->p = LINK_PTR(item)->p ;
      if ( item == (void *)list->lastNode )
      {
         if ( list->lastNode->p == list->lastNode )
            list->lastNode = 0 ;
         else
            list->lastNode = list->lastNode->p ;
      }
   #endif  /* !S4DATA_ALIG2 */

   list->nLink-- ;

   #ifdef E4LINK
      LINK_PTR(item)->p = 0 ;
      LINK_PTR(item)->n = 0 ;
      l4check( list ) ;
   #endif
}

/* returns true if the selected link is contained in the list */
int l4seek( const LIST4 *list, const void *item )
{
   LINK4 *linkOn ;
   #ifdef E4LINK
      long nLink ;
   #endif

   #ifdef E4PARM_LOW
      if ( list == 0 || item == 0 )
         error4( 0, e4parm_null, E96211 ) ;
   #endif

   #ifdef E4LINK
      for ( linkOn = 0, nLink = 0 ;; nLink++ )
   #else
      for ( linkOn = 0 ;; )
   #endif
   {
      linkOn = (LINK4 *)l4next( list, linkOn ) ;
      if ( linkOn == 0 )
         break ;
      if ( (void *)linkOn == item )
         return 1 ;
      #ifdef E4LINK
         if ( nLink > l4numNodes( list ) )
            error4( 0, e4struct, E96211 ) ;
      #endif
   }

   return 0 ;
}

/*int l4reset( const LIST4 *list, const void *item ) */
