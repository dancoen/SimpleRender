#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>
#include <array>
#include <math.h>


using namespace std;
typedef vector<vector<float>> Matrix;

Matrix getIdentity() {
	Matrix mat =
	{ { 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 } };
	return mat;
}

Matrix multMatrix(Matrix a, Matrix b) {			//only 4x4 for now
	float c[4];
	Matrix result = getIdentity();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				c[k] = a[i][k] * b[k][j];
			}
			result[i][j] = c[0] + c[1] + c[2] + c[3];
		}
	}
	return result;
}

class vec3f {
public:
	float x, y, z;

public:
	vec3f();
	vec3f(float x, float y, float z);
	float			dot(const vec3f &v) const;
	float			magnitude(void) const;
	vec3f &			normalize(void);
	vec3f			operator-(void) const;
};

vec3f::vec3f() {
	x = y = z = 0;
}

vec3f::vec3f(float x, float y, float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

float vec3f::dot(const vec3f &v) const {
	return x * v.x + y * v.y + z * v.z;
}

float vec3f::magnitude(void) const {
	return sqrtf(x * x + y * y + z * z);
}

vec3f &vec3f::normalize(void) {
	float mag = sqrtf(x * x + y * y + z * z);
	if (mag < 1e-6f) {
		x = y = z = 0;
	}
	else {
		float inv = 1.0f / mag;

		x *= inv;
		y *= inv;
		z *= inv;
	}
	return *this;
}

vec3f crossproduct(vec3f v1, vec3f v2) {
	vec3f result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

vec3f vectorsum(vec3f v1, vec3f v2) {
	return vec3f(v1.x + v2.x, v2.y + v2.y, v1.z + v2.z);
}

vec3f vec3f::operator-(void) const {
	return vec3f(-x, -y, -z);
}





class RenderObject {
public:
	int numpoints;
	int numpolys;
	vec3f location;									//do I need this?
	std::vector<vec3f> points;						//point data
	std::vector<std::vector<int>> polys;			//each polygon index list
	Matrix modelmat;

	void Scale(float s);
};

void RenderObject::Scale(float s) {
	for (int i = 0; i < points.size(); i++) {
		points[i].x *= s; points[i].y *= s; points[i].z *= s;
	}
};

RenderObject ParseObj(char *path) {
	FILE *objfile = fopen(path, "r");
	if (objfile == NULL) {
		printf("Impossible to open the file !\n");
		exit(1);
	}

	RenderObject obj;
	char lineHeader[128];
	int res = fscanf(objfile, "%s %d %d", lineHeader, &obj.numpoints, &obj.numpolys);
	printf("%s\n", lineHeader);
	printf("%d\n %d\n \n \n", obj.numpoints, obj.numpolys);
	obj.points = std::vector<vec3f>(obj.numpoints);
	obj.polys = std::vector<std::vector<int>>(obj.numpolys);
	int i = 0;
	for (i = 0; i < obj.numpoints; i++) {
		res = fscanf(objfile, "%f %f %f\n", &obj.points[i].x, &obj.points[i].y, &obj.points[i].z);		//scan the coordinates of a point
		printf("%f	%f	%f\n", obj.points[i].x, obj.points[i].y, obj.points[i].z);						//debug
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
			obj.polys[i].push_back(tempindex);					//append index to polygon vecto
			printf("%d	", obj.polys[i].at(j));
		}
		printf("\n");
	}
	return obj;
}
