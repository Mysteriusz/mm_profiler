#undef _WIN32_WINNT
#define _WIN32_WINNT NTDDI_WIN10_19H1

#include <memoryapi.h>
#include <errhandlingapi.h>
#include <wtypesbase.h>
#include <winbase.h>

#include <stdio.h>

#define cyc_to_sec(cyc, hz) (long double)((cyc) / ((hz) * 1e9))

struct cpu_info{
	double freq;
};
struct cpu_cache_info{
	unsigned long long size;
	unsigned int line_size;
	unsigned int set_size;
	unsigned int set_count;
	unsigned int ways;
};

int ca_test(
	struct cpu_info cpu,
	struct cpu_cache_info cache
);
int mm_test(
	struct cpu_info cpu,
	const unsigned long long size
);

