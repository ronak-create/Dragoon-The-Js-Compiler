.data
.align 8
d_fmt:
	.ascii "%d\n"
	.byte 0
/* end data */

.text
.globl printi
printi:
	pushq %rbp
	movq %rsp, %rbp
	movl %edi, %esi
	leaq d_fmt(%rip), %rdi
	movl $0, %eax
	callq printf
	movl $0, %eax
	leave
	ret
/* end function printi */

.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $7, %edi
	callq printi
	movl $0, %eax
	leave
	ret
/* end function main */

.section .note.GNU-stack,"",@progbits
