#define _WIN32_WINNT NTDDI_WIN10_19H1

#include <cpuid.h>
#include <stdio.h>
#include <processthreadsapi.h>
#include <errhandlingapi.h>
#include <memoryapi.h>

union lab{
	unsigned long val;
	char arr[4];
};

struct chinf{
	unsigned long coh : 12;
	unsigned long parts : 10;
	unsigned long ways : 10;
};

__attribute__((noinline))
int log_cache(){
	unsigned long eax, ebx, ecx, edx;
	__cpuid(0x00, eax, ebx, ecx, edx); 
	printf("%.4s%.4s%.4s\n",
			((union lab){.val = ebx}).arr,
			((union lab){.val = edx}).arr,
			((union lab){.val = ecx}).arr); 

	struct chinf *linf;
	int i = 0;

	while(1){
		__cpuid_count(0x04, i, eax, ebx, ecx, edx); 
		if ((eax & 15) == 0){
			break;
		}

		linf = (struct chinf*)&ebx;
		printf("L%lu Cache info:\n"
			"Ways of associativity: %u\n"
			"Set count: %lu\n"
			"Size in bytes: %lu\n",
			(eax >> 5) & 7,
			linf->ways + 1,
			ecx + 1,
			(linf->ways + 1) * (linf->parts + 1) * (linf->coh + 1) * (ecx + 1)
		);
		i++;
	}

	return 0;
}

#define STRIDE 64

#define wmain(bufsize) ({ \
	c0 = __rdtscp(&a); \
	while(i < ((bufsize) / sizeof(int))){ \
		buf[i] = val; \
		i += STRIDE; acc++; \
	} \
	c1 = __rdtscp(&a); \
	cpa = ((long double)(c1 - c0)) / acc; /*cycles per access*/ \
	sec = (long double)(c1 - c0) / 2.5e9; \
	printf("Main memory write cycles per access: %Lf\n", cpa); \
	printf("Main memory write in seconds: %Lf\n", sec); \
	printf("Main memory write in GB/s: %Lf GB/s\n", (bufsize / (1 << 30)) / sec); \
	i = 0; \
	acc = 0; \
})
#define rmain(bufsize) ({ \
	c0 = __rdtscp(&a); \
	while(i < ((bufsize) / sizeof(int))){ \
		val = buf[i]; \
		i += STRIDE; acc++; \
	} \
	c1 = __rdtscp(&a); \
	cpa = ((long double)(c1 - c0)) / acc; /*cycles per access*/ \
	sec = (long double)(c1 - c0) / 2.5e9; \
	printf("Main memory read cycles per access: %Lf\n", cpa); \
	printf("Main memory read in seconds: %Lf\n", sec); \
	printf("Main memory read in GB/s: %Lf GB/s\n", (bufsize / (1 << 30)) / sec); \
	i = 0; \
	acc = 0; \
})

__attribute__((always_inline))
int rwmain(int rc){
	return 0;
}

int main() {
	unsigned int a = 0;
	unsigned long long c0 = 0;
	unsigned long long c1 = 0;
	long double cpa = 0;
	long double sec = 0;

	// PAGE_NOCACHE to bypass cpu caching
	int *buf = VirtualAlloc(0, (1 << 30), MEM_COMMIT, PAGE_READWRITE | PAGE_NOCACHE);
	if (buf == NULL){
		return GetLastError();
	}

	volatile int acc = 0;
	volatile int i = 0;

	volatile register int val = 1;

	int rc = 100; // read count
	int wc = 100; // write count

	while(rc > 0){
		rmain(1 << 30);
		rc--;
	}
	while(wc > 0){
		wmain(1 << 30);
		wc--;
	}

	(void)val;

	log_cache();
	return 0;
}

