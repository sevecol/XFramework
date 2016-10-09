
#pragma once

namespace Math
{
	struct Color4
	{
		union
		{
			UINT32	dwColor;
			UINT8	byColor[4];
		};

		Color4()
		{
			dwColor = 0;
		}

		Color4(UINT32 c )
		{
			dwColor = c;
		}

		Color4(UINT8 a, UINT8 r, UINT8 g, UINT8 b )
		{
			byColor[0] = b;
			byColor[1] = g;
			byColor[2] = r;
			byColor[3] = a;
		}

		operator UINT32() const
		{
			return dwColor;
		}

		const Color4& operator= ( const UINT32& arg )
		{
			dwColor = arg;
			return *this;
		}

		const Color4& operator= ( const Color4& arg )
		{
			dwColor = arg.dwColor;
			return *this;
		}

		const Color4 operator* ( float arg ) const
		{
			return Color4( (UINT8)( byColor[3]*arg + 0.5f ), (UINT8)( byColor[2]*arg + 0.5f ),
				(UINT8)( byColor[1]*arg + 0.5f ), (UINT8)( byColor[0]*arg + 0.5f ) );
		}

		const Color4 operator+ ( const Color4& arg ) const
		{
			return Color4( (UINT8)min( 255, (int)byColor[3] + arg.byColor[3] ), (UINT8)min( 255, (int)byColor[2] + arg.byColor[2] ),
				(UINT8)min( 255, (int)byColor[1] + arg.byColor[1] ), (UINT8)min( 255, (int)byColor[0] + arg.byColor[0] ) );
		}

		const Color4 operator- ( const Color4& arg ) const
		{
			return Color4( (UINT8)max( 0, (int)byColor[3] - arg.byColor[3] ), (UINT8)max( 0, (int)byColor[2] - arg.byColor[2] ),
				(UINT8)max( 0, (int)byColor[1] - arg.byColor[1] ), (UINT8)max( 0, (int)byColor[0] - arg.byColor[0] ) );
		}

		const Color4 operator! () const
		{
			return Color4( byColor[3], byColor[0], byColor[1], byColor[2] );
		}

	};
}
using namespace Math;