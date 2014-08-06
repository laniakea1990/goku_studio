	.file	"cat.c"
	.section	.rodata
.LC0:
	.string	"r"
.LC1:
	.string	"cat:can't open %s\n"
	.text
.globl main
	.type	main, @function
main:
	pushl	%ebp
	movl	%esp, %ebp
	andl	$-16, %esp
	subl	$32, %esp
	cmpl	$1, 8(%ebp)
	jne	.L4
	movl	stdout, %edx
	movl	stdin, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	filecopy
	jmp	.L3
.L7:
	movl	$.LC0, %edx
	addl	$4, 12(%ebp)
	movl	12(%ebp), %eax
	movl	(%eax), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, 28(%esp)
	cmpl	$0, 28(%esp)
	jne	.L5
	movl	12(%ebp), %eax
	movl	(%eax), %edx
	movl	$.LC1, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	printf
	movl	$1, %eax
	jmp	.L6
.L5:
	movl	stdout, %eax
	movl	%eax, 4(%esp)
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	filecopy
	movl	28(%esp), %eax
	movl	%eax, (%esp)
	call	fclose
.L4:
	subl	$1, 8(%ebp)
	cmpl	$0, 8(%ebp)
	jg	.L7
.L3:
	movl	$0, %eax
.L6:
	leave
	ret
	.size	main, .-main
.globl filecopy
	.type	filecopy, @function
filecopy:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$40, %esp
	jmp	.L10
.L11:
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, (%esp)
	call	_IO_putc
.L10:
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	_IO_getc
	movl	%eax, -12(%ebp)
	cmpl	$-1, -12(%ebp)
	jne	.L11
	leave
	ret
	.size	filecopy, .-filecopy
	.ident	"GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3"
	.section	.note.GNU-stack,"",@progbits
