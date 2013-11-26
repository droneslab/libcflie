// Copyright (c) 2013, Jan Winkler <winkler@cs.uni-bremen.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Universität Bremen nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glfw.h>
#include <GL/glut.h>
#include <cflie/CCrazyflie.h>
#include <ncurses.h>
#include <GL/glut.h>

using namespace std;


bool g_bGoon;

void printGL(int, int, int, char *);

void drawGL(float fX, float fY, float fZ) {
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);

  /* Move down the z-axis. */
  glTranslatef(0.0, 0.0, -5.0);
  
  /* Rotate. */
  glRotatef(fX - 45.0f, 1.0, 0.0, 0.0);
  glRotatef(fY, 0.0, 1.0, 0.0);
  glRotatef(fZ, 0.0, 0.0, 1.0);
  
  float fQuadWidth = 2;
  float fQuadHeight = fQuadWidth;
  
  glBegin(GL_QUADS); {
    glColor3f(1, 1, 1);
    
    glVertex2f(fQuadWidth / 2, fQuadHeight / 2);
    glVertex2f(fQuadWidth / 2, -fQuadHeight / 2);
    glVertex2f(-fQuadWidth / 2, -fQuadHeight / 2);
    glVertex2f(-fQuadWidth / 2, fQuadHeight / 2);
  }
  glEnd();
  
  glfwSwapBuffers();
}

int main(int argc, char **argv) {
  CCrazyRadio *crRadio = new CCrazyRadio("radio://0/10/250K");
  char pstring[20], pidControl=0;
  FILE* f;
  
  if(crRadio->startRadio()) {
    CCrazyflie *cflieCopter = new CCrazyflie(crRadio);
    cflieCopter->setSendSetpoints(true);
    cflieCopter->setThrust(0);

    if(glfwInit() == GL_TRUE) {
      g_bGoon = true;
      
      int nWidth = 800, nHeight = 600;
      int nBitsPerComponent = 8, nDepthBits = 0, nStencilBits = 0;
      int nOpenGLMode = GLFW_WINDOW;
      double gyroX1=0.0, gyroX2=0.0, dPitch=0.0, kp_pitch=-1.0;
      double gyroY1=0.0, gyroY2=0.0, dRoll=0.0, kp_roll=1.0;
      double gyroZ1=0.0, gyroZ2=0.0, dYaw=0.0, kp_yaw=1.0;
      double accZ1=0.0, accZ2=0.0, dThrust=0.0, kp_thrust=-500000000.0;
      double gyroX0=0.0, gyroY0=0.0, gyroZ0=0.0, accZ0=0.0;
      bool start=true;

      if(glfwOpenWindow(nWidth, nHeight,
			nBitsPerComponent, nBitsPerComponent, nBitsPerComponent, nBitsPerComponent, nDepthBits, nStencilBits, nOpenGLMode)) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float fAspectRatio = ((float)nHeight) / ((float)nWidth);
	glFrustum(.5, -.5, -.5 * fAspectRatio, .5 * fAspectRatio, 1, 50);
	glMatrixMode(GL_MODELVIEW);
	
	//	cout << "Running, exit with 'ESC'." << endl;
	// init console screen
	for(int i=0; i < 6; i++)
	  cflieCopter->cycle();

	cflieCopter->setThrust(30000);
	//	sleep(3);
	WINDOW *win;
	win = initscr();
	f=fopen("log.txt","w+");

	while(g_bGoon) {
	  // Print data
	  initscr();
	  mvprintw(5, 5, "accx: %f accy: %f accz: %f", cflieCopter->accX(), cflieCopter->accY(), cflieCopter->accZ());
	  refresh();
	  getch();
	  endwin();
	  if(cflieCopter->cycle()) {
	    drawGL(cflieCopter->roll(),
		   cflieCopter->pitch(),
		   cflieCopter->yaw());

	      accZ2 = accZ1;
	      accZ1 = cflieCopter->accZ();
	      dThrust =  kp_thrust * (accZ1 - accZ0);

	      gyroX2 = gyroX1;
	      gyroX1 = cflieCopter->gyroX();
	      dPitch = kp_pitch * (gyroX1 - gyroX0);

	      gyroY2 = gyroY1;
	      gyroY1 = cflieCopter->gyroY();
	      dRoll = kp_roll * (gyroY1 - gyroY0);

	      gyroZ2 = gyroZ1;
	      gyroZ1 = cflieCopter->gyroZ();
	      dYaw = kp_yaw * (gyroZ1 - gyroZ0);
	      
	      if(start) {
		gyroX0 = gyroX2;
		gyroY0 = gyroY2;
		gyroZ0 = gyroZ2;
		accZ0 = accZ2;
		start = false;
	      }
	      
	      fprintf(f, "%f %f %f %f %f %f %f %f %f %f %f %d\n", (gyroX1 - gyroX0), dPitch, cflieCopter->pitch(), (gyroY1 - gyroY0), dRoll, cflieCopter->roll(),(gyroZ1 - gyroZ0), dYaw, cflieCopter->yaw(), (accZ1 - accZ0), dThrust, cflieCopter->thrust()); 
	      
	    mvprintw(50, 5, "accx: %f accy: %f accz: %f", cflieCopter->accX() *100, cflieCopter->accY() *100, cflieCopter->accZ()*100);
	    mvprintw(51, 5, "gyrox: %f gyroy: %f gyroz: %f", cflieCopter->gyroX(), cflieCopter->gyroY(), cflieCopter->gyroZ());
	    //	    mvprintw(52, 5, "barometer pressure:%f temp: %f asl: %f", cflieCopter->pressure(), cflieCopter->temperature(), cflieCopter->asl());
	    mvprintw(52, 5, "Quad thrust: %f roll: %f pitch: %f yaw: %f ", cflieCopter->thrust(), cflieCopter->roll(), cflieCopter->pitch(), cflieCopter->yaw());
	    mvprintw(53, 5, "Yaw: %f", cflieCopter->yaw());
	    mvprintw(54, 5, "PID Control: %d", pidControl);
	    mvprintw(55, 5, "Thrust: %f %f %d", (accZ1 - accZ0), dThrust, cflieCopter->thrust());
	    mvprintw(56, 5, "Pitch : %f %f %f", (gyroX1 - gyroX0), dPitch, cflieCopter->pitch());
	    mvprintw(57, 5, "Roll  : %f %f %f", (gyroY1 - gyroY0), dRoll, cflieCopter->roll());
	    mvprintw(58, 5, "Yaw   : %f %f %f", (gyroZ1 - gyroZ0), dYaw, cflieCopter->yaw());
	    refresh();

	    if(pidControl) {
	      //	      cflieCopter->setThrust(dThrust);
	      cflieCopter->setPitch(dPitch);
	      cflieCopter->setRoll(dRoll);
	      cflieCopter->setYaw(dYaw);
	    }

	    //	    cout << "yaw " << cflieCopter->yaw() << endl;
	    if(glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS) {
	      cflieCopter->setThrust(0);
	      g_bGoon = false;
	      endwin();
	    } 
	    else {
	      if(glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS) {
		cflieCopter->setThrust(45000);
	      } 
	      else {
		cflieCopter->setThrust(30000);
	      }

	      if(glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS) {
		if(pidControl) 
		  pidControl = 0;
		else
		  pidControl = 1;
	      }
	      
	      double dRoll = 0;
	      double dYaw = cflieCopter->yaw();
	      
	      if(glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
		dRoll = 20.0f;//dYaw += 20.0f;
	      } else if(glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
		dRoll = -20.0f;//dYaw -= 20.0f;
	      }
	      
	      /*
	      if(glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS) {
		dPitch = 20.0f;
	      } else if(glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
		dPitch = -20.0f;
	      }
	      */
	      
	      cflieCopter->setRoll(dRoll);
	      cflieCopter->setPitch(dPitch);
	      //      cflieCopter->setYaw(dYaw);
	    }
	  } else {
	    g_bGoon = false;
	  }
	}
	
	glfwCloseWindow();
      }
      
      glfwTerminate();
    }
    
    delete cflieCopter;
  } else {
    cerr << "Radio could not be started." << endl;
  }
  
  delete crRadio;
  
  return 0;
}


void printGL(int x, int y,int z, char *string) {
  //set the position of the text in the window using the x and y coordinates
  glRasterPos2f(x,y);
  //get the length of the string to display
  int len = (int) strlen(string);
  
  //loop to display character by character
  for (int i = 0; i < len; i++) 
    {
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,string[i]);
    }
};
