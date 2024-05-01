# MeshVectorOps_Demo
Test space for demonstrating some computational geometry algorithms and performance testing.

## Support Point Serial Performance
### Description
This application is intended to get a baseline test for the cost to execute both a generic and hill climb optimized support point function serially.

### Methods
Performance is based on averaging 100,000 executions of each support point function. Three different geometries were tested, each with a brute-force and a hill-climb execution -- resulting in a total of *six trials*.

The program was executed on an Intel i7-12900K.

The program outputs both the average execution time but also the resulting support point vertex. The search vector was chosen to ensure that no discrepancy in ties would occur in the generic vs. hill climb implementation. The algorithm is deterministic, but variations in how similar maximums are compared could mean that a hill climb operation performaning the search in an order different from the brute force algorithm might result in different results. Of course, this could be avoided by finding a consistent secondary value (say index number) to settles ties in the maximum operation.

#### Shapes
The geometries used were three different convex hulls generated in Blender, with vertex and edge data extracted using a Python script. The geometries were as follows:

- Cube (8 vertices)
- Suzanne's Convex Hull (66 vertices)
- UV Sphere (242 vertices)

None of the shapes were forced to be triangulated and instead the shapes utilize a minimum number of edges in their construction.

#### Algorithm
The *support point* is a function that maps a direction to a location within a convex geometry. Specifically, it returns the position on the geometry that is furthest along a given direction. Some support point functions work in constant time, for instance a circle or sphere can take the direction of interest, normalize it, and then scale it by the radius to compute in **O(1)** the location on the hull furthest in a direction.

For discretized geometry, among other representations, O(1) is not viable for a general formulation and instead is generalized to **O(N)** where *N is the number of vertices*. For a mesh, a graph of vertices connected with linear edges, the linear complexity is from testing the direction of interest against every vertex in the mesh. This is done with a dot product, where the result of interest is the vertex whose dot product with the search direction is greatest.

Hill climbing is an optimization that can be deployed to prune some of the search space. Given a convex mesh, if we start our support point search at an arbitrary vertex, and then test its neighboring vertices, we can choose to continue the search in the direction of the neighbor whose magnitude was furthest in the search direction (still a maximum dot product). This ensures that the algorithm only searches in the direction of the current known maximum.

**There is a discrepancy with the hill climb,** *if the complexity of a mesh is small enough, then the hill climb adds more constant time work to the algorithm -- leading to worse performance*. This discrepancy is one reason why both serial algorithms are used in testing, *but, this will be useful when examining parallel performance to determine if the parallel variant is faster than the serial hill climb optimization*.

#### Baseline Performance

- **Cube**
  - Brute force: *29.7303 ns*
  - Hill climb: *42.7851 ns*
- **Suzanne Convex Hull**
  - Brute force: *129.675 ns*
  - Hill climb: *99.9375 ns*
- **UV Sphere**
  - Brute force: *421.309 ns*
  - Hill climb: *95.2129 ns*
 

## Parallel Options

### Almost Pleasantly Parallel (Reduction Required)
The support point function relies on a series of independent dot products to be performed, however there is a dependency with an operation that is both commutative and associative -- a maximum operation. This is not a simple *max* call, however, and instead is an *arg max* style operation where we are required to store the index associated with the maximum dot product result. This means that if the for-loop of the support point function was treated in a pleasantly parallel fashion, then we must consider a parallel reduction operation. This can be achieved with existing utilities like OpenMP, but requires a custom reduction function to be defined to keep track of not just the maximum result but also the associated index with this maximum value.

The implementation provided, using the altered source and header files found in the OMP directory, seem to cause an excess of overhead in performing the reduction operation. However, this might be a misuse of the OMP tools, but it could be possible that the overhead is quite large. To understand these thoughts, please observe the results below:

#### OMP Reduction Performance
For 8 Threads
- **Cube**
  - Brute force: *2109.41 ns*
- **Suzanne Convex Hull**
  - Brute force: *1979.85 ns*
- **UV Sphere**
  - Brute force: *2009.57*

It is suspected that overhead is involved because the cost of executing this custom OMP reduction on the Cube is significantly slower than when trying it on Suzanne and the UV Sphere. More over, the UV Sphere and Suzanne actually have similar performance which might indicate that until a certain threshold of vertices is reached in a complex shape, we might be likely to continue to see execution perform around this 2000 ns range.

#### SIMD Using AVX
The second best option is to consider not parallelizing the for-loop of a support point function, but instead to try to parallelize the primary operation of the function -- the dot product.

AVX 1 utilizes 256-bit ranges of words, and in this case we are using 3-part vectors of floats, and so can put at most 2 of these vectors into a single AVX 256 register at a time. However, the primary operation that can fully exploit AVX here is the component-wise multipliation of floats meaning we still need to compute 2 floating-point additions.

There are two flavors of expectation that were tested. In the first case, all of the *vec3* objects were not used directly, and instead, for **optimal caching**, the ShapeSIMD class is storing an array of the floats for each vector. This arrangement is in 4-parts to account for the 256-bit size of the AVX 1 registers. Then, before the dot product is performed, the array elements are converted into a __m256 type to be used with AVX 1. **This was performed specifically in the *brute force* case of the SIMD code available in the /SIMD directory**.

The second option was to try to abstract the SIMD optimization within the vec3 class itself. The problem with this is we will not utilize a maximum increase in performance -- a theoretical 6x increase specifically to the factoring of components in the dot-product operations (as seen in the brute-force case above). Instead, we can take the three vector values and in the dot product method, go ahead and push the three-part data into a 256-bit AVX type in the hopes that we should still see performance gains. This is shown in the hill-climb function results.

In both of these methods, we take away some overhead by converting the search direction vector once to a __m256 type, but it is possible this could be extended to all of the vector objects as well. Listed below are the brute-force and then the hill-climb flavors with their respective approaches to using AVX 1 instructions.

#### AVX Component-wise Multiplication (Brute force -- Flavor 1)
- **Cube**
  - *33.1247 ns*
- **Suzanne Convex Hull**
  - *129.733 ns*
- **UV Sphere**
  - *409.655 ns*
 
#### AVX Component-wise Multiplication (Hill Climb -- Flavor 2)
- **Cube**
  - *62.7897 ns*
- **Suzanne Convex Hull**
  - *156.997 ns*
- **UV Sphere**
  - *224.429 ns*


## Results
We can obverse that in general, support point operations happen frequently, so trying to maximize performance with parallel-for loop execution is likely moot. Instead, utilizing the most efficient optimizations for hill-climb operations where several parallel hill-climb support point functions can be executed would be ideal. In this regard, the first flavor of the AVX execution seems to shave a reasonable amount of time off of the dot product operation. Sadly, the second flavor, which offers a better Software Engineering approach to this problem is not nearly as performant as the baseline code that did not have additional compiler optimizations turned on. Of course, the original version, as examined in assembly, did have vectorization used -- which has been a default feature of gnu c++ for a few versions.

In conclusion, we can possibly shave offer execution costs through SIMD. This will technically add up in savings over time, but only if sufficiently large numbers of support point executions are required.
