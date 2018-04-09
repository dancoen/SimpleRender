#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>
#include <array>
#include <list>

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

Matrix multMatrix4(Matrix a, Matrix b) {			//only 4x4 for now
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

vec3f matrixmultvec(Matrix a, vec3f b) {
	vec3f result;
	result.x = a[0][0] * b.x + a[0][1] * b.y + a[0][2] * b.z + a[0][3] * 1;			//multiply by modelview matrix (make a function for this)
	result.y = a[1][0] * b.x + a[1][1] * b.y + a[1][2] * b.z + a[1][3] * 1;
	result.z = a[2][0] * b.x + a[2][1] * b.y + a[2][2] * b.z + a[2][3] * 1;
	return result;
}

struct pixel {
	vec3f color;
	float z;
	bool set = false;
};