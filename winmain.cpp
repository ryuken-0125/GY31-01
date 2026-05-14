//=============================================================================
// GY31
// ウインドウ表示処理 [winmain.cpp]
//=============================================================================
#undef UNICODE  // Unicodeではなく、マルチバイト文字を使う

#include <Windows.h>
#include <stdio.h> // sprintf_sを使うために必要

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define CLASS_NAME   "GY31_sample"// ウインドウクラスの名前
#define WINDOW_NAME  "GY31_GDIプログラム"// ウィンドウの名前

#define SCREEN_WIDTH (960)	// ウインドウの幅
#define SCREEN_HEIGHT (540)	// ウインドウの高さ

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
int g_Mode;
int g_TimerCount;				//タイマーのカウント
float g_PositionX, g_PositionY;	//変位（座標）
float g_VelocityX, g_VelocityY;	//速度
float g_AccelX, g_AccelY;		//加速度


void InitUniformMotion();						 // 等速運動の初期化
void UpdateUniformMotion(float dt);				 // 等速運動の更新
void InitUniformlyAcceleratedMotion();           // 等加速度運動の初期化
void UpdateUniformlyAcceleratedMotion(float dt); // 等加速度運動の更新
void InitFreeFall();							 // 自由落下の初期化
void UpdateFreeFall(float dt);					 // 自由落下の更新
void InitVerticalThrow();						 // 鉛直投げ上げの初期化
void UpdateVerticalThrow(float dt);				 // 鉛直投げ上げの更新
void InitHorizontalProjection();				 // 水平投射の初期化
void UpdateHorizontalProjection(float dt);		 // 水平投射の更新
void InitObliqueProjection();					 // 斜方投射の初期化
void UpdateObliqueProjection(float dt);			 // 斜方投射の更新


//=============================================================================
// メイン関数
//  (WINAPI : Win32API関数を呼び出す時の規約)
//  hInstance     : このアプリケーションのインスタンスハンドル(識別子)
//  hPrevInstance : 常にNULL(16bit時代の名残り)
//  lpCmdLine     : コマンドラインから受け取った引数へのポインタ
//  nCmdShow      : アプリケーションの初期ウインドウ表示方法の指定
//=============================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex =
	{
		sizeof(WNDCLASSEX),				// WNDCLASSEXのメモリサイズを指定
		CS_CLASSDC,						// 表示するウインドウのスタイルを設定
		WndProc,						// ウィンドウプロシージャのアドレス(関数名)を指定
		0,								// 通常は使用しないので"0"を指定
		0,								// 通常は使用しないので"0"を指定
		hInstance,						// WinMainのパラメータのインスタンスハンドルを設定
		NULL,							// 使用するアイコンを指定(Windowsがもっているアイコンを使うならNULL)
		LoadCursor(NULL, IDC_ARROW),	// マウスカーソルを指定
		(HBRUSH)(COLOR_WINDOW + 1),		// ウインドウのクライアント領域の背景色を設定
		NULL,							// Windowにつけるメニューを設定
		CLASS_NAME,						// ウインドウクラスの名前
		NULL							// 小さいアイコンが設定された場合の情報を記述
	};

	RegisterClassEx(&wcex); // Windowsにウインドウ情報を登録

	// ウインドウを作成
	HWND hWnd;
	hWnd = CreateWindowEx(0,						// 拡張ウィンドウスタイル
		CLASS_NAME,				// ウィンドウクラスの名前
		WINDOW_NAME,			// ウィンドウの名前
		WS_OVERLAPPEDWINDOW,	// ウィンドウスタイル
		CW_USEDEFAULT,			// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,			// ウィンドウの左上Ｙ座標 
		SCREEN_WIDTH,			// ウィンドウの幅
		SCREEN_HEIGHT,			// ウィンドウの高さ
		NULL,					// 親ウィンドウのハンドル
		NULL,					// メニューハンドルまたは子ウィンドウID
		hInstance,				// インスタンスハンドル
		NULL);					// ウィンドウ作成データ

	// 指定されたウィンドウの表示状態を設定(ウィンドウを表示)
	ShowWindow(hWnd, nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(hWnd);

	MSG msg;


	// メインループ
	for (;;) {
		// 前回のループからユーザー操作があったか調べる
		BOOL doesMessageExist = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

		if (doesMessageExist)
		{
			// 間接的にウインドウプロシージャを呼び出す
			DispatchMessage(&msg);

			// アプリ終了命令が来た
			if (msg.message == WM_QUIT) {
				break;
			}
		}
	}

	UnregisterClass(CLASS_NAME, hInstance);

	return (int)msg.wParam;
}

//=============================================================================
// ウインドウプロシージャ
//  (CALLBACK : Win32API関数を呼び出す時の規約)
//  hWnd   : ウィンドウのハンドル
//  uMsg   : メッセージの識別子
//  wParam : メッセージの最初のパラメータ
//  lParam : メッセージの2番目のパラメータ
//=============================================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// uMsg（この関数の第2引数）が、ユーザー操作のID情報
	switch (uMsg)
	{
	case WM_DESTROY:// ウィンドウ破棄のメッセージ
		PostQuitMessage(0);// “WM_QUIT”メッセージを送る　→　アプリ終了
		break;

	case WM_CLOSE:  // xボタンが押されたら
			// メッセージボックスで修了確認
		int result;
		result = MessageBox(NULL, "終了してよろしいですか？", "終了確認", MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES) // 「はい」ボタンが押された時
		{
			DestroyWindow(hWnd);  // “WM_DESTROY”メッセージを送る
		}		
		break;

	case WM_KEYDOWN:
		// キーが押された時のリアクションを書く
		

		break;

	case WM_CREATE:
		g_Mode = 0;      
		g_TimerCount = 0; 

		InitUniformMotion(); 

		SetTimer(hWnd, 1, 100, NULL);
		break;

	case WM_TIMER://更新
	{
		float dt = 0.1f;//経過時間

		
		switch (g_Mode) {
		case 0: UpdateUniformMotion(dt); break;              // 等速運動
		case 1: UpdateUniformlyAcceleratedMotion(dt); break; // 等加速度運動
		case 2: UpdateFreeFall(dt); break;                   // 自由落下
		case 3: UpdateVerticalThrow(dt); break;              // 鉛直投げ上げ
		case 4: UpdateHorizontalProjection(dt); break;       // 水平投射
		case 5: UpdateObliqueProjection(dt); break;          // 斜方投射
		}

		
		g_TimerCount++;

		
		if (g_TimerCount >= 50) {
			g_TimerCount = 0; // カウンターをリセット
			g_Mode++;      

			if (g_Mode > 5) {
				g_Mode = 0;  
			}

			
			switch (g_Mode) {
			case 0: InitUniformMotion(); break;
			case 1: InitUniformlyAcceleratedMotion(); break;
			case 2: InitFreeFall(); break;
			case 3: InitVerticalThrow(); break;
			case 4: InitHorizontalProjection(); break;
			case 5: InitObliqueProjection(); break;
			}
		}

		InvalidateRect(hWnd, NULL, true);//画面再描画
		break;
	}

	case WM_PAINT://描画
		HDC hDC;
		PAINTSTRUCT  ps;
		HPEN hPen0, hOldPen;

		hDC = BeginPaint(hWnd, &ps);

		hPen0 = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		hOldPen = (HPEN)SelectObject(hDC, hPen0);

		//MoveToEx(hDC, g_PositionX, g_PositionY, NULL);
		//LineTo(hDC, 100, 100);

		Ellipse(hDC, (int)(g_PositionX - 20.0f),
					(int)(g_PositionY - 20.0f),
					(int)(g_PositionX + 20.0f),
					(int)(g_PositionY + 20.0f));//楕円描画

		SelectObject(hDC, hOldPen);
		DeleteObject(hPen0);

		EndPaint(hWnd, &ps);

		break;

	default:
		// 上のcase以外の場合の処理を実行
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

// 重力加速度の定義
const float GRAVITY = 98.0f;


// ----------------------------------------------------
// 1. 等速運動 (一定の速度で進む。加速度0)
// ----------------------------------------------------
void InitUniformMotion() {
	g_PositionX = 0.0f;  g_PositionY = 100.0f; 
	g_VelocityX = 50.0f; g_VelocityY = 0.0f;   
	g_AccelX = 0.0f;  g_AccelY = 0.0f;  
}
void UpdateUniformMotion(float dt) {
	g_PositionX += g_VelocityX * dt; 
	g_PositionY += g_VelocityY * dt;
}

// ----------------------------------------------------
// 2. 等加速度運動
// ----------------------------------------------------
void InitUniformlyAcceleratedMotion() {
	g_PositionX = 0.0f;  g_PositionY = 200.0f;
	g_VelocityX = 0.0f;  g_VelocityY = 0.0f;  
	g_AccelX = 20.0f; g_AccelY = 0.0f;  
}
void UpdateUniformlyAcceleratedMotion(float dt) {
	g_VelocityX += g_AccelX * dt;   
	g_PositionX += g_VelocityX * dt; 
}

// ----------------------------------------------------
// 3. 自由落下運動 (初速0で下に落ちる)
// ----------------------------------------------------
void InitFreeFall() {
	g_PositionX = 480.0f; g_PositionY = 0.0f; 
	g_VelocityX = 0.0f;   g_VelocityY = 0.0f;  
	g_AccelX = 0.0f;   g_AccelY = GRAVITY;
}
void UpdateFreeFall(float dt) {
	g_VelocityY += g_AccelY * dt;
	g_PositionY += g_VelocityY * dt;
}

// ----------------------------------------------------
// 4. 鉛直投げ上げ (真上に投げ、重力で落ちてくる)
// ----------------------------------------------------
void InitVerticalThrow() {
	g_PositionX = 480.0f; g_PositionY = 500.0f; 
	g_VelocityX = 0.0f;   g_VelocityY = -250.0f;
	g_AccelX = 0.0f;   g_AccelY = GRAVITY; 
}
void UpdateVerticalThrow(float dt) {
	g_VelocityY += g_AccelY * dt;
	g_PositionY += g_VelocityY * dt;
}

// ----------------------------------------------------
// 5. 水平投射 (真横に投げ、重力で落ちていく)
// ----------------------------------------------------
void InitHorizontalProjection() {
	g_PositionX = 0.0f;   g_PositionY = 0.0f;  
	g_VelocityX = 100.0f; g_VelocityY = 0.0f;  
	g_AccelX = 0.0f;   g_AccelY = GRAVITY; 
}
void UpdateHorizontalProjection(float dt) {
	g_VelocityX += g_AccelX * dt; 
	g_VelocityY += g_AccelY * dt;
	g_PositionX += g_VelocityX * dt;
	g_PositionY += g_VelocityY * dt;
}

// ----------------------------------------------------
// 6. 斜方投射 (斜め上に投げ、放物線を描いて落ちる)
// ----------------------------------------------------
void InitObliqueProjection() {
	g_PositionX = 0.0f;   g_PositionY = 500.0f; 
	g_VelocityX = 120.0f; g_VelocityY = -250.0f;
	g_AccelX = 0.0f;   g_AccelY = GRAVITY;  
}
void UpdateObliqueProjection(float dt) {
	g_VelocityX += g_AccelX * dt;
	g_VelocityY += g_AccelY * dt;
	g_PositionX += g_VelocityX * dt;
	g_PositionY += g_VelocityY * dt;
}
