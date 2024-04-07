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
#include "vec3.hpp"
#include "Shape.hpp"
#include "ExampleShapes.hpp"

constexpr size_t AVERAGE = 100000;
unsigned int searchResult = 0;

float averageSupportTime(Shape& s, vec3 dir, bool hill = false)
{
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	size_t avg = 0;
	if (hill)
	{
		for (size_t i = 0; i < AVERAGE; ++i)
		{
			start = std::chrono::steady_clock::now();
			searchResult = s.supportPointHillClimb(dir, 0);
			end = std::chrono::steady_clock::now();
			avg += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	}
	else
	{
		for (size_t i = 0; i < AVERAGE; ++i)
		{
			start = std::chrono::steady_clock::now();
			searchResult = s.supportPoint(dir);
			end = std::chrono::steady_clock::now();
			avg += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
		}
	}

	return static_cast<float>(avg) / static_cast<float>(AVERAGE);
}

int main(int argc, char** argv)
{

	vec3 test(2.5f);
	vec3 testToo(1.0f, 2.0f, 0.0f);

	std::cout << test << (cross(test, testToo));

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
