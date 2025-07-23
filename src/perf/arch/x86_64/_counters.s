.intel_syntax noprefix
.section .text

.globl x86_64_get_rdtsc_counter
.globl x86_64_get_rdtscp_counter

x86_64_get_rdtsc_counter:
  rdtsc # we call the cpu to read the counter 
  shl rdx, 32 # shift the high value by 32 bits
  or rax,rdx # we combine it lol
  ret

x86_64_get_rdtscp_counter:
  rdtscp
  shl rdx, 32
  or rax,rdx
  ret
