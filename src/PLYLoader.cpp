#include "PLYLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Reads ASCII PLY files with:
// - element vertex N
// - element face M
// - vertex properties (x y z plus optional nx ny nz r g b u v)
// - face lines like: "3 i0 i1 i2"
//
// I’m not trying to write a “universal” PLY parser, just what the assignment needs.
void readPLYFile(const std::string& fname, std::vector<VertexData>& vertices, std::vector<TriData>& faces) {
    std::ifstream in(fname);
    if (!in) {
        std::cerr << "ERROR: Cannot open PLY: " << fname << "\n";
        return;
    }

    std::string line;
    int numVertices = 0;
    int numFaces = 0;

    // Store vertex property names in the order they appear in header
    std::vector<std::string> props;

    // --- 1) Parse header ---
    bool inHeader = true;
    while (inHeader && std::getline(in, line)) {
        std::stringstream ss(line);
        std::string word;
        ss >> word;

        if (word == "element") {
            std::string what;
            ss >> what;
            if (what == "vertex") ss >> numVertices;
            else if (what == "face") ss >> numFaces;
        }
        else if (word == "property") {
            // Two common patterns:
            //   property float x
            //   property list uchar int vertex_indices
            std::string type;
            ss >> type;

            if (type == "list") {
                // This is usually face property, not a vertex property. Ignore here.
                // Face parsing later will handle the indices.
            } else {
                std::string name;
                ss >> name;
                props.push_back(name);
            }
        }
        else if (word == "end_header") {
            inHeader = false;
        }
    }

    vertices.clear();
    faces.clear();
    vertices.reserve(numVertices);
    faces.reserve(numFaces);

    // --- 2) Read vertex data ---
    for (int i = 0; i < numVertices; i++) {
        if (!std::getline(in, line)) break;

        std::stringstream ss(line);
        VertexData v;

        // Read tokens in the same order as header properties
        for (size_t p = 0; p < props.size(); p++) {
            float val = 0.0f;
            ss >> val;

            const std::string& name = props[p];

            // Assign based on property name
            if      (name == "x") v.x = val;
            else if (name == "y") v.y = val;
            else if (name == "z") v.z = val;
            else if (name == "nx") v.nx = val;
            else if (name == "ny") v.ny = val;
            else if (name == "nz") v.nz = val;
            else if (name == "red")   v.r = val / 255.0f;
            else if (name == "green") v.g = val / 255.0f;
            else if (name == "blue")  v.b = val / 255.0f;
            else if (name == "u") v.u = val;
            else if (name == "v") v.v = val;

            // If the file had extra fields, I’d ignore them, but the assignment says it won’t.
        }

        vertices.push_back(v);
    }

    // --- 3) Read face data ---
    for (int i = 0; i < numFaces; i++) {
        if (!std::getline(in, line)) break;

        std::stringstream ss(line);
        int count = 0;
        ss >> count;

        // Assignment says all faces are triangles
        if (count != 3) {
            // If anything weird shows up, I skip it
            continue;
        }

        TriData t;
        ss >> t.i0 >> t.i1 >> t.i2;
        faces.push_back(t);
    }
}