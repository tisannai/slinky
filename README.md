# Slinky - Simple String Library

Slinky is a small string library that has automatic string storage
resizing and fairly high performance. Slinky is compatible with
standard C library string functions.

Slinky String looks like a "normal" C string, but it includes a hidden
descriptor. Descriptor is located just before string Content in
memory. Descriptor includes storage size of string Content, and
effective length of the string. Effective length means string length
without the terminating null. Slinky Strings are always null
terminated. The total Slinky Allocation is sum Descriptor allocation
plus the string storage allocation. Storage size is an even number,
since the LSB is used to decide whether Slinky is statically allocated
or dynamic (see below: sluse).

Slinky struct:

                      Field     Type          Addr
                      -----------------------------
     Descriptor ----- storage   (uint32_t)  | N + 0
                   `- length    (uint32_t)  | N + 4
        Content ----- string    (char*)     | N + 8

Basic Slinky datatype is `sl_t`. Most Slinky library functions take it as
argument and they also return values in that type. `sl_t` is a
typedef of "char*", hence it is usable by standard C library
functions.

Slinky library functions expect that `sl_t` typed arguments have the
"hidden" descriptor in place. Using CSTR in place of a Slinky string
will cause memory corruption. CSTR has potentially "garbage" in
front of its first character in memory.

Part of the Slinky library functions mutate the string and some only
reference it (read only access). Some of the mutating functions may
need to allocate more space in order to fit the growing string. These
functions require `sl_p` type, Slinky Reference, to be used as
argument type. `sl_p` is a pointer to `sl_t`. `sl_p` is needed since
Slinky String might appear in different memory address after
resizing. Even when the argument have to be of `sl_p` type, the return
value is still of `sl_t` type. This makes use of Slinky more
convenient.

Some Slinky library functions may use both Slinky and CSTR type
arguments. These functions use `char*` as argument type, since Slinky
is fully compatible with CSTR.

Functions that convert CSTR to Slinky, use `_c` as postfix to designate
conversion. Note that even in this case you could replace CSTR with
Slinky, since Slinky is a superset of CSTR.

Typically Slinky Strings are created with larger storage than what the
initial content would require. For example we could create Slinky as:

    sl_t ss;
    ss = slsiz_c( "hello", 128 );

Length would be 5 and storage size 128. If we want to append ` world!`
to Slinky, we don't have to redo any allocations, since we have spare
storage. Concatenation would be done as:

    slcat_c( &ss, " world!" );

We could also create Slinky with minimum size:

    ss = slstr_c( "hello" );

Again the length would be 5, but storage would be only 6. If we now
concatenate the rest to Slinky, there will be a re-allocation. After
re-allocation, the storage is still the minimum, i.e 5+7+1 = 13.

Any heap allocated Slinky should be de-allocated after use.

    sldel( &ss );

De-allocation takes `sl_p` type argument in order to ensure that Slinky
becomes NULL after memory free.

Slinky can also be used within stack allocated strings. First you have to
have some stack storage available.

    char buf[ 128 ];
    sl_t ss;

Then you can take that into use with:

    ss = sluse( buf, 128 );

When Slinky is taken into use through `sluse` it is marked as "local",
and means that it will not be freed with `sldel`. Stack allocated
Slinky is automatically changed to a heap allocated Slinky if Slinky
requires resizing. This is quite powerful optimization, since often
stack allocated strings are enough and heap reservation (which is
slowish) is not needed. `sldel` can be called for Slinky whether its
"local" or not. If Slinky is "local", no memory is released, but
Slinky is set to NULL.

By default Slinky library uses malloc and friends to do heap
allocations. If you define SL_MEM_API, you can use your own memory
allocation functions.

Custom memory function prototypes:
    void* sl_malloc ( size_t size );
    void  sl_free   ( void*  ptr  );
    void* sl_realloc( void*  ptr, size_t size );


Basic usage example:

    sl_t sl;

    /* Create Slinky from CSTR with storage size of 128. */
    sl = slsiz_c( "hello", 128 );

    /* Concatenate Slinky with CSTR. */
    slcat_c( &sl, " world!" );

    /* Get Slinky length (= 12). */
    length = sllen( sl );

    /* Append Slinky with 10 'a' letters. */
    slfil( &sl, 'a', 10 );

    /* Cut off the 'a' letters. */
    slcut( &sl, 12 );

    /* De-allocate Slinky after use. */
    sldel( &sl );


See `slinky.h` for complete list of Slinky library API functions.


# Testing

Tests are in `test` directory. Tests are a reasonable example for Slinky
use. The boilerplate Ceedling files are not in GIT.

Slinky is tested with Ceedling. Execute

    shell> rake test:all

to run all tests. Execute

    shell> rake gcov:all

to generate coverage info, and finally, execute

    shell> rake utils:gcov

to generate coverage reports.


## Ceedling

Slinky uses Ceedling for building and testing. Standard Ceedling files
are not in GIT. These can be added by executing:

    shell> ceedling new slinky

in the directory above Slinky. Ceedling prompts for file
overwrites. You should answer NO in order to use the customized files.
