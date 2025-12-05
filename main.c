#include <cpuid.h>
#include <stdio.h>
#include <errhandlingapi.h>

#include "test.h"

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

int main(){
	/*if (mm_test((struct cpu_info){.freq = 2.5}, 1 << 30) != 0){
	  	return -1;
	}*/
	ca_test(
		(struct cpu_info){
			.freq = 4.2
		},
		(struct cpu_cache_info){
			.size = 49152, 
			.set_count = 64, 
			.line_size = 49152 / 64 / 12, 
			.set_size = 49152 / 64, 
			.ways = 12
		}
	);
	/*ca_test(
		(struct cpu_info){
			.freq = 4.4
		},
		(struct cpu_cache_info){
			.size = 1310720,
			.set_count = 2048,
			.line_size = 1310720 / 2048 / 10,
			.set_size = 1310720 / 2048,
			.ways = 10
		}
	);
	ca_test(
		(struct cpu_info){
			.freq = 4.4
		},
		(struct cpu_cache_info){
			.size = 18874368,
			.set_count = 32768,
			.line_size = 18874368 / 32768 / 9,
			.set_size = 18874368 / 32768,
			.ways = 9
		}
	);*/
	log_cache();
	return 0;
}

