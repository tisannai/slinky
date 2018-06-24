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


/* clang-format off */

#ifdef SLINKY_MEM_API

/*
 * SL_MEM_API allows to use custom memory allocation functions,
 * instead of the default: sl_malloc, sl_free, sl_realloc.
 *
 * If SL_MEM_API is used, the user must provide implementation for the
 * above functions and they must be compatible with malloc etc.
 *
 * Additionally user should compile the library by own means.
 */

extern void* sl_malloc( size_t size );
extern void sl_free( void* ptr );
extern void* sl_realloc( void* ptr, size_t size );

#else /* SLINKY_MEM_API */


#    if SIXTEN_USE_MEM == 1

#        define sl_malloc st_alloc
#        define sl_free st_del
#        define sl_realloc st_realloc


#    else /* SIXTEN_USE_MEM == 1 */

/* Default to regular memory management functions. */
/** @cond slinky_none */
#        define sl_malloc malloc
#        define sl_free free
#        define sl_realloc realloc
/** @endcond slinky_none */

#    endif /* SIXTEN_USE_MEM == 1 */

#endif /* SLINKY_MEM_API */

/* clang-format on */



/** @cond slinky_none */

/* clang-format off */
#define slnew     sl_new
#define sluse     sl_use
#define sldel     sl_del
#define slres     sl_reserve
#define slcom     sl_compact
#define slcpy     sl_copy
#define slcpy_c   sl_copy_c
#define slfil     sl_fill_with_char
#define slmul     sl_multiple_str_append
#define sldup     sl_duplicate
#define sldup_c   sl_duplicate_c
#define slrep     sl_replicate
#define slclr     sl_clear
#define slstr_c   sl_from_str_c
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
 * Delete Slinky.
 *
 * @param sp Slinky Reference.
 *
 * @return NULL
 */
sl_t sl_del( sl_p sp );


/**
 * Update Slinky storage to size.
 *
 * If current storage is bigger, do nothing.
 *
 * @param sp   Slinky Reference.
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
 * @param sp Slinky Reference.
 *
 * @return Slinky.
 */
sl_t sl_compact( sl_p sp );


/**
 * Copy Slinky content from another Slinky.
 *
 * @param s1 Slinky Reference.
 * @param s2 Slinky.
 *
 * @return Slinky.
 */
sl_t sl_copy( sl_p s1, sl_t s2 );


/**
 * Copy Slinky content from CSTR.
 *
 * @param s1 Slinky Reference.
 * @param s2 Slinky.
 *
 * @return Slinky.
 */
sl_t sl_copy_c( sl_p s1, char* s2 );


/**
 * Fill (append) Slinky with character by "cnt" times.
 *
 * @param sp  Slinky Reference.
 * @param c   Char for filling.
 * @param cnt Fill count.
 *
 * @return Slinky.
 */
sl_t sl_fill_with_char( sl_p sp, char c, sl_size_t cnt );


/**
 * Fill (append) Slinky with string by "cnt" times.
 *
 * @param sp  Slinky Reference.
 * @param cs  CSTR for filling.
 * @param cnt Fill count.
 *
 * @return Slinky.
 */
sl_t sl_multiple_str_append( sl_p sp, char* cs, sl_size_t cnt );


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
sl_t sl_from_str_c( char* cs );


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
sl_t sl_from_str_with_size_c( char* cs, sl_size_t size );


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
 * @param s1 Slinky Reference.
 * @param s2 Slinky to add.
 *
 * @return Slinky.
 */
sl_t sl_concatenate( sl_p s1, sl_t s2 );


/**
 * Concatenate CSTR to Slinky.
 *
 * @param s1 Slinky Reference.
 * @param s2 CSTR to add.
 *
 * @return Slinky.
 */
sl_t sl_concatenate_c( sl_p s1, char* s2 );


/**
 * Push (insert) character to pos.
 *
 * Pos can be positive or negative.
 *
 * @param sp  Slinky Reference.
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
 * string. sl_tel() is not sensitive to the order of boundaries. End
 * index is exclusive.
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
 * @param s1  Target Slinky Reference.
 * @param pos Insertion position.
 * @param s2  Inserted Slinky.
 *
 * @return Target.
 */
sl_t sl_insert_to( sl_p s1, int pos, sl_t s2 );


/**
 * Insert CSTR into Slinky.
 *
 * @param s1  Target Slinky Reference.
 * @param pos Insertion position.
 * @param s2  Inserted CSTR.
 *
 * @return Target.
 */
sl_t sl_insert_to_c( sl_p s1, int pos, char* s2 );


/**
 * Formatted (printf style) print to Slinky.
 *
 * @param sp   Slinky Reference.
 * @param fmt  Format.
 *
 * @return Slinky.
 */
sl_t sl_format( sl_p sp, const char* fmt, ... );


/**
 * Variable Arguments (VA) version of slfmt().
 *
 * @param sp  Slinky Reference.
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
 *     %s = C string.
 *     %S = Slinky string.
 *     %i = Integer.
 *     %I = 64-bit integer.
 *     %u = Unsigned integer.
 *     %U = Unsigned 64-bit integer.
 *     %c = Character.
 *     %% = Literal '%'.
 *
 * @param sp   Slinky Reference.
 * @param fmt  Quick Format.
 *
 * @return Slinky.
 */
sl_t sl_format_quick( sl_p sp, const char* fmt, ... );


/**
 * Variable Arguments (VA) version of slfmq().
 *
 * @param sp  Slinky Reference.
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
int sl_find_index( sl_t s1, char* s2 );


/**
 * Divide (split) Slinky to pieces by character "c".
 *
 * Return the number of pieces after split. Pieces are stored to array
 * pointed by "div". If character "c" hits the last char of Slinky, the
 * last piece will be of length 0.
 *
 * Slinky will be modified by replacing "c" with 0. This can be cancelled
 * with sl_twp() or user can use a duplicate Slinky, which does not require
 * fixing.
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
 * Same as sldiv() except segmentation (split) is done using CSTR.
 *
 * Both sldiv() and sl_teg() terminates the segment with single 0.
 *
 * @param ss   Slinky.
 * @param sc   CSTR to split with.
 * @param size Size of div storage (-1 for na).
 * @param div  Address of div storage.
 *
 * @return Number of pieces.
 */
int sl_segment_with_str( sl_t ss, char* sc, int size, char*** div );


/**
 * Glue (join) string array with string.
 *
 * @param sa   Str array.
 * @param size Str array size.
 * @param glu  Glue string.
 *
 * @return Slinky.
 */
sl_t sl_glue_array( sl_v sa, sl_size_t size, char* glu );


/**
 * Split "ss" into tokens delimited by "delim".
 *
 * sltok() is called multiple times for each iteration
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
 *   t = sltok( s, delim, &pos );
 *   t = sltok( s, delim, &pos );
 *   t = sltok( s, delim, &pos );
 *
 * @param ss     Slinky.
 * @param delim  Token delimiter.
 * @param pos    Iteration state.
 *
 * @return Start of current token (or NULL if no token).
 */
char* sl_tokenize( sl_t ss, char* delim, char** pos );


/**
 * Drop the extension "ext" from "ss".
 *
 * @param ss  Slinky.
 * @param ext Extension (i.e. file suffix).
 * @return Updated Slinky (or NULL if no ext found).
 */
sl_t sl_rm_extension( sl_t ss, char* ext );


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
 * Useful to cleanup after sldiv() or sl_teg().
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
 * @param sp Slinky Reference.
 * @param f  From string.
 * @param t  To string.
 *
 * @return Slinky
 */
sl_t sl_map_str( sl_p sp, char* f, char* t );


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
 * Write Slinky content to file.
 *
 * @param ss       Slinky including file content.
 * @param filename File to write to.
 *
 * @return Slinky.
 */
sl_t sl_write_file( sl_t ss, const char* filename );


/**
 * Display Slinky content.
 *
 * @param ss Slinky.
 */
void sl_print( sl_t ss );


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
 * Current allocatin is not freed if resize occurs.
 *
 * @param ss Slinky.
 *
 * @return 1 if local (else 0).
 */
int sl_get_local( sl_t ss );


#endif
