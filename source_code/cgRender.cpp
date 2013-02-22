#include <cgRender.h>

VTKData * mesh;
PPMImage * img;
GLuint texture;
vector<tuple<float,float,float>> vertexNormals;
int WindowWidth, WindowHeight;
tuple<float,float,float> objCentre;
bool texturesEnabled = true;
bool lightEnabled = true;

void init() {
  glClearColor (0.0, 0.0, 0.0, 0.0);

  // Setting light and material properties
  const GLfloat ambientLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };
  const GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  const GLfloat specularLight[] = { 0.12f, 0.2f, 0.32f, 1.0f };
  const GLfloat position[] = { 1.0f, 1.0f, 5.0f, 0.3f };
  const GLfloat material = 0.0f;

  glShadeModel (GL_SMOOTH);

  // Enable lighting
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, position);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  ambientLight);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuseLight);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

  // Set material parameters
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  specularLight);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, specularLight);

  // Enable Z-buffering
  glEnable(GL_DEPTH_TEST);

  // allocate a texture name
  glGenTextures( 1, &texture );
  // select our current texture
  glBindTexture( GL_TEXTURE_2D, texture );

  // select modulate to mix texture with color for shading
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // when texture area is small, bilinear filter the closest mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST  );
  // when texture area is large, bilinear filter the first mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // Repeat texture
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  // Enable Texture
  glEnable(GL_TEXTURE_2D);

  // build our texture mipmaps
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);   //Requires GL 1.4. Removed from GL 3.1 and above.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->x, img->y, 0, GL_BGR, GL_UNSIGNED_BYTE, img->pixels);
  delete[] img->pixels;
  delete img;
}

// Calculate average of all points - should approximate centre of object
// Used for setting initial camera
void calculateCentre() {
  float temp1, temp2, temp3;
  temp1 = temp2 = temp3 = 0;
  for(auto vertex : mesh->vertices) {
    temp1 += get<0>(vertex);
    temp2 += get<1>(vertex);
    temp3 += get<2>(vertex);
  }
  temp1 /= mesh->vertices.size();
  temp2 /= mesh->vertices.size();
  temp3 /= mesh->vertices.size();
  objCentre = make_tuple(temp1, temp2, temp3);

}

// Calculate vector from point a to point b
tuple<float,float,float> makeVector(tuple<float,float,float> const &a, tuple<float,float,float> const &b) {
  float x, y, z;
  x = get<0>(b) - get<0>(a);
  y = get<1>(b) - get<1>(a);
  z = get<2>(b) - get<2>(a);
  return make_tuple(x, y, z);
}

// Calculate cross product of two vectors namely a x b
tuple<float,float,float> crossProduct(tuple<float,float,float> const &a, tuple<float,float,float> const &b) {
  float x, y, z;
  x = get<1>(a)*get<2>(b) - get<2>(a)*get<1>(b);
  y = get<2>(a)*get<0>(b) - get<1>(a)*get<2>(b);
  z = get<0>(a)*get<1>(b) - get<1>(a)*get<0>(b);
  return make_tuple(x, y, z);
}

// Return length of the vector
double vectorLength(tuple<float,float,float> const &vec) {
  return sqrt(get<0>(vec) * get<0>(vec) + get<1>(vec) * get<1>(vec) + get<2>(vec) * get<2>(vec));
}

// Normalise given vector returning vector whose length is 1
tuple<float,float,float> normalise(tuple<float,float,float> const &a) {
  float x, y, z;
  double length = vectorLength(a);
  x = get<0>(a)/length;
  y = get<1>(a)/length;
  z = get<2>(a)/length;
  return make_tuple(x, y, z);
}

// Compute normal for a polygon
tuple<float,float,float> normalForPolygon(int polygonIndex) {
  auto polygon = mesh->polygons[polygonIndex];
  vector<tuple<float,float,float>> vertices;
  for(auto vertexIdx : polygon) {
    vertices.push_back(mesh->vertices[vertexIdx]);
  }
  auto Va = makeVector(vertices[0], vertices[1]);
  auto Vb = makeVector(vertices[0], vertices[2]);
  auto normal = crossProduct(Va, Vb);
  return normalise(normal);
}

// Compute average normal at a vertex
// Normal for a given polygon is not stored so it can be calculated more than once
tuple<float,float,float> normalForVertex(int vertexIndex) {
  auto polygons = mesh->verticePolygons[vertexIndex];
  vector<tuple<float,float,float>> polygonNormals;
  for(auto polygonIndex : polygons) {
    polygonNormals.push_back(normalForPolygon(polygonIndex));
  }
  int count = polygonNormals.size();
  auto normalSum = accumulate(polygonNormals.begin(), polygonNormals.end(), make_tuple(0.0f,0.0f,0.0f),
   [] (tuple<float,float,float> const &init, tuple<float,float,float> const &a) -> tuple<float,float,float> const {
    return make_tuple(get<0>(init) + get<0>(a),get<1>(init) + get<1>(a),get<2>(init) + get<2>(a));
  });
  return make_tuple(get<0>(normalSum)/count, get<1>(normalSum)/count, get<2>(normalSum)/count);
}

// Precompute normals for all vertices
void calculateNormals(int maxVertex) {
  for(int i = 0; i < maxVertex; i++) {
    vertexNormals.push_back(normalForVertex(i));
  }
}

// Display OpenGL primitives
void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  float v1, v2, v3;
  glPushMatrix();
  tbMatrix();
  for (auto polygon : mesh->polygons) {
    glBegin(GL_POLYGON);
    for (auto vertexIndex : polygon) {
      // Define texture coordinates of vertex
      tie(v1, v2) = mesh->texture[vertexIndex];
      glTexCoord2f(v1, v2);
      // Define normal of vertex
      tie(v1, v2, v3) = vertexNormals[vertexIndex];
      glNormal3f(v1, v2, v3);
      // Define coordinates of vertex
      tie(v1, v2, v3) = mesh->vertices[vertexIndex];
      glVertex3f(v1, v2, v3);
    }
    glEnd();
  }
  glPopMatrix();
  glutSwapBuffers();
}

// Set perspective and camera
void reshape (int w, int h) {
  tbReshape(w, h);

  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  WindowWidth = w;
  WindowHeight = h;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30, 1, 0.1, 2);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  float centerx, centery, centerz;
  tie(centerx, centery, centerz) = objCentre;
  gluLookAt(0.5, centery, -0.05, centerx, centery, centerz, 0, 1, 0);
}

// Switch ligth with 'l' and textures with 't'
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27: // ESC
    exit(0);
    break;
  case 't':
    if(!texturesEnabled) {
      glEnable(GL_TEXTURE_2D);
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    glutPostRedisplay();
    texturesEnabled = !texturesEnabled;
    break;
  case 'l':
    if(!lightEnabled) {
      glEnable (GL_LIGHTING);
    } else {
      glDisable(GL_LIGHTING);
    }
    glutPostRedisplay();
    lightEnabled = !lightEnabled;
    break;
  }
}

// Add rotation with mouse drag
void mouse(int button, int state, int x, int y) {
  tbMouse(button, state, x, y);
}

// Add rotation with mouse drag
void motion(int x, int y) {
  tbMotion(x, y);
}

// Parse VTK file and put results in VTKData object
VTKData * readVTKFile(const char * filename) {
  ifstream inFile(filename);
  string buffer;
  int readCount;
  int sideCount;
  int temp;

  // Initialise return structure holding object vertices and polygons
  VTKData * mesh = new VTKData;

  if(inFile.is_open()) {

    // skip the header
    while(buffer != "POINTS") {
      inFile >> buffer;
    }

    // read number of vertices
    inFile >> readCount;
    inFile >> buffer;
    float temp1, temp2, temp3;
    for(int i = 0; i < readCount; i++) {
      inFile >> temp1 >> temp2 >> temp3;
      mesh->vertices.push_back(make_tuple(temp1, temp2, temp3));
    }

    mesh->verticePolygons.assign( readCount, vector<int>() );

    while(buffer != "POLYGONS") {
      inFile >> buffer;
    }

    // Read polygon definition
    inFile >> readCount;
    inFile >> buffer;
    vector<int> polygon;
    for(int i = 0; i < readCount; i++) {
      inFile >> sideCount;
      for(int j = 0; j < sideCount; j++) {
        inFile >> temp;
        polygon.push_back(temp);
        // Create reverse mapping from vertex to polygon
        mesh->verticePolygons[temp].push_back(i);
      }
      mesh->polygons.push_back(polygon);
      polygon.clear();
    }

    while(buffer != "POINT_DATA") {
      inFile >> buffer;
    }

    inFile >> readCount;
    inFile >> buffer;
    inFile >> buffer;
    inFile >> buffer;
    inFile >> buffer;
    for(int i = 0; i < readCount; i++) {
      inFile >> temp1 >> temp2;
      mesh->texture.push_back(make_pair(temp1,temp2));
    }
  }

  inFile.close();
  return mesh;
}

// Parse ppm texture file for texture colours
PPMImage * readPPM(const char *filename) {
  ifstream ppmFile(filename, ios::in | ios::binary);
  PPMImage * img = new PPMImage();
  unsigned char buffer, buffer2;
  string line;

  if(ppmFile.is_open()) {
    // Read header
    ppmFile >> buffer >> buffer2;
    if(buffer != 'P' || buffer2 != '6') {
      cout << "Wrong file type" << endl;
    }

    // Consume comments
    while(getline(ppmFile, line) && line[0] == '#') {}
    ppmFile >> img->x >> img->y;

    // Check color depth
    int color_depth;
    ppmFile >> color_depth;
    if(color_depth != 255) {
      cout << "Wrong color depth" << endl;
    }
    ppmFile >> buffer;

    img->pixels = new char[img->x*img->y*3];
    ppmFile.read(img->pixels, img->x*img->y*3);
  }
  ppmFile.close();
  return img;
}

int main(int argc, char** argv) {
  if(argc != 3) {
    cout << "USAGE: " << argv[0] << " FILE" << " FILE" << endl;
    return 1;
  }
  // Initialize graphics window
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  mesh = readVTKFile(argv[1]);
  img = readPPM(argv[2]);

  calculateNormals(mesh->vertices.size());
  calculateCentre();

  glutInitWindowSize (512, 512);
  glutInitWindowPosition (0, 0);
  glutCreateWindow (argv[0]);

  // Initialize OpenGL
  init();

  // Initialize callback functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  tbInit(GLUT_LEFT_BUTTON);
  tbAnimate(GL_TRUE);

  // Start rendering
  glutMainLoop();
}