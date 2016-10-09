
#pragma once

namespace  XMath
{
	template< typename T >
	class XRectangle
	{
	public:
		XRectangle() : _left( (T)0 ), _top( (T)0 ), _width( (T)0 ), _height( (T)0 ){};
		XRectangle( T left, T top, T width, T height ) : _left(left), _top(top), _width(width), _height(height) {};
		~XRectangle(void){};

	public:
		T Left()   const  { return _left; }
		T Right()  const  { return _left + _width; }
		T Top()    const  { return _top; }
		T Bottom() const  { return _top + _height; }
		T Width()  const  { return _width; }
		T Height() const  { return _height; }

		void SetValue( T left, T top, T width, T height )
		{
			_left   = left;
			_top    = top;
			_width  = width;
			_height = height;
		}


	public:
		T _left;
		T _top;
		T _width;
		T _height;
	};

	typedef XRectangle<int> Rect;
	typedef XRectangle<float>RectF;
}
using namespace XMath;

