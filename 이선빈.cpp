#define _CRT_SECURE_NO_WARNINGS
#include <bangtal>
#include <iostream>

using namespace std;
using namespace bangtal;

ObjectPtr board[8][8]; //다른 함수들에서도 사용하기 위해 글로벌변수로 만들어줌. 

enum class State {
	BLANK,
	POSSIBLE,
	BLACK,
	WHITE
};
State board_state[8][8]; //board 현재 상태 저장용

enum class Turn {
	BLACK,
	WHITE
};
Turn turn = Turn::BLACK;

void setState(int x, int y, State state) {

	switch (state) {
		case State::BLANK: board[y][x]->setImage("Images/blank.png"); break;
		case State::POSSIBLE: board[y][x]->setImage(turn == Turn::BLACK ? "Images/black possible.png":"Images/white possible.png"); break;
		case State::BLACK: board[y][x]->setImage("Images/black.png"); break;
		case State::WHITE: board[y][x]->setImage("Images/white.png"); break;

	} 
	board_state[y][x] = state;

}

bool checkPossible(int x, int y, int dx, int dy) 
{
	State mine = turn == Turn::BLACK ? State::BLACK : State::WHITE;
	State other = turn == Turn::BLACK ? State::WHITE : State::BLACK;

	bool possible = false;
	for (x += dx, y += dy; x >= 0 && x <= 8 && y >= 0 && y <= 8; x += dx, y += dy) 
		/*현재 주어진 x, y에서 dx, dy만큼 증가// x와 y가 board의 범위를 벗어나지 않는 동안*/
	{
		if (board_state[y][x] == other) {
			possible = true;
		}
		else if (board_state[y][x] == mine)  {
			return possible;
		}  
		else//아예 빈칸이 나온 경우 
		{
			return false;
		}
	}
	return false;
}

bool checkPossible(int x, int y)
{
	if (board_state[y][x] == State::BLACK) return false;
	if (board_state[y][x] == State::WHITE) return false;
	setState(x, y, State::BLANK); 

	int delta[8][2] = { //각각 방향마다 checkPossible로 돌의 상태 체크하는 배열
		{1,0},
		{0,1},
		{1,1},
		{1,-1},
		{0,-1},
		{-1,-1}, 
		{-1,0},
		{-1,1},
	};

	/*
	for (int i = 0; i < 8; i++) {
		checkPossible(x, y, delta[i][0], delta[i][1]);
	}
	*/

	bool possible = false;

	for (auto d : delta) {
		if(checkPossible(x, y, d[0], d[1])) possible = true;
	}

	return possible; //8방향중 하나만 가능해도 possible 된다고 반환
}

void reverse(int x, int y, int dx, int dy)
{
	State mine = turn == Turn::BLACK ? State::BLACK : State::WHITE;
	State other = turn == Turn::BLACK ? State::WHITE : State::BLACK;

	bool possible = false;
	for (x += dx, y += dy; x >= 0 && x <= 8 && y >= 0 && y <= 8; x += dx, y += dy)
	{
		if (board_state[y][x] == other) {
			possible = true;
		}
		else if (board_state[y][x] == mine) {
			if (possible) {
				for (x -= dx, y -= dy; ; x -= dx, y = dy) { //앞에서 이미 범위검사가 되었기 때문에 범위 설정 없이 무한루프 만들어도 괜찮음. 
					if (board_state[y][x] == other) {
						setState(x, y, mine);
					}
					else {
						return; //상대편의 색깔이 아니면 그대로 둠. 
					}
				}
			}
		}
		else//아예 빈칸이 나온 경우 
		{
			return;
		}
	}

}

void reverse(int x, int y) //setPossible이랑 똑같이 가면 됨
{
	int delta[8][2] = {
		{1,0},
		{0,1},
		{1,1},
		{1,-1},
		{0,-1},
		{-1,-1},
		{-1,0},
		{-1,1},
	};


	bool possible = false;
	for (auto d : delta) {
		reverse(x, y, d[0], d[1]);
	}
}

bool setPossible() //놓을 수 있는 위치 검사뿐만 아니라 true/false도 반환할 수 있게 함
{
	//8x8의 모든 위치에 대해서 
	//현재 턴에 놓여질 수 있으면 상태를 possible로 바꾼다
	bool possible = false;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (checkPossible(x, y)) { //checkPossible이 true이면 
				setState(x, y, State::POSSIBLE); //setState로 possible이미지 띄움
				possible = true;
			}
		}
	}
	return possible;
}

int main() {

	setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
	setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);

	auto scene = Scene::create("Othello", "Images/background.png");

	auto game_board = Object::create("Images/board.png", scene, 42, 42);


	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++)
		{
			board[y][x] = Object::create("Images/blank.png", scene, 40 + x * 80, 40 + y * 80);
			board[y][x]->setOnMouseCallback([&,x,y](ObjectPtr object, int, int, MouseAction action)->bool {
				/*마우스의 위치를 안쓸거면 변수이름 안써도 됨.
				&,x,y ->디폴트는 참조, x,y값만 카피해서 람다함수 내에서 사용하라는 의미*/

				if (board_state[y][x] == State::POSSIBLE) {
					if (turn == Turn::BLACK) {
						setState(x, y, State::BLACK);
						reverse(x, y);
						turn = Turn::WHITE; //턴을 바꿔준다. 
					}
					else {
						setState(x, y, State::WHITE);
						reverse(x, y);
						turn = Turn::BLACK;
					}
					if(!setPossible()) //턴이 바뀌었을 때 그 턴에 맞는 setPossible 보임 
					{
						turn = turn == Turn::BLACK ? Turn::WHITE : Turn::BLACK; //toggle

						if (!setPossible()) {
							showMessage("End Game");
						}
					}
				}
				return true;
				/*처리를 했으면 true리턴(콜백함수에서 일을 처리해서 더이상 진행 안했으면 좋겠다), 
				처리를 안했으면 false리턴(처리 안하고 기존 루틴대로 동작되었으면 좋겠다)*/
				});
				board_state[y][x] = State::BLANK;
		}
	}

	setState(3, 3, State::BLACK);
	setState(4, 4, State::BLACK);
	setState(3, 4, State::WHITE);
	setState(4, 3, State::WHITE);

	setPossible();
	
	startGame(scene);

	
}