























































































































































































































































































































































































































































































































































































































	.text
	.balign 32
.globl INT123_dct36_3dnowext
	
INT123_dct36_3dnowext:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%esi
	pushl	%ebx
	movl	8(%ebp), %eax
	movl	12(%ebp), %esi
	movl	16(%ebp), %ecx
	movl	20(%ebp), %edx
	movl	24(%ebp), %ebx

	movq (%eax),%mm0
	movq 4(%eax),%mm1
	pfadd %mm1,%mm0
	movq %mm0,4(%eax)
	psrlq $32,%mm1
	movq 12(%eax),%mm2
	punpckldq %mm2,%mm1
	pfadd %mm2,%mm1
	movq %mm1,12(%eax)
	psrlq $32,%mm2
	movq 20(%eax),%mm3
	punpckldq %mm3,%mm2
	pfadd %mm3,%mm2
	movq %mm2,20(%eax)
	psrlq $32,%mm3
	movq 28(%eax),%mm4
	punpckldq %mm4,%mm3
	pfadd %mm4,%mm3
	movq %mm3,28(%eax)
	psrlq $32,%mm4
	movq 36(%eax),%mm5
	punpckldq %mm5,%mm4
	pfadd %mm5,%mm4
	movq %mm4,36(%eax)
	psrlq $32,%mm5
	movq 44(%eax),%mm6
	punpckldq %mm6,%mm5
	pfadd %mm6,%mm5
	movq %mm5,44(%eax)
	psrlq $32,%mm6
	movq 52(%eax),%mm7
	punpckldq %mm7,%mm6
	pfadd %mm7,%mm6
	movq %mm6,52(%eax)
	psrlq $32,%mm7
	movq 60(%eax),%mm0
	punpckldq %mm0,%mm7
	pfadd %mm0,%mm7
	movq %mm7,60(%eax)
	psrlq $32,%mm0
	movd 68(%eax),%mm1
	pfadd %mm1,%mm0
	movd %mm0,68(%eax)
	movd 4(%eax),%mm0
	movd 12(%eax),%mm1
	punpckldq %mm1,%mm0
	punpckldq 20(%eax),%mm1
	pfadd %mm1,%mm0
	movd %mm0,12(%eax)
	psrlq $32,%mm0
	movd %mm0,20(%eax)
	psrlq $32,%mm1
	movd 28(%eax),%mm2
	punpckldq %mm2,%mm1
	punpckldq 36(%eax),%mm2
	pfadd %mm2,%mm1
	movd %mm1,28(%eax)
	psrlq $32,%mm1
	movd %mm1,36(%eax)
	psrlq $32,%mm2
	movd 44(%eax),%mm3
	punpckldq %mm3,%mm2
	punpckldq 52(%eax),%mm3
	pfadd %mm3,%mm2
	movd %mm2,44(%eax)
	psrlq $32,%mm2
	movd %mm2,52(%eax)
	psrlq $32,%mm3
	movd 60(%eax),%mm4
	punpckldq %mm4,%mm3
	punpckldq 68(%eax),%mm4
	pfadd %mm4,%mm3
	movd %mm3,60(%eax)
	psrlq $32,%mm3
	movd %mm3,68(%eax)
	movq 24(%eax),%mm0
	movq 48(%eax),%mm1
	movd INT123_COS9+12,%mm2
	punpckldq %mm2,%mm2
	movd INT123_COS9+24,%mm3
	punpckldq %mm3,%mm3
	pfmul %mm2,%mm0
	pfmul %mm3,%mm1
	pushl %eax
	movl $1,%eax
	movd %eax,%mm7
	pi2fd %mm7,%mm7
	popl %eax
	movq 8(%eax),%mm2
	movd INT123_COS9+4,%mm3
	punpckldq %mm3,%mm3
	pfmul %mm3,%mm2
	pfadd %mm0,%mm2
	movq 40(%eax),%mm3
	movd INT123_COS9+20,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfadd %mm3,%mm2
	movq 56(%eax),%mm3
	movd INT123_COS9+28,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfadd %mm3,%mm2
	movq (%eax),%mm3
	movq 16(%eax),%mm4
	movd INT123_COS9+8,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfadd %mm4,%mm3
	movq 32(%eax),%mm4
	movd INT123_COS9+16,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfadd %mm4,%mm3
	pfadd %mm1,%mm3
	movq 64(%eax),%mm4
	movd INT123_COS9+32,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfadd %mm4,%mm3
	movq %mm2,%mm4
	pfadd %mm3,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+0,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 108(%edx),%mm6
	punpckldq 104(%edx),%mm6
	pfmul %mm6,%mm5
	pswapd %mm5,%mm5
	movq %mm5,32(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 32(%edx),%mm6
	punpckldq 36(%edx),%mm6
	pfmul %mm6,%mm5
	movd 32(%esi),%mm6
	punpckldq 36(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,1024(%ebx)
	psrlq $32,%mm5
	movd %mm5,1152(%ebx)
	movq %mm3,%mm4
	pfsub %mm2,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+32,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 140(%edx),%mm6
	punpckldq 72(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,68(%ecx)
	psrlq $32,%mm5
	movd %mm5,0(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 0(%edx),%mm6
	punpckldq 68(%edx),%mm6
	pfmul %mm6,%mm5
	movd 0(%esi),%mm6
	punpckldq 68(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,0(%ebx)
	psrlq $32,%mm5
	movd %mm5,2176(%ebx)
	movq 8(%eax),%mm2
	movq 40(%eax),%mm3
	pfsub %mm3,%mm2
	movq 56(%eax),%mm3
	pfsub %mm3,%mm2
	movd INT123_COS9+12,%mm3
	punpckldq %mm3,%mm3
	pfmul %mm3,%mm2
	movq 16(%eax),%mm3
	movq 32(%eax),%mm4
	pfsub %mm4,%mm3
	movq 64(%eax),%mm4
	pfsub %mm4,%mm3
	movd INT123_COS9+24,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	movq 48(%eax),%mm4
	pfsub %mm4,%mm3
	movq (%eax),%mm4
	pfadd %mm4,%mm3
	movq %mm2,%mm4
	pfadd %mm3,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+4,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 112(%edx),%mm6
	punpckldq 100(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,40(%ecx)
	psrlq $32,%mm5
	movd %mm5,28(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 28(%edx),%mm6
	punpckldq 40(%edx),%mm6
	pfmul %mm6,%mm5
	movd 28(%esi),%mm6
	punpckldq 40(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,896(%ebx)
	psrlq $32,%mm5
	movd %mm5,1280(%ebx)
	movq %mm3,%mm4
	pfsub %mm2,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+28,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 136(%edx),%mm6
	punpckldq 76(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,64(%ecx)
	psrlq $32,%mm5
	movd %mm5,4(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 4(%edx),%mm6
	punpckldq 64(%edx),%mm6
	pfmul %mm6,%mm5
	movd 4(%esi),%mm6
	punpckldq 64(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,128(%ebx)
	psrlq $32,%mm5
	movd %mm5,2048(%ebx)
	movq 8(%eax),%mm2
	movd INT123_COS9+20,%mm3
	punpckldq %mm3,%mm3
	pfmul %mm3,%mm2
	pfsub %mm0,%mm2
	movq 40(%eax),%mm3
	movd INT123_COS9+28,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfsub %mm3,%mm2
	movq 56(%eax),%mm3
	movd INT123_COS9+4,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfadd %mm3,%mm2
	movq (%eax),%mm3
	movq 16(%eax),%mm4
	movd INT123_COS9+32,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfsub %mm4,%mm3
	movq 32(%eax),%mm4
	movd INT123_COS9+8,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfsub %mm4,%mm3
	pfadd %mm1,%mm3
	movq 64(%eax),%mm4
	movd INT123_COS9+16,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfadd %mm4,%mm3
	movq %mm2,%mm4
	pfadd %mm3,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+8,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 116(%edx),%mm6
	punpckldq 96(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,44(%ecx)
	psrlq $32,%mm5
	movd %mm5,24(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 24(%edx),%mm6
	punpckldq 44(%edx),%mm6
	pfmul %mm6,%mm5
	movd 24(%esi),%mm6
	punpckldq 44(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,768(%ebx)
	psrlq $32,%mm5
	movd %mm5,1408(%ebx)
	movq %mm3,%mm4
	pfsub %mm2,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+24,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 132(%edx),%mm6
	punpckldq 80(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,60(%ecx)
	psrlq $32,%mm5
	movd %mm5,8(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 8(%edx),%mm6
	punpckldq 60(%edx),%mm6
	pfmul %mm6,%mm5
	movd 8(%esi),%mm6
	punpckldq 60(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,256(%ebx)
	psrlq $32,%mm5
	movd %mm5,1920(%ebx)
	movq 8(%eax),%mm2
	movd INT123_COS9+28,%mm3
	punpckldq %mm3,%mm3
	pfmul %mm3,%mm2
	pfsub %mm0,%mm2
	movq 40(%eax),%mm3
	movd INT123_COS9+4,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfadd %mm3,%mm2
	movq 56(%eax),%mm3
	movd INT123_COS9+20,%mm4
	punpckldq %mm4,%mm4
	pfmul %mm4,%mm3
	pfsub %mm3,%mm2
	movq (%eax),%mm3
	movq 16(%eax),%mm4
	movd INT123_COS9+16,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfsub %mm4,%mm3
	movq 32(%eax),%mm4
	movd INT123_COS9+32,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfadd %mm4,%mm3
	pfadd %mm1,%mm3
	movq 64(%eax),%mm4
	movd INT123_COS9+8,%mm5
	punpckldq %mm5,%mm5
	pfmul %mm5,%mm4
	pfsub %mm4,%mm3
	movq %mm2,%mm4
	pfadd %mm3,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+12,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 120(%edx),%mm6
	punpckldq 92(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,48(%ecx)
	psrlq $32,%mm5
	movd %mm5,20(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 20(%edx),%mm6
	punpckldq 48(%edx),%mm6
	pfmul %mm6,%mm5
	movd 20(%esi),%mm6
	punpckldq 48(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,640(%ebx)
	psrlq $32,%mm5
	movd %mm5,1536(%ebx)
	movq %mm3,%mm4
	pfsub %mm2,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+20,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 128(%edx),%mm6
	punpckldq 84(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,56(%ecx)
	psrlq $32,%mm5
	movd %mm5,12(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 12(%edx),%mm6
	punpckldq 56(%edx),%mm6
	pfmul %mm6,%mm5
	movd 12(%esi),%mm6
	punpckldq 56(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,384(%ebx)
	psrlq $32,%mm5
	movd %mm5,1792(%ebx)
	movq (%eax),%mm4
	movq 16(%eax),%mm3
	pfsub %mm3,%mm4
	movq 32(%eax),%mm3
	pfadd %mm3,%mm4
	movq 48(%eax),%mm3
	pfsub %mm3,%mm4
	movq 64(%eax),%mm3
	pfadd %mm3,%mm4
	movq %mm7,%mm5
	punpckldq INT123_tfcos36+16,%mm5
	pfmul %mm5,%mm4
	movq %mm4,%mm5
	pfacc %mm5,%mm5
	movd 124(%edx),%mm6
	punpckldq 88(%edx),%mm6
	pfmul %mm6,%mm5
	movd %mm5,52(%ecx)
	psrlq $32,%mm5
	movd %mm5,16(%ecx)
	movq %mm4,%mm6
	punpckldq %mm6,%mm5
	pfsub %mm6,%mm5
	punpckhdq %mm5,%mm5
	movd 16(%edx),%mm6
	punpckldq 52(%edx),%mm6
	pfmul %mm6,%mm5
	movd 16(%esi),%mm6
	punpckldq 52(%esi),%mm6
	pfadd %mm6,%mm5
	movd %mm5,512(%ebx)
	psrlq $32,%mm5
	movd %mm5,1664(%ebx)
	femms
	

	popl	%ebx
	popl	%esi
	leave
	ret
	


