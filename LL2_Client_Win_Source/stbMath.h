#pragma once
#include <cmath>

namespace stb::math
{
	#define PI 3.141592

	// 라디안 -> 각도
	static float ConvertDegree(float radian) { return (radian * (180.0f / PI)); }

	struct Vector2
	{
		//임의의 점을 중심으로 각도만큼 회전시키는 함수
		static Vector2 Rotate(Vector2 vector, float degree)
		{
			// 각도 -> 라디안
			float radian = (degree / 180.0f) * PI;

			vector.normalize();

			float x = cosf(radian) * vector.x - sinf(radian) * vector.y;
			float y = sinf(radian) * vector.x + sinf(radian) * vector.y;

			return Vector2(x, y);

		}

		static float Dot(Vector2& v1, Vector2& v2)
		{
			return v1.x * v2.x + v1.y * v2.y;
		}


		static float Cross(Vector2& v1, Vector2& v2)
		{
			return v1.x * v2.y - v1.y * v2.x;
		}

		float x;
		float y;

		Vector2()
			: x(0.0f)
			, y(0.0f)
		{

		}

		Vector2(float _x, float _y)
			: x(_x)
			, y(_y)
		{
			
		}

		static Vector2 Zero;
		static Vector2 One;
		static Vector2 Up;
		static Vector2 Down;
		static Vector2 Right;
		static Vector2 Left;


		Vector2 operator+(Vector2 other)
		{
			return Vector2(x + other.x, y + other.y);
		}

		Vector2 operator-(Vector2 other)
		{
			return Vector2(x - other.x, y - other.y);
		}

		Vector2 operator*(Vector2 other)
		{
			return Vector2(x * other.x, y * other.y);
		}

		Vector2 operator*(float value)
		{
			return Vector2(x * value, y * value);
		}

		Vector2 operator/(Vector2 other)
		{
			return Vector2(x * other.x, y * other.y);
		}

		Vector2 operator/(float value)
		{
			return Vector2(x / value, y / value);
		}

		Vector2 operator+=(Vector2 other)
		{
			return Vector2(x += other.x, y += other.y);
		}

		Vector2 operator-=(Vector2 other)
		{
			return Vector2(x -= other.x, y -= other.y);
		}

		// 벡터의 크기를 구하는 함수
		float length()
		{
			return sqrtf(x * x + y * y);
		}

		// 벡터의 정규화 
		Vector2 normalize()
		{
			float len = length();
			x = x / len;
			y = y / len;
			return *this;
		}
	
		
		// 이후에 필요한 연산 밑에 구현 예정





	};
}

