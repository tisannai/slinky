#ifdef SLINKY_USE_MEMTUN
# include <memtun.h>
#endif

#include "unity.h"
#include "slinky.h"
#include <string.h>
#include <unistd.h>


void test_basics( void )
{
    sl_t   s, s2;
    char* t1 = "text1";
    char* sd;

//#ifdef SLINKY_MEM_API
//    sl_cfg_alloc( sl_malloc_f, sl_free_f, sl_realloc_f );
//#endif

#ifdef SLINKY_USE_MEMTUN
    sl_set_memtun( mt_new_std() );
#endif

    s = slnew( 128 );

    slcpy_c( &s, t1 );
    TEST_ASSERT_TRUE( !strcmp( s, t1 ) );
    TEST_ASSERT( slrss( s ) == 128 );
    TEST_ASSERT( sllen( s ) == 5 );

    slcom( &s );
    TEST_ASSERT( slrss( s ) == 6 );
    TEST_ASSERT( sllen( s ) == 5 );

    slcpy( &s, s );
    TEST_ASSERT( slrss( s ) == 6 );
    TEST_ASSERT( sllen( s ) == 5 );

    slcat( &s, s );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 10 );

    s2 = sldup( s );
    TEST_ASSERT_TRUE( !slcmp( s, s2 ) );
    sldel( &s2 );

    sd = sldup_c( s );
    TEST_ASSERT_TRUE( !slcmp( s, sd ) );
    sl_free( sd );

    s2 = slrep( s );
    TEST_ASSERT_TRUE( !sldff( s, s2 ) );
    TEST_ASSERT_TRUE( slsme( s, s2 ) );
    slach( &s2, 'a' );
    TEST_ASSERT_TRUE( sldff( s, s2 ) );
    TEST_ASSERT_FALSE( slsme( s, s2 ) );
    slpop( s2, 0 );
    TEST_ASSERT_TRUE( sldff( s, s2 ) );
    /* "ext1text1a" */
    /*      ^       */
    slpsh( &s2, -6, 'K' );
    TEST_ASSERT_TRUE( !slcmp( s2, "ext1Ktext1a" ) );

    slclr( s2 );
    slasn( &s2, t1, 3 );
    TEST_ASSERT_TRUE( !slcmp( s2, "text1text1text1" ) );
    slclr( s2 );
    slass( &s2, t1, 3 );
    TEST_ASSERT_TRUE( !slcmp( s2, "tex" ) );
    slclr( s2 );
    slast( &s2, t1 );
    TEST_ASSERT_TRUE( !slcmp( s2, "text1" ) );
    slclr( s2 );
    slasv( &s2, t1, t1, NULL );
    TEST_ASSERT_TRUE( !slcmp( s2, "text1text1" ) );
    slclr( s2 );
    slasv( &s2, NULL );
    TEST_ASSERT_TRUE( !slcmp( s2, "" ) );
    sldel( &s2 );

    TEST_ASSERT( slend( s ) == '1' );
    slclr( s );
    TEST_ASSERT( slend( s ) == 0 );
    sldel( &s );

    s2 = slsiz_c( t1, 2 );
    TEST_ASSERT_TRUE( !strcmp( s2, "text1" ) );
    TEST_ASSERT( slrss( s2 ) == 6 );
    TEST_ASSERT( sllen( s2 ) == 5 );
    sl_base_p s2sl;
    s2sl = slptr( s2 );
    TEST_ASSERT( !strcmp( s2sl->str, "text1" ) );
    TEST_ASSERT( s2sl->res == 6 );
    TEST_ASSERT( s2sl->len == 5 );
    sldel( &s2 );

    char buf[ 24 ];
    s = sluse( buf, 24 );
    slcpy_c( &s, t1 );
    slcat( &s, s );
    slcat_c( &s, t1 );
    TEST_ASSERT_TRUE( !strcmp( s, "text1text1text1" ) );
    TEST_ASSERT_TRUE( sl_get_local( s ) );
    slcat_c( &s, t1 );
    TEST_ASSERT_FALSE( sl_get_local( s ) );
    sl_set_local( s, 1 );
    TEST_ASSERT_TRUE( sl_get_local( s ) );
    sl_set_local( s, 0 );
    TEST_ASSERT_FALSE( sl_get_local( s ) );
    sldel( &s );

    s = sluse( buf, 24 );
    sldel( &s );

    s = slstr_c( t1 );
    sd = sldrp( s );
    TEST_ASSERT_TRUE( !strcmp( sd, "text1" ) );
    sl_free( sd );

    s = slstv_c( t1, t1, t1, NULL );
    TEST_ASSERT_TRUE( !strcmp( s, "text1text1text1" ) );
    slde2( s );

    s = slstv_c( NULL );
    TEST_ASSERT_TRUE( s == NULL );

#ifdef SLINKY_USE_MEMTUN
    sl_free( sl_get_memtun() );
#else 
    sl_free( s );
#endif
}


void test_sizing( void )
{
    sl_t s;

    s = slnew( 128 );

    slres( &s, 64 );
    TEST_ASSERT( slrss( s ) == 128 );

    slres( &s, 128 );
    TEST_ASSERT( slrss( s ) == 128 );

    slres( &s, 129 );
    TEST_ASSERT( slrss( s ) == 130 );

    slcom( &s );
    TEST_ASSERT( slrss( s ) == 2 );

    slres( &s, 64 );
    TEST_ASSERT( slrss( s ) == 64 );

    sldel( &s );
}


void test_content( void )
{
    sl_t s;

    char* t1 = "text1";

    s = slstr_c( t1 );
    TEST_ASSERT_TRUE( !strcmp( s, t1 ) );
    TEST_ASSERT( slrss( s ) == 6 );
    TEST_ASSERT( sllen( s ) == 5 );
    sldel( &s );

    s = slsiz_c( t1, 12 );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 5 );

    slcat( &s, s );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 10 );

    slcat_c( &s, t1 );
    TEST_ASSERT( slrss( s ) == 16 );
    TEST_ASSERT( sllen( s ) == 15 );

    slcut( s, 2 );
    TEST_ASSERT_TRUE( !strcmp( s, "text1text1tex" ) );
    TEST_ASSERT( slrss( s ) == 16 );
    TEST_ASSERT( sllen( s ) == 13 );

    slcut( s, -2 );
    TEST_ASSERT_TRUE( !strcmp( s, "xt1text1tex" ) );
    TEST_ASSERT( slrss( s ) == 16 );
    TEST_ASSERT( sllen( s ) == 11 );

    sl_t s2;
    s2 = slsel( sldup( s ), 1, -2 );
    TEST_ASSERT_TRUE( !strcmp( s2, "t1text1t" ) );
    TEST_ASSERT( slrss( s2 ) == 16 );
    TEST_ASSERT( sllen( s2 ) == 8 );
    sldel( &s2 );

    s2 = slsel( slrep( s ), -2, 1 );
    TEST_ASSERT_TRUE( !strcmp( s2, "t1text1t" ) );
    TEST_ASSERT( slrss( s2 ) == 12 );
    TEST_ASSERT( sllen( s2 ) == 8 );

    int pos;
    pos = 2;
    pos = slinv( s2, pos );
    TEST_ASSERT( pos == -6 );
    pos = slinv( s2, pos );
    TEST_ASSERT( pos == 2 );

    sllim( s2, 1 );
    TEST_ASSERT_TRUE( !strcmp( s2, "t" ) );
    TEST_ASSERT( slrss( s2 ) == 12 );
    TEST_ASSERT( sllen( s2 ) == 1 );
    sldel( &s2 );

    slcpy_c( &s, t1 );
    slfmt( &s, "__%s_", t1 );
    TEST_ASSERT_TRUE( !strcmp( s, "text1__text1_" ) );
    TEST_ASSERT( slrss( s ) == 16 );
    TEST_ASSERT( sllen( s ) == 13 );

    slclr( s );
    slfmt( &s, "__%s_", t1 );
    TEST_ASSERT_TRUE( !strcmp( s, "__text1_" ) );
    TEST_ASSERT( slrss( s ) == 16 );
    TEST_ASSERT( sllen( s ) == 8 );

    slacn( &s, 'a', 10 );
    TEST_ASSERT_TRUE( !strcmp( s, "__text1_aaaaaaaaaa" ) );
    TEST_ASSERT( slrss( s ) == 20 );
    TEST_ASSERT( sllen( s ) == 18 );

    slclr( s );
    slacn( &s, 'a', 10 );
    TEST_ASSERT_TRUE( !strcmp( s, "aaaaaaaaaa" ) );
    TEST_ASSERT( slrss( s ) == 20 );
    TEST_ASSERT( sllen( s ) == 10 );

    slclr( s );
    slfmq( &s, "_%s_%i_%I_%u_%U_%c_%%_%X", t1, -123456, 654321, 123456789, 9876543210, 'X' );
    TEST_ASSERT_TRUE( !strcmp( s, "_text1_-123456_654321_123456789_9876543210_X_%_X" ) );

    s2 = slnew( 0 );
    slfmq( &s2, "%S%S", s, s );
    TEST_ASSERT_TRUE( !strcmp( s2,
                               "_text1_-123456_654321_123456789_9876543210_X_%_X"
                               "_text1_-123456_654321_123456789_9876543210_X_%_X" ) );

    slclr( s );
    slfmq( &s, "_%s_%p", t1, 10 );
    TEST_ASSERT_TRUE( !strcmp( s, "_text1_   " ) );

    slclr( s );
    slfmq( &s, "_%s_%p", t1, 7 );
    TEST_ASSERT_TRUE( !strcmp( s, "_text1_" ) );

    sldel( &s );
    sldel( &s2 );
}


void test_insert( void )
{
    sl_t   s, s2;
    char* t1 = "text1";

    s = slstr_c( t1 );
    s2 = slstr_c( t1 );

    slins_c( &s, 0, t1 );
    slins_c( &s2, 0, t1 );
    TEST_ASSERT_TRUE( !strcmp( s, "text1text1" ) );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 10 );

    //slins( &s, 128, s );
    slins( &s, 5, s2 );
    //slins( &s, 5, s );
    TEST_ASSERT_TRUE( !strcmp( s, "text1text1text1text1" ) );
    TEST_ASSERT( slrss( s ) == 22 );
    TEST_ASSERT( sllen( s ) == 20 );

    sldel( &s );
    sldel( &s2 );

    s = slstr_c( t1 );
    strcpy( s, "foo" );
    TEST_ASSERT( sllen( s ) == 5 );
    sl_refresh( s );
    TEST_ASSERT( sllen( s ) == 3 );
    slde2( s );
}


void test_examine( void )
{
    sl_t s;
    int pos;

    char* t1 = "abcdefghijkl";

    s = slstr_c( t1 );

    pos = slfcr( s, 'a', 0 );
    TEST_ASSERT( pos == 0 );

    pos = slfcr( s, 'e', 10 );
    TEST_ASSERT( pos == -1 );

    pos = slfcr( s, 'l', 10 );
    TEST_ASSERT( pos == 11 );


    pos = slfcl( s, 'a', 0 );
    TEST_ASSERT( pos == 0 );

    pos = slfcl( s, 'a', 5 );
    TEST_ASSERT( pos == 0 );

    pos = slfcl( s, 'l', 5 );
    TEST_ASSERT( pos == -1 );


    pos = slidx( s, "a" );
    TEST_ASSERT( pos == 0 );

    pos = slidx( s, "b" );
    TEST_ASSERT( pos == 1 );

    pos = slidx( s, "k" );
    TEST_ASSERT( pos == 10 );

    pos = slidx( s, "l" );
    TEST_ASSERT( pos == 11 );

    pos = slidx( s, "ab" );
    TEST_ASSERT( pos == 0 );

    pos = slidx( s, "kl" );
    TEST_ASSERT( pos == 10 );

    /* Invalid search. */
    pos = slidx( s, "" );
    TEST_ASSERT( pos == -1 );
}


void test_pieces( void )
{
    sl_t s, s2;

    int cnt;

    s = slstr_c( "XYabcXYabcXY" );
    TEST_ASSERT( slrss( s ) == 14 );
    TEST_ASSERT( sllen( s ) == 12 );

    cnt = sldiv( s, 'X', -1, NULL );
    TEST_ASSERT( cnt == 4 );

    cnt = sldiv( s, 'Y', -1, NULL );
    TEST_ASSERT( cnt == 4 );

    cnt = sldiv( s, 'a', -1, NULL );
    TEST_ASSERT( cnt == 3 );

    char** pcs;

    /* ------------------------------------------------------------
     * slsrt
     */
    pcs = NULL;
    cnt = sldiv( s, 'X', 0, &pcs );
    slsrt( pcs, cnt );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "Y" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "Yabc" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 3 ], "Yabc" ) );
    slswp( s, 0, 'X' );
    sl_free( pcs );

    /* ------------------------------------------------------------
     * sldiv
     */
    pcs = NULL;
    cnt = sldiv( s, 'X', 0, &pcs );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "Yabc" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "Yabc" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 3 ], "Y" ) );
    s2 = slglu( pcs, cnt, "H" );
    TEST_ASSERT_TRUE( !strcmp( s2, "HYabcHYabcHY" ) );
    TEST_ASSERT( slrss( s2 ) == 14 );
    TEST_ASSERT( sllen( s2 ) == 12 );
    slswp( s, 0, 'X' );
    sl_free( pcs );

    pcs = NULL;
    cnt = sldiv( s, 'Y', 0, &pcs );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "X" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "abcX" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "abcX" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 3 ], "" ) );
    s2 = slglu( pcs, cnt, "H" );
    TEST_ASSERT_TRUE( !strcmp( s2, "XHabcXHabcXH" ) );
    TEST_ASSERT( slrss( s2 ) == 14 );
    TEST_ASSERT( sllen( s2 ) == 12 );
    slswp( s, 0, 'Y' );
    sl_free( pcs );

    pcs = NULL;
    cnt = sldiv( s, 'a', 0, &pcs );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "XY" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "bcXY" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "bcXY" ) );
    slswp( s, 0, 'a' );
    sl_free( pcs );

    {
        char*  spc[ cnt ];
        char** help = spc;
        cnt = sldiv( s, 'a', -1, NULL );
        sldiv( s, 'a', cnt, &help );
        TEST_ASSERT_TRUE( !strcmp( spc[ 0 ], "XY" ) );
        TEST_ASSERT_TRUE( !strcmp( spc[ 1 ], "bcXY" ) );
        TEST_ASSERT_TRUE( !strcmp( spc[ 2 ], "bcXY" ) );
        s2 = slglu( spc, cnt, "A" );
        TEST_ASSERT_TRUE( !strcmp( s2, "XYAbcXYAbcXY" ) );
        TEST_ASSERT( slrss( s2 ) == 14 );
        TEST_ASSERT( sllen( s2 ) == 12 );
        slswp( s, 0, 'a' );
    }


    /* ------------------------------------------------------------
     * slseg
     */
    pcs = NULL;
    cnt = slseg( s, "XY", 0, &pcs );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "abc" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "abc" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 3 ], "" ) );
    s2 = slglu( pcs, cnt, "H" );
    TEST_ASSERT_TRUE( !strcmp( s2, "HabcHabcH" ) );
    TEST_ASSERT( slrss( s2 ) == 10 );
    TEST_ASSERT( sllen( s2 ) == 9 );
    slswp( s, 0, 'X' );
    sl_free( pcs );

    pcs = NULL;
    cnt = slseg( s, "a", 0, &pcs );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 0 ], "XY" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 1 ], "bcXY" ) );
    TEST_ASSERT_TRUE( !strcmp( pcs[ 2 ], "bcXY" ) );
    slswp( s, 0, 'a' );
    sl_free( pcs );

    {
        char*  spc[ cnt ];
        char** help = spc;
        cnt = slseg( s, "a", -1, NULL );
        slseg( s, "a", cnt, &help );
        TEST_ASSERT_TRUE( !strcmp( spc[ 0 ], "XY" ) );
        TEST_ASSERT_TRUE( !strcmp( spc[ 1 ], "bcXY" ) );
        TEST_ASSERT_TRUE( !strcmp( spc[ 2 ], "bcXY" ) );
        s2 = slglu( spc, cnt, "A" );
        TEST_ASSERT_TRUE( !strcmp( s2, "XYAbcXYAbcXY" ) );
        TEST_ASSERT( slrss( s2 ) == 14 );
        TEST_ASSERT( sllen( s2 ) == 12 );
        slswp( s, 0, 'a' );
    }

    sl_free( pcs );
}


void test_tok( void )
{
    sl_t   s;
    char *t, *pos, *delim = "XY";

    s = slstr_c( "XYabXYabcXYc" );
    pos = NULL;
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "ab" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "abc" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "c" ) );

    t = sltok( s, delim, &pos );
    TEST_ASSERT( t == NULL );

    sldel( &s );

    s = slstr_c( "XYabXYabcXYcXY" );
    pos = NULL;
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "ab" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "abc" ) );
    t = sltok( s, delim, &pos );
    TEST_ASSERT_TRUE( !strcmp( t, "c" ) );

    t = sltok( s, delim, &pos );
    TEST_ASSERT( t == NULL );

    sldel( &s );

    s = slstr_c( "XYabXYabcXYcXY" );
    pos = NULL;
    t = sltok( s, "foo", &pos );
    TEST_ASSERT( t == NULL );

    sldel( &s );
}


void test_map( void )
{
    sl_t s;

    s = slstr_c( "XYabcXYabcXY" );
    TEST_ASSERT( slrss( s ) == 14 );
    TEST_ASSERT( sllen( s ) == 12 );

    slmap( &s, "XY", "GIG" );
    TEST_ASSERT_TRUE( !strcmp( s, "GIGabcGIGabcGIG" ) );
    sldel( &s );

    s = slstr_c( "XYabcXYabc" );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 10 );

    slmap( &s, "XY", "GIG" );
    TEST_ASSERT_TRUE( !strcmp( s, "GIGabcGIGabc" ) );
    sldel( &s );

    s = slstr_c( "XYabcXYabc" );
    TEST_ASSERT( slrss( s ) == 12 );
    TEST_ASSERT( sllen( s ) == 10 );

    slmap( &s, "XY", "GG" );
    TEST_ASSERT_TRUE( !strcmp( s, "GGabcGGabc" ) );
    sldel( &s );
}


void test_file( void )
{
    char* filetext = "\
line1\n\
line2\n\
line3\n\
line4\n\
line5\n\
";

    sl_t s, s2;
    s = slstr_c( filetext );
    slwrf( s, "test/test_file.txt" );
    s2 = slrdf( "test/test_file.txt" );
    TEST_ASSERT_TRUE( !strcmp( s2, filetext ) );

    sldmp( s );
    sl_print( s );
    TEST_ASSERT( sllen( s ) == 0 );

    sldel( &s );
    sldel( &s2 );
    TEST_ASSERT( s == NULL );
    TEST_ASSERT( s2 == NULL );
}


void test_path( void )
{
    char* path1 = "/foo/bar/dii.txt";
    char* path2 = "./foo/bar/dii.txt";
    char* path3 = "/foo";
    char* path4 = "./foo";
    char* path5 = "dii.txt";

    sl_t s;

    s = slstr_c( path1 );
    sldir( s );
    TEST_ASSERT_TRUE( !strcmp( s, "/foo/bar" ) );
    TEST_ASSERT( sllen( s ) == 8 );
    sldel( &s );

    s = slstr_c( path2 );
    sldir( s );
    TEST_ASSERT_TRUE( !strcmp( s, "./foo/bar" ) );
    TEST_ASSERT( sllen( s ) == 9 );
    sldel( &s );

    s = slstr_c( path3 );
    sldir( s );
    TEST_ASSERT_TRUE( !strcmp( s, "/" ) );
    TEST_ASSERT( sllen( s ) == 1 );
    sldel( &s );

    s = slstr_c( path4 );
    sldir( s );
    TEST_ASSERT_TRUE( !strcmp( s, "." ) );
    TEST_ASSERT( sllen( s ) == 1 );
    sldel( &s );

    s = slstr_c( path5 );
    sldir( s );
    TEST_ASSERT_TRUE( !strcmp( s, "." ) );
    TEST_ASSERT( sllen( s ) == 1 );
    sldel( &s );

    s = slstr_c( path1 );
    slbas( s );
    TEST_ASSERT_TRUE( !strcmp( s, "dii.txt" ) );
    TEST_ASSERT( sllen( s ) == 7 );
    sldel( &s );

    s = slstr_c( path2 );
    slbas( s );
    TEST_ASSERT_TRUE( !strcmp( s, "dii.txt" ) );
    TEST_ASSERT( sllen( s ) == 7 );
    sldel( &s );

    s = slstr_c( path3 );
    slbas( s );
    TEST_ASSERT_TRUE( !strcmp( s, "foo" ) );
    TEST_ASSERT( sllen( s ) == 3 );
    sldel( &s );

    s = slstr_c( path4 );
    slbas( s );
    TEST_ASSERT_TRUE( !strcmp( s, "foo" ) );
    TEST_ASSERT( sllen( s ) == 3 );
    sldel( &s );

    s = slstr_c( path5 );
    slbas( s );
    TEST_ASSERT_TRUE( !strcmp( s, "dii.txt" ) );
    TEST_ASSERT( sllen( s ) == 7 );
    sldel( &s );

    s = slstr_c( path5 );
    slext( s, ".txt" );
    TEST_ASSERT_TRUE( !strcmp( s, "dii" ) );

    s = slstr_c( path5 );
    TEST_ASSERT( slext( s, ".dii" ) == NULL );

    sltou( s );
    TEST_ASSERT_TRUE( !strcmp( s, "DII.TXT" ) );

    sltol( s );
    TEST_ASSERT_TRUE( !strcmp( s, "dii.txt" ) );

    slcap( s );
    TEST_ASSERT_TRUE( !strcmp( s, "Dii.txt" ) );

    sldel( &s );
}
