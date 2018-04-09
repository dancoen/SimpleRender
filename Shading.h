#pragma once
#include "stdafx.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <stdio.h>
#include <vector>
#include <array>
#include <list>
#include "math.h"
#include "Model.h"

Flat:
constant across face
-just need normal of face to use for all pixels in polygon

Gauraud:
interpolate COLOR across face 
-need vertex normals

Phong:
-interpolate NORMAL across face to determine color
-need vertex normals

