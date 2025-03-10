//---------------------------------------------------------------------------
//
// Copyright (c) 2015 Taehyun Rhee, Joshua Scott, Ben Allen
//
// This software is provided 'as-is' for assignment of COMP308 in ECS,
// Victoria University of Wellington, without any express or implied warranty. 
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// The contents of this file may not be copied or duplicated in any form
// without the prior permission of its owner.
//
//----------------------------------------------------------------------------

#include <cmath>
#include <iostream> // input/output streams
#include <fstream>  // file streams
#include <sstream>  // string streams
#include <string>
#include <stdexcept>
#include <vector>

#include "comp308.hpp"
#include "geometry.hpp"

using namespace std;
using namespace comp308;


Geometry::Geometry(string filename) {
	m_filename = filename;
	readOBJ(filename);
	if (m_triangles.size() > 0) {
		createDisplayListPoly();
		createDisplayListWire();
	}
}


void Geometry::readOBJ(string filename) {

	// Make sure our geometry information is cleared
	m_points.clear();
	m_uvs.clear();
	m_normals.clear();
	m_triangles.clear();

	// Load dummy points because OBJ indexing starts at 1 not 0
	m_points.push_back(vec3(0,0,0));
	m_uvs.push_back(vec2(0,0));
	m_normals.push_back(vec3(0,0,1));


	ifstream objFile(filename);

	if(!objFile.is_open()) {
		cerr << "Error reading " << filename << endl;
		throw runtime_error("Error :: could not open file.");
	}

	cout << "Reading file " << filename << endl;
	// int bunny = 0;
	// if (filename == "bunny.obj"){
	// 	bunny = 1;
	// }
	
	// good() means that failbit, badbit and eofbit are all not set
	while(objFile.good()) {

		// Pull out line from file
		string line;
		std::getline(objFile, line);
		istringstream objLine(line);

		// Pull out mode from line
		string mode;
		objLine >> mode;

		// Reading like this means whitespace at the start of the line is fine
		// attempting to read from an empty string/line will set the failbit
		if (!objLine.fail()) {

			if (mode == "v") {
				vec3 v;
				objLine >> v.x >> v.y >> v.z;
				m_points.push_back(v);

			} else if(mode == "vn") {
				vec3 vn;
				objLine >> vn.x >> vn.y >> vn.z;
				m_normals.push_back(vn);

			} else if(mode == "vt") {
				vec2 vt;
				objLine >> vt.x >> vt.y;
				m_uvs.push_back(vt);

			} else if(mode == "f") {

				vector<vertex> verts;
				while (objLine.good()){
					vertex v;

					//-------------------------------------------------------------
					// [Assignment 1] :
					// Modify the following to parse the bunny.obj. It has no uv
					// coordinates so each vertex for each face is in the format
					// v//vn instead of the usual v/vt/vn.
					//
					// Modify the following to parse the dragon.obj. It has no
					// normals or uv coordinates so the format for each vertex is
					// v instead of v/vt/vn or v//vn.
					//
					// Hint : Check if there is more than one uv or normal in
					// the uv or normal vector and then parse appropriately.
					//-------------------------------------------------------------

					// Assignment code (assumes you have all of v/vt/vn for each vertex)

					/*
						Original Code
					*/
					// objLine >> v.p;		// Scan in position index
					// objLine.ignore(1);	// Ignore the '/' character
					// objLine >> v.t;		// Scan in uv (texture coord) index
					// objLine.ignore(1);	// Ignore the '/' character
					// objLine >> v.n;		// Scan in normal index

					/*
						My Parser
					*/
					std::vector<int> output;
					int data;

					if (filename.find("bunny.obj") != std::string::npos){
						while (objLine >> data) {
							output.push_back(data);
							objLine.ignore(1);
							objLine.ignore(1);
							objLine >> data;
							output.push_back(data);
						}
					}

					while (objLine >> data) {
						output.push_back(data);
						objLine.ignore(1);
					}
					for (int i = 0; i < 3; i++){
						if(output.size() == 3){
							v.p = output[i];
						}
						if (output.size() == 6){
							v.p = output[i*2];
							v.n = output[i*2+1];
						}
						if (output.size() == 9){
							v.p = output[i*3];
							v.t = output[i*3+1];
							v.n = output[i*3+2];
						}
						verts.push_back(v);
					}

				}

				// IFF we have 3 verticies, construct a triangle
				if(verts.size() == 3){
					triangle tri;
					tri.v[0] = verts[0];
					tri.v[1] = verts[1];
					tri.v[2] = verts[2];
					m_triangles.push_back(tri);

				}
			}
		}
	}

	cout << "Reading OBJ file is DONE." << endl;
	cout << m_points.size()-1 << " points" << endl;
	cout << m_uvs.size()-1 << " uv coords" << endl;
	cout << m_normals.size()-1 << " normals" << endl;
	cout << m_triangles.size() << " faces" << endl;


	// If we didn't have any normals, create them
	if (m_normals.size() <= 1) createNormals();
}

//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to populate the normals for 
// the model currently loaded. Compute per face normals
// first and get that working before moving onto calculating
// per vertex normals.
//-------------------------------------------------------------
void Geometry::createNormals() {
	// YOUR CODE GOES HERE
	// ...
	m_normals.clear();

	// vector<vec3> face_list[m_points.size()];
	vector<vec3> face_list[m_points.size()];

	for (int i = 0; i < m_triangles.size(); ++i){
		triangle t = m_triangles[i];
		vec3 v1 = m_points[t.v[1].p] - m_points[t.v[0].p];
		vec3 v2 = m_points[t.v[2].p] - m_points[t.v[1].p];

		vec3 nor = cross(v1, v2);
		nor = normalize(nor);

		for (int j = 0; j < 3; ++j){
			vector<vec3> temp = face_list[t.v[j].p];
			temp.push_back(nor);
			face_list[t.v[j].p] = temp;
			m_triangles[i].v[j].n = t.v[j].p;
		}
	}

	for (int i = 0; i < m_points.size(); ++i){
		vec3 ver_n;
		for (int j = 0; j < face_list[i].size(); ++j){
			ver_n += face_list[i][j];
		}
		m_normals.push_back(ver_n);
	}
	
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as wireframe model
//-------------------------------------------------------------
void Geometry::createDisplayListPoly() {
	// Delete old list if there is one
	if (m_displayListPoly) glDeleteLists(m_displayListPoly, 1);

	// Create a new list
	cout << "Creating Poly Geometry" << endl;
	m_displayListPoly = glGenLists(1);
	glNewList(m_displayListPoly, GL_COMPILE);


	// YOUR CODE GOES HERE
	// ...
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_triangles.size(); ++i){
		triangle t = m_triangles[i];

		glNormal3f(m_normals[t.v[0].n].x, m_normals[t.v[0].n].y, m_normals[t.v[0].n].z);
		glVertex3f(m_points[t.v[0].p].x, m_points[t.v[0].p].y, m_points[t.v[0].p].z);

		glNormal3f(m_normals[t.v[1].n].x, m_normals[t.v[1].n].y, m_normals[t.v[1].n].z);
		glVertex3f(m_points[t.v[1].p].x, m_points[t.v[1].p].y, m_points[t.v[1].p].z);

		glNormal3f(m_normals[t.v[2].n].x, m_normals[t.v[2].n].y, m_normals[t.v[2].n].z);
		glVertex3f(m_points[t.v[2].p].x, m_points[t.v[2].p].y, m_points[t.v[2].p].z);

	}
    glEnd();
	
	glEndList();
	cout << "Finished creating Poly Geometry" << endl;
}


//-------------------------------------------------------------
// [Assignment 1] :
// Fill the following function to create display list
// of the obj file to show it as polygon model
//-------------------------------------------------------------
void Geometry::createDisplayListWire() {
	// Delete old list if there is one
	if (m_displayListWire) glDeleteLists(m_displayListWire, 1);


	// Create a new list
	cout << "Creating Wire Geometry" << endl;
	m_displayListWire = glGenLists(1);
	glNewList(m_displayListWire, GL_COMPILE);


	// YOUR CODE GOES HERE
	// ...
	for (int i = 0; i < m_triangles.size(); ++i){
		glBegin(GL_LINE_LOOP);
		triangle t = m_triangles[i];

		glNormal3f(m_normals[t.v[0].n].x, m_normals[t.v[0].n].y, m_normals[t.v[0].n].z);
		glVertex3f(m_points[t.v[0].p].x, m_points[t.v[0].p].y, m_points[t.v[0].p].z);

		glNormal3f(m_normals[t.v[1].n].x, m_normals[t.v[1].n].y, m_normals[t.v[1].n].z);
		glVertex3f(m_points[t.v[1].p].x, m_points[t.v[1].p].y, m_points[t.v[1].p].z);

		glNormal3f(m_normals[t.v[2].n].x, m_normals[t.v[2].n].y, m_normals[t.v[2].n].z);
		glVertex3f(m_points[t.v[2].p].x, m_points[t.v[2].p].y, m_points[t.v[2].p].z);
		glEnd();
	}

	glEndList();
	cout << "Finished creating Wire Geometry" << endl;
}


void Geometry::renderGeometry() {
	if (m_wireFrameOn) {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// glutWireTeapot function & uncomment the glCallList function
		//-------------------------------------------------------------

		glShadeModel(GL_SMOOTH);
		// glutWireTeapot(5.0);
		glCallList(m_displayListWire);

	} else {

		//-------------------------------------------------------------
		// [Assignment 1] :
		// When moving on to displaying your obj, comment out the
		// glutWireTeapot function & uncomment the glCallList function
		//-------------------------------------------------------------

		glShadeModel(GL_SMOOTH);
		// glutSolidTeapot(5.0);
		glCallList(m_displayListPoly);

	}
}


void Geometry::toggleWireFrame() {
	m_wireFrameOn = !m_wireFrameOn;
}