/*
Author: Dan Rehberg
Date: April 7, 2024
Purpose: This project is meant to assess performance gains, if any,
	when parallelizing a Support Point mapping function (general algorithm
	pertinent to Computational Geometry problems).
	The support point mapping has independent (pleasantly parallel) dot product
		operations, but we need to assess the maximum dot product result.
	Because extrema operators are associative, parallelizing a support point
		query can be done using a "reduction" process in parallel to minimize
		the "span" (dependencies) that are incurred when comparing these results.
*/

#include <iostream>
#include <chrono>
#include "vec3SIMD.hpp"
#include "ShapeSIMD4.hpp"
#include "ExampleShapesSIMD.hpp"

constexpr size_t AVERAGE = 1000000;
unsigned int searchResult = 0;


float averageSupportTime(Shape& s, vec3 dir, bool hill = false)
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	size_t avg = 0;
	float direction[8];
	direction[0] = dir.x;
	direction[1] = dir.y;
	direction[2] = dir.z;
	direction[3] = dir.x;
	direction[4] = dir.y;
	direction[5] = dir.z;
	direction[6] = dir.x;
	direction[7] = dir.y;
	__m256 B = _mm256_loadu_ps(&direction[0]);

	float dir_two[8];
	dir_two[0] = dir.z;
	dir_two[1] = dir.x;	
	dir_two[2] = dir.y;
	dir_two[3] = dir.z;
	dir_two[4] = dir.x;
	dir_two[5] = dir.y;
	dir_two[6] = dir.z;
	dir_two[7] = dir.x;
	__m256 C = _mm256_loadu_ps(&dir_two[0]);

	float dir_three[8];
	dir_three[0] = dir.y;
	dir_three[1] = dir.z;
	dir_three[2] = dir.x;
	dir_three[3] = dir.y;
	dir_three[4] = dir.z;
	dir_three[5] = dir.x;
	dir_three[6] = dir.y;
	dir_three[7] = dir.z;
	__m256 D = _mm256_loadu_ps(&dir_three[0]);

	if (hill)
	{
		for (size_t i = 0; i < AVERAGE; ++i)
		{
			start = std::chrono::steady_clock::now();
			searchResult = s.supportPointHillClimb(B, 0);
			end = std::chrono::steady_clock::now();
			avg += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	}
	else
	{	
		for (size_t i = 0; i < AVERAGE; ++i)
		{
			start = std::chrono::steady_clock::now();
			searchResult = s.supportPoint(B, C, D);
			end = std::chrono::steady_clock::now();
			avg += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	}

	return static_cast<float>(avg) / static_cast<float>(AVERAGE);
}

int main(int argc, char** argv)
{


	buildShapes();
	//Cube

	float avg = averageSupportTime(cube, vec3(1.0f, 2.0f, 1.0f), false);

	std::cout << "Average support point time for Cube: " << avg << " ns; vec: " << 
		cube.getVertex(searchResult);

	avg = averageSupportTime(cube, vec3(1.0f, 2.0f, 1.0f), true);

	std::cout << "Average support point (Hill Climb) time for Cube: " << avg << " ns; vec: " <<
		cube.getVertex(searchResult);

	//Suzanne

	avg = averageSupportTime(suzanne, vec3(1.0f, 2.0f, 1.0f), false);

	std::cout << "Average support point time for Suzanne: " << avg << " ns; vec: " <<
		suzanne.getVertex(searchResult);

	avg = averageSupportTime(suzanne, vec3(1.0f, 2.0f, 1.0f), true);

	std::cout << "Average support point (Hill Climb) time for Suzanne: " << avg << " ns; vec: " <<
		suzanne.getVertex(searchResult);

	//Sphere

	avg = averageSupportTime(sphere, vec3(1.0f, 2.0f, 1.0f), false);

	std::cout << "Average support point time for Sphere: " << avg << " ns; vec: " <<
		sphere.getVertex(searchResult);

	avg = averageSupportTime(sphere, vec3(1.0f, 2.0f, 1.0f), true);

	std::cout << "Average support point (Hill Climb) time for Sphere: " << avg << " ns; vec: " <<
		sphere.getVertex(searchResult);

	return 0;
}
