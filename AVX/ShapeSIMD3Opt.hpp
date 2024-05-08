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
#include "vec3SIMD.hpp"



class Shape
{
public:
	Shape()
	{
		//default, mostly for CUDA to control memory allocation on Host
		vertices[0].xyz[0] = 12.0f;
		vertices[0].xyz[1] = 0.0f;
		vertices[0].xyz[2] = 0.0f;
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
		if (verts == nullptr)
			verts = new float[4*N];

		size_t index = 0;
		size_t prior = 0;
		for (int i = 0; i < N; ++i)
		{
			vertices[i] = positions[i];
			verts[index++] = positions[i].x;
			verts[index++] = positions[i].y;
			verts[index++] = positions[i].z;
			verts[index++] = 0.00001f;
			prior += 4;
		}
		int mTotal = (count * 4) / 8 + (((count*4) % 8 > 0) ? 1 : 0);
		//if (mverts != nullptr)delete[] mverts;
		//mverts = new __m256[mTotal];
		mCount = mTotal;
		index = 0;
		for (unsigned int i = 0; i < count*4; i += 8)
		{
			mverts[index++] = _mm256_loadu_ps(&verts[i]);
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
		if (verts != nullptr) delete[] verts;
		//if(mverts != nullptr) delete[] mverts;
		if (edges != nullptr)
		{
			for (unsigned int i = 0; i < 242; ++i)
			{
				delete[] edges[i];
			}
			delete[] edges;
			//std::cout << "deleting edges\n";
		}
		if (edgeAlt != nullptr)
		{
			delete[] edgeAlt;
			//std::cout << "deleting edgeAlternative\n";
		}
		if (edgeCount != nullptr)
		{
			delete[] edgeCount;
			//std::cout << "deleting edgeCounts\n";
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
		count = cp.count;
		if (verts != nullptr)delete[] verts;
		verts = new float[4*cp.count];
		for (unsigned int i = 0; i < 4*count; ++i)
		{
			verts[i] = cp.verts[i];
		} 
		int mTotal = (count * 3) / 8 + (((count*3) % 8 > 0) ? 1 : 0);
		//if (mverts != nullptr)delete[] mverts;
		//mverts = new __m256[mTotal];
		mCount = mTotal;
		int index = 0;
		for (unsigned int i = 0; i < count*4; i += 8)
		{
			mverts[index++] = _mm256_loadu_ps(&verts[i]);
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
		if (verts != nullptr)delete[] verts;
		verts = new float[4*cp.count];
		size_t index = 0;
		for (unsigned int i = 0; i < 4*count; ++i)
		{
			++index;
			verts[i] = cp.verts[i];
		}
		int mTotal = (count * 3) / 8 + (((count*3) % 8 > 0) ? 1 : 0);
		mCount = mTotal;
		//if (mverts != nullptr)delete[] mverts;
		//mverts = new __m256[mTotal];
		index = 0;
		for (unsigned int i = 0; i < count*4; i += 8)
		{
			mverts[index++] = _mm256_loadu_ps(&verts[i]);
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
			//std::cout << sizeof(edges) << " edge size\n";
		if (edgeAlt == nullptr)edgeAlt = new int[242 * 16];
		if (edgeCount == nullptr)edgeCount = new int[242];
		return *this;
	}
	vec3 getVertex(const uint32_t& index) const
	{
		return vertices[index];
	}
	uint32_t supportPoint(__m256& B) const
	{
		//float results[8];
		float magnitude = -999999;
		uint32_t tempID = 0;
		uint32_t indFirst = 0;
		for (int i = 0; i < mCount; ++i)
		{

			__m256 mults = _mm256_mul_ps(mverts[i], B);
			//_mm256_storeu_ps(&results[0], mults);

			float dR = mults[0] + mults[1] + mults[2];
			float dRR = mults[4] + mults[5] + mults[6];
			if (dR > dRR)
			{
				if (dR > magnitude)
				{
					magnitude = dR;
					tempID = indFirst;
				}
			}
			else 
			{
				if (dRR > magnitude)
				{
					magnitude = dRR;
					tempID = indFirst+1;
				}

			}
			indFirst+=2;
		}
		return tempID;
	}
	uint32_t supportPointHillClimb(__m256& direction, const int& prevID)
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
	vec3  vertices[242];
	float* verts = nullptr;
	__m256 mverts[121];//__m256* mverts = nullptr;
	int count = 0;
	int mCount = 0;
	int edgeTotal = 496;
	int** edges = nullptr;
	int* edgeAlt = nullptr;
	int* edgeCount = nullptr;
};

#endif
