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
	vec3f color;				//color of face
	float ks, kd, ka, alpha;	//specular, diffuse, ambient, shininess(alpha)
};

class RenderObject {
public:
	int numpoints;
	int numpolys;
	vec3f location;									
	std::vector<vec3f> points;						//point data
	std::vector<std::vector<int>> polys;			//each polygon index list
	std::vector<Face> Faces;
	vec3f orientation;
	//Matrix modelmat;
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
	int res = fscanf(objfile, "%s %d %d", lineHeader, &obj.numpoints, &obj.numpolys);
	obj.points = std::vector<vec3f>(obj.numpoints);
	obj.polys = std::vector<std::vector<int>>(obj.numpolys);
	int i = 0;
	for (i = 0; i < obj.numpoints; i++) {
		res = fscanf(objfile, "%f %f %f\n", &obj.points[i].x, &obj.points[i].y, &obj.points[i].z);		//scan the coordinates of a point
		if (res == EOF)
			break;												// EOF = End Of File. Quit the loop.
	}

	int polypoints = 0;											//number of points in the current polygon in a file line
	int tempindex;												//index of a point in one of the polygons in a line

	for (i = 0; i < obj.numpolys; i++) {							//loop scanning for the number of indices in every polygon (each line)
		res = fscanf(objfile, "%d", &polypoints);
		if (res == EOF)
			break;												// EOF = End Of File. Quit the loop.
		for (int j = 0; j < polypoints; j++) {
			fscanf(objfile, "%d", &tempindex);
			obj.polys[i].push_back(tempindex);					//append index to polygon vector
		}
	}
	return obj;
}

