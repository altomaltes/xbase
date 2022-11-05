/* d4data.hpp (c)Copyright Sequiter Software Inc., 1988-1998.  All rights reserved. */

#ifndef __DATA4HEADER
   #define __DATA4HEADER

#ifdef _MSC_VER
   #if _MSC_VER >= 900
      #pragma pack(push,1)
   #else
      #pragma pack(1)
   #endif
#else
   #ifdef __BORLANDC__
      #pragma pack(1)
   #endif
#endif

#ifdef __WIN32
   #define s4memset memset
#endif
#ifdef S4OS2
  #ifdef __BORLANDC__
     #define s4memset memset
  #endif
#endif

#ifdef __TURBOC__
  #pragma warn -inl
#endif

#ifndef S4WINTEL
   #define s4memset memset
#endif

#ifndef s4memset
   #define s4memset _fmemset
#endif

#ifndef S4INLINE2
   #ifdef __BORLANDC__
      #define S4INLINE2
   #else
      #define S4INLINE2 inline
   #endif
#endif

#ifdef E4PARM_HIGH
   #define DEBUG4VOID( a,b )   { if ( a ) { error4( 0, e4struct, ( b ) ); return ; } }
   #define DEBUG4INT( a,b )    { if ( a ) return error4( 0, e4struct, ( b ) ) ; }
   #define DEBUG4PTR( a,b )    { if ( a ) { error4( 0, e4struct, ( b ) ) ; return ( 0 ) ; } }
#else
   #define DEBUG4VOID( a,b )
   #define DEBUG4INT( a,b )
   #define DEBUG4PTR( a,b )
#endif

class S4CLASS Data4 ;
class S4CLASS Index4 ;
class S4CLASS Str4len ;
class S4CLASS Str4ptr ;
class S4CLASS Field4info ;
class S4CLASS Tag4info ;
class S4CLASS List4 ;
class S4CLASS File4 ;
class S4CLASS Tag4 ;
class S4CLASS Date4 ;
class S4CLASS Code4 ;
class S4CLASS Str4flex ;
class S4CLASS Expr4
{
public:
   EXPR4 S4PTR *expr ;

   Expr4()                    { expr = 0 ; }
   Expr4( EXPR4 S4PTR *ex )   { expr = ex ; }
   Expr4( Data4 d, const char * ) ;
   #ifdef __TURBOC__
      operator double() const { double d ; expr4double2( expr, &d ) ; return d ; }
   #else
      operator double() const { return expr4double( expr ) ; }
   #endif

   Data4 data() const ;
   void free()                { expr4free( expr ) ; expr = 0 ; }
   int len()                  { return expr4len( expr ) ; }
   int parse( Data4, const char * ) ;
   int isValid() const        { return expr != 0 ; }
   const char S4PTR*source() const  { return expr4source( expr ) ; }
   const char S4PTR*str()     { return expr4str( expr ) ; }
   int type()                 { return expr4type( expr ) ; }
   const char *vary()         { char *result ; int rc ; rc = expr4vary( expr, &result ) ; return result ; }

   #ifdef S4USE_TRUE
      int true()              { return expr4true( expr ) ; }
   #else
      int isTrue()            { return expr4true( expr ) ; }
   #endif
} ;

class S4CLASS Code4 : public CODE4
{
protected:
   MEM4 *stringTypes[16] ;
   friend class Str4flex ;
public:
   Code4( int initialize = 1 ) { if ( initialize ) init() ; }

   Code4( Code4& ) ;           /* Illegal operation, force link error */
   Code4 &operator = ( Code4 & ) ;    /* Illegal operation, force link error */

   int calcCreate( Expr4 ex, const char *name ) { return ( code4calcCreate( this, ex.expr, name ) ) ? r4success : -1 ; }
   void calcReset()            { code4calcReset( this ) ; }
   int  closeAll()             { return code4close( this ) ; }
#ifdef S4CLIENT
   int connect( S4CONST char *serverId=DEF4SERVER_ID, S4CONST char *processId=DEF4PROCESS_ID,
                S4CONST char *userName = "PUBLIC", S4CONST char *password = 0, S4CONST char *protocol = ( char * )PROT4DEFAULT )
       { return code4connect( this, serverId, processId, userName, password, protocol ) ; }
#endif
   Data4 data( const char * ) ;
   const char S4PTR* dateFormat()  { return code4dateFormat( this ) ; }
   int dateFormat( const char *format ) { return code4dateFormatSet( this, format ) ; }
   int error( const int er, const long er2 = 0L, const char *p1=0, const char *p2 = 0, const char *p3 = 0 )
                       { return error4describe( this, er, er2, p1, p2, p3 ) ; }
   int errorFile( S4CONST char *fileName, int overwrite=1 ) { return error4file( this,fileName,overwrite ); }
   int errorSet( int c = 0 ) { return error4set( this, c ) ; }
   const char S4PTR* errorText( const long c ) { return error4text( this, c ) ; }
   void exit()                { code4exit( this ) ; }
   void exitTest()            { error4exitTest( this ) ; }
   int flushFiles()           { return code4flush( this ) ; }
   const char S4PTR* indexExtension()  { return code4indexExtension( this ) ; }
   S4INLINE2 int   init() ;
   void largeOn()             { code4largeOn( this ) ; }
   int initUndo()             { return code4initUndo( this ) ; }
   int lock()                 { return code4lock( this ) ; }
   void lockClear()           { code4lockClear( this ) ; }
   const char *lockFileName() {return code4lockFileName( this ); }
   long lockItem()            { return code4lockItem( this ) ; }
   #ifdef S4CLIENT
      const char S4PTR* lockUserId()    { return code4lockUserId( this ) ; }
      const char S4PTR* lockNetworkId() { return code4lockNetworkId( this ) ; }
   #endif
   #ifdef S4SERVER
      const char S4PTR* lockUserId()    { return code4lockUserId( this ) ; }
      const char S4PTR* lockNetworkId() { return code4lockNetworkId( this ) ; }
   #endif
   #ifdef S4STAND_ALONE
      #ifndef S4OFF_WRITE
         #ifndef S4OFF_TRAN
            int logCreate( const char *name,const char *userId ) { return code4logCreate( this, name,userId ) ; }
            const char S4PTR* logFileName() { return code4logFileName( this ) ; }
            int logOpen( const char *name, const char *userId ) { return code4logOpen( this, name, userId ) ; }
            void logOpenOff() { code4logOpenOff( this ) ; }
         #endif
      #endif
   #endif
   int optAll()               { return code4optAll( this ) ; }
   int optStart()             { return code4optStart( this ) ; }
   int optSuspend()           { return code4optSuspend( this ) ; }
   long timeout()             { return code4timeout( this ); }
   void timeout( long l )     { code4timeoutSet( this, l ) ; }
   int tranCommit()           { return code4tranCommit( this ) ; }
   int tranRollback()         { return code4tranRollback( this ) ; }
   int tranStart()            { return code4tranStart( this ) ; }
   int tranStatus()           { return  code4tranStatus( this ); }
   int unlock()               { return code4unlock( this ) ; }
   int unlockAuto()           { return code4unlockAuto( this ) ; }
   int unlockAuto( short c )  { return code4unlockAutoSet( this, c ) ; }

} ;

class S4CLASS Data4
{
public:
   DATA4 S4PTR *data ;
   Data4()                           { data = 0 ; }
   Data4( DATA4 S4PTR*d )            { data = d ; }
   Data4( Code4& code, const char *name )  { data = d4open( &code, name ) ; }
   operator DATA4 *()  const         { return data; }
   const char   S4PTR* alias()             { return d4alias( data ) ; }
   DATA4 S4PTR *dat()                { return data; }
   void alias( const char S4PTR *ptr )   { d4aliasSet( dat(),ptr ) ; }
   int append()                   { return d4append( data ) ; }
   int appendBlank()              { return d4appendBlank( data ) ; }
   int appendStart( int memo = 0 ){ return d4appendStart( data, memo ) ; }
   void blank()                   {d4blank( data ) ; }
   int bof()                      { return d4bof( data ) ; }
   int bottom()                   { return d4bottom( data ) ; }
   int changed( int flag = -1 )   { return d4changed( data, flag ) ; }
   int check()                    { return d4check( data ) ; }
   int close()                    { int rc = d4close( data ) ; data = 0; return rc; }
   int create( Code4& code, S4CONST char *name, FIELD4INFO f[], TAG4INFO t[])
       { data = d4create( &code, name, f, t ) ; return code.errorCode ; }
   int create( Code4& code, S4CONST char *name, FIELD4INFO f[])
       { data = d4create( &code, name, f, 0 ) ; return code.errorCode ; }
   int create( Code4& code, S4CONST char *name, Field4info& f );
   int create( Code4& code, S4CONST char *name, Field4info& f, Tag4info& t );

   void deleteRec()               { d4delete( data ) ; }
   int deleted()                  { return d4deleted( data ) ; }
   int eof()                      { return d4eof( data ) ; }
   int fieldNumber( const char *name ) { return d4fieldNumber( data, name ) ; }
   const char S4PTR* fileName()   { return d4fileName( data ) ; }
   int flush()                 { return d4flush( data ) ; }
   int freeBlocks()               { return d4freeBlocks( data ) ; }
   int go( const long rec )       { return d4go( data, rec ) ; }
   int goBof()                    { return d4goBof( data ) ; }
   int goEof()                    { return d4goEof( data ) ; }
   Index4 index( const char *name ) ;
   int isValid()                  { return data != 0 ; }
   int lock( const long recNum )  { return d4lock( data, recNum ) ; }
   int lockAdd( long recnum )     { return d4lockAdd( data, recnum ) ; }
   int lockAddAppend()            { return d4lockAddAppend( data ) ; }
   int lockAddFile()              { return d4lockAddFile( data ) ; }
   int lockAddAll()               { return d4lockAddAll( data ) ; }
   int lockAll()                  { return d4lockAll( data ) ; }

   int lockAppend()               { return d4lockAppend( data ) ; }
   int lockFile()                 { return d4lockFile( data ) ; }

   int log( int l )               { return d4log( data, l ) ; }
   int log()                      { return d4log( data, -1 ) ; }
   int memoCompress()             { return d4memoCompress( data ) ; }
   int numFields()                { return d4numFields( data ) ; }
   int open( Code4 &code, S4CONST char *name )
       { data =  d4open( &code, name ) ; return ( (code.errOpen == 0) ? (code.errorCode) : (code.errorCode < 0 ? code.errorCode : 0 )) ; }
   S4INLINE2 int openClone( Data4 ) ;
   int optimize( const int optFlag ) { return d4optimize( data, optFlag ) ; }
   int optimizeWrite( const int optFlag ) { return d4optimizeWrite( data,  optFlag ) ; }
   int pack()                     { return d4pack( data ) ; }
   #ifdef __TURBOC__
      double position()           { double d; d4position2( data, &d ) ; return d; }
   #else
      double position()           { return d4position( data ) ; }
   #endif
   int position( const double pos ) { return d4positionSet( data, pos ) ; }
   void recall()                  { d4recall( data ) ; }
   long recCount()                { return d4recCount( data ) ; }
   long recNo()                   { return d4recNo( data ) ; }
   char S4PTR* record()           { return d4record( data ) ; }
   unsigned int recWidth()        { return d4recWidth( data ) ; }
   int refresh()                  { return d4refresh( data ) ; }
   int refreshRecord()            { return d4refreshRecord( data ) ; }
   int reindex()                  { return d4reindex( data ) ; }
   int remove()                   { return d4remove( data ) ; }
   int seek( S4CONST char *ptr )  { return d4seek( data, ptr ) ; }
   int seek( S4CONST char *ptr, short len ) { return d4seekN( data, ptr, len ) ; }
   int seek( S4CONST double d )   { return d4seekDouble( data, d ) ; }
   #ifndef S4CB51
      int seekNext( const char *ptr ) { return d4seekNext( data, ptr ) ; }
      int seekNext( const char *ptr, short len ) { return d4seekNextN( data, ptr, len ) ; }
      int seekNext( const double d ) { return d4seekNextDouble( data, d ) ; }
   #endif
   void select()                  { d4tagSelect( data, 0 ) ; }
   void select( Tag4 ) ;
   void select( const char * ) ;
   int skip( const long n = 1 )   { return d4skip( data, n ) ; }
   int tagSync()                  { return d4tagSync( data, d4tagSelected( data ) ) ; }
   int top()                      { return d4top( data ) ; }
   int unlock()                   { return d4unlock( data ) ; }
   int write( long rec = -1 )     { return d4write( data,rec ) ; }
   int zap( long f = 1, long last = 1000000000L ) { return d4zap( data, f, last ) ; }
} ;

class S4CLASS Index4
{
public:
   INDEX4 S4PTR *index ;
public:
   Index4()                       { index = 0 ; }
   Index4( INDEX4 S4PTR *in )     { index = in ; }
   Index4( Data4 d, S4CONST char *name = NULL ) { index = i4open( d.dat(), name ) ; }
   int close()                    { int rc =  i4close( index ) ; index = 0 ; return rc; }
   int create( Data4 d, S4CONST char *name, S4CONST TAG4INFO *info ) ;
   int create( Data4 d, S4CONST TAG4INFO *info ) ;
   int create( Data4 d, S4CONST char *name, Tag4info& info) ;
   int create( Data4 d, Tag4info& info ) ;

   Data4 data() ; // { DEBUG4PTR( index == 0, 60704 ) return Data4( index->data ) ; }
   const char S4PTR* fileName()   { return i4fileName( index ) ; }
   void init( Data4 d, const char *name = NULL ) { index = d4index( d.dat(), name );  }
   int isValid()                  { return index != 0 ; }
   S4INLINE2 int open( Data4 d, S4CONST char *file ) ;
   int reindex()                  { return i4reindex( index ) ; }
   Tag4 tag( const char * ) ;
   int tagAdd( const TAG4INFO *newTag ) { return i4tagAdd( this->index, newTag ) ; }
} ;

class S4CLASS Tag4
{
public:
   TAG4 S4PTR *tag ;
   Tag4()                         { tag = NULL ; }
   Tag4( TAG4 S4PTR *tg )         { tag = tg ; }
   Tag4( Data4 d, const char * const name = NULL ) { init(d,name); }
   operator TAG4*() const         { return tag ; }
   #ifndef S4SERVER
      const char S4PTR* alias()   { return t4alias( tag ) ; }
   #else
      const char S4PTR* alias()   { DEBUG4PTR( tag == 0, 60901 ) return tag->tagFile->alias ; }
   #endif
   #ifdef S4CLIPPER
       int close()                { return t4close( tag ) ; }
   #endif
      int descending() ;

   const char S4PTR* expr()       { return t4expr( tag ) ; }
   const char S4PTR* filter()     { return t4filter( tag ) ; }
   void init( Data4, const char *name = NULL ) ;
   void initFirst( Data4 d )      { tag = d4tagNext( d.dat(), NULL ) ; }
   void initLast( Data4 d )       { tag = d4tagPrev( d.dat(), NULL ) ; }
   void initNext()                { if( isValid() ) tag = d4tagNext( tag->index->data, tag ) ; }
   void initPrev()                { if( isValid() ) tag = d4tagPrev( tag->index->data, tag ) ; }
   void initSelected( Data4 d )   { tag = d4tagSelected( d.dat() ) ; }
   int isValid()                  { return tag!=0; } ;
   #ifdef S4CLIPPER
      void open( Data4 d, const char *name ) { tag = t4open( d.dat(), NULL, name ); }
      void open( Data4 d, Index4 i, const char *name ) { tag = t4open( d.dat(), i.index, name ) ; }
   #endif
   int unique()                   { return t4unique( tag ) ; }
   int unique( const short uniqueCode ) { return t4uniqueSet( tag, uniqueCode ) ; }
} ;

class S4CLASS Str4len ;

class S4CLASS Str4
{
public:
   #ifdef __ZTC__
      virtual operator char() ;
      virtual operator double() ;
      virtual operator int() ;
   #else
      operator char() ;
      operator double() ;
      operator int() ;
   #endif
   virtual operator long() ;
   char& operator[]( int ) ;
   int operator==( Str4& ) ;  /* Equal, including length */
   int operator!=( Str4& s ) { return ! operator==( s ) ; }
   int operator< ( Str4& ) ;
   int operator> ( Str4& ) ;
   int operator>=( Str4& s ) { return ! operator<( s ) ; }
   int operator<=( Str4& s ) { return ! operator>( s ) ; }
   int add( Str4& ) ;
   int add( char * ) ;
   int assign( const char S4PTR * ) ;
   int assign( const char S4PTR *, const unsigned ) ;
   int assign( const Str4& ) ;
   void assignDouble( double, int newLen = -1, int n_dec = -1 ) ;
   void assignLong( long, int newLen = -1, int zeros_in_front = 0 ) ;
   int at( Str4& ) ;
   int encode( char *, char *, char * ) ;
   char *endPtr() ;
   int  insert( Str4&, unsigned pos = 0 ) ;
   void lower() ;
   unsigned ncpy( char *, unsigned ) ;
   int  replace( Str4&, unsigned pos = 0 ) ;
   void set( int ) ;
   void trim() ;
   void upper() ;

   /* Get information */
   const Str4len left( unsigned ) const ;
   const Str4len right( unsigned ) const ;
   const Str4len substr( unsigned, unsigned ) const ;
   #ifdef S4USE_TRUE
      int true() ;
   #else
      int isTrue() ;
   #endif
   virtual void      changed()         {}
   virtual int       decimals()        { return 0 ; }
   virtual unsigned  len() ;
   virtual unsigned len1() const ;
   #ifndef S4OFF_ENFORCE_LOCK
      // Returns r4success if its ok, otherwise returns -1
      virtual int    lockCheck()       { return r4success ; }
   #endif
   virtual unsigned  maximum()         { return len() ; }
   virtual char S4PTR*ptr() = 0 ;
   virtual const char S4PTR * ptr1() const = 0 ;
   virtual const char S4PTR * str() ;
   virtual int setLen( unsigned )      { return -1 ; }
   virtual int setMax( unsigned )      { return -1 ; }

} ;

class S4CLASS Str4char : public Str4
{
public:
   Str4char( char ch = 0 ) ;
   char S4PTR*ptr()           { return &c ; }
   char const S4PTR*ptr1() const { return &c ; }
   unsigned len()             { return 1 ; }
   unsigned len1() const      { return 1 ; }
private:
   char c ;
} ;

class S4CLASS Str4ptr : public Str4
{
public:
   Str4ptr( char *ptr )        { p =   ptr ; }
   char S4PTR*ptr()            { return p ; }
   char const S4PTR*ptr1() const { return p ; }
   char *p ;
} ;

class S4CLASS Str4len : public Str4ptr
{
public:
   Str4len( const void *ptr, unsigned ptrLen ) : Str4ptr( ( char * )ptr ) { curLen =  ptrLen; }
   unsigned len()          { return curLen ; }
   unsigned len1() const   { return curLen ; }
   unsigned curLen ;
} ;

class S4CLASS Str4max : public Str4len
{
public:
   Str4max( void *ptr, unsigned m ) : Str4len( ptr,m ) { maxLen = m; }
   unsigned maximum()          { return maxLen ; }
   int setLen( unsigned ) ;
   unsigned maxLen ;
} ;

/* The following classes  are always guaranteed to be null ended. */
class S4CLASS Str4ten : public Str4
{
public:
   Str4ten()                   { curLen = 0 ; buf[0] = 0 ; buf[10] = 0 ; }
   Str4ten( const char *p )    { curLen = 0 ; buf[10] = 0 ; assign( p ) ; }
   Str4ten( Str4 &s )          { curLen = 0 ; buf[10]=0 ; assign( s ) ; }

   unsigned len()              { return curLen ; }
   unsigned len1() const       { return curLen ; }
   unsigned maximum()          { return 10 ; }
   char S4PTR*ptr()            { return buf ; }
   char const S4PTR*ptr1() const { return buf ; }
   int setLen( unsigned ) ;

   unsigned curLen ;
private:
   char buf[12] ;
} ;

class S4CLASS Str4large: public Str4
{
public:
   Str4large() ;
   Str4large( const char * ) ;
   Str4large( Str4 & ) ;

   unsigned maximum()              { return 255 ; }
   unsigned len()                  { return curLen; }
   unsigned len1() const           { return curLen; }
   char S4PTR*ptr()                { return buf ; }
   char const S4PTR*ptr1() const   { return buf ; }
   int setLen( unsigned ) ;

   unsigned curLen ;
private:
   char buf[256] ;
} ;

class S4CLASS Str4flex : public Str4max
{
public:
   Str4flex( Code4 & ) ;
   Str4flex( Str4flex& ) ;
  ~Str4flex() ;
   Str4flex &operator =( Str4flex &s ) { assign( s ) ; return s ; }

   void free() ;
   int setMax( unsigned ) ;
   const char S4PTR *str()             { return ptr() ; }

   Code4 S4PTR *codeBase ;
} ;

class S4CLASS Field4 : public Str4
{
public:
   Field4()                      { field =  0 ; }
   Field4( Data4 d, int j )      { field =  d4fieldJ( d.dat(), j ) ; }
   Field4( Data4 d, const char *name ) { field =  d4field( d.dat(), name ) ; }
   Field4( FIELD4 *f )           { field =  f ; }

   void assignField( Field4 f )  { f4assignField( field, f.field ) ; }
   void changed() ;
   Data4 data() ; // { DEBUG4PTR( field == 0, E61204 ) return Data4( field->data ) ; }
   int decimals()                { return f4decimals( field ) ; }
   int init( Data4 d, const char *name ) { field = d4field( d.dat(), name ) ; return ( field == 0 ) ? -1 : 0 ; }
   int init( Data4 d, int j )    { field = d4fieldJ( d.dat(), j ) ; return ( field == 0 ) ? -1 : 0 ; }
   int isValid()                 { return field!=0; }
   unsigned len()                { return f4len( field ) ; }
   unsigned len1() const         { return f4len( field ) ; }
   #ifndef S4OFF_ENFORCE_LOCK
      int lockCheck() ;
   #endif
   const char S4PTR * name()     { return f4name( field ) ; }
   int number() ;
   int type()                    { return f4type( field ) ; }
   char S4PTR * ptr()            { return f4ptr( field ) ; }
   char const S4PTR * ptr1() const { return f4ptr( field ) ; }
   const char *str();

   FIELD4    S4PTR *field ;
} ;

class S4CLASS Field4info
{
public:
   Field4info( Code4 & ) ;
   Field4info( Data4 ) ;

   ~Field4info() ;
   Field4info( Field4info & ) ;  // Illegal operation, force link error
   Field4info &operator =( Field4info& ) ;   // Illegal operation, force link error
   int add( const char *, char, int len = 0, int dec = 0 ) ;
   int add( Field4 ) ;
   int add( Data4 ) ;
   int del( int ) ;
   int del( const char * ) ;
   void free() ;
   FIELD4INFO *fields() { return field ; }
   const FIELD4INFO *operator[]( int ) ;
   int numFields()      { return size ; } ;
private:
   Code4 S4PTR *codeBase ;
   FIELD4INFO *field ;
   int size ;
   unsigned length ;
} ;

class S4CLASS Tag4info
{
private:
   char *expr, *filt ;
public:
   Tag4info( Code4 & ) ;
   Tag4info( Data4 ) ;
   Tag4info( Index4 ) ;
   ~Tag4info() ;
   Tag4info( Tag4info& ) ;     // Illegal action, force a link error
   Tag4info &operator =( Tag4info & ) ;  // Illegal action, force a link error

   int add( const char *, const char *, const char *filter = NULL, short uniq = 0, unsigned short desc = 0 ) ;
   int add( Tag4 ) ;
   int del( int ) ;
   int del( const char * ) ;
   TAG4INFO * tags() { return tag ; }
   void free() ;
   int  numTags()    { return size ; } ;
private:
   int  addIndex_tags( INDEX4 * ) ;
   char S4PTR * getName( TAG4INFO *, int tagPos = 0 ) ;
   const char S4PTR* getExpr( TAG4INFO *, int tagPos = 0 ) ;
   const char S4PTR* getFilter( TAG4INFO *, int tagPos = 0 ) ;
   short getUniqueKey( TAG4INFO *, int tagPos = 0 ) ;
   unsigned short getDescendKey( TAG4INFO *, int tagPos = 0 ) ;

   Code4 S4PTR *codeBase ;
   TAG4INFO *tag ;
   int size ;
   unsigned length ;
   friend class Data4 ;
   friend class Index4 ;
} ;

class S4CLASS Field4memo : public Field4
{
public:
   Field4memo() ;
   Field4memo( Data4 &, int ) ;
   Field4memo( Data4 &, const char * ) ;
   Field4memo( Field4 ) ;

   void changed() ;
   void free()             { f4memoFree( field ) ; }
   unsigned len()          { return f4memoLen( field ) ; }
   unsigned len1() const   { return f4memoLen( field ) ; }
   int  setLen( unsigned ) ;

   char S4PTR*ptr()        { return f4memoPtr( field ) ; }
   char const S4PTR*ptr1() const { return f4memoPtr( field ) ; }
   const char S4PTR*str()  { return f4memoStr( field ) ; }

} ;

class S4CLASS  Date4 : public Str4
{
public:
   Date4() ;
   Date4( long ) ;
   Date4( const char S4PTR * ) ;
   Date4( const char S4PTR *, char S4PTR * ) ;
   operator long()                  { return date4long( ptr() ) ; }
   #ifdef __TURBOC__
      operator double()             { double d ; date4formatMdx2( ptr(), &d ) ; return d ; }
   #else
      operator double()             { return date4formatMdx( ptr() ) ; }
   #endif
   long operator +( const long l )  { return ( date4long( ptr() )+l ) ; }
   long operator -( const long l )  { return ( date4long( ptr() )-l ) ; }
   void operator +=( const long l ) { date4assign( ptr(), date4long( ptr() )+l ) ; }
   void operator -=( const long l ) { date4assign( ptr(), date4long( ptr() )-l ) ; }
   long operator ++()               { *this += 1L ; return long( *this ) ; }
   long operator ++( int )          { *this += 1L ; return long( *this )-1L ; }
   long operator --()               { *this -= 1L ; return long( *this ) ; }
   long operator --( int )          { *this -= 1L ; return long( *this )+1L ; }
   void assign( const long l )      { date4assign( ptr(),l ) ; }
   void assign( const char *p )     { u4ncpy( ptr(), p, sizeof( dt ) ) ; }
   void assign( const char *p, char *pict ) { date4init( ptr(), p, pict ) ; }
   void assign( Str4 &s )           { Str4::assign( s ) ; }
   const char S4PTR * cdow()        { return date4cdow( ptr() ) ; }
   const char S4PTR * cmonth()      { return date4cmonth( ptr() ) ; }
   int day()                        { return date4day( ptr() ) ; }    /* Day of month  ( 1-31 ) */
   int dow()                        { return date4dow( ptr() ) ; }    /* Day of week   ( 1-7 ) */
   void format( char *result, char *pict ) { date4format( ptr(), result, pict ) ; }
   const char *format( char * ) const ;
   int isLeap() const               { long y = date4year( dt ) ; return ( y%4 == 0 && y%100 != 0 || y%400 == 0 ) ?  1 : 0 ; }
   unsigned len()                   { return 8 ; }
   unsigned len1() const            { return 8 ; }
   int month()                      { return date4month( ptr() ) ; }/* Month of year ( 1-12 ) */
   void today()                     { date4today( ptr() ) ; }
   int year()                       { return date4year( ptr() ) ; } ;
   char S4PTR*ptr()                 { return  dt ; }
   char const S4PTR*ptr1() const    { return  dt ; }
private:
   char dt[9] ;
} ;

class S4CLASS File4 : public FILE4
{
public:
   File4()
#if defined(S4WINTEL) & !defined(S4DOS)
      { hand = (HANDLE)-1 ; }
#else
      { hand = (int)-1 ; }
#endif

   File4( Code4 &code, S4CONST char *namestr, int doAlloc = 0 ) { open( code, namestr, doAlloc ) ; }
   File4( File4& ) ;     /* Illegal operation, force link error */

   int close()                { return file4close( this ) ; }
   int create( Code4 &code, S4CONST char *namestr, const int doAlloc = 0 )
       { return file4create( this, &code, namestr, doAlloc ) ; }

   int flush()                { return file4flush( this ) ; }
   int isValid() const        { return ( hand >= 0 ) ? 1 : 0 ; }
   long len()                 { return file4longGetLo( file4lenLow( this ) ) ; }
   long len1()                { return file4longGetLo( file4lenLow( this ) ) ; }
   #ifndef S4INTERNAL_COMPILE_CHECK
      int setLen( const long l ) { return file4lenSet( this, l ) ; }
   #endif
   int lock( const long start, const long bytes ) { return file4lockInternal( this, start, 0, bytes, 0 ) ; }
   const char S4PTR*fileName() const {return name; }
   int open( Code4 &code, S4CONST char *namestr, const int doAlloc = 0 ) { return file4open( this, &code, namestr, doAlloc ) ; }
   int optimize( const int optFlag, const int fileType ) { return file4optimize( this, optFlag, fileType ) ; }
   int optimizeWrite( const int optFlag ) { return file4optimizeWrite( this, optFlag ) ; }
   #ifndef S4INTERNAL_COMPILE_CHECK
      unsigned read( const long pos, void *ptr, const unsigned len ) { return file4read( this, pos, ptr, len ) ; }
      unsigned read( const long pos, Str4 &string ) { return read( pos, ( void * )string.ptr(), string.len() ); }
      unsigned readAll( const long pos, void *ptr, const unsigned len ) { return file4readAll( this, pos, ptr, len ) ;  }
      unsigned readAll( const long pos, Str4 &string ) { return readAll( pos, ( void * )string.ptr(), string.len() ) ; }
   #endif
   int refresh()              { return file4refresh( this ) ; }
   int replace( File4 &newFile ) { return file4replace( this, &newFile ) ; }
   int unlock( long posStart, long numBytes ) { return file4unlockInternal( this, posStart, 0, numBytes, 0 ) ; }
   #ifndef S4INTERNAL_COMPILE_CHECK
      int write( const long pos, const void S4PTR* ptr, const unsigned len ) { return file4write( this, pos, ptr, len ) ;  }
      int write( const long pos, const char *nullended ) { return write( pos, nullended, strlen( nullended ) ) ; }
      int write( const long pos, Str4 &s ) { return write( pos, ( void * )s.ptr(), s.len() ) ; }
   #endif
} ;

#ifndef S4INTERNAL_COMPILE_CHECK
class S4CLASS File4seqRead : public FILE4SEQ_READ
{
public:
   File4seqRead( File4 &f, const long startPos, void *buf, const unsigned bufLen )
      { file4seqReadInit( this, &f, startPos, buf, bufLen ) ; }
   File4seqRead()  {}

   File4seqRead& operator>>( Str4 &string ) { read( string ); return *this; }

   void init( File4 &f, long startPos, void *buf, const unsigned bufLen )
      { file4seqReadInit( this, &f, startPos, buf, bufLen ) ; }
   unsigned read( void *ptr, const unsigned len ) { return file4seqRead( this, ptr, len ) ; }
   unsigned read( Str4 &s ) { return file4seqRead( this, s.ptr(), s.len() ); }
   int readAll( void *ptr, const unsigned len ) { return file4seqReadAll( this, ptr, len ); }
   int readAll( Str4 &s ) { return file4seqReadAll( this, s.ptr(), s.len() ); }
} ;

class S4CLASS File4seqWrite : public FILE4SEQ_WRITE
{
public:
   File4seqWrite( File4 &f, const long startPos, void *buf, const unsigned bufLen )
       { file4seqWriteInit( this, &f, startPos, buf, bufLen ) ; }
   File4seqWrite() {}
   File4seqWrite &operator<<( Str4 &buf ) { write( buf ) ; return *this ; }
   File4seqWrite &operator<<( const long  l ) { char t[256] ; c4ltoa45( l, t, 9 ) ; t[9] = '\0' ; write( t ) ; return *this; }
   File4seqWrite &operator<<( const char *buf ) { write( buf, strlen( buf ) ); return *this ; }

   void init( File4 &f, const long startPos, void *buf, const unsigned bufLen )
       { file4seqWriteInit( this, &f, startPos, buf, bufLen ) ; }
   int flush()                 { return file4seqWriteFlush( this ); }
   int repeat( const long numRepeat, const char ch ) { return file4seqWriteRepeat( this, numRepeat, ch ); }
   int write( const void *info, const unsigned infoLen ) { return file4seqWrite( this, info, infoLen ) ; }
   int write( const char *info ) { return file4seqWrite( this, info, strlen( info ) ); }
   int write( Str4 &s ) { return file4seqWrite( this, s.ptr(), s.len() ) ; }
} ;
#endif /* S4INTERNAL_COMPILE_CHECK */

class S4CLASS List4 : public LIST4
{
public:
   List4()                     { init() ; }
/*
   these result in error when attempting to derives classes
   List4( List4 & ) ;          // illegal operation, force link error
   List4 &operator =( List4 & );      // illegal operation, force link error
*/
   void *add( void *item )     { l4add( this, item ) ; return item ; }
   void *addAfter( void *anchor, void *item ) { l4addAfter( this, anchor, item ) ; return item; }
   void *addBefore( void *anchor, void *item ) { l4addBefore( this, anchor, item ) ; return item; }
   void S4PTR*first()          { return l4first( this ) ; }
   void S4PTR*last()           { return l4last( this ) ; }
   void S4PTR*next( void *item = NULL ) { return l4next( this, item ) ; }
   int numNodes()              { return nLink ; }
   void S4PTR*pop()            { return l4pop( this ) ; }
   void S4PTR*prev( void *item ) { return l4prev( this, item ) ; }
   void remove( void *item )   { l4remove( this, item ) ; }
   void init()                 { lastNode = 0 ; nLink = 0 ; }
} ;

class S4CLASS Sort4 : public SORT4
{
public:
   Sort4() {} ;
   Sort4( Code4 &code, int sLen, int otherLen = 0 ) { sort4init( this, &code, sLen, otherLen ) ; }
   Sort4( Sort4 & ) ;      // Illegal operation, force link error
   Sort4 &operator =( Sort4& ) ;  // Illegal operation, force link error
   void *result, *resultOther ;
   long  resultRec ;

   void assignCmp( S4CMP_FUNCTION_PTR f ) { sort4assignCmp( this, f ) ; }
   int free() { return sort4free( this ) ; }
   int get() { return sort4get( this, &resultRec, ( void S4PTR*S4PTR* )&result, ( void S4PTR*S4PTR* )&resultOther ) ; }
   int getInit() { return sort4getInit( this ) ; }
   int init( Code4 &code, int sLen, int otherLen = 0 ) { return sort4init( this, &code, sLen, otherLen ) ; }
   int put( void *sortInfo, void *otherInfo = NULL, long rec = 0L ) { return sort4put( this, rec, sortInfo, otherInfo ) ; }
} ;

class S4CLASS Mem4
{
public:
   Mem4() { mem = 0; }
   Mem4( Code4 &code, int start, int size, int expand, int temp = 1 )
       { mem = mem4create( &code, start, size, expand, temp ) ; }
   Mem4( MEM4 S4PTR * m ) { this->mem = m ; }
   MEM4 S4PTR *mem ;
   void S4PTR*alloc()     { return mem4alloc( mem ) ; }
   int create( Code4 &code, int unitStart, int unitSize, int unitExpand, int makeTemp = 1 )
       { mem = mem4create( &code, unitStart, unitSize, unitExpand, makeTemp ) ; return ( mem==0 ) ? -1 : 0; }
   void free( void *ptr ) { mem4free( mem, ptr ) ; }
   int isValid()          { return mem != 0 ; }
   void release()         { mem4release( mem ) ; }
} ;

class S4CLASS Relate4
{
public:
   Relate4() { relate = 0 ; }
   Relate4( Relate4 master, Data4 slave, char *masterExpr, Tag4 t )
       { relate = relate4createSlave( master.relate, slave.dat(),masterExpr, t.tag ) ; }
   Relate4( RELATE4 S4PTR *r ) { relate = r ; }
   Data4 data()        { return ( relate ) ? Data4( ( DATA4 * )relate->data ) : Data4( ( DATA4 * )NULL ); }
   Tag4 dataTag()      { return ( relate ) ? Tag4( ( TAG4 * )relate->dataTag ) : Tag4( ( TAG4 * )NULL ); }
   int doOne()         { return relate4doOne( relate ) ; }
   int errorAction( int a ) { return relate4errorAction( relate, a ) ; }
   int init( Relate4 master, Data4 slave, char *masterExpr, Tag4 t ) ;

   int isValid()       { return relate != NULL ; }
   Relate4 master()    { return ( relate ) ? Relate4( ( RELATE4 * )relate->master ):Relate4( ( RELATE4 * )NULL ) ; }

   #ifndef S4CB51
      const char S4PTR *masterExpr() { return relate4masterExpr( relate ) ; }
   #endif

   int matchLen( int p ) { return relate4matchLen( relate, p ) ; }
   int type( int p )   { return relate4type( relate, p ) ; }

   RELATE4 S4PTR *relate ;
} ;

class S4CLASS Relate4iterator : public Relate4
{
public:
  Relate4iterator()                  { relate = 0 ; }
  Relate4iterator( class Relate4 r ) { relate = r.relate ; }
  int next()                         { return relate4next( &relate ) !=2 ; }
  int nextPosition()                 { return relate4next( &relate ) ; }
} ;

class S4CLASS Relate4set:public Relate4
{
public:
   Relate4set( Data4 data ) { relate = relate4init( data.dat() ) ; }
   Relate4set()          {relate = 0; }
   Relate4set( RELATE4 S4PTR *r ) { relate = r ; }

   int bottom()             { return relate4bottom( relate ) ; }
   void changed()           { relate4changed( relate ) ; }
   int doAll()              { return relate4doAll( relate ) ; }
   int eof()                { return relate4eof( relate ) ; }
   int free( int p = 0 )    { int rc = relate4free( relate, p ) ; relate = 0; return rc; }
   int init( Data4 data ) ;
   int lockAdd()            { return relate4lockAdd( relate ) ; }
   #ifndef S4CB51
      int optimizeable()    { return relate4optimizeable( relate ) ; }
   #endif
   int querySet( S4CONST char *p = NULL )  { return relate4querySet( relate, p ) ; }
   int skip( long l = 1 )   { return relate4skip( relate, l ) ; }
   int skipEnable( int doEnable = 1 ) { return relate4skipEnable( relate,doEnable ); }
   int sortSet( const char *sort=NULL ) {return relate4sortSet( relate, sort ); }
   int top()                { return relate4top( relate ) ; }
} ;

S4INLINE2 Data4 Code4::data( const char *alias )
{
   Data4 r ;
   r.data = code4data( this,alias ) ;
   return r ;
}

S4INLINE2 int Code4::init( void )
{
   int rc = code4init( this ) ;
   s4memset( stringTypes, 0, sizeof( stringTypes ) ) ;
   return rc ;
}

S4INLINE2 int Data4::create( Code4& code, S4CONST char *name, Field4info& f)
{
   data=d4create(&code , name, f.fields(), 0);
   return code.errorCode;
}

S4INLINE2 int Data4::create( Code4& code, S4CONST char *name, Field4info& f, Tag4info& t)
{
   data=d4create(&code, name, f.fields(), t.tags());
   return code.errorCode;
}

S4INLINE2 Index4 Data4::index( const char *name = NULL )
{
   return Index4( d4index( data, name ) ) ;
}

S4INLINE2 int Data4::openClone( Data4 d )
{
   data = d4openClone( d.dat() ) ;

   if ( data != 0 )
      return data->codeBase->errorCode ;
   else
      return -1 ;
}

S4INLINE2 void Data4::select( Tag4 tag )
{
   d4tagSelect( data, tag.tag ) ;
}

S4INLINE2 void Data4::select( const char *tagName )
{
   if( tagName != NULL )
   {
      TAG4 *tag = d4tag( data, tagName ) ;
      d4tagSelect( data, tag ) ;
   }
   else
      d4tagSelect( data, NULL ) ;
}

S4INLINE2 Expr4::Expr4( Data4 d, const char *ex )
{
   expr = expr4parse( d.dat(),ex ) ;
}

S4INLINE2 int Expr4::parse( Data4 d, const char *ex )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( d.dat() == 0 )
         rc = error4( 0, e4struct, ( E60603 ) ) ;
      else
   #endif
   #ifdef E4DEBUG
      if ( expr )
         error4( d.dat()->codeBase, e4info, E60601 ) ;
      else
   #endif
   {
      expr = expr4parse( d.dat(), ex ) ;
      rc = d.dat()->codeBase->errorCode ;
   }
   return rc ;
}

/* S4INLINE2 Data4 Expr4::data()const
   {
      DEBUG4PTR( expr == 0, E60602 )
      return Data4( expr->data ) ;
   }
*/

S4INLINE2 void Field4::changed()
{
#ifdef E4PARM_HIGH
   if ( field == 0 )
      error4( 0, e4struct, ( E61203 ) );
   else
#endif
   d4changed( field->data, 1 ) ;
}

S4INLINE2 int Field4::number()
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( field == 0 )
         rc = error4( 0, e4struct, ( E61210 ) ) ;
      else
   #endif
   rc = d4fieldNumber( field->data, name() ) ;
   return rc ;
}

#ifdef __BORLANDC__
   #pragma warn -aus
   #pragma warn -par
#endif
S4INLINE2 int Field4memo::setLen( unsigned newLen )
{
   #ifdef S4OFF_MEMO
      return e4notMemo ;
   #else
      #ifdef S4OFF_WRITE
         return e4notWrite ;
      #else
         #ifdef E4PARM_HIGH
            if ( field == 0 ) return error4( 0, e4struct, ( E60525 ) ) ;
         #endif
         int rc ;
         char *buf = 0 ;

         if( field->memo == 0 )
            rc = -1 ;
         else
         {
            if( newLen > field->memo->lenMax )
            {
               buf = ( char * ) u4allocFree( ( CODE4 * ) field->data->codeBase, newLen ) ;
               memcpy( buf, field->memo->contents, field->memo->lenMax ) ;
            }
            rc = f4memoSetLen( field,newLen ) ;
            if( buf != 0 )
            {
               memcpy( field->memo->contents, buf, field->memo->lenMax ) ;
               u4free( buf ) ;
            }
         }
         return rc ;
      #endif
   #endif
}
#ifdef __BORLANDC__
   #pragma warn .aus
   #pragma warn .par
#endif

S4INLINE2 int Index4::create( Data4 d, Tag4info& info )
{
   int rc ;
   #ifdef E4PARM_HIGH
   if ( d.dat() == 0 )
      rc = error4( 0, e4struct, ( E60702 ) ) ;
   else
   {
   #endif
   index = i4create( d.dat(), 0, info.tags()) ;
   rc = d.dat()->codeBase->errorCode ;
   #ifdef E4PARM_HIGH
   }
   #endif
   return rc;
}

S4INLINE2 int Index4::create( Data4 d, S4CONST char *name, S4CONST TAG4INFO *info )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( d.dat() == 0 )
         rc = error4( 0, e4struct, ( 60702L ) ) ;
      else
   #endif
   {
      index = i4create( d.dat(), name, info ) ;
      rc = d.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 int Index4::create( Data4 d, S4CONST TAG4INFO *info )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( d.dat() == 0 )
         rc = error4( 0, e4struct, ( 60702L ) ) ;
      else
   #endif
   {
      index = i4create( d.dat(), 0, info) ;
      rc = d.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 int Index4::create( Data4 d, S4CONST char *name, Tag4info& info )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( d.dat() == 0 )
         rc = error4( 0, e4struct, ( E60702 ) ) ;
      else
   #endif
   {
      index = i4create( d.dat(), name, info.tags() ) ;
      rc = d.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 int Index4::open( Data4 d, S4CONST char *file = NULL )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( d.dat() == 0 )
         rc = error4( 0, e4struct, ( 60709L ) ) ;
      else
   #endif
   {
      index = i4open( d.dat(), file ) ;
      rc = d.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 Tag4 Index4::tag( const char *name )
{
   Tag4 t ;
   t.tag = i4tag( index,name ) ;
   return t ;
}

S4INLINE2 int Relate4::init( Relate4 master, Data4 slave, char *masterExpr, Tag4 t )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( slave.dat() == 0 )
         rc = error4( 0, e4struct, ( E61301 ) ) ;
      else
   #endif
   {
      relate = relate4createSlave( master.relate, slave.dat(), masterExpr, t.tag ) ;
      rc = slave.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 int Relate4set::init( Data4 data )
{
   int rc ;
   #ifdef E4PARM_HIGH
      if ( data.dat() == 0 )
         rc = error4( 0, e4struct, ( E61302 ) ) ;
      else
   #endif
   {
      relate = relate4init( data.dat() ) ;
      rc = data.dat()->codeBase->errorCode ;
   }
   return rc ;
}

S4INLINE2 int Tag4::descending()
{
   int rc;
   #ifdef E4PARM_HIGH
   if ( tag == 0 )
      {  error4( 0, e4struct, ( 60913L ) ) ;
         rc = 0 ;
      }
   else
   #endif
   rc = tfile4isDescending( tag->tagFile ) ;
   return rc ;
}

#ifndef S4OFF_REPORT
   REPORT4 * S4FUNCTION  report4retrieve( Code4 &, char *, int, char * ) ;
   RELATE4 * S4FUNCTION relate4retrieve( Code4 &, char *, int, char * ) ;
   int S4FUNCTION relate4save( Relate4set &, char *, int ) ;
#endif /* NOT S4OFF_REPORT */

#undef s4memset

#ifdef __TURBOC__
  #pragma warn .inl
#endif

#ifdef _MSC_VER
   #if _MSC_VER >= 900
      #pragma pack(pop)
   #else
      #pragma pack()
   #endif
#else
   #ifdef __BORLANDC__
      #pragma pack()
   #endif
#endif

#endif /* __DATA4HEADER */

