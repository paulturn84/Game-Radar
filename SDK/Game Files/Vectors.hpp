#pragma once
#include <math.h>

class Vector3
{
public:
	float x, y, z;

	Vector3(float X = 0.f, float Y = 0.f, float Z = 0.f)
	{
		x = X;
		y = Y;
		z = Z;
	}

	inline float Dot(Vector3 v)
	{
		return x * v.x + y * v.y + z * v.z;
	}

	inline float Distance(Vector3 v)
	{
		return sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0) + powf(v.z - z, 2.0));
	}

	inline float Length()
	{
		return sqrt(x * x + y * y + z * z);
	}

	Vector3 Normalized()
	{
		return this->Divide(Length());
	}

	inline void Scale(float Value)
	{
		x *= Value;
		y *= Value;
		z *= Value;
	}

	Vector3 operator+(Vector3 v)
	{
		return Vector3(x + v.x, y + v.y, z + v.z);
	}

	Vector3 operator-(Vector3 v)
	{
		return Vector3(x - v.x, y - v.y, z - v.z);
	}

	Vector3 Divide(float v)
	{
		return Vector3(x / v, y / v, z / v);
	}

	Vector3 Inverse()
	{
		return {-x, -y, -z};
	}

	bool operator==(Vector3 v)
	{
		return (x == v.x && y == v.y && z == v.z);
	}

	bool operator==(int v)
	{
		return (x == v || y == v || z == v);
	}

	Vector3 operator*(float number)
		const {
		return Vector3(x * number, y * number, z * number);
	}

	void Clamp()
	{
		if (x > 180.f)
			x -= 360.f;

		else if (x < -180.f)
			x += 360.f;

		if (z > 180.f)
			z -= 360.f;
	}
};

class Vector2
{
public:
	float x, y;

	Vector2(float X = 0.f, float Y = 0.f)
	{
		x = X;
		y = Y;
	}

	inline float Distance(Vector2 v)
	{
		return sqrtf(powf(v.x - x, 2.0) + powf(v.y - y, 2.0));
	}
};

class Vector3P
{
public:
	union
	{
		float _e[3];

		struct { float x, y, z; };
	};

	__forceinline float operator [] (int i) const { return _e[i]; }
	__forceinline float& operator [] (int i) { return _e[i]; }
};

class Matrix3P
{
public:
	union
	{
		struct
		{
			Vector3P _aside;
			Vector3P _up;
			Vector3P _dir;
		};
	};

	__forceinline float Get(int i, int j) const { return (&_aside)[j][i]; }
	__forceinline float& Set(int i, int j) { return (&_aside)[j][i]; }
};

class Matrix4P
{
public:
	union
	{
		struct
		{
			Vector3P _aside;
			Vector3P _up;
			Vector3P _dir;
			Vector3 _position;
		};
	};

	__forceinline float Get(int i, int j) const { return (&_aside)[j][i]; }
};

class ViewMatrix
{
public:
	Vector3 ViewRight;
	Vector3 ViewUp;
	Vector3 ViewForward;
	Vector3 ViewTranslation;
	Vector3 ViewPortMatrix;
	Vector3 ViewProjection1;
	Vector3 ViewProjection2;
};
