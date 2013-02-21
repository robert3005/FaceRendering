#ifndef _CG_RENDER_H

#define _CG_RENDER_H

#include <GL/glut.h>
#include <stdlib.h>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <tuple>
#include "trackball.h"

using namespace std;

typedef struct {
  vector<tuple<float,float,float>> vertices;
  vector<vector<int>> polygons;
  vector<vector<int>> verticePolygons;
  vector<pair<float,float>> texture;
} VTKData;

typedef struct {
  int x, y;
  vector<tuple<int,int,int>> pixels;
} PPMImage;

#endif
