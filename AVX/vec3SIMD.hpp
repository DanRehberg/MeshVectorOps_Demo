/*
Author: Dan Rehberg
Date: June 6, 2020
Last Modified: April 7, 2024
Purpose: Originally used for CPU vs CUDA testing over
	GJK executions for three different meshes of 
	differing complexity (Cube, Suzanne Convex Hull, 
	UV Sphere).*/

#ifndef __VEC3_HPP__
#define __VEC3_HPP__

#include <cmath>
#include <ostream>
#include <immintrin.h>

class vec3
{
public:
	vec3() : x(0.0f), y(0.0f), z(0.0f)
	{
		//NULL
	}
	vec3(const float x_, const float y_, const float z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
	vec3(const float x_)
	{
		x = x_;
		y = x;
		z = x;
	}
	~vec3()
	{

	}
	vec3(const vec3& cp)
	{
		this->x = cp.x;
		this->y = cp.y;
		this->z = cp.z;
	}
	vec3& operator=(const vec3& cp)
	{
		this->x = cp.x;
		this->y = cp.y;
		this->z = cp.z;
		return *this;
	}
	//Binary operators
	friend vec3 operator-(const vec3&, const vec3&);
	friend vec3 operator+(const vec3&, const vec3&);
	friend vec3 operator*(const float&, const vec3&);
	friend vec3 operator*(const vec3&, const float&);
	friend vec3 operator/(const float&, const vec3&);
	friend vec3 operator/(const vec3&, const float&);
	
	//Utility features
	friend std::ostream& operator<<(std::ostream& out, const vec3& v);

	//Not protected, prefer ease of mutability for demonstration
	union
	{
		struct
		{
			float xyz[4];
		};
		struct
		{
			float x, y, z, w;//, u, v, r, s;
		};
	};
};

vec3 operator-(const vec3& a, const vec3& b)
{
	return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
vec3 operator+(const vec3& a, const vec3& b)
{
	return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
vec3 operator*(const float& s, const vec3& v)
{
	return vec3(v.x * s, v.y * s, v.z * s);
}
vec3 operator*(const vec3& v, const float& s)
{
	return s * v;
}
vec3 operator/(const float& s, const vec3& v)
{
	return (1.0f / s) * v;
}
vec3 operator/(const vec3& v, const float& s)
{
	return (1.0f / s) * v;
}

std::ostream& operator<<(std::ostream& out, const vec3& v)
{
	return out << "x: " << v.x << " y: " << v.y << " z: " << v.z << '\n';
}

vec3 cross(const vec3& a, const vec3& b)
{
	//x = yz-zy
	//y = zx-xz
	//z = xy-yx
	vec3 temp;
	temp.x = (a.y * b.z) - (a.z * b.y);
	temp.y = (a.z * b.x) - (a.x * b.z);
	temp.z = (a.x * b.y) - (a.y * b.x);
	return temp;
}

float dot(const vec3& a, __m256& b)
{
	float c[3];
	float result = 0.0f;
	//return a.x * b.x + a.y * b.y + a.z * b.z;
	/*#pragma omp simd
	for (int i = 0; i < 3; ++i)
	{
		c.xyz[i] = a.xyz[i] * b.xyz[i];
	}
	//#pragma omp simd reduction(+:result)
	//for (int i = 0; i < 3; ++i)
	//{
	//	result += c.xyz[i];
	//}
	return c.x + c.y + c.z;//result;*/

	__m256 A = _mm256_loadu_ps(&a.xyz[0]);

	__m256 multResult = _mm256_mul_ps(A, b);

	_mm256_storeu_ps(&c[0], multResult);

	return c[0] + c[1] + c[2];
}
float dot(const vec3& a, const vec3& b)
{
	vec3 c;
	float result = 0.0f;
	//return a.x * b.x + a.y * b.y + a.z * b.z;
	/*#pragma omp simd
	for (int i = 0; i < 3; ++i)
	{
		c.xyz[i] = a.xyz[i] * b.xyz[i];
	}
	//#pragma omp simd reduction(+:result)
	//for (int i = 0; i < 3; ++i)
	//{
	//	result += c.xyz[i];
	//}
	return c.x + c.y + c.z;//result;*/

	__m256 A = _mm256_loadu_ps(&a.xyz[0]);
	__m256 B = _mm256_loadu_ps(&b.xyz[0]);

	__m256 multResult = _mm256_mul_ps(A, B);

	_mm256_storeu_ps(&c.xyz[0], multResult);

	return c.x + c.y + c.z;
}

vec3 barycentricCoordinates(const vec3& S, const vec3& A, const vec3& B, const vec3& C)
{//triangle case
	vec3 AB = B - A, AC = C - A, AS = S - A;
	float abAB = dot(AB, AB), abAC = dot(AB, AC), acAC = dot(AC, AC), abAS = dot(AB, AS), acAS = dot(AC, AS);
	float dividor = 1.0f / ((abAB * acAC) - (abAC * abAC));
	vec3 temp;
	temp.y = ((acAC * abAS) - (abAC * acAS)) * dividor;
	temp.z = ((abAB * acAS) - (abAC * abAS)) * dividor;
	temp.x = 1.0f - temp.y - temp.z;
	return temp;
}

vec3 closestPoint(const vec3& S, const vec3& A, const vec3& B)
{//find closest point on AB to S
	vec3 AB = B - A, AS = S - A;
	float distance = dot(AS, AB) / dot(AB, AB);//should be able to find barycentric coordinates using this
	if (distance <= 0)
	{
		return A;
	}
	else if (distance > 1)
	{
		return B;
	}
	return A + (AB * distance);
}

vec3 closestPoint(const vec3& S, const vec3& A, const vec3& B, const vec3& C)
{//find closest point on ABC to S
	vec3 AB = B - A, BC = C - B, CA = A - C, AS = S - A, BS = S - B, CS = S - C;
	//vec3 abNorm = normalize(AB), bcNorm = normalize(BC), caNorm = normalize(CA);
	//trying with magnitude division only first
	float uAB = (dot(AS, AB)) / dot(AB, AB), vAB = 0.0f, uBC = (dot(BS, BC)) / dot(BC, BC), vBC = 0.0f,
		uCA = (dot(CS, CA)) / dot(CA, CA), vCA = 0.0f;
	vAB = 1.0f - uAB;
	if (uAB < 0.0f)
	{
		uAB = 1.0f;//possibly switched
		vAB = 0.0f;
	}
	else if (uAB > 1.0f)
	{
		uAB = 0.0f;
		vAB = 1.0f;
	}
	vBC = 1.0f - uBC;
	if (uBC < 0.0f)
	{
		uBC = 1.0f;
		vBC = 0.0f;
	}
	else if (uBC > 1.0f)
	{
		uBC = 0.0f;
		vBC = 1.0f;
	}
	vCA = 1.0f - uCA;
	if (uCA < 0.0f)
	{
		uCA = 1.0f;
		vCA = 0.0f;
	}
	else if (uCA > 1.0f)
	{
		uCA = 0.0f;
		vCA = 1.0f;
	}
	//test vertices
	if (vAB <= 0.0f && uCA <= 0.0f)
	{
		//A
		return A;
	}
	if (vBC <= 0.0f && uAB <= 0.0f)
	{
		//B
		return B;
	}
	if (vCA <= 0.0f && uBC <= 0.0f)
	{
		//C
		return C;
	}
	//test edges
	vec3 baryCoords = barycentricCoordinates(S, A, B, C);
	if (uAB > 0.0f && vAB > 0.0f && baryCoords.z <= 0.0f)
	{
		//AB
		return closestPoint(S, A, B);
	}
	if (uBC > 0.0f && vBC > 0.0f && baryCoords.x <= 0.0f)
	{
		//BC
		return closestPoint(S, B, C);
	}
	if (uCA > 0.0f && vCA > 0.0f && baryCoords.y <= 0.0f)
	{
		//CA
		return closestPoint(S, A, C);
	}
	//else all -- inside triangle
	return (baryCoords.x * A) + (baryCoords.y * B) + (baryCoords.z * C);
}

vec3 normalize(const vec3& v)
{
	return v / std::sqrt(dot(v, v));
}

#endif

