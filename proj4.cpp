
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
    GLfloat rot_angle;
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
cone *cones, *cones_pic;
bool sites = false;
bool is_rotating = false;
bool is_pict = false;
GLfloat t = 0;
bool needs_draw = true;

/* --------------------------------------------- */

using namespace std;

GLvoid gen_cones()
{
    cones = new cone[num_cones];
    cones_pic = new cone[num_cones];
    for(int i = 0; i < num_cones; i++)
    {
        GLfloat tmpx = (GLfloat)rand()/RAND_MAX;
        GLfloat tmpy = (GLfloat)rand()/RAND_MAX;
        cones[i].x = cones_pic[i].x = tmpx*2-1;
        cones[i].y = cones_pic[i].y = tmpy*2-1;
        cones[i].color.r = rand()%255 + 1;
        cones[i].color.g = rand()%255 + 1;
        cones[i].color.b = rand()%255 + 1;
        cones[i].rot_angle = rand()%10 + 1;

        GLfloat scalex = picture_width*(cones_pic[i].x + 1)/2.0;
        GLfloat scaley = picture_height*(-cones_pic[i].y + 1)/2.0;

        GLint ix = (int)(scalex+0.5);
        GLint iy = (int)(scaley+0.5);

        if(ix < 0)
            ix = 0;
        if(ix >= picture_width)
            ix = picture_width-1;
        if(iy < 0)
            iy = 0;
        if(iy >= picture_height)
            iy = picture_height-1;

        rgb color = picture[ix+iy*picture_width];
        cones_pic[i].color.r = color.r;
        cones_pic[i].color.g = color.g;
        cones_pic[i].color.b = color.b;
    }
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

GLuint draw_cone(GLint cone_num)
{
    GLfloat r, g, b;
    if(is_pict){
        b = cones_pic[cone_num].color.r/255.0;
        r = cones_pic[cone_num].color.g/255.0;
        g = cones_pic[cone_num].color.b/255.0;
    }else{
        r = (GLfloat)cones[cone_num].color.r/255;
        g = (GLfloat)cones[cone_num].color.g/255;
        b = (GLfloat)cones[cone_num].color.b/255;
    }

    glPushMatrix();


    if(is_rotating)
    {
        t += .001;
    }
    glRotatef(t*cones[cone_num].rot_angle, 0,0,1);
    glTranslatef(cones[cone_num].x, cones[cone_num].y, 0);

    glColor3f(r, g, b);

    glBegin(GL_TRIANGLE_FAN);

    glVertex3f(0,0,0);
    glVertex3f(1,0,1);

    for(int i = 1; i < iters; i++)
    {
        glVertex4f(cos(2*PI*i/iters), sin(2*PI*i/iters), 1, 0);
    }

    glVertex3f(1,0,1);

    glEnd();

    if(sites){

        glColor3f(0,0,0);

        glBegin(GL_POINTS);

        glVertex3f(0, 0, -0.5);

        glEnd();
    }

    glPopMatrix();
}

GLuint draw_cones ( )
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for(GLint i = 0; i < num_cones; i++)
    {
        draw_cone(i);
    }
}

/* --------------------------------------------- */

/* redraw the scene */

GLvoid draw()
{
    if(is_rotating || needs_draw){
      /* set the projection matrix */
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-1,1,-1,1,1,-3);

      /* initialize light */
    //  init_lightsource();

      /* ensure we're drawing to the correct GLUT window */
      glutSetWindow(wid);

      /* clear the color buffers */
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      /* DRAW WHAT IS IN THE DISPLAY LIST */
      draw_cones();

      /* flush the pipeline */
      glFlush();

      /* look at our handiwork */
      glutSwapBuffers();

      glutPostRedisplay();

      needs_draw = false;
    }
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
        t = 0;
        num_cones *= 2;
        delete [] cones;
        delete [] cones_pic;
        gen_cones();
        needs_draw = true;
        break;
    case MENU_SITES:
        sites = !sites;
        needs_draw = true;
        break;
    case MENU_RESET:
        t = 0;
        num_cones = 32;
        delete [] cones;
        delete [] cones_pic;
        gen_cones();
        needs_draw = true;
        break;
    case MENU_MOVE_STOP:
        is_rotating = !is_rotating;
        needs_draw = true;
        break;
    case MENU_COLORING:
        is_pict = !is_pict;
        needs_draw = true;
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
  glutInitWindowSize(vpd, vpd);
  glutInitWindowPosition(10,10);

  /* double buffered, RGB color mode */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

  /* create a GLUT window (not drawn until glutMainLoop() is entered) */
  id = glutCreateWindow("Voronoi Diagram Mosaics");

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

