#pragma once

// ---------------------------some standard libraries that are helpfull for reading/writing text files
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "Point.h"

#define _NULL_ 9999;

class Point;
class Dart;
class Vertex;
class Edge;
class Face;
class Volume;

/*
Below you find the basic elements that you need to build the generalised map.
The main thing you need to fill out are the links between the elements:
  * the involutions and cells on the Dart
  * the darts on the cells

One way to do this is by using pointers. eg. define a member on the dart struct like

  Struct Dart {
    // involutions:
    Dart* a0 = nullptr;
    // ...

    // cells:
    // ...

  };

Then you could create and link Darts like:

  Dart* dart_a = new Dart();
  Dart* dart_b = new Dart();

  dart_a->a0 = dart_b;
*/


class Dart {
private:  
    int m_id; // dart id
    Dart* involution_pointers[4];  // involutions: a0, a1, a2, a3
    std::size_t cell_dimensions[4]; // cells: 0-dimensional, 1-dimensional, 2-dimensional, 3-dimensional

public:
    Dart(): m_id(0){
        for (auto& pDart : involution_pointers)pDart = nullptr;
        for (auto c : cell_dimensions)c = 0;
    }

    // settings
    // set dart id
    int& id() { return m_id; }

    // set involutions
    void set_involution_to_dart(int dimension, Dart* dptr) {
        involution_pointers[dimension] = dptr; //dptr could be nullptr
    }

    /*set incident cells :
    * cell_dimensions[0]: id of 0-cell (vertex id)
    * cell_dimensions[1]: id of 1-cell (edge id)
    * cell_dimensions[2]: id of 2-cell (face id)
    * cell_dimensions[3]: id of 3-cell (volume id) -- always null(_NULL_)*/
    void set_incident_cell(int dimension, std::size_t cell_id) {
        if (dimension >= 0 && dimension <= 3) {
            cell_dimensions[dimension] = cell_id;
        }
    }

    // helpful for debugging
    void print_id()const { std::cout << m_id << '\n'; }
    void print_cell(int dimension) { std::cout << cell_dimensions[dimension] << '\n'; }
    void print_involution(int dimension) { std::cout << involution_pointers[dimension]->id() << '\n'; }


    // cells:
    // ...

};


class Vertex : public Point{
private:
    int m_id;

    // a dart incident to this Vertex:
    // dart id? or a pointer?

public:
    // constructor without arguments
    Vertex() : Point(),m_id(0){}

    // constructor with x,y,z arguments to immediately initialise the point member on this Vertex.
    Vertex(float x, float y, float z) : Point(x, y, z),m_id(0){}

    // access id
    int& id() { return m_id; }


    virtual ~Vertex() = default;
    
};


class Edge {
public:
    // a dart incident to this Edge:
    // ...

    // function to compute the barycenter for this Edge (needed for triangulation output):
    // Point barycenter() {}
};


class Face {
public:
    // a dart incident to this Face:
    // ...

    // function to compute the barycenter for this Face (needed for triangulation output):
    // Point barycenter() {}

};


class Volume {
public:
    // a dart incident to this Volume:
    // ...

};


class ReadOBJ {
public:
    /*
    * function: read obj file and store vertices and face indexes.
    * @parameter:
    * param_filepath: file path
    * param_filename: file name
    * vptr: a vector pointer pointing to a std::vector<Vertex> array, vertex list
    * fptr: a vector pointer pointing to a std::vector<std::vector<int>> array, face list
    */
    static void readobj(
        const std::string& param_filepath,
        const std::string& param_filename,
        std::vector<Vertex>* vptr,
        std::vector<std::vector<int>>* fptr)
    {
        std::string filename = param_filepath + param_filename; // filename
        std::string line;
        std::ifstream file(filename);
        if (!file.is_open()) { std::cerr << "file open failed! " << '\n'; }

        while (std::getline(file, line)) { // for each line in the file
            if (line.empty()) {
                continue; // skip empty lines:
            }

            std::stringstream ss(line);
            std::string field;
            std::string::size_type sz; // NOT std::size_t, size of std::string

            std::vector<float> coordinates; // store xyz coordinates in each line
            std::vector<int> face_vertex; // store face-point index in each line
            bool vertex_flag(false), face_flag(false); // to mark the current line: vertex or face, mark it as 'false' before process each line
            
            // for each element(field, type: string) in each line
            while (std::getline(ss, field, ' ')) { 
                
                 // current line is vertex
                if (field == "v") { 
                    vertex_flag = true; 
                    continue; // jump to next element in this line
                }

                // current line is face
                else if (field == "f") { 
                    face_flag = true; 
                    continue; // jump to next element in this line
                }

                if (vertex_flag) {
                    // process xyz coordinates
                    coordinates.emplace_back(std::stof(field, &sz));
                }
                else if (face_flag) {
                    // process f numbers
                    face_vertex.emplace_back(std::stoi(field, &sz));
                }
            } // end while: process each element in one line

            // process each point
            if (!coordinates.empty() && coordinates.size() == 3) vptr->emplace_back(coordinates[0], coordinates[1], coordinates[2]); 
            
            // process each face
            if (!face_vertex.empty())fptr->emplace_back(face_vertex);
        
        } // end while: each line in the file

    }

};


class BuildGmap {
public:

    /*
    * function: to judge whether the given elements are repeate
    * @parameter:
    * edge: current edge, type: a std::vector<int> array, contains two elements
    * eptr: edge list
    */
    static bool repeateEdgeCheck(
        std::vector<int>* edge,
        std::vector<std::vector<int>>* eptr)
    {
        for (auto& e : *eptr) {
            if (
                ((*edge)[0] == e[0] && (*edge)[1] == e[1]) ||
                ((*edge)[1] == e[0] && (*edge)[0] == e[1])
                ) {
                return true;
            }
        }
        return false;
    }


    /*
    * function: build edge_list based on face_list
    * @parameter:
    * fptr: a vector pointer pointing to a std::vector<std::vector<int>> array, face list
    * eptr: a vector pointer pointing to a std::vector<std::vector<int>> array. edge list
    */
    static void buildEdgeList(
        std::vector<std::vector<int>>* fptr,
        std::vector<std::vector<int>>* eptr)
    {
        for (auto& f : *fptr) { // each face: contains multiple vertex indexes, NB: 1-based indexes

            if (!f.empty()) {
                int N = (int)f.size(); // DO NOT use f.size() directly, for f.size()-1 may lead to index errors

                std::vector<int> edge_index; // initialize: add the first edge into the edge list
                edge_index.emplace_back(f[0]);
                edge_index.emplace_back(f[1]);
                if (repeateEdgeCheck(&edge_index, eptr)) {}
                else eptr->emplace_back(edge_index);

                for (int i = 1; i < N - 1; ++i) {
                    // repeate check
                    edge_index[0] = f[i];
                    edge_index[1] = f[i + 1];
                    if (repeateEdgeCheck(&edge_index, eptr))continue;
                    else eptr->emplace_back(edge_index);

                } // end for

                // make edge: the last id in a face-index, and the first id in a face-index
                // ie: face-index:[1,2,3,4], add [1,4](same as [4,1]) to the edge list
                // No need to execute repeate check first
                // Cuz the first element in a face-index is different from the last element in a face-index
                edge_index[0] = f[N - 1];
                edge_index[1] = f[0]; // caution: may overflow
                if (repeateEdgeCheck(&edge_index, eptr)) {}
                else eptr->emplace_back(edge_index);

            }// end if

        }// end for

    }// end of function


    /*
    * function: build face-edge maps
    * ie:
    * face-edge[0]:
    * all the edges belonging to face 0
    * it should contains multiple integers, indicating the edge id in the edge list
    * 
    * @parameter:
    * fptr: a vector pointer pointing to a std::vector<std::vector<int>> array, face list
    * eptr: a vector pointer pointing to a std::vector<std::vector<int>> array, edge list
    * faceid: the id of a certain face in the face list
    * 
    * @return:
    * a std::vector<int> contains the edge indexes in the edge list
    */
    static std::vector<int> facefindEdge(
        std::vector<std::vector<int>>* fptr,
        std::vector<std::vector<int>>* eptr,
        int faceid)
    {
        std::vector<int> result;              
        if (faceid < 0 || faceid >= fptr->size())return result;


        std::vector<int>* faceptr = &fptr->at(faceid); // face[faceid]: std::vector<int>
        int numEdge = (int)eptr->size();
        int numFace = (int)faceptr->size();


        // each integer(vertex id) in one face
        // traverse until the last-1 integer and the last integer
        // ie face[0]: [1,2,3,4]
        // look up: (1,2)(or(2,1)),(2,3)(or(3,2)),(3,4)(or(4,3)) in the edge list 
        // to find (4,1), need to manually find the last integer and the first integer
        for (int fid = 0; fid != numFace - 1; ++fid) {
            int vertexA = faceptr->at(fid);
            int vertexB = faceptr->at(fid + 1); // caution: may overflow

            for (int eid = 0; eid != numEdge; ++eid) {
                int vertex_a = (*eptr)[eid][0]; // each element in eptr array contains two integers
                int vertex_b = (*eptr)[eid][1];

                if ((vertexA == vertex_a && vertexB == vertex_b) ||
                    (vertexA == vertex_b && vertexB == vertex_a))
                {
                    result.emplace_back(eid); // add the current edge index into the result
                } // end if
            } // end for: each edge in the edge list

        } // end for: each integer in a face, not including the last and the first


        // find the last integer and the first integer
        int vertexA = faceptr->at(numFace - 1); // the last integer
        int vertexB = faceptr->at(0); // the first integer
        for (int eid = 0; eid != numEdge; ++eid) {
            int vertex_a = (*eptr)[eid][0]; // each element in eptr array contains two integers
            int vertex_b = (*eptr)[eid][1];

            if ((vertexA == vertex_a && vertexB == vertex_b) ||
                (vertexA == vertex_b && vertexB == vertex_a))
            {
                result.emplace_back(eid); // add the current edge index into the result
            } // end if
        } // end for: each edge in the edge list

        return result;
    }
};
