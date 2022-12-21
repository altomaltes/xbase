/* d4inline.h   (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifdef S4INLINE

/* B4BLOCK.C */
#ifndef S4INDEX_OFF

#ifdef S4MDX
   #define b4goEof( b4 )         ( (b4)->keyOn = (b4)->nKeys )
   #define b4key( b4, iKey )     ( (B4KEY_DATA *)((char *)&((b4)->info.num) + (b4)->tag->header.groupLen * (iKey)) )
   #define b4keyKey( b4, iKey ) ( (unsigned char *)(((B4KEY_DATA *)( (char *)&((b4)->info.num) + (b4)->tag->header.groupLen * (iKey) ))->value ) )
   #define b4lastpos( b4 )        ( ( b4leaf( (b4) ) ) ? ( (b4)->nKeys - 1 ) : ( (b4)->nKeys ) )
   #define b4leaf( b4 )           ( b4key( (b4), (b4)->nKeys )->num == 0L )
   #define b4recNo( b4, i )       ( b4key( (b4), (i) )->num )
#endif /* S4MDX */

#ifdef S4FOX
   #define b4insert( b4, k, r, r2, nf ) ( b4leaf( (b4) ) ? b4insertLeaf( (b4), (k), (r) ) : b4insertBranch( (b4), (k), (r), (r2), (nf) ) )
   #define b4go( b4, iKey )      ( b4skip( (b4), (iKey) - (b4)->keyOn ) )
   #define b4keyKey( b4, iKey ) ( (unsigned char *)b4key( (b4), (iKey) )->value )
   #define b4lastpos( b4 )        ( (b4)->header.nKeys - 1 )
   #define b4leaf( b4 )           (  (b4)->header.nodeAttribute >= 2 )
#endif /* S4FOX */

#ifdef N4OTHER
   #define b4goEof( b4 )         ( (b4)->keyOn = (b4)->nKeys )
   #define b4keyKey( b4, iKey ) ( (unsigned char *) b4key( (b4), (iKey) )->value )
   #define b4lastpos( b4 )        ( ( b4leaf( (b4) ) ) ? ( (b4)->nKeys - 1 ) : ( (b4)->nKeys ) )
   #define b4leaf( b4 )           ( ( b4key( (b4), 0 )->pointer == 0L ) )
   #define b4recNo( b4, i )       ( b4key( (b4), i )->num )

   #ifdef S4CLIPPER
      #define b4key( b4, iKey )     ( (B4KEY_DATA *)((char *)&((b4)->nKeys) + ((b4)->pointers)[(iKey)] ) )
   #endif /* S4CLIPPER */

   #ifdef S4NDX
      #define b4key( b4, iKey )     ( (B4KEY_DATA *)((char *)&(b4)->data + (b4)->tag->header.groupLen * (iKey) ) )
   #endif /* S4NDX */

#endif /* N4OTHER */

#endif  /* S4INDEX_OFF */


/* D4DATA.C */
   #define data4serverId( d4 ) ( (d4)->clientId )
   #define data4clientId( d4 ) ( (d4)->clientId )

   #ifndef S4SINGLE
   #endif
   #define code4trans( c4 ) ( &(c4)->c4trans.trans )
   #ifndef S4OFF_TRAN
      #ifndef S4OFF_WRITE
      #endif
      #ifndef S4OFF_WRITE
            #define code4transEnabled( c4 ) ( (c4)->c4trans.enabled && ( code4tranStatus( (c4) ) != r4rollback ) && ( code4tranStatus( (c4) ) != r4off ) )
      #endif
   #endif


/* C4TRANS.C */
   #define code4tranRollbackSingle( c4 )     ( tran4lowRollback( &((c4)->c4trans.trans), 0, 0 ) )
#ifndef S4OFF_TRAN
   #define tran4bottom( t4 )        ( tran4fileBottom( (t4)->c4trans->transFile, (t4) ) )
   #define tran4entryLen( t4 )      ( sizeof( LOG4HEADER ) + (t4)->dataLen + sizeof( TRAN4ENTRY_LEN ) )
   #define tran4clientDataId( t4 )  ( (t4)->header.clientDataId )
   #define tran4clientId( t4 )      ( (t4)->header.clientId )
   #define tran4id( t4 )            ( (t4)->header.transId )
   #define tran4len( t4 )           ( (t4)->header.dataLen )
   #define tran4serverDataId( t4 )  ( (t4)->header.serverDataId )
   #define tran4skip( t4, d )       ( tran4fileSkip( (t4)->c4trans->transFile, (t4), (d) ) )
   #define tran4top( t4 )           ( tran4fileTop( (t4)->c4trans->transFile, (t4) ) )
   #define tran4type( t4 )          ( (t4)->header.type )
#endif

#define u4ptrEqual( a, b ) ( a == b )
#define u4delaySec() ( u4delayHundredth( 100 ) )

#define tran4dataList( t4 ) ( (t4)->dataList )
#define tran4dataListSet( t4, l4 ) ( (t4)->dataList = l4 )

   #define error4code( a ) ( (a)->errorCode )
   #define error4code2( a ) ( (a)->errorCode2 )
#define expr4parse( a, b ) ( expr4parseLow( (a), (b), 0 ) )

#else   /* NOT S4INLINE STARTS NOW... */

#ifdef __cplusplus
   extern "C" {
#endif

PUBLIC void S4PTR * S4FUNCTION l4first( S4CONST LIST4 S4PTR * ) ;  /* Returns 0 if none */
PUBLIC void S4PTR * S4FUNCTION l4last(  S4CONST LIST4 S4PTR * ) ;   /* Returns 0 if none */
PUBLIC void S4PTR * S4FUNCTION l4next(  S4CONST LIST4 S4PTR *, S4CONST void S4PTR * ) ;  /* Returns 0 if none */
PUBLIC void S4FUNCTION l4add(  LIST4 S4PTR *, void S4PTR * ) ;

PUBLIC int S4FUNCTION error4code( CODE4 S4PTR * ) ;

LIST4 * S4FUNCTION tran4dataList( TRAN4 * ) ;
int tran4dataListSet( TRAN4 *, LIST4 * ) ;

void *s4real( FIXED4MEM ) ;
void *s4protected( FIXED4MEM ) ;
long data4clientId( DATA4 * ) ;
long S4FUNCTION data4serverId( DATA4 * ) ;
#ifdef __cplusplus
   }
#endif
#ifdef __cplusplus
   extern "C" {
#endif
#ifndef S4SINGLE
   int code4unlockAutoSave( CODE4 *c) ;
#endif
#ifndef S4OFF_WRITE
   #ifndef S4OFF_TRAN
      int code4transEnabled( CODE4 * ) ;
   #endif
#endif
TRAN4 *code4trans( CODE4 * ) ;

/* C4TRANS.C */
#ifndef S4OFF_TRAN
unsigned short int tran4entryLen( LOG4HEADER * ) ;
#endif

PUBLIC int    S4FUNCTION u4ptrEqual( const void S4PTR *, const void S4PTR * ) ;

#ifdef __cplusplus
   }
#endif
#endif   /* S4INLINE */


#ifndef S4NO_FILELENGTH
   #define u4filelength( a )          ( filelength( a ) )
#endif

#define E4PARHIGH(   param, errno  ) if (!( param )) {  return( error4( 0, e4parmNull, errno )); }
#define E4PARMLOW(   param, errno  ) if (!( param )) {  error4( 0, e4parmNull, errno ) ; return 0 ; }
#define E4PARM_TEST( param, errno  ) if (   param  ) {  return( error4( 0, e4parm, errno )); }
#define E4PARM_TRET( prm, err, ret ) if (   prm    ) {  error4( 0, e4parm    , err ) ; return ret ; }
#define E4PARM_HRET( prm, err, ret ) if (!( prm   )) {  error4( 0, e4parmNull, err ) ; return ret ; }

#define  E4ANA( cond, ret ) if ( cond ) { return( ret ); } 

#define  C4PARMTAG(    tag, msg, ret ) if ( c4ParmCheckTag(   tag, msg ) < 0 ) return ret ;
#define  C4PARMRELATE( rel, msg, ret ) if ( c4ParmCheckRelate(rel, msg ) < 0 ) return ret ;
#define  C4PARMDATA(   dat, msg, ret ) if ( c4ParmCheckData(  dat, msg ) < 0 ) return ret ;
#define  C4PARMCODE(    cb, msg, ret ) if ( c4ParmCheckCode(   cb, msg ) < 0 ) return ret ;
#define  C4PARMFIELD(  fld, msg, ret ) if ( c4ParmCheckField( fld, msg ) < 0 ) return ret ;



