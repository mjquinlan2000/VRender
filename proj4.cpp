
/*
 NAME: MICHAEL QUINLAN
*/


#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <fstream>
#include <cassert>

#define VPD_MIN     200
#define VPD_DEFAULT 800
#define VPD_MAX     1024

#define MENU_SPRAY 1
#define MENU_SITES 2
#define MENU_RESET 3
#define MENU_MOVE_STOP 4
#define MENU_COLORING 5


#define TWOPI (2.0 * M_PI)

#define PI 3.14159265

class rgb{
public:
    unsigned char r,g,b;
};

class cone{
public:
    rgb color;
    GLfloat x;
    GLfloat y;
};


/* --------------------------------------------- */

GLint wid, /* GLUT window id               */
      vpd = VPD_DEFAULT; /* (square) viewport dimensions */

/* --------------------------------------------- */

GLuint sceneID;      /* display list ID */

/* --------------------------------------------- */


GLint flat_shading = 1;
rgb* picture;
GLint picture_width;
GLint picture_height;
static const GLint iters = 200;
static GLint num_cones = 32;
cone* cones;

/* --------------------------------------------- */

using namespace std;

GLfloat mini(GLfloat x, GLfloat y)
{
    if(x < y)
        return x;
    else
        return y;
}

GLfloat maxi(GLfloat x, GLfloat y)
{
    if(x > y)
        return x;
    else
        return y;
}

void makeUnit(GLfloat a[])
{
    GLfloat length;
    length = sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    for(int i = 0; i < 3; i++)
    {
        a[i] /= length;
    }
    return;
}

void cross(GLfloat a[],GLfloat b[], GLfloat n[])
{
    n[0] = a[1]*b[2] - b[1]*a[2];
    n[1] = b[0]*a[2] - a[0]*b[2];
    n[2] = a[0]*b[1] - b[0]*a[1];
    return;
}

double dot(GLfloat a[], GLfloat b[])
{
	double x = 0;
	for(int i = 0; i < 3; i++)
	{
		x += a[i]*b[i];
	}
	return x;
}

void subtract(GLfloat a[],GLfloat b[], GLfloat r[])
{
	for(int i = 0; i < 3; i++)
	{
		r[i] = a[i] - b[i];
	}
}

void add(GLfloat a[],GLfloat b[], GLfloat r[])
{
	for(int i = 0; i < 3; i++)
	{
		r[i] = a[i] + b[i];
	}
}

GLfloat *make_unit(GLfloat *normal)
{
    GLfloat mag = 0; 
    mag = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
    for(int i = 0; i < 3; i++)
    {
        normal[i] /= mag;
    }
    
    return normal;
}

GLfloat *calc_tex_coord(GLfloat *normal)
{
    GLfloat *pixel;
    normal = make_unit(normal);
    pixel = new GLfloat[2];
    pixel[0] = (1 + normal[0])/2;
    pixel[1] = (1 + normal[1])/2;
    return pixel;
}

GLvoid gen_cones()
{
    cones = new cone[num_cones];
    for(int i = 0; i < num_cones; i++)
    {
//        srand(time(0));
        GLfloat tmpx = (GLfloat)rand()/RAND_MAX;
        GLfloat tmpy = (GLfloat)rand()/RAND_MAX;
        cones[i].x = tmpx*2-1;
        cones[i].y = tmpy*2-1;
        cones[i].color.r = rand()%255 + 1;
        cones[i].color.g = rand()%255 + 1;
        cones[i].color.b = rand()%255 + 1;
//        cones[i].color.r = 255/(i+1);
//        cones[i].color.g = 255/(i+1);
//        cones[i].color.b = 255/(i+1);

        printf("Cone %i: %i %i %i\n", i, cones[i].color.r,cones[i].color.g,cones[i].color.b);
    }
}

GLvoid init_lightsource (  )
{
  GLfloat light_ambient[] = { 1,1,1,1};
  GLfloat light_diffuse[] = { 1,1,1,1};
  GLfloat light_specular[] = {1,1,1,1};
  GLfloat light_position[] = { -2.0, -2.0, -2.0, 0.0 };

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,1.0);
  glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0);
  glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
}

rgb* read_ppm_file(const char* filename)
{
    rgb* pixels;
    ifstream reader;
    reader.open(filename);
    if(!reader)
    {
        printf("%s\n", "NOT A VALID PPM FILE");
        exit(3);
    }
    char c;
    reader >> c;
    assert(c=='P');
    reader >> c;
    assert(c=='6');

    reader >> picture_width >> picture_height;

    int i;
    reader >> i;
    assert(i==255);

    pixels = new rgb[picture_width*picture_height];

    reader.read((char*)pixels, picture_width*picture_height*sizeof(rgb));
    return pixels;
}

GLvoid set_material_properties ( GLfloat r, GLfloat g, GLfloat b )
{
  GLfloat mat_specular[4] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_ambient_and_diffuse[4] = { 0.5, 0.5, 0.5, 1.0 };
  GLfloat mat_shininess[1] = { 0.0 };

  mat_specular[0] = mat_ambient_and_diffuse[0] = r;
  mat_specular[1] = mat_ambient_and_diffuse[1] = g;
  mat_specular[2] = mat_ambient_and_diffuse[2] = b;

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_ambient_and_diffuse);

}

/* --------------------------------------------- */
/*
*/

GLuint draw_cone(GLint cone_num)
{
    GLfloat r = (GLfloat)cones[cone_num].color.r/255;
    GLfloat g = (GLfloat)cones[cone_num].color.g/255;
    GLfloat b = (GLfloat)cones[cone_num].color.b/255;

    set_material_properties(r, g, b);

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(0,0,0);
    glVertex3f(1,0,1);

    for(int i = 1; i < iters; i++)
    {
        glVertex4f(cos(2*PI*i/iters), sin(2*PI*i/iters), 1, 0);
    }

    glVertex3f(1,0,1);


    glEnd();

    glTranslatef(cones[cone_num].x, cones[cone_num].y, 0);
}

GLuint draw_sites()
{
    set_material_properties(0, 0, 0);

    glBegin(GL_POINTS);

    for(GLint i = 0; i < num_cones; i++)
    {
        glVertex3f(cones[i].x, cones[i].y, -0.5);
    }

    glEnd();
}

GLuint draw_cones ( )
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    set_material_properties(1,1,1);

    draw_sites();

    for(GLint i = 0; i < num_cones; i++)
    {
        draw_cone(i);
    }
    
    glPopMatrix();
}


GLuint draw_scene ( )
{
    draw_cones();
    glPopMatrix();
}

/* --------------------------------------------- */

/* redraw the scene */

GLvoid draw()
{
  /* set the projection matrix */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1,1,-1,1,1,-3);


  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* initialize light */
  init_lightsource();

  

  /* ensure we're drawing to the correct GLUT window */
  glutSetWindow(wid);

  /* clear the color buffers */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* DRAW WHAT IS IN THE DISPLAY LIST */
  draw_scene();

  /* flush the pipeline */
  glFlush();

  /* look at our handiwork */
  glutSwapBuffers();

  glutPostRedisplay();
}

/* --------------------------------------------- */

/* handle mouse events */

GLvoid mouse_button(GLint btn, GLint state, GLint mx, GLint my)
{
  switch( btn ) {
    case GLUT_LEFT_BUTTON:
      switch( state ) {
        case GLUT_DOWN:
	  break;
        case GLUT_UP:
	  break;
      }
      break;
    case GLUT_MIDDLE_BUTTON:
      switch( state ) {
        case GLUT_DOWN:
	  break;
        case GLUT_UP:
	  break;
      }
      break;
    case GLUT_RIGHT_BUTTON:
      switch( state ) {
        case GLUT_DOWN:

	  break;
        case GLUT_UP:
	  break;
      }
      break;
  }
}

GLvoid button_motion(GLint mx, GLint my)
{
  return;
}

GLvoid passive_motion(GLint mx, GLint my)
{
  return;
}

/* --------------------------------------------- */

/* handle keyboard events; here, just exit if ESC is hit */

GLvoid keyboard(GLubyte key, GLint x, GLint y)
{
  switch(key) {
    case 27:  /* ESC */
              exit(0);

    default:  break;
  }
}

/* --------------------------------------------- */

GLvoid menu ( int value )
{
  switch(value)
    {
    case MENU_SPRAY:
        break;
    case MENU_SITES:
        break;
    case MENU_RESET:
        break;
    case MENU_MOVE_STOP:
        break;
    case MENU_COLORING:
        break;
    }
}

/* --------------------------------------------- */

/* handle resizing the glut window */

GLvoid reshape(GLint vpw, GLint vph)
{
  glutSetWindow(wid);

  /* maintain a square viewport, not too small, not too big */
  if( vpw < vph ) vpd = vph;
  else            vpd = vpw;

  if( vpd < VPD_MIN ) vpd = VPD_MIN;
  if( vpd > VPD_MAX ) vpd = VPD_MAX;

  glViewport(0, 0, vpd, vpd);
  glutReshapeWindow(vpd, vpd);

  glutPostRedisplay();
}

/* --------------------------------------------- */

GLint init_glut(GLint *argc, char **argv)
{
  GLint id;

  glutInit(argc,argv);

  /* size and placement hints to the window system */
  glutInitWindowSize(picture_width, picture_height);
  glutInitWindowPosition(10,10);

  /* double buffered, RGB color mode */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  /* create a GLUT window (not drawn until glutMainLoop() is entered) */
  id = glutCreateWindow("GLUT Textures");

  /* register callbacks */

  /* window size changes */
  glutReshapeFunc(reshape);

  /* keypress handling when the current window has input focus */
  glutKeyboardFunc(keyboard);

  /* mouse event handling */
  glutMouseFunc(mouse_button);           /* button press/release        */
  glutMotionFunc(button_motion);         /* mouse motion w/ button down */
  glutPassiveMotionFunc(passive_motion); /* mouse motion with button up */

  /* window obscured/revealed event handler */
  glutVisibilityFunc(NULL);

  /* handling of keyboard SHIFT, ALT, CTRL keys */
  glutSpecialFunc(NULL);

  /* what to do when mouse cursor enters/exits the current window */
  glutEntryFunc(NULL);

  /* what to do on each display loop iteration */
  glutDisplayFunc(draw);

  /* create menu */
  GLint menuID = glutCreateMenu(menu);
  glutAddMenuEntry("Spray more points",MENU_SPRAY);
  glutAddMenuEntry("Show/hide sites",MENU_SITES);
  glutAddMenuEntry("Reset",MENU_RESET);
  glutAddMenuEntry("Move/stop points",MENU_MOVE_STOP);
  glutAddMenuEntry("Toggle coloring", MENU_COLORING);
  glutSetMenu(menuID);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  return id;
}

/* --------------------------------------------- */

GLvoid init_opengl()
{
  /* back-face culling on */
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  /* automatically scale normals to unit length after transformation */
  glEnable(GL_NORMALIZE);

  /* clear to BLACK */
  glClearColor(0.0, 0.0, 0.0, 1.0);

  /* Enable depth test  */
  glEnable(GL_DEPTH_TEST);
}

/* --------------------------------------------- */

GLint main(GLint argc, char **argv)
{
    string filename;
    if(argc != 2)
    {
        filename = "input.ppm";
    }else{
        filename = argv[1];
    }

    if(argc > 2)
    {
        printf("%s\n", "INVALID NUMBER OF ARGUMENTS");
        exit(0);
    }

    picture = read_ppm_file(filename.c_str());

    gen_cones();

  /* initialize GLUT: register callbacks, etc */
  wid = init_glut(&argc, argv);

  /* any OpenGL state initialization we need to do */
  init_opengl();

  glutMainLoop();

  return 0;
}

