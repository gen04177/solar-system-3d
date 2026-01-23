#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "OSMesa/gl_mangle.h"
#include "OSMesa/gl.h"
#include "OSMesa/osmesa.h"

#include "stars.h"

int enablePlanetLighting = 1;
int showOrbits = 1;
int buttonBPrevState = 0;
int rightShoulderPrevState = 0;
const float MAX_CAMERA_DISTANCE = 100.0f;
int startButtonPrevState = 0;
int isPaused = 0;
float orbitRadii[8] = { 5, 7, 10, 13, 18, 22, 28, 35 };

float scaleFactor = 0.1;
GLuint saturnRingTexture;
float sunRotationAngle = 0.0f;
float sunRotationSpeed = 0.2f;

float moonOrbitRadius = 0.5f;
float moonSize;
float moonOrbitSpeed = 1.0f;
float moonOrbitAngle = 0.0f;
GLuint moonTexture;

float moonMaxSize;
float orbitAngles[8] = { 0 };

float jupiterX, jupiterY = 0.0f, jupiterZ;

void
drawSphere (float radius, int slices, int stacks)
{
  float stepPhi = M_PI / stacks;
  float stepTheta = 2 * M_PI / slices;

  for (int i = 0; i < stacks; i++)
    {
      float phi = i * stepPhi;
      float nextPhi = (i + 1) * stepPhi;

      glBegin (GL_QUAD_STRIP);
      for (int j = 0; j <= slices; j++)
	{
	  float theta = j * stepTheta;

	  float x = cos (theta) * sin (phi);
	  float y = sin (theta) * sin (phi);
	  float z = cos (phi);

	  float u = (float) j / slices;
	  float v = (float) i / stacks;

	  glTexCoord2f (u, v);

	  glNormal3f (x, y, z);
	  glVertex3f (radius * x, radius * y, radius * z);

	  x = cos (theta) * sin (nextPhi);
	  y = sin (theta) * sin (nextPhi);
	  z = cos (nextPhi);

	  u = (float) j / slices;
	  v = (float) (i + 1) / stacks;

	  glTexCoord2f (u, v);
	  glNormal3f (x, y, z);
	  glVertex3f (radius * x, radius * y, radius * z);
	}
      glEnd ();
    }
}


void
drawRings (float innerRadius, float outerRadius, int segments, GLuint texture)
{
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat mat_ambient[] = { 0.5f, 0.5f, 0.5f, 0.7f };
  GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 0.7f };
  GLfloat mat_specular[] = { 0.2f, 0.2f, 0.2f, 0.7f };
  GLfloat mat_shininess[] = { 15.0f };

  glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
  glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  if (texture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, texture);
      glColor4f (1.0f, 1.0f, 1.0f, 0.7f);
    }

  glBegin (GL_TRIANGLE_STRIP);
  for (int i = 0; i <= segments; i++)
    {
      float angle = 2.0f * M_PI * i / segments;
      float x = cos (angle);
      float z = sin (angle);

      float u = (float) i / segments;

      float nx = 0.0f;
      float ny = 1.0f;
      float nz = 0.0f;

      glNormal3f (nx, ny, nz);
      glTexCoord2f (u, 0.0f);
      glVertex3f (innerRadius * x, 0.0f, innerRadius * z);

      glNormal3f (nx, ny, nz);
      glTexCoord2f (u, 1.0f);
      glVertex3f (outerRadius * x, 0.0f, outerRadius * z);
    }
  glEnd ();

  if (texture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, 0);
    }

  glDisable (GL_BLEND);
}




void
drawOrbit (float radius)
{
  glBegin (GL_LINE_LOOP);
  for (int i = 0; i < 360; i++)
    {
      float theta = i * M_PI / 180.0;
      glVertex3f (radius * cos (theta), 0.0, radius * sin (theta));
    }
  glEnd ();
}

void
drawPlanet (float orbitRadius, float size, float orbitAngle,
	    float rotationAngle, GLuint texture)
{

  glEnable (GL_LIGHTING);

  glPushMatrix ();
  glRotatef (orbitAngle, 0.0, 1.0, 0.0);
  glTranslatef (orbitRadius, 0.0, 0.0);

  glPushMatrix ();
  glRotatef (-90.0, 0.0, 0.0, 1.0);
  glRotatef (90.0, 0.0, 1.0, 0.0);
  glRotatef (rotationAngle, 0.0, 0.0, 1.0);

  if (enablePlanetLighting)
    {
      GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
      GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
      GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      GLfloat mat_shininess[] = { 50.0f };

      glMaterialfv (GL_FRONT, GL_AMBIENT, mat_ambient);
      glMaterialfv (GL_FRONT, GL_DIFFUSE, mat_diffuse);
      glMaterialfv (GL_FRONT, GL_SPECULAR, mat_specular);
      glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);
    }
  else
    {
      glDisable (GL_LIGHTING);
    }

  if (texture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, texture);
      glColor3f (1.0f, 1.0f, 1.0f);
    }

  if (orbitRadius == orbitRadii[5])
    {
      glPushMatrix ();
      glRotatef (-90.0, 1.0, 0.0, 0.0);

      drawRings (8.45 * 1.3 * scaleFactor, 8.45 * 1.9 * scaleFactor, 100,
		 saturnRingTexture);

      drawRings (8.45 * 2.0 * scaleFactor, 8.45 * 2.3 * scaleFactor, 100,
		 saturnRingTexture);

      glPopMatrix ();
    }



  drawSphere (size, 20, 20);

  if (texture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, 0);
    }

  if (!enablePlanetLighting)
    {
      glEnable (GL_LIGHTING);
    }



  glPopMatrix ();
  glPopMatrix ();
}


void
drawMoon (float earthOrbitRadius, float earthOrbitAngle,
	  float moonOrbitRadius, float moonSize, float moonOrbitAngle,
	  GLuint moonTexture)
{
  glPushMatrix ();
  glRotatef (earthOrbitAngle, 0.0, 1.0, 0.0);
  glTranslatef (earthOrbitRadius, 0.0, 0.0);

  glRotatef (moonOrbitAngle, 0.0, 1.0, 0.0);
  glTranslatef (moonOrbitRadius, 0.0, 0.0);

  glRotatef (-moonOrbitAngle, 0.0, 1.0, 0.0);

  if (moonTexture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, moonTexture);
      glColor3f (1.0f, 1.0f, 1.0f);
    }
  drawSphere (moonSize, 20, 20);
  if (moonTexture > 0)
    {
      glBindTexture (GL_TEXTURE_2D, 0);
    }

  glPopMatrix ();
}

float
interpolate (float start, float end, float factor)
{
  return start + factor * (end - start);
}

void
setStarColor (Star * star)
{
  float mag = star->magnitude;

  float factor;

  if (mag < 0.0)
    {
      star->r = 0.4f;
      star->g = 0.6f;
      star->b = 1.0f;
    }
  else if (mag < 1.5)
    {
      factor = (mag - 0.0f) / (1.5f - 0.0f);
      star->r = interpolate (0.4f, 0.7f, factor);
      star->g = interpolate (0.6f, 0.8f, factor);
      star->b = interpolate (1.0f, 1.0f, factor);
    }
  else if (mag < 3.0)
    {
      factor = (mag - 1.5f) / (3.0f - 1.5f);
      star->r = interpolate (0.7f, 1.0f, factor);
      star->g = interpolate (0.8f, 1.0f, factor);
      star->b = interpolate (1.0f, 1.0f, factor);
    }
  else if (mag < 4.5)
    {
      factor = (mag - 3.0f) / (4.5f - 3.0f);
      star->r = interpolate (1.0f, 1.0f, factor);
      star->g = interpolate (1.0f, 0.9f, factor);
      star->b = interpolate (1.0f, 0.7f, factor);
    }
  else if (mag < 6.0)
    {
      factor = (mag - 4.5f) / (6.0f - 4.5f);
      star->r = interpolate (1.0f, 1.0f, factor);
      star->g = interpolate (0.9f, 0.7f, factor);
      star->b = interpolate (0.7f, 0.5f, factor);
    }
  else
    {
      star->r = 1.0f;
      star->g = 0.5f;
      star->b = 0.5f;
    }
}

void
initializeStars ()
{
  for (int i = 0; i < NUM_STARS; i++)
    {
      float alpha = starData[i][0] * M_PI / 12.0;
      float delta = starData[i][1] * M_PI / 180.0;

      float x = cos (delta) * cos (alpha);
      float y = cos (delta) * sin (alpha);
      float z = sin (delta);

      stars[i].x = x * 100.0f;
      stars[i].y = y * 100.0f;
      stars[i].z = z * 100.0f;

      stars[i].magnitude = starData[i][2];
      setStarColor (&stars[i]);
    }
}

void
drawStars ()
{
  glDisable (GL_LIGHTING);

  glEnable (GL_POINT_SMOOTH);
  glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (int i = 0; i < NUM_STARS; i++)
    {
      float size = (6.0f - stars[i].magnitude) * 0.5f;
      if (size < 1.0f)
      size = 1.0f;

      glColor4f (stars[i].r, stars[i].g, stars[i].b, 1.0f);
      glPointSize (size);
      glBegin (GL_POINTS);
      glVertex3f (stars[i].x, stars[i].y, stars[i].z);
      glEnd ();
    }

  glDisable (GL_BLEND);
}

float targetX, targetY, targetZ;


void
printHelp (void)
{

  printf
    ("\n\nSolar System 3D Sim by gen04177 - v0.001 - Running with SDL %d.%d.%d (compiled with %d.%d.%d)\n",
     SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL,
     SDL_COMPILEDVERSION >> 24, (SDL_COMPILEDVERSION >> 16) & 0xFF,
     SDL_COMPILEDVERSION & 0xFFFF);

}


int
main (int argc, char *argv[])
{

  printHelp ();

  initializeStars ();


  if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
      fprintf (stderr, "Failed to initialize SDL: %s\n", SDL_GetError ());
      return -1;
    }

  SDL_Window *window;
  SDL_Surface *screen;
  OSMesaContext ctx;
  void *frameBuffer;
  const int width = 1920;
  const int height = 1080;

  window =
    SDL_CreateWindow ("sss3d v0.001", SDL_WINDOWPOS_CENTERED,
		      SDL_WINDOWPOS_CENTERED, width, height,
		      SDL_WINDOW_SHOWN);
  if (!window)
    {
      fprintf (stderr, "Failed to create window: %s\n", SDL_GetError ());
      SDL_Quit ();
      return -1;
    }

  screen = SDL_GetWindowSurface (window);
  if (!screen)
    {
      fprintf (stderr, "“Failed to get the window surface: %s\n",
	       SDL_GetError ());
      SDL_DestroyWindow (window);
      SDL_Quit ();
      return -1;
    }

  ctx = OSMesaCreateContext (OSMESA_RGBA, NULL);
  frameBuffer = malloc (width * height * 4);
  if (!OSMesaMakeCurrent (ctx, frameBuffer, GL_UNSIGNED_BYTE, width, height))
    {
      fprintf (stderr, "Failed to create OSMesa context.\n");
      free (frameBuffer);
      SDL_DestroyWindow (window);
      SDL_Quit ();
      return -1;
    }

  glViewport (0, 0, width, height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  float aspect = (float) width / (float) height;
  glFrustum (-0.1f * aspect, 0.1f * aspect, -0.1f, 0.1f, 0.1f, 1000.0f);

  glMatrixMode (GL_MODELVIEW);

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_2D);

  moonSize = 0.27f * scaleFactor;
  moonMaxSize = 0.27f * scaleFactor;


  GLuint saturnRingTexture;

  GLuint planetTextures[8];

  SDL_GameController *controller = NULL;
  if (SDL_NumJoysticks () > 0)
    {
      controller = SDL_GameControllerOpen (0);
      if (!controller)
	{
	  fprintf (stderr, "Failed to open controller: %s\n",
		   SDL_GetError ());
	}
    }

  float orbitAngles[8] = { 0 };
  float rotationAngles[8] = { 0 };
  float orbitSpeeds[8] = { 0.5, 0.4, 0.3, 0.2, 0.1, 0.08, 0.06, 0.05 };
  float rotationSpeeds[8] = { 1, 1.5, 1.2, 1, 0.8, 0.7, 0.6, 0.5 };
  float orbitRadii[8] = { 5, 7, 10, 13, 18, 22, 28, 35 };
  float scaleFactor = 0.1;
  float sizes[8] =
    { 0.38 * scaleFactor, 0.95 * scaleFactor, 1.0 * scaleFactor,
0.53 * scaleFactor,
    12.21 * scaleFactor, 8.45 * scaleFactor, 4.01 * scaleFactor,
      3.88 * scaleFactor
  };
  float colors[8][3] = {
    {1.0, 0.0, 0.0},
    {0.8, 0.4, 0.0},
    {0.2, 0.2, 1.0},
    {0.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {0.5, 0.0, 0.5},
    {0.3, 0.3, 0.3},
    {0.0, 0.5, 1.0}
  };

  float cameraX = 0.0f, cameraY = -1.0f, cameraZ = -50.0f;
  float cameraYaw = 0.0f, cameraPitch = 30.0f;


  int running = 1;
  SDL_Event event;

  while (running)
    {
      while (SDL_PollEvent (&event))
	{
	  if (event.type == SDL_QUIT)
	    {
	      running = 0;
	    }
	}

      GLfloat light_position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
      GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
      GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
      GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

      glEnable (GL_LIGHTING);
      glEnable (GL_LIGHT0);
      glLightfv (GL_LIGHT0, GL_POSITION, light_position);
      glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
      glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
      glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);




      if (controller)
	{

	  int startButtonState =
	    SDL_GameControllerGetButton (controller,
					 SDL_CONTROLLER_BUTTON_START);
	  int rightShoulderState =
	    SDL_GameControllerGetButton (controller,
					 SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	  int buttonBState =
	    SDL_GameControllerGetButton (controller, SDL_CONTROLLER_BUTTON_A);

	  if (startButtonState && !startButtonPrevState)
	    {
	      isPaused = !isPaused;
	    }
	  startButtonPrevState = startButtonState;

	  if (buttonBState && !buttonBPrevState)
	    {
	      showOrbits = !showOrbits;
	    }
	  buttonBPrevState = buttonBState;

	  if (rightShoulderState && !rightShoulderPrevState)
	    {
	      enablePlanetLighting = !enablePlanetLighting;
	    }
	  rightShoulderPrevState = rightShoulderState;


	  float leftX =
	    SDL_GameControllerGetAxis (controller,
				       SDL_CONTROLLER_AXIS_LEFTX) / 32768.0f;
	  float leftY =
	    SDL_GameControllerGetAxis (controller,
				       SDL_CONTROLLER_AXIS_LEFTY) / 32768.0f;
	  float rightX =
	    SDL_GameControllerGetAxis (controller,
				       SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
	  float rightY =
	    SDL_GameControllerGetAxis (controller,
				       SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;

	  cameraYaw += rightX * 2.0f;
	  cameraPitch -= rightY * 2.0f;

	  cameraX += leftX * 0.5f;
	  cameraZ += leftY * 0.5f;

	  float distance = sqrt (cameraX * cameraX + cameraZ * cameraZ);
	  if (distance > MAX_CAMERA_DISTANCE)
	    {
	      float scale = MAX_CAMERA_DISTANCE / distance;
	      cameraX *= scale;
	      cameraZ *= scale;
	    }

	}


      if (!isPaused)
	{
	  for (int i = 0; i < 8; i++)
	    {
	      orbitAngles[i] += orbitSpeeds[i];
	      rotationAngles[i] += rotationSpeeds[i];
	    }

	  sunRotationAngle += sunRotationSpeed;
	  if (sunRotationAngle >= 360.0f)
	    {
	      sunRotationAngle -= 360.0f;
	    }
	}

      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glLoadIdentity ();
      glTranslatef (cameraX, cameraY, cameraZ);
      glRotatef (cameraPitch, 1.0, 0.0, 0.0);
      glRotatef (cameraYaw, 0.0, 1.0, 0.0);


      if (showOrbits)
	{
	  glDisable (GL_LIGHTING);
	  for (int i = 0; i < 8; i++)
	    {
	      drawOrbit (orbitRadii[i]);
	    }
	  glEnable (GL_LIGHTING);
	}


      glDisable (GL_LIGHTING);
      glPushMatrix ();
      glRotatef (sunRotationAngle, 0.0, 1.0, 0.0);
      drawSphere (3.0, 50, 50);
      glBindTexture (GL_TEXTURE_2D, 0);
      glPopMatrix ();
      glEnable (GL_LIGHTING);

      drawStars ();

      for (int i = 0; i < 8; i++)
	{
	  drawPlanet (orbitRadii[i], sizes[i], orbitAngles[i],
		      rotationAngles[i], planetTextures[i]);
	}

      if (!isPaused)
	{
	  moonOrbitAngle += moonOrbitSpeed;
	  if (moonOrbitAngle >= 360.0f)
	    {
	      moonOrbitAngle -= 360.0f;
	    }
	}

      drawMoon (orbitRadii[2], orbitAngles[2], moonOrbitRadius, moonSize,
		moonOrbitAngle, moonTexture);




      glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
		    screen->pixels);
      SDL_UpdateWindowSurface (window);
      SDL_Delay (16);
    }




  if (controller)
    {
      SDL_GameControllerClose (controller);
    }


  for (int i = 0; i < 8; i++)
    {
      glDeleteTextures (1, &planetTextures[i]);
    }

  free (frameBuffer);
  OSMesaDestroyContext (ctx);
  SDL_DestroyWindow (window);
  SDL_Quit ();

  return 0;
}
