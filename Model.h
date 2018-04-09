#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>
#include <array>
#include <list>
#include "math.h"

class Face {
public:
	std::vector<int> points;
	std::vector<vec3f> screencords;
	std::vector<vec3f> vertnorms;
	vec3f normal;
	vec3f color;				//color of face
	float ks, kd, ka, alpha;	//specular, diffuse, ambient, shininess(alpha)
};

class RenderObject {
public:
	int numpoints;
	int numfaces;
	vec3f location;									
	std::vector<vec3f> points;						//point data
	std::vector<std::vector<int>> polys;			//each polygon index list
	std::vector<Face> Faces;
	std::vector<vec3f> vertnorms;
	vec3f orientation;
	vec3f color;				//color of face
	float ks, kd, ka, alpha;	//specular, diffuse, ambient, shininess(alpha)

	void Scale(float s);
	void Translate(const vec3f &offset);
	void Rotate(const vec3f &angles);
};

void RenderObject::Scale(float s) {
	for (int i = 0; i < points.size(); i++) {
		points[i].x *= s; points[i].y *= s; points[i].z *= s;
	}
};

void RenderObject::Translate(const vec3f &offset) {			//update vertices
	location.x += offset.x; location.y += offset.y; location.z += offset.z;
	for (unsigned int i = 0; i < points.size(); i++) {
		points[i].x += offset.x; points[i].y += offset.y; points[i].z += offset.z;
	}
	// translate ...
}

void RenderObject::Rotate(const vec3f &angles) {					//translate to origin, rotate, translate back
	vec3f templocation = location;
	Translate(-location);
	orientation.x += angles.x; orientation.y += angles.y; orientation.z += angles.z;
	Matrix rotatemat =


	{ {cos(angles.y) * cos(angles.z),		-cos(angles.y) * sin(angles.z),			sin(angles.y),		0},
	{ sin(angles.x) * sin(angles.y) * cos(angles.z) + cos(angles.x) * sin(angles.z),	 -sin(angles.x) * sin(angles.y) * sin(angles.z) + cos(angles.x) * cos(angles.z),			-sin(angles.x) * cos(angles.y),		 0.},
	{ -cos(angles.x) * sin(angles.y) * cos(angles.z) + sin(angles.x) * sin(angles.z),		cos(angles.x) * sin(angles.y) * sin(angles.z) + sin(angles.x) * cos(angles.z),			cos(angles.x) * cos(angles.y),		0. },
	{0.,0., 0., 1. } };


	for (unsigned int i = 0; i < points.size(); i++) {
		points[i] = matrixmultvec(rotatemat, points[i]);
	}
	Translate(templocation);
	// rotate ...
}

RenderObject ParseObj(char *path) {
	FILE *objfile = fopen(path, "r");
	if (objfile == NULL) {
		printf("Impossible to open the file !\n");
		exit(1);
	}

	RenderObject obj;
	char lineHeader[128];
	int res = fscanf(objfile, "%s %d %d", lineHeader, &obj.numpoints, &obj.numfaces);
	obj.points.resize(obj.numpoints);
	obj.Faces.resize(obj.numfaces);
	obj.vertnorms.resize(obj.numpoints);
	int i = 0;
	for (i = 0; i < obj.numpoints; i++) {
		res = fscanf(objfile, "%f %f %f\n", &obj.points[i].x, &obj.points[i].y, &obj.points[i].z);		//scan the coordinates of a point
		if (res == EOF)
			break;												// EOF = End Of File. Quit the loop.
	}

	int facepoints = 0;											//number of points in the current polygon in a file line
	int tempindex;												//index of a point in one of the polygons in a line

	std::vector< std::list< vec3f > > vertexnormals(obj.points.size());				//vector of lists of face normals of each polygon

	for (i = 0; i < obj.numfaces; i++) {							//loop scanning for the number of indices in every polygon (each line)
		res = fscanf(objfile, "%d", &facepoints);
		if (res == EOF)
			break;												// EOF = End Of File. Quit the loop.
		obj.Faces[i].screencords.resize(facepoints);
		obj.Faces[i].vertnorms.resize(facepoints);
		for (int j = 0; j < facepoints; j++) {
			fscanf(objfile, "%d", &tempindex);
			obj.Faces[i].points.push_back(tempindex);					//append index to polygon vector
			obj.Faces[i].color = { ((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)) };
			obj.Faces[i].ks = .2;
			obj.Faces[i].kd = .2;
			obj.Faces[i].ks = .2;
			obj.Faces[i].alpha = .2;
		}
		vec3f edge1 = vec3f(obj.points[obj.Faces[i].points[1] - 1].x - obj.points[obj.Faces[i].points[0] - 1].x, obj.points[obj.Faces[i].points[1] - 1].y - obj.points[obj.Faces[i].points[0] - 1].y, obj.points[obj.Faces[i].points[1] - 1].z - obj.points[obj.Faces[i].points[0] - 1].z);
		vec3f edge2 = vec3f(obj.points[obj.Faces[i].points[2] - 1].x - obj.points[obj.Faces[i].points[0] - 1].x, obj.points[obj.Faces[i].points[2] - 1].y - obj.points[obj.Faces[i].points[0] - 1].y, obj.points[obj.Faces[i].points[2] - 1].z - obj.points[obj.Faces[i].points[0] - 1].z);
		obj.Faces[i].normal = crossproduct(edge2, edge1);
		obj.Faces[i].normal.normalize();
		for (int j = 0; j < facepoints; j++) {
			vertexnormals[obj.Faces[i].points[j] - 1].push_back(obj.Faces[i].normal);
		}
	}

	for (int i = 0; i < obj.numpoints; i++) {
		vec3f tempnormal;
		float tempcount = 0.;
		for (std::list<vec3f>::iterator it = vertexnormals[i].begin(); it != vertexnormals[i].end(); it++) {
			tempnormal.x += it->x;
			tempnormal.y += it->y;
			tempnormal.z += it->z;
			tempcount++;
		}
		obj.vertnorms[i] = { tempnormal.x / tempcount, tempnormal.y / tempcount, tempnormal.z / tempcount };		//average the normals for vertex normal
	}
	
	for (int i = 0; i < obj.numfaces; i++) {
		for (int j = 0; j < obj.Faces[i].points.size(); j++) {
			obj.Faces[i].vertnorms[j] = obj.vertnorms[obj.Faces[i].points[j] - 1];
		}
	}

	return obj;
}

