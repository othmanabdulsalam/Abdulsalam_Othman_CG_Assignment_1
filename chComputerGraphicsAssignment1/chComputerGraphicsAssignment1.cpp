#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <map>
#include <conio.h>

#include <chCamera/chCamera.h>
#include <chUtilities/chUtilities.h>
#include <chMaths/chMaths.h>
#include <chMaths/chVector.h>
#include <chSystem/chSystem.h>
#include <chPajParser/chPajParser.h>
#include <chText/chText.h>

#include "chConstants.h"
#include "chParse.h"
#include "chControl.h"

// NOTES
// look should look through the libraries and additional files I have provided to familarise yourselves with the functionallity and code.
// The data is loaded into the data structure, managed by the linked list library, and defined in the chSystem library.
// You will need to expand the definitions for chNode and chArc in the chSystem library to include additional attributes for the siumulation process
// If you wish to implement the mouse selection part of the assignment you may find the camProject and camUnProject functions usefull


// core system global data
chCameraInput g_Input; // structure to hadle input to the camera comming from mouse/keyboard events
chCamera g_Camera; // structure holding the camera position and orientation attributes
chSystem g_System; // data structure holding the imported graph of data - you may need to modify and extend this to support your functionallity
chControl g_Control; // set of flag controls used in my implmentation to retain state of key actions

// global var: parameter name for the file to load
const static char csg_acFileParam[] = {"-input"};

// global var: file to load data from
char g_acFile[256];


// color data
float blue[] = { 0.0f, 0.0f, 1.0f, 10.0f };
float green[] = { 0.0f, 1.0f, 0.0f, 1.0f };
float red[] = { 1.0f, 0.0f, 0.0f, 1.0f };
float purple[] = { 1.0f, 0.0f, 1.0f, 1.0f };
float orange[] = { 1.0f, 0.5f, 0.0f, 1.0f };
float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };

// boolean that sets simulation on or off
bool simulationFlag = false;
// boolean flag for menu
bool menuActive = false;

/* PROTOTYPES*/

// core functions -> reduce to just the ones needed by glut as pointers to functions to fulfill tasks
void display(); // The rendering function. This is called once for each frame and you should put rendering code here
void idle(); // The idle function is called at least once per frame and is where all simulation and operational code should be placed
void reshape(int iWidth, int iHeight); // called each time the window is moved or resived
void keyboard(unsigned char c, int iXPos, int iYPos); // called for each keyboard press with a standard ascii key
void keyboardUp(unsigned char c, int iXPos, int iYPos); // called for each keyboard release with a standard ascii key
void sKeyboard(int iC, int iXPos, int iYPos); // called for each keyboard press with a non ascii key (eg shift)
void sKeyboardUp(int iC, int iXPos, int iYPos); // called for each keyboard release with a non ascii key (eg shift)
void mouse(int iKey, int iEvent, int iXPos, int iYPos); // called for each mouse key event
void motion(int iXPos, int iYPos); // called for each mouse motion event

// Non glut functions
void myInit(); // the myinit function runs once, before rendering starts and should be used for setup
void nodeDisplay(chNode *pNode); // callled by the display function to draw nodes
void arcDisplay(chArc *pArc); // called by the display function to draw arcs
void buildGrid(); // 
void nodeAttributes(chNode *pNode, unsigned int continent, unsigned int worldSystem, float* position, char* name);

// Functions related to the simulation
void isSimulationOn();// Function for handling whether simulation should run or not
void runSimulation(); // Function for running the simulation
void resetResultantForce(chNode* targetNode); // reset the resultant force of the node
void applyForces(chArc* pArc); // apply spring forces for each of the 2 nodes making up the arc
void nodeBehaviour(chNode* targetNode); // change the behaviour of the node based off the forces applied to it
void applyMovement(chNode* targetNode); // apply movement to the node

/* END OF PROTOTYPES*/


void nodeDisplay(chNode *pNode) // function to render a node (called from display())
{
	// put your node rendering (ogl) code here
	float* position = pNode->m_afPosition; // set float pointer for the position of country data

	unsigned int continent = pNode->m_uiContinent; // set unsigned int for the continent of country data
	char* name = pNode->m_acName; // set the name of the country node 
	unsigned int worldSystem = pNode->m_uiWorldSystem; // set  unsigned int for the world system of the country

	glPushMatrix();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// call to function that handles attributes of the node
	nodeAttributes(pNode,continent, worldSystem,position,name);

	glPopMatrix();
	glPopAttrib();
}

void nodeAttributes(chNode* pNode, unsigned int continent, unsigned int worldSystem, float* position,char* name)
{

	// use gl translate d because using integer numbers 
	// pass x y and z position variables
	glTranslated(position[0], position[1], position[2]);

	// switch statement handling the colour that is chosen depending on continent that country belongs to
	switch (continent)
	{
		
	case 6: // South America
		// colour blue
		utilitiesColourToMat(blue, 2.0f);
		break;
	case 5: // Oceania
		// colour purple
		utilitiesColourToMat(purple, 0.0f);
		break;
	case 4: // North America
		// colour red
		utilitiesColourToMat(red, 2.0f);
		break;
	case 3: // Europe
		// colour orange
		utilitiesColourToMat(orange, 2.0f);
		break;
	case 2: // Asia
		// colour white
		utilitiesColourToMat(white, 2.0f);
		break;

	case 1: // Africa
		// colour green
		utilitiesColourToMat(green, 2.0f);
		break;
	default:
		break;
	}

	// switch statement handling shape displayed depending on world system
	switch (worldSystem)
	{
	case 3: // third world represents sphere
		glutSolidSphere(mathsRadiusOfSphereFromVolume(pNode->m_fMass), 15, 15);
		glTranslated(0, 20, 0); // translate text to be above the node object in scene
		break;
	case 2:
		glutSolidCube(mathsDimensionOfCubeFromVolume(pNode->m_fMass));
		glTranslated(0, 20, 0); // translate text to be above the node object in scene
		break;
	case 1: // first world is cone
		glRotatef(270, 1.0, 0.0, 0.0); // rotate so cone points upwards

		glutSolidCone(mathsDimensionOfCubeFromVolume(pNode->m_fMass) / 3, mathsDimensionOfCubeFromVolume(pNode->m_fMass), 15, 15);
		
		// return rotation so text isnt also rotated
		glRotatef(-270, 1.0, 0.0, 0.0);

		glTranslated(0, 40, 0); // translate text to be above the node object in scene
		break;
	default:
		break;
	}

	glMultMatrixf(camRotMatInv(g_Camera));// ensure text is always facing towards camera view
	glScalef(16, 16, 0.1f);
	outlinePrint(name, true);
}



void arcDisplay(chArc *pArc) // function to render an arc (called from display())
{
	// put your arc rendering (ogl) code here

	// node0 indicates start node, node1 indicates end node
	chNode* m_pNode0 = pArc->m_pNode0;
	chNode* m_pNode1 = pArc->m_pNode1;

	// floats for the two end points of the arc

	float* arcPos0 = m_pNode0->m_afPosition;
	float* arcPos1 = m_pNode1->m_afPosition;


	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		glColor3f(1.0f,0.0f,0.0f);
		glVertex3f(arcPos0[0],arcPos0[1], arcPos0[2]);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(arcPos1[0], arcPos1[1], arcPos1[2]);
	glEnd();


}


void isSimulationOn()
{
	if (simulationFlag)
	{
		runSimulation(); // run the simulation
	}
}

void runSimulation()
{
	int i;

	// loop through every node, reseting the force of each to 0
	visitNodes(&g_System, resetResultantForce);
	
	// loop through every arc, calculating the spring behaviour between the 2 nodes forming the arc
	visitArcs(&g_System, applyForces);


	// loop through every node, moving them in response to the forces applied to them
	visitNodes(&g_System, nodeBehaviour);

	// loop through the nodes applying the movement to them
	visitNodes(&g_System, applyMovement);

}


void resetResultantForce(chNode *targetNode)
{
	int i;
	for (i = 0; i < 3; i++)
	{
		// resets the force of the node to 0
		targetNode->resultantForce[i] = 0;
	}
}


static float COULOMB_CONSTANT_DEFAULT = 500.0f;
float power = 2.0f;
float restingLength = 1.0f;

void applyForces(chArc* pArc)
{
	/* Hooke's Law */

	// node0 indicates start node, node1 indicates end node
	chNode* m_pNode0 = pArc->m_pNode0;
	chNode* m_pNode1 = pArc->m_pNode1;

	// direction vector between the 2 nodes
	float directionVector[3];
	float directionVectorSquared[3];

	float directionVectorMagnitude;

	// extension = current length - resting length
	float extension = pArc->m_fIdealLen - restingLength;

	// calculate the spring force vector : Force = extension * coefficient of restitution
	float springForce = extension * pArc->m_fSpringCoef;

	// vector forces for each node: +- force * directionVector
	// one node has a +ve force and the other a -ve due to 3rd newton law

	float vectorForce0[3];
	float vectorForce1[3];


	int i;

	for (i = 0; i < 3; i++)
	{
		directionVector[i] = m_pNode1->m_afPosition[i] - m_pNode0->m_afPosition[i];
		directionVectorSquared[i] = directionVector[i] * directionVector[i];
	}

	/* Coulumb's Law */


	float dx = directionVector[0];
	float dy = directionVector[1];
	float dz = directionVector[2];
	float distance = sqrt(dx * dx + dy * dy + dz * dz);
	float xUnit = dx / distance;
	float yUnit = dy / distance;
	float zUnit = dz / distance;

	float coulumbForceX = COULOMB_CONSTANT_DEFAULT * (m_pNode0->m_fMass * m_pNode1->m_fMass) / pow(distance, power) * xUnit;
	float coulumbForceY = COULOMB_CONSTANT_DEFAULT * (m_pNode0->m_fMass * m_pNode1->m_fMass) / pow(distance, power) * yUnit;
	float coulumbForceZ = COULOMB_CONSTANT_DEFAULT * (m_pNode0->m_fMass * m_pNode1->m_fMass) / pow(distance, power) * zUnit;

	// adding the forces to each nodes resultant force
	m_pNode0->resultantForce[0] += coulumbForceX;
	m_pNode0->resultantForce[1] += coulumbForceY;
	m_pNode0->resultantForce[2] += coulumbForceZ;


	/* Hooke's Law */

	// calculate magnitude 

	directionVectorMagnitude = directionVectorSquared[0] + directionVectorSquared[1] + directionVectorSquared[2];
	directionVectorMagnitude = sqrtf(directionVectorMagnitude);

	for (i = 0; i < 3; i++)
	{
		// normalising the vector
		directionVector[i] = directionVector[i] / directionVectorMagnitude;

		vectorForce0[i] = directionVector[i] * springForce;
		vectorForce1[i] = -(directionVector[i] * springForce);


		// adding the forces to each nodes resultant force
		m_pNode0->resultantForce[i] += vectorForce0[i];
		m_pNode1->resultantForce[i] += vectorForce1[i];
	}

}

void nodeBehaviour(chNode* targetNode)
{
	// acceleration of the node: resultant force / mass of node
	float acceleration[3];

	// 1/60 assumes a constant framerate of 60 frames per second.
	float timeSinceLastFrame = 1.0f / 60.0f;

	// velocity of the node: final velocity = initial velocity + (acceleration * time)
	float velocity[3];

	// motion of node: motion = (velocity * time frame for motion) * 0.5 * (acceleration * time^2)
	float motion[3];

	int i;
	for (i = 0; i < 3; i++)
	{
		acceleration[i] = targetNode->resultantForce[i] / targetNode->m_fMass; // calculate acceleration
		velocity[i] = targetNode->velocity[i] + acceleration[i] * timeSinceLastFrame; // calculate velocity
		motion[i] = (velocity[i] * timeSinceLastFrame) * 0.5 * (acceleration[i] * (timeSinceLastFrame * timeSinceLastFrame)); // calculate motion

		// add motion to the node by increasing its position values
		//targetNode->m_afPosition[i] += motion[i];


		// set target location for the node to travel towards
		targetNode->targetLocation[i] = targetNode->m_afPosition[i] + motion[i];

		// calculate new velocity: velocity = displacement \ motion /time
		velocity[i] = motion[i] / timeSinceLastFrame;

		// apply damping
		targetNode->velocity[i] = velocity[i] * (1.0 - 0.2);
	}



}


void applyMovement(chNode* targetNode)
{
	// calculate distance from current to target node, move node based on the current position + velocity


	float distance[3];
	float travelDistance[3];
	int i;

	for (i = 0; i < 3; i++)
	{
		distance[i] = targetNode->m_afPosition[i] - targetNode->targetLocation[i];

		travelDistance[i] = distance[i] / targetNode->acceleration[i];

		targetNode->m_afPosition[i] += travelDistance[i];
	}
	
}



// mouse callback function
//void mouseCB(int button, int state, int x, int y)
//{
//	if (button == GLUT_RIGHT_BUTTON)
//	{
//		glutCreateMenu();
//	}
//}





// draw the scene. Called once per frame and should only deal with scene drawing (not updating the simulator)
void display() 
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); // clear the rendering buffers

	glLoadIdentity(); // clear the current transformation state
	glMultMatrixf(camObjMat(g_Camera)); // apply the current camera transform

	// draw the grid if the control flag for it is true	
	if (controlActive(g_Control, csg_uiControlDrawGrid)) glCallList(gs_uiGridDisplayList);

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attribute state to enable constrained state changes
	visitNodes(&g_System, nodeDisplay); // loop through all of the nodes and draw them with the nodeDisplay function
	visitArcs(&g_System, arcDisplay); // loop through all of the arcs and draw them with the arcDisplay function
	glPopAttrib();

	glFlush(); // ensure all the ogl instructions have been processed
	glutSwapBuffers(); // present the rendered scene to the screen
}

// processing of system and camera data outside of the renderng loop
void idle() 
{
	controlChangeResetAll(g_Control); // re-set the update status for all of the control flags
	camProcessInput(g_Input, g_Camera); // update the camera pos/ori based on changes since last render
	camResetViewportChanged(g_Camera); // re-set the camera's viwport changed flag after all events have been processed
	glutPostRedisplay();// ask glut to update the screen
	isSimulationOn(); // check status of simulation boolean value

}

// respond to a change in window position or shape
void reshape(int iWidth, int iHeight)  
{
	glViewport(0, 0, iWidth, iHeight);  // re-size the rendering context to match window
	camSetViewport(g_Camera, 0, 0, iWidth, iHeight); // inform the camera of the new rendering context size
	glMatrixMode(GL_PROJECTION); // switch to the projection matrix stack 
	glLoadIdentity(); // clear the current projection matrix state
	gluPerspective(csg_fCameraViewAngle, ((float)iWidth)/((float)iHeight), csg_fNearClip, csg_fFarClip); // apply new state based on re-sized window
	glMatrixMode(GL_MODELVIEW); // swap back to the model view matrix stac
	glGetFloatv(GL_PROJECTION_MATRIX, g_Camera.m_afProjMat); // get the current projection matrix and sort in the camera model
	glutPostRedisplay(); // ask glut to update the screen
}

// detect key presses and assign them to actions
void keyboard(unsigned char c, int iXPos, int iYPos)
{
	switch(c)
	{
	case 'w':
		camInputTravel(g_Input, tri_pos); // mouse zoom
		break;
	case 's':
		camInputTravel(g_Input, tri_neg); // mouse zoom
		break;
	case 'c':
		camPrint(g_Camera); // print the camera data to the comsole
		break;
	case 'g':
		controlToggle(g_Control, csg_uiControlDrawGrid); // toggle the drawing of the grid
		break;
	case 'r':
		if (simulationFlag)// if already true then turn false
		{
			simulationFlag = false;
			//printf("Off -- ");
		}
		else // if already false then turn true
		{
			simulationFlag = true;
			//printf("On -- ");
		}
		break;		
	case 27: // If escape key is pressed, exit the program
		exit(0);
		break;
	}
}

// detect standard key releases
void keyboardUp(unsigned char c, int iXPos, int iYPos) 
{
	switch(c)
	{
		// end the camera zoom action
		case 'w': 
		case 's':
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void sKeyboard(int iC, int iXPos, int iYPos)
{
	// detect the pressing of arrow keys for ouse zoom and record the state for processing by the camera
	switch(iC)
	{
		case GLUT_KEY_UP:
			camInputTravel(g_Input, tri_pos);
			break;
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_neg);
			break;
	}
}

void sKeyboardUp(int iC, int iXPos, int iYPos)
{
	// detect when mouse zoom action (arrow keys) has ended
	switch(iC)
	{
		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			camInputTravel(g_Input, tri_null);
			break;
	}
}

void mouse(int iKey, int iEvent, int iXPos, int iYPos)
{
	// capture the mouse events for the camera motion and record in the current mouse input state
	if (iKey == GLUT_LEFT_BUTTON)
	{
		camInputMouse(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
	else if (iKey == GLUT_MIDDLE_BUTTON)
	{
		camInputMousePan(g_Input, (iEvent == GLUT_DOWN) ? true : false);
		if (iEvent == GLUT_DOWN)camInputSetMouseStart(g_Input, iXPos, iYPos);
	}
}

void motion(int iXPos, int iYPos)
{
	// if mouse is in a mode that tracks motion pass this to the camera model
	if(g_Input.m_bMouse || g_Input.m_bMousePan) camInputSetMouseLast(g_Input, iXPos, iYPos);
}


void myInit()
{
	// setup my event control structure
	controlInit(g_Control);

	// initalise the maths library
	initMaths();

	// Camera setup
	camInit(g_Camera); // initalise the camera model
	camInputInit(g_Input); // initialise the persistant camera input data 
	camInputExplore(g_Input, true); // define the camera navigation mode

	// opengl setup - this is a basic default for all rendering in the render loop
	glClearColor(csg_afColourClear[0], csg_afColourClear[1], csg_afColourClear[2], csg_afColourClear[3]); // set the window background colour
	glEnable(GL_DEPTH_TEST); // enables occusion of rendered primatives in the window
	glEnable(GL_LIGHT0); // switch on the primary light
	glEnable(GL_LIGHTING); // enable lighting calculations to take place
	glEnable(GL_BLEND); // allows transparency and fine lines to be drawn
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // defines a basic transparency blending mode
	glEnable(GL_NORMALIZE); // normalises the normal vectors used for lighting - you may be able to switch this iff (performance gain) is you normalise all normals your self
	glEnable(GL_CULL_FACE); // switch on culling of unseen faces
	glCullFace(GL_BACK); // set culling to not draw the backfaces of primatives

	// build the grid display list - display list are a performance optimization 
	buildGrid();

	// initialise the data system and load the data file
	initSystem(&g_System);
	parse(g_acFile, parseSection, parseNetwork, parseArc, parsePartition, parseVector);
}

int main(int argc, char* argv[])
{
	// check parameters to pull out the path and file name for the data file
	for (int i = 0; i<argc; i++) if (!strcmp(argv[i], csg_acFileParam)) sprintf_s(g_acFile, "%s", argv[++i]);


	if (strlen(g_acFile)) 
	{ 
		// if there is a data file

		glutInit(&argc, (char**)argv); // start glut (opengl window and rendering manager)

		glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA); // define buffers to use in ogl
		glutInitWindowPosition(csg_uiWindowDefinition[csg_uiX], csg_uiWindowDefinition[csg_uiY]);  // set rendering window position
		glutInitWindowSize(csg_uiWindowDefinition[csg_uiWidth], csg_uiWindowDefinition[csg_uiHeight]); // set rendering window size
		glutCreateWindow("chAssignment1-2020");  // create rendering window and give it a name

		buildFont(); // setup text rendering (use outline print function to render 3D text


		myInit(); // application specific initialisation

		// provide glut with callback functions to enact tasks within the event loop
		glutDisplayFunc(display);
		glutIdleFunc(idle);
		glutReshapeFunc(reshape);
		glutKeyboardFunc(keyboard);
		glutKeyboardUpFunc(keyboardUp);
		glutSpecialFunc(sKeyboard);
		glutSpecialUpFunc(sKeyboardUp);
		glutMouseFunc(mouse);
		glutMotionFunc(motion);
		glutMainLoop(); // start the rendering loop running, this will only ext when the rendering window is closed 

		killFont(); // cleanup the text rendering process

		return 0; // return a null error code to show everything worked
	}
	else
	{
		// if there isn't a data file 

		printf("The data file cannot be found, press any key to exit...\n");
		_getch();
		return 1; // error code
	}
}

void buildGrid()
{
	if (!gs_uiGridDisplayList) gs_uiGridDisplayList= glGenLists(1); // create a display list

	glNewList(gs_uiGridDisplayList, GL_COMPILE); // start recording display list

	glPushAttrib(GL_ALL_ATTRIB_BITS); // push attrib marker
	glDisable(GL_LIGHTING); // switch of lighting to render lines

	glColor4fv(csg_afDisplayListGridColour); // set line colour

	// draw the grid lines
	glBegin(GL_LINES);
	for (int i = (int)csg_fDisplayListGridMin; i <= (int)csg_fDisplayListGridMax; i++)
	{
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMin*csg_fDisplayListGridSpace);
		glVertex3f(((float)i)*csg_fDisplayListGridSpace, 0.0f, csg_fDisplayListGridMax*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMin*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
		glVertex3f(csg_fDisplayListGridMax*csg_fDisplayListGridSpace, 0.0f, ((float)i)*csg_fDisplayListGridSpace);
	}
	glEnd(); // end line drawing

	glPopAttrib(); // pop attrib marker (undo switching off lighting)

	glEndList(); // finish recording the displaylist
}
