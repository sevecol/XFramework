
#pragma once
#include "Vector3.h"

namespace Math
{
	class Plane
	{
	public:
		enum PlaneDirection
		{
			Plane_Front,
			Plane_In,
			Plane_Back
		};

	public:
		Plane(){};
		virtual ~Plane(){}

		Plane( float a, float b, float c, float d ) : _vNormal(a, b, c), _d(d)
		{
		}

		Plane( const Vector3f& vNor, float d ) : _vNormal(vNor), _d(d)
		{
		}

		// 点法式
		Plane( const Vector3f& vNor, const Vector3f& vPoint )
		{
			_vNormal = vNor;
			_vNormal.Normalize();
			_d = -_vNormal.Dot(vPoint);
		}

		// v0，v1，v2顺时针排列
		Plane( const Vector3f& v0, const Vector3f& v1, const Vector3f& v2 )
		{
			Vector3f vDir1 = v1 - v0;
			Vector3f vDir2 = v2 - v0;
			_vNormal = vDir1.Cross( vDir2 );
			_vNormal.Normalize();

			_d = -_vNormal.Dot( v0 );
		}

		// Normalize
		void Normalize()
		{
			float len = _vNormal.Length();
			_vNormal.Normalize();
			_d /= len; 
		}

		// 
		PlaneDirection RelationWithPoint( const Vector3f& Point )
		{
			float dot = Point.Dot(_vNormal);
			if( IsZero(dot) )
				return Plane_In;
			else if( dot < 0 )
				return Plane_Back;
			else
				return Plane_Front;
		}

			

	public:
		Vector3f _vNormal;			
		float    _d;
	};
}
using namespace Math;