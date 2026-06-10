#pragma once
#include <string>
#include <vector>

// This matches the assignment’s description:
// x,y,z required; any subset of nx,ny,nz, rgb, uv can appear.
struct VertexData {
    float x=0, y=0, z=0;
    float nx=0, ny=0, nz=0;
    float r=1, g=1, b=1;
    float u=0, v=0;
};

// Faces are triangles (3 indices)
struct TriData {
    unsigned int i0=0, i1=0, i2=0;
};

// Read ASCII PLY into vertices + triangle list.
// (These assets are small enough that a simple parser is fine.)
void readPLYFile(const std::string& fname, std::vector<VertexData>& vertices, std::vector<TriData>& faces);