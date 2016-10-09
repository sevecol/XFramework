
#pragma once

//
//  Title:  Cyclic Reducancy Check Template Class Definition
//
//  Copyright 1999, Colosseum Builders, Inc.
//  All rights reserved.
//
//  Colosseum Builders, Inc. makes no warranty, expressed or implied
//  with regards to this software. It is provided as is.
//
//  Author:  John M. Miano (miano@colosseumbuilders.com)
//
//  Date:    March 15, 1999
//
//  Version: 1
//
//  Description:
//
//    This is a template class for implementing various forms of the
//    CRC function.
//
//    Imporant Member functions:
//
//    reset - Places the CRC register in its initial state.
//    update - Update the value of the CRC register from a buffer.
//    value - Fetch the value of the CRC register.
//
//  Template Parameters:
//
//    BITCOUNT: The number of bits in the CRC (16 or 32)
//    POLYNOMIAL: The CRC polynomial value
//    REVERSED: true => Use the reverse form of the polynomial
//              false => The non-reversed form.
//    INITIAL:  The initial value for the CRC Register
//    FINALMASK: A value to be XORed with the register when
//               retrieving the CRC value. This value should be zero
//               if no XOR is required and 0xFFFFFFFF otherwise.
//
//  Revision History:
//
//

namespace Math
{
	template <unsigned int BITCOUNT,
		unsigned long POLYNOMIAL,
		const bool REVERSE,
		unsigned long INITIAL,
		unsigned long FINALMASK>
	class XAPI Crc
	{
	public:
		Crc();
		Crc(const Crc &);
		~Crc() {}
		Crc &operator=(const Crc &);

		uint32 Value() const;
		void Reset();
		void Update(const char *pBuffer, unsigned int length);
	private:
		struct CrcTable
		{
			enum { MAXBYTEVALUES = 256 };
			CrcTable();
			uint32 values[MAXBYTEVALUES];
		};
		static const CrcTable crc_table;
		uint32 crc_register;
	};

	typedef Crc<32, 0x04C11DB7, true,	0xFFFFFFFF, 0xFFFFFFFF> CCRC32Digester;
	typedef Crc<16, 0x8005,		true,	0,			0>			CCRC16Digester;
	typedef Crc<16, 0x1021,		false,	0xFFFF,		0>			CCRCCcittDigester;
	typedef Crc<16, 0x1021,		true,	0xFFFF,		0xFFFF>		CCRCX25Digester;
	typedef Crc<16, 0x1021,		false,	0,			0>			CCRCXmodemDigester;
}
using namespace Math;