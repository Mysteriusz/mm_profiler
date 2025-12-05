#include "test.h"

#define to_gb(val) ((long double)(val) / (1 << 30))

int ca_test(
	struct cpu_info cpu,
	struct cpu_cache_info cache
){
	//unsigned int a = 0;
	long double cpa = 0;
	long double sec = 0;

	char *buf = VirtualAlloc(
		0,
		cache.size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE);
	if (buf == NULL){
		return GetLastError();
	}

	for (int i = 0; i < cache.size; i++){
		buf[i] = 1;
	}

#define T 10000000ULL
	unsigned long long 
		n = T,
		c0 = 0,
		c1 = 0;

	volatile unsigned int lo_cpa = UINT_MAX;
	volatile unsigned int hi_cpa = 0;

	volatile register unsigned long long cyc = 0;
	volatile register unsigned long long res = 0;
	volatile register int ran = 0;
	volatile register char *ptr = 0;
	while (n--) {
        // use processor clock timer for exact measurement
	        ran += rand();
	        ran %= cache.size;
		ptr = (buf + ran);
		asm volatile(
			"mfence\n\t"
			"lfence\n\t"
			"rdtsc\n\t"
			"shl $32, %%rdx\n\t"
			"or %%rdx, %%rax\n\t"
			"mov %%rax, %0\n\t"
			"movb $1, %2\n\t"
			"rdtsc\n\t"
			"shl $32, %%rdx\n\t"
			"or %%rdx, %%rax\n\t"
			"mov %%rax, %1\n\t"
			: "=r"(c0), "=r"(c1)
			: "m"(ptr)
			: "memory"
		);
		res = (c1 - c0)
			- 2 // Exclude latencies of shl and or
			- (2 % (n + 1)); // exclude first mov instruction
		cyc += res;

		if (res < lo_cpa){
			lo_cpa = res;
		}
		if (res > hi_cpa){
			hi_cpa = res;
		}
    	}

	cpa = ((long double)cyc / T); /*cycles per access*/
	sec = cyc_to_sec(cyc, cpu.freq);
	printf("Cache access count: %llu\n", T);
	printf("Cache cycles count: %llu\n", cyc);
	printf("Cache in seconds: %Lf\n", sec);
	printf("Average cycles per access (CPA): %Lf\n", cpa);
	printf("Lowest CPA: %i\n", lo_cpa);
	printf("Highest CPA: %i\n", hi_cpa);

	VirtualFree(buf, cache.size, 0);

	return 0;
}

