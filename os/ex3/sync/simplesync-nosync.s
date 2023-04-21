	.file	"simplesync-nosync.c"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"About to increase variable %d times\n"
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"Done increasing variable.\n"
	.text
	.p2align 4
	.globl	increase_fn
	.type	increase_fn, @function
increase_fn:
.LFB51:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	movq	stderr(%rip), %rdi
	movl	$10000000, %ecx
	leaq	.LC0(%rip), %rdx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movl	$10000000, %edx
	.p2align 4,,10
	.p2align 3
.L2:
	movl	(%rbx), %eax
	addl	$1, %eax
	movl	%eax, (%rbx)
	subl	$1, %edx
	jne	.L2
	movq	stderr(%rip), %rcx
	movl	$26, %edx
	movl	$1, %esi
	leaq	.LC1(%rip), %rdi
	call	fwrite@PLT
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE51:
	.size	increase_fn, .-increase_fn
	.section	.rodata.str1.8
	.align 8
.LC2:
	.string	"About to decrease variable %d times\n"
	.section	.rodata.str1.1
.LC3:
	.string	"Done decreasing variable.\n"
	.text
	.p2align 4
	.globl	decrease_fn
	.type	decrease_fn, @function
decrease_fn:
.LFB52:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	movq	stderr(%rip), %rdi
	movl	$10000000, %ecx
	leaq	.LC2(%rip), %rdx
	movl	$1, %esi
	xorl	%eax, %eax
	call	__fprintf_chk@PLT
	movl	$10000000, %edx
	.p2align 4,,10
	.p2align 3
.L7:
	movl	(%rbx), %eax
	subl	$1, %eax
	movl	%eax, (%rbx)
	subl	$1, %edx
	jne	.L7
	movq	stderr(%rip), %rcx
	movl	$26, %edx
	movl	$1, %esi
	leaq	.LC3(%rip), %rdi
	call	fwrite@PLT
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE52:
	.size	decrease_fn, .-decrease_fn
	.section	.rodata.str1.1
.LC4:
	.string	""
.LC5:
	.string	"NOT "
.LC6:
	.string	"\n Mutex init has failed"
.LC7:
	.string	"pthread_create"
.LC8:
	.string	"pthread_join"
	.section	.rodata.str1.8
	.align 8
.LC9:
	.string	"Error in pthread_mutex_destroy"
	.section	.rodata.str1.1
.LC10:
	.string	"%sOK, val = %d.\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB53:
	.cfi_startproc
	endbr64
	pushq	%r12
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	xorl	%esi, %esi
	leaq	lock(%rip), %rdi
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$40, %rsp
	.cfi_def_cfa_offset 64
	movq	%fs:40, %rax
	movq	%rax, 24(%rsp)
	xorl	%eax, %eax
	movl	$0, 4(%rsp)
	call	pthread_mutex_init@PLT
	testl	%eax, %eax
	jne	.L32
	leaq	4(%rsp), %r12
	leaq	8(%rsp), %rdi
	xorl	%esi, %esi
	movq	%r12, %rcx
	leaq	increase_fn(%rip), %rdx
	call	pthread_create@PLT
	movl	%eax, %ebx
	testl	%eax, %eax
	jne	.L31
	leaq	16(%rsp), %rdi
	movq	%r12, %rcx
	leaq	decrease_fn(%rip), %rdx
	xorl	%esi, %esi
	call	pthread_create@PLT
	movl	%eax, %ebx
	testl	%eax, %eax
	jne	.L31
	movq	8(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join@PLT
	movl	%eax, %ebx
	testl	%eax, %eax
	jne	.L33
.L15:
	movq	16(%rsp), %rdi
	xorl	%esi, %esi
	call	pthread_join@PLT
	movl	%eax, %ebx
	testl	%eax, %eax
	jne	.L34
.L16:
	leaq	lock(%rip), %rdi
	call	pthread_mutex_destroy@PLT
	testl	%eax, %eax
	jne	.L35
.L17:
	movl	4(%rsp), %ebx
	leaq	.LC5(%rip), %rax
	leaq	.LC4(%rip), %rdx
	movl	$1, %edi
	leaq	.LC10(%rip), %rsi
	testl	%ebx, %ebx
	movl	%ebx, %ecx
	cmovne	%rax, %rdx
	xorl	%eax, %eax
	call	__printf_chk@PLT
	xorl	%eax, %eax
	testl	%ebx, %ebx
	setne	%al
.L10:
	movq	24(%rsp), %rcx
	xorq	%fs:40, %rcx
	jne	.L36
	addq	$40, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%r12
	.cfi_def_cfa_offset 8
	ret
.L35:
	.cfi_restore_state
	leaq	.LC9(%rip), %rdi
	call	perror@PLT
	jmp	.L17
.L34:
	call	__errno_location@PLT
	leaq	.LC8(%rip), %rdi
	movl	%ebx, (%rax)
	call	perror@PLT
	jmp	.L16
.L33:
	call	__errno_location@PLT
	leaq	.LC8(%rip), %rdi
	movl	%ebx, (%rax)
	call	perror@PLT
	jmp	.L15
.L32:
	leaq	.LC6(%rip), %rdi
	call	puts@PLT
	movl	$1, %eax
	jmp	.L10
.L31:
	call	__errno_location@PLT
	leaq	.LC7(%rip), %rdi
	movl	%ebx, (%rax)
	call	perror@PLT
	movl	$1, %edi
	call	exit@PLT
.L36:
	call	__stack_chk_fail@PLT
	.cfi_endproc
.LFE53:
	.size	main, .-main
	.comm	lock,40,32
	.ident	"GCC: (Ubuntu 9.4.0-1ubuntu1~20.04.1) 9.4.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
