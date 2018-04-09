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



class Edge {
public:
	vec3f coordinates;		//endpoint coordinates
	float xinc, zinc;		//increment of x and z values to move to onext scanline   - - - - xinc = delta x / delta y,    zinc = delta z / delta y
	float yupper;				//y value of last point in edge (the top)
	float upperN;				//vertex normal of higher point in edge
	float lowerN;				//vertex normal of lower point in edge
	float ks, kd, ka, alpha;	//specular, diffuse, ambient, shininess(alpha)
	vec3f color;				

	Edge() {};
	Edge(vec3f coordinates, float xinc, float zinc, float yupper) {
		this->coordinates = coordinates;
		this->xinc = xinc;
		this->zinc = zinc;
		this->yupper = yupper;
	};
};

class EdgeTable {
public:
	vector<list<Edge>> Table;
	//float FillTable(vector<vec3f> *points);
	float FillTable(Face *face);
	
};


/*float EdgeTable::FillTable(vector<vec3f> *points) {					//do this for pixel-point array of each polygon - returns ymin of edges
	float y_min = 600;
	bool horizontal = true;
	vec3f coordinates;
	float y_upper = 0.;
	float y_lower = 0.;				//where to store the edge in list
	float xinc = 0;
	float zinc = 0;
	for (int j = 0; j < points->size(); j++) {
		vec3f p1 = (*points)[j];
		vec3f p2 = (*points)[(j + 1) % points->size()];
		horizontal = (int(p1.y) == int(p2.y));
		if (p1.y > p2.y) {
			coordinates = { p2.x, p2.y, p2.z };
			y_upper = p1.y;
			y_lower = p2.y;
			xinc = (p1.x - p2.x) / (p1.y - p2.y);
			zinc = (p1.z - p2.z) / (p1.y - p2.y);
		}
		else if (p2.y > p1.y) {
			coordinates = { p1.x, p1.y, p1.z };
			y_upper = p2.y;
			y_lower = p1.y;
			xinc = (p2.x - p1.x) / (p2.y - p1.y);
			zinc = (p2.z - p1.z) / (p2.y - p1.y);
		}
		if (!horizontal) {
			if (y_lower < y_min) y_min = y_lower;
			Edge e = Edge(coordinates, xinc, zinc, y_upper);
			if (y_lower > 0. && y_lower < 600) {
				Table[int(y_lower)].push_back(e);
			}
		}
	}
	return (y_min);
}
*/

float EdgeTable::FillTable(Face *face) {					//do this for pixel-point array of each polygon - returns ymin of edges
	float y_min = 600;
	bool horizontal = true;
	vec3f coordinates;
	float y_upper = 0.;
	float y_lower = 0.;				//where to store the edge in list
	float xinc = 0;
	float zinc = 0;
	for (int j = 0; j < (*face).screencords.size(); j++) {
		vec3f p1 = (*face).screencords[j];
		vec3f p2 = (*face).screencords[(j + 1) % (*face).screencords.size()];
		horizontal = (int(p1.y) == int(p2.y));
		if (p1.y > p2.y) {
			coordinates = { p2.x, p2.y, p2.z };
			y_upper = p1.y;
			y_lower = p2.y;
			xinc = (p1.x - p2.x) / (p1.y - p2.y);
			zinc = (p1.z - p2.z) / (p1.y - p2.y);
		}
		else if (p2.y > p1.y) {
			coordinates = { p1.x, p1.y, p1.z };
			y_upper = p2.y;
			y_lower = p1.y;
			xinc = (p2.x - p1.x) / (p2.y - p1.y);
			zinc = (p2.z - p1.z) / (p2.y - p1.y);
		}
		if (!horizontal) {
			if (y_lower < y_min) y_min = y_lower;
			Edge e = Edge(coordinates, xinc, zinc, y_upper);
			e.color = face->color;
			//printf("color is %f %f %f\n", e.color.x, e.color.y, e.color.z);
			if (y_lower > 0. && y_lower < 600) {
				Table[int(y_lower)].push_back(e);
			}
		}
	}
	return (y_min);
}

bool comparex(Edge *a, Edge *b) {
	return(a->coordinates.x < b->coordinates.x);
}

void Polyfill(vector<vector<pixel>> *drawbuffer, EdgeTable *ET, int ymin) {					//currently - inserts in order the edges starting on each scan line into the active edge table.
	//vec3f color = { ((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)), ((float)rand() / (RAND_MAX)) };
	list<Edge *> AET;
	for (int currentline = 0; currentline < ET->Table.size(); currentline++) {		//for each scanline

		std::list<Edge *>::iterator AEit1 = AET.begin();
		std::list<Edge *>::iterator AEit2 = AET.begin();
		if (AEit2 != AET.end()) {
			AEit2++;
		}

		for (AEit2; AEit2 != AET.end(); AEit2++) {
			int i = int((*AEit1)->coordinates.x);
			for (i; (i < ((*AEit2)->coordinates.x) && i >= 0 && i < ET->Table.size()); i++) {
				if ((*AEit1)->coordinates.z > 0) {
					if ((*AEit1)->coordinates.z < (*drawbuffer)[i][currentline].z || (*drawbuffer)[i][currentline].set == false) {
						(*drawbuffer)[i][currentline].color = (*AEit1)->color;
						(*drawbuffer)[i][currentline].z = (*AEit1)->coordinates.z;
						(*drawbuffer)[i][currentline].set = true;
					}
				}
			}
			AEit1++;
		}

		for (std::list<Edge *>::iterator AEit = AET.begin(); AEit != AET.end(); AEit++) {		//draw according to active edges
			if (int((*AEit)->yupper) <= currentline) {												//remove this edge from from the AE table
																							//printf("removing Edge with yupper = %f\n", (*AEit)->yupper);
				AET.remove(*AEit);
				break;
			}
			else {
				(*AEit)->coordinates.x += (*AEit)->xinc;
				(*AEit)->coordinates.z += (*AEit)->zinc;
			}
		}

		for (std::list<Edge>::iterator ETit = (*ET).Table[currentline].begin(); ETit != (*ET).Table[currentline].end(); ETit++) {		//iterate through list of edges starting on this scanline and add to active edge table sorted by x
			bool inserted = false;
			std::list<Edge *>::iterator AETit;
			for (AETit = AET.begin(); AETit != AET.end(); AETit++) {
				if (ETit->coordinates.x < (*AETit)->coordinates.x) {
					//printf("inserting Edge. x = %f y = %f z = %f. xinc = %f zinc = %f and yupper is %f. Inserted at scanline %d\n\n", ETit->coordinates.x, ETit->coordinates.y, ETit->coordinates.z, ETit->xinc, ETit->zinc, ETit->yupper, currentline);
					AET.insert(AETit, &*ETit);
					inserted = true;
					break;
				}
			}
			if (!inserted) {
				//printf("pushing back Edge. x = %f y = %f z = %f. xinc = %f zinc = %f and yupper is %f\n\n", ETit->coordinates.x, ETit->coordinates.y, ETit->coordinates.z, ETit->xinc, ETit->zinc, ETit->yupper, currentline);
				AET.push_back(&*ETit);
			}
		}

		AET.sort(comparex);
	}
	for (int i = 0; i < ET->Table.size(); i++) {
		(*ET).Table[i].clear();
	}
}