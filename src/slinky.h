#ifndef SLINKY_H
#define SLINKY_H


/**
 * @file   slinky.h
 * @author Tero Isannainen <tero.isannainen@gmail.com>
 * @date   Sun Mar 18 16:14:49 2018
 *
 * @brief  Simple String Library.
 *
 */


#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef SLINKY_USE_MEMTUN
#include <memtun.h>
void sl_set_memtun( mt_t mt );
mt_t sl_get_memtun( void );
#endif

/** Framer library version. */
extern const char* slinky_version;


/* ------------------------------------------------------------
 * Type definitions:
 * ------------------------------------------------------------ */

/** Size type. */
typedef uint32_t sl_size_t;

/** Slinky structure. */
typedef struct
{
    sl_size_t res;      /**< String storage size. */
    sl_size_t len;      /**< Length (used). */
    char      str[ 0 ]; /**< String content. */
} sl_s;

/** Slinky reference. */
typedef struct
{
    const char* str; /**< String reference. */
    sl_size_t   len; /**< Length (used). */
} sr_s;
typedef sr_s* sr_t;


/** Pointer to Slinky. */
typedef sl_s* sl_base_p;

/** Type for Slinky String. */
typedef char* sl_t;

/** Handle for mutable Slinky. */
typedef sl_t* sl_p;

/** Slinky array type. */
typedef sl_t* sl_v;

/** @{ Extra Slinky type alises. */
typedef sl_base_p slb;
typedef sl_t      sls;
typedef sl_p      slp;
typedef sl_v      sla;
/** @} */


#define SR_NULL \
    {           \
        NULL, 0 \
    }
#define SR_INIT \
    ( sr_s )    \
    {           \
        NULL, 0 \
    }

/* clang-format off */

#ifdef SLINKY_USE_MEM_API

/*
 * SLINKY_USE_MEM_API allows to use custom memory allocation functions,
 * instead of the default: sl_malloc, sl_free, sl_realloc.
 *
 * If SLINKY_USE_MEM_API is used, the user must provide implementation for
 * the above functions and they must be compatible with malloc etc.
 *
 * Additionally user should compile the library by own means.
 */

extern void* sl_malloc( size_t size );
extern void  sl_free( void* ptr );
extern void* sl_realloc( void* ptr, size_t size );

#else /* SLINKY_USE_MEM_API */


#    if SIXTEN_USE_MEM_API == 1

#        define sl_malloc  st_alloc
#        define sl_free    st_del
#        define sl_realloc st_realloc


#    else /* SIXTEN_USE_MEM_API == 1 */

/* Default to regular memory management functions. */
/** @cond slinky_none */
#        define sl_malloc  malloc
#        define sl_free    free
#        define sl_realloc realloc
/** @endcond slinky_none */

#    endif /* SIXTEN_USE_MEM_API == 1 */

#endif /* SLINKY_USE_MEM_API */

/* clang-format on */



/** @cond slinky_none */

/* clang-format off */
#define slnew     sl_new
#define sluse     sl_use
#define sldel     sl_del
#define slde2     sl_del2
#define slres     sl_reserve
#define slcom     sl_compact
#define slcpy     sl_copy
#define slcpy_c   sl_copy_c
#define slach     sl_append_char
#define slacn     sl_append_n_char
#define slass     sl_append_substr
#define slast     sl_append_str
#define slasn     sl_append_n_str
#define slasr     sl_append_sr
#define slasv     sl_append_va_str
#define sldup     sl_duplicate
#define sldup_c   sl_duplicate_c
#define slrep     sl_replicate
#define sldrp     sl_drop
#define slclr     sl_clear
#define slstr_c   sl_from_str_c
#define slstv_c   sl_from_va_str_c
#define slsiz_c   sl_from_str_with_size_c
#define slref     sl_refresh
#define sllen     sl_length
#define slrss     sl_reservation_size
#define slptr     sl_base_ptr
#define slend     sl_end_char
#define slcmp     sl_compare
#define slsme     sl_is_same
#define sldff     sl_is_different
#define slsrt     sl_sort
#define slcat     sl_concatenate
#define slcat_c   sl_concatenate_c
#define slpsh     sl_push_char_to
#define slpop     sl_pop_char_from
#define sllim     sl_limit_to_pos
#define slcut     sl_cut
#define slsel     sl_select_slice
#define slins     sl_insert_to
#define slins_c   sl_insert_to_c
#define slfmt     sl_format
#define slvft     sl_va_format
#define slfmq     sl_format_quick
#define slvfq     sl_va_format_quick
#define slinv     sl_invert_pos
#define slfcr     sl_find_char_right
#define slfcl     sl_find_char_left
#define slidx     sl_find_index
#define sldiv     sl_divide_with_char
#define slseg     sl_segment_with_str
#define slglu     sl_glue_array
#define sltok     sl_tokenize
#define slext     sl_rm_extension
#define sldir     sl_directory_name
#define slbas     sl_basename
#define slswp     sl_swap_chars
#define slmap     sl_map_str
#define slcap     sl_capitalize
#define sltou     sl_toupper
#define sltol     sl_tolower
#define slrdf     sl_read_file
#define slwrf     sl_write_file
#define slprn     sl_print
#define slwrt     sl_write
#define sldmp     sl_dump
/* clang-format off */

/** @endcond slinky_none */



/* ------------------------------------------------------------
 * Library
 * ------------------------------------------------------------ */


/**
 * Create new Slinky.
 *
 * Storage size should be at least string length + 1.
 *
 * @param size String storage size.
 *
 * @return Slinky.
 */
sl_t sl_new( sl_size_t size );


/**
 * Use existing memory allocation for Slinky.
 *
 * "size" is for the whole Slinky, including descriptor and string
 * storage. Hence string storage is 8 bytes smaller that "size". Also
 * size must be an even value.
 *
 * @param mem   Allocation for Slinky.
 * @param size  Allocation size (even number).
 *
 * @return Slinky.
 */
sl_t sl_use( void* mem, sl_size_t size );


/**
 * Delete Slinky using reference.
 *
 * @param sp Pointer to Slinky.
 *
 * @return NULL
 */
sl_t sl_del( sl_p sp );


/**
 * Delete Slinky.
 *
 * @param ss Slinky.
 *
 * @return NA
 */
void sl_del2( sl_t ss );


/**
 * Update Slinky storage to size.
 *
 * If current storage is bigger, do nothing.
 *
 * @param sp   Pointer to Slinky.
 * @param size Storage size.
 *
 * @return Slinky.
 */
sl_t sl_reserve( sl_p sp, sl_size_t size );


/**
 * Compact storage to minimum size.
 *
 * Minimum is string length + 1.
 *
 * @param sp Pointer to Slinky.
 *
 * @return Slinky.
 */
sl_t sl_compact( sl_p sp );


/**
 * Copy Slinky content from another Slinky.
 *
 * @param s1 Pointer to Slinky.
 * @param s2 Slinky.
 *
 * @return Slinky.
 */
sl_t sl_copy( sl_p s1, sl_t s2 );


/**
 * Copy Slinky content from CSTR.
 *
 * @param s1 Pointer to Slinky.
 * @param s2 Slinky.
 *
 * @return Slinky.
 */
sl_t sl_copy_c( sl_p s1, const char* s2 );


/**
 * Append Slinky with character.
 *
 * @param sp  Pointer to Slinky.
 * @param c   Char to append.
 *
 * @return Slinky.
 */
sl_t sl_append_char( sl_p sp, char c );


/**
 * Append Slinky with character n times.
 *
 * @param sp  Pointer to Slinky.
 * @param c   Char to append.
 * @param n   Append count.
 *
 * @return Slinky.
 */
sl_t sl_append_n_char( sl_p sp, char c, sl_size_t n );


/**
 * Append characters from string taking "cnt" chars.
 *
 * @param sp   Pointer to Slinky.
 * @param cs   CSTR for appending.
 * @param clen Sub-string length.
 *
 * @return Slinky.
 */
sl_t sl_append_substr( sl_p sp, const char* cs, sl_size_t clen );


/**
 * Append string to Slinky.
 *
 * @param sp Pointer to Slinky.
 * @param cs CSTR.
 *
 * @return Slinky.
 */
sl_t sl_append_str( sl_p sp, const char* cs );


/**
 * Append string to Slinky n times.
 *
 * @param sp Pointer to Slinky.
 * @param cs CSTR.
 * @param n  Append count.
 *
 * @return Slinky.
 */
sl_t sl_append_n_str( sl_p sp, const char* cs, sl_size_t n );


/**
 * Append Pointer to Slinky to Slinky.
 *
 * @param sp Pointer to Slinky.
 * @param cs CSTR.
 * @param n  Append count.
 *
 * @return Slinky.
 */
sl_t sl_append_sr( sl_p sp, sr_s sr );


/**
 * Append many strings to Slinky.
 *
 * Variable amount of strings are appended. The argument list must be
 * terminated with NULL.
 *
 * @param sp Pointer to Slinky.
 * @param cs First CSTR.
 *
 * @return Slinky.
 */
sl_t sl_append_va_str( sl_p sp, const char* cs, ... );


/**
 * Duplicate Slinky, using same storage as original.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_duplicate( sl_t ss );


/**
 * Duplicate Slinky as CSTR.
 *
 * @param ss Slinky.
 *
 * @return CSTR.
 */
char* sl_duplicate_c( sl_t ss );


/**
 * Replicate (duplicate) Slinky, using mininum storage.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_replicate( sl_t ss );


/**
 * Change Slinky into normal CSTR.
 *
 * Memory allocation remains the same. Slinky content is moved to
 * base.
 *
 * The returned CSTR should be freed as any heap allocated string.
 *
 * @param ss Slinky.
 *
 * @return CSTR.
 */
char* sl_drop( sl_t ss );


/**
 * Clear content of Slinky.
 *
 * Set string length to 0. No change to storage.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_clear( sl_t ss );


/**
 * Create Slinky based on CSTR with size from CSTR.
 *
 * @param cs CSTR.
 *
 * @return Slinky.
 */
sl_t sl_from_str_c( const char* cs );


/**
 * Create Slinky based on CSTR with given len.
 *
 * @param cs  CSTR.
 * @param len CSTR length.
 *
 * @return Slinky.
 */
sl_t sl_from_len_c( const char* cs, sl_size_t clen );


/**
 * Create Slinky based on variable number of CSTR.
 *
 * @param cs First CSTR.
 *
 * @return Slinky.
 */
sl_t sl_from_va_str_c( const char* cs, ... );


/**
 * Create Slinky based on CSTR with given size.
 *
 * Size is enlarged if CSTR is longer than given size.
 *
 * @param cs    CSTR.
 * @param size  Storage size.
 *
 * @return Slinky.
 */
sl_t sl_from_str_with_size_c( const char* cs, sl_size_t size );


/**
 * Refresh Slinky length.
 *
 * Useful when some function has filled string content and it should
 * be in sync with the Slinky itself.
 *
 * @param sp Slinky.
 *
 * @return Slinky.
 */
sl_t sl_refresh( sl_t ss );


/**
 * Return Slinky length.
 *
 * @param ss Slinky.
 *
 * @return Length.
 */
sl_size_t sl_length( sl_t ss );


/**
 * Set Slinky length.
 *
 * @param ss  Slinky.
 * @param len New length.
 *
 * @return Slinky.
 */
sl_t sl_set_length( sl_t ss, sl_size_t len );


/**
 * Return Slinky storage size.
 *
 * Storage size is the string content storage.
 *
 * @param ss Slinky.
 *
 * @return Storage size.
 */
sl_size_t sl_reservation_size( sl_t ss );


/**
 * Return Slinky body size.
 *
 * Body size is the size of the Slinky non-string content storage,
 * i.e. the bookkeeping part.
 *
 * @return Body size.
 */
sl_size_t sl_body_size( void );


/**
 * Return normalized Slinky size.
 *
 * Body size is the size of the Slinky non-string content storage,
 * i.e. the bookkeeping part.
 *
 * @param size Suggested size.
 *
 * @return Normalized size.
 */
sl_size_t sl_normalize_size( sl_size_t size );


/**
 * Return Slinky base type.
 *
 * @param ss Slinky.
 *
 * @return Base type.
 */
slb sl_base_ptr( sl_t ss );


/**
 * Return last character in Slinky.
 *
 * @param ss Slinky.
 *
 * @return Last char (or NULL).
 */
char sl_end_char( sl_t ss );


/**
 * Compare two Slinky.
 *
 * @param s1 Reference Slinky.
 * @param s2 Compared Slinky.
 *
 * @return -1,0,1 (see strcmp).
 */
int sl_compare( sl_t s1, sl_t s2 );


/**
 * Are two Slinky strings same?
 *
 * @param s1 Reference Slinky.
 * @param s2 Compared Slinky.
 *
 * @return 1 if same.
 */
int sl_is_same( sl_t s1, sl_t s2 );


/**
 * Are two Slinky strings different?
 *
 * @param s1 Reference Slinky.
 * @param s2 Compared Slinky.
 *
 * @return 1 if different.
 */
int sl_is_different( sl_t s1, sl_t s2 );


/**
 * Sort Slinky array to alphabetical order.
 *
 * @param sa   Slinky array.
 * @param len  Slinky array length.
 */
void sl_sort( sl_v sa, sl_size_t len );


/**
 * Concatenate Slinky to Slinky.
 *
 * @param s1 Pointer to Slinky.
 * @param s2 Slinky to add.
 *
 * @return Slinky.
 */
sl_t sl_concatenate( sl_p s1, sl_t s2 );


/**
 * Concatenate CSTR to Slinky.
 *
 * @param s1 Pointer to Slinky.
 * @param s2 CSTR to add.
 *
 * @return Slinky.
 */
sl_t sl_concatenate_c( sl_p s1, const char* s2 );


/**
 * Push (insert) character to pos.
 *
 * Pos can be positive or negative.
 *
 * @param sp  Pointer to Slinky.
 * @param pos Pos.
 * @param c   Char to push.
 *
 * @return Slinky.
 */
sl_t sl_push_char_to( sl_p sp, int pos, char c );


/**
 * Pop (remove) character at pos.
 *
 * Pos can be positive or negative.
 *
 * @param ss  Slinky.
 * @param pos Pos.
 *
 * @return Slinky.
 */
sl_t sl_pop_char_from( sl_t ss, int pos );


/**
 * Cut to pos.
 *
 * Tail is removed and string length becomes same as pos.
 *
 * @param ss  Slinky.
 * @param pos Pos.
 *
 * @return Slinky.
 */
sl_t sl_limit_to_pos( sl_t ss, int pos );


/**
 * Cut off either end or start of string.
 *
 * With positive "cnt", cut off "cnt" characters from end.
 * With negative "cnt", cut off "cnt" characters from start.
 *
 * @param ss   Slinky.
 * @param cnt  Cut cnt.
 *
 * @return Slinky.
 */
sl_t sl_cut( sl_t ss, int cnt );


/**
 * Select a slice from Slinky and mutate Slinky.
 *
 * Positive index is from start and negative from end of
 * string. sl_select_slice() is not sensitive to the order of
 * boundaries. End index is exclusive.
 *
 * @param ss   Slinky.
 * @param a    A boundary.
 * @param b    B boundary.
 *
 * @return Slinky.
 */
sl_t sl_select_slice( sl_t ss, int a, int b );


/**
 * Insert Slinky into another Slinky.
 *
 * @param s1  Target Pointer to Slinky.
 * @param pos Insertion position.
 * @param s2  Inserted Slinky.
 *
 * @return Target.
 */
sl_t sl_insert_to( sl_p s1, int pos, sl_t s2 );


/**
 * Insert CSTR into Slinky.
 *
 * @param s1  Target Pointer to Slinky.
 * @param pos Insertion position.
 * @param s2  Inserted CSTR.
 *
 * @return Target.
 */
sl_t sl_insert_to_c( sl_p s1, int pos, const char* s2 );


/**
 * Unquote Slinky.
 *
 * Remove all escapes characters.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_unquote( sl_t ss );


/**
 * Quote Slinky.
 *
 * Add escape character (backslash) in front of special characters.
 *
 * @param sp Pointer to Slinky.
 *
 * @return Slinky.
 */
sl_t sl_quote( sl_p sp );


/**
 * Formatted (printf style) print to Slinky.
 *
 * @param sp   Pointer to Slinky.
 * @param fmt  Format.
 *
 * @return Slinky.
 */
sl_t sl_format( sl_p sp, const char* fmt, ... );


/**
 * Variable Arguments (VA) version of sl_format().
 *
 * @param sp  Pointer to Slinky.
 * @param fmt Format.
 * @param ap  VA list.
 *
 * @return Slinky.
 */
sl_t sl_va_format( sl_p sp, const char* fmt, va_list ap );


/**
 * Quick Formatted print to Slinky.
 *
 * Quick Format is close to printf format, but is significantly
 * reduced and faster.
 *
 *     %! = Reset.
 *     %s = C string.
 *     %S = Slinky string.
 *     %i = Integer.
 *     %I = 64-bit integer.
 *     %u = Unsigned integer.
 *     %U = Unsigned 64-bit integer.
 *     %c = Character.
 *     %p = Pad upto column.
 *     %r = Slinky Reference.
 *     %% = Literal '%'.
 *
 * @param sp   Pointer to Slinky.
 * @param fmt  Quick Format.
 *
 * @return Slinky.
 */
sl_t sl_format_quick( sl_p sp, const char* fmt, ... );


/**
 * Variable Arguments (VA) version of sl_format_quick().
 *
 * @param sp  Pointer to Slinky.
 * @param fmt Quick Format.
 * @param ap  VA list.
 *
 * @return Slinky.
 */
sl_t sl_va_format_quick( sl_p sp, const char* fmt, va_list ap );


/**
 * Invert position index.
 *
 * Positive index is converted to negative and vice versa. Logical
 * position is same for inverted and non-inverted indeces.
 *
 * @param ss  Slinky.
 * @param pos Pos.
 *
 * @return Inverted pos.
 */
int sl_invert_pos( sl_t ss, int pos );


/**
 * Find char towards right.
 *
 * @param ss  Slinky.
 * @param c   Char to find.
 * @param pos Search start pos.
 *
 * @return Pos (or -1 if not found).
 */
int sl_find_char_right( sl_t ss, char c, sl_size_t pos );


/**
 * Find char towards left.
 *
 * @param ss  Slinky.
 * @param c   Char to find.
 * @param pos Search start pos.
 *
 * @return Pos (or -1 if not found).
 */
int sl_find_char_left( sl_t ss, char c, sl_size_t pos );


/**
 * Find "s2" from "s1". Return position or -1 if not found.
 *
 * "s2" can be Slinky or CSTR.
 *
 * @param s1  Base.
 * @param s2  Find.
 *
 * @return Pos (or -1 if not found).
 */
int sl_find_index( sl_t s1, const char* s2 );


/**
 * Divide (split) Slinky to pieces by character "c".
 *
 * Return the number of pieces after split. Pieces are stored to array
 * pointed by "div". If character "c" hits the last char of Slinky, the
 * last piece will be of length 0.
 *
 * Slinky will be modified by replacing "c" with 0. This can be
 * cancelled with sl_swap_chars() or user can use a duplicate Slinky,
 * which does not require fixing.
 *
 * If called with "size" < 0, return only the number of parts. No
 * modification is done to Slinky.
 *
 * If called with "*div" != NULL, fill the pre-allocated "div".
 *
 * Otherwise construct pieces, and allocate storage for it. Storage
 * should be freed by the user when done. In this case, "size" is
 * dont-care and will be calculated for the user.
 *
 * @param ss   Slinky.
 * @param c    Char to split with.
 * @param size Size of div storage (-1 for na).
 * @param div  Address of div storage.
 *
 * @return Number of pieces.
 */
int sl_divide_with_char( sl_t ss, char c, int size, char*** div );


/**
 * Same as sl_divide_with_char() except segmentation (split) is done
 * using CSTR.
 *
 * Both sl_divide_with_char() and sl_segment_with_str() terminates the
 * segment with single 0.
 *
 * @param ss   Slinky.
 * @param sc   CSTR to split with.
 * @param size Size of div storage (-1 for na).
 * @param div  Address of div storage.
 *
 * @return Number of pieces.
 */
int sl_segment_with_str( sl_t ss, const char* sc, int size, char*** div );


/**
 * Glue (join) string array with string.
 *
 * @param sa   Str array.
 * @param size Str array size.
 * @param glu  Glue string.
 *
 * @return Slinky.
 */
sl_t sl_glue_array( sl_v sa, sl_size_t size, const char* glu );


/**
 * Split "ss" into tokens delimited by "delim".
 *
 * sl_tokenize() is called multiple times for each iteration
 * separately. "pos" holds the state between iterations.
 *
 * For first call "ss" should be itself and "*pos" should be NULL. For
 * subsequent calls only "*pos" will be considered.
 *
 * After last token, "*pos" will be set to "ss".
 *
 * Example:
 *   char* t, *pos, *delim = "XY";
 *   s = sl_ttr_c( "abXYabcXYc" );
 *   pos = NULL;
 *   t = sl_tokenize( s, delim, &pos );
 *   t = sl_tokenize( s, delim, &pos );
 *   t = sl_tokenize( s, delim, &pos );
 *
 * @param ss     Slinky.
 * @param delim  Token delimiter.
 * @param pos    Iteration state.
 *
 * @return Start of current token (or NULL if no token).
 */
char* sl_tokenize( sl_t ss, const char* delim, char** pos );


/**
 * Drop the extension "ext" from "ss".
 *
 * @param ss  Slinky.
 * @param ext Extension (i.e. file suffix).
 * @return Updated Slinky (or NULL if no ext found).
 */
sl_t sl_rm_extension( sl_t ss, const char* ext );


/**
 * Change to dirname, i.e. take out the basename.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_directory_name( sl_t ss );


/**
 * Change to basename (file basename), i.e. take out the directory
 * part.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_basename( sl_t ss );


/**
 * Swap (repair) Slinky by mapping "f" char to "t" char.
 *
 * Useful to cleanup after sl_divide_with_char() or
 * sl_segment_with_str().
 *
 * @param ss Slinky.
 * @param f  From char.
 * @param t  To char.
 *
 * @return Slinky.
 */
sl_t sl_swap_chars( sl_t ss, char f, char t );


/**
 * Map (replace) string "f" to "t" in "ss".
 *
 * "f" and "t" can be either Slinky or CSTR.
 *
 * @param sp Pointer to Slinky.
 * @param f  From string.
 * @param t  To string.
 *
 * @return Slinky
 */
sl_t sl_map_str( sl_p sp, const char* f, const char* t );


/**
 * Map (replace) part of Slinky with "to".
 *
 * @param sp      Pointer to Slinky.
 * @param from_a  Replace part start index.
 * @param from_b  Replace part end index (exclusive).
 * @param to      New part.
 * @param to_len  New part length.
 *
 * @return Slinky
 */
sl_t sl_map_part( sl_p sp, sl_size_t from_a, sl_size_t from_b, const char* to, sl_size_t to_len );


/**
 * Capitalize Slinky, i.e. upcase the first letter.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_capitalize( sl_t ss );


/**
 * Convert Slinky to upper case letters.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_toupper( sl_t ss );


/**
 * Convert Slinky to lower case letters.
 *
 * @param ss Slinky.
 *
 * @return Slinky.
 */
sl_t sl_tolower( sl_t ss );


/**
 * Read complete file and return Slinky containing the file content.
 *
 * @param filename Name of file.
 *
 * @return Slinky.
 */
sl_t sl_read_file( const char* filename );


/**
 * Read complete file and return Slinky containing the file content.
 *
 * Returned Slinky starts with left number of zeros and the left zeros
 * are also part of Slinky length. Right padding adds only to the
 * allocation, but does affect the Slinky length. Typical usage has
 * left=0 and right>0.
 *
 * @param filename Name of file.
 * @param left     Number of extra bytes for left pad.
 * @param right    Number of extra bytes for right pad.
 *
 * @return Slinky.
 */
sl_t sl_read_file_with_pad( const char* filename, sl_size_t left, sl_size_t right );
// sl_t sl_read_file_plus_extra( const char* filename, sl_size_t extra );


/**
 * Write Slinky content to file.
 *
 * @param ss       Slinky including file content.
 * @param filename File to write to.
 *
 * @return Slinky.
 */
sl_t sl_write_file( sl_t ss, const char* filename );


/**
 * Print Slinky.
 *
 * Clear Slinky content.
 *
 * @param ss Slinky.
 */
// void sl_print( sl_t ss );
void sl_print( const char* fmt, ... );


/**
 * Print Slinky with write to file.
 *
 * Clear Slinky content.
 *
 * @param ss Slinky.
 */
void sl_write( const int fd, const char* fmt, ... );


/**
 * Display Slinky content.
 *
 * @param ss Slinky.
 */
void sl_dump( sl_t ss );


/**
 * Set Slinky as local.
 *
 * Slinky is not freed.
 *
 * @param ss  Slinky.
 * @param val Local value.
 */
void sl_set_local( sl_t ss, int val );


/**
 * Return Slinky local mode.
 *
 * Current allocation is not freed if resize occurs.
 *
 * @param ss Slinky.
 *
 * @return 1 if local (else 0).
 */
int sl_get_local( sl_t ss );


/**
 * Return Slinky Reference.
 *
 * @param str CSTR to string start.
 * @param len Length of the reference.
 *
 * @return Slinky Reference.
 */
sr_s sr_new( const char* str, sl_size_t len );


/**
 * Return Slinky Reference.
 *
 * @param str CSTR to string start (with NULL terminator).
 *
 * @return Slinky Reference.
 */
sr_s sr_new_c( const char* str );


/**
 * Return Slinky Reference string content.
 *
 * @param sr Slinky Reference.
 *
 * @return CSTR content.
 */
const char* sr_text( sr_s sr );


/**
 * Return Slinky Reference string length.
 *
 * @param sr Slinky Reference.
 *
 * @return Length of CSTR content.
 */
sl_size_t sr_length( sr_s sr );


/**
 * Compare two Slinky References.
 *
 * @param s1  One.
 * @param s2  The other.
 *
 * @return 0 for match, and non-zero for mismatch.
 */
int sr_compare( sr_s s1, sr_s s2 );


/**
 * Compare two Slinky References upto shared lenght.
 *
 * @param s1  One.
 * @param s2  The other.
 *
 * @return 0 for match, and non-zero for mismatch.
 */
int sr_compare_full( sr_s s1, sr_s s2 );


#endif
