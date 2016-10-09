
#pragma once

namespace Math
{
	// reinterpret a float as an int32
	#define fpBits(f) (*reinterpret_cast<const INT32*>(&(f))) 
	//:	highestBitSet
	//----------------------------------------------------------------------------------------
	//
	//	Returns the index of the highest bit set in the input value.
	//
	//-------------------------------------------------------------------------------------://
	template<class T>
		inline int highestBitSet(T input)
	{
		register int result;
		assert(input); // zero is invalid input!
		assert(sizeof(T)==4); // 32bit data only!
		_asm bsr eax, input
		_asm mov result, eax
		return result;
	}

	template<>
		inline int highestBitSet (UINT8 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dl, input // copy into a 32bit reg
		_asm and edx, 0xff // keep only the bits we want
		_asm bsr eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}
	template<>
		inline int highestBitSet (INT8 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dl, input // copy into a 32bit reg
		_asm and edx, 0xff // keep only the bits we want
		_asm bsr eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}

	template<>
		inline int highestBitSet (UINT16 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dx, input // copy into a 32bit reg
		_asm and edx, 0xffff // keep only the bits we want
		_asm bsr eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}
	template<>
		inline int highestBitSet (INT16 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dx, input // copy into a 32bit reg
		_asm and edx, 0xffff // keep only the bits we want
		_asm bsr eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}

	template<>
		inline int highestBitSet (float f)
	{
		register int result;
		register uint32 input = fpBits(f);
		assert(input); // zero is invalid input!
		_asm bsr eax, input
		_asm mov result, eax
		return result;
	}

	//:	lowestBitSet
	//----------------------------------------------------------------------------------------
	//
	//	Returns the index of the lowest bit set in the input value.
	//
	//-------------------------------------------------------------------------------------://
	template<class T>
		inline int lowestBitSet(T input)
	{
		register int result;
		assert(input); // zero is invalid input!
		assert(sizeof(T)==4); // 32bit data only!
		_asm bsf eax, input
		_asm mov result, eax
		return result;
	}

	template<>
		inline int lowestBitSet (UINT8 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dl, input // copy into a 32bit reg
		_asm and edx, 0xff // keep only the bits we want
		_asm bsf eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}
	template<>
		inline int lowestBitSet (INT8 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dl, input // copy into a 32bit reg
		_asm and edx, 0xff // keep only the bits we want
		_asm bsf eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}

	template<>
		inline int lowestBitSet (UINT16 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dx, input // copy into a 32bit reg
		_asm and edx, 0xffff // keep only the bits we want
		_asm bsf eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}
	template<>
		inline int lowestBitSet (INT16 input)
	{
		register int result;
		assert(input); // zero is invalid input!
		_asm mov dx, input // copy into a 32bit reg
		_asm and edx, 0xffff // keep only the bits we want
		_asm bsf eax, edx // perform the scan
		_asm mov result, eax
		return result;
	}

	template<>
		inline int lowestBitSet (float f)
	{
		register int result;
		register uint32 input = fpBits(f);
		assert(input); // zero is invalid input!
		_asm bsf eax, input
		_asm mov result, eax
		return result;
	}
}
using namespace Math;