#include <windows.h>
#include <gl/gl.h>
#include <math.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

struct {
    float x,y,z;
    float Xr,Zr;
} camera;
void Camera_Apply() {
    glRotatef(-camera.Xr, 1,0,0);
    glRotatef(-camera.Zr, 0,0,1);
    glTranslatef(-camera.x, -camera.y, -camera.z);
}
void Camera_Rotation(float xAngle, float zAngle) {
    camera.Zr += zAngle;
    if (camera.Zr < 0) camera.Zr += 360;
    if (camera.Zr > 360) camera.Zr -= 360;
    camera.Xr  += xAngle;
    if (camera.Xr < 0) camera.Xr = 0;
    if (camera.Xr > 180) camera.Xr = 180;
}
void Camera_Move(int forwardMove, int rightMove, float speed) {
    float kray = -camera.Zr / 180 * M_PI;

    if (forwardMove > 0)
        kray += rightMove > 0 ? M_PI_4 : (rightMove < 0 ? -M_PI_4 : 0);
    if (forwardMove < 0)
        kray += M_PI + (rightMove > 0 ? -M_PI_4 : (rightMove < 0 ? M_PI_4 : 0));
    if (forwardMove == 0)
    {
        kray += rightMove > 0 ? M_PI_2 : -M_PI_2;
        if (forwardMove == 0) speed = 0;
    }

    if (speed != 0)
    {
        camera.x += sin(kray) * speed;
        camera.y += cos(kray) * speed;
    }
}
void PlayerMove(int centerX, int centerY, float speed) {
    POINT cursor;
    POINT base = {centerX, centerY};
    GetCursorPos(&cursor);
    Camera_Rotation( (base.y - cursor.y) / 5.0, (base.x - cursor.x) / 5.0 );
    SetCursorPos(base.x, base.y);
}
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "SabachkaEngine /C/ DoL";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "SabachkaEngine /C/ DoL",
                          "SabachkaEngineHOST /langue/C/ DoL",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          600,
                          600,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
                if (GetForegroundWindow() == hwnd)
                    PlayerMove(400, 400, 2);
                    Camera_Move( GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0)
                    ,GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0)
                    ,0.1);
                Camera_Apply();

                glBegin(GL_TRIANGLES);

                    glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f,  1.0f);
                    glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f,  -0.5f);
                    glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);

                glEnd();

            glPopMatrix();

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_SETCURSOR:
            ShowCursor(FALSE);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

