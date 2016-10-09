
#pragma once

#include "emmintrin.h"
#include "XMath.h"

#define NEW_SSEARRAT(p,classname,num,align)		{p=(classname*)_mm_malloc(sizeof(classname)*num,align);}
#define SAFE_FREE_SSEARRAY(p)					{if(p){_mm_free(p);(p)=NULL;}}

#define _SHUFFLE_PS(x,y,z,w)					(((w)&3)<<6|((z)&3)<<4|((y)&3)<<2|((x)&3))

#define ALIGN16(x)								__declspec(align(16)) (x)
#define ALIGN4_INIT4(x,a,b,c,d)					ALIGN16(static x[4] = {a,b,c,d})

#define IEEE_SP_ZERO					0
#define IEEE_SP_SIGN					((unsigned long)(1<<31))

__declspec(align(16)) static float SIMD_SP_ONE[4] = {1.0f,1.0f,1.0f,1.0f};
__declspec(align(16)) static float SIMD_SP_ZERO[4] = {1.0f,1.0f,1.0f,0.0f};

__declspec(align(16)) static unsigned long SIMD_SP_quat2mat_x0[4] = {IEEE_SP_ZERO,IEEE_SP_SIGN,IEEE_SP_SIGN,IEEE_SP_SIGN};
__declspec(align(16)) static unsigned long SIMD_SP_quat2mat_x1[4] = {IEEE_SP_SIGN,IEEE_SP_ZERO,IEEE_SP_SIGN,IEEE_SP_SIGN};
__declspec(align(16)) static unsigned long SIMD_SP_quat2mat_x2[4] = {IEEE_SP_ZERO,IEEE_SP_SIGN,IEEE_SP_SIGN,IEEE_SP_SIGN};

namespace XMathSIMD
{

	/* SSE
	void _memcpy(void* dst,const void* src,unsigned long nSize);
	说明:复制指定的内存到指写的内存
	参数: void* dst 目标内存指针,4字节对齐
	const void* src 源内存的指针,4字节对齐
	unsigned long 欲填充的长度,单位为字节,必须为DWORD的整数倍,即能被4整除,大于等于4字节
	返回值:无
	*/
	inline void _memcpy(void* dst,const void* src,unsigned long nSize)
	{
		_asm
		{
		mov edi,dst
		mov esi,src
		mov edx,nSize
		;前置检查,检查源地址与目标地址的对齐情况,以便分别处理
		/*test esi,07h ;检查源地址是否8字节对齐
		jnz chkalign1

		test edi,07h ;检查目标地址是否8字节对齐
		jnz copy2
		jmp copy1
		chkalign1:
		test edi,07h ;检查目标地址是否8字节对齐
		jnz copy4
		jmp copy3*/

		;为四种不同的情况做单独的处理

		//copy1: ;对齐读对齐写
		mov ecx,edi ;取得目标地址
		and ecx,0FFFFFFE0h ;取得上一个32字节对齐的地址
		jz premain ;对齐则跳过头块处理

		add ecx,20h ;计算出下一个32字节对齐的地址

		sub ecx,edi ;计算出到下一个32字节对齐的地址之间的字节数
		and ecx,1Fh ;取低5位

		cmp edx,ecx ;减掉即将写的字节数
		jl preback

		sub edx,ecx
		shr ecx,2h ;将字节数转换为循环次数
		jz premain

		mov ebx,[esi+0]
		align 8
		forwardblock1: ;处理未对齐的头部块
		movd mm0,[esi+0]
		movd [edi+0],mm0
		add esi,4
		add edi,4
		loop forwardblock1

		premain:
		mov ebx,edx
		shr ebx,10h ;以64K字节为一个块进行n个块拷贝
		jz preback ;小于64K字节则跳到尾块处理
		push edx ;压入栈中待用

		align 8
		mainloop1:
		mov eax,8192*8/128 ;预读128字节的次数
		xor ecx,ecx
		align 8
		prefetchloop1:
		mov edx, [esi+ecx*8]  ;预读
		mov edx, [esi+ecx*8+64] ;预读下64字节
		add ecx, 16
		dec eax
		jnz prefetchloop1

		mov eax,8192*8/8 ;每个64K块中写64字节的次数
		xor ecx,ecx
		align 8
		copyloop1:
		movq mm0,qword ptr [esi+ecx*8+00]
		movq mm1,qword ptr [esi+ecx*8+8]
		movq mm2,qword ptr [esi+ecx*8+16]
		movq mm3,qword ptr [esi+ecx*8+24]

		movntq qword ptr [edi+ecx*8+00],mm0
		movntq qword ptr [edi+ecx*8+8],mm1
		movntq qword ptr [edi+ecx*8+16],mm2
		movntq qword ptr [edi+ecx*8+24],mm3

		movq mm4,qword ptr [esi+ecx*8+32]
		movq mm5,qword ptr [esi+ecx*8+40]
		movq mm6,qword ptr [esi+ecx*8+48]
		movq mm7,qword ptr [esi+ecx*8+56]

		movntq qword ptr [edi+ecx*8+32],mm4
		movntq qword ptr [edi+ecx*8+40],mm5
		movntq qword ptr [edi+ecx*8+48],mm6
		movntq qword ptr [edi+ecx*8+56],mm7

		add ecx,8
		cmp eax,ecx
		jnz copyloop1

		add esi,8192*8 ;移动到下一个读入地址
		add edi,8192*8 ;移动到下一个写入地址
		dec ebx ;是否所有的块都已拷贝完
		jnz mainloop1

		pop edx ;取得压入栈中的字节数
		and edx,0FFFFh ;取得剩余的字节数
		jz EXIT

		preback:
		mov ecx,edx
		shr ecx,6 ;剩余的64字节的块数
		jnz backwardblock1_64

		mov ecx,edx
		shr ecx,2
		jz EXIT

		mov ebx,[esi] ;预读
		jmp backwardblock1_4

		align 8
		backwardblock1_64: ;处理剩余的64字节尾部块
		mov ebx,[esi+0]

		movq mm0,qword ptr[esi]
		movq mm1,qword ptr[esi+8]
		movq mm2,qword ptr[esi+16]
		movq mm3,qword ptr[esi+24]

		movntq qword ptr[edi],mm0
		movntq qword ptr[edi+8],mm1
		movntq qword ptr[edi+16],mm2
		movntq qword ptr[edi+24],mm3

		movq mm4,qword ptr[esi+32]
		movq mm5,qword ptr[esi+40]
		movq mm6,qword ptr[esi+48]
		movq mm7,qword ptr[esi+56]

		movntq qword ptr[edi+32],mm4
		movntq qword ptr[edi+40],mm5
		movntq qword ptr[edi+48],mm6
		movntq qword ptr[edi+56],mm7

		add esi,64
		add edi,64
		loop backwardblock1_64

		mov ecx,edx
		and ecx,03Fh
		shr ecx,2
		jz EXIT

		prefetchnta [esi] ;预读

		align 8
		backwardblock1_4: ;处理剩余的4字节尾部块
		movd mm0,dword ptr [esi]
		movd [edi],mm0

		add esi,4
		add edi,4
		loop backwardblock1_4

		jmp EXIT
		/*copy2: ;对齐读未对齐写
		nop
		copy3: ;未对齐读对齐写
		nop
		copy4: ;未对齐读未对齐写
		nop*/
		EXIT:
		sfence
		emms
		}
		return;
	}

	// Very optimized memcpy() routine for all AMD Athlon and Duron family.
	// This code uses any of FOUR different basic copy methods, depending
	// on the transfer size.
	// NOTE:  Since this code uses MOVNTQ (also known as "Non-Temporal MOV" or
	// "Streaming Store"), and also uses the software prefetchnta instructions,
	// be sure youre running on Athlon/Duron or other recent CPU before calling!

	#define TINY_BLOCK_COPY 64       // upper limit for movsd type copy
	// The smallest copy uses the X86 "movsd" instruction, in an optimized
	// form which is an "unrolled loop".

	#define IN_CACHE_COPY 64 * 1024  // upper limit for movq/movq copy w/SW prefetch
	// Next is a copy that uses the MMX registers to copy 8 bytes at a time,
	// also using the "unrolled loop" optimization.   This code uses
	// the software prefetch instruction to get the data into the cache.

	#define UNCACHED_COPY 197 * 1024 // upper limit for movq/movntq w/SW prefetch
	// For larger blocks, which will spill beyond the cache, its faster to
	// use the Streaming Store instruction MOVNTQ.   This write instruction
	// bypasses the cache and writes straight to main memory.  This code also
	// uses the software prefetch instruction to pre-read the data.
	// USE 64 * 1024 FOR THIS VALUE IF YOURE ALWAYS FILLING A "CLEAN CACHE"

	#define BLOCK_PREFETCH_COPY  infinity // no limit for movq/movntq w/block prefetch 
	#define CACHEBLOCK 80h // number of 64-byte blocks (cache lines) for block prefetch
	// For the largest size blocks, a special technique called Block Prefetch
	// can be used to accelerate the read operations.   Block Prefetch reads
	// one address per cache line, for a series of cache lines, in a short loop.
	// This is faster than using software prefetch.  The technique is great for
	// getting maximum read bandwidth, especially in DDR memory systems.

	// Inline assembly syntax for use with Visual C++

	inline void * memcpy_amd(void *dest, const void *src, size_t n)
	{
        //return memcpy(dest, src, n);
	__asm {

		mov		ecx, [n]		; number of bytes to copy
		mov		edi, [dest]		; destination
		mov		esi, [src]		; source
		mov		ebx, ecx		; keep a copy of count

		cld
		cmp		ecx, TINY_BLOCK_COPY
		jb		$memcpy_ic_3	; tiny? skip mmx copy

		cmp		ecx, 32*1024		; dont align between 32k-64k because
		jbe		$memcpy_do_align	;  it appears to be slower
		cmp		ecx, 64*1024
		jbe		$memcpy_align_done
	$memcpy_do_align:
		mov		ecx, 8			; a trick thats faster than rep movsb...
		sub		ecx, edi		; align destination to qword
		and		ecx, 111b		; get the low bits
		sub		ebx, ecx		; update copy count
		neg		ecx				; set up to jump into the array
		add		ecx, offset $memcpy_align_done
		jmp		ecx				; jump to array of movsbs

	align 4
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb

	$memcpy_align_done:			; destination is dword aligned
		mov		ecx, ebx		; number of bytes left to copy
		shr		ecx, 6			; get 64-byte block count
		jz		$memcpy_ic_2	; finish the last few bytes

		cmp		ecx, IN_CACHE_COPY/64	; too big 4 cache? use uncached copy
		jae		$memcpy_uc_test

	// This is small block copy that uses the MMX registers to copy 8 bytes
	// at a time.  It uses the "unrolled loop" optimization, and also uses
	// the software prefetch instruction to get the data into the cache.
	align 16
	$memcpy_ic_1:			; 64-byte block copies, in-cache copy

		prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		movq	mm0, [esi+0]	; read 64 bits
		movq	mm1, [esi+8]
		movq	[edi+0], mm0	; write 64 bits
		movq	[edi+8], mm1	;    note:  the normal movq writes the
		movq	mm2, [esi+16]	;    data to cache; a cache line will be
		movq	mm3, [esi+24]	;    allocated as needed, to store the data
		movq	[edi+16], mm2
		movq	[edi+24], mm3
		movq	mm0, [esi+32]
		movq	mm1, [esi+40]
		movq	[edi+32], mm0
		movq	[edi+40], mm1
		movq	mm2, [esi+48]
		movq	mm3, [esi+56]
		movq	[edi+48], mm2
		movq	[edi+56], mm3

		add		esi, 64			; update source pointer
		add		edi, 64			; update destination pointer
		dec		ecx				; count down
		jnz		$memcpy_ic_1	; last 64-byte block?

	$memcpy_ic_2:
		mov		ecx, ebx		; has valid low 6 bits of the byte count
	$memcpy_ic_3:
		shr		ecx, 2			; dword count
		and		ecx, 1111b		; only look at the "remainder" bits
		neg		ecx				; set up to jump into the array
		add		ecx, offset $memcpy_last_few
		jmp		ecx				; jump to array of movsds

	$memcpy_uc_test:
		cmp		ecx, UNCACHED_COPY/64	; big enough? use block prefetch copy
		jae		$memcpy_bp_1

	$memcpy_64_test:
		or		ecx, ecx		; tail end of block prefetch will jump here
		jz		$memcpy_ic_2	; no more 64-byte blocks left

	// For larger blocks, which will spill beyond the cache, its faster to
	// use the Streaming Store instruction MOVNTQ.   This write instruction
	// bypasses the cache and writes straight to main memory.  This code also
	// uses the software prefetch instruction to pre-read the data.
	align 16
	$memcpy_uc_1:				; 64-byte blocks, uncached copy

		prefetchnta [esi + (200*64/34+192)]		; start reading ahead

		movq	mm0,[esi+0]		; read 64 bits
		add		edi,64			; update destination pointer
		movq	mm1,[esi+8]
		add		esi,64			; update source pointer
		movq	mm2,[esi-48]
		movntq	[edi-64], mm0	; write 64 bits, bypassing the cache
		movq	mm0,[esi-40]	;    note: movntq also prevents the CPU
		movntq	[edi-56], mm1	;    from READING the destination address
		movq	mm1,[esi-32]	;    into the cache, only to be over-written
		movntq	[edi-48], mm2	;    so that also helps performance
		movq	mm2,[esi-24]
		movntq	[edi-40], mm0
		movq	mm0,[esi-16]
		movntq	[edi-32], mm1
		movq	mm1,[esi-8]
		movntq	[edi-24], mm2
		movntq	[edi-16], mm0
		dec		ecx
		movntq	[edi-8], mm1
		jnz		$memcpy_uc_1	; last 64-byte block?

		jmp		$memcpy_ic_2		; almost done

	// For the largest size blocks, a special technique called Block Prefetch
	// can be used to accelerate the read operations.   Block Prefetch reads
	// one address per cache line, for a series of cache lines, in a short loop.
	// This is faster than using software prefetch, in this case.
	// The technique is great for getting maximum read bandwidth,
	// especially in DDR memory systems.
	$memcpy_bp_1:			; large blocks, block prefetch copy

		cmp		ecx, CACHEBLOCK			; big enough to run another prefetch loop?
		jl		$memcpy_64_test			; no, back to regular uncached copy

		mov		eax, CACHEBLOCK / 2		; block prefetch loop, unrolled 2X
		add		esi, CACHEBLOCK * 64	; move to the top of the block
	align 16
	$memcpy_bp_2:
		mov		edx, [esi-64]		; grab one address per cache line
		mov		edx, [esi-128]		; grab one address per cache line
		sub		esi, 128			; go reverse order
		dec		eax					; count down the cache lines
		jnz		$memcpy_bp_2		; keep grabbing more lines into cache

		mov		eax, CACHEBLOCK		; now that its in cache, do the copy
	align 16
	$memcpy_bp_3:
		movq	mm0, [esi   ]		; read 64 bits
		movq	mm1, [esi+ 8]
		movq	mm2, [esi+16]
		movq	mm3, [esi+24]
		movq	mm4, [esi+32]
		movq	mm5, [esi+40]
		movq	mm6, [esi+48]
		movq	mm7, [esi+56]
		add		esi, 64				; update source pointer
		movntq	[edi   ], mm0		; write 64 bits, bypassing cache
		movntq	[edi+ 8], mm1		;    note: movntq also prevents the CPU
		movntq	[edi+16], mm2		;    from READING the destination address 
		movntq	[edi+24], mm3		;    into the cache, only to be over-written,
		movntq	[edi+32], mm4		;    so that also helps performance
		movntq	[edi+40], mm5
		movntq	[edi+48], mm6
		movntq	[edi+56], mm7
		add		edi, 64				; update dest pointer

		dec		eax					; count down

		jnz		$memcpy_bp_3		; keep copying
		sub		ecx, CACHEBLOCK		; update the 64-byte block count
		jmp		$memcpy_bp_1		; keep processing chunks

	// The smallest copy uses the X86 "movsd" instruction, in an optimized
	// form which is an "unrolled loop".   Then it handles the last few bytes.
	align 4
		movsd
		movsd			; perform last 1-15 dword copies
		movsd
		movsd
		movsd
		movsd
		movsd
		movsd
		movsd
		movsd			; perform last 1-7 dword copies
		movsd
		movsd
		movsd
		movsd
		movsd
		movsd

	$memcpy_last_few:		; dword aligned from before movsds
		mov		ecx, ebx	; has valid low 2 bits of the byte count
		and		ecx, 11b	; the last few cows must come home
		jz		$memcpy_final	; no more, lets leave
		rep		movsb		; the last 1, 2, or 3 bytes

	$memcpy_final: 
		emms				; clean up the MMX state
		sfence				; flush the write buffer
		mov		eax, [dest]	; ret value = destination pointer

		}
	}

	// DOOM3 opti
	inline float ReciprocalSqrt(float x) 
	{
		long i;
		float y, r;
		y = x * 0.5f;
		i = *(long *)( &x );
		i = 0x5f3759df - ( i >> 1 );
		r = *(float *)( &i );
		r = r * ( 1.5f - r * r * y );
		return r;
	};

	inline float SinZeroHalfPI(float a) 
	{
		float s, t;
		s = a * a;
		t = -2.39e-08f;
		t *= s;
		t += 2.7526e-06f;
		t *= s;
		t += -1.98409e-04f;
		t *= s;
		t += 8.3333315e-03f;
		t *= s;
		t += -1.666666664e-01f;
		t *= s;
		t += 1.0f;
		t *= a;
		return t;
	};

	inline float ATanPositive(float y,float x) 
	{
		float a, d, s, t;
		if ( y > x ) 
		{
			a = -x / y;
			d = 3.14159265358979323846f / 2.0f;
		}
		else
		{
			a = y / x;
			d = 0.0f;
		}
		s = a * a;
		t = 0.0028662257f;
		t *= s;
		t += -0.0161657367f;
		t *= s;
		t += 0.0429096138f;
		t *= s;
		t += -0.0752896400f;
		t *= s;
		t += 0.1065626393f;
		t *= s;
		t += -0.1420889944f;
		t *= s;
		t += 0.1999355085f;
		t *= s;
		t += -0.3333314528f;
		t *= s;
		t += 1.0f;
		t *= a;
		t += d;
		return t;
	};

	//quaternion float[4]
	inline void QuaternionSlerpSSE(float* destquaternion,const float* srcquaternion1,const float* srcquaternion2,float value)
	{
		float cosom, absCosom, sinom, sinSqr, omega, scale0, scale1;

		cosom = srcquaternion1[0]*srcquaternion2[0]+srcquaternion1[1]*srcquaternion2[1]+srcquaternion1[2]*srcquaternion2[2]+srcquaternion1[3]*srcquaternion2[3];
		absCosom = fabs( cosom );
		if ( ( 1.0f - absCosom ) > 1e-6f ) {
		sinSqr = 1.0f - absCosom * absCosom;
		sinom = ReciprocalSqrt( sinSqr );
		omega = ATanPositive( sinSqr * sinom, absCosom );
		scale0 = SinZeroHalfPI( ( 1.0f - value ) * omega ) * sinom;
		scale1 = SinZeroHalfPI( value * omega ) * sinom;
		} else {
		scale0 = 1.0f - value;
		scale1 = value;
		}
		scale1 = ( cosom >= 0.0f ) ? scale1 : -scale1;
		destquaternion[0] = scale0 * srcquaternion1[0] + scale1 * srcquaternion2[0];
		destquaternion[1] = scale0 * srcquaternion1[1] + scale1 * srcquaternion2[1];
		destquaternion[2] = scale0 * srcquaternion1[2] + scale1 * srcquaternion2[2];
		destquaternion[3] = scale0 * srcquaternion1[3] + scale1 * srcquaternion2[3];
	};

	// matrix float[4][4],quaternion float[4],translation float[4]
	inline void QuaternionTranslation2MatrixSSE(float* matrix,const float* quaternion,const float* translation)
	{
	_asm
		{
			push	ebx

			mov		esi,quaternion
			mov		edi,matrix
			mov		eax,translation
				
			movaps	xmm0,[esi]
			movaps	xmm6,[eax]

			movaps	xmm1,xmm0
			addps	xmm1,xmm1

			pshufd	xmm2,xmm0,_SHUFFLE_PS(1,0,0,1)
			pshufd	xmm3,xmm1,_SHUFFLE_PS(1,1,2,2)
			mulps	xmm2,xmm3

			pshufd	xmm4,xmm0,_SHUFFLE_PS(2,3,3,3)
			pshufd	xmm5,xmm1,_SHUFFLE_PS(2,2,1,0)
			mulps	xmm4,xmm5

			mulss	xmm0,xmm1

			//
			movss	xmm7,SIMD_SP_ONE
			subss	xmm7,xmm0
			subss	xmm7,xmm2

			//
			xorps	xmm2,SIMD_SP_quat2mat_x0
			xorps	xmm4,SIMD_SP_quat2mat_x1
			addss	xmm4,SIMD_SP_ONE
			movaps	xmm3,xmm4
			subps	xmm3,xmm2
			movaps	xmm1,SIMD_SP_ZERO
			mulps	xmm1,xmm3
			movaps	[edi],xmm1

			//
			movss	xmm2,xmm0
			xorps	xmm4,SIMD_SP_quat2mat_x2
			subps	xmm4,xmm2
			shufps	xmm4,xmm4,_SHUFFLE_PS(1,0,3,2)
			movaps	xmm1,SIMD_SP_ZERO
			mulps	xmm1,xmm4
			movaps	[edi+16],xmm1

			//
			movhlps	xmm3,xmm4
			shufps	xmm3,xmm7,_SHUFFLE_PS(1,3,0,2)
			movaps	[edi+32],xmm3

			//
			movaps	xmm1,SIMD_SP_ZERO
			mulps	xmm1,xmm6
			movaps	[edi+48],xmm1

			pop		ebx
		};

		matrix[15] = 1.0f; // add by lily
	};

	// matrix float[4][4],quaternion float[4],translation float[4]
	inline void QuaternionTranslationS2MatrixSSSE(float* matrix,int matrixdistance,const float* quaternion,int quaterniondistance,const float* translation,int translationdistance,int count)
	{
		_asm
		{
			push	ebx

			mov		ecx,count

			mov		esi,quaternion
			mov		edi,matrix
			mov		eax,translation

		loopquaterniontranslation2matrix:
				
			movaps	xmm0,[esi]
			movaps	xmm6,[eax]
				
			add		esi,quaterniondistance
			add 	eax,translationdistance

			movaps	xmm1,xmm0
			addps	xmm1,xmm1

			pshufd	xmm2,xmm0,_SHUFFLE_PS(1,0,0,1)
			pshufd	xmm3,xmm1,_SHUFFLE_PS(1,1,2,2)
			mulps	xmm2,xmm3

			pshufd	xmm4,xmm0,_SHUFFLE_PS(2,3,3,3)
			pshufd	xmm5,xmm1,_SHUFFLE_PS(2,2,1,0)
			mulps	xmm4,xmm5

			mulss	xmm0,xmm1

			//
			movss	xmm7,SIMD_SP_ONE
			subss	xmm7,xmm0
			subss	xmm7,xmm2

			//
			xorps	xmm2,SIMD_SP_quat2mat_x0
			xorps	xmm4,SIMD_SP_quat2mat_x1
			addss	xmm4,SIMD_SP_ONE
			movaps	xmm3,xmm4
			subps	xmm3,xmm2
			movaps	xmm1,SIMD_SP_ZERO
			mulps	xmm1,xmm3
			movaps	[edi],xmm1

			//
			movss	xmm2,xmm0
			xorps	xmm4,SIMD_SP_quat2mat_x2
			subps	xmm4,xmm2
			shufps	xmm4,xmm4,_SHUFFLE_PS(1,0,3,2)
			movaps	xmm1,SIMD_SP_ZERO
			mulps	xmm1,xmm4
			movaps	[edi+16],xmm1

			//
			movhlps	xmm3,xmm4
			shufps	xmm3,xmm7,_SHUFFLE_PS(1,3,0,2)
			movaps	[edi+32],xmm3

			//
			movaps	[edi+48],xmm6

			add		edi,matrixdistance
			dec		ecx
			jnz		loopquaterniontranslation2matrix

			pop		ebx
		};
	};
	// matrix float[4][4]
	inline void MatrixMultiplyMatrixSSE(float* destmatrix,const float* srcmatrix1,const float* srcmatrix2)
	{
		_asm
		{
			push	ebx

			mov		edx,srcmatrix1
			mov		ecx,srcmatrix2
			mov		eax,destmatrix

			movss	xmm0,[edx+16*0+4*0]
			movaps	xmm4,[ecx+16*0]
			movss	xmm1,[edx+16*0+4*1]
			shufps	xmm0,xmm0,00h
			movaps	xmm5,[ecx+16*1]
			movss	xmm2,[edx+16*0+4*2]
			shufps	xmm1,xmm1,00h
			mulps	xmm0,xmm4
			movaps	xmm6,[ecx+16*2]
			mulps	xmm1,xmm5
			movss	xmm3,[edx+16*0+4*3]
			shufps	xmm2,xmm2,00h
			movaps	xmm7,[ecx+16*3]
			shufps	xmm3,xmm3,00h
			mulps	xmm2,xmm6
			addps	xmm1,xmm0

			movss	xmm0,[edx+16*1+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0,xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*1+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*1+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*0],    xmm3
			movss	xmm3,[edx+16*1+4*3]
			mulps	xmm2,xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
				
			movss	xmm0,[edx+16*2+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0,xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*2+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*2+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*1],    xmm3
			movss	xmm3,[edx+16*2+4*3]
			mulps	xmm2,xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
				
			movss	xmm0,[edx+16*3+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0, xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*3+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*3+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*2],    xmm3
			movss	xmm3,[edx+16*3+4*3]
			mulps	xmm2, xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
			mulps	xmm3,xmm7
			addps	xmm2,xmm1
			addps	xmm3,xmm2
			movaps	[eax+16*3],xmm3

			pop		ebx
		}
	};

	// matrix float[4][4]
	inline void MatrixSMultiplyMatrixSSE(float* destmatrix,int destmatrixdistance,const float* srcmatrix1,int srcmatrix1distance,const float* srcmatrix2,int count)
	{
		_asm
		{
			push	ebx

			mov		edx,srcmatrix1
			mov		ecx,srcmatrix2
			mov		eax,destmatrix

			mov		ebx,count

			movups	xmm4,[ecx+16*0]		// 16byte 对器
			movups	xmm5,[ecx+16*1]		// 16byte 对器
			movups	xmm6,[ecx+16*2]		// 16byte 对器
			movups	xmm7,[ecx+16*3]		// 16byte 对器

		loopmatrixmultipymatrix:

			movss	xmm0,[edx+16*0+4*0]
			movss	xmm1,[edx+16*0+4*1]
			shufps	xmm0,xmm0,00h
			movss	xmm2,[edx+16*0+4*2]
			shufps	xmm1,xmm1,00h
			mulps	xmm0,xmm4
			mulps	xmm1,xmm5
			movss	xmm3,[edx+16*0+4*3]
			shufps	xmm2,xmm2,00h
				
			shufps	xmm3,xmm3,00h
			mulps	xmm2,xmm6
			addps	xmm1,xmm0

			movss	xmm0,[edx+16*1+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0,xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*1+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*1+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*0],xmm3
			movss	xmm3,[edx+16*1+4*3]
			mulps	xmm2,xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
				
			movss	xmm0,[edx+16*2+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0,xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*2+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*2+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*1],xmm3
			movss	xmm3,[edx+16*2+4*3]
			mulps	xmm2,xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
				
			movss	xmm0,[edx+16*3+4*0]
			mulps	xmm3,xmm7
			shufps	xmm0, xmm0,00h
			addps	xmm2,xmm1
			movss	xmm1,[edx+16*3+4*1]
			mulps	xmm0,xmm4
			shufps	xmm1,xmm1,00h
			addps	xmm3,xmm2
			movss	xmm2,[edx+16*3+4*2]
			mulps	xmm1,xmm5
			shufps	xmm2,xmm2,00h
			movaps	[eax+16*2],xmm3
			movss	xmm3,[edx+16*3+4*3]
			mulps	xmm2, xmm6
			shufps	xmm3,xmm3,00h
			addps	xmm1,xmm0
			mulps	xmm3,xmm7
			addps	xmm2,xmm1
			addps	xmm3,xmm2
			movaps	[eax+16*3],xmm3

			add		edx,srcmatrix1distance
			add		eax,destmatrixdistance
			dec		ebx
			jnz		loopmatrixmultipymatrix

			pop		ebx
		}
	};

	// vector float[4],matrix float[4][4]
	inline void VectorSMultiplyMatrixSSE(float* destvector,int destvectordistance,const float* srcvector,int srcvectordistance,const float* matrix,int count)
	{
		_asm
		{
			push	ebx

			mov		edx,matrix
			mov		eax,srcvector
			mov		ebx,destvector
				
			mov		ecx,count

		loopvectormultiplymatrix:

			movaps	xmm0,[eax]
			add		eax,srcvectordistance

			movaps	xmm1,xmm0
			movups	xmm4,[edx+16*0]						// 16byte 对器
			shufps	xmm1,xmm1,_SHUFFLE_PS(0,0,0,0)
			movups	xmm5,[edx+16*1]						// 16byte 对器
			mulps	xmm1,xmm4
			movaps	xmm2,xmm0
			shufps	xmm2,xmm2,_SHUFFLE_PS(1,1,1,1)
			movups	xmm6,[edx+16*2]						// 16byte 对器
			mulps	xmm2,xmm5
			addps	xmm1,xmm2
			movaps	xmm2,xmm0
			shufps	xmm2,xmm2,_SHUFFLE_PS(2,2,2,2)
			movups	xmm7,[edx+16*3]						// 16byte 对器
			mulps	xmm2,xmm6
			addps	xmm1,xmm2
			movaps	xmm2,xmm0
			shufps	xmm2,xmm2,_SHUFFLE_PS(3,3,3,3)
			mulps	xmm2,xmm7
			addps	xmm1,xmm2

			movaps	[ebx],xmm1
			add		ebx,destvectordistance

			dec		ecx
			jnz		loopvectormultiplymatrix

			pop		ebx
		};
	};

	// vector float[4]
	inline void VectorSMaxMinSSE(const float* vector,int vectordistance,float* maxvector,float* minvector,int count)
	{
		_asm
		{
			push	ebx

			mov		edx,maxvector
			mov		eax,minvector
			mov		ebx,vector
				
			mov		ecx,count

			movaps	xmm0,[edx]			
			movaps	xmm1,[eax]			

		loopvectormaxmin:
		
			movups	xmm2,[ebx]
			maxps	xmm0,xmm2
			minps	xmm1,xmm2

			add		ebx,vectordistance

			dec		ecx
			jnz		loopvectormaxmin

			movaps	[edx],xmm0
			movaps	[eax],xmm1

			pop		ebx
		};
	};

	// vector float[4]
	inline void VectorDotVectorSSE(float* result,const float* srcvector1,const float* srcvector2)
	{
		_asm
		{
			push	ebx

			mov		eax,srcvector1
			mov		ebx,srcvector2
			mov		edx,result

			movaps	xmm0,[eax]
			movaps	xmm1,[ebx]

			mulps	xmm0,xmm1

			movaps	xmm1,xmm0
			shufps	xmm2,xmm0,_SHUFFLE_PS(1,1,1,1)
			movaps	xmm3,xmm0
				
			addps	xmm1,xmm2
				
			shufps	xmm3,xmm3,_SHUFFLE_PS(2,2,2,2)
			movaps	xmm4,xmm0
			addps	xmm1,xmm3
				
			shufps	xmm4,xmm4,_SHUFFLE_PS(2,2,2,2)
			addps	xmm1,xmm4

			movss	[edx],xmm1

			pop		ebx
		};
	};

	inline void MemAddDWORDSSE(DWORD* dest,const DWORD* src,const DWORD* value,int count)
	{
		static int t_count1;
		static int t_count2;

		t_count1 = count/28;
		t_count2 = count - t_count1*28;

		_asm
		{
			push	ebx

			mov		eax,dest
			mov		edx,src
			mov		ebx,value

			movss	xmm0,[ebx]
			shufps	xmm0,xmm0,_SHUFFLE_PS(0,0,0,0)

			mov		ecx,t_count1
			test	ecx,ecx
			jz		loopmemsetsseend
				
		loopmemsetsse:

			movaps	xmm1,[edx+ 0]
			movaps	xmm2,[edx+16]
			movaps	xmm3,[edx+32]
			movaps	xmm4,[edx+48]
			movaps	xmm5,[edx+64]
			movaps	xmm6,[edx+80]
			movaps	xmm7,[edx+96]

			addps	xmm1,xmm0
			addps	xmm2,xmm0
			addps	xmm3,xmm0
			addps	xmm4,xmm0
			addps	xmm5,xmm0
			addps	xmm6,xmm0
			addps	xmm7,xmm0
				
			movups	[eax+ 0],xmm1
			movups	[eax+16],xmm2
			movups	[eax+32],xmm3
			movups	[eax+48],xmm4
			movups	[eax+64],xmm5
			movups	[eax+80],xmm6
			movups	[eax+96],xmm7

			add		eax,112
			add		edx,112

			dec		ecx
			jnz		loopmemsetsse

		loopmemsetsseend:

			mov		ecx,t_count2
			test	ecx,ecx
			jz		loopmemsetend

		loopmemset:
				
			movss	xmm1,[edx]
			addss	xmm1,xmm0
			movss	[eax],xmm1
			add		eax,4
			add		edx,4
				
			dec		ecx
			jnz		loopmemset
			
		loopmemsetend:
			pop		ebx
		};
	};

	inline void MemAddDWORDSSE2(DWORD* dest,const DWORD* src,const DWORD* value,int count)
	{
		static int t_count1;
		static int t_count2;

		t_count1 = count/4;
		t_count2 = count - t_count1*4;

		_asm
		{
			push	ebx

			mov		eax,dest
			mov		edx,src
			mov		ebx,value

			movss	xmm0,[ebx]
			shufps	xmm0,xmm0,_SHUFFLE_PS(0,0,0,0)

			mov		ecx,t_count1
			test	ecx,ecx
			jz		loopmemsetsseend
				
		loopmemsetsse:

			movdqa	xmm1,xmmword ptr [edx]
			paddd	xmm1,xmm0
			movntdq	xmmword ptr [eax],xmm1

			add		eax,16
			add		edx,16

			dec		ecx
			jnz		loopmemsetsse

		loopmemsetsseend:

			mov		ecx,t_count2
			test	ecx,ecx
			jz		loopmemsetend

		loopmemset:
				
			movss	xmm1,[edx]
			addss	xmm1,xmm0
			movss	[eax],xmm1
			add		eax,4
			add		edx,4
				
			dec		ecx
			jnz		loopmemset
			
		loopmemsetend:
			pop		ebx
		};
	};

	// vector float[4]
	inline void MemSetDWORDSSE(DWORD* dest,const DWORD* value,int count)
	{
		static int t_count1;
		static int t_count2;

		t_count1 = count/4;
		t_count2 = count - t_count1*4;

		_asm
		{
			push	ebx

			mov		eax,dest
			mov		ebx,value

			movss	xmm1,[ebx]
			shufps	xmm1,xmm1,_SHUFFLE_PS(0,0,0,0)

			mov		ecx,t_count1
			test	ecx,ecx
			jz		loopmemsetsseend
				
		loopmemsetsse:

			movups	[eax],xmm1

			add		eax,16

			dec		ecx
			jnz		loopmemsetsse

		loopmemsetsseend:

			mov		ecx,t_count2
			test	ecx,ecx
			jz		loopmemsetend

		loopmemset:
				
			movss	[eax],xmm1
			add		eax,4
				
			dec		ecx
			jnz		loopmemset
			
		loopmemsetend:
			pop		ebx
		};
	};


	// vector float[4]
	inline void MemSetDWORDSSE2(DWORD* dest,const DWORD* value,int count)
	{
		static int t_count1;
		static int t_count2;

		t_count1 = count/4;
		t_count2 = count - t_count1*4;

		_asm
		{
			push	ebx

			mov		eax,dest
			mov		ebx,value

			movss	xmm0,[ebx]
			shufps	xmm0,xmm0,_SHUFFLE_PS(0,0,0,0)

			mov		ecx,t_count1
			test	ecx,ecx
			jz		loopmemsetsseend
				
		loopmemsetsse:

			movntdq	xmmword ptr [eax],xmm0

			add		eax,16

			dec		ecx
			jnz		loopmemsetsse

		loopmemsetsseend:

			mov		ecx,t_count2
			test	ecx,ecx
			jz		loopmemsetend

		loopmemset:
				
			movss	[eax],xmm0
			add		eax,4
				
			dec		ecx
			jnz		loopmemset
			
		loopmemsetend:
			pop		ebx
		};
	};
}
using namespace XMathSIMD;