// Arkanoid.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "Arkanoid.h"


HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
RECT window = {};
HDC hdc;

bool active = true;
bool keys[256];
const int wallWidth = WALL_WIDTH;
const int wallHeight = WALL_HEIGHT;
bool bricks[wallHeight * wallWidth];
int color[wallHeight * wallWidth];
float stickPosition = START_STICK_POSITION;
RECT stick = {};
float ballV = START_BALL_VECTOR;
float ballX = NULL_XY;
float ballY = NULL_XY;
bool pause = false;
bool done = false;
bool start = false;
float stickSpeed = STICK_SPEED;
float ballSpeed = BALL_SPEED;
float boost = 0;
int life = LIFE;
bool isBonus[wallHeight * wallWidth];
int bonusX[wallHeight * wallWidth];
float bonusY[wallHeight * wallWidth];
int bonus[wallHeight * wallWidth];
int bonusP = BONUS_PROBABILITY;
int bonusLifeP = BONUS_LIFE_PROBABILITY;



RECT getBrick(int i)
{
	int widthShift = window.right / wallWidth / 30;
	int heightShift = window.bottom / wallHeight / 50;
	RECT brick = {};
	brick.left = (i % wallWidth) * (window.right / wallWidth) + ((window.right % wallWidth) / 2) + widthShift;
	brick.top = (i / wallWidth + 3) * (window.bottom / wallHeight / 4) + heightShift;
	brick.right = (i % wallWidth + 1) * (window.right / wallWidth) - widthShift;
	brick.bottom = (i / wallWidth + 4) * (window.bottom / wallHeight / 4) - heightShift;

	return brick;
}

void createBricks()
{
	for (int i = 0; i < wallWidth * wallHeight; ++i)
	{
		bricks[i] = true;
		int mark = rand() % 30;
		if (mark == 9)
		{
			color[i] = 3;
			continue;
		}
		mark = rand() % 5;
		if (mark == 4)
			color[i] = 2;
		else
			color[i] = 1;
	}
}

int getRadius()
{
	return ((window.bottom / wallHeight / 3 / 2) + (window.right / wallWidth / 6) )/ 2;
}

//функция для обновления кадра
int DrawFrame(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC oldDc = GetDC(hwnd);
	int shift = 40 * pause;
	
	HBRUSH brush;
	GetWindowRect(hwnd, &window);
	window.right = window.right - window.left - 15;
	window.bottom = window.bottom - window.top - 60;
	window.left = 0;
	window.top = 0;

	HDC memDc = CreateCompatibleDC(oldDc);
	HBITMAP memBM = CreateCompatibleBitmap(oldDc, window.right, window.bottom);
	SelectObject(memDc, memBM);
	 
	//рисуем фон темного цвета
	brush = CreateSolidBrush(RGB(60 + shift, 67 + shift, 73 + shift));
	SelectObject(memDc, brush);
	Rectangle(memDc, 0, 0, window.right, window.bottom );
	DeleteObject(brush);

	//выводим количество жизней
	std::wstring text = (std::wstring) (L"ЖИЗНЕЙ: ") + std::to_wstring(life);
	LPWSTR wstr = &text[0];
	wstr[text.size()] = 0;

	TextOut(memDc, window.right - 9 * text.size(), 0, wstr, text.size() + 1);

	/*brush = CreateSolidBrush(RGB(71, 127, 49));
	SelectObject(dc, brush);
	Rectangle(dc, 0, 0, window.right / wallWidth - 1, window.bottom / wallHeight / 3 - 1);
	DeleteObject(brush);*/


	//рисуем кирпичи
	for (int i = 0; i < wallHeight * wallWidth; ++i)
	{
		if (bricks[i] == false)
			continue;
				
		switch (color[i])
		{
		case 1:
			brush = CreateSolidBrush(RGB(123 + shift, 181 + shift, 101 + shift));
			break;
		case 2:
			brush = CreateSolidBrush(RGB(118 + shift, 49 + shift, 132 + shift));
			break;
		case 3:
			//brush = CreateSolidBrush(RGB(34, 125, 155));
			brush = CreateSolidBrush(RGB(206 + shift, 206 + shift, 206 + shift));
			break;
		}
		
		SelectObject(memDc, brush);

		// находим координаты очередного кирпича
		RECT brick = getBrick(i);

		Rectangle(memDc, brick.left, brick.top, brick.right, brick.bottom);
		DeleteObject(brush);
	}
	
	//нарисуем бонусы, если они есть
	for (int i = 0; i < wallHeight * wallWidth; ++i)
	{
		if (isBonus[i])
		{
			switch (bonus[i])
			{
			case 0: //бонус в виде дополнительной жизни, красного цвета
				brush = CreateSolidBrush(RGB(204 + shift, 49 + shift, 14 + shift));
				break;
			case 1: //двойное ускорение каретки, синего цвета
				brush = CreateSolidBrush(RGB(50 + shift, 74 + shift, 211 + shift));
				break;
			case 2:  //двойное замедление каретки, фиолетового цвета
				brush = CreateSolidBrush(RGB(186 + shift, 57 + shift, 204 + shift));
				break;
			}
			SelectObject(memDc, brush);

			RECT brick = getBrick(i);
			bonusX[i] = brick.left + (brick.right - brick.left) / 2;
			Ellipse(memDc, bonusX[i] - getRadius(), bonusY[i] * window.bottom - getRadius(),
				bonusX[i] + getRadius(), bonusY[i] * window.bottom + getRadius());
			DeleteObject(brush);
		}
	}
	

	//рисуем каретку
	brush = CreateSolidBrush(RGB(123 + shift, 181 + shift, 101 + shift));
	SelectObject(memDc, brush);
	stick.left = (window.right * stickPosition) - (window.right / wallWidth);
	stick.top = window.bottom - window.bottom / wallHeight / 6;
	stick.right = (window.right * stickPosition) + (window.right / wallWidth);
	stick.bottom = window.bottom;

	Rectangle(memDc, stick.left, stick.top, stick.right, stick.bottom);
	DeleteObject(brush);


	//рисуем шар
	int r = getRadius();
	brush = CreateSolidBrush(RGB(186 + shift, 177 + shift, 7 + shift));
	SelectObject(memDc, brush);
	if (ballY == -100.0f)
	{
		ballY = stick.top - r - 1;
		ballY = ballY / window.bottom;
	}
	if (ballX == -100.0f)
	{
		ballX = START_STICK_POSITION;
	}
	if (!start)
	{
		ballX = stickPosition;
	}
	Ellipse(memDc, ballX * window.right - r, ballY * window.bottom - r, ballX * window.right + r, ballY * window.bottom + r);
	DeleteObject(brush);

	//если мы на паузе, то скажем это
	if (pause)
	{
		HFONT hfont = CreateFont(30, 0, 0, 0, FW_BOLD, false, 0, 0, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, 
			CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH, _T("Arial"));
		SelectObject(memDc, hfont);
		TextOut(memDc, window.right / 2 - 150, window.bottom / 2 - 100, _T("ИГРА ПРИОСТАНОВЛЕНА"), 19);
		DeleteObject(hfont);
	}

	BitBlt(oldDc, 0, 0, window.right, window.bottom, memDc, 0, 0, SRCCOPY);

	ReleaseDC(hwnd, oldDc);
	ReleaseDC(hwnd, memDc);
	DeleteObject(memBM);	
	DeleteDC(oldDc);
	DeleteDC(memDc);
	return 0;
}

// обнуление - рестарт
void startPosition()
{
	stickPosition = START_STICK_POSITION;
	ballV = START_BALL_VECTOR;
	ballX = NULL_XY;
	ballY = NULL_XY;
	pause = false;
	start = false;
	stickSpeed = STICK_SPEED;
	ballSpeed = BALL_SPEED;
	boost = 0;
	for (int i = 0; i < wallHeight * wallWidth; ++i)
	{
		isBonus[i] = false;
	}
	
}

void Rebuild()
{
	createBricks();
	startPosition();
	life = LIFE;
}

void moveStick(float delta)
{
	if (keys[VK_LEFT] && stick.left > 0)
		if (keys[VK_SHIFT])
			stickPosition -= 0.2f * delta / 1000 * stickSpeed * 2;
		else
			stickPosition -= 0.2f * delta / 1000 * stickSpeed;
	if (keys[VK_RIGHT] && stick.right < window.right)
		if (keys[VK_SHIFT])
			stickPosition += 0.2f * delta / 1000 * stickSpeed * 2;
		else
			stickPosition += 0.2f * delta / 1000 * stickSpeed;
}

void showMessage(LPWSTR text)
{
	int msgboxID = MessageBox(
		NULL,
		text,
		(LPCWSTR)L"One more game?",
		MB_RETRYCANCEL | MB_DEFBUTTON2 | MB_ICONASTERISK
	);
	switch (msgboxID)
	{
	case IDRETRY:
		Rebuild();
		break;
	case IDCANCEL:
		done = true;
		PostQuitMessage(0);
		break;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_ARKANOID, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARKANOID));

	MSG msg;
	HWND hwnd = GetActiveWindow();

	createBricks();

	hdc = GetDC(hwnd);

	int oldTime = GetTickCount();

	while (!done)
	{
		int time = GetTickCount();
		float delta = time - oldTime;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				done = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (active)
			{
				int r = getRadius();
				if (!pause && start)
				{
					// двигаем шар
					boost += delta / 10000;
					ballX += (float)cos(ballV * PI / 180) * (ballSpeed + boost) / 1000 * 0.5f;  // 1000 / 0.5 было норм без дельты
					ballY += (float)sin(ballV * PI / 180) * (ballSpeed + boost) / 3000 * 1.2f;  // 3000 / 1.2 было норм без дельты

					//отражение от боковых стенок
					if (ballX * window.right + r >= window.right || ballX * window.right - r <= window.left)
					{
						ballV = 180 - ballV;
					}
					//отражение от потолка
					if (ballY * window.bottom - r <= 0)
					{
						ballV = -ballV;
					}
					//если мяч коснулся земли
					if ((ballY * window.bottom + r >= window.bottom) || (ballY >= 1) || (ballY <= 0) || (ballX >= 1) || (ballX <= 0))
					{
						--life;
						if (life < 0)
						{
							showMessage(_T("Вы проиграли!"));
							Rebuild();
						}							
						else
							startPosition();
					}

					//отражение от каретки
					if ((ballY * window.bottom + r >= stick.top) && (ballX * window.right + r >= stick.left) && (ballX * window.right - r <= stick.right))
					{
						ballV = -ballV;
						if ((ballX * window.right < stick.left) || (ballX * window.right > stick.right))
							ballV = 180 - ballV;
						else
						{
							float tmp = (stickPosition - ballX) * window.right / (stick.right - stick.left) / 2;
							ballV = -(acos(-tmp) * 180 / PI);							
						}
					}

					// проверяем на пересечение с кирпичами
					for (int i = 0; i < wallHeight * wallWidth; ++i)
					{
						if (!bricks[i])
							continue;

						RECT brick = getBrick(i);

						if (ballX * window.right + r > brick.left &&
							ballX * window.right - r < brick.right &&
							ballY * window.bottom + r > brick.top &&
							ballY * window.bottom - r < brick.bottom)
						{
							if (color[i] < 3 && color[i] > 0)
								--color[i];
							if (color[i] == 0)
							{
								bricks[i] = false;

								//если разбили кирпич, то может появиться бонус
								if (!isBonus[i])
								{
									int chance = rand() % bonusP;
									if (chance == 0)
									{
										isBonus[i] = true;
										bonusX[i] = brick.left + (brick.right - brick.left) / 2;
										float part = brick.top + (brick.bottom - brick.top) / 2;
										bonusY[i] = part / window.bottom;
										int bonusType = rand() % bonusLifeP;
										if (bonusType == 0)
											bonus[i] = 0;
										else
											bonus[i] = rand() % 2 + 1;
									}
								}
							}
								
							
							//отражение от кирпича
							if (ballX * window.right < brick.left || ballX * window.right > brick.right)
								ballV = 180 - ballV;
								
							if (ballY * window.bottom < brick.top || ballY * window.bottom > brick.bottom)
								ballV = -ballV;

							if (ballX * window.right < brick.left && ballY * window.bottom > brick.bottom)
							{
								float x = brick.left - ballX * window.right;
								float y = ballY * window.bottom - brick.bottom;
								if (x > y)
									ballV = -ballV;
								else
									ballV = 180 - ballV;
							}
							if (ballX * window.right < brick.left && ballY * window.bottom < brick.top)
							{
								float x = brick.left - ballX * window.right;
								float y = -ballY * window.bottom + brick.top;
								if (x > y)
									ballV = -ballV;
								else
									ballV = 180 - ballV;
							}
							if (ballX * window.right > brick.right && ballY * window.bottom > brick.bottom)
							{
								float x = -brick.right + ballX * window.right;
								float y = ballY * window.bottom - brick.bottom;
								if (x > y)
									ballV = -ballV;
								else
									ballV = 180 - ballV;
							}
							if (ballX * window.right > brick.right && ballY * window.bottom < brick.top)
							{
								float x = -brick.right + ballX * window.right;
								float y = -ballY * window.bottom - brick.top;
								if (x > y)
									ballV = -ballV;
								else
									ballV = 180 - ballV;
							}

							break;
						}
						
					}

					//двигаем бонус, если он есть
					int br = getRadius();
					for (int i = 0; i < wallHeight * wallWidth; ++i)
					{
						if (isBonus[i])
						{
							bonusY[i] += (ballSpeed) / 3000 * 1.2f / 4;
							//ballY += (float)sin(ballV * PI / 180) * (ballSpeed + boost) / 3000 * 1.2f;

							if (bonusY[i] >= 1)
								isBonus[i] = false;
							
							if ((isBonus[i]) && (bonusX[i] + br >= stick.left) && (bonusX[i] - br <= stick.right) &&
								(bonusY[i] * window.bottom + br >= stick.top) && (bonusY[i] * window.bottom - br <= stick.bottom))
							{
								isBonus[i] = false;
								switch (bonus[i])
								{
								case 0:
									++life;
									break;
								case 1:
									stickSpeed = stickSpeed * 2;
									break;
								case 2:
									stickSpeed = stickSpeed / 2;
								}
							}
						}
					}
					

					//двигаем каретку
					moveStick(delta);

					//проверка на завершение
					bool isFinish = false;
					for (int i = 0; i < wallWidth * wallHeight; ++i)
						isFinish = isFinish || (bricks[i] && color[i] < 3);
					if (!isFinish)
					{
						showMessage(_T("Поздравляем! Вы победили!"));
					}
						
				}

				if (!pause && !start)
				{
					moveStick(delta);
				}
				

				DrawFrame(hwnd);
			}
		}
		oldTime = GetTickCount();
		Sleep(1000 / 500);
	}

	return 0;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ARKANOID));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ARKANOID);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATE:
	{
		if (!HIWORD(wParam))
			active = true;
		else
			active = false;
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			done = true;
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 0, 0, _T("ХОЧУ ЗАЧЕТ"), 10);
		EndPaint(hWnd, &ps);
		DrawFrame(hWnd);
	}
	break;
	case WM_KEYDOWN:
	{
		keys[wParam] = true;
		if (keys[VK_RETURN])
			start = true;
		if (keys[VK_ESCAPE])
			Rebuild();
		if (start && keys[VK_SPACE])
		{
			pause = 1 - pause;
		}
			
	}
	break;
	case WM_KEYUP:
	{
		keys[wParam] = false;
	}
	break;
	case WM_DESTROY:
	{
		done = true;
		PostQuitMessage(0);
	}
	break;
	case WM_CLOSE:
	{
		done = true;
		PostQuitMessage(0);
	}
	break;
	case WM_SIZE:
		DrawFrame(hWnd);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
