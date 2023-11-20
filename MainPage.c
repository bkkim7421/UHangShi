#pragma warning(disable: 4996)

#include "all_header.h"

#define NEWGAME_BTN_IDX 3
#define LOAD_BTN_IDX 5
#define EXIT_BTN_IDX 7
#define LOAD_SCREEN_IDX 8
#define REPLACE_SCREEN_IDX 9
#define REPLACE_TXT_SIZE 3

const int REPLACE_TXT_IDX[REPLACE_TXT_SIZE] = { 0,1,2 };

extern COORD mouse;
extern int clicked;
extern FILE* Storage[4];
extern KEY_INFO key;
extern DWORD mode;
extern INPUT_RECORD record; // 콘솔 입력 버퍼의 입력 이벤트 구조체
extern DWORD read; // unsigned long ( 데이터 기본 처리 단위인 WORD의 2배 )
extern HANDLE CIN, COUT;
extern GamePageFlag;

extern void StartGame(int N);
extern FILE* OpenStorage_i(int n, int rw);

bool MainPageFlag;
ManyLayer manyLayer = { NULL, 0, NULL, 0, RGB(255, 255, 255), NULL, NULL, _initialize, _renderAll, NULL, _getBitmapHandle };

// thread painting layer repeatedly
void BGThread()
{
	while (MainPageFlag) {
		manyLayer.renderAll(&manyLayer);
		Sleep(10);
	}
	return;
}

bool OnNewgame(COORD p){ return 93 <= p.X && p.X <= 151 && 25 <= p.Y && p.Y <= 31; }
bool OnLoad(COORD p) { return 93 <= p.X && p.X <= 151 && 34 <= p.Y && p.Y <= 41; }
bool OnExit(COORD p) { return 93 <= p.X && p.X <= 151 && 43 <= p.Y && p.Y <= 50; }
bool OnLoadExitBtn(COORD p) { return 173 <= p.X && p.X <= 182 && 15 <= p.Y && p.Y <= 20; }
bool OnLoad_i(COORD p, int n) { return  66 <= p.X && p.X <= 176 && 19 + n * 10 + (n >= 2) <= p.Y && p.Y <= 28 + n * 10 + (n >= 2) && !OnLoadExitBtn(p); }

// open or close load screen
void ToggleLoadScreen()
{
	manyLayer.images[LOAD_SCREEN_IDX].isHidden = !manyLayer.images[LOAD_SCREEN_IDX].isHidden;
	// code showing text
}

// Load menu
void LoadStorage()
{
	bool flag = 0;
	ToggleLoadScreen();

	Sleep(600);
	while (1) {
		if (flag)break;
		if (clicked) {
			for (int i = 0; i < 4; i++) {
				if (OnLoad_i(mouse, i)) {
					Storage[i] = OpenStorage_i(i, 0);
					printf("Storage %d opened\n", i);

					int PG;
					fscanf(Storage[i], "%d", &PG);

					fclose(Storage[i]);
					Storage[i] = NULL;

					printf("State : %d\n", (Storage[i] ? 1 : 0));
					if (PG >= 0) {
						StartGame(i, PG);
						flag = 1;
						ToggleLoadScreen();
					}
					break;
				}
			}
			if (OnLoadExitBtn(mouse)) {
				ToggleLoadScreen();
				break;
			}
		}
		Sleep(100);
	}
	return;
}

int GetEmptyStorage()
{
	for (int i = 0; i < 4; i++) {
		Storage[i] = OpenStorage_i(i, 0);
		int PG;
		fscanf(Storage[i], "%d", &PG);
		fclose(Storage[i]);
		Storage[i] = NULL;
		if (PG < 0)
			return i;
	}
	return -1;
}

bool ReplaceStorage()
{
	int input = 0;
	while (1) {
		if (key.Y) {
			Sleep(200);
			return 1;
		}
		else if (key.N) {
			Sleep(200);
			return 0;
		}
	}
}

void ToggleReplaceScreen()
{
	manyLayer.images[REPLACE_SCREEN_IDX].isHidden = !manyLayer.images[REPLACE_SCREEN_IDX].isHidden;
	for (int i = 0; i < REPLACE_TXT_SIZE; i++)
		manyLayer.texts[REPLACE_TXT_IDX[i]].isHidden = !manyLayer.texts[REPLACE_TXT_IDX[i]].isHidden;
	Sleep(100);
}

// create main page
void MainPage()
{
	manyLayer.initialize(&manyLayer);
	manyLayer.images = (Image[]){
		(Image){ manyLayer.getBitmapHandle(L"start_bg.bmp"), 0, 0, 2.38, false },
		(Image){ manyLayer.getBitmapHandle(L"title.bmp"), 900, 50, 1.6, false },
		(Image){ manyLayer.getBitmapHandle(L"new_game_clicked.bmp"), 1450, 700, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"new_game.bmp"), 1450, 700, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"load_clicked.bmp"), 1450, 1000, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"load.bmp"), 1450, 1000, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"exit_clicked.bmp"), 1450, 1300, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"exit.bmp"), 1450, 1300, 1.6, false },
		(Image){ manyLayer.getBitmapHandle(L"load_screen.bmp"), 900, 450, 1.3, true},
		(Image){ manyLayer.getBitmapHandle(L"box.bmp"), 900, 700, 1.3, true}
	};
	manyLayer.imageCount = 10;

	manyLayer.texts = (Text[]){
		{ L"1번 저장소를 덮어쓰겠습니까?", 1150, 900, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
		{ L"[Y]es", 1350, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
		{ L"[N]o", 2150, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
	};
	manyLayer.textCount = 3;

	while (1) {
		GamePageFlag = 0;
		if (!MainPageFlag) {
			_beginthreadex(NULL, 0, (_beginthreadex_proc_type)BGThread, 0, 0, NULL);
			MainPageFlag = 1;
		}

		if (OnNewgame(mouse)) {
			manyLayer.images[NEWGAME_BTN_IDX].isHidden = true;
			if (clicked == 1) {
				int idx = GetEmptyStorage();
				if (idx < 0) {
					ToggleReplaceScreen();
					if (ReplaceStorage()) {
						printf("help me\n");
						StartGame(0, 0);
					}
					ToggleReplaceScreen();
				}
				else
					StartGame(idx, 0);
			}
		}
		else
			manyLayer.images[NEWGAME_BTN_IDX].isHidden = false;

		if (OnLoad(mouse)) {
			manyLayer.images[LOAD_BTN_IDX].isHidden = true;
			if (clicked == 1) {
				LoadStorage();
			}
		}
		else
			manyLayer.images[LOAD_BTN_IDX].isHidden = false;

		if (OnExit(mouse)) {
			manyLayer.images[EXIT_BTN_IDX].isHidden = true;
			if (clicked == 1)
				return;
		}
		else
			manyLayer.images[EXIT_BTN_IDX].isHidden = false;
		Sleep(100);
	}

	return;
}