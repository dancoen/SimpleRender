#include "stdafx.h"
#include "Model.h"

using namespace std;

vec3f upvector;
vec3f camera;
vec3f pref;
float nearClipz;
float farClipz;
float clipHeight;
RenderObject obj;

void render(void) {
	// clear buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//line color
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// draw lines
	glBegin(GL_LINES);


	//N = (Pref - C) / |Pref - C|
	//U = N x V' / |N x V'|
	//V = U x N


	//mview = 
	//{ Ux, Uy, Uz, 0 },	*     { 1, 0, 0, -Cx },
	//{ Vx, Vy, Vz, 0 },		  { 0, 1, 0, -Cy },
	//{ Nx, Nx, Nz, 0 },		  { 0, 0, 1, -Cz },
	//{ 0, 0, 0, 1 }			  { 0, 0, 0, 1 }


	//line of sight vector N = pref - camera
	vec3f n = vec3f(pref.x - camera.x, pref.y - camera.y, pref.z - camera.z);
	n.normalize();



	//cross product of up, z then normalize
	vec3f u = crossproduct(n, upvector);
	u.normalize();



	//cross product of z, x then normalize
	vec3f v = crossproduct(u, n);
	v.normalize();



	//translate to camera
	Matrix tr = 
	{ { 1, 0, 0, -camera.x },
	{ 0, 1, 0, -camera.y },
	{ 0, 0, 1, -camera.z },
	{ 0, 0, 0, 1 } };

	//rotate for viewing 
	Matrix r = 
	{ { u.x, u.y, u.z, 0 },
	{ v.x, v.y, v.z, 0 },
	{ n.x, n.y, n.z, 0 },
	{ 0, 0, 0, 1 } };

	//modelview matrix (for now, just for one model)
	Matrix ModelView = multMatrix(r, tr);


	for (int i = 0; i < obj.polys.size(); i++) {
		vec3f normal;
		float result = 0;
		vector<vec3f> points;					//temporary list of modelview-transformed points for 


		//find normal of poly for back face culling
		if (obj.polys[i].size() > 2) {
			//find normal of this polygon - need two edges, using points 0-2
			vec3f edge2 = vec3f(obj.points[obj.polys[i][2] - 1].x - obj.points[obj.polys[i][1] - 1].x, obj.points[obj.polys[i][2] - 1].y - obj.points[obj.polys[i][1] - 1].y, obj.points[obj.polys[i][2] - 1].z - obj.points[obj.polys[i][1] - 1].z);
			vec3f edge1 = vec3f(obj.points[obj.polys[i][1] - 1].x - obj.points[obj.polys[i][0] - 1].x, obj.points[obj.polys[i][1] - 1].y - obj.points[obj.polys[i][0] - 1].y, obj.points[obj.polys[i][1] - 1].z - obj.points[obj.polys[i][0] - 1].z);
			//vec3f edge2 = vec3f(obj.points[obj.polys[i][0] - 1].x - obj.points[obj.polys[i][2] - 1].x, obj.points[obj.polys[i][0] - 1].y - obj.points[obj.polys[i][2] - 1].y, obj.points[obj.polys[i][0] - 1].z - obj.points[obj.polys[i][1] - 1].z);
			normal = crossproduct(edge1, edge2);
			normal.normalize();
			result = n.dot(normal);
		}

		//populate list of modelview points
		if (result > 0) {													//if the normal of the poly is facing the camera or perpendicular, draw it - points are 1 indexed so subract one for their location in array
			for (int j = 0; j < obj.polys[i].size(); j++) {
				vec3f p;
				float objx = obj.points[obj.polys[i][j] - 1].x;
				float objy = obj.points[obj.polys[i][j] - 1].y;
				float objz = obj.points[obj.polys[i][j] - 1].z;
				p.x = ModelView[0][0] * objx + ModelView[0][1] * objy + ModelView[0][2] * objz + ModelView[0][3] * 1;			//multiply by modelview matrix (make a function for this)
				p.y = ModelView[1][0] * objx + ModelView[1][1] * objy + ModelView[1][2] * objz + ModelView[1][3] * 1;
				p.z = ModelView[2][0] * objx + ModelView[2][1] * objy + ModelView[2][2] * objz + ModelView[2][3] * 1;
				points.push_back(p);
			}

			//line render
			//use viewing volume to apply proper perspective projection
			int j = 1;
			for (j; j < obj.polys[i].size(); j++) {
				glVertex2f((points[j - 1].x * nearClipz / (points[j - 1].z * clipHeight)), (points[j - 1].y * nearClipz / (points[j - 1].z*clipHeight)));
				glVertex2f((points[j].x * nearClipz / (points[j].z * clipHeight)), (points[j].y * nearClipz / (points[j].z * clipHeight)));
			}
			//draw last line connecting first vertex to last
			glVertex2f((points[j - 1].x * nearClipz / (points[j - 1].z * clipHeight)), (points[j - 1].y * nearClipz / (points[j - 1].z * clipHeight)));
			glVertex2f((points[0].x * nearClipz / (points[0].z * clipHeight)), (points[0].y * nearClipz / (points[0].z * clipHeight)));

			//for hidden surface removal:
			//zs = (farClipz * (1-(nearClipz/points[j-1].z))) / (farClipz - nearClipz);
		}
	}

	glEnd();
	

	// flush
	glFlush();

	// swap buffers
	glutSwapBuffers();
}

void init() {
	//set parameters for viewing 
	obj = ParseObj("house.d.txt");
	camera = { -40., 45, -20 };
	upvector = { 0.,1.,0. };
	pref = { 16.0, 10.0, 54.0 };
	nearClipz = 1.;
	farClipz = 10.;
	clipHeight = 1.;
}


void main(int argc, char *argv[]) {
	init();

	// init glut
	glutInit(&argc, argv);

	// set windows size
	glutInitWindowSize(1920, 1080);

	// set OpenGL display mode 
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	// set window title
	glutCreateWindow("OpenGL Renderer");

	// set rendering function
	glutDisplayFunc(render);

	glutMainLoop();
}
