/* d4secure.h   (c)Copyright Sequiter Software Inc., 1988-1996.  All rights reserved. */

#ifndef D4SECURE_INC
#define D4SECURE_INC

#ifdef ACCOUNT4CREATE
static FIELD4INFO account4fields[] =
{
   { "ACCOUNTID",  'C', LEN4ACCOUNT_ID, 0 },
   { "PASSWORD",   'C', LEN4PASSWORD, 0 },
   { "TCP_BEGIN",  'C', 15, 0 },
   { "TCP_END",    'C', 15, 0 },
   { "CREATE_TMP", 'L', 1, 0 },
   { "CREATE",     'L', 1, 0 },      // implies create-tmp
   { "CREATE_DEL", 'L', 1, 0 },  // implies create-tmp
   { "DIRECTORY",  'L', 1, 0 },
   { "READ",       'L', 1, 0 },
   { "APPEND",     'L', 1, 0 },
   { "DELETE",     'L', 1, 0 },
   { "UPDATE",     'L', 1, 0 },
   { "INDEX",      'L', 1, 0 },
   { "COMPRESS",   'L', 1, 0 },
   { "DISCONNECT", 'L', 1, 0 },
   { 0, 0, 0, 0 },
} ;
#endif

/* tags always required to possibly create on fly */
static TAG4INFO account4tags[] =
{
   { "NAME", "UPPER(ACCOUNTID)", ".NOT.DELETED()", r4unique, 0 },
   { "NAMEPASS", "UPPER(ACCOUNTID)+PASSWORD", ".NOT.DELETED()", r4unique, 0 },
   { "DELETED", "RECNO()", "DELETED()", 0, 0 },
//   { "ACCOUNT", "UPPER(ACCOUNTID)+ASCEND(TCP_BEGIN)+ASCEND(TCP_END)", ".NOT.DELETED()", r4unique, 0 },
   { 0, 0, 0, 0, 0 },
} ;

#ifdef ACCOUNT4CREATE
static FIELD4INFO privilege4fields[] =
{
   { "ACCOUNTID", 'C', LEN4ACCOUNT_ID, 0 },
   { "PATH",      'C', LEN4PATH, 0 },           // if no table id, then rights apply to entire directory
   { "TABLE",     'C', LEN4TABLE_NAME, 0 },  // includes extension
   { "READ",      'L', 1, 0 },
   { "APPEND",    'L', 1, 0 },
   { "DELETE",    'L', 1, 0 },
   { "UPDATE",    'L', 1, 0 },
   { "INDEX",     'L', 1, 0 },
   { "COMPRESS",  'L', 1, 0 },
   { 0, 0, 0, 0 },
} ;
#endif

// can use '|' as a seperator under NT for long file names

/* tags always required to possibly create on fly */
static TAG4INFO privilege4tags[] =
{
   { "PRIVILEG", "UPPER(ACCOUNTID) + SUBSTR((TRIM(PATH)+'|'+TRIM(TABLE)),1,220)", ".NOT.DELETED()", e4unique, 0 },
   { "ACCOUNT", "UPPER(ACCOUNTID)", ".NOT.DELETED()", 0, 0 },
   { "ACCTQPTH", "UPPER(TRIM(ACCOUNTID)+','+SUBSTR(TRIM(PATH)+'\\'+TRIM(TABLE),1,219))", ".NOT.DELETED()", 0, 0 },
   { "PATH_TBL", "UPPER(SUBSTR(TRIM(PATH)+'\\'+TRIM(TABLE),1,220))", ".NOT.DELETED()", 0, 0 },
   { "TABLE", "UPPER(TABLE)", ".NOT.DELETED()", 0, 0 },
   { "DELETED", "RECNO()", "DELETED()", 0, 0 },
//   { "TABLE", "SUBSTR((TRIM(PATH)+'|'+TRIM(TABLE)),1,220)", ".NOT.DELETED()", 0, 0 },
   { 0, 0, 0, 0, 0 },
} ;

#ifndef S4OFF_SECURITY

#define LEN4ACCOUNT_ID 20
// account name must also include room for an extension

typedef struct
{
   DATA4 *privilegeData ;

   FIELD4 *accountIdField, *pathField, *tableField, *readField,
          *appendField, *deleteField, *updateField, *indexField, *compressField ;
} PRIVILEGE4 ;

typedef struct
{
   DATA4 *data ;

   FIELD4 *accountIdField, *passwordField, *tcpBeginField, *tcpEndField, *readField,
          *appendField, *deleteField, *updateField, *indexField, *compressField,
          *createField, *createDelField, *createTmpField, *disconnectField ;
} ACCOUNT4 ;

typedef struct
{
   PRIVILEGE4 *privileges ;
   ACCOUNT4 *accounts ;

   char accountId[LEN4ACCOUNT_ID+1] ;
   char tcpAddress[16] ;            // used for informational purposes

   Bool5 allowCreate ;
   Bool5 allowCreateTmp ;
   Bool5 allowCreateDel ;
   Bool5 allowRead ;
   Bool5 allowAppend ;
   Bool5 allowDelete ;
   Bool5 allowUpdate ;
   Bool5 allowIndex ;
   Bool5 allowCompress ;
   Bool5 allowDisconnect ;
} ACCOUNT4USER ;

/* returns success or failures (id not found, password mismatch */
PUBLIC int S4FUNCTION account4userInit( ACCOUNT4USER S4PTR *, ACCOUNT4 S4PTR *, PRIVILEGE4 S4PTR *, TCP4ADDRESS, const char S4PTR *, const char S4PTR * ) ;

/* following functions check the privilegeTable if generic rights are not set */
PUBLIC Bool5 S4FUNCTION account4userAllowCreate( ACCOUNT4USER S4PTR *userAccount, int isTemp, int doReplace ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowRead( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowAppend( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowDelete( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowUpdate( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowIndex( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
PUBLIC Bool5 S4FUNCTION account4userAllowCompress( ACCOUNT4USER S4PTR *userAccount, DATA4 S4PTR *data ) ;
#define account4userAllowDisconnect( userAccount ) ( (userAccount)->allowDisconnect )

PUBLIC int S4FUNCTION account4init( CODE4 S4PTR *c4, ACCOUNT4 S4PTR *account, const char S4PTR *accountFileName ) ;
PUBLIC void S4FUNCTION account4initUndo( ACCOUNT4 S4PTR *account ) ;

PUBLIC int S4FUNCTION privilege4init( CODE4 S4PTR *c4, PRIVILEGE4 S4PTR *privilege, const char S4PTR *privilegeFileName ) ;
PUBLIC void S4FUNCTION privilege4initUndo( PRIVILEGE4 S4PTR *privilege ) ;
PUBLIC Bool5 S4FUNCTION privilege4allowDo( PRIVILEGE4 S4PTR *privilege, const char S4PTR *accountId, const char S4PTR *fullPathName, FIELD4 S4PTR *field ) ;
#define privilege4allowRead( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->readField ))
#define privilege4allowAppend( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->appendField ))
#define privilege4allowDelete( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->deleteField ))
#define privilege4allowUpdate( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->updateField ))
#define privilege4allowIndex( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->indexField ))
#define privilege4allowCompress( p, a, f ) (privilege4allowDo( (p), (a), (f), (p)->compressField ))

int f4tcpAddress( FIELD4 *field, TCP4ADDRESS *address ) ;
#endif /* S4OFF_SECURITY */

#endif /* D4SECURE_INC */
