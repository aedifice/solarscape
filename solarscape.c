/*
 * Program that displays a solar system for user to explore.
 *
 * KEY BINDINGS:
 * up/down arrow keys - z axis
 *      - forward thrust
 * left/right arrow keys - x axis
 * x/c - moves camera up or down (y axis), but keeps camera pointed at (0,0)
 *      - pitch
 * r/t - rotates left or right
 *      - yaw
 * s/d  - tilts camera
 *      - roll
 * p    - toggles teapot mode
 */

#include <GLUT/glut.h>
#include <stdlib.h>
#include <unistd.h>     // for usleep()
#include <math.h>       // for sin and cos
#include <time.h>       // to seed randomness

#define ESC 27
#define TRANS_AMT 0.2    // how much camera should move per key press
#define MAX_DEPTH 25     // max depth the camera can go in any direction
#define MAX_PLANETS 10   // max number of planets
#define MAX_STARS 1000   // max number of stars
#define SPIN_SPEED 10.0  // how fast planets spin on their axes
#define STAR_DEPTH 25    // how far away in space the stars are
#define PI 3.1415926

GLfloat lightColor[] = {1.0, 1.0, 1.0, 1.0};
GLfloat totalX = 0;   // stores coords for camera
GLfloat totalY = 0;
GLfloat totalZ = -15;
GLfloat rotX = 0;
GLfloat tilt = 0;

int isTeapot = 0;


struct planet {
    // color of planet
    GLfloat r, g, b;
    
    // center coord of planet
    GLfloat x, y, z;
    
    GLfloat scale;  // size of planet
    GLint hostNum;  // relates to array index of host
    GLint depth;    // relates to how "deep" planet is in host system (moon, etc)
    GLfloat dist;   // distance from host planet (describes radius)
    GLfloat spin;   // amoumt the planet has rotated around axis
    GLfloat rotate; // amount planet has rotated around host planet
};

struct star {
    // just store coords for now, but could be expanded later to hold size,
    // color, etc.
    // also, coords are stored as a fraction of the screen to be
    // reconstituted later
    GLfloat x, y, z;
};

struct planet planetList[MAX_PLANETS];
struct star starList[MAX_STARS];
int planIndex = 0;


/*
 * where i is the planet number or index
 */
void dispPlanet(GLint i) {
    // calculate where planet is based on host
    int hosti = planetList[i].hostNum;
    if(hosti >= 0) {
        planetList[i].x = planetList[hosti].x + (planetList[i].dist * cos(planetList[i].rotate * (2.0 * PI) / 360));
        planetList[i].z = planetList[hosti].z + (planetList[i].dist * sin(planetList[i].rotate * (2.0 * PI) / 360));
    }
    
    
    glPushMatrix();
        // set color
        lightColor[0] = planetList[i].r;
        lightColor[1] = planetList[i].g;
        lightColor[2] = planetList[i].b;
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    
        // translate it to new location
        glTranslatef(planetList[i].x, planetList[i].y, planetList[i].z);
    
        // rotate it to make it spin
        glRotatef(planetList[i].spin, 0.0, 1.0, 0.0);
    
        if (isTeapot) {
            glutSolidTeapot(planetList[i].scale);
        } else {
            glutSolidSphere(planetList[i].scale, 40, 40);
        }
    glPopMatrix();
}

/*
 * unpacks list of stars and draws them all
 */
void displayStars() {
    // set point size and light color
    glPointSize(2.0);
    lightColor[0] = 1.0;
    lightColor[1] = 1.0;
    lightColor[2] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    
    for(int i = 0; i < MAX_STARS; i++) {
        glBegin(GL_POINTS);
            glVertex3f(starList[i].x, starList[i].y, starList[i].z);
        glEnd();
    }
}

/*
 * display callback; call planet func as necessary and then
 * transform camera to current user position.
 */
void display(void) {
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // make a box of stars for the planets to be in
    displayStars();
	
    // display all the planets
    for(int i = 0; i < planIndex; i++)
        dispPlanet(i);
    
    
    // after all the planets are placed, place the camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(totalX, totalY, totalZ,
              rotX, 0.0, totalZ + 1.0,
              tilt, 1.0, 0.0);
    
	glFlush();
	glutSwapBuffers();
}

/*
 * keyboard callback function
 */
void keyboard(unsigned char key, int x, int y) {
	switch(key) {
		case 'x':
            if(totalY - TRANS_AMT > -MAX_DEPTH)
                totalY -= TRANS_AMT;
			break;
        case 'c':
            if(totalY + TRANS_AMT < MAX_DEPTH)
                totalY += TRANS_AMT;
            break;
        case 'r':
            rotX -= TRANS_AMT;
            break;
        case 't':
            rotX += TRANS_AMT;
            break;
        case 's':
            tilt -= TRANS_AMT;
            break;
        case 'd':
            tilt += TRANS_AMT;
            break;
        case 'p':
            if (isTeapot) {
                isTeapot = 0;
            } else {
                isTeapot = 1;
            }
            break;
		case ESC:
			exit(0);
			break;
		default:
			break;
	}
    
    glutPostRedisplay();
}

/*
 * use a different function for special keys
 */
void arrowKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:
            if(totalX + TRANS_AMT < MAX_DEPTH) {
                totalX += TRANS_AMT;
                rotX = totalX;  // snap rotation back to looking forward
            }
            break;
        case GLUT_KEY_RIGHT:
            if(totalX - TRANS_AMT > -MAX_DEPTH) {
                totalX -= TRANS_AMT;
                rotX = totalX;
            }
            break;
        case GLUT_KEY_DOWN:
            if(totalZ - TRANS_AMT > -MAX_DEPTH)
                totalZ -= TRANS_AMT;
            break;
        case GLUT_KEY_UP:
            if(totalZ + TRANS_AMT < MAX_DEPTH)
                totalZ += TRANS_AMT;
            break;
        default:
            break;
    }
    
    glutPostRedisplay();
}



/*
 * idle function
 * every time this updates, update the spin and rotation of planets before
 * going to sleep for a while again.
 */
void spin() {
    // spin each planet
    for(int i = 0; i < planIndex; i++) {
        planetList[i].spin += SPIN_SPEED;
        if(planetList[i].spin > 360.0)
            planetList[i].spin -= 360.0;
        
        // also rotate it around host planet
        if(planetList[i].hostNum >= 0)
            planetList[i].rotate += SPIN_SPEED * (1/planetList[i].dist);
        else
            planetList[i].rotate += SPIN_SPEED;
        
        
        // if it's a moon, make it go a little faster so it gets around
        // its host all right
        if(planetList[i].hostNum > 0) {
            planetList[i].rotate += SPIN_SPEED * planetList[i].depth;
        }
        
        
        if(planetList[i].rotate > 360.0)
            planetList[i].rotate -= 360.0;
    }
    
    glutPostRedisplay();
    usleep(100000);
}



/*
 * window reshape/resize callback
 */
void resize(int w, int h) {
	glViewport(0, 0, w, h);
    
	// use a perspective view
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
	// for perspective view - add 5 to MAX_DEPTH just to add a buffer
    // so the stars in the corners don't start disappearing
	gluPerspective(45.0, ((GLdouble) w)/h, 1.0, MAX_DEPTH + STAR_DEPTH + 5);
    
	glMatrixMode(GL_MODELVIEW);
}

/*
 * If you're wanting to add a planet or change an existing one,
 * this is where to look; could be a bit more elegant in future - 
 * perhaps a way to randomly generate planets?
 */
void generatePlanets() {
    planetList[planIndex].r = 1.0;
    planetList[planIndex].g = 1.0;
    planetList[planIndex].b = 0.8;
    planetList[planIndex].scale = 2;
    planetList[planIndex].hostNum = -1;
    planetList[planIndex].depth = 0;
    planIndex++;
    
    planetList[planIndex].r = 0.0;
    planetList[planIndex].g = 1.0;
    planetList[planIndex].b = 0.5;
    planetList[planIndex].scale = 1;
    planetList[planIndex].hostNum = 0;
    planetList[planIndex].dist = 4.5;
    planetList[planIndex].depth =
        planetList[planetList[planIndex].hostNum].depth + 1;
    planIndex++;
    
    planetList[planIndex].r = 0.8;
    planetList[planIndex].g = 0.2;
    planetList[planIndex].b = 0.2;
    planetList[planIndex].scale = 0.5;
    planetList[planIndex].hostNum = 1;
    planetList[planIndex].dist = 2;
    planetList[planIndex].depth =
        planetList[planetList[planIndex].hostNum].depth + 1;
    planIndex++;
    
    planetList[planIndex].r = 0.8;
    planetList[planIndex].g = 0.6;
    planetList[planIndex].b = 0.0;
    planetList[planIndex].scale = 1.4;
    planetList[planIndex].hostNum = 0;
    planetList[planIndex].dist = 8.5;
    planetList[planIndex].depth =
        planetList[planetList[planIndex].hostNum].depth + 1;
    planIndex++;
}

/*
 * Randomly assign and store star positions
 */
void generateStars() {
    // fill up star list
    for(int i = 0; i < MAX_STARS; i++) {
        // randomly generate a couple signs and numbers
        GLint sign1 = rand() % 2;
        GLint sign2 = rand() % 2;
        GLint num1 = rand() % STAR_DEPTH;   // about how wide the viewing plane is
        GLint num2 = rand() % STAR_DEPTH;
        
        // if the sign is zero, flip the number to negative
        if(!sign1) {
            num1 = 0 - num1;
        }
        if(!sign2) {
            num2 = 0 - num2;
        }
        
        // add star coords to list depending on where in the list
        // we will create six "sides" to form a box of stars
        if(i > (5/6.0 * MAX_STARS)) {
            starList[i].x = num1;
            starList[i].y = num2;
            starList[i].z = STAR_DEPTH;
        } else if(i > (4/6.0 * MAX_STARS)) {
            starList[i].x = num1;
            starList[i].y = num2;
            starList[i].z = -STAR_DEPTH;
        } else if(i > (3/6.0 * MAX_STARS)) {
            starList[i].x = num1;
            starList[i].y = STAR_DEPTH;
            starList[i].z = num2;
        } else if(i > (2/6.0 * MAX_STARS)) {
            starList[i].x = num1;
            starList[i].y = -STAR_DEPTH;
            starList[i].z = num2;
        } else if(i > (1/6.0 * MAX_STARS)) {
            starList[i].x = STAR_DEPTH;
            starList[i].y = num1;
            starList[i].z = num2;
        } else {
            starList[i].x = -STAR_DEPTH;
            starList[i].y = num1;
            starList[i].z = num2;
        }
    }
    
}

/* initialize the OpenGL state */
void init(void) {
    GLfloat light_pos[] = {1.0f, 2.0f, 3.0f, 1.0f};
    
	// set background color
	glClearColor (0.0, 0.0, 0.0, 1.0);
    
    // rendering options
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

    // lighting setup
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    
	// lighting positioning
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightColor);
    
    
    // some final generation before handing control over to display loop
    // create and save the planets and the star field
    generatePlanets();
    generateStars();
    
}

/* need both double buffering and z buffer */
int main(int argc, char **argv) {
	// initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Roundworld, or 'World Not on a Turtle'");
    
	// register callback functions
	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrowKeys);
    glutIdleFunc(spin);
    
	init();
    
	glutMainLoop();
	return 0;
}

