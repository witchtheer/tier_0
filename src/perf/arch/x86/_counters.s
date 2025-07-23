.intel_syntax noprefix
.section .text

.globl x86_get_rdtsc_counter
.globl x86_get_rdtscp_counter

x86_get_rdtsc_counter:
  rdtsc # we call the cpu to read the counter 
  ret

x86_get_rdtscp_counter:
  rdtscp 
  ret
