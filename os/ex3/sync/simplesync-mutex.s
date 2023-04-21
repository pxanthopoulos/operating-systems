	.file	"simplesync.c"
	.text
	.comm	lock,40,32
	.section	.rodata
	.align 8
.LC0:
	.string	"About to increase variable %d times\n"
.LC1:
	.string	"Done increasing variable.\n"
	.text
	.globl	increase_fn
	.type	increase_fn, @function
increase_fn:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	stderr(%rip), %rax
	movl	$10000000, %edx
	leaq	.LC0(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fprintf@PLT
	movl	$0, -12(%rbp)
	jmp	.L2
.L3:
	leaq	lock(%rip), %rdi
	call	pthread_mutex_lock@PLT
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	leal	1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, (%rax)
	leaq	lock(%rip), %rdi
	call	pthread_mutex_unlock@PLT
	addl	$1, -12(%rbp)
.L2:
	cmpl	$9999999, -12(%rbp)
	jle	.L3
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$26, %edx
	movl	$1, %esi
	leaq	.LC1(%rip), %rdi
	call	fwrite@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	increase_fn, .-increase_fn
	.section	.rodata
	.align 8
.LC2:
	.string	"About to decrease variable %d times\n"
.LC3:
	.string	"Done decreasing variable.\n"
	.text
	.globl	decrease_fn
	.type	decrease_fn, @function
decrease_fn:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	stderr(%rip), %rax
	movl	$10000000, %edx
	leaq	.LC2(%rip), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fprintf@PLT
	movl	$0, -12(%rbp)
	jmp	.L6
.L7:
	leaq	lock(%rip), %rdi
	call	pthread_mutex_lock@PLT
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	leal	-1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, (%rax)
	leaq	lock(%rip), %rdi
	call	pthread_mutex_unlock@PLT
	addl	$1, -12(%rbp)
.L6:
	cmpl	$9999999, -12(%rbp)
	jle	.L7
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$26, %edx
	movl	$1, %esi
	leaq	.LC3(%rip), %rdi
	call	fwrite@PLT
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	decrease_fn, .-decrease_fn
	.section	.rodata
.LC4:
	.string	"\n Mutex init has failed"
.LC5:
	.string	"pthread_create"
.LC6:
	.string	"pthread_join"
	.align 8
.LC7:
	.string	"Error in pthread_mutex_destroy"
.LC8:
	.string	""
.LC9:
	.string	"NOT "
.LC10:
	.string	"%sOK, val = %d.\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movl	%edi, -52(%rbp)
	movq	%rsi, -64(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$0, -36(%rbp)
	movl	$0, %esi
	leaq	lock(%rip), %rdi
	call	pthread_mutex_init@PLT
	testl	%eax, %eax
	je	.L10
	leaq	.LC4(%rip), %rdi
	call	puts@PLT
	movl	$1, %eax
	jmp	.L19
.L10:
	leaq	-36(%rbp), %rdx
	leaq	-24(%rbp), %rax
	movq	%rdx, %rcx
	leaq	increase_fn(%rip), %rdx
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_create@PLT
	movl	%eax, -32(%rbp)
	cmpl	$0, -32(%rbp)
	je	.L12
	call	__errno_location@PLT
	movl	-32(%rbp), %edx
	movl	%edx, (%rax)
	leaq	.LC5(%rip), %rdi
	call	perror@PLT
	movl	$1, %edi
	call	exit@PLT
.L12:
	leaq	-36(%rbp), %rdx
	leaq	-16(%rbp), %rax
	movq	%rdx, %rcx
	leaq	decrease_fn(%rip), %rdx
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_create@PLT
	movl	%eax, -32(%rbp)
	cmpl	$0, -32(%rbp)
	je	.L13
	call	__errno_location@PLT
	movl	-32(%rbp), %edx
	movl	%edx, (%rax)
	leaq	.LC5(%rip), %rdi
	call	perror@PLT
	movl	$1, %edi
	call	exit@PLT
.L13:
	movq	-24(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_join@PLT
	movl	%eax, -32(%rbp)
	cmpl	$0, -32(%rbp)
	je	.L14
	call	__errno_location@PLT
	movl	-32(%rbp), %edx
	movl	%edx, (%rax)
	leaq	.LC6(%rip), %rdi
	call	perror@PLT
.L14:
	movq	-16(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_join@PLT
	movl	%eax, -32(%rbp)
	cmpl	$0, -32(%rbp)
	je	.L15
	call	__errno_location@PLT
	movl	-32(%rbp), %edx
	movl	%edx, (%rax)
	leaq	.LC6(%rip), %rdi
	call	perror@PLT
.L15:
	leaq	lock(%rip), %rdi
	call	pthread_mutex_destroy@PLT
	testl	%eax, %eax
	je	.L16
	leaq	.LC7(%rip), %rdi
	call	perror@PLT
.L16:
	movl	-36(%rbp), %eax
	testl	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	movl	%eax, -28(%rbp)
	movl	-36(%rbp), %edx
	cmpl	$0, -28(%rbp)
	je	.L17
	leaq	.LC8(%rip), %rax
	jmp	.L18
.L17:
	leaq	.LC9(%rip), %rax
.L18:
	movq	%rax, %rsi
	leaq	.LC10(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	cmpl	$0, -28(%rbp)
	sete	%al
	movzbl	%al, %eax
.L19:
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L20
	call	__stack_chk_fail@PLT
.L20:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	main, .-main
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
