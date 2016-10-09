
#pragma once
#include "BoundingVolume.h"
#include "Vector3.h"
#include "Ray.h"

namespace Math
{
	enum IntersectionResult
	{
		IR_Inside,   // 里面
		IR_Outside,  // 外面
		IR_Intersect // 相交
	};

	enum ClipMask
	{
		CM_Left = 1,
		CM_Right = 1 << 2,
		CM_Top = 1 << 3,
		CM_Bottom = 1 << 4,
		CM_Front = 1 << 5,
		CM_Back = 1 << 6,
		CM_ALL = 0x003f // 0000 0000 0011 1111
	};

	class AxisAlignedBoundingBox : public BoundingVolume
	{
	public:
		Vector3f		_vMinMax[2];
		unsigned long   _startID;
		ClipMask		_planeMask;

		enum MinMaxFlag
		{
			Ver_Min, 
			Ver_Max
		};

	public:
		AxisAlignedBoundingBox(const Vector3f& vMin, const Vector3f& vMax) : BoundingVolume(BoundingVolume::VT_AABB), _startID(0), _planeMask(CM_ALL)
		{
			_vMinMax[Ver_Min] = vMin;   _vMinMax[Ver_Max] = vMax;
		}
		AxisAlignedBoundingBox() : BoundingVolume(BoundingVolume::VT_AABB), _startID(0), _planeMask(CM_ALL)
		{
			Empty();
		};
		~AxisAlignedBoundingBox() {};

	public:

		void     SetPlaneMask(ClipMask mask) { _planeMask = mask; }
		ClipMask GetPlaneMask() { return _planeMask; }

		virtual void Add( BoundingVolume* pBV )
		{
			switch( pBV->GetVolumeType() )
			{
			case BoundingVolume::VT_AABB:
				{
					AxisAlignedBoundingBox* pAABB = static_cast<AxisAlignedBoundingBox*>(pBV);
					if( pAABB )
					{
						Add( pAABB->_vMinMax[0] );
						Add( pAABB->_vMinMax[1] );
					}
				}
				break;
			};
		}

	public:
		//---------------------------------------------------------------------------
		void SetValue( const Vector3f& vMin, const Vector3f& vMax )
		{
			_vMinMax[Ver_Min] = vMin;   _vMinMax[Ver_Max] = vMax;
		}

		//---------------------------------------------------------------------------
		Vector3f Size() const
		{
			return _vMinMax[Ver_Max] - _vMinMax[Ver_Min];
		}

		//---------------------------------------------------------------------------
		float XSize() const
		{
			return _vMinMax[Ver_Max].x - _vMinMax[Ver_Min].x;
		}

		//---------------------------------------------------------------------------
		float YSize() const
		{
			return _vMinMax[Ver_Max].y - _vMinMax[Ver_Min].y;
		}

		//---------------------------------------------------------------------------
		float ZSize() const
		{
			return _vMinMax[Ver_Max].z - _vMinMax[Ver_Min].z;
		}

		//---------------------------------------------------------------------------
		Vector3f Center() const
		{
			return ( _vMinMax[Ver_Min] + _vMinMax[Ver_Max] ) * 0.5f;
		}

		//---------------------------------------------------------------------------
		Vector3f Corner( int i ) const
		{
			// Return one of the eight corner points. The points are numbered as follows:
			//
			//            6/---------------------|  7
			//            /|                    /|
			//           / |                   / |
			//          /  |                  /  |
			//         /   |                 /   |
			//        /    |                /    |
			//       /     |               /     |
			//     2|------|--------------| 3    |
			//      |      |              |      |
			//      |      |              |      |
			//      |      |              |      |
			//      |    4 /--------------|------| 5
			//      |     /               |     /
			//      |    /                |    / 
			//      |   /                 |   /
			//      |  /                  |  /
			//      | /                   | /
			//     0|---------------------| 1

			// 有效的索引
			assert( i >= 0 && i <= 7 );

			// Return it
			return Vector3f(
				(i & 1) ? _vMinMax[Ver_Max].x : _vMinMax[Ver_Min].x,
				(i & 2) ? _vMinMax[Ver_Max].y : _vMinMax[Ver_Min].y,
				(i & 4) ? _vMinMax[Ver_Max].z : _vMinMax[Ver_Min].z
				);
		}


		//---------------------------------------------------------------------------
		// 初始化操作
		virtual void Empty() 
		{
			const float kBigNumber = 1e37f;
			_vMinMax[Ver_Min].x = _vMinMax[Ver_Min].y = _vMinMax[Ver_Min].z = kBigNumber;
			_vMinMax[Ver_Max].x = _vMinMax[Ver_Max].y = _vMinMax[Ver_Max].z = -kBigNumber;
		}

		//---------------------------------------------------------------------------
		// 把一个点加入AABB，扩大AABB
		void Add(const Vector3f &p) 
		{
			if (p.x < _vMinMax[Ver_Min].x) _vMinMax[Ver_Min].x = p.x;
			if (p.x > _vMinMax[Ver_Max].x) _vMinMax[Ver_Max].x = p.x;
			if (p.y < _vMinMax[Ver_Min].y) _vMinMax[Ver_Min].y = p.y;
			if (p.y > _vMinMax[Ver_Max].y) _vMinMax[Ver_Max].y = p.y;
			if (p.z < _vMinMax[Ver_Min].z) _vMinMax[Ver_Min].z = p.z;
			if (p.z > _vMinMax[Ver_Max].z) _vMinMax[Ver_Max].z = p.z;
		}

		/* 
		// Sample
		// 从一系列点构造AABB的方法
			
		const int n;
		Vector3f list[n];

		// First, empty the box
		AABB3 box;
		box.empty();

		// Add each point into the box			
		for (int i = 0 ; i < n ; ++i) 
		{
			box.Add(list[i]);
		}
		*/
			

		//---------------------------------------------------------------------------
		// IsEmpty
		//
		bool IsEmpty() const 
		{
			// Check if we're inverted on any axis
			return (_vMinMax[Ver_Min].x > _vMinMax[Ver_Max].x) || (_vMinMax[Ver_Min].y > _vMinMax[Ver_Max].y) || (_vMinMax[Ver_Min].z > _vMinMax[Ver_Max].z);
		}

		//---------------------------------------------------------------------------
		// Contains--
		// Return true if the box contains a point
		// 
		bool Contains(const Vector3f &p) const 
		{
			return
				(p.x >= _vMinMax[Ver_Min].x) && (p.x <= _vMinMax[Ver_Max].x) &&
				(p.y >= _vMinMax[Ver_Min].y) && (p.y <= _vMinMax[Ver_Max].y) &&
				(p.z >= _vMinMax[Ver_Min].z) && (p.z <= _vMinMax[Ver_Max].z);
		}

		//---------------------------------------------------------------------------
		// OutPoint--
		// Render the outPoint 
		//
		Vector3f OutPoint(RayEx& rayex) const
		{
			Vector3f t_outpoint;

			// L
			float t_b = 0.0f;
			if (rayex._vDir.Dot(rayex.m_axis[Laxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Laxis]);
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Laxis]);

			float t_u = rayex.m_intercept[Laxis][Saxis] + t_b * rayex.m_increment[Laxis][Saxis];
			float t_v = rayex.m_intercept[Laxis][Taxis] + t_b * rayex.m_increment[Laxis][Taxis];

			if ((_vMinMax[Ver_Min].Dot(rayex.m_axis[Saxis])<=t_u)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Saxis])>=t_u)&&
					(_vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis])<=t_v)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis])>=t_v))
			{
				t_outpoint		=	rayex.m_axis[Laxis] * t_b;
				t_outpoint		+=rayex.m_axis[Saxis] * t_u;
				t_outpoint		+=rayex.m_axis[Taxis] * t_v;
					
				Vector3f t_dir = t_outpoint - rayex._vOrign;
				t_dir.Normalize();

				return t_outpoint;
			}

			if (rayex._vDir.Dot(rayex.m_axis[Saxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Saxis]);
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Saxis]);

			t_u = rayex.m_intercept[Saxis][Laxis] + t_b * rayex.m_increment[Saxis][Laxis];
			t_v = rayex.m_intercept[Saxis][Taxis] + t_b * rayex.m_increment[Saxis][Taxis];

			if ((_vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis])<=t_v)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis])>=t_v))
			{
				t_outpoint		=	rayex.m_axis[Laxis] * t_u;
				t_outpoint		+=rayex.m_axis[Saxis] * t_b;
				t_outpoint		+=rayex.m_axis[Taxis] * t_v;

				Vector3f t_dir = t_outpoint - rayex._vOrign;
				t_dir.Normalize();

				return t_outpoint;
			}

			if (rayex._vDir.Dot(rayex.m_axis[Taxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis]);
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis]);

			t_u = rayex.m_intercept[Taxis][Laxis] + t_b * rayex.m_increment[Taxis][Laxis];
			t_v = rayex.m_intercept[Taxis][Saxis] + t_b * rayex.m_increment[Taxis][Saxis];

			t_outpoint		=	rayex.m_axis[Laxis] * t_u;
			t_outpoint		+=rayex.m_axis[Saxis] * t_v;
			t_outpoint		+=rayex.m_axis[Taxis] * t_b;

			Vector3f t_dir = t_outpoint - rayex._vOrign;
			t_dir.Normalize();

			return t_outpoint;
		}
			
		Vector3f OutPoint(RayEx& rayex,float t_offset) const
		{
			Vector3f t_outpoint;

			// L
			float t_b = 0.0f;
			if (rayex._vDir.Dot(rayex.m_axis[Laxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Laxis]) + t_offset;
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Laxis]) - t_offset;

			float t_u = rayex.m_intercept[Laxis][Saxis] + t_b * rayex.m_increment[Laxis][Saxis];
			float t_v = rayex.m_intercept[Laxis][Taxis] + t_b * rayex.m_increment[Laxis][Taxis];

			if ((_vMinMax[Ver_Min].Dot(rayex.m_axis[Saxis])<=t_u)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Saxis])>=t_u)&&
					(_vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis])<=t_v)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis])>=t_v))
			{
				t_outpoint		=	rayex.m_axis[Laxis] * t_b;
				t_outpoint		+=rayex.m_axis[Saxis] * t_u;
				t_outpoint		+=rayex.m_axis[Taxis] * t_v;
					
				Vector3f t_dir = t_outpoint - rayex._vOrign;
				t_dir.Normalize();

				return t_outpoint;
			}

			if (rayex._vDir.Dot(rayex.m_axis[Saxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Saxis]) + t_offset;
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Saxis]) - t_offset;

			t_u = rayex.m_intercept[Saxis][Laxis] + t_b * rayex.m_increment[Saxis][Laxis];
			t_v = rayex.m_intercept[Saxis][Taxis] + t_b * rayex.m_increment[Saxis][Taxis];

			if ((_vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis])<=t_v)&&(_vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis])>=t_v))
			{
				t_outpoint		=	rayex.m_axis[Laxis] * t_u;
				t_outpoint		+=rayex.m_axis[Saxis] * t_b;
				t_outpoint		+=rayex.m_axis[Taxis] * t_v;

				Vector3f t_dir = t_outpoint - rayex._vOrign;
				t_dir.Normalize();

				return t_outpoint;
			}

			if (rayex._vDir.Dot(rayex.m_axis[Taxis])>0.0f)
				t_b = _vMinMax[Ver_Max].Dot(rayex.m_axis[Taxis]) + t_offset;
			else
				t_b = _vMinMax[Ver_Min].Dot(rayex.m_axis[Taxis]) - t_offset;

			t_u = rayex.m_intercept[Taxis][Laxis] + t_b * rayex.m_increment[Taxis][Laxis];
			t_v = rayex.m_intercept[Taxis][Saxis] + t_b * rayex.m_increment[Taxis][Saxis];

			t_outpoint		=	rayex.m_axis[Laxis] * t_u;
			t_outpoint		+=rayex.m_axis[Saxis] * t_v;
			t_outpoint		+=rayex.m_axis[Taxis] * t_b;

			Vector3f t_dir = t_outpoint - rayex._vOrign;
			t_dir.Normalize();

			return t_outpoint;
		}
		////---------------------------------------------------------------------------
		//// SetToTransformedBox
		////
		//// Transform the box and compute the new AABB. Remember, this always
		//// results in an AABB that is at least as big as the origin, and it may be
		//// considerably bigger. 矩阵m的m._30, m._31, m._32是平移
		////
		//void SetToTransformedBox(const BoundingAABB &box, const Matrix4 &m )
		//{
		//	if (box.IsEmpty()) 
		//	{
		//		empty();
		//		return;
		//	}
		//	// Start with the translation portion
		//	_vMin = _vMax = Vector3f( m._30, m._31, m._32 );//getTranslation(m);
		//	// Examine each of the nine matrix elements
		//	// and compute the new AABB
		//	if (m._00 > 0.0f) 
		//	{
		//		_vMin.x += m._00 * box._vMin.x; 
		//		_vMax.x += m._00 * box._vMax.x;
		//	} 
		//	else 
		//	{
		//		_vMin.x += m._00 * box._vMax.x;
		//		_vMax.x += m._00 * box._vMin.x;
		//	}

		//	if (m._01 > 0.0f) 
		//	{
		//		_vMin.y += m._01 * box._vMin.x; 
		//		_vMax.y += m._01 * box._vMax.x;
		//	} 
		//	else 
		//	{
		//		_vMin.y += m._01 * box._vMax.x; 
		//		_vMax.y += m._01 * box._vMin.x;
		//	}

		//	if (m._02 > 0.0f) 
		//	{
		//		_vMin.z += m._02 * box._vMin.x; 
		//		_vMax.z += m._02 * box._vMax.x;
		//	} 
		//	else 
		//	{
		//		_vMin.z += m._02 * box._vMax.x; 
		//		_vMax.z += m._02 * box._vMin.x;
		//	}

		//	if (m._10 > 0.0f) 
		//	{
		//		_vMin.x += m.m21 * box._vMin.y; 
		//		_vMax.x += m.m21 * box._vMax.y;
		//	} 
		//	else 
		//	{
		//		_vMin.x += m.m21 * box._vMax.y; 
		//		_vMax.x += m.m21 * box._vMin.y;
		//	}

		//	if (m._11 > 0.0f) 
		//	{
		//		_vMin.y += m._11 * box._vMin.y; 
		//		_vMax.y += m._11 * box._vMax.y;
		//	} 
		//	else 
		//	{
		//		_vMin.y += m._11 * box._vMax.y; 
		//		_vMax.y += m._11 * box._vMin.y;
		//	}

		//	if (m._12 > 0.0f) 
		//	{
		//		_vMin.z += m._12 * box._vMin.y; 
		//		_vMax.z += m._12 * box._vMax.y;
		//	} 
		//	else 
		//	{
		//		_vMin.z += m._12 * box._vMax.y; 
		//		_vMax.z += m._12 * box._vMin.y;
		//	}

		//	if (m._20 > 0.0f) 
		//	{
		//		_vMin.x += m._20 * box._vMin.z; 
		//		_vMax.x += m._20 * box._vMax.z;
		//	} 
		//	else 
		//	{
		//		_vMin.x += m._20 * box._vMax.z; 
		//		_vMax.x += m._20 * box._vMin.z;
		//	}

		//	if (m._21 > 0.0f)
		//	{
		//		_vMin.y += m._21 * box._vMin.z; 
		//		_vMax.y += m._21 * box._vMax.z;
		//	} 
		//	else 
		//	{
		//		_vMin.y += m._21 * box._vMax.z;
		//		_vMax.y += m._21 * box._vMin.z;
		//	}

		//	if (m._22 > 0.0f) 
		//	{
		//		_vMin.z += m._22 * box._vMin.z; 
		//		_vMax.z += m._22 * box._vMax.z;
		//	} 
		//	else 
		//	{
		//		_vMin.z += m._22 * box._vMax.z;
		//		_vMax.z += m._22 * box._vMin.z;
		//	}				
		//}

		////---------------------------------------------------------------------------
		//// ClosestPointTo
		//// Return the closest point on this box to another point
		//// 
		//Vector3f ClosestPointTo(const Vector3f &p) const 
		//{
		//	// "Push" p into the box on each dimension
		//	Vector3f r;
		//	if (p.x < _vMin.x)
		//	{
		//		r.x = _vMin.x;
		//	} 
		//	else if (p.x > _vMax.x) 
		//	{
		//		r.x = _vMax.x;
		//	} 
		//	else 
		//	{
		//		r.x = p.x;
		//	}
		//	if (p.y < _vMin.y) 
		//	{
		//		r.y = _vMin.y;
		//	} 
		//	else if (p.y > _vMax.y)
		//	{
		//		r.y = _vMax.y;
		//	} 
		//	else 
		//	{
		//		r.y = p.y;
		//	}

		//	if (p.z < _vMin.z) 
		//	{
		//		r.z = _vMin.z;
		//	} 
		//	else if (p.z > _vMax.z) 
		//	{
		//		r.z = _vMax.z;
		//	} 
		//	else 
		//	{
		//		r.z = p.z;
		//	}

		//	return r;
		//}

		////---------------------------------------------------------------------------
		//// RayIntersect
		////
		//// Parametric intersection with a ray. Returns parametric point
		//// of intsersection in range 0...1 or a really big number (>1) if no
		//// intersection.
		////
		//float RayIntersect( const Vector3f& rayOrg, const Vector3f& rayDelta, // length and direction of the ray
		//	Vector3f *returnNormal    // optionally, the normal is returned
		//	) const 
		//{
		//	// We'll return this huge number if no intersection
		//	const float kNoIntersection = 1e30f;

		//	// Check for point inside box, trivial reject, and determine parametric
		//	// distance to each front face
		//	bool inside = true;
		//	float xt, xn;
		//	if (rayOrg.x < _vMin.x) 
		//	{
		//		xt = _vMin.x – rayOrg.x;
		//		if (xt > rayDelta.x) return kNoIntersection;
		//		xt /= rayDelta.x;
		//		inside = false;
		//		xn = –1.0f;
		//	} 
		//	else if (rayOrg.x > _vMax.x)
		//	{
		//		xt = _vMax.x – rayOrg.x;
		//		if (xt < rayDelta.x) return kNoIntersection;
		//		xt /= rayDelta.x;
		//		inside = false;
		//		xn = 1.0f;
		//	} 
		//	else
		//	{
		//		xt = –1.0f;
		//	}

		//	float yt, yn;
		//	if (rayOrg.y < _vMin.y) 
		//	{
		//		yt = _vMin.y – rayOrg.y;
		//		if (yt > rayDelta.y) return kNoIntersection;
		//		yt /= rayDelta.y;
		//		inside = false;
		//		yn = –1.0f;
		//	} 
		//	else if (rayOrg.y > _vMax.y) 
		//	{
		//		yt = _vMax.y – rayOrg.y;
		//		if (yt < rayDelta.y) return kNoIntersection;
		//		yt /= rayDelta.y;
		//		inside = false;
		//		yn = 1.0f;
		//	} 
		//	else
		//	{
		//		yt = –1.0f;
		//	}
		//	float zt, zn;
		//	if (rayOrg.z < _vMin.z) {
		//		zt = _vMin.z – rayOrg.z;
		//		if (zt > rayDelta.z) return kNoIntersection;
		//		zt /= rayDelta.z;
		//		inside = false;
		//		zn = –1.0f;
		//	} else if (rayOrg.z > _vMax.z) {
		//		zt = _vMax.z – rayOrg.z;
		//		if (zt < rayDelta.z) return kNoIntersection;
		//		zt /= rayDelta.z;
		//		inside = false;
		//		zn = 1.0f;
		//	} else {
		//		zt = –1.0f;
		//	}

		//	// Inside box?
		//	if (inside) 
		//	{
		//		if (returnNormal != NULL) 
		//		{
		//			*returnNormal = –rayDelta;
		//			returnNormal->Normalize();
		//		}
		//		return 0.0f;
		//	}
		//	// Select farthest plane - this is
		//	// the plane of intersection.
		//	int which = 0;
		//	float t = xt;
		//	if (yt > t) 
		//	{
		//		which = 1;
		//		t = yt;
		//	}
		//	if (zt > t) 
		//	{
		//		which = 2;
		//		t = zt;
		//	}
		//	switch (which) 
		//	{
		//	case 0: // intersect with yz plane
		//		{
		//			float y = rayOrg.y + rayDelta.y*t;
		//			if (y < _vMin.y || y > _vMax.y) return kNoIntersection;
		//			float z = rayOrg.z + rayDelta.z*t;
		//			if (z < _vMin.z || z > _vMax.z) return kNoIntersection;
		//			if (returnNormal != NULL) {
		//				returnNormal–>x = xn;
		//				returnNormal–>y = 0.0f;
		//				returnNormal–>z = 0.0f;
		//			}
		//		} break;
		//	case 1: // intersect with xz plane
		//		{
		//			float x = rayOrg.x + rayDelta.x*t;
		//			if (x < _vMin.x || x > _vMax.x) return kNoIntersection;
		//			float z = rayOrg.z + rayDelta.z*t;
		//			if (z < _vMin.z || z > _vMax.z) return kNoIntersection;
		//			if (returnNormal != NULL) {
		//				returnNormal–>x = 0.0f;
		//				returnNormal–>y = yn;
		//				returnNormal–>z = 0.0f;
		//			}
		//		}
		//		break;
		//	case 2: // intersect with xy plane
		//		{
		//			float x = rayOrg.x + rayDelta.x*t;
		//			if (x < _vMin.x || x > _vMax.x) return kNoIntersection;
		//			float y = rayOrg.y + rayDelta.y*t;
		//			if (y < _vMin.y || y > _vMax.y) return kNoIntersection;
		//			if (returnNormal != NULL) {
		//				returnNormal–>x = 0.0f;
		//				returnNormal–>y = 0.0f;
		//				returnNormal–>z = zn;
		//			}
		//		} break;
		//	}
		//	return t;
		//}
		//			


	};
}
using namespace Math;