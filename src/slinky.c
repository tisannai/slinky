/**
 * @file   slinky.c
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sun Mar 18 16:14:49 2018
 *
 * @brief  Simple String Library.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include <memtun.h>

#include "slinky.h"


const char* slinky_version = "0.0.1";


/* ------------------------------------------------------------
 * Utility macros
 * ------------------------------------------------------------ */

/* clang-format off */

/** @cond slinky_none */
#define sl_malsize(s)  (sizeof(sl_s) + (s))

#define sl_smsk        0xFFFFFFFE

#define sl_str(s)      ((char*)&((s)->str[0]))
#define sl_base(s)     ((sl_base_p)((s)-(sizeof(sl_s))))
#define sl_len(s)      (((sl_base_p)((s)-(sizeof(sl_s))))->len)
#define sl_len1(s)     ((((sl_base_p)((s)-(sizeof(sl_s))))->len)+1)
#define sl_res(s)      (((sl_base_p)((s)-(sizeof(sl_s))))->res & sl_smsk)
#define sl_end(s)      ((char*)((s)+sl_len(s)))

#define sl_snor(size)  (((size) & 0x1) ? (size) + 1 : (size))
#define sl_local(s)    (((sl_base_p)((s)-(sizeof(sl_s))))->res&0x1)

#define sc_len(s)      strlen(s)
#define sc_len1(s)     (strlen(s)+1)

#define sc_mp(s)       (((sl_base_p)((s)-(sizeof(sl_s))))->mp)

/** @endcond slinky_none */

/* clang-format on */



/* ------------------------------------------------------------
 * Utility functions.
 * ------------------------------------------------------------ */

static char*     sl_copy_setup( char* dst, const char* src );
static off_t     sl_file_size( const char* filename );
static sl_size_t sl_norm_idx( sl_t ss, int idx );
static sl_t      sl_copy_base( sl_p s1, const char* s2, sl_size_t len1 );
static int       sl_compare_base( const void* s1, const void* s2 );
static sl_t      sl_concatenate_base( sl_p s1, const char* s2, sl_size_t len1 );
static sl_t      sl_insert_base( sl_p s1, int pos, const char* s2, sl_size_t len1 );
static int       sl_divide_base( sl_t ss, char c, int size, char** div );
static int       sl_segment_base( sl_t ss, const char* sc, int size, char** div );

static sl_size_t sl_u64_str_len( uint64_t u64 );
static char*     sl_u64_to_str( uint64_t u64, char* str );
static sl_size_t sl_i64_str_len( int64_t i64 );
static char*     sl_i64_to_str( int64_t i64, char* str );

static sl_size_t sl_va_str_len( va_list va );

static char      sl_char_is_special( char c );
static int       sl_str_to_number( const char** str_p );
static sl_size_t sl_va_format_quick_size( const char* fmt, va_list ap );
static void      sl_va_format_quick_append( char** wpp, char ch, va_list ap );


#ifdef SLINKY_USE_MEMTUN
static mt_t slinky_mt = NULL;

void sl_set_memtun( mt_t mt )
{
    slinky_mt = mt;
}

mt_t sl_get_memtun( void )
{
    return slinky_mt;
}
#endif




/* ------------------------------------------------------------
 * Library
 * ------------------------------------------------------------ */

sl_t sl_new( sl_size_t size )
{
    sl_base_p s;

    size = sl_snor( size );
#ifdef SLINKY_USE_MEMTUN
    s = (sl_base_p)mt_alloc( slinky_mt, sl_malsize( size ) );
#else
    s = (sl_base_p)sl_malloc( sl_malsize( size ) );
    // memset( s, 0, sl_malsize( size ) );
#endif
    s->res = size;
    s->len = 0;
    s->str[ 0 ] = 0;
    return sl_str( s );
}


sl_t sl_use( void* mem, sl_size_t size )
{
    assert( ( size & 0x1 ) == 0 );

    sl_base_p s = mem;
    s->res = size - sizeof( sl_s );
    /* Mark allocation as static. */
    s->res = s->res | 0x1;
    s->len = 0;
    s->str[ 0 ] = 0;
    return sl_str( s );
}


sl_t sl_del( sl_p sp )
{
    sl_del2( *sp );
    *sp = 0;
    return NULL;
}


void sl_del2( sl_t ss )
{
#ifdef SLINKY_USE_MEMTUN
    if ( !sl_get_local( ss ) )
        mt_free( slinky_mt, sl_base( ss ) );
#else
    if ( !sl_get_local( ss ) )
        sl_free( sl_base( ss ) );
#endif
}


sl_t sl_reserve( sl_p sp, sl_size_t size )
{
    if ( sl_res( *sp ) < size ) {
        sl_base_p s;
        size = sl_snor( size );
        s = sl_base( *sp );
        if ( sl_get_local( *sp ) ) {
            sl_t sn;
            sn = sl_new( size );
            sl_len( sn ) = sl_len( *sp );
            memcpy( sn, *sp, sl_len1( sn ) );
            s = sl_base( sn );
        } else {
#ifdef SLINKY_USE_MEMTUN
            s = (sl_base_p)mt_realloc( slinky_mt, s, sl_malsize( size ) );
#else
            s = (sl_base_p)sl_realloc( s, sl_malsize( size ) );
#endif
            s->res = size;
        }
        *sp = sl_str( s );
    }

    return *sp;
}


sl_t sl_compact( sl_p sp )
{
    sl_size_t len = sl_len1( *sp );

    len = sl_snor( len );
    if ( sl_res( *sp ) > len ) {
        sl_base_p s;
        s = sl_base( *sp );
        if ( !sl_get_local( *sp ) ) {
#ifdef SLINKY_USE_MEMTUN
            s = (sl_base_p)mt_realloc( slinky_mt, s, sl_malsize( len ) );
#else
            s = (sl_base_p)sl_realloc( s, sl_malsize( len ) );
#endif
            s->res = len;
            *sp = sl_str( s );
        }
    }

    return *sp;
}


sl_t sl_copy( sl_p s1, sl_t s2 )
{
    return sl_copy_base( s1, s2, sl_len1( s2 ) );
}


sl_t sl_copy_c( sl_p s1, const char* s2 )
{
    return sl_copy_base( s1, s2, sc_len1( s2 ) );
}


sl_t sl_append_char( sl_p sp, char c )
{
    sl_size_t len = sl_len( *sp );
    sl_reserve( sp, len + 1 + 1 );
    char* p = &( ( *sp )[ len ] );
    *p++ = c;
    *p = 0;
    sl_len( *sp ) += 1;
    return *sp;
}


sl_t sl_append_n_char( sl_p sp, char c, sl_size_t n )
{
    sl_size_t len = sl_len( *sp );
    sl_reserve( sp, len + n + 1 );
    char* p = &( ( *sp )[ len ] );
    for ( sl_size_t i = 0; i < n; i++, p++ )
        *p = c;
    *p = 0;
    sl_len( *sp ) += n;
    return *sp;
}


sl_t sl_append_substr( sl_p sp, const char* cs, sl_size_t clen )
{
    sl_size_t len = sl_len( *sp );
    sl_reserve( sp, len + clen + 1 );
    char* p = &( ( *sp )[ len ] );
    memcpy( p, cs, clen );
    p += clen;
    *p = 0;
    sl_len( *sp ) += clen;
    return *sp;
}


sl_t sl_append_str( sl_p sp, const char* cs )
{
    sl_size_t len = sl_len( *sp );
    sl_size_t clen = sc_len( cs );
    sl_reserve( sp, len + clen + 1 );
    char* p = &( ( *sp )[ len ] );
    memcpy( p, cs, clen );
    p += clen;
    *p = 0;
    sl_len( *sp ) += clen;
    return *sp;
}


sl_t sl_append_n_str( sl_p sp, const char* cs, sl_size_t n )
{
    sl_size_t len = sl_len( *sp );
    sl_size_t clen = sc_len( cs );

    sl_reserve( sp, len + n * clen + 1 );
    char* p = &( ( *sp )[ len ] );
    for ( sl_size_t i = 0; i < n; i++, p += clen )
        memcpy( p, cs, clen );
    *p = 0;
    sl_len( *sp ) += n * clen;
    return *sp;
}


sl_t sl_append_sr( sl_p sp, sr_s sr )
{
    return sl_append_substr( sp, sr.str, sr.len );
}


sl_t sl_append_va_str( sl_p sp, const char* cs, ... )
{
    if ( cs == NULL )
        return *sp;

    sl_size_t len = sl_len( *sp );
    sl_size_t clen = sc_len( cs );

    va_list   va;
    sl_size_t va_len;

    va_start( va, cs );
    va_len = clen + sl_va_str_len( va );
    va_end( va );

    sl_reserve( sp, len + va_len + 1 );

    char*       p = &( ( *sp )[ len ] );
    const char* p2;
    memcpy( p, cs, clen );
    p += clen;

    va_start( va, cs );
    while ( ( cs = va_arg( va, char* ) ) != NULL ) {
        p2 = cs;
        while ( *p2 )
            *p++ = *p2++;
    }
    va_end( va );

    *p = 0;
    sl_len( *sp ) += va_len;

    return *sp;
}


sl_t sl_duplicate( sl_t ss )
{
    sl_t sn;
    sn = sl_new( sl_res( ss ) );
    sl_copy( &sn, ss );
    return sn;
}


char* sl_duplicate_c( sl_t ss )
{
    sl_size_t len1 = sl_len1( ss );
#ifdef SLINKY_USE_MEMTUN
    char* dup = mt_alloc( slinky_mt, len1 );
#else
    char* dup = sl_malloc( len1 );
#endif
    memcpy( dup, ss, len1 );
    return dup;
}


sl_t sl_replicate( sl_t ss )
{
    sl_t sn;
    sn = sl_new( sl_len1( ss ) );
    sl_copy( &sn, ss );
    return sn;
}


char* sl_drop( sl_t ss )
{
    char* ret = (char*)sl_base( ss );
    memmove( ret, (void*)ss, sl_len1( ss ) );
    return ret;
}


sl_t sl_clear( sl_t ss )
{
    sl_len( ss ) = 0;
    *ss = 0;
    return ss;
}


sl_t sl_from_str_c( const char* cs )
{
    sl_size_t len = sc_len1( cs );
    sl_t      ss = sl_new( len );
    memcpy( ss, cs, len );
    sl_len( ss ) = len - 1;
    return ss;
}


sl_t sl_from_len_c( const char* cs, sl_size_t clen )
{
    sl_size_t len = clen + 1;
    sl_t      ss = sl_new( len );
    memcpy( ss, cs, len );
    sl_len( ss ) = len - 1;
    return ss;
}


sl_t sl_from_va_str_c( const char* cs, ... )
{
    if ( cs == NULL )
        return NULL;

    sl_size_t clen = sc_len( cs );

    va_list   va;
    sl_size_t va_len;

    va_start( va, cs );
    va_len = clen + sl_va_str_len( va );
    va_end( va );

    sl_size_t   len = va_len + 1;
    sl_t        ss = sl_new( len );
    char*       p;
    const char* p2;

    p = (char*)ss;

    memcpy( p, cs, clen );
    p += clen;

    va_start( va, cs );
    while ( ( cs = va_arg( va, char* ) ) != NULL ) {
        p2 = cs;
        while ( *p2 )
            *p++ = *p2++;
    }
    va_end( va );

    *p = 0;
    sl_len( ss ) += va_len;

    return ss;
}


sl_t sl_from_str_with_size_c( const char* cs, sl_size_t size )
{
    sl_size_t len = sc_len1( cs );
    sl_t      ss;
    if ( size > len )
        ss = sl_new( size );
    else
        ss = sl_new( len );
    sl_copy_c( &ss, cs );
    return ss;
}


sl_t sl_refresh( sl_t ss )
{
    sl_size_t len = sc_len1( ss );
    assert( len <= sl_res( ss ) );
    sl_len( ss ) = len - 1;
    return ss;
}


sl_size_t sl_length( sl_t ss )
{
    return sl_len( ss );
}


sl_t sl_set_length( sl_t ss, sl_size_t len )
{
    ss[ len ] = 0;
    sl_len( ss ) = len;
    return ss;
}


sl_size_t sl_reservation_size( sl_t ss )
{
    return sl_res( ss );
}


sl_size_t sl_body_size( void )
{
    return sizeof( sl_s );
}


sl_size_t sl_normalize_size( sl_size_t size )
{
    return sl_snor( size );
}


sl_base_p sl_base_ptr( sl_t ss )
{
    return sl_base( ss );
}


char sl_end_char( sl_t ss )
{
    if ( sl_len( ss ) == 0 )
        return 0;
    else
        return ss[ sl_len( ss ) - 1 ];
}


int sl_compare( sl_t s1, sl_t s2 )
{
    return strcmp( s1, s2 );
}


int sl_is_same( sl_t s1, sl_t s2 )
{
    if ( ( sl_len( s1 ) == sl_len( s2 ) ) && ( strcmp( s1, s2 ) == 0 ) )
        return 1;
    else
        return 0;
}


int sl_is_different( sl_t s1, sl_t s2 )
{
    if ( sl_len( s1 ) != sl_len( s2 ) )
        return 1;
    else if ( strcmp( s1, s2 ) == 0 )
        return 0;
    else
        return 1;
}


void sl_sort( sl_v sa, sl_size_t len )
{
    qsort( sa, len, sizeof( char* ), sl_compare_base );
}


sl_t sl_concatenate( sl_p s1, const sl_t s2 )
{
    return sl_concatenate_base( s1, s2, sl_len1( s2 ) );
}


sl_t sl_concatenate_c( sl_p s1, const char* s2 )
{
    return sl_concatenate_base( s1, s2, sc_len1( s2 ) );
}


sl_t sl_push_char_to( sl_p sp, int pos, char c )
{
    pos = sl_norm_idx( *sp, pos );
    sl_base_p s = sl_base( *sp );
    sl_reserve( sp, sl_len( *sp ) + 1 );
    if ( (sl_size_t)pos != s->len )
        memmove( &s->str[ pos + 1 ], &s->str[ pos ], s->len - pos );
    s->str[ pos ] = c;
    s->len++;
    s->str[ s->len ] = 0;
    return *sp;
}


sl_t sl_pop_char_from( sl_t ss, int pos )
{
    pos = sl_norm_idx( ss, pos );
    sl_base_p s = sl_base( ss );
    if ( (sl_size_t)pos != s->len ) {
        memmove( &s->str[ pos ], &s->str[ pos + 1 ], s->len - pos );
        s->len--;
    }
    return ss;
}


sl_t sl_limit_to_pos( sl_t ss, int pos )
{
    sl_base_p s = sl_base( ss );
    s->str[ pos ] = 0;
    s->len = pos;
    return ss;
}


sl_t sl_cut( sl_t ss, int cnt )
{
    int       pos;
    sl_base_p s = sl_base( ss );
    if ( cnt >= 0 ) {
        pos = s->len - cnt;
        s->str[ pos ] = 0;
        s->len = pos;
        return ss;
    } else {
        sl_size_t len = s->len + cnt;
        pos = -cnt;
        memmove( s->str, &s->str[ pos ], len );
        s->len = len;
        s->str[ len ] = 0;
        return ss;
    }
}


sl_t sl_select_slice( sl_t ss, int a, int b )
{
    sl_size_t an, bn;

    /* Normalize a. */
    an = sl_norm_idx( ss, a );

    /* Normalize b. */
    bn = sl_norm_idx( ss, b );

    /* Reorder. */
    if ( bn < an ) {
        int t;
        t = an;
        an = bn;
        bn = t;
    }

    sl_base_p s = sl_base( ss );
    memmove( s->str, &s->str[ an ], bn - an );
    s->str[ bn - an ] = 0;
    s->len = bn - an;

    return ss;
}


sl_t sl_insert_to( sl_p s1, int pos, sl_t s2 )
{
    return sl_insert_base( s1, pos, s2, sl_len1( s2 ) );
}


sl_t sl_insert_to_c( sl_p s1, int pos, const char* s2 )
{
    return sl_insert_base( s1, pos, s2, sc_len1( s2 ) );
}


sl_t sl_unquote( sl_t ss )
{
    sl_size_t ri;
    sl_size_t wi;
    sl_size_t cnt;
    sl_size_t lim;

    ri = 0;
    wi = 0;
    cnt = 0;
    lim = sl_len( ss );

    if ( ss[ 0 ] == '\"' ) {
        ri++;
        cnt++;
    }

    if ( ss[ lim - 1 ] == '\"' ) {
        lim--;
        cnt++;
    }

    for ( ; ri < lim; ri++ ) {
        if ( ss[ ri ] == '\\' ) {
            ri++;
            cnt++;

            /*
              Allow the single character escapes.

              \a	07	Alert (Beep, Bell) (added in C89)[1]
              \b	08	Backspace
              \f	0C	Formfeed
              \n	0A	Newline (Line Feed); see notes below
              \r	0D	Carriage Return
              \t	09	Horizontal Tab
              \v	0B	Vertical Tab
              \\	5C	Backslash
              \'	27	Single quotation mark
              \"	22	Double quotation mark
              \?	3F	Question mark (used to avoid trigraphs)
              \nnnnote 1	any	The byte whose numerical value is given by nnn interpreted as an
              octal number
              \xhh…	any	The byte whose numerical value is given by hh… interpreted as a hexadecimal
              number \enote 2	1B	escape character (some character sets) \Uhhhhhhhhnote 3	none
              Unicode code point where h is a hexadecimal digit \uhhhhnote 4	none	Unicode code
              point below 10000 hexadecimal
            */

            switch ( ss[ ri ] ) {
                case 'a':
                    ss[ wi++ ] = '\a';
                    break;
                case 'b':
                    ss[ wi++ ] = '\b';
                    break;
                case 'f':
                    ss[ wi++ ] = '\f';
                    break;
                case 'n':
                    ss[ wi++ ] = '\n';
                    break;
                case 'r':
                    ss[ wi++ ] = '\r';
                    break;
                case 'v':
                    ss[ wi++ ] = '\v';
                    break;
                case '\\':
                    ss[ wi++ ] = '\\';
                    break;
                case '\'':
                    ss[ wi++ ] = '\'';
                    break;
                case '"':
                    ss[ wi++ ] = '\"';
                    break;
                case '?':
                    ss[ wi++ ] = '\?';
                    break;
            }
        } else {
            ss[ wi++ ] = ss[ ri ];
        }
    }

    sl_len( ss ) -= cnt;
    ss[ sl_len( ss ) ] = 0;

    return ss;
}


sl_t sl_quote( sl_p sp )
{
    sl_size_t wi;
    sl_size_t cnt;
    char      tc;

    cnt = 0;
    for ( sl_size_t ri = 0; ri < sl_len( *sp ); ri++ ) {
        if ( sl_char_is_special( *sp[ ri ] ) )
            cnt++;
    }

    /* Add start and end quotes. */
    cnt += 2;

    sl_reserve( sp, sl_len( *sp ) + cnt + 1 );

    wi = sl_len( *sp ) + 1;
    *( sp[ wi-- ] ) = 0;
    *( sp[ wi-- ] ) = '\"';
    for ( int32_t ri = sl_len( *sp ) - cnt; ri >= 0; ri-- ) {
        if ( ( tc = sl_char_is_special( *sp[ ri ] ) ) ) {
            *( sp[ wi-- ] ) = tc;
            *( sp[ wi-- ] ) = '\\';
        } else {
            *( sp[ wi-- ] ) = *( sp[ ri ] );
        }
    }

    *( sp[ wi-- ] ) = '\"';

    return *sp;
}


sl_t sl_format( sl_p sp, const char* fmt, ... )
{
    sl_t    ret;
    va_list ap;

    va_start( ap, fmt );
    ret = sl_va_format( sp, fmt, ap );
    va_end( ap );

    return ret;
}


sl_t sl_va_format( sl_p sp, const char* fmt, va_list ap )
{
    va_list coap;

    /* Copy ap to coap for second va-call. */
    va_copy( coap, ap );

    int size;
    size = vsnprintf( NULL, 0, fmt, ap );

    if ( size < 0 )
        return NULL; // GCOV_EXCL_LINE

    size++;
    sl_reserve( sp, sl_len( *sp ) + size );

    size = vsnprintf( sl_end( *sp ), size, fmt, coap );
    va_end( coap );

    sl_len( *sp ) += size;

    return *sp;
}


sl_t sl_format_quick( sl_p sp, const char* fmt, ... )
{
    sl_t    ret;
    va_list ap;

    va_start( ap, fmt );
    ret = sl_va_format_quick( sp, fmt, ap );
    va_end( ap );

    return ret;
}


sl_t sl_va_format_quick( sl_p sp, const char* fmt, va_list ap )
{

    va_list coap;

    va_copy( coap, ap );

    sl_size_t   size;
    sl_size_t   extension;
    const char* c;
    char*       ts;
    int64_t     i64;
    uint64_t    u64;
    sr_s        sr;

    extension = sl_va_format_quick_size( fmt, ap );
    sl_reserve( sp, sl_len1( *sp ) + extension );


    /* ------------------------------------------------------------
     * Expand format string.
     */

    char* wp = sl_end( *sp );
    char* first = wp;

    c = fmt;

    while ( *c ) {

        switch ( *c ) {

            case '%': {
                c++;

                switch ( *c ) {

                    case '!': {
                        wp = sl_end( *sp );
                        break;
                    }

                    case 's':
                    case 'S': {
                        ts = va_arg( coap, char* );
                        if ( *c == 's' ) {
                            size = strlen( ts );
                        } else {
                            size = sl_len( ts );
                        }
                        memcpy( wp, ts, size );
                        wp += size;
                        break;
                    }

                    case 'i':
                    case 'I': {
                        if ( *c == 'i' ) {
                            i64 = va_arg( coap, int );
                        } else {
                            i64 = va_arg( coap, int64_t );
                        }
                        wp = sl_i64_to_str( i64, wp );
                        break;
                    }

                    case 'u':
                    case 'U': {
                        if ( *c == 'u' ) {
                            u64 = va_arg( coap, unsigned int );
                        } else {
                            u64 = va_arg( coap, uint64_t );
                        }
                        wp = sl_u64_to_str( u64, wp );
                        break;
                    }

                    case 'c': {
                        char ch;
                        ch = (char)va_arg( coap, int );
                        *wp++ = ch;
                        break;
                    }

                    case 'p': {
                        int pos;
                        i64 = va_arg( coap, int );
                        pos = wp - first;
                        if ( i64 > pos ) {
                            for ( sl_size_t i = pos; i < i64; i++ ) {
                                *wp++ = ' ';
                            }
                        }
                        break;
                    }

                    case 'r': {
                        sr = va_arg( coap, sr_s );
                        memcpy( wp, sr.str, sr.len );
                        wp += sr.len;
                        break;
                    }

                    case 'a': {
                        char      left_pad;
                        char      pad_char;
                        sl_size_t width;
                        sl_size_t nominal_size;
                        int       gap;
                        char*     first;

                        // %al012i

                        c++;
                        if ( *c == 'l' ) {
                            left_pad = 1;
                        } else {
                            left_pad = 0;
                        }
                        c++;
                        pad_char = *c;
                        c++;

                        width = sl_str_to_number( &c );

                        first = wp;
                        sl_va_format_quick_append( &wp, *c, coap );
                        nominal_size = ( wp - first );

                        gap = ( width - nominal_size );

                        if ( left_pad && gap ) {
                            memmove( first + gap, first, gap );
                            for ( int i = 0; i < gap; i++ ) {
                                *first = pad_char;
                                first++;
                            }
                            wp += gap;
                        }

                        if ( !left_pad && gap ) {
                            for ( int i = 0; i < gap; i++ ) {
                                *wp = pad_char;
                                wp++;
                            }
                        }
                        break;
                    }

                    case '%': {
                        *wp++ = '%';
                        break;
                    }

                    default: {
                        *wp++ = *c;
                        break;
                    }
                }

                c++;
                break;
            }

            default: {
                *wp++ = *c++;
                break;
            }
        }
    }
    va_end( coap );

    sl_len( *sp ) += ( wp - first );

    *wp = 0;

    return *sp;
}


int sl_invert_pos( sl_t ss, int pos )
{
    if ( pos > 0 )
        return -1 * ( sl_len( ss ) - pos );
    else
        return sl_len( ss ) + pos;
}


int sl_find_char_right( sl_t ss, char c, sl_size_t pos )
{
    while ( pos < sl_len( ss ) && ss[ pos ] != c )
        pos++;

    if ( pos == sl_len( ss ) )
        return -1;
    else
        return pos;
}


int sl_find_char_left( sl_t ss, char c, sl_size_t pos )
{
    while ( pos > 0 && ss[ pos ] != c )
        pos--;

    if ( pos == 0 && ss[ pos ] != c )
        return -1;
    else
        return pos;
}


int sl_find_index( sl_t s1, const char* s2 )
{
    if ( s2[ 0 ] == 0 )
        return -1;

    int i1 = 0;
    int i2 = 0;
    int i;

    while ( s1[ i1 ] ) {
        i = i1;
        if ( s1[ i ] == s2[ i2 ] ) {
            while ( s1[ i ] == s2[ i2 ] && s2[ i2 ] ) {
                i++;
                i2++;
            }
            if ( s2[ i2 ] == 0 )
                return i1;
        }
        i1++;
    }

    return -1;
}


int sl_divide_with_char( sl_t ss, char c, int size, char*** div )
{
    if ( size < 0 ) {
        /* Just count size, don't replace chars. */
        return sl_divide_base( ss, c, -1, NULL );
    } else if ( *div ) {
        /* Use pre-allocated storage. */
        return sl_divide_base( ss, c, size, *div );
    } else {
        /* Calculate size and allocate storage. */
        size = sl_divide_base( ss, c, -1, NULL );
#ifdef SLINKY_USE_MEMTUN
        *div = (char**)mt_alloc( slinky_mt, size * sizeof( char* ) );
#else
        *div = (char**)sl_malloc( size * sizeof( char* ) );
#endif
        return sl_divide_base( ss, c, size, *div );
    }
}


int sl_segment_with_str( sl_t ss, const char* sc, int size, char*** div )
{
    if ( size < 0 ) {
        /* Just count size, don't replace chars. */
        return sl_segment_base( ss, sc, -1, NULL );
    } else if ( *div ) {
        /* Use pre-allocated storage. */
        return sl_segment_base( ss, sc, size, *div );
    } else {
        /* Calculate size and allocate storage. */
        size = sl_segment_base( ss, sc, -1, NULL );
#ifdef SLINKY_USE_MEMTUN
        *div = (char**)mt_alloc( slinky_mt, size * sizeof( char* ) );
#else
        *div = (char**)sl_malloc( size * sizeof( char* ) );
#endif
        return sl_segment_base( ss, sc, size, *div );
    }
}


sl_t sl_glue_array( sl_v sa, sl_size_t size, const char* glu )
{
    int       len = 0;
    sl_size_t i;

    /* Calc sa len. */
    i = 0;
    while ( i < size )
        len += strlen( sa[ i++ ] );

    /* Add glu len. */
    len += ( size - 1 ) * strlen( glu );

    sl_t ss;
    ss = sl_new( len + 1 );
    sl_len( ss ) = len;

    /* Build result. */
    char* p = ss;
    i = 0;
    while ( i < size ) {
        p = sl_copy_setup( p, sa[ i ] );
        if ( i < ( size - 1 ) )
            p = sl_copy_setup( p, glu );
        i++;
    }
    ss[ sl_len( ss ) ] = 0;

    return ss;
}


char* sl_tokenize( sl_t ss, const char* delim, char** pos )
{
    if ( *pos == 0 ) {
        /* First iteration. */
        int idx;
        idx = sl_find_index( ss, delim );
        if ( idx < 0 )
            return NULL;
        else {
            ss[ idx ] = 0;
            *pos = &( ss[ idx ] );
            return ss;
        }
    } else if ( *pos == sl_end( ss ) ) {
        /* Passed the last iteration. */
        return NULL;
    } else {
        /* Fix nulled char. */
        **pos = delim[ 0 ];
        char* p = *pos;
        p += strlen( delim );

        if ( *p == 0 ) {
            /* ss ended with delim, so we are done. */
            *pos = sl_end( ss );
            return NULL;
        }

        /* Find next delim. */
        int idx;
        idx = sl_find_index( p, delim );
        if ( idx < 0 ) {
            /* Last token, mark this by: */
            *pos = sl_end( ss );
            return p;
        } else {
            p[ idx ] = 0;
            *pos = &( p[ idx ] );
            return p;
        }
    }
}


sl_t sl_rm_extension( sl_t ss, const char* ext )
{
    char* pos;
    char* t;

    pos = NULL;
    t = sl_tokenize( ss, ext, &pos );
    if ( t ) {
        sl_len( ss ) = pos - ss;
        return ss;
    } else
        return NULL;
}


sl_t sl_directory_name( sl_t ss )
{
    int i;

    /* Find first "/" from end. */
    i = sl_len( ss );
    while ( i > 0 && ss[ i ] != '/' )
        i--;

    if ( i == 0 ) {
        if ( ss[ i ] == '/' ) {
            ss[ 1 ] = 0;
            sl_len( ss ) = 1;
        } else {
            ss[ 0 ] = '.';
            ss[ 1 ] = 0;
            sl_len( ss ) = 1;
            return ss;
        }
    } else {
        ss[ i ] = 0;
        sl_len( ss ) = i;
    }

    return ss;
}


sl_t sl_basename( sl_t ss )
{
    int i;

    /* Find first "/" from end. */
    i = sl_len( ss );
    while ( i > 0 && ss[ i ] != '/' )
        i--;

    if ( i == 0 && ss[ i ] != '/' ) {
        return ss;
    } else {
        i++;
        sl_len( ss ) = sl_len( ss ) - i;
        memmove( ss, &( ss[ i ] ), sl_len( ss ) );
        ss[ sl_len( ss ) ] = 0;
    }

    return ss;
}


sl_t sl_swap_chars( sl_t ss, char f, char t )
{
    sl_size_t i;

    i = 0;
    while ( i < sl_len( ss ) ) {
        if ( ss[ i ] == f )
            ss[ i ] = t;
        i++;
    }

    return ss;
}


sl_t sl_map_str( sl_p sp, const char* f, const char* t )
{
    /*
     * If "t" is longer than "f", loop and count how many instances of
     * "f" is found. Increase size of sp by N*t.len - N*f.len.
     *
     * Loop and find the next "f" index. Skip upto index and insert "t"
     * inplace. Continue until "f" is no more found and insert tail of
     * "sp".
     */

    sl_size_t f_len = sc_len( f );
    sl_size_t t_len = sc_len( t );

    int   idx;
    char *a, *b;

    if ( t_len > f_len ) {
        /* Calculate number of parts. */
        sl_size_t cnt = 0;

        /*
         * Replace XXX with YYYY.
         *
         * foooXXXfiiiXXXdiii
         * foooYYYYfiiiYYYYdiii
         *
         * Prepare org before copy as:
         * --foooXXXfiiiXXXdiii
         *
         *   OR
         *
         * foooXXXfiiiXXXdiiiXXX
         * foooYYYYfiiiYYYYdiiiYYYY
         */
        a = *sp;

        while ( 1 ) {
            idx = sl_find_index( a, f );
            if ( idx >= 0 ) {
                cnt++;
                a += ( idx + f_len );
            } else {
                break;
            }
        }

        sl_size_t nlen;
        sl_size_t olen = sl_len( *sp );
        nlen = sl_len( *sp ) - ( cnt * f_len ) + ( cnt * t_len );
        sl_reserve( sp, nlen + 1 );
        sl_len( *sp ) = nlen;

        /*
         * Shift original sp content to right in order to enable copying
         * chars safely from right to left.
         */
        b = &( ( *sp )[ nlen - olen ] );
        memmove( b, *sp, olen + 1 );
        a = *sp;
    } else {
        /*
         * Replace XXX with YY.
         *
         * foooXXXfiiiXXXdiii
         * foooYYfiiiYYdiii
         */
        a = *sp;
        b = *sp;
    }

    while ( *b ) {
        idx = sl_find_index( b, f );
        if ( idx >= 0 ) {
            memcpy( a, b, idx );
            a += idx;
            a = sl_copy_setup( a, t );
            b += ( idx + f_len );
        } else {
            a = sl_copy_setup( a, b );
            *a = 0;
            break;
        }
    }

    if ( *b == 0 )
        *a = 0;

    return *sp;
}


sl_t sl_map_part( sl_p sp, sl_size_t from_a, sl_size_t from_b, const char* to, sl_size_t to_len )
{
    int   size_diff;
    char* start;
    char* orgtail;
    char* newtail;

    size_diff = to_len - ( from_b - from_a );
    if ( size_diff > 0 ) {
        sl_reserve( sp, sl_len1( *sp ) + size_diff );
    }

    start = *sp;
    orgtail = &start[ from_b ];
    newtail = &start[ from_a + to_len ];

    memmove( newtail, orgtail, sl_len1( *sp ) - from_b );
    memcpy( &start[ from_a ], to, to_len );

    sl_len( *sp ) = sl_len( *sp ) + size_diff;

    return *sp;
}


sl_t sl_capitalize( sl_t ss )
{
    if ( sl_len( ss ) > 0 )
        ss[ 0 ] = toupper( ss[ 0 ] );

    return ss;
}


sl_t sl_toupper( sl_t ss )
{
    for ( sl_size_t i = 0; i < sl_len( ss ); i++ ) {
        ss[ i ] = toupper( ss[ i ] );
    }
    return ss;
}


sl_t sl_tolower( sl_t ss )
{
    for ( sl_size_t i = 0; i < sl_len( ss ); i++ ) {
        ss[ i ] = tolower( ss[ i ] );
    }
    return ss;
}


sl_t sl_read_file( const char* filename )
{
    sl_t ss;

    off_t size = sl_file_size( filename );
    if ( size < 0 )
        return NULL; // GCOV_EXCL_LINE

    ss = sl_new( size + 1 );

    int fd;

    fd = open( filename, O_RDONLY );
    if ( fd == -1 )
        return NULL; // GCOV_EXCL_LINE
    read( fd, ss, size );
    ss[ size ] = 0;
    sl_len( ss ) = size;
    close( fd );

    return ss;
}


sl_t sl_read_file_with_pad( const char* filename, sl_size_t left, sl_size_t right )
{
    sl_t ss;

    off_t size = sl_file_size( filename );
    if ( size < 0 )
        return NULL; // GCOV_EXCL_LINE

    ss = sl_new( size + left + right + 1 );

    int fd;

    fd = open( filename, O_RDONLY );
    if ( fd == -1 )
        return NULL; // GCOV_EXCL_LINE
    read( fd, &ss[ left ], size );
    /* Zero the tail. */
    memset( &ss[ size + left ], 0, right + 1 );
    sl_len( ss ) = size + left;
    close( fd );

    return ss;
}


sl_t sl_write_file( sl_t ss, const char* filename )
{
    int fd;

    fd = creat( filename, S_IWUSR | S_IRUSR );
    if ( fd == -1 )
        return NULL; // GCOV_EXCL_LINE
    write( fd, ss, sl_len( ss ) );
    close( fd );

    return ss;
}


void sl_print( const char* fmt, ... )
{
    char mem[ 2048 ];
    sl_t sl;

    sl = sl_use( mem, 2048 );

    va_list ap;

    va_start( ap, fmt );
    sl_va_format_quick( &sl, fmt, ap );
    va_end( ap );

    printf( "%s", sl );
}


void sl_write( const int fd, const char* fmt, ... )
{
    char mem[ 2048 ];
    sl_t sl;

    sl = sl_use( mem, 2048 );

    va_list ap;

    va_start( ap, fmt );
    sl_va_format_quick( &sl, fmt, ap );
    va_end( ap );

    write( fd, sl, sl_length( sl ) );
}


void sl_dump( sl_t ss )
{
    printf( "%s\n", ss );
    printf( "  len: %d\n", sl_len( ss ) );
    printf( "  res: %d\n", sl_res( ss ) );
}


void sl_set_local( sl_t ss, int val )
{
    sl_base_p s = sl_base( ss );
    if ( val != 0 )
        s->res = s->res | 0x1;
    else
        s->res = s->res & sl_smsk;
}


int sl_get_local( sl_t ss )
{
    return sl_local( ss );
}


sr_s sr_new( const char* str, sl_size_t len )
{
    sr_s ret;
    ret.str = str;
    ret.len = len;
    return ret;
}


sr_s sr_new_c( const char* str )
{
    sr_s ret;
    ret.str = str;
    ret.len = strlen( str );
    return ret;
}

const char* sr_text( sr_s sr )
{
    return sr.str;
}

sl_size_t sr_length( sr_s sr )
{
    return sr.len;
}

int sr_compare( sr_s s1, sr_s s2 )
{
    if ( s1.len != s2.len ) {
        return 1;
    } else {
        return memcmp( s1.str, s2.str, s1.len );
    }
}

int sr_compare_full( sr_s s1, sr_s s2 )
{
    if ( s1.len != s2.len ) {
        size_t len;
        if ( s1.len < s2.len ) {
            len = s1.len;
        } else {
            len = s2.len;
        }
        return strncmp( s1.str, s2.str, len );
    } else {
        return strncmp( s1.str, s2.str, s1.len );
    }
}




/* ------------------------------------------------------------
 * Utility functions.
 * ------------------------------------------------------------ */


/**
 * Copy "src" to "dst" and return pointer to end of "dst".
 *
 * @param dst Destination str.
 * @param src Source str.
 *
 * @return Pointer to dst end.
 */
static char* sl_copy_setup( char* dst, const char* src )
{
    int i = 0;
    while ( src[ i ] ) {
        dst[ i ] = src[ i ];
        i++;
    }
    return &( dst[ i ] );
}


/**
 * Return file size or (-1 on error).
 *
 * @param filename Name of file.
 *
 * @return File size.
 */
static off_t sl_file_size( const char* filename )
{
    struct stat st;

    if ( stat( filename, &st ) == 0 )
        return st.st_size;

    return -1; // GCOV_EXCL_LINE
}


/**
 * Normalize (possibly negative) SL index. Positive index is saturated
 * to SL length, and negative index is normalized.
 *
 * -1 means last char in SL, -2 second to last, etc. Index after last
 * char can only be expressed by positive indeces. E.g. for SL with
 * length of 4 the indeces are:
 *
 * Chars:     a  b  c  d  \0
 * Positive:  0  1  2  3  4
 * Negative: -4 -3 -2 -1
 *
 * @param ss  SL.
 * @param idx Index to SL.
 *
 * @return Unsigned (positive) index to SL.
 */
static sl_size_t sl_norm_idx( sl_t ss, int idx )
{
    sl_size_t ret;

    if ( idx < 0 ) {
        ret = sl_len( ss ) + idx;
    } else if ( (sl_size_t)idx > sl_len( ss ) ) {
        ret = sl_len( ss );
    } else {
        ret = idx;
    }

    return ret;
}


/**
 * Copy s2 to s1.
 *
 * @param s1   SL target.
 * @param s2   Source string.
 * @param len1 s2 length + 1.
 *
 * @return SL.
 */
static sl_t sl_copy_base( sl_p s1, const char* s2, sl_size_t len1 )
{
    sl_reserve( s1, len1 );
    memcpy( *s1, s2, len1 );
    sl_len( *s1 ) = len1 - 1;
    return *s1;
}


/**
 * Compare two strings.
 *
 * @param s1 String 1.
 * @param s2 String 2.
 *
 * @return Return "strcmp" result.
 */
static int sl_compare_base( const void* s1, const void* s2 )
{
    return strcmp( *(char* const*)s1, *(char* const*)s2 );
}


/**
 * Concatenate s2 to s1.
 *
 * @param s1   SL target.
 * @param s2   Source string.
 * @param len1 s2 length + 1.
 *
 * @return SL.
 */
static sl_t sl_concatenate_base( sl_p s1, const char* s2, sl_size_t len1 )
{
    sl_reserve( s1, sl_len( *s1 ) + len1 );
    memcpy( sl_end( *s1 ), s2, len1 );
    sl_len( *s1 ) += len1 - 1;
    return *s1;
}


/**
 * Insert s2 of with length+1 to s1 into position pos.
 *
 * @param s1    String 1.
 * @param pos   Insert position.
 * @param s2    String 2.
 * @param len1  s2 length + 1.
 *
 * @return SL.
 */
static sl_t sl_insert_base( sl_p s1, int pos, const char* s2, sl_size_t len1 )
{
    sl_size_t len = sl_len( *s1 ) + len1;
    sl_reserve( s1, len );

    len1--;

    sl_size_t posn = sl_norm_idx( *s1, pos );

    /*
     *          tail
     *         /
     * abcd....efghi
     */

    sl_size_t tail = posn + len1;

    memmove( ( *s1 ) + tail, ( *s1 ) + posn, ( sl_len( *s1 ) - posn ) );
    memcpy( ( *s1 ) + posn, s2, len1 );
    sl_len( *s1 ) += len1;

    /* Terminate change SL. */
    sl_base_p s = sl_base( *s1 );
    s->str[ s->len ] = 0;


    return *s1;
}


/**
 * Divide SL by replacing "c" with 0. Count the number of segments and
 * assign "div" to point to start of each segment.
 *
 * If size is less than 0, count only the number of segments.
 *
 * @param ss   SL.
 * @param c    Division char.
 * @param size Segment limit (size of div).
 * @param div  Storage for segments.
 *
 * @return Number of segments.
 */
static int sl_divide_base( sl_t ss, char c, int size, char** div )
{
    int   divcnt = 0;
    char *a, *b;

    a = ss;
    b = ss;

    while ( *b ) {
        if ( *b == c ) {
            if ( size >= 0 )
                *b = 0;
            if ( divcnt < size ) {
                div[ divcnt ] = a;
                b += 1;
                a = b;
            }
            divcnt++;
        }
        b++;
    }

    if ( divcnt < size && size >= 0 )
        div[ divcnt ] = a;

    return divcnt + 1;
}


/**
 * Segment SL by replacing "sc" with 0. Count the number of segments
 * and assign "div" to point to start of each segment.
 *
 * If size is less than 0, count only the number of segments.
 *
 * @param ss   SL.
 * @param c    Division char.
 * @param size Segment limit (size of div).
 * @param div  Storage for segments.
 *
 * @return Number of segments.
 */
static int sl_segment_base( sl_t ss, const char* sc, int size, char** div )
{
    int   divcnt = 0;
    int   idx;
    int   len = strlen( sc );
    char *a, *b;

    a = ss;
    b = ss;

    while ( *a ) {
        idx = sl_find_index( a, sc );
        if ( idx >= 0 ) {
            b = a + idx;
            if ( size >= 0 )
                *b = 0;
            if ( divcnt < size ) {
                div[ divcnt ] = a;
            }
            b += len;
            a = b;
            divcnt++;
        } else {
            break;
        }
    }

    if ( divcnt < size && size >= 0 )
        div[ divcnt ] = a;

    return divcnt + 1;
}


/**
 * Calculate string length of u64 string conversion.
 *
 * @param u64 Integer to convert.
 *
 * @return Length.
 */
static sl_size_t sl_u64_str_len( uint64_t u64 )
{
    sl_size_t len = 0;

    do {
        u64 /= 10;
        len++;
    } while ( u64 != 0 );

    return len;
}



/**
 * Convert u64 to string.
 *
 * @param u64 Integer to convert.
 * @param str Storage for conversion.
 *
 * @return Storage position after conversion.
 */
static char* sl_u64_to_str( uint64_t u64, char* str )
{
    char* c;
    char* ret;

    c = str;
    do {
        *c++ = ( u64 % 10 ) + '0';
        u64 /= 10;
    } while ( u64 != 0 );

    *c = 0;
    ret = c;
    c--;

    /* Reverse the string. */
    char ch;
    while ( str < c ) {
        ch = *str;
        *str = *c;
        *c = ch;
        str++;
        c--;
    }

    return ret;
}


/**
 * Calculate string length of i64 string conversion.
 *
 * @param i64 Integer to convert.
 *
 * @return Length.
 */
static sl_size_t sl_i64_str_len( int64_t i64 )
{
    if ( i64 < 0 ) {
        return sl_u64_str_len( -i64 ) + 1;
    } else {
        return sl_u64_str_len( i64 );
    }
}


/**
 * Convert i64 to string.
 *
 * @param i64 Integer to convert.
 * @param str Storage for conversion.
 *
 * @return Storage position after conversion.
 */
static char* sl_i64_to_str( int64_t i64, char* str )
{
    if ( i64 < 0 ) {
        *str++ = '-';
        return sl_u64_to_str( -i64, str );
    } else {
        return sl_u64_to_str( i64, str );
    }
}


/**
 * Calculate the total length of variable number of strings.
 *
 * Argument list must be NULL terminated.
 *
 * @param first First string.
 * @param va    Rest of the strings.
 *
 * @return Length of all strings combined.
 */
static sl_size_t sl_va_str_len( va_list va )
{
    sl_size_t ret;
    char*     cs;

    ret = 0;

    while ( ( cs = va_arg( va, char* ) ) != NULL )
        ret += strlen( cs );

    return ret;
}

static char sl_char_is_special( char c )
{
    switch ( c ) {
        case '\a':
            return 'a';
        case '\b':
            return 'b';
        case '\f':
            return 'f';
        case '\n':
            return 'n';
        case '\r':
            return 'r';
        case '\v':
            return 'v';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '\?':
            return '?';
        default:
            return 0;
    }
}


static int sl_str_to_number( const char** str_p )
{
    int         i;
    int         ret;
    const char* str;

    str = *str_p;
    i = 0;
    ret = 0;
    while ( str[ i ] >= '0' && str[ i ] <= '9' ) {
        ret = 10 * ret + ( str[ i ] - '0' );
        i++;
    }

    *str_p += i;

    return ret;
}


#if 0
static sl_size_t sl_quick_size( const char ch, void* item )
{
    switch ( ch ) {
        case 's': return strlen( *((char**)item) );
        case 'S': return sl_len( *((char**)item) );
        case 'i': return sl_i64_str_len( *((int*)item) );
        case 'I': return sl_i64_str_len( *((int64_t*)item) );
        case 'u': return sl_u64_str_len( *((unsigned int*)item) );
        case 'U': return sl_u64_str_len( *((uint64_t*)item) );
        case 'c': return 1;
        case 'r': return ((sr_s*)item)->len;
        case '%': return 1;
        default: return 1;
    }
}
#endif


static sl_size_t sl_va_format_quick_item_size( const char ch, va_list ap )
{
    /* ------------------------------------------------------------
     * Calculate string size.
     */

    char*    ts;
    int64_t  i64;
    uint64_t u64;
    sr_s     sr;

    switch ( ch ) {

        case 's': {
            ts = va_arg( ap, char* );
            return strlen( ts );
        }

        case 'S': {
            ts = va_arg( ap, char* );
            return sl_len( ts );
        }

        case 'i': {
            i64 = va_arg( ap, int );
            return sl_i64_str_len( i64 );
        }

        case 'I': {
            i64 = va_arg( ap, int64_t );
            return sl_i64_str_len( i64 );
        }

        case 'u': {
            u64 = va_arg( ap, unsigned int );
            return sl_u64_str_len( u64 );
        }

        case 'U': {
            u64 = va_arg( ap, uint64_t );
            return sl_u64_str_len( u64 );
        }

        case 'c': {
            return 1;
        }

        case 'r': {
            sr = va_arg( ap, sr_s );
            return sr.len;
        }

        case '%': {
            return 1;
        }

        default: {
            return 1;
        }
    }
}


static sl_size_t sl_va_format_quick_size( const char* fmt, va_list ap )
{
    //     va_list coap;
    //
    //     /* Copy ap to coap for second va-call. */
    //     va_copy( coap, ap );

    sl_size_t size = 0;
    sl_size_t max_size = 0;


    /* ------------------------------------------------------------
     * Calculate string size.
     */

    const char* c;
    char*       ts;
    int64_t     i64;
    uint64_t    u64;
    sr_s        sr;

    c = fmt;

    while ( *c ) {

        switch ( *c ) {

            case '%': {
                c++;

                switch ( *c ) {

                    case '!': {
                        max_size = size;
                        size = 0;
                        break;
                    }

                    case 's': {
                        ts = va_arg( ap, char* );
                        size += strlen( ts );
                        break;
                    }

                    case 'S': {
                        ts = va_arg( ap, char* );
                        size += sl_len( ts );
                        break;
                    }

                    case 'i': {
                        i64 = va_arg( ap, int );
                        size += sl_i64_str_len( i64 );
                        break;
                    }

                    case 'I': {
                        i64 = va_arg( ap, int64_t );
                        size += sl_i64_str_len( i64 );
                        break;
                    }

                    case 'u': {
                        u64 = va_arg( ap, unsigned int );
                        size += sl_u64_str_len( u64 );
                        break;
                    }

                    case 'U': {
                        u64 = va_arg( ap, uint64_t );
                        size += sl_u64_str_len( u64 );
                        break;
                    }

                    case 'c': {
                        size++;
                        break;
                    }

                    case 'p': {
                        i64 = va_arg( ap, int );
                        if ( i64 > size )
                            size = i64;
                        break;
                    }

                    case 'r': {
                        sr = va_arg( ap, sr_s );
                        size += sr.len;
                        break;
                    }

                    case 'a': {
                        sl_size_t width;
                        sl_size_t nominal_size;

                        // %al012i

                        c += 3;
                        width = sl_str_to_number( &c );
                        nominal_size = sl_va_format_quick_item_size( *c, ap );
                        if ( width > nominal_size ) {
                            size += width;
                        } else {
                            size += nominal_size;
                        }
                        break;
                    }

                    case '%': {
                        size++;
                        break;
                    }

                    default: {
                        size++;
                        break;
                    }
                }

                c++;
                break;
            }

            default: {
                size++;
                c++;
                break;
            }
        }
    }

    va_end( ap );

    if ( max_size > size ) {
        size = max_size;
    }

    return size;
}


static void sl_va_format_quick_append( char** wpp, const char ch, va_list ap )
{

    sl_size_t   size;
    char*       ts;
    int64_t     i64;
    uint64_t    u64;
    sr_s        sr;
    char*       wp;

    /* ------------------------------------------------------------
     * Expand format string.
     */

    wp = *wpp;

    switch ( ch ) {

        case 's':
        case 'S': {
            ts = va_arg( ap, char* );
            if ( ch == 's' ) {
                size = strlen( ts );
            } else {
                size = sl_len( ts );
            }
            memcpy( wp, ts, size );
            wp += size;
            break;
        }

        case 'i':
        case 'I': {
            if ( ch == 'i' ) {
                i64 = va_arg( ap, int );
            } else {
                i64 = va_arg( ap, int64_t );
            }
            wp = sl_i64_to_str( i64, wp );
            break;
        }

        case 'u':
        case 'U': {
            if ( ch == 'u' ) {
                u64 = va_arg( ap, unsigned int );
            } else {
                u64 = va_arg( ap, uint64_t );
            }
            wp = sl_u64_to_str( u64, wp );
            break;
        }

        case 'c': {
            char ch;
            ch = (char)va_arg( ap, int );
            *wp++ = ch;
            break;
        }

        case 'r': {
            sr = va_arg( ap, sr_s );
            memcpy( wp, sr.str, sr.len );
            wp += sr.len;
            break;
        }

        default: break;
    }


    *wp = 0;
    *wpp = wp;
}
