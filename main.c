#pragma warning(disable: 4996)

#include "all_header.h"


DWORD mode;
INPUT_RECORD record; // 콘솔 입력 버퍼의 입력 이벤트 구조체
DWORD read; // unsigned long ( 데이터 기본 처리 단위인 WORD의 2배 )
HANDLE CIN, COUT;

COORD mouse;
int clicked; // 0: not clicked / 1 : left click / 2 : right click
KEY_INFO key;

FILE* Storage[4];

extern void MainPage();

// play sound
void SoundEffect(char filePath[], MCI_OPEN_PARMS* soundEffect, int* dwID, bool load, bool playing, bool repeat)
{

	soundEffect->lpstrElementName = filePath;//파일 오픈
	soundEffect->lpstrDeviceType = "mpegvideo";//mp3 형식
	*dwID = soundEffect->wDeviceID;
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID) & *soundEffect);

	if (!load)
	{
		if (playing) {
			if (repeat) {
				mciSendCommand(*dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID) & *soundEffect); //음악 반복 재생
			}
			else {
				mciSendCommand(*dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //음원 재생 위치를 처음으로 초기화
				mciSendCommand(*dwID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID) & *soundEffect);   //음악 한 번만 재생
			}
		}
		else {
			mciSendCommand(*dwID, MCI_PAUSE, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID) & *soundEffect);   //음악 재생 중지
			mciSendCommand(*dwID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD)(LPVOID)NULL); //음원 재생 위치를 처음으로 초기화
		}
	}
	return;
}

// thread reading mouse position and whether it is clicked
void MouseThread() {
	while (1) {
		ReadConsoleInput(CIN, &record, 1, &read); // read console event

		if (record.EventType == MOUSE_EVENT) {
			mouse.X = record.Event.MouseEvent.dwMousePosition.X;
			mouse.Y = record.Event.MouseEvent.dwMousePosition.Y;
			if (record.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) clicked = 1;
			else if (record.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) clicked = 2;
			else clicked = 0;
		}

		if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
			int KC = record.Event.KeyEvent.wVirtualKeyCode;
			key.UP = (bool)(KC == VK_UP);
			key.DOWN = (bool)(KC == VK_DOWN);
			key.RIGHT = (bool)(KC == VK_RIGHT);
			key.LEFT = (bool)(KC == VK_LEFT);
			key.ENTER = (bool)(KC == VK_RETURN);
			key.SPACE = (bool)(KC == VK_SPACE);
			key.ESC = (bool)(KC == VK_ESCAPE);
			key.A = (bool)(KC == 0x41);
			key.B = (bool)(KC == 0x42);
			key.C = (bool)(KC == 0x43);
			key.D = (bool)(KC == 0x44);
			key.E = (bool)(KC == 0x45);
			key.F = (bool)(KC == 0x46);
			key.G = (bool)(KC == 0x47);
			key.H = (bool)(KC == 0x48);
			key.I = (bool)(KC == 0x49);
			key.J = (bool)(KC == 0x4A);
			key.K = (bool)(KC == 0x4B);
			key.L = (bool)(KC == 0x4C);
			key.M = (bool)(KC == 0x4D);
			key.N = (bool)(KC == 0x4E);
			key.O = (bool)(KC == 0x4F);
			key.P = (bool)(KC == 0x50);
			key.Q = (bool)(KC == 0x51);
			key.R = (bool)(KC == 0x52);
			key.S = (bool)(KC == 0x53);
			key.T = (bool)(KC == 0x54);
			key.U = (bool)(KC == 0x55);
			key.V = (bool)(KC == 0x56);
			key.W = (bool)(KC == 0x57);
			key.X = (bool)(KC == 0x58);
			key.Y = (bool)(KC == 0x59);
			key.Z = (bool)(KC == 0x5A);
		}
		else {
			key.UP = key.DOWN = key.RIGHT = key.LEFT = key.ENTER = key.SPACE = key.ESC
				= key.A = key.B = key.C = key.D = key.E = key.F = key.G = key.H = key.I = key.J = key.K = key.L = key.M
				= key.N = key.O = key.P = key.Q = key.R = key.S = key.T = key.U = key.V = key.W = key.X = key.Y = key.Z = 0;
		}
		FlushConsoleInputBuffer(CIN);
	}
	return;
}

// open n-th storage with mode ( 0 : read / 1 : write / 2 : append)
FILE* OpenStorage_i(int n, int rwa)
{
	char fn[20] = "Storage_9.txt", md[5] = "X";
	fn[8] = (char)(n + 48);
	md[0] = (!rwa ? 'r' : rwa==1 ? 'w' : 'a');
	return fopen(fn, md);
}

// toggle image or text in layer
void ToggleLayer(ManyLayer *ML, int img_s, int img_e, int txt_s, int txt_e)
{
	if(img_s>=0 && img_e>=0)
		for (int i = img_s; i <= img_e; i++) 
			ML->images[i].isHidden = !ML->images[i].isHidden;
	if(txt_s>=0 && txt_e>=0)
		for (int i = txt_s; i <= txt_e; i++) 
			ML->texts[i].isHidden = !ML->texts[i].isHidden;
}

// initial settings
void Init()
{
	CIN = GetStdHandle(STD_INPUT_HANDLE);
	COUT = GetStdHandle(STD_OUTPUT_HANDLE);

	// fullscreen
	SetConsoleDisplayMode(COUT, CONSOLE_FULLSCREEN_MODE, 0);

	// window width/height
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(COUT, &csbi);
	CONSOLE_WIDTH = csbi.srWindow.Right - csbi.srWindow.Left + 2;
	CONSOLE_HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top + 2;
	printf("columns: %d\n", CONSOLE_WIDTH); //236
	printf("rows: %d\n", CONSOLE_HEIGHT); //62

	// change to mouse mode
	SetConsoleMode(CIN, mode | ENABLE_MOUSE_INPUT);

	srand(time(0));

	_beginthreadex(NULL, 0, (_beginthreadex_proc_type)MouseThread, 0, 0, NULL);

	return;
}


int main() {
	Init();
	MainPage();

	system("cls");
	return 0;
}

