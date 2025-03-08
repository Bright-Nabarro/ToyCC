	.text
	.file	"toycc.expr"
	.globl	int_add
	.p2align	4, 0x90
	.type	int_add,@function
int_add:
	.cfi_startproc
	movl	%edi, -4(%rsp)
	movl	%esi, -8(%rsp)
	leal	(%rdi,%rsi), %eax
	retq
.Lfunc_end0:
	.size	int_add, .Lfunc_end0-int_add
	.cfi_endproc

	.globl	int_sub
	.p2align	4, 0x90
	.type	int_sub,@function
int_sub:
	.cfi_startproc
	movl	%edi, %eax
	movl	%edi, -4(%rsp)
	movl	%esi, -8(%rsp)
	subl	%esi, %eax
	retq
.Lfunc_end1:
	.size	int_sub, .Lfunc_end1-int_sub
	.cfi_endproc

	.globl	int_mul
	.p2align	4, 0x90
	.type	int_mul,@function
int_mul:
	.cfi_startproc
	movl	%edi, %eax
	movl	%edi, -4(%rsp)
	movl	%esi, -8(%rsp)
	imull	%esi, %eax
	retq
.Lfunc_end2:
	.size	int_mul, .Lfunc_end2-int_mul
	.cfi_endproc

	.globl	int_div
	.p2align	4, 0x90
	.type	int_div,@function
int_div:
	.cfi_startproc
	movl	%edi, %eax
	movl	%edi, -4(%rsp)
	movl	%esi, -8(%rsp)
	cltd
	idivl	%esi
	retq
.Lfunc_end3:
	.size	int_div, .Lfunc_end3-int_div
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
