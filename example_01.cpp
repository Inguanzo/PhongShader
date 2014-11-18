
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }

using namespace std;

//****************************************************
// Some Classes
//****************************************************

class Viewport;

class Viewport {
  public:
    int w, h; // width and height
};

float dotProduct(float vector1[], float vector2[]){
  float soln = 0;
  for (int i = 0; i < 3; ++i)
  {
    soln += (float)(vector1[i] * vector2[i]);
  }
  return soln;
}

void multiply(float vector1[], float vector2[], float result[]){
  int size = 3;
  for (int i = 0; i < 3; ++i){
    result[i] = vector1[i] * vector2[i];
  }
}
  
void multiply_with_number(float vector1[], float num, float result[]){
  for (int i = 0; i < 3; ++i){
    result[i] = vector1[i] * num;
  }
}

void add(float vector1[], float vector2[], float result[]) {
  for (int i = 0; i < 3; i++) {
    result[i] = vector1[i] + vector2[i];
  }
}
  
void sub(float vector1[], float vector2[], float result[]) {
  for (int i = 0; i < 3; i++) {
    result[i] = vector1[i] - vector2[i];
  }
}
  
void normalize(float vector[]) {
  float mag = sqrt(pow(vector[0], 2) + pow(vector[1], 2) + pow(vector[2], 2));
  vector[0] = vector[0] / mag;
  vector[1] = vector[1] / mag;
  vector[2] = vector[2] / mag;
}
  
void reflect(float vector[], float normal[], float result[]) {
  //R = 2(v dot n)n - v
  normalize(normal);
  float temp;
  temp = dotProduct(vector, normal);
  temp = 2 * temp;
  multiply_with_number(normal, temp, result);
  sub(result, vector, result);
}

// Helper functions for calculating Ra, Rd and Rs
void ambient(float ka[], float lcolor[], float Ra[]) {
  multiply(ka, lcolor, Ra);
}

void diffuse(float kd[], float lcolor[], float ldirection[], float surface_normal[], float Rd[]) {
  float temp;
  temp = dotProduct(ldirection, surface_normal);
  temp = max(temp, 0.0f);
  multiply(kd, lcolor, Rd);
  multiply_with_number(Rd, temp, Rd);
}

void specular(float ks[], float lcolor[], float reflected[], float viewer[], float shininess, float Rs[]) {
  //ks * I * max(rˆ dot vˆ,0)^p
  float temp;
  normalize(reflected);
  normalize(viewer);
  temp = dotProduct(reflected, viewer);
  temp = max(temp, 0.0f);
  temp = pow(temp, shininess);
  multiply(ks, lcolor, Rs);
  multiply_with_number(Rs, temp, Rs);
}

void adding2R(float Ra[], float Rd[], float Rs[], float R[]) {
  R[0] = (R[0] + Ra[0] + Rd[0] + Rs[0]);
  R[1] = (R[1] + Ra[1] + Rd[1] + Rs[1]);
  R[2] = (R[2] + Ra[2] + Rd[2] + Rs[2]);
}

//****************************************************
// Global Variables
//****************************************************
Viewport  viewport;
string input; // user input

float viewer_direction[3] = {0, 0, 1}; //view direction (always z-direction)

float ka [3] = {0, 0, 0};     //ambient color
float kd [3] = {0, 0, 0};     //diffuse color
float ks [3] = {0, 0, 0};     //specular color

float powerCoefficient = 1;

int num_of_dl = 0;      // number of direactional light
int num_of_pl = 0;      // number of position light
float pl_pos [5][3];    // position light
float dl_dir [5][3];    // directional light
float pl_color [5][3];  // position light color
float dl_color [5][3];  // directional light color
  
//****************************************************
// Simple init function
//****************************************************
void initScene(){
  // Tokenize the string and update the terms
  float temp1[3] = {1, 2, 3};
  float temp2[3] = {4, 5, 6};
  float temp3[3] = {0, 0, 0};

  std::string token;
  std::string delimiter = " ";
  int pos = 0;
  input = input + " ";
  while (input.length() > 0) {
    pos = input.find(delimiter);
    token = input.substr(0, pos);
    if (token.compare("-ka") == 0) {
      input.erase(0, pos + 1);
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        ka[k] = val;
        input.erase(0, pos + 1);
      }
    }
    else if (token.compare("-kd") == 0) {
      input.erase(0, pos + 1);
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        kd[k] = val;
        input.erase(0, pos + 1);
      }
    }
    else if (token.compare("-ks") == 0) {
      input.erase(0, pos + 1);
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        ks[k] = val;
        input.erase(0, pos + 1);
      }
    }
    else if (token.compare("-sp") == 0) {
      input.erase(0, pos + 1);
      pos = input.find(delimiter);
      token = input.substr(0, pos);
      token = input.substr(0, pos);
      std::string tokenStr = token;
      float val = ::atof(tokenStr.c_str());
      powerCoefficient = val;
      input.erase(0, pos + 1);
    }
    else if (token.compare("-pl") == 0) {
      input.erase(0, pos + 1);
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        pl_pos[num_of_pl][k] = val;
        input.erase(0, pos + 1);
      }      
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        pl_color[num_of_pl][k] += val;
        input.erase(0, pos + 1);
      }
      num_of_pl++;
    }
    else if (token.compare("-dl") == 0) {
      input.erase(0, pos + 1);
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        dl_dir[num_of_dl][k] = val;
        input.erase(0, pos + 1);
      }
      for(int k = 0; k < 3; k++) {
        pos = input.find(delimiter);
        token = input.substr(0, pos);
        token = input.substr(0, pos);
        std::string tokenStr = token;
        float val = ::atof(tokenStr.c_str());
        dl_color[num_of_dl][k] += val;
        input.erase(0, pos + 1);
      }
      num_of_dl++;
    }
  }
};
    
//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, viewport.w, 0, viewport.h);
}

//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  glColor3f(r, g, b);
  glVertex2f(x + 0.5, y + 0.5);   // The 0.5 is to target pixel
  // centers 
  // Note: Need to check for gap
  // bug on inst machines.
}

//****************************************************
// Exit program with spacebar
//****************************************************
void spacebar(unsigned char bar){
  switch (bar){
    case ' ':

    break;
    default:

    glutPostRedisplay();
    return;
  }
}

//****************************************************
// Draw a filled circle.  
//****************************************************
void circle(float centerX, float centerY, float radius) {
  // Draw inner circle
  glBegin(GL_POINTS);

  // We could eliminate wasted work by only looping over the pixels
  // inside the sphere's radius.  But the example is more clear this
  // way.  In general drawing an object by loopig over the whole
  // screen is wasteful.

  int i,j;  // Pixel indices

  int minI = max(0,(int)floor(centerX-radius));
  int maxI = min(viewport.w-1,(int)ceil(centerX+radius));

  int minJ = max(0,(int)floor(centerY-radius));
  int maxJ = min(viewport.h-1,(int)ceil(centerY+radius));

  // Scale the position vector
  for (int m = 0; m <num_of_pl; m++) {
    multiply_with_number(pl_pos[m], radius, pl_pos[m]);
  }
  
  for (i=0;i<viewport.w;i++) {
    for (j=0;j<viewport.h;j++) {
      // Location of the center of pixel relative to center of sphere
      float x = (i+0.5-centerX);
      float y = (j+0.5-centerY);
      float dist = sqrt(sqr(x) + sqr(y));

      if (dist<=radius) {
        // This is the front-facing Z coordinate
        float z = sqrt(radius*radius-dist*dist);
        
        // Surface Vector
        float surface_position[3] = {x,y,z};
        normalize(surface_position);
        
        // Surface normal
        float center[3] = {0, 0, 0};
        float normal[3];
        sub(surface_position, center, normal);
        normalize(normal);
        
        float R_p[3] = {0, 0, 0};
        float R_d[3] = {0, 0, 0};
        float R[3] = {0, 0, 0};

        // Loop thru all point lights
        for (int n = 0; n < num_of_pl ; n++) {
          // Light direction
          float light_direction[3] = {0, 0, 0};
          float light_color[3] = {0, 0, 0};
          float reflected_ray[3] = {0, 0, 0};
          float Ra[3] = {0, 0, 0};
          float Rd[3] = {0, 0, 0};
          float Rs[3] = {0, 0, 0};
          
          sub(pl_pos[n], surface_position, light_direction);
          normalize(light_direction);

          // Reflected ray
          reflect(light_direction, normal, reflected_ray);
          normalize(reflected_ray);
          
          //light color
          light_color[0] = pl_color[n][0];
          light_color[1] = pl_color[n][1];
          light_color[2] = pl_color[n][2];

          // Ra, Rd, Rs
          ambient(ka, light_color, Ra);
          diffuse(kd, light_color, light_direction, normal, Rd);
          specular(ks, light_color, reflected_ray, viewer_direction, powerCoefficient, Rs);
          adding2R(Ra, Rd, Rs, R);
        }

        //Loop thru all directional lights
        for (int n = 0; n < num_of_dl; n++) {
          float light_direction[3] = {0, 0, 0};
          float light_color[3] = {0, 0, 0};
          float reflected_ray[3] = {0, 0, 0};
          float Ra[3] = {0, 0, 0};
          float Rd[3] = {0, 0, 0};
          float Rs[3] = {0, 0, 0};
          float tempR[3] = {0, 0, 0};

          //Light direction
          light_direction[0] = dl_dir[n][0];
          light_direction[1] = dl_dir[n][1];
          light_direction[2] = dl_dir[n][2];
          multiply_with_number(light_direction, float(-1), light_direction); // Flip the light
          normalize(light_direction);

          //light color
          light_color[0] = dl_color[n][0];
          light_color[1] = dl_color[n][1];
          light_color[2] = dl_color[n][2];

          // Reflected_ray
          reflect(light_direction, normal, reflected_ray);
          normalize(reflected_ray);

          // Ra, Rd, Rs
          ambient(ka, light_color, Ra);
          diffuse(kd, light_color, light_direction, normal, Rd);
          specular(ks, light_color, reflected_ray, viewer_direction, powerCoefficient, Rs);
          adding2R(Ra, Rd, Rs, R);      
        }
        //add(R_d, R_p, R);

        setPixel(i, j, R[0], R[1], R[2]);

        // This is amusing, but it assumes negative color values are treated reasonably.
        //setPixel(i,j, x/radius, y/radius, z/radius );
      }
    }
  }
  glEnd();
}
//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {

  glClear(GL_COLOR_BUFFER_BIT);       // clear the color buffer

  glMatrixMode(GL_MODELVIEW);             // indicate we are specifying camera transformations
  glLoadIdentity();               // make sure transformation is "zero'd"

  // Start drawing
  circle(viewport.w / 2.0 , viewport.h / 2.0 , min(viewport.w, viewport.h) * 0.45);

  glFlush();
  glutSwapBuffers();          // swap buffers (we earlier set double buffer)
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
  cout << "Please enter a command ";
  getline(cin,input);

  //This initializes glut
  glutInit(&argc, argv);

  //This tells glut to use a double-buffered window with red, green, and blue channels 
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

  // Initalize theviewport size
  viewport.w = 400;
  viewport.h = 400;

  //The size and position of the window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0,0);
  glutCreateWindow(argv[0]);

  initScene();              // quick function to set up scene

  glutDisplayFunc(myDisplay);       // function to run when its time to draw something
  glutReshapeFunc(myReshape);       // function to run when the window gets resized

  glutMainLoop();             // infinite loop that will keep drawing and resizing
  // and whatever else
};
