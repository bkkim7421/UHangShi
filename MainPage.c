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
extern bool GamePageFlag;
extern MCI_OPEN_PARMS InGameMusic;
extern int InGameMusicID;


extern void StartGame(int N);
extern FILE* OpenStorage_i(int n, int rw);
extern void SoundEffect(char filePath[], MCI_OPEN_PARMS* soundEffect, int* dwID, bool load, bool playing, bool repeat);

bool MainPageFlag;
ManyLayer manyLayer = { NULL, 0, NULL, 0, RGB(255, 255, 255), NULL, NULL, _initialize, _renderAll, NULL, _getBitmapHandle };

MCI_OPEN_PARMS MainMusic;

int MainMusicID;

MCI_OPEN_PARMS openBgm;		//bgm 구조체
MCI_PLAY_PARMS playBgm;
MCI_OPEN_PARMS openfallsound;		//블록착지음 구조체
MCI_PLAY_PARMS playfallsound;
MCI_OPEN_PARMS openlinesound;		//줄 삭제음 구조체
MCI_PLAY_PARMS playlinesound;

int dwID;		//음악 파일 불러오는 값
int dwID1;
int dwID2;

void MainSound(int a)
{
	openBgm.lpstrElementName = TEXT("BabyElephantWalk60.wav"); //파일 오픈
	openBgm.lpstrDeviceType = TEXT("mpegvideo"); //mp3 형식
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
	dwID = openBgm.wDeviceID;
	if (a == 0)		//재생신호시 재생
		mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm); //음악 반복 재생
	else	//재생 정지 신호시 정지
		mciSendCommand(dwID, MCI_PAUSE, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm);		//재생 정지
}

void playsoundline()
{
	openlinesound.lpstrElementName = TEXT("Pretender.wav"); //파일 오픈
	openlinesound.lpstrDeviceType = TEXT("mpegvideo"); //mp3 형식
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openlinesound);
	dwID1 = openlinesound.wDeviceID;
	mciSendCommand(dwID1, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //음원 재생 위치를 처음으로 초기화
	mciSendCommand(dwID1, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&openlinesound); //음악을 한 번 재생
}

void playsoundfall()
{
	openfallsound.lpstrElementName = TEXT("BabyElephantWalk60.wav"); //파일 오픈
	openfallsound.lpstrDeviceType = TEXT("mpegvideo"); //mp3 형식
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openfallsound);
	dwID2 = openfallsound.wDeviceID;
	mciSendCommand(dwID2, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //음원 재생 위치를 처음으로 초기화
	mciSendCommand(dwID2, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&openfallsound); //음악을 한 번 재생

}

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
		(Image){ manyLayer.getBitmapHandle(L"MainPage/start_bg.bmp"), 0, 0, 2.38, false },
		(Image){ manyLayer.getBitmapHandle(L"MainPage/title.bmp"), 900, 50, 1.6, false },
		(Image){ manyLayer.getBitmapHandle(L"MainPage/new_game_clicked.bmp"), 1450, 700, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/new_game.bmp"), 1450, 700, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/load_clicked.bmp"), 1450, 1000, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/load.bmp"), 1450, 1000, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/exit_clicked.bmp"), 1450, 1300, 1.6, false},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/exit.bmp"), 1450, 1300, 1.6, false },
		(Image){ manyLayer.getBitmapHandle(L"MainPage/load_screen.bmp"), 900, 450, 1.3, true},
		(Image){ manyLayer.getBitmapHandle(L"MainPage/box.bmp"), 900, 700, 1.3, true}
	};
	manyLayer.imageCount = 10;

	manyLayer.texts = (Text[]){
		{ L"1번 저장소를 덮어쓰겠습니까?", 1150, 900, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
		{ L"[Y]es", 1400, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
		{ L"[N]o", 2250, 1150, 48, 64, 600, L"둥근모꼴", RGB(0, 0, 0), true },
	};
	manyLayer.textCount = 3;

	while (1) {
		GamePageFlag = 0;
		if (!MainPageFlag) {
			MainPageFlag = 1;
			_beginthreadex(NULL, 0, (_beginthreadex_proc_type)BGThread, 0, 0, NULL);

			SoundEffect("Musics/InGameBGM.wav", &InGameMusic, &InGameMusicID, 0, 0, 0);
			SoundEffect("Musics/BabyElephantWalk60.wav", &MainMusic, &MainMusicID, 1, 0, 0);
			SoundEffect("Musics/BabyElephantWalk60.wav", &MainMusic, &MainMusicID, 0, 1, 1);
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