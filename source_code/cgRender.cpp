#include <cgRender.h>

typedef struct {
  vector<tuple<float,float,float>> vertices;
  vector<vector<int>> polygons;
  vector<vector<int>> verticePolygons;
  vector<pair<float,float>> texture;
} VTKData;

void init() {
  glClearColor (0.0, 0.0, 0.0, 0.0);
  cout << "init" << endl;

  /*
  glShadeModel (GL_SMOOTH);

  // Enable lighting
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
  glLightfv(GL_LIGHT0, GL_AMBIENT,  LightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  LightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);

  // Set material parameters
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  MaterialSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, MaterialShininess);

  // Enable Z-buffering
  glEnable(GL_DEPTH_TEST);
  */
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  cout << "display" << endl;

  /*
  for (all polygons)
    glBegin(GL_POLYGON);
    for (all vertices of polygon)
      // Define texture coordinates of vertex
      glTexCoord2f(...);
      // Define normal of vertex
      glNormal3f(...);
      // Define coordinates of vertex
      glVertex3f(...);
    }
    glEnd();
  }
  glFlush ();
  //  or, if double buffering is used,
  //  glutSwapBuffers();
  */
}

void reshape (int w, int h) {
  cout << "reshape" << endl;

  glViewport (0, 0, (GLsizei) w, (GLsizei) h);
  /*
  glMatrixMode (GL_PROJECTION);
  glLoadIdentityd ();
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
    cout << buffer << endl;
    for(int i = 0; i < readCount; i++) {
      inFile >> temp1 >> temp2;
      mesh->texture.push_back(make_pair(temp1,temp2));
    }
  }

  inFile.close();
  return mesh;
}

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;

#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

PPMImage *readPPM(const char *filename) {
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
      //open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp) {
      fprintf(stderr, "Unable to open file '%s'\n", filename);
      exit(1);
    }

       //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
      perror(filename);
      exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
    while (getc(fp) != '\n') {}
         c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
         fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
         exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') {}
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }

    fclose(fp);
    return img;
}

int main(int argc, char** argv) {
  // Initialize graphics window
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  //  Or, can use double buffering
  //  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  PPMImage * img = readPPM("../data/face.ppm");
  cout << img->x << " | " << img->y << endl;

  /*PPMPixel * pixelData = img->data;
  for(int i = 0; i < img->x*img->y; i++) {
    cout << "R: " << (int) pixelData->red << " G: " << (int) pixelData->green << " B: " << (int) pixelData->blue << endl;
    pixelData++;
  }*/
  readVTKFile("../data/face.vtk");

  glutInitWindowSize (256, 256);
  glutInitWindowPosition (0, 0);
  glutCreateWindow (argv[0]);

  // Initialize OpenGL
  init();

  // Initialize callback functions
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);

  // Start rendering
  glutMainLoop();
}
