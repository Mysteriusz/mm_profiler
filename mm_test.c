#include "test.h"

#define STRIDE 64

#define mm_write(bufsize) ({ \
	c0 = __rdtscp(&a); \
	while(i < ((bufsize) / sizeof(int))){ \
		buf[i] = val; \
		i += STRIDE; acc++; \
	} \
	c1 = __rdtscp(&a); \
	cpa = ((long double)(c1 - c0)) / acc; /*cycles per access*/ \
	sec = cyc_to_sec(c1 - c0, cpu.freq); \
	printf("Main memory write cycles per access: %Lf\n", cpa); \
	printf("Main memory write in seconds: %Lf\n", sec); \
	printf("Main memory write in GB/s: %Lf GB/s\n", (((long double)bufsize) / (1 << 30)) / sec); \
	i = 0; \
	acc = 0; \
})
#define mm_read(bufsize) ({ \
	c0 = __rdtscp(&a); \
	while(i < ((bufsize) / sizeof(int))){ \
		val = buf[i]; \
		i += STRIDE; acc++; \
	} \
	c1 = __rdtscp(&a); \
	cpa = ((long double)(c1 - c0)) / acc; /*cycles per access*/ \
	sec = cyc_to_sec(c1 - c0, cpu.freq); \
	printf("Main memory read cycles per access: %Lf\n", cpa); \
	printf("Main memory read in seconds: %Lf\n", sec); \
	printf("Main memory read in GB/s: %Lf GB/s\n", (((long double)bufsize) / (1 << 30)) / sec); \
	i = 0; \
	acc = 0; \
})

int mm_test(
	struct cpu_info cpu,
	const unsigned long long size
){
	unsigned int a = 0;
	unsigned long long c0 = 0;
	unsigned long long c1 = 0;
	long double cpa = 0;
	long double sec = 0;

	int *buf = VirtualAlloc(
		0,
		size,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE | PAGE_NOCACHE);
	if (buf == NULL){
		return GetLastError();
	}

	c0 = __rdtscp(&a);
	// Prefetch memory range to mitigate page miss
	if (!PrefetchVirtualMemory(
		GetCurrentProcess(),
		1,
		&(WIN32_MEMORY_RANGE_ENTRY){
			.VirtualAddress = buf,
			.NumberOfBytes = size
		},
		0)
	){
		return GetLastError();
	}
	SYSTEM_INFO sys = {0};
	GetSystemInfo(&sys);
	for (int i = 0; i < size; i += sys.dwPageSize){
		((char*)buf)[i] = 0;
	}
	c1 = __rdtscp(&a);

	printf("Main memory prefetch in seconds: %Lf\n", cyc_to_sec(c1 - c0, cpu.freq));

	volatile int acc = 0;
	volatile int i = 0;

	volatile register int val = 1;

	int rc = 10; // read count
	int wc = 100; // write count

	while(rc > 0){
		mm_read(size);
		rc--;
	}
	while(wc > 0){
		mm_write(size);
		wc--;
	}

	VirtualFree(buf, size, 0);

	return 0;
}

