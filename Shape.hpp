/* Author: Dan Rehberg
   Date: June 6, 2020
   Last Modified: April 7, 2024
   Purpose: Originally used for CPU vs CUDA. Limiting the
			scope of code here to just CPU-based execution
			for conceptual demostration of vector ops over
			meshes. See https://danrehberg.github.io/gjkCPU_GPUTrials.htm
			for original testing and analysis.
			NOTE: This class is fixed for testing against three different 
				meshes where the bounds of this class is based on the upper
				bound memory requirements for the most complex shape: a UV
				sphere.*/

#ifndef __SHAPE_HPP__
#define __SHAPE_HPP__

#include <iostream>
#include <cstdint>
#include "vec3.hpp"

class Shape
{
public:
	Shape()
	{
		//default, mostly for CUDA to control memory allocation on Host
		vertices[0] = vec3(12.0f, 0.0f, 0.0f);
		if (edges == nullptr)
		{
			edges = new int* [242];
			for (unsigned int i = 0; i < 242; ++i)
			{
				edges[i] = new int[16];
			}
		}
		if (edgeAlt == nullptr)edgeAlt = new int[242 * 16];
		if (edgeCount == nullptr)edgeCount = new int[242];
	}
	Shape(vec3* positions, int N)
	{
		count = N;
		for (int i = 0; i < N; ++i)
		{
			vertices[i] = positions[i];
		}
		if (edges == nullptr)
		{
			edges = new int* [242];
			for (unsigned int i = 0; i < 242; ++i)
			{
				edges[i] = new int[16];
			}
		}
		if (edgeAlt == nullptr)edgeAlt = new int[242 * 16];
		if (edgeCount == nullptr)edgeCount = new int[242];
	}
	~Shape()
	{
		//if (vertices != NULL)delete[] vertices;
		if (edges != nullptr)
		{
			for (unsigned int i = 0; i < 242; ++i)
			{
				delete[] edges[i];
			}
			delete[] edges;
			std::cout << "deleting edges\n";
		}
		if (edgeAlt != nullptr)
		{
			delete[] edgeAlt;
			std::cout << "deleting edgeAlternative\n";
		}
		if (edgeCount != nullptr)
		{
			delete[] edgeCount;
			std::cout << "deleting edgeCounts\n";
		}
	}
	//Host only on the copy constructor and copy assignment
	Shape(const Shape& cp)
	{
		if (cp.vertices != NULL)
		{
			this->count = cp.count;
			//Below should not happen, just adding it
			//if (this->vertices != NULL)delete[] this->vertices;
			//this->vertices = new vec3[cp.count];
			for (int i = 0; i < cp.count; ++i)
			{
				this->vertices[i] = cp.vertices[i];
			}
		}
		if (edges == nullptr)
		{
			edges = new int* [242];
			for (unsigned int i = 0; i < 242; ++i)
			{
				edges[i] = new int[16];
			}
		}
		if (edgeAlt == nullptr)edgeAlt = new int[242 * 16];
		if (edgeCount == nullptr)edgeCount = new int[242];
	}
	Shape& operator=(const Shape& cp)
	{
		this->count = cp.count;
		for (int i = 0; i < cp.count; ++i)
		{
			this->vertices[i] = cp.vertices[i];
		}
		if (edges == nullptr)
		{
			edges = new int* [242];
			for (unsigned int i = 0; i < 242; ++i)
			{
				edges[i] = new int[16];
			}
		}
		else
			std::cout << sizeof(edges) << " edge size\n";
		if (edgeAlt == nullptr)edgeAlt = new int[242 * 16];
		if (edgeCount == nullptr)edgeCount = new int[242];
		return *this;
	}
	vec3 getVertex(const uint32_t& index) const
	{
		return vertices[index];
	}
	uint32_t supportPoint(const vec3& direction) const
	{
		float magnitude = -999999;
		uint32_t tempID = 0;
		for (int i = 0; i < count; ++i)
		{
			float dR = dot(vertices[i], direction);
			if (dR > magnitude)
			{
				magnitude = dR;
				tempID = i;
			}
		}
		return tempID;
	}
	uint32_t supportPoint(const vec3& direction, const vec3& t) const
	{
		float magnitude = -999999;
		uint32_t tempID = 0;
		for (int i = 0; i < count; ++i)
		{
			float dR = dot((vertices[i] + t), direction);
			if (dR > magnitude)
			{
				magnitude = dR;
				tempID = i;
			}
		}
		return tempID;
	}
	uint32_t supportPointHillClimb(const vec3& direction, const int& prevID)
	{
		float magnitude = -999999;
		float nMag = magnitude;
		uint32_t tempID = 0;
		uint32_t curID = prevID;
		uint32_t lastID = prevID;
		bool end = false;
		while (!end)
		{
			for (int i = 0; i < edgeCount[curID]; ++i)
			{
				float dR = dot(vertices[edges[curID][i]], direction);
				if (dR > nMag)
				{
					nMag = dR;
					tempID = edges[curID][i];
				}
			}
			if (nMag > magnitude)
			{
				lastID = curID;
				curID = tempID;
				magnitude = nMag;
			}
			else end = true;
		}
		return tempID;
	}
	//private://Removing for forced inline testing as Cuda will forcibly inline information
	vec3 vertices[242];
	int count = 0;
	int edgeTotal = 496;
	int** edges = nullptr;
	int* edgeAlt = nullptr;
	int* edgeCount = nullptr;
};

#endif