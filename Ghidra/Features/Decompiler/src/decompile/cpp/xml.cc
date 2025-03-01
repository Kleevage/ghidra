/* ###
 * IP: GHIDRA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Substitute the type names.  */
#define YYSTYPE         XMLSTYPE
/* Substitute the variable and function names.  */
#define yyparse         xmlparse
#define yylex           xmllex
#define yyerror         xmlerror
#define yydebug         xmldebug
#define yynerrs         xmlnerrs

#define yylval          xmllval
#define yychar          xmlchar

/* Copy the first part of user declarations.  */


#include "xml.hh"
// CharData mode   look for '<' '&' or "]]>"
// Name mode       look for non-name char
// CData mode      looking for "]]>"
// Entity mode     looking for ending ';'
// AttValue mode   looking for endquote  or '&'
// Comment mode    looking for "--"

#include <iostream>
#include <string>

string Attributes::bogus_uri("http://unused.uri");

/// \brief The XML character scanner
///
/// Tokenize a byte stream suitably for the main XML parser.  The scanner expects an ASCII or UTF-8
/// encoding.  Characters is XML tag and attribute names are restricted to ASCII "letters", but
/// extended UTF-8 characters can be used in any other character data: attribute values, content, comments. 
class XmlScan {
public:
  /// \brief Modes of the scanner
  enum mode { CharDataMode, CDataMode, AttValueSingleMode,
	      AttValueDoubleMode, CommentMode, CharRefMode,
	      NameMode, SNameMode, SingleMode };
  /// \brief Additional tokens returned by the scanner, in addition to byte values 00-ff
  enum token { CharDataToken = 258,
	       CDataToken = 259,
	       AttValueToken = 260,
	       CommentToken =261,
	       CharRefToken = 262,
	       NameToken = 263,
	       SNameToken = 264,
	       ElementBraceToken = 265,
	       CommandBraceToken = 266 };
private:
  mode curmode;			///< The current scanning mode
  istream &s;			///< The stream being scanned
  string *lvalue;		///< Current string being built
  int4 lookahead[4];	///< Lookahead into the byte stream
  int4 pos;				///< Current position in the lookahead buffer
  bool endofstream;		///< Has end of stream been reached
  void clearlvalue(void);	///< Clear the current token string

  /// \brief Get the next byte in the stream
  ///
  /// Maintain a lookahead of 4 bytes at all times so that we can check for special
  /// XML character sequences without consuming.
  /// \return the next byte value as an integer
  int4 getxmlchar(void) {
    char c;	    
    int4 ret=lookahead[pos];
    if (!endofstream) {
      s.get(c); 
      if (s.eof()||(c=='\0')) {
	endofstream = true;
	lookahead[pos] = '\n';
      }
      else
	lookahead[pos] = c;
    }
    else
      lookahead[pos] = -1;
    pos = (pos+1)&3;
    return ret;
  }
  int4 next(int4 i) { return lookahead[(pos+i)&3]; }	///< Peek at the next (i-th) byte without consuming
  bool isLetter(int4 val) { return (((val>=0x41)&&(val<=0x5a))||((val>=0x61)&&(val<=0x7a))); }	///< Is the given byte a \e letter
  bool isInitialNameChar(int4 val);		///< Is the given byte/character the valid start of an XML name
  bool isNameChar(int4 val);			///< Is the given byte/character valid for an XML name	
  bool isChar(int4 val);				///< Is the given byte/character valid as an XML character
  int4 scanSingle(void);				///< Scan for the next token in Single Character mode
  int4 scanCharData(void);				///< Scan for the next token is Character Data mode
  int4 scanCData(void);					///< Scan for the next token in CDATA mode
  int4 scanAttValue(int4 quote);		///< Scan for the next token in Attribute Value mode
  int4 scanCharRef(void);				///< Scan for the next token in Character Reference mode
  int4 scanComment(void);				///< Scan for the next token in Comment mode
  int4 scanName(void);					///< Scan a Name or return single non-name character
  int4 scanSName(void);					///< Scan Name, allow white space before
public:
  XmlScan(istream &t);					///< Construct scanner given a stream
  ~XmlScan(void);						///< Destructor
  void setmode(mode m) { curmode = m; }	///< Set the scanning mode
  int4 nexttoken(void);					///< Get the next token
  string *lval(void) { string *ret = lvalue; lvalue = (string *)0; return ret; }	///< Return the last \e lvalue string
};

/// \brief A parsed name/value pair
struct NameValue {
  string *name;		///< The name
  string *value;	///< The value
};

extern int xmllex(void);				///< Interface to the scanner
extern int xmlerror(const char *str);			///< Interface for registering an error in parsing
extern void print_content(const string &str);	///< Send character data to the ContentHandler
extern int4 convertEntityRef(const string &ref);	///< Convert an XML entity to its equivalent character
extern int4 convertCharRef(const string &ref);	///< Convert an XML character reference to its equivalent character
static XmlScan *global_scan;					///< Global reference to the scanner
static ContentHandler *handler;					///< Global reference to the content handler



# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef XMLDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define XMLDEBUG 1
#  else
#   define XMLDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define XMLDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined XMLDEBUG */
#if XMLDEBUG
extern int xmldebug;
#endif

/* Token type.  */
#ifndef XMLTOKENTYPE
# define XMLTOKENTYPE
  enum xmltokentype
  {
    CHARDATA = 258,
    CDATA = 259,
    ATTVALUE = 260,
    COMMENT = 261,
    CHARREF = 262,
    NAME = 263,
    SNAME = 264,
    ELEMBRACE = 265,
    COMMBRACE = 266
  };
#endif

/* Value type.  */
#if ! defined XMLSTYPE && ! defined XMLSTYPE_IS_DECLARED

union XMLSTYPE
{


  int4 i;
  string *str;
  Attributes *attr;
  NameValue *pair;


};

typedef union XMLSTYPE XMLSTYPE;
# define XMLSTYPE_IS_TRIVIAL 1
# define XMLSTYPE_IS_DECLARED 1
#endif


extern XMLSTYPE xmllval;

int xmlparse (void);



/* Copy the second part of user declarations.  */



#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined XMLSTYPE_IS_TRIVIAL && XMLSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  25
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   205

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  50
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  70
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  151

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   266

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,    15,
      13,     2,     2,    14,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    12,    18,    17,    48,     2,     2,    47,    16,
       2,     2,     2,     2,     2,    19,     2,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    49,
       2,    32,    20,    21,     2,    25,     2,    23,    24,    31,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    28,
      30,     2,     2,     2,    26,     2,     2,     2,     2,    29,
       2,    22,     2,    27,     2,     2,     2,     2,     2,    40,
      41,    34,     2,    42,     2,    37,     2,     2,    45,    44,
      39,    38,     2,     2,    35,    36,     2,     2,    33,     2,
      43,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11
};

#if XMLDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   135,   135,   136,   137,   138,   139,   140,   141,   142,
     144,   145,   146,   147,   148,   149,   150,   151,   152,   153,
     154,   155,   156,   157,   158,   160,   161,   162,   163,   164,
     165,   166,   168,   169,   170,   171,   172,   173,   174,   176,
     177,   178,   179,   180,   181,   182,   184,   185,   187,   188,
     189,   190,   192,   193,   194,   195,   196,   197,   199,   200,
     201,   202,   203,   204,   205,   207,   208,   210,   211,   212,
     213
};
#endif

#if XMLDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CHARDATA", "CDATA", "ATTVALUE",
  "COMMENT", "CHARREF", "NAME", "SNAME", "ELEMBRACE", "COMMBRACE", "' '",
  "'\\n'", "'\\r'", "'\\t'", "'\\''", "'\"'", "'!'", "'-'", "'>'", "'?'",
  "'['", "'C'", "'D'", "'A'", "'T'", "']'", "'O'", "'Y'", "'P'", "'E'",
  "'='", "'v'", "'e'", "'r'", "'s'", "'i'", "'o'", "'n'", "'c'", "'d'",
  "'g'", "'x'", "'m'", "'l'", "'/'", "'&'", "'#'", "';'", "$accept",
  "document", "whitespace", "S", "attsinglemid", "attdoublemid",
  "AttValue", "elemstart", "commentstart", "Comment", "PI", "CDSect",
  "CDStart", "CDEnd", "doctypepro", "prologpre", "prolog", "doctypedecl",
  "Eq", "Misc", "VersionInfo", "EncodingDecl", "xmldeclstart", "XMLDecl",
  "element", "STag", "EmptyElemTag", "stagstart", "SAttribute",
  "etagbrace", "ETag", "content", "Reference", "refstart", "charrefstart",
  "CharRef", "EntityRef", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,    32,    10,    13,     9,    39,    34,    33,    45,
      62,    63,    91,    67,    68,    65,    84,    93,    79,    89,
      80,    69,    61,   118,   101,   114,   115,   105,   111,   110,
      99,   100,   103,   120,   109,   108,    47,    38,    35,    59
};
# endif

#define YYPACT_NINF -136

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-136)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     132,  -136,    42,  -136,  -136,  -136,  -136,    22,  -136,   125,
       9,    20,  -136,  -136,   143,    28,  -136,    79,  -136,   148,
    -136,  -136,    16,    18,     6,  -136,  -136,  -136,    32,    65,
     148,  -136,  -136,   148,    38,    40,    93,    91,  -136,    -1,
      63,  -136,    39,    27,  -136,    45,    26,    52,   -12,  -136,
    -136,  -136,  -136,    69,    57,    77,   104,  -136,    -3,  -136,
    -136,  -136,  -136,    94,  -136,    95,  -136,  -136,    -4,   103,
    -136,  -136,  -136,    67,   136,  -136,  -136,   106,  -136,    68,
     109,    87,  -136,    90,  -136,   144,     2,  -136,   138,   108,
     117,  -136,   118,  -136,  -136,  -136,   125,    -2,     3,  -136,
    -136,   125,  -136,   145,   131,  -136,   147,   146,  -136,  -136,
     121,  -136,  -136,  -136,  -136,  -136,  -136,  -136,  -136,    54,
    -136,   149,   130,   150,   152,  -136,   142,   151,   140,   153,
    -136,   154,   155,   156,   157,   158,   159,   137,   161,   160,
    -136,    63,   162,   163,   136,  -136,   164,  -136,    63,   136,
    -136
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    18,     0,     4,     5,     6,     7,     0,     8,    38,
       0,     0,    36,    37,    31,     0,    28,     0,    27,     0,
      58,    46,     0,     0,    21,     1,     9,    52,     0,     0,
      30,    25,    29,     0,     0,     0,     0,     0,     2,     0,
       0,    48,     0,     0,    53,     0,     0,     0,     0,    21,
      26,     3,    42,     0,     0,     0,     0,    59,     0,    67,
      64,    63,    62,     0,    60,     0,    47,    61,     0,     0,
      66,    65,    33,     0,     0,    50,    49,     0,    19,     0,
       0,     0,    43,     0,    44,     0,     0,    55,     0,     0,
       0,    68,     0,    34,    10,    13,    35,     0,     0,    54,
      51,     0,    20,     0,     0,    45,     0,     0,    22,    56,
       0,    70,    69,    11,    16,    12,    14,    17,    15,     0,
      41,     0,     0,     0,     0,    57,     0,     0,     0,     0,
      24,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      32,     0,     0,     0,     0,    23,     0,    40,     0,     0,
      39
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -136,  -136,    -8,   -17,  -136,  -136,  -133,  -136,  -136,   165,
     166,  -136,  -136,  -136,  -136,  -136,  -136,  -136,  -135,    71,
    -136,  -136,  -136,  -136,    17,  -136,  -136,  -136,  -136,  -136,
    -136,  -136,   -64,  -136,  -136,  -136,  -136
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     7,     8,     9,    97,    98,    99,    10,    11,    12,
      13,    62,    63,   108,    30,    14,    15,    31,    74,    16,
     120,    36,    17,    18,    19,    20,    21,    22,    44,    65,
      66,    39,    67,    68,    69,    70,    71
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      35,    26,    57,   113,    90,    43,   144,    45,   116,     1,
      58,   147,    81,   149,   114,    86,   150,    27,    49,    56,
     117,    45,    25,    73,   106,    40,    28,    26,     3,     4,
       5,     6,    33,   115,   118,    26,    41,    45,     1,     3,
       4,     5,     6,    87,    91,    59,    59,    76,    26,    46,
      59,    47,     3,     4,     5,     6,    64,    96,    52,    75,
      23,    53,    42,    24,    78,    26,     3,     4,     5,     6,
      79,    80,   110,    77,    54,     3,     4,     5,     6,     3,
       4,     5,     6,    48,   119,    32,    49,   126,    26,    82,
      38,     3,     4,     5,     6,    72,    83,    84,    88,    93,
      34,    50,    26,    89,    51,     3,     4,     5,     6,    23,
      92,    26,    49,   101,    55,   103,     3,     4,     5,     6,
       3,     4,     5,     6,    73,    85,   100,    96,   109,   102,
     104,    73,    96,     3,     4,     5,     6,     3,     4,     5,
       6,   125,     1,     2,     3,     4,     5,     6,     3,     4,
       5,     6,    94,    95,    29,     3,     4,     5,     6,    37,
       3,     4,     5,     6,   105,   107,   111,   112,   121,   122,
     123,   128,   130,   124,   129,   127,   131,   133,   134,   141,
     132,     0,     0,   138,   145,   136,   142,     0,     0,   135,
     140,     0,     0,     0,   139,   137,     0,   143,     0,     0,
       0,   146,     0,   148,    60,    61
};

static const yytype_int16 yycheck[] =
{
      17,     9,     3,     5,     8,    22,   141,    19,     5,    10,
      11,   144,    24,   148,    16,    18,   149,     8,    21,    36,
      17,    19,     0,    40,    22,     9,     6,    35,    12,    13,
      14,    15,    15,    97,    98,    43,    20,    19,    10,    12,
      13,    14,    15,    46,    48,    47,    47,    20,    56,    43,
      47,    19,    12,    13,    14,    15,    39,    74,    20,    20,
      18,    21,    46,    21,    19,    73,    12,    13,    14,    15,
      44,    19,    89,    46,    34,    12,    13,    14,    15,    12,
      13,    14,    15,    18,   101,    14,    21,    33,    96,    20,
      19,    12,    13,    14,    15,    32,    39,    20,     4,    32,
      21,    30,   110,     8,    33,    12,    13,    14,    15,    18,
       7,   119,    21,    45,    21,    28,    12,    13,    14,    15,
      12,    13,    14,    15,   141,    21,    20,   144,    20,    20,
      40,   148,   149,    12,    13,    14,    15,    12,    13,    14,
      15,    20,    10,    11,    12,    13,    14,    15,    12,    13,
      14,    15,    16,    17,    11,    12,    13,    14,    15,    11,
      12,    13,    14,    15,    20,    27,    49,    49,    23,    38,
      23,    41,    20,    27,    24,    26,    34,    37,    25,    42,
      29,    -1,    -1,    26,    22,    30,    25,    -1,    -1,    35,
      31,    -1,    -1,    -1,    36,    39,    -1,    37,    -1,    -1,
      -1,    38,    -1,    39,    39,    39
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    10,    11,    12,    13,    14,    15,    51,    52,    53,
      57,    58,    59,    60,    65,    66,    69,    72,    73,    74,
      75,    76,    77,    18,    21,     0,    52,     8,     6,    11,
      64,    67,    69,    74,    21,    53,    71,    11,    69,    81,
       9,    20,    46,    53,    78,    19,    43,    19,    18,    21,
      69,    69,    20,    21,    34,    21,    53,     3,    11,    47,
      59,    60,    61,    62,    74,    79,    80,    82,    83,    84,
      85,    86,    32,    53,    68,    20,    20,    46,    19,    44,
      19,    24,    20,    39,    20,    21,    18,    46,     4,     8,
       8,    48,     7,    32,    16,    17,    53,    54,    55,    56,
      20,    45,    20,    28,    40,    20,    22,    27,    63,    20,
      53,    49,    49,     5,    16,    82,     5,    17,    82,    53,
      70,    23,    38,    23,    27,    20,    33,    26,    41,    24,
      20,    34,    29,    37,    25,    35,    30,    39,    26,    36,
      31,    42,    25,    37,    68,    22,    38,    56,    39,    68,
      56
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    50,    51,    51,    52,    52,    52,    52,    53,    53,
      54,    54,    54,    55,    55,    55,    56,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    64,    65,    65,    65,
      66,    66,    67,    68,    68,    68,    69,    69,    69,    70,
      71,    72,    73,    73,    73,    73,    74,    74,    75,    75,
      76,    76,    77,    77,    78,    79,    80,    80,    81,    81,
      81,    81,    81,    81,    81,    82,    82,    83,    84,    85,
      86
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     3,     1,     1,     1,     1,     1,     2,
       1,     2,     2,     1,     2,     2,     2,     2,     1,     4,
       5,     2,     3,     9,     3,     1,     2,     1,     1,     2,
       2,     1,     9,     1,     2,     2,     1,     1,     1,    10,
      11,     6,     3,     4,     4,     5,     1,     3,     2,     3,
       3,     4,     2,     2,     3,     2,     3,     4,     0,     2,
       2,     2,     2,     2,     2,     1,     1,     1,     2,     3,
       3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if XMLDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !XMLDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !XMLDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 10:

    { (yyval.str) = new string; global_scan->setmode(XmlScan::AttValueSingleMode); }

    break;

  case 11:

    { (yyval.str) = (yyvsp[-1].str); *(yyval.str) += *(yyvsp[0].str); delete (yyvsp[0].str); global_scan->setmode(XmlScan::AttValueSingleMode); }

    break;

  case 12:

    { (yyval.str) = (yyvsp[-1].str); *(yyval.str) += (yyvsp[0].i); global_scan->setmode(XmlScan::AttValueSingleMode); }

    break;

  case 13:

    { (yyval.str) = new string; global_scan->setmode(XmlScan::AttValueDoubleMode); }

    break;

  case 14:

    { (yyval.str) = (yyvsp[-1].str); *(yyval.str) += *(yyvsp[0].str); delete (yyvsp[0].str); global_scan->setmode(XmlScan::AttValueDoubleMode); }

    break;

  case 15:

    { (yyval.str) = (yyvsp[-1].str); *(yyval.str) += (yyvsp[0].i); global_scan->setmode(XmlScan::AttValueDoubleMode); }

    break;

  case 16:

    { (yyval.str) = (yyvsp[-1].str); }

    break;

  case 17:

    { (yyval.str) = (yyvsp[-1].str); }

    break;

  case 18:

    { global_scan->setmode(XmlScan::NameMode); delete (yyvsp[0].str); }

    break;

  case 19:

    { global_scan->setmode(XmlScan::CommentMode); delete (yyvsp[-3].str); }

    break;

  case 20:

    { delete (yyvsp[-3].str); }

    break;

  case 21:

    { delete (yyvsp[-1].str); yyerror("Processing instructions are not supported"); YYERROR; }

    break;

  case 22:

    { (yyval.str) = (yyvsp[-1].str); }

    break;

  case 23:

    { global_scan->setmode(XmlScan::CDataMode); delete (yyvsp[-8].str); }

    break;

  case 32:

    { delete (yyvsp[-8].str); yyerror("DTD's not supported"); YYERROR; }

    break;

  case 39:

    { handler->setVersion(*(yyvsp[0].str)); delete (yyvsp[0].str); }

    break;

  case 40:

    { handler->setEncoding(*(yyvsp[0].str)); delete (yyvsp[0].str); }

    break;

  case 46:

    { handler->endElement((yyvsp[0].attr)->getelemURI(),(yyvsp[0].attr)->getelemName(),(yyvsp[0].attr)->getelemName()); delete (yyvsp[0].attr); }

    break;

  case 47:

    { handler->endElement((yyvsp[-2].attr)->getelemURI(),(yyvsp[-2].attr)->getelemName(),(yyvsp[-2].attr)->getelemName()); delete (yyvsp[-2].attr); delete (yyvsp[0].str); }

    break;

  case 48:

    { handler->startElement((yyvsp[-1].attr)->getelemURI(),(yyvsp[-1].attr)->getelemName(),(yyvsp[-1].attr)->getelemName(),*(yyvsp[-1].attr)); (yyval.attr) = (yyvsp[-1].attr); }

    break;

  case 49:

    { handler->startElement((yyvsp[-2].attr)->getelemURI(),(yyvsp[-2].attr)->getelemName(),(yyvsp[-2].attr)->getelemName(),*(yyvsp[-2].attr)); (yyval.attr) = (yyvsp[-2].attr); }

    break;

  case 50:

    { handler->startElement((yyvsp[-2].attr)->getelemURI(),(yyvsp[-2].attr)->getelemName(),(yyvsp[-2].attr)->getelemName(),*(yyvsp[-2].attr)); (yyval.attr) = (yyvsp[-2].attr); }

    break;

  case 51:

    { handler->startElement((yyvsp[-3].attr)->getelemURI(),(yyvsp[-3].attr)->getelemName(),(yyvsp[-3].attr)->getelemName(),*(yyvsp[-3].attr)); (yyval.attr) = (yyvsp[-3].attr); }

    break;

  case 52:

    { (yyval.attr) = new Attributes((yyvsp[0].str)); global_scan->setmode(XmlScan::SNameMode); }

    break;

  case 53:

    { (yyval.attr) = (yyvsp[-1].attr); (yyval.attr)->add_attribute( (yyvsp[0].pair)->name, (yyvsp[0].pair)->value); delete (yyvsp[0].pair); global_scan->setmode(XmlScan::SNameMode); }

    break;

  case 54:

    { (yyval.pair) = new NameValue; (yyval.pair)->name = (yyvsp[-2].str); (yyval.pair)->value = (yyvsp[0].str); }

    break;

  case 55:

    { global_scan->setmode(XmlScan::NameMode); delete (yyvsp[-1].str); }

    break;

  case 56:

    { (yyval.str) = (yyvsp[-1].str); }

    break;

  case 57:

    { (yyval.str) = (yyvsp[-2].str); }

    break;

  case 58:

    { global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 59:

    { print_content( *(yyvsp[0].str) ); delete (yyvsp[0].str); global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 60:

    { global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 61:

    { string *tmp=new string(); *tmp += (yyvsp[0].i); print_content(*tmp); delete tmp; global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 62:

    { print_content( *(yyvsp[0].str) ); delete (yyvsp[0].str); global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 63:

    { global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 64:

    { global_scan->setmode(XmlScan::CharDataMode); }

    break;

  case 65:

    { (yyval.i) = convertEntityRef(*(yyvsp[0].str)); delete (yyvsp[0].str); }

    break;

  case 66:

    { (yyval.i) = convertCharRef(*(yyvsp[0].str)); delete (yyvsp[0].str); }

    break;

  case 67:

    { global_scan->setmode(XmlScan::NameMode); }

    break;

  case 68:

    { global_scan->setmode(XmlScan::CharRefMode); }

    break;

  case 69:

    { (yyval.str) = (yyvsp[-1].str); }

    break;

  case 70:

    { (yyval.str) = (yyvsp[-1].str); }

    break;



      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}



XmlScan::XmlScan(istream &t) : s(t)

{
  curmode = SingleMode;
  lvalue = (string *)0;
  pos = 0;
  endofstream = false;
  getxmlchar(); getxmlchar(); getxmlchar(); getxmlchar(); // Fill lookahead buffer
}

XmlScan::~XmlScan(void)

{
  clearlvalue();
}

void XmlScan::clearlvalue(void)

{
  if (lvalue != (string *)0)
    delete lvalue;
}

int4 XmlScan::scanSingle(void)

{
  int4 res = getxmlchar();
  if (res == '<') {
    if (isInitialNameChar(next(0))) return ElementBraceToken;
    return CommandBraceToken;
  }
  return res;
}

int4 XmlScan::scanCharData(void)

{
  clearlvalue();
  lvalue = new string();
  
  while(next(0) != -1) {		// look for '<' '&' or ']]>'
    if (next(0) == '<') break;
    if (next(0) == '&') break;
    if (next(0) == ']')
      if (next(1)== ']')
	if (next(2)=='>')
	  break;
    *lvalue += getxmlchar();
  }
  if (lvalue->size()==0)
    return scanSingle();
  return CharDataToken;
}

int4 XmlScan::scanCData(void)

{
  clearlvalue();
  lvalue = new string();

  while(next(0) != -1) {	// Look for "]]>" and non-Char
    if (next(0)==']')
      if (next(1)==']')
	if (next(2)=='>')
	  break;
    if (!isChar(next(0))) break;
    *lvalue += getxmlchar();
  }
  return CDataToken;		// CData can be empty
}

int4 XmlScan::scanCharRef(void)

{
  int4 v;
  clearlvalue();
  lvalue = new string();
  if (next(0) == 'x') {
    *lvalue += getxmlchar();
    while(next(0) != -1) {
      v = next(0);
      if (v < '0') break;
      if ((v>'9')&&(v<'A')) break;
      if ((v>'F')&&(v<'a')) break;
      if (v>'f') break;
      *lvalue += getxmlchar();
    }
    if (lvalue->size()==1)
      return 'x';		// Must be at least 1 hex digit
  }
  else {
    while(next(0) != -1) {
      v = next(0);
      if (v<'0') break;
      if (v>'9') break;
      *lvalue += getxmlchar();
    }
    if (lvalue->size()==0)
      return scanSingle();
  }
  return CharRefToken;
}

int4 XmlScan::scanAttValue(int4 quote)

{
  clearlvalue();
  lvalue = new string();
  while(next(0) != -1) {
    if (next(0) == quote) break;
    if (next(0) == '<') break;
    if (next(0) == '&') break;
    *lvalue += getxmlchar();
  }
  if (lvalue->size() == 0)
    return scanSingle();
  return AttValueToken;
}

int4 XmlScan::scanComment(void)

{
  clearlvalue();
  lvalue = new string();

  while(next(0) != -1) {
    if (next(0)=='-')
      if (next(1)=='-')
	break;
    if (!isChar(next(0))) break;
    *lvalue += getxmlchar();
  }
  return CommentToken;
}

int4 XmlScan::scanName(void)

{
  clearlvalue();
  lvalue = new string();

  if (!isInitialNameChar(next(0)))
    return scanSingle();
  *lvalue += getxmlchar();
  while(next(0) != -1) {
    if (!isNameChar(next(0))) break;
    *lvalue += getxmlchar();
  }
  return NameToken;
}

int4 XmlScan::scanSName(void)

{
  int4 whitecount = 0;
  while((next(0)==' ')||(next(0)=='\n')||(next(0)=='\r')||(next(0)=='\t')) {
    whitecount += 1;
    getxmlchar();
  }
  clearlvalue();
  lvalue = new string();
  if (!isInitialNameChar(next(0))) {	// First non-whitespace is not Name char
    if (whitecount > 0)
      return ' ';
    return scanSingle();
  }
  *lvalue += getxmlchar();
  while(next(0) != -1) {
    if (!isNameChar(next(0))) break;
    *lvalue += getxmlchar();
  }
  if (whitecount>0)
    return SNameToken;
  return NameToken;
}

bool XmlScan::isInitialNameChar(int4 val)

{
  if (isLetter(val)) return true;
  if ((val=='_')||(val==':')) return true;
  return false;
}

bool XmlScan::isNameChar(int4 val)

{
  if (isLetter(val)) return true;
  if ((val>='0')&&(val<='9')) return true;
  if ((val=='.')||(val=='-')||(val=='_')||(val==':')) return true;
  return false;
}

bool XmlScan::isChar(int4 val)

{
  if (val>=0x20) return true;
  if ((val == 0xd)||(val==0xa)||(val==0x9)) return true;
  return false;
}

int4 XmlScan::nexttoken(void)

{
  mode mymode = curmode;
  curmode = SingleMode;
  switch(mymode) {
  case CharDataMode:
    return scanCharData();
  case CDataMode:
    return scanCData();
  case AttValueSingleMode:
    return scanAttValue('\'');
  case AttValueDoubleMode:
    return scanAttValue('"');
  case CommentMode:
    return scanComment();
  case CharRefMode:
    return scanCharRef();
  case NameMode:
    return scanName();
  case SNameMode:
    return scanSName();
  case SingleMode:
    return scanSingle();
  }
  return -1;
}

void print_content(const string &str)

{
  uint4 i;
  for(i=0;i<str.size();++i) {
    if (str[i]==' ') continue;
    if (str[i]=='\n') continue;
    if (str[i]=='\r') continue;
    if (str[i]=='\t') continue;
    break;
  }
  if (i==str.size())
    handler->ignorableWhitespace(str.c_str(),0,str.size());
  else
    handler->characters(str.c_str(),0,str.size());  
}

int4 convertEntityRef(const string &ref)

{
  if (ref == "lt") return '<';
  if (ref == "amp") return '&';
  if (ref == "gt") return '>';
  if (ref == "quot") return '"';
  if (ref == "apos") return '\'';
  return -1;
}

int4 convertCharRef(const string &ref)

{
  uint4 i;
  int4 mult,val,cur;

  if (ref[0]=='x') {
    i = 1;
    mult = 16;
  }
  else {
    i = 0;
    mult = 10;
  }
  val = 0;
  for(;i<ref.size();++i) {
    if (ref[i]<='9') cur = ref[i]-'0';
    else if (ref[i]<='F') cur = 10+ref[i]-'A';
    else cur=10+ref[i]-'a';
    val *= mult;
    val += cur;
  }
  return val;
}

int xmllex(void)

{
  int res = global_scan->nexttoken();
  if (res>255)
    yylval.str = global_scan->lval();
  return res;
}

int xmlerror(const char *str)

{
  handler->setError(str);
  return 0;
}

int4 xml_parse(istream &i,ContentHandler *hand,int4 dbg)

{
#if YYDEBUG
  yydebug = dbg;
#endif
  global_scan = new XmlScan(i);
  handler = hand;
  handler->startDocument();
  int4 res = yyparse();
  if (res == 0)
    handler->endDocument();
  delete global_scan;
  return res;
}

void TreeHandler::startElement(const string &namespaceURI,const string &localName,
			       const string &qualifiedName,const Attributes &atts)
{
  Element *newel = new Element(cur);
  cur->addChild(newel);
  cur = newel;
  newel->setName(localName);
  for(int4 i=0;i<atts.getLength();++i)
    newel->addAttribute(atts.getLocalName(i),atts.getValue(i));
}

void TreeHandler::endElement(const string &namespaceURI,const string &localName,
			     const string &qualifiedName)
{
  cur = cur->getParent();
}

void TreeHandler::characters(const char *text,int4 start,int4 length)

{
  cur->addContent(text,start,length);
}

Element::~Element(void)

{
  List::iterator iter;
  
  for(iter=children.begin();iter!=children.end();++iter)
    delete *iter;
}

const string &Element::getAttributeValue(const string &nm) const

{
  for(uint4 i=0;i<attr.size();++i)
    if (attr[i] == nm)
      return value[i];
  throw DecoderError("Unknown attribute: "+nm);
}

DocumentStorage::~DocumentStorage(void)

{
  for(int4 i=0;i<doclist.size();++i) {
    if (doclist[i] != (Document *)0)
      delete doclist[i];
  }
}

Document *DocumentStorage::parseDocument(istream &s)

{
  doclist.push_back((Document *)0);
  doclist.back() = xml_tree(s);
  return doclist.back();
}

Document *DocumentStorage::openDocument(const string &filename)

{
  ifstream s(filename.c_str());
  if (!s)
    throw DecoderError("Unable to open xml document "+filename);
  Document *res = parseDocument(s);
  s.close();
  return res;
}

void DocumentStorage::registerTag(const Element *el)

{
  tagmap[el->getName()] = el;
}

const Element *DocumentStorage::getTag(const string &nm) const

{
  map<string,const Element *>::const_iterator iter;

  iter = tagmap.find(nm);
  if (iter != tagmap.end())
    return (*iter).second;
  return (const Element *)0;
}

Document *xml_tree(istream &i)

{
  Document *doc = new Document();
  TreeHandler handle(doc);
  if (0!=xml_parse(i,&handle)) {
    delete doc;
    throw DecoderError(handle.getError());
  }
  return doc;
}

void xml_escape(ostream &s,const char *str)

{
  while(*str!='\0') {
    if (*str < '?') {
      if (*str=='<') s << "&lt;";
      else if (*str=='>') s << "&gt;";
      else if (*str=='&') s << "&amp;";
      else if (*str=='"') s << "&quot;";
      else if (*str=='\'') s << "&apos;";
      else s << *str;
    }
    else
      s << *str;
    str++;
  }
}
