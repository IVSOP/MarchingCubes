#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLErrors.hpp"


// https://paulbourke.net/geometry/polygonise/

struct LookupTable {
	static const uint16_t edgeTable[256]; // ????
	static const uint8_t cornerIndexAFromEdge[12];
	static const uint8_t cornerIndexBFromEdge[12];
	static const glm::vec3 corner_coords[8];
	// for each of the 256 cube configuration cases, array with the indices of the vertex
	static const int8_t triTable[256][16];
	static const glm::vec3 finalCoords[12];
};

// typedef struct {
//    XYZ p[3];
// } TRIANGLE;

// typedef struct {
//    XYZ p[8];      // ??????
//    double val[8]; // iso value from surface, at each vertex
// } GRIDCELL;

// /*
// 	Given a grid cell and an isolevel, calculate the triangular
// 	facets required to represent the isosurface through the cell.
// 	Return the number of triangular facets, the array "triangles"
// 	will be loaded up with the vertices at most 5 triangular facets.
// 	0 will be returned if the grid cell is either totally above
// 	of totally below the isolevel.
// */
// int Polygonise(GRIDCELL grid, double isolevel, TRIANGLE *triangles) {
// 	int i,ntriang;
// 	int cubeindex;
// 	XYZ vertlist[12];

// 	// check what vertices should be set to 1
// 	cubeindex = 0;
// 	if (grid.val[0] < isolevel) cubeindex |= 1;
// 	if (grid.val[1] < isolevel) cubeindex |= 2;
// 	if (grid.val[2] < isolevel) cubeindex |= 4;
// 	if (grid.val[3] < isolevel) cubeindex |= 8;
// 	if (grid.val[4] < isolevel) cubeindex |= 16;
// 	if (grid.val[5] < isolevel) cubeindex |= 32;
// 	if (grid.val[6] < isolevel) cubeindex |= 64;
// 	if (grid.val[7] < isolevel) cubeindex |= 128;


// 	// cube is completely inside/outside the surface, don't generate anything
// 	if (edgeTable[cubeindex] == 0) {
// 		return(0);
// 	}
// 	/* Find the vertices where the surface intersects the cube */
// 	if (edgeTable[cubeindex] & 1) {
// 		vertlist[0] = VertexInterp(isolevel, grid.p[0], grid.p[1], grid.val[0], grid.val[1]);
// 	}
// 	if (edgeTable[cubeindex] & 2) {
// 		vertlist[1] = VertexInterp(isolevel, grid.p[1], grid.p[2], grid.val[1], grid.val[2]);
// 	}
// 	if (edgeTable[cubeindex] & 4) {
// 		vertlist[2] = VertexInterp(isolevel, grid.p[2], grid.p[3], grid.val[2], grid.val[3]);
// 	}
// 	if (edgeTable[cubeindex] & 8) {
// 		vertlist[3] = VertexInterp(isolevel, grid.p[3], grid.p[0], grid.val[3], grid.val[0]);
// 	}
// 	if (edgeTable[cubeindex] & 16) {
// 		vertlist[4] = VertexInterp(isolevel, grid.p[4], grid.p[5], grid.val[4], grid.val[5]);
// 	}
// 	if (edgeTable[cubeindex] & 32) {
// 		vertlist[5] = VertexInterp(isolevel, grid.p[5], grid.p[6], grid.val[5], grid.val[6]);
// 	}
// 	if (edgeTable[cubeindex] & 64) {
// 		vertlist[6] = VertexInterp(isolevel, grid.p[6], grid.p[7], grid.val[6], grid.val[7]);
// 	}
// 	if (edgeTable[cubeindex] & 128) {
// 		vertlist[7] = VertexInterp(isolevel, grid.p[7], grid.p[4], grid.val[7], grid.val[4]);
// 	}
// 	if (edgeTable[cubeindex] & 256) {
// 		vertlist[8] = VertexInterp(isolevel, grid.p[0], grid.p[4], grid.val[0], grid.val[4]);
// 	}
// 	if (edgeTable[cubeindex] & 512) {
// 		vertlist[9] = VertexInterp(isolevel, grid.p[1], grid.p[5], grid.val[1], grid.val[5]);
// 	}
// 	if (edgeTable[cubeindex] & 1024) {
// 		vertlist[10] = VertexInterp(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6]);
// 	}
// 	if (edgeTable[cubeindex] & 2048) {
// 		vertlist[11] = VertexInterp(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7]);
// 	}

// 	/* Create the triangle */
// 	ntriang = 0;
// 	for (i = 0; triTable[cubeindex][i] != -1; i += 3) {
// 		triangles[ntriang].p[0] = vertlist[triTable[cubeindex][i  ]];
// 		triangles[ntriang].p[1] = vertlist[triTable[cubeindex][i+1]];
// 		triangles[ntriang].p[2] = vertlist[triTable[cubeindex][i+2]];
// 		ntriang++;
// 	}

// 	return(ntriang);
// }

// /*
//    Linearly interpolate the position where an isosurface cuts
//    an edge between two vertices, each with their own scalar value
// */
// XYZ VertexInterp(isolevel,p1,p2,valp1,valp2)
// double isolevel;
// XYZ p1,p2;
// double valp1,valp2;
// {
//    double mu;
//    XYZ p;

//    if (ABS(isolevel-valp1) < 0.00001)
//       return(p1);
//    if (ABS(isolevel-valp2) < 0.00001)
//       return(p2);
//    if (ABS(valp1-valp2) < 0.00001)
//       return(p1);
//    mu = (isolevel - valp1) / (valp2 - valp1);
//    p.x = p1.x + mu * (p2.x - p1.x);
//    p.y = p1.y + mu * (p2.y - p1.y);
//    p.z = p1.z + mu * (p2.z - p1.z);

//    return(p);
// }




// mpVector LinearInterp(mp4Vector p1, mp4Vector p2, float value)
// {
//     if (p2 < p1)
//     {
//         mp4Vector temp;
//         temp = p1;
//         p1 = p2;
//         p2 = temp;    
//     }

//     mpVector p;
//     if(fabs(p1.val - p2.val) > 0.00001)
//         p = (mpVector)p1 + ((mpVector)p2 - (mpVector)p1)/(p2.val - p1.val)*(value - p1.val);
//     else 
//         p = (mpVector)p1;
//     return p;
// }

// bool operator<(const mp4Vector &right) const
// {
// 	if (x < right.x)
// 		return true;
// 	else if (x > right.x)
// 		return false;

// 	if (y < right.y)
// 		return true;
// 	else if (y > right.y)
// 		return false;

// 	if (z < right.z)
// 		return true;
// 	else if (z > right.z)
// 		return false;

// 	return false;
// 	}




#endif
