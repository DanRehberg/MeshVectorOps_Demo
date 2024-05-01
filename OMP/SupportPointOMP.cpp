/*
Author: Dan Rehberg
Date: April 20, 2024
Purpose: Testing performance change when doing support point with a parallel for loop.
  NOTE, this requires a custom reduction for the ARG Max style associate operation.
*/

#include <iostream>
#include <chrono>
#include "vec3.hpp"
#include "ShapeOMP.hpp"
#include "ExampleShapes.hpp"

constexpr size_t AVERAGE = 1000000;
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
