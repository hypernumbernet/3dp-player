// @author Tomohito Inoue <hypernumbernet@users.noreply.github.com>

#include "stdafx.h"
using namespace my_math;

//再描画時間間隔(ミリ秒)
//コメントアウトすると、待ち時間なし
#define RECALL_TIME 16
//#define RECALL_TIME 1000

// 視点位置初期値
#define EYE_RADIUS_START 8

//球のパラメータ
#define SPHERE_RADIUS 0.01
#define SPHERE_SLICES 8
#define SPHERE_STACKS 4
#define NUM_COLOR 6

//データファイル関連
#define VERSION 1
#define HEADER_SIZE 256

//using namespace math;

//粒子数
int nParticle;

Vector3<GLdouble> *locat;
HANDLE hFile;
DWORD wReadSize;

GLfloat black[]  = { 0.0, 0.0, 0.0, 0.0 };
GLfloat white[]  = { 1.0, 1.0, 1.0, 1.0 };

GLfloat ambient_set[][4] = 
{
    { 0.5, 0.2, 0.2, 1.0 },//red
    { 0.2, 0.2, 0.5, 1.0 },//blue
    { 0.2, 0.5, 0.2, 1.0 },//green
    { 0.4, 0.4, 0.2, 1.0 },//yellow
    { 0.4, 0.2, 0.4, 1.0 },
    { 0.2, 0.4, 0.4, 1.0 }
};

GLfloat diffuse_set[][4] = 
{
    { 0.8, 0.0, 0.0, 1.0 },
    { 0.0, 0.0, 0.8, 1.0 },
    { 0.0, 0.8, 0.0, 1.0 },
    { 0.8, 0.8, 0.0, 1.0 },
    { 0.8, 0.0, 0.8, 1.0 },
    { 0.0, 0.8, 0.8, 1.0 }
};
GLfloat specular[]  = {0.0};
GLfloat shininess[] = {1.0};

GLfloat emission_set[][4] = 
{
    { 0.8, 0.2, 0.2, 1.0 },//red
    { 0.6, 0.0, 0.0, 1.0 },
    { 0.8, 0.8, 0.2, 1.0 },//yellow
    { 0.5, 0.5, 0.8, 1.0 },//blue
    { 0.2, 0.2, 0.8, 1.0 },
    { 0.8, 0.8, 0.8, 1.0 }
};

GLfloat light0_ambient[]        = { 0, 0, 0, 0 };
GLfloat light0_diffuse[]        = { 0, 0, 0, 0 };
GLfloat light0_specular[]       = { 0, 0, 0, 0 };
GLfloat light0_position[]       = { 0, 0, 0, 0 };
GLfloat light0_spot_direction[] = { 0, 0, 0 };

GLfloat light1_ambient[]        = { 1.0 , 1.0 , 1.0, 1.0};
GLfloat light1_diffuse[]        = { 0.8 , 0.8 , 0.8, 1.0};
GLfloat light1_specular[]       = { 0.5 , 0.5 , 0.5, 1.0};
GLfloat light1_position[]       = { 9.0 , 9.0 , 5.0, 1.0};
GLfloat light1_spot_direction[] = { 0.0 , 0.0 , 0.0};

GLint mouse_down_x, mouse_down_y;
GLdouble eye_theta, eye_phi, eye_radius;
GLuint xyzLines;
GLuint xyzGrid;
int xyzLinesDisplay = 0;
bool play = true;
int colorSet = 0;

void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
		//Grid line
        case 'g' : 
			++ xyzLinesDisplay;
			if (xyzLinesDisplay == 3) 
				xyzLinesDisplay = 0; 
			break;
		//Pause/Play
        case 'p' : play = ! play; break;
		//Back one frame
        case 'b' : 
            wReadSize = SetFilePointer(hFile, -nParticle * 48, NULL, FILE_CURRENT);
            if (wReadSize == INVALID_SET_FILE_POINTER)
                SetFilePointer(hFile, HEADER_SIZE, NULL, FILE_BEGIN);
            ReadFile(hFile , locat , nParticle * 24 , &wReadSize , NULL);
            break;
		//Forward one frame
        case 'f' : 
            ReadFile(hFile , locat , nParticle * 24 , &wReadSize , NULL);
            break;
		//jump Start 
        case 's' : 
            SetFilePointer(hFile, HEADER_SIZE, NULL, FILE_BEGIN);
            ReadFile(hFile , locat , nParticle * 24 , &wReadSize , NULL);
            break;
		case 'c' :
			++ colorSet;
			if (colorSet == 2)
				colorSet = 0;
			break;
    }
}
void skeyboard(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_PAGE_UP   : eye_radius -= 0.2; break;
        case GLUT_KEY_PAGE_DOWN : eye_radius += 0.2; break;

		case GLUT_KEY_LEFT  : eye_theta -= 0.4; break;
		case GLUT_KEY_RIGHT : eye_theta += 0.4; break;
		case GLUT_KEY_UP    : eye_phi   += 0.4; break;
		case GLUT_KEY_DOWN  : eye_phi   -= 0.4; break;

		case GLUT_KEY_F1 :
			MessageBox
			(
				NULL, 
				TEXT("[Mouse Drag] : Rotation\n\n[Page Up/Down] : Zoom In/Out\n[Up/Down/Right/Left] : Rotation\n\ng : Grid line\np : Pause/Play\nb : play Back\nf : play Forward\ns : jump Start\nc : change Color set"), 
				TEXT("Help"), 
				MB_OK
			);
			break;
    }
}
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            mouse_down_x = x;
            mouse_down_y = y;
        }
    }
}
void motion(int x, int y)
{
    eye_theta -= (x - mouse_down_x);
    eye_phi   += (y - mouse_down_y);
    mouse_down_x = x;
    mouse_down_y = y;
}

void gridline(double i, double j, double k, double di, double dj, double dk)
{
	Quaternion<double> q1, q2;
	Vector3<double> v1;
	double r = 32.0;
	int count = 0;

	q1.i0 = 16.0;
	q1.i1 = i;
	q1.i2 = j;
	q1.i3 = k;

	q1.Normalize();

	q2 = Quaternion<double>::Exp(di, dj, dk);

	for (count = 0; count < 2; ++count)
	{

		v1 = q1.LnV3() * r;
		glVertex3d(v1.x, v1.y, v1.z);

		q1 *= q2;

		v1 = q1.LnV3() * r;
		glVertex3d(v1.x, v1.y, v1.z);

	}

	q1.i0 = 16.0;
	q1.i1 = i;
	q1.i2 = j;
	q1.i3 = k;

	q1.Normalize();

	q2 = Quaternion<double>::Exp(-di, -dj, -dk);

	for (count = 0; count < 2; ++count)
	{

		v1 = q1.LnV3() * r;
		glVertex3d(v1.x, v1.y, v1.z);

		q1 *= q2;

		v1 = q1.LnV3() * r;
		glVertex3d(v1.x, v1.y, v1.z);

	}
}
void scene(void)
{
	double j, k;

    xyzLines = glGenLists(1);
    glNewList(xyzLines, GL_COMPILE);

    glColor3d(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3d( 0.0, 0.0, 0.0);
    glVertex3d(10.0, 0.0, 0.0);
    glEnd();

    glColor3d(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3d( 0.0,  0.0, 0.0);
    glVertex3d( 0.0, 10.0, 0.0);
    glEnd();

    glColor3d(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3d( 0.0, 0.0,  0.0);
    glVertex3d( 0.0, 0.0, 10.0);
    glEnd();

	glEndList();


    xyzGrid = glGenLists(1);
    glNewList(xyzGrid, GL_COMPILE);

	glColor3d(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
	k = 0;
	for (j = -3.0; j < 4.0; j++)
		gridline(0, j, k, 0.1, 0.0, 0.0);
	j = 0;
	for (k = -3.0; k < 4.0; k++)
		gridline(0, j, k, 0.1, 0.0, 0.0);
    glEnd();

    glColor3d(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
	k = 0;
	for (j = -3.0; j < 4.0; j++)
		gridline(j, 0, k, 0.0, 0.1, 0.0);
	j = 0;
	for (k = -3.0; k < 4.0; k++)
		gridline(j, 0, k, 0.0, 0.1, 0.0);
    glEnd();

    glColor3d(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
	k = 0;
	for (j = -3.0; j < 4.0; j++)
		gridline(j, k, 0, 0.0, 0.0, 0.1);
	j = 0;
	for (k = -3.0; k < 4.0; k++)
		gridline(j, k, 0, 0.0, 0.0, 0.1);
    glEnd();

    glEndList();
}
void display(void)
{
    if (play)
        ReadFile(hFile , locat , nParticle * 24 , &wReadSize , NULL);

	if (eye_phi >= 90.0)
    {
        eye_phi = 89.9999;
    }
    else if (eye_phi <= -90.0)
    {
        eye_phi = -89.9999;
    }
    GLdouble theta = DegreeToRadian(eye_theta);
    GLdouble phi   = DegreeToRadian(eye_phi  );
    int i;
    int j;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt
    (
        eye_radius * sin(theta) * cos(phi),
        eye_radius * sin(phi),
        eye_radius * cos(theta) * cos(phi),
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0
    );

    if (xyzLinesDisplay == 1)
        glCallList(xyzLines);
	else if(xyzLinesDisplay == 2)
        glCallList(xyzGrid);

	glEnable(GL_LIGHTING);

	if (colorSet == 0)
	{
		glEnable(GL_LIGHT0);
		glDisable(GL_LIGHT1);

		glMaterialfv(GL_FRONT, GL_AMBIENT, white);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, white);

		glMaterialfv(GL_FRONT, GL_SPECULAR , black);
		glMaterialfv(GL_FRONT, GL_SHININESS, black);
	}
	else if (colorSet == 1)
	{
		glEnable(GL_LIGHT1);
		glDisable(GL_LIGHT0);

		glLightfv(GL_LIGHT1, GL_POSITION      , light1_position      );
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_spot_direction);

		glMaterialfv(GL_FRONT, GL_SPECULAR , specular );
		glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION , black    );
	}

    if (wReadSize == nParticle * 24)
    {
        for (i = 0; i < nParticle; i++)
        {
            glPushMatrix();
            glTranslated(locat[i].x, locat[i].y, locat[i].z);
            
			j = i % NUM_COLOR;
            
			if (colorSet == 0)
				glMaterialfv(GL_FRONT, GL_EMISSION, emission_set[j]);
			else if (colorSet == 1)
			{
				glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_set[j]);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_set[j]);
			}
            
			
            glutSolidSphere(SPHERE_RADIUS, SPHERE_SLICES, SPHERE_STACKS);
            glPopMatrix();
        }
    }

    glDisable(GL_LIGHTING);

    glutSwapBuffers();
}
void reshape(int width, int height)
{
    GLdouble ratio = (GLdouble)height / (GLdouble)width;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -ratio, ratio, 1.0, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

#ifdef RECALL_TIME

void timer(int value)
{
    glutTimerFunc(RECALL_TIME, timer, 0);
    glutPostRedisplay();
}

#else

void idle()
{
    glutPostRedisplay();
}

#endif

void init(void)
{
    //int i, j;

    //初期視点位置
    eye_theta  = 30.0;
    eye_phi    = 15.0;
    eye_radius = EYE_RADIUS_START;

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	glLightfv(GL_LIGHT0, GL_AMBIENT       , light0_ambient       );
	glLightfv(GL_LIGHT0, GL_DIFFUSE       , light0_diffuse       );
	glLightfv(GL_LIGHT0, GL_SPECULAR      , light0_specular      );
	glLightfv(GL_LIGHT0, GL_POSITION      , light0_position      );
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_spot_direction);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT1, GL_AMBIENT       , light1_ambient       );
	glLightfv(GL_LIGHT1, GL_DIFFUSE       , light1_diffuse       );
	glLightfv(GL_LIGHT1, GL_SPECULAR      , light1_specular      );
	glLightfv(GL_LIGHT1, GL_POSITION      , light1_position      );
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_spot_direction);
	//glEnable(GL_LIGHT1);

    glDisable(GL_LIGHTING);
}

//WinMainで始めないと、コンソール窓が開いてしまう。
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
//int _tmain(int argc, _TCHAR* argv[])
{
	//コマンドライン引数を得る。UnicodeはWinMainから取得しにくい。
	int    nArgs;
	LPWSTR *lplpszArgs;
	lplpszArgs = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	if (nArgs < 2)
    {
		MessageBox(NULL, TEXT("データファイルをドロップしてください。"), TEXT("エラー"), MB_OK);
		return 1;
	}

	hFile = CreateFileW
    (
		//TEXT("out.dat") ,
		//lpCmdLine,
		lplpszArgs[1] ,
        GENERIC_READ , 
        0 , 
        NULL ,
		OPEN_EXISTING , 
        FILE_FLAG_SEQUENTIAL_SCAN/*FILE_ATTRIBUTE_NORMAL*/ , 
        NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
    {
		MessageBox(NULL, TEXT("ファイルを開けませんでした"), TEXT("エラー"), MB_OK);
		//MessageBox(NULL, GetCommandLine(), TEXT("エラー"), MB_OK);
		return 1;
	}

    int header[HEADER_SIZE / 4];
    ReadFile(hFile , header , HEADER_SIZE , &wReadSize , NULL);
    if (wReadSize != HEADER_SIZE)
    {
		MessageBox(NULL, TEXT("ヘッダ読み込みエラー"), TEXT("エラー"), MB_OK);
		return 1;
	}
    nParticle = header[0];
    locat = new Vector3<double> [nParticle];

    //glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);

    glutCreateWindow("3D Particles Player (F1 for HELP)");

    glutDisplayFunc (display  );
    glutReshapeFunc (reshape  );
    glutMotionFunc  (motion   );
    glutKeyboardFunc(keyboard );
    glutSpecialFunc (skeyboard);
    glutMouseFunc   (mouse    );

    init();
    scene();

#ifdef RECALL_TIME

    glutTimerFunc(RECALL_TIME, timer, 0);

#else

    glutIdleFunc(idle);

#endif

    glutMainLoop();

	//これ以下は実行されない。
	//delete [] locat;
	//CloseHandle(hFile);

	return 0;
}
