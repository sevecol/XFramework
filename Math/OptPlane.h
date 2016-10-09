
#pragma  once
#include "AxisAlignedBoundingBox.h"

namespace Math
{
	class OptPlane : public Plane
	{
	public:
		OptPlane(){};
		~OptPlane(){};

		OptPlane( float a, float b, float c, float d ) : Plane( a, b, c, d )
		{
			CalNPVertex();
		}

		OptPlane( const Vector3f& vNor, float d ) : Plane( _vNormal, _d )
		{
			CalNPVertex();
		}

		OptPlane( const Vector3f& vNor, const Vector3f& vPoint ) : Plane( vNor, vPoint )
		{
			CalNPVertex();
		}

		OptPlane( const Vector3f& v0, const Vector3f& v1, const Vector3f& v2 ) : Plane( v0, v1, v2 )
		{
			CalNPVertex();
		}

		void CalNPVertex()
		{
			// _NPVer[0-3] N点索引
			if( _vNormal.x >= 0.0f )
				_NPVer[0] = AxisAlignedBoundingBox::Ver_Min;
			else
				_NPVer[0] = AxisAlignedBoundingBox::Ver_Max;

			if( _vNormal.y >= 0.0f )
				_NPVer[1] = AxisAlignedBoundingBox::Ver_Min;
			else
				_NPVer[1] = AxisAlignedBoundingBox::Ver_Max;

			if( _vNormal.z >= 0.0f )
				_NPVer[2] = AxisAlignedBoundingBox::Ver_Min;
			else
				_NPVer[2] = AxisAlignedBoundingBox::Ver_Max;

		}

	public:
		AxisAlignedBoundingBox::MinMaxFlag _NPVer[4];  // 存放的是N点的索引， P点的分别用1-_NPVer[i]; 0 < i < 3 // 4 for algin
	};
}
using namespace Math;