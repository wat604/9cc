	.file	"test_func.c"
	.intel_syntax noprefix
	.text
	.section	.rodata
.LC0:
	.string	"function call OK "
	.text
	.globl	foo
	.type	foo, @function
foo:
	endbr64
	push	rbp
	mov	rbp, rsp
	lea	rax, .LC0[rip]
	mov	rdi, rax
	call	puts@PLT
	nop
	pop	rbp
	ret
	.size	foo, .-foo
	.globl	myadd
	.type	myadd, @function
myadd:
	endbr64
	push	rbp
	mov	rbp, rsp
	mov	DWORD PTR -4[rbp], edi
	mov	DWORD PTR -8[rbp], esi
	mov	edx, DWORD PTR -4[rbp]
	mov	eax, DWORD PTR -8[rbp]
	add	eax, edx
	pop	rbp
	ret
	.size	myadd, .-myadd
	.globl	myadd3
	.type	myadd3, @function
myadd3:
	endbr64
	push	rbp
	mov	rbp, rsp
	mov	DWORD PTR -4[rbp], edi
	mov	DWORD PTR -8[rbp], esi
	mov	DWORD PTR -12[rbp], edx
	mov	edx, DWORD PTR -4[rbp]
	mov	eax, DWORD PTR -8[rbp]
	add	edx, eax
	mov	eax, DWORD PTR -12[rbp]
	add	eax, edx
	pop	rbp
	ret
	.size	myadd3, .-myadd3
	.globl	call_myadd
	.type	call_myadd, @function
call_myadd:
	endbr64
	push	rbp
	mov	rbp, rsp
	mov	esi, 2
	mov	edi, 1
	call	myadd
	mov	edx, 3
	mov	esi, 2
	mov	edi, 1
	call	myadd3
	pop	rbp
	ret
	.size	call_myadd, .-call_myadd
	.ident	"GCC: (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
