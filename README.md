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
