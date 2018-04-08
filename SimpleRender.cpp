#include "stdafx.h"
#include "Model.h"
#include "math.h"
#include "ScanConvert.h"

using namespace std;

#define PI 3.14


vec3f upvector;
vec3f camera;
vec3f pref;
float nearClipz;
float farClipz;
float clipHeight;
float reswidth;
float resheight;
vector<RenderObject> objectlist;
vector<vector<pixel>> drawbuffer;
EdgeTable ET;

void render(void) {
	// clear buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw points
	glBegin(GL_POINTS);


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
	Matrix ModelView = multMatrix4(r, tr);
	Matrix Perspective = 
	{ { farClipz/clipHeight, 0., 0., 0. },
	{ 0., farClipz / clipHeight, 0., 0. },
	{ 0., 0., farClipz/(farClipz - nearClipz), -(nearClipz * farClipz) / (farClipz - nearClipz) },
	{ 0, 0, 1/*should be z value*/, 0 } };

	for (int j = 0; j < objectlist.size(); j++) {
		RenderObject obj = objectlist[j];
		for (int i = 0; i < obj.polys.size(); i++) {
			vec3f normal;
			float result = 0;
			vector<vec3f> points(obj.polys[i].size());  //temporary list of modelview-transformed points
			vector<vec3f> temppoints(obj.polys[i].size());

			//find normal of poly for back face culling
			if (obj.polys[i].size() > 2) {
				//find normal of this polygon - need two edges, using points 0-2
				vec3f edge1 = vec3f(obj.points[obj.polys[i][1] - 1].x - obj.points[obj.polys[i][0] - 1].x, obj.points[obj.polys[i][1] - 1].y - obj.points[obj.polys[i][0] - 1].y, obj.points[obj.polys[i][1] - 1].z - obj.points[obj.polys[i][0] - 1].z);
				vec3f edge2 = vec3f(obj.points[obj.polys[i][2] - 1].x - obj.points[obj.polys[i][0] - 1].x, obj.points[obj.polys[i][2] - 1].y - obj.points[obj.polys[i][0] - 1].y, obj.points[obj.polys[i][2] - 1].z - obj.points[obj.polys[i][0] - 1].z);
				normal = crossproduct(edge2, edge1);
				normal.normalize();
				result = normal.dot(n);
			}

			//populate list of modelview points
			if (result < 0) {													//if the normal of the poly is facing the camera or perpendicular, draw it - points are 1 indexed so subract one for their location in array
				for (int j = 0; j < obj.polys[i].size(); j++) {
					vec3f p = vec3f(obj.points[obj.polys[i][j] - 1].x, obj.points[obj.polys[i][j] - 1].y, obj.points[obj.polys[i][j] - 1].z);
					p = matrixmultvec(ModelView, p);			//multiply by modelview matrix (make a function for this)
					p = matrixmultvec(Perspective, p);
					//for scan convert
					temppoints[j] = p;
					p.x = p.x / p.z;
					p.x = (reswidth / 2.) + (reswidth * p.x) / 2.;
					p.y = p.y / p.z;
					p.y = (resheight / 2.) + (resheight * p.y) / 2.;
					points[j] = p;
				}
				float y_min = ET.FillTable(&points);
				Polyfill(&drawbuffer, &ET, y_min);
			}
		}
	}

	for (int x = 0; x < drawbuffer.size(); x++) {
		for (int y = 0; y < drawbuffer[x].size(); y++) {
			pixel p = drawbuffer[x][y];
			glColor3f(p.color.x, p.color.y, p.color.z);
			glVertex2f(x, y);
			drawbuffer[x][y].color = { 0,0,0 };
			drawbuffer[x][y].set = false;
		}
	}

	glEnd();
	

	// flush
	glFlush();

	// swap buffers
	glutSwapBuffers();
}

void mousemove(int x, int y) {		// when mouse is moved, detect what quadrant it is (top/bottom 3rd or right or left 3rd of screen) and adjust camera x and y
	if (x < reswidth / 3) {
		camera.x -= .5;
		//mousex = x;
	}
	else if (x > 2 * reswidth / 3) {
		camera.x += .5;
		//mousex = x;
	}
	if (y < resheight / 3) {
		camera.y -= .5;
		//mousey = y;
	}
	else if (y > 2 * resheight / 3) {
		camera.y += .5;
		//mousey = y;
	}
}

void special_key(int key, int x, int y) {								//translate object in x, y ,z
	switch (key) {
	case GLUT_KEY_RIGHT: //right arrow									//x
	{
		camera.x += 5;
		break;
	}
	case GLUT_KEY_LEFT: //left arrow
	{
		camera.x -= 5;
		break;
	}
	break;
	case GLUT_KEY_UP: //left arrow										//y
	{
		camera.y -= 5;
		break;
	}
	case GLUT_KEY_DOWN: //left arrow
	{
		camera.y += 5;
		break;
	}
	case GLUT_KEY_PAGE_UP: //left arrow									//z
	{
		camera.z += 5;
		break;
	}
	case GLUT_KEY_PAGE_DOWN: //left arrow
	{
		camera.z -= 5;
		break;
	}
	default:
		break;
	}
	glutPostRedisplay();
}

void init() {
	//set parameters for viewing 
	reswidth = 600;
	resheight = 600;
	camera = {-5, 30, -25 };
	upvector = { 0.,1.,0. };
	pref = { 0., 0.0, 0.0 };
	nearClipz = 1;
	farClipz = 25;
	clipHeight = 25;
	drawbuffer = vector<vector<pixel>>(reswidth);			//init draw buffer to black (0., 0., 0.)
	ET.Table = vector<list<Edge>>(resheight);
	for (int i = 0; i < reswidth; i++) {
		drawbuffer[i] = vector<pixel>(resheight);
		for (int j = 0; j < resheight; j++) {
			pixel temp;
			temp.color = { 0.,0.,0. };
			drawbuffer[i][j] = temp;
		}
	}
	//RenderObject obj1 = ParseObj("car.d.txt");
	//obj1.Scale(15);
	//obj1.Translate({ -5, 5, 35 });
	//obj1.Rotate({ (float)PI/4., 0, 0 });
	//objectlist.push_back(obj1);
	RenderObject obj2 = ParseObj("house.d.txt");
	//obj2.Scale(5);
	objectlist.push_back(obj2);
}

void update(void) {
	// do something before rendering...
}

void timer(int value) {
	// increase frame index
	//g_frameIndex++;

	update();

	// render
	glutPostRedisplay();

	// reset timer
	glutTimerFunc(16, timer, 0);
}


void main(int argc, char *argv[]) {
	init();
	
	// init glut
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(600, 600);

	glutCreateWindow("Dan Coen's Simple Renderer");

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 600, 0.0, 600);
	
	// set rendering function
	glutDisplayFunc(render);
	glutSpecialFunc(special_key);

	//glutPassiveMotionFunc(mousemove);

	//glutTimerFunc(32, timer, 0);
	//glutMotionFunc(mouseclicked);

	glutMainLoop();
}

