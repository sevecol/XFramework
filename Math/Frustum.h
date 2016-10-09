
#pragma once
#include "Plane.h"
#include "Matrix4.h"

#define SAFE_DELETE1(p)              { if(p) { delete (p);       (p)=NULL; } }
#define SAFE_FREE1(p)                { if(p) { free(p);          (p)=NULL; } }
#define SAFE_DELGRP1(p)              { if(p) { delete[] (p);     (p)=NULL; } }
#define SAFE_RELEASE1(p)             { if(p) { (p)->Release();   (p)=NULL; } }

namespace Math
{
	class Frustum
	{
	public:
		enum PlaneFlag
		{
			Plane_Left,
			Plane_Right,
			Plane_Top,
			Plane_Bottom,
			Plane_Front,
			Plane_Back,
			Plane_Count
		};
	
		Plane**  _planes;//[Plane_Count];

	protected:
		virtual void InitData( const Matrix4& comboMatrix, bool bNeedNormalize = false )
		{
			for( int i = 0; i < 6; i++ )
				SAFE_DELETE1(_planes[i]);
			SAFE_DELGRP1( _planes );
			_planes = new Plane* [Plane_Count];
			for( int i = 0; i < 6; i++ )
				_planes[i] = new Plane;

			SetData( comboMatrix, bNeedNormalize );
		}

	public:

		Frustum() : _planes( 0 ) {};
		virtual ~Frustum() 
		{
			SAFE_DELGRP1( _planes );
		}

		//Frustum()
		//{
		//	//InitData( comboMatrix, bNeedNormalize );
		//}

	protected:
		void SetData( const Plane* pPlane, bool bNeedNormalize = false )
		{
			// Left|Right|Top|Bottom|Near|Far clipping Plane
			for (int i = 0; i < 6; ++i)
				*_planes[i] = pPlane[i];

			// Normalize the Plane equations, if requested
			if (bNeedNormalize == true)
			{
				for( int i = 0; i < Plane_Count; i++ )
					(*_planes[i]).Normalize();
			}
		}

		void SetData( const Matrix4& comboMatrix, bool bNeedNormalize = false )
		{
			// Left clipping Plane
			(*_planes[0])._vNormal.x = comboMatrix._03 + comboMatrix._00;
			(*_planes[0])._vNormal.y = comboMatrix._13 + comboMatrix._10;
			(*_planes[0])._vNormal.z = comboMatrix._23 + comboMatrix._20;
			(*_planes[0])._d = comboMatrix._33 + comboMatrix._30;

			// Right clipping Plane
			(*_planes[1])._vNormal.x = comboMatrix._03 - comboMatrix._00;
			(*_planes[1])._vNormal.y = comboMatrix._13 - comboMatrix._10;
			(*_planes[1])._vNormal.z = comboMatrix._23 - comboMatrix._20;
			(*_planes[1])._d = comboMatrix._33 - comboMatrix._30;

			// Top clipping Plane
			(*_planes[2])._vNormal.x = comboMatrix._03 - comboMatrix._01;
			(*_planes[2])._vNormal.y = comboMatrix._13 - comboMatrix._11;
			(*_planes[2])._vNormal.z = comboMatrix._23 - comboMatrix._21;
			(*_planes[2])._d = comboMatrix._33 - comboMatrix._31;

			// Bottom clipping Plane
			(*_planes[3])._vNormal.x = comboMatrix._03 + comboMatrix._01;
			(*_planes[3])._vNormal.y = comboMatrix._13 + comboMatrix._11;
			(*_planes[3])._vNormal.z = comboMatrix._23 + comboMatrix._21;
			(*_planes[3])._d = comboMatrix._33 + comboMatrix._31;

			// Near clipping Plane
			(*_planes[4])._vNormal.x = comboMatrix._02;
			(*_planes[4])._vNormal.y = comboMatrix._12;
			(*_planes[4])._vNormal.z = comboMatrix._22;
			(*_planes[4])._d = comboMatrix._32;

			//(*_planes[4])._vNormal.x = comboMatrix._02 + comboMatrix._03;
			//(*_planes[4])._vNormal.y = comboMatrix._12 + comboMatrix._13;
			//(*_planes[4])._vNormal.z = comboMatrix._22 + comboMatrix._23;
			//(*_planes[4])._d = comboMatrix._32 + comboMatrix._33;

			// Far clipping Plane
			(*_planes[5])._vNormal.x = comboMatrix._03 - comboMatrix._02;
			(*_planes[5])._vNormal.y = comboMatrix._13 - comboMatrix._12;
			(*_planes[5])._vNormal.z = comboMatrix._23 - comboMatrix._22;
			(*_planes[5])._d = comboMatrix._33 - comboMatrix._32;

			// Normalize the Plane equations, if requested
			if (bNeedNormalize == true)
			{
				for( int i = 0; i < Plane_Count; i++ )
					(*_planes[i]).Normalize();
			}
		}

	public:
		// 判断一个点是否在裁剪体里面
		bool IsPointInFrustum( Vector3f& vPoint ) 
		{
			bool bIn = true;
			for( int i = 0; i < Plane_Count; i++ )
			{
				Plane::PlaneDirection pd = (*_planes[i]).RelationWithPoint( vPoint );
				if( pd == Plane::Plane_Back )
				{
					return false;
				}
			}

			return bIn;
		}

	};
}
using namespace Math;