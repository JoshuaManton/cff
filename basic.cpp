typedef unsigned char      byte; static_assert(sizeof(byte) == 1, "byte size was not 1");
typedef unsigned char      u8;   static_assert(sizeof(u8)   == 1, "u8 size was not 1");
typedef unsigned short     u16;  static_assert(sizeof(u16)  == 2, "u16 size was not 2");
typedef unsigned int       u32;  static_assert(sizeof(u32)  == 4, "u32 size was not 4");
typedef unsigned long long u64;  static_assert(sizeof(u64)  == 8, "u64 size was not 8");

typedef u32 uint; static_assert(sizeof(uint)  == 4, "uint size was not 4");

typedef char      i8;   static_assert(sizeof(i8)   == 1, "i8 size was not 1");
typedef short     i16;  static_assert(sizeof(i16)  == 2, "i16 size was not 2");
typedef int       i32;  static_assert(sizeof(i32)  == 4, "i32 size was not 4");
typedef long long i64;  static_assert(sizeof(i64)  == 8, "i64 size was not 8");

static_assert(sizeof(int)  == 4, "int size was not 4");

typedef float  f32; static_assert(sizeof(f32) == 4, "f32 size was not 4");
typedef double f64; static_assert(sizeof(f64) == 8, "f64 size was not 8");

static_assert(sizeof(float) == 4, "float size was not 4");

static_assert(sizeof(void *) == 8, "void * size was not 8");

#define assert(cond) assert(cond, "#cond")
