#include <cgRender.h>

VTKData * mesh;
PPMImage * img;
int WindowWidth, WindowHeight;

void init() {
  glClearColor (0.0, 0.0, 0.0, 0.0);
  cout << "init" << endl;

  const GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
  const GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.8, 1.0f };
  const GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
  const GLfloat position[] = { -1.5f, 1.0f, -4.0f, 1.0f };
  const GLfloat material = 64.0f;

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
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  cout << "display" << endl;

  float v1, v2, v3;
  glPushMatrix();
  tbMatrix();
  for (auto polygon : mesh->polygons) {
    glBegin(GL_POLYGON);
    for (auto verticeIndex : polygon) {
      tie(v1, v2, v3) = mesh->vertices[verticeIndex];
      // Define texture coordinates of vertex
      // glTexCoord2f(...);
      // Define normal of vertex
      // glNormal3f(...);
      // Define coordinates of vertex
      glVertex3f(v1, v2, v3);
    }
    glEnd();
  }
  glPopMatrix();
  glutSwapBuffers();
}

void reshape (int w, int h) {
  cout << "reshape" << endl;

  tbReshape(w, h);

  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  WindowWidth = w;
  WindowHeight = h;
  /*
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fovy, aspect, near, far);
  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
  */
}

void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 27: // ESC
    exit(0);
    break;
  }
}

void mouse(int button, int state, int x, int y) {
  tbMouse(button, state, x, y);
}

void motion(int x, int y) {
  tbMotion(x, y);
}

VTKData * readVTKFile(const char * filename) {
  ifstream inFile(filename);
  string buffer;
  int readCount;
  int sideCount;
  int temp;

  VTKData * mesh = new VTKData;

  if(inFile.is_open()) {

    while(buffer != "POINTS") {
      inFile >> buffer;
    }

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

    inFile >> readCount;
    inFile >> buffer;
    vector<int> polygon;
    for(int i = 0; i < readCount; i++) {
      inFile >> sideCount;
      for(int j = 0; j < sideCount; j++) {
        inFile >> temp;
        polygon.push_back(temp);
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

PPMImage * readPPM(const char *filename) {
  ifstream ppmFile(filename, ios::in | ios::binary);
  PPMImage * img = new PPMImage();
  unsigned char buffer, buffer2, red, green, blue;
  string line;

  if(ppmFile.is_open()) {
    ppmFile >> buffer >> buffer2;
    if(buffer != 'P' || buffer2 != '6') {
      cout << "Wrong file type" << endl;
    }

    while(getline(ppmFile, line) && line[0] == '#') {}
    ppmFile >> img->x >> img->y;

    int color_depth;
    ppmFile >> color_depth;
    if(color_depth != 255) {
      cout << "Wrong color depth" << endl;
    }

    for(int i = 0; i < img->x*img->y; i++) {
      ppmFile >> red >> green >> blue;
      img->pixels.push_back(make_tuple(red, green, blue));
    }
  }

  ppmFile.close();
  return img;
}

int main(int argc, char** argv) {
  // Initialize graphics window
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  mesh = readVTKFile("../data/face.vtk");
  img = readPPM("../data/face.ppm");

  glutInitWindowSize (256, 256);
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