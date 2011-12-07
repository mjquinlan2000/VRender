
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

#define ZOOM_IN 1
#define ZOOM_OUT 2
#define MENU_FASTER 3
#define MENU_SLOWER 4
#define MENU_STOP_RUN 5
#define MENU_3D_TEXTURE 6
#define MENU_DOUGHNUT 7
#define MENU_ENV_MAPPING 8

#define ENV_MAPPING 1
#define THREED_MAPPING 2
#define DOUGHNUT_MAPPING 3


#define TWOPI (2.0 * M_PI)

#define PI 3.14159265

struct triangle{
    int a[3];
};

struct vertex{
    GLfloat p[3];
};

class rgb{
public:
    unsigned char r,g,b;
};


/* --------------------------------------------- */

GLint wid, /* GLUT window id               */
      vpd = VPD_DEFAULT; /* (square) viewport dimensions */

/* --------------------------------------------- */

GLuint sceneID;      /* display list ID */

/* --------------------------------------------- */


GLint flat_shading = 1;

int num_triangles, num_vertices;
triangle *triangle_table;
vertex *vertex_table, *shading_table, *normal_table;
GLfloat center[3], minimum[3], maximum[3], dscale = 0, field_of_view(15), dist;
GLfloat R[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}, R0[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1}, cvec[] = {1,1,1};
GLfloat rot_angle = .05, rotvec[] = {-1,1,1};
GLint current_mode = 0;

rgb *env_pixels, *texture_pixels, *tilable_pixels;
GLint *res_env, *res_tilable;
bool is_env = true;

GLfloat max_dimen = 0;

bool is_rotating = true;

/* --------------------------------------------- */

using namespace std;

GLvoid calculate_translated(GLfloat vec[])
{
    GLfloat x,y,z;
    x = 2*vec[0]/(vpd-1)-1;
    y = 1-2*vec[1]/(vpd-1);

    

    if(1-x*x-y*y >= 0)
    {
        vec[0] = x;
        vec[1] = y;
        vec[2] = sqrt(1-x*x-y*y);
    }else
    {
        vec[0] = x/sqrt(x*x+y*y);
        vec[1] = y/sqrt(x*x+y*y);
        vec[2] = 0;
    }
}

GLvoid set_distance()
{
    dist = 1/tan(field_of_view*PI/360.0 );

}

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
//    pixel[0] = normal[0];
//    pixel[1] = normal[1];
    return pixel;
}

GLvoid init_lightsource (  )
{
  GLfloat light_ambient[] = { .1, .1, .1, 1.0 };
  GLfloat light_diffuse[] = { .9, .9, .9, 1.0 };
  GLfloat light_specular[] = { 0, 0, 0, 1.0 };
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

GLvoid calculate_torus(GLfloat psi, GLfloat phi, GLfloat point[])
{
    GLfloat r = .25*max_dimen;
    GLfloat R = .9*max_dimen;

    point[0] = (R + r*cos(psi))*cos(phi);
    point[1] = (R + r*cos(psi))*sin(phi);
    point[2] = r*sin(psi);
}

GLvoid read_from_file(const char* filename)
{
    ifstream reader;
    reader.open(filename);
    if(!reader)
    {
        printf("%s IS NOT A VALID FILE\n", filename);
        exit(0);
    }

    reader >> num_triangles;
    reader >> num_vertices;
    bool first = true;

    triangle_table = new triangle[num_triangles];
    vertex_table = new vertex[num_vertices];
    shading_table = new vertex[num_vertices];
    normal_table = new vertex[num_triangles];

    for(int i = 0; i < num_triangles; i++)
    {
        reader >> triangle_table[i].a[0];
        reader >> triangle_table[i].a[1];
        reader >> triangle_table[i].a[2];
    }

    for(int i = 0; i < num_vertices; i++)
    {
        reader >> vertex_table[i].p[0];
        reader >> vertex_table[i].p[1];
        reader >> vertex_table[i].p[2];

        shading_table[i].p[0] = 0.0;
        shading_table[i].p[1] = 0.0;
        shading_table[i].p[2] = 0.0;

        if(first)
        {
            minimum[0] = maximum[0] = vertex_table[i].p[0];
            minimum[1] = maximum[1] = vertex_table[i].p[1];
            minimum[2] = maximum[2] = vertex_table[i].p[2];
            first = false;
        }else
        {
            minimum[0] = mini(minimum[0], vertex_table[i].p[0]);
            minimum[1] = mini(minimum[1], vertex_table[i].p[1]);
            minimum[2] = mini(minimum[2], vertex_table[i].p[2]);

            maximum[0] = maxi(maximum[0], vertex_table[i].p[0]);
            maximum[1] = maxi(maximum[1], vertex_table[i].p[1]);
            maximum[2] = maxi(maximum[2], vertex_table[i].p[2]);
        }
    }

    for(int i = 0; i < num_triangles; i++)
    {
          GLfloat a[3], b[3];

          subtract(vertex_table[triangle_table[i].a[1]].p, vertex_table[triangle_table[i].a[0]].p, a);
          subtract(vertex_table[triangle_table[i].a[2]].p, vertex_table[triangle_table[i].a[0]].p, b);
          cross(a, b, normal_table[i].p);

          add(shading_table[triangle_table[i].a[0]].p, normal_table[i].p, shading_table[triangle_table[i].a[0]].p);

          add(shading_table[triangle_table[i].a[1]].p, normal_table[i].p, shading_table[triangle_table[i].a[1]].p);

          add(shading_table[triangle_table[i].a[2]].p, normal_table[i].p, shading_table[triangle_table[i].a[2]].p);
    }

    for(int i = 0; i < 3; i++)
    {
        center[i] = (maximum[i]+minimum[i])/2.0;
    }

    dscale = maximum[0] - minimum[0];
    dscale = maxi(dscale, maximum[1] - minimum[1]);
    dscale = maxi(dscale, maximum[2] - minimum[2]);
    dscale = 2.0/dscale;
}

rgb* read_ppm_file(const char* filename)
{
    rgb* pixels;
    int res_x, res_y;
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

    reader >> res_x >> res_y;

    if(is_env)
    {
        is_env = false;
        res_env = new GLint[2];
        res_env[0] = res_x;
        res_env[1] = res_y;
    }else{
        res_tilable = new GLint[2];
        res_tilable[0] = res_x;
        res_tilable[1] = res_y;
    }

    int i;
    reader >> i;
    assert(i==255);

    pixels = new rgb[res_x*res_y];

    reader.read((char*)pixels, res_x*res_y*sizeof(rgb));
    return pixels;
}

rgb* read_texture_file(const char* filename)
{
    ifstream reader;
    rgb *pixels;
    reader.open(filename, ios::binary);
    if(!reader)
    {
        printf("%s\n", "3D TEXTURE FILE NOT FOUND");
    }

    pixels = new rgb[128*128*128];
    reader.read((char*)pixels, 128*128*128*sizeof(rgb));

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

GLuint draw_triangles ( )
{
    if(current_mode == ENV_MAPPING)
    {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        
        glTranslatef(.5,.5,.5);
        glScalef(.5,.5,.5);
        glMultMatrixf(R);

        glMatrixMode(GL_MODELVIEW);
        GLfloat *n;

        glBegin(GL_TRIANGLES);

        for(int i = 0; i < num_triangles; i++)
        {
            glNormal3fv(shading_table[triangle_table[i].a[2]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[2]].p);
            n = make_unit(shading_table[triangle_table[i].a[2]].p);
            glTexCoord3fv(n);
            glVertex3fv(vertex_table[triangle_table[i].a[2]].p);

            glNormal3fv(shading_table[triangle_table[i].a[1]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[1]].p);
            n = make_unit(shading_table[triangle_table[i].a[1]].p);
            glTexCoord3fv(n);
            glVertex3fv(vertex_table[triangle_table[i].a[1]].p);

            glNormal3fv(shading_table[triangle_table[i].a[0]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[0]].p);
            n = make_unit(shading_table[triangle_table[i].a[0]].p);
            glTexCoord3fv(n);
            glVertex3fv(vertex_table[triangle_table[i].a[0]].p);
        }

        glEnd();
    }


    if(current_mode == THREED_MAPPING || current_mode == 0)
    {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        glScalef(1/(max_dimen),1/(max_dimen),1/(max_dimen));
        glTranslatef(-minimum[0], -minimum[1], -minimum[2]);
                
        glMatrixMode(GL_MODELVIEW);

        glBegin(GL_TRIANGLES);

        for(int i = 0; i < num_triangles; i++)
        {

            glNormal3fv(shading_table[triangle_table[i].a[2]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[2]].p);
            glVertex3fv(vertex_table[triangle_table[i].a[2]].p);

            glNormal3fv(shading_table[triangle_table[i].a[1]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[1]].p);
            glVertex3fv(vertex_table[triangle_table[i].a[1]].p);
            
            glNormal3fv(shading_table[triangle_table[i].a[0]].p);
            glTexCoord3fv(vertex_table[triangle_table[i].a[0]].p);
            glVertex3fv(vertex_table[triangle_table[i].a[0]].p);

        }

        glEnd();
    }

    if(current_mode == DOUGHNUT_MAPPING)
    {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);

        GLfloat psi, phi, increment;
        GLfloat pi = acos(-1);
        increment = 2*pi/50;
        GLfloat point1[3], point2[3], point3[3], normal1[3], normal2[3], normal3[3], a[3], b[3];

        glBegin(GL_TRIANGLES);

        for(phi = 0; phi < 2*pi; phi += increment)
        {
            for(psi = 0; psi < 2*pi; psi += increment)
            {
                calculate_torus(psi, phi, point1);
                calculate_torus(psi + increment, phi, point2);
                calculate_torus(psi + increment, phi + increment, point3);

                subtract(point3, point1, a);
                subtract(point2, point1, b);

                cross(b, a, normal1);

                glNormal3fv(normal1);

                glTexCoord2f((psi+increment)/(2*pi), phi/(2*pi));
                glVertex3fv(point2);

                glTexCoord2f(psi/(2*pi), phi/(2*pi));
                glVertex3fv(point1);

                glTexCoord2f((psi+increment)/(2*pi), (phi + increment)/(2*pi));
                glVertex3fv(point3);

                calculate_torus(psi, phi + increment, point2);

                subtract(point3, point1, a);
                subtract(point2, point1, b);

                cross(a, b, normal1);

                glNormal3fv(normal1);

                glTexCoord2f(psi/(2*pi), (phi + increment)/(2*pi));
                glVertex3fv(point2);

                glTexCoord2f((psi+increment)/(2*pi), (phi + increment)/(2*pi));
                glVertex3fv(point3);

                glTexCoord2f(psi/(2*pi), phi/(2*pi));
                glVertex3fv(point1);
                
            }
        }

        glEnd();
    }
}


GLuint draw_scene ( )
{
    set_material_properties(cvec[0],cvec[1],cvec[2]);
    glPushMatrix();
    glScalef(dscale, dscale, dscale);
    glTranslatef(-center[0], -center[1], -center[2]);
    
    draw_triangles();
    glPopMatrix();
}

/* --------------------------------------------- */

/* redraw the scene */

GLvoid draw()
{
  /* set the projection matrix */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluPerspective(field_of_view,1,dist-1,dist + 3);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* initialize light */
  init_lightsource();

  if(is_rotating){
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(rot_angle, rotvec[0], rotvec[1], rotvec[2]);
    glMultMatrixf(R);
    glGetFloatv(GL_MODELVIEW_MATRIX,R);
    glPopMatrix();
  }

  glTranslatef(0,0, -(dist+1));
  glMultMatrixf(R);

  

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
    case ZOOM_IN:
        field_of_view--;
        break;
    case ZOOM_OUT:
        field_of_view++;
        break;
    case MENU_FASTER:
        rot_angle *= 1.3;
        is_rotating = true;
        break;
    case MENU_SLOWER:
        rot_angle *= 0.7;
        is_rotating = true;
        break;
    case MENU_STOP_RUN:
        if(is_rotating)
        {
            is_rotating = false;
        }else
        {
            is_rotating = true;
        }
        break;
    case MENU_3D_TEXTURE:
        current_mode = THREED_MAPPING;
        glEnable(GL_TEXTURE_3D);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, 128, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_pixels);
        break;
    case MENU_ENV_MAPPING:
        current_mode = ENV_MAPPING;
        glDisable(GL_TEXTURE_3D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_env[0], res_env[1], 0, GL_RGB, GL_UNSIGNED_BYTE, env_pixels);
        break;
    case MENU_DOUGHNUT:
        current_mode = DOUGHNUT_MAPPING;
        glDisable(GL_TEXTURE_3D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, res_tilable[0], res_tilable[1], 0, GL_RGB, GL_UNSIGNED_BYTE, tilable_pixels);
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
  glutAddMenuEntry("Zoom In",ZOOM_IN);
  glutAddMenuEntry("Zoom Out",ZOOM_OUT);
  glutAddMenuEntry("Animate Faster",MENU_FASTER);
  glutAddMenuEntry("Animate Slower",MENU_SLOWER);
  glutAddMenuEntry("Stop/Run", MENU_STOP_RUN);
  glutAddMenuEntry("3D Texture", MENU_3D_TEXTURE);
  glutAddMenuEntry("Environment Mapping", MENU_ENV_MAPPING);
  glutAddMenuEntry("Doughnut", MENU_DOUGHNUT);
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
        filename = "input.t";
    }else{
        filename = argv[1];
    }

    if(argc > 2)
    {
        printf("%s\n", "INVALID NUMBER OF ARGUMENTS");
        exit(0);
    }

    set_distance();

    read_from_file(filename.c_str());

    env_pixels = read_ppm_file("envmap.ppm");
    tilable_pixels = read_ppm_file("tilable.ppm");
    texture_pixels = read_texture_file("marble.rgb");

    max_dimen = maxi(maximum[0], maximum[1]);
    max_dimen = maxi(max_dimen, maximum[2]);

  /* initialize GLUT: register callbacks, etc */
  wid = init_glut(&argc, argv);

  /* any OpenGL state initialization we need to do */
  init_opengl();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_3D);

  glutMainLoop();

  return 0;
}

