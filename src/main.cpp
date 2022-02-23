#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "Gmap.h"

int main(int argc, const char * argv[]) {

    std::string file_in = "./cube.txt";
    std::ifstream ifs;
    ifs.open(file_in);

    bool status=ifs.is_open();
    std::cout<<"the result is "<<status<<std::endl;

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    int vertices_id =0;
    int face_id=0;
    if (ifs.is_open()) {
        std::string line;
        while (getline(ifs, line)) {
            std::istringstream iss(line);
            std::string word;
            iss >> word;
            if (word == "v") {
                std::vector<float> coordinates;
                while (iss >> word) coordinates.push_back(std::stof(word));
                if (coordinates.size() == 3) vertices.emplace_back(coordinates[0], coordinates[1], coordinates[2],vertices_id);
                else vertices.emplace_back();
                vertices_id++;
            }
            else if (word == "f") {
                std::vector<int> face;
                while(iss>>word) face.push_back(std::stof(word));
                if (face.size()==4) {
                    faces.emplace_back( face, vertices,face_id);
                    face_id++;
                }
                else faces.emplace_back();
            }
        }
    }
    for (int i = 0; i < faces.size(); ++i) {
        std::cout<<" this is face no. "<< faces[i].face_id <<std::endl;

        // for(auto ver:faces[i].edge_list) std::cout<<"the vertex is "<< ver.edge_id <<std::endl;

        // for (int j = 0; j < faces[i].edge_list.size(); ++j) std::cout<<"the edge is "<< faces[i].edge_list[j].edge_start <<", "<<faces[i].edge_list[j].edge_end << std::endl;
    }

    // ## Construct generalised map using the structures from Gmap.h ##

    std::vector<std::vector<Dart *>>darts_list=GeneralizedMap::build_darts(faces);
    for (auto darts : darts_list) {
        std::cout<< darts.size() <<std::endl;
        for (auto dart:darts) {
            std::cout<<"The dart is "<<dart->dart_id <<", the vertex is "<< dart->vertex->vertex_id<<", the edge is "<<dart->edge->edge_id<<", the face is "<<dart->face->face_id<<std::endl;
        }
    }
    // ## Output generalised map to CSV ##

    // ## Create triangles from the darts ##

    // ## Write triangles to obj ##

    return 0;
}
