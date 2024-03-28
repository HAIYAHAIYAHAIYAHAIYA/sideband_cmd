#ifndef _TYPE_H_
#define _TYPE_H_

#define ntohs(x)     (((x & 0xff) << 8) | ((x) >> 8) & 0xff)
#define ntohl(x)     (((x & 0xff) << 24) | ((((x) >> 8) & 0xff) << 16) | ((((x) >> 16) & 0xff) << 8) | (((x) >> 24) & 0xff))
#define htons(x)     (((x & 0xff) << 8) | ((x) >> 8) & 0xff)
#define htonl(x)     (((x & 0xff) << 24) | ((((x) >> 8) & 0xff) << 16) | ((((x) >> 16) & 0xff) << 8) | (((x) >> 24) & 0xff))

typedef unsigned char 			u8;
typedef unsigned short			u16;
typedef unsigned int			u32;
typedef unsigned long long		u64;

typedef signed char 			s8;
typedef signed short			s16;
typedef signed int				s32;
typedef signed long long 		s64;

typedef unsigned long long		u47;
typedef unsigned long long		u128;
typedef unsigned long long		u192;

#define TRUE 0x01
#define FALSE 0x00

#define SUCCESS 0
#define FAILURE -1

#define BIT_0 0x00000001
#define BIT_1 0x00000002
#define BIT_2 0x00000004
#define BIT_3 0x00000008
#define BIT_4 0x00000010
#define BIT_5 0x00000020
#define BIT_6 0x00000040
#define BIT_7 0x00000080
#define BIT_8 0x00000100
#define BIT_9 0x00000200
#define BIT_10 0x00000400
#define BIT_11 0x00000800
#define BIT_12 0x00001000
#define BIT_13 0x00002000
#define BIT_14 0x00004000
#define BIT_15 0x00008000
#define BIT_16 0x00010000
#define BIT_17 0x00020000
#define BIT_18 0x00040000
#define BIT_19 0x00080000
#define BIT_20 0x00100000
#define BIT_21 0x00200000
#define BIT_22 0x00400000
#define BIT_23 0x00800000
#define BIT_24 0x01000000
#define BIT_25 0x02000000
#define BIT_26 0x04000000
#define BIT_27 0x08000000
#define BIT_28 0x10000000
#define BIT_29 0x20000000
#define BIT_30 0x40000000
#define BIT_31 0x80000000

#define SZ_1 0x00000001
#define SZ_2 0x00000002
#define SZ_4 0x00000004
#define SZ_8 0x00000008
#define SZ_16 0x00000010
#define SZ_32 0x00000020
#define SZ_64 0x00000040
#define SZ_128 0x00000080
#define SZ_256 0x00000100
#define SZ_512 0x00000200

#define SZ_1K 0x00000400
#define SZ_2K 0x00000800
#define SZ_4K 0x00001000
#define SZ_8K 0x00002000
#define SZ_16K 0x00004000
#define SZ_32K 0x00008000
#define SZ_64K 0x00010000
#define SZ_128K 0x00020000
#define SZ_256K 0x00040000
#define SZ_512K 0x00080000

#define SZ_1M 0x00100000
#define SZ_2M 0x00200000
#define SZ_4M 0x00400000
#define SZ_8M 0x00800000
#define SZ_16M 0x01000000
#define SZ_32M 0x02000000
#define SZ_64M 0x04000000
#define SZ_128M 0x08000000
#define SZ_256M 0x10000000
#define SZ_512M 0x20000000

#define SZ_1G 0x40000000
#define SZ_2G 0x80000000
#define SZ_4G (0x100000000ULL)
#define SZ_8G (0x200000000ULL)
#define SZ_16G (0x400000000ULL)
#define SZ_128G (0x2000000000ULL)
#define SZ_16T (0x100000000000ULL)
#define SZ_64T (0x400000000000ULL)

#define CBIT(pos) BIT_##pos
#define BIT(pos) ((u32)1 << (pos))

#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define ALIGN(dat, align)    (((dat) + (align) - 1) & (~((align) - 1)))

#define REG(reg)        (*(volatile unsigned long *)(reg))

#endif