#include <Windows.h>
#include <vector>
using namespace std;
typedef struct
{
	int x;
	int y;
}Point;
typedef struct
{
	Point point;
	int penIndex;
}Trace;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	static WCHAR szWndClassName[] = L"paintwindow";	//窗口类名，用于创建窗口类
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;		//创建窗口类

	wndclass.style = CS_HREDRAW | CS_VREDRAW;		//窗口类风格：窗口在水平和垂直方向 都重绘
	wndclass.lpfnWndProc = WndProc;						//窗口过程函数指针
	wndclass.cbClsExtra = 0;										//额外空间，留空
	wndclass.cbWndExtra = 0;										//同上
	wndclass.hInstance = hInstance;							//窗口句柄，从WinMain参数获取
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//窗口图标
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);	//窗口光标
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);	//背景画刷
	wndclass.lpszMenuName = NULL;							//菜单名
	wndclass.lpszClassName = szWndClassName;		//窗口类名

	if (!RegisterClass(&wndclass))	/*注册窗口类*/
	{
		MessageBox(NULL, L"注册失败", L"错误", MB_ICONERROR);
		return 0;
	}

	/*创建窗口，并传给窗口句柄*/
	hwnd = CreateWindow(
		szWndClassName,		//window class name
		L"画板",				//window caption
		WS_OVERLAPPEDWINDOW,		//window style
		CW_USEDEFAULT,		//initial x position
		CW_USEDEFAULT,		//initial y position
		CW_USEDEFAULT,		//initial x size
		CW_USEDEFAULT,		//initial y size
		NULL,						//parent window handle
		NULL,						//window menu handle
		hInstance,					//program instance handle
		NULL						//creation paraneters
	);

	ShowWindow(hwnd, nCmdShow);		//显示窗口
	UpdateWindow(hwnd);						//规定机制，更新窗口

	while (GetMessage(&msg, NULL, 0, 0))		//消息队列
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);			//分发消息，将消息传给系统，系统再传给WndProc窗口过程
	}
	return msg.wParam;		//WM_QUIT :  Windows Message Quit
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)		/*窗口过程函数，用于响应消息，操作系统调用*/
{
	HDC hdc;
	PAINTSTRUCT ps;
	
	RECT rect;
	static Point prePoint;		//前一个点
	static HPEN hpen[3];
	static int index;
	static vector<Trace> trace;	//记录轨迹以重绘

	switch (uMsg)
	{
	case WM_CREATE:		//窗口创建
	{
		hpen[0] = CreatePen(PS_SOLID, 4, RGB(255, 0, 0));		//创建画笔
		hpen[1] = CreatePen(PS_SOLID, 6, RGB(0, 255, 0));
		hpen[2] = CreatePen(PS_SOLID, 8, RGB(0, 0, 255));
	}
	return 0;

	case WM_LBUTTONDOWN:		//鼠标左键单击时取前点
	{
		prePoint.x = LOWORD(lParam);
		prePoint.y = HIWORD(lParam);
		Trace temp;
		temp.point = prePoint;
		temp.penIndex = index;
		trace.push_back(temp);
	}
	return 0;

	case WM_MOUSEMOVE:
	{
		if (MK_LBUTTON&wParam)		//位运算取指定位，判断鼠标左键被按下
		{
			Point point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			hdc = GetDC(hwnd);		//返回设备描述表句柄
			
			SelectObject(hdc, hpen[index]);
			MoveToEx(hdc, prePoint.x, prePoint.y, NULL);
			LineTo(hdc, point.x, point.y);
			
			Trace temp;
			temp.point = point;
			temp.penIndex = index;
			trace.push_back(temp);

			ReleaseDC(hwnd, hdc);
			prePoint = point;
		}
	}
		return 0;

	case WM_RBUTTONDOWN:
	{
		index++;
		if (index >= 3)
			index = 0;
	}
	return 0;

	case WM_KEYDOWN:
		if (VK_RETURN == wParam)
		{
			InvalidateRect(hwnd, NULL, true);			//使整个矩形区域无效，以触发 case WM_PAINT 重绘
		}
		return 0;

	case WM_PAINT:		//窗口必须重绘的时候:最大化/最小化/调整窗口大小等时候，完成后会清除消息队列的WM_PAINT
		hdc = BeginPaint(hwnd, &ps);		//返回设备描述表句柄 (无效区域)
		GetClientRect(hwnd, &rect);			//窗口客户区矩形
		/*提示字符*/
		DrawText(hdc, L"提示：左键绘画，右键换笔", -1, &rect, DT_SINGLELINE | DT_TOP | DT_CENTER);
		/*绘图区域*/
		Point prePoint;
		Point point;
		if (trace.size() > 0)
		{
			prePoint = trace[0].point;
		}
		for(int i=1;i<trace.size();i++)
		{
			point = trace[i].point;
			SelectObject(hdc, hpen[trace[i].penIndex]);
			MoveToEx(hdc, prePoint.x, prePoint.y, NULL);
			LineTo(hdc, point.x, point.y);
			prePoint = point;
		}
		EndPaint(hwnd, &ps);

		return 0;	//只要处理了消息，就返回0

	case WM_DESTROY:	//程序关闭的时候
		DeleteObject(hpen[0]);		//删除画笔
		DeleteObject(hpen[1]);
		DeleteObject(hpen[2]);

		PostQuitMessage(0);		//发送Quit消息到消息队列
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);		//其他消息给默认窗口过程处理：窗口拖动等时候
}