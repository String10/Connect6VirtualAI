#define _CRT_SECURE_NO_WARNINGS
#include "Reversi.h"

#define random(x) (rand()%x)
#define ROWS 19
#define COLS 19
#define ROUNDS 3

#include <io.h>
#include <direct.h>

Reversi::Reversi(){
    client_socket = ClientSocket();
    oppositeColor = ownColor = -1;

	std::string prefix = ".\\logs\\";
	if (_access(prefix.c_str(), 0) == -1) {
		_mkdir(prefix.c_str());
	}

	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d_%H_%M_%S", localtime(&timep));
	filename = tmp;
	filename = prefix + filename + ".txt";
}

Reversi::~Reversi(){}
void Reversi::setOwnColor(int color)
{
	if (color != 0 || color != 1)
		return;
	ownColor = color;
	oppositeColor = 1 - color;
}
;

/*
 send id and password to server by socket
 rtn != 0 represents socket transfer error
 */
void Reversi::authorize(const char *id , const char *pass)
{
    client_socket.connectServer();
    std::cout << "Authorize " << id << std::endl;
    char msgBuf[BUFSIZE];
    memset(msgBuf , 0 , BUFSIZE);
    msgBuf[0] = 'A';
    memcpy(&msgBuf[1] , id , 9);
    memcpy(&msgBuf[10] , pass , 6);
    int rtn = client_socket.sendMsg(msgBuf);
    if (rtn != 0) printf("Authorized Failed!\n");
}

// 用户id输入，服务器上需要有对应的账号密码：对应文件 players-0.txt
void Reversi::gameStart()
{
    char id[12] = {0}, passwd[10] = {0};
	strcpy(id, ID);
	strcpy(passwd, PASSWORD);
    authorize(id, passwd);
    
    printf("Game Start!\n");
    
    for (int round = 0 ; round < ROUNDS ; round++){
        roundStart(round);
        oneRound();
        roundOver(round);
    }
    gameOver();
    client_socket.close();
}

void Reversi::gameOver()
{
    printf("Game Over!\n");
}

//发一次消息，走哪一步，等两个消息，1.自己的步数行不行 2.对面走了哪一步 
void Reversi::roundStart(int round)
{
    printf("Round %d Ready Start!\n" , round);
    
    // first time receive msg from server
    int rtn = client_socket.recvMsg();
    if (rtn != 0) return ;
    if(strlen(client_socket.getRecvMsg()) < 2)
        printf("Authorize Failed!\n");
    else
        printf("Round start received msg %s\n", client_socket.getRecvMsg());
    switch (client_socket.getRecvMsg()[1]) {
            // this client : black chessman
        case 'B':
            ownColor = 0;
            oppositeColor = 1;
            rtn = client_socket.sendMsg("BB");
            printf("Send BB -> rtn: %d\n", rtn);
            if (rtn != 0) return ;
            break;
        case 'W':
            ownColor = 1;
            oppositeColor = 0;
            rtn = client_socket.sendMsg("BW");
            printf("Send BW -> rtn: %d\n", rtn);
            if (rtn != 0) return ;
            break;
        default:
            printf("Authorized Failed!\n");
            break;
    }
}

void Reversi::oneRound()
{
    int STEP = 1;
    switch (ownColor) {
        case 0:
            while (STEP < 10000) {
                
				pair<pair<int, int>, pair<int, int>> chess = step();                        // take action, send message
                
                // lazi only excute after server's message confirm  in observe function
                generateOneStepMessage(chess.first.first, chess.first.second, chess.second.first, chess.second.second);
                
                
                if (observe() >= 1) break;     // receive RET Code
                
                if (observe() >= 1) break;    // see white move
                STEP++;
                // saveChessBoard();
            }
            printf("One Round End\n");
            break;
        case 1:
            while (STEP < 10000) {
                
                if (observe() >= 1) break;    // see black move
                
				pair<pair<int, int>, pair<int, int>> chess = step();                        // take action, send message
                // lazi only excute after server's message confirm  in observe function
                generateOneStepMessage(chess.first.first,chess.first.second,chess.second.first, chess.second.second);
                
                
                if (observe() >= 1) break;     // receive RET Code
                // saveChessBoard();
                STEP++;
            }
            printf("One Round End\n");
            break;
            
        default:
            break;
    }
}

void Reversi::roundOver(int round)
{
    printf("Round %d Over!\n", round);
    // reset initializer

    ownColor = oppositeColor = -1;

	board = chessboard::ChessBoard();
}

int Reversi::observe()
{
	int rtn = 0;
	int recvrtn = client_socket.recvMsg();
	if (recvrtn != 0) return 1;
	printf("receive msg %s\n", client_socket.getRecvMsg());
	switch (client_socket.getRecvMsg()[0]) {
	case 'R':
	{
		switch (client_socket.getRecvMsg()[1]) {
		case 'Y':   // valid step
			switch (client_socket.getRecvMsg()[2]) {
			case 'P':   // update chessboard
			{
				int desRow1 = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol1 = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';
				int desRow2 = (client_socket.getRecvMsg()[7] - '0') * 10 + client_socket.getRecvMsg()[8] - '0';
				int desCol2 = (client_socket.getRecvMsg()[9] - '0') * 10 + client_socket.getRecvMsg()[10] - '0';
				int color = (client_socket.getRecvMsg()[11] - '0');
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow1, desCol1, desRow2, desCol2, color);

				printf("a valid step of : (%d %d) (%d %d)\n", desRow1, desCol1, desRow2, desCol2);
				break;
			}
			case 'N':   // R0N: enemy wrong step
			{
				//
				printf("a true judgement of no step\n");
				break;
			}
			}

			break;
		case 'W':
			// invalid step
			switch (client_socket.getRecvMsg()[2]) {
			case 'P': {
				int desRow1 = (client_socket.getRecvMsg()[3] - '0') * 10 + client_socket.getRecvMsg()[4] - '0';
				int desCol1 = (client_socket.getRecvMsg()[5] - '0') * 10 + client_socket.getRecvMsg()[6] - '0';
				int desRow2 = (client_socket.getRecvMsg()[7] - '0') * 10 + client_socket.getRecvMsg()[8] - '0';
				int desCol2 = (client_socket.getRecvMsg()[9] - '0') * 10 + client_socket.getRecvMsg()[10] - '0';
				int color = (client_socket.getRecvMsg()[11] - '0');
				printf("Invalid step , server random a true step of : (%d %d) (%d %d)\n", desRow1, desCol1, desRow2, desCol2);
				//你应该在这里处理desRow和desCol，推荐使用函数
				handleMessage(desRow1, desCol1, desRow2, desCol2, color);
				break;
			}
			case 'N': {
				printf("a wrong judgement of no step\n");
				break;
			}
			default:
				break;
			}
			break;
		case '1':

			printf("Error -1: Msg format error!\n");
			rtn = -1;
			break;
		case '2':

			printf("Error -2: Corrdinate error!\n");
			rtn = -2;
			break;
		case '4':

			printf("Error -4: Invalid step!\n");
			rtn = -4;
			break;
		default:

			printf("Error -5: Other error!\n");
			rtn = -5;
			break;
		}
		break;
	}
	case 'E':
	{
		switch (client_socket.getRecvMsg()[1]) {
		case '0':
			// game over
			rtn = 2;
			break;
		case '1':
			// round over
			rtn = 1;
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	return rtn;
}

void Reversi::generateOneStepMessage(int row1, int col1, int row2, int col2)
{
	char msg[BUFSIZE];
	memset(msg, 0, sizeof(msg));

	//put row and col in the message
	msg[0] = 'S';
	msg[1] = 'P';
	msg[2] = '0' + row1 / 10;
	msg[3] = '0' + row1 % 10;
	msg[4] = '0' + col1 / 10;
	msg[5] = '0' + col1 % 10;
	msg[6] = '0' + row2 / 10;
	msg[7] = '0' + row2 % 10;
	msg[8] = '0' + col2 / 10;
	msg[9] = '0' + col2 % 10;
	msg[10] = '\0';

	//print
	printf("generate one step at possition (%2d,%2d,%2d,%2d) : %s\n", row1, col1, row2, col2, msg);


	client_socket.sendMsg(msg);
}

/*-------------------------last three function--------------------------------
* step : find a good position to lazi your chess.
* saveChessBoard : save the chess board now.
* handleMessage: handle the message from server.
*/

const int LEN = 6;

static int to[8][2] = {
	{0, 1},
	{1, 1},
	{1, 0},
	{1, -1},
	{0, -1},
	{-1, -1},
	{-1, 0},
	{-1, 1},
};

bool Reversi::CertainStep(int &r1, int &c1, int &r2, int &c2) {
	auto abs = [](int k)->int {
		return k > 0 ? k : -k;
	};

	for (int i = 0; i < chessboard::BOARD_HEIGHT; i++) {
		for (int j = 0; j < chessboard::BOARD_WIDTH; j++) {
			if (board.GetColor(i, j) == ownColor + 1) {
				for (int k = 0; k < 8; k++) {
					for (int u = i + to[k][0], v = j + to[k][1];
						u < chessboard::BOARD_HEIGHT && v < chessboard::BOARD_WIDTH;
						u += to[k][0], v += to[k][1]) {
						if (board.GetColor(u, v) != ownColor + 1) {
							break;
						}
						if (abs(u - i + 1) == 4 || abs(v - j + 1) == 4) {
							if (board.GetColor(i - to[k][0], j - to[k][1]) == 0
								&& board.GetColor(u + to[k][0], v + to[k][1]) == 0) {
								r1 = i - to[k][0], c1 = j - to[k][1];
								r2 = u + to[k][0], c2 = v + to[k][1];
								return true;
							}
							if (board.GetColor(u + to[k][0], v + to[k][1]) == 0
								&& board.GetColor(u + to[k][0] * 2, v + to[k][1] * 2) == 0) {
								r1 = u + to[k][0], c1 = v + to[k][1];
								r2 = u + to[k][0] * 2, c2 = v + to[k][1] * 2;
								return true;
							}
						}
						else if (abs(u - i + 1) == 5 || abs(v - j + 1) == 5) {
							if (board.GetColor(i - to[k][0], j - to[k][1]) == 0) {
								r1 = i - to[k][0], c1 = j - to[k][1];
								return true;
							}
							if (board.GetColor(u + to[k][0], v + to[k][1]) == 0) {
								r1 = u + to[k][0], c1 = v + to[k][1];
								return true;
							}
						}
						else {
							if (board.GetColor(u + to[k][0], v + to[k][1]) == 0
								&& board.GetColor(u + to[k][0] * 2, v + to[k][1] * 2) == 0) {
								r1 = u + to[k][0], c1 = v + to[k][1];
								r2 = u + to[k][0] * 2, c2 = v + to[k][1] * 2;
							}
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < chessboard::BOARD_HEIGHT; i++) {
		for (int j = 0; j < chessboard::BOARD_WIDTH; j++) {
			if (board.GetColor(i, j) == (!ownColor) + 1) {
				for (int k = 0; k < 8; k++) {
					for (int u = i + to[k][0], v = j + to[k][1];
						u < chessboard::BOARD_HEIGHT && v < chessboard::BOARD_WIDTH;
						u += to[k][0], v += to[k][1]) {
						if (board.GetColor(u, v) != (!ownColor) + 1) {
							break;
						}
						if (abs(u - i + 1) == 4 || abs(v - j + 1) == 4) {
							if (board.GetColor(i - to[k][0], j - to[k][1]) == 0
								&& board.GetColor(u + to[k][0], v + to[k][1]) == 0) {
								r1 = i - to[k][0], c1 = j - to[k][1];
								r2 = u + to[k][0], c2 = v + to[k][1];
								return true;
							}
						}
						if (abs(u - i + 1) == 5 || abs(v - j + 1) == 5) {
							if (board.GetColor(i - to[k][0], j - to[k][1]) == 0
								&& board.GetColor(u + to[k][0], v + to[k][1]) == 0) {
								r1 = i - to[k][0], c1 = j - to[k][1];
								r2 = u + to[k][0], c2 = v + to[k][1];
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void Reversi::CheckCertainStep(int& r1, int& c1, int& r2, int& c2) {
	auto abs = [](int k)->int {
		return k > 0 ? k : -k;
	};
	auto getPoint = [&](int _color)->int {
		if (_color == ownColor + 1) {
			return 1;
		}
		if (_color == (!ownColor) + 1) {
			return 7;
		}
		return 0;
	};

	int h1, h2, w1, w2;
	h1 = h2 = w1 = w2 = -1;
	for (int x = 0; x < chessboard::BOARD_HEIGHT; x++) {
		for (int y = 0; y < chessboard::BOARD_WIDTH; y++) {
			if (board.GetColor(x, y) == chessboard::whiteChess ||
				board.GetColor(x, y) == chessboard::blackChess) {
				h2 = x;
				if (h1 == -1) {
					h1 = x;
				}
				break;
			}
		}
	}
	for (int y = 0; y < chessboard::BOARD_WIDTH; y++) {
		for (int x = 0; x < chessboard::BOARD_HEIGHT; x++) {
			if (board.GetColor(x, y) == chessboard::whiteChess ||
				board.GetColor(x, y) == chessboard::blackChess) {
				w2 = y;
				if (w1 == -1) {
					w1 = y;
				}
				break;
			}
		}
	}

	bool isSecond = false;
	for (int x = h1; x <= h2; x++) {
		for (int y = w1; y <= w2; y++) {
			for (int k = 0; k < 8; k++) {
				int _r1, _c1, _r2, _c2;
				_r1 = _r2 = x, _c1 = _c2 = y;
				for (int i = 0; i < LEN; i++) {
					if (board.CheckInside(x + to[k][0] * i, y + to[k][1] * i)) {
						_r2 = x + to[k][0] * i, _c2 = y + to[k][1] * i;
					}
					else {
						break;
					}
				}
				int sum = 0;
				for (int u = _r1, v = _c1; ; u += to[k][0], v += to[k][1]) {
					sum += getPoint(board.GetColor(u, v));
					if (u == _r2 && v == _c2) {
						if (sum == 5) {
							for (int _u = _r1, _v = _c1; ; _u += to[k][0], _v += to[k][1]) {
								if (board.GetColor(_u, _v) == 0 && !(r2 == _u && c2 == _v) && !isSecond) {
									r1 = _u, c1 = _v;
									isSecond = true;
								}
								if (board.GetColor(_u, _v) == 0 && !(r1 == _u && c1 == _v) && isSecond) {
									r2 = _u, c2 = _v;
									return;
								}
								if (_u == _r2 && _v == _c2) {
									break;
								}
							}
						}
						if (sum == 4) {
							for (int _u = _r1, _v = _c1; ; _u += to[k][0], _v += to[k][1]) {
								if (board.GetColor(_u, _v) == 0) {
									if (isSecond && !(r1 == _u && c1 == _v)) {
										r2 = _u, c2 = _v;
										return;
									}
									if (!isSecond && !(r2 == _u && c2 == _v)) {
										r1 = _u, c1 = _v;
										isSecond = true;
										if (!(_u == _r1 && _v == _c1) || !(_u == _r2 && _v == _c1)) {
											break;
										}
									}
								}
								if (_u == _r2 && _v == _c2) {
									break;
								}
							}
						}
						if (sum == 35) {
							for (int _u = _r1, _v = _c1; ; _u += to[k][0], _v += to[k][1]) {
								if (board.GetColor(_u, _v) == 0 && !(r2 == _u && c2 == _v) && !isSecond) {
									r1 = _u, c1 = _v;
									isSecond = true;
								}
								if (board.GetColor(_u, _v) == 0 && !(r1 == _u && c1 == _v) && isSecond) {
									r2 = _u, c2 = _v;
									return;
								}
								if (_u == _r2 && _v == _c2) {
									break;
								}
							}
						}
						if (sum == 28) {
							for (int _u = _r1, _v = _c1; ; _u += to[k][0], _v += to[k][1]) {
								if (board.GetColor(_u, _v) == 0) {
									if (isSecond && !(r1 == _u && c1 == _v)) {
										r2 = _u, c2 = _v;
										return;
									}
									if (!isSecond && !(r2 == _u && c2 == _v)) {
										r1 = _u, c1 = _v;
										isSecond = true;
										if (!(_u == _r1 && _v == _c1) || !(_u == _r2 && _v == _c1)) {
											break;
										}
									}
								}
								if (_u == _r2 && _v == _c2) {
									break;
								}
							}
						}
						break;
					}
				}
			}
		}
	}
}

#include <set>
#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>

const int SEARCH_DEPTH = 2;
const int SEARCH_WIDTH = 4;

void Reversi::MakeOptionVec(vector <pair <Position, int> >& options, int color) {
	set < pair <int, int> > alternative;
	for (int i = 0; i < chessboard::BOARD_HEIGHT; i++) {
		for (int j = 0; j < chessboard::BOARD_WIDTH; j++) {
			if (board.GetColor(i, j) == chessboard::blackChess
				|| board.GetColor(i, j) == chessboard::whiteChess) {
				for (int k = 0; k < 8; k++) {
					int u = i + to[k][0], v = j + to[k][1];
					if (board.GetColor(u, v) == 0) {
						alternative.insert(make_pair(u, v));
					}
				}
			}
		}
	}

	for (auto pos : alternative) {
		options.push_back(make_pair(pos, CalcHeuristicValue(pos, color)));
	}

	if (color == ownColor) {
		sort(options.begin(), options.end(),
			[](pair <Position, int> a, pair <Position, int> b)->bool {
				return a.second > b.second;
			});
	}
	else {
		sort(options.begin(), options.end(),
			[](pair <Position, int> a, pair <Position, int> b)->bool {
				return a.second < b.second;
			});
	}
}
void Reversi::MakeOption2Vec(vector <pair <pair <Position, Position>, int> >& options2, int color) {
	int w = sqrt(SEARCH_WIDTH);
	vector <pair <Position, int> > optionsFirstStep;
	MakeOptionVec(optionsFirstStep, color);

	for (int i = 0; i < w; i++) {
		vector <pair <Position, int> > optionsSecondStep;

		board.TakeMove(optionsFirstStep[i].first, color + 1);
		MakeOptionVec(optionsSecondStep, color);
		board.RestoreMove(optionsFirstStep[i].first, color + 1);

		for (int j = 0; j < w; j++) {
			options2.push_back(make_pair(make_pair(optionsFirstStep[i].first, optionsSecondStep[j].first), optionsFirstStep[i].second + optionsSecondStep[j].second));
		}
	}
}
int Reversi::CalcHeuristicValue(Position pos, int color) {
	int x = pos.first, y = pos.second;

	int preValue = 0, movValue = 0;

	auto abs = [](int k)->int {
		return k > 0 ? k : -k;
	};
	auto getPoint = [&](int _color)->int {
		if (_color == color + 1) {
			return 1;
		}
		if (_color == (!color) + 1) {
			return 7;
		}
		return 0;
	};
	auto calc = [](int k)->int {
		if (k / 7 == 0) {
			switch (k)
			{
			case 6:
				return 1e6;
			case 5:
				return 200;
			case 4:
				return 200;
			case 3:
				return 40;
			case 2:
				return 20;
			case 1:
				return 1;
			default:
				break;
			}
		}
		else if (k % 7 == 0) {
			switch (k / 7)
			{
			case 6:
				return -1e6;
			case 5:
				return -6000;
			case 4:
				return -6000;
			case 3:
				return -45;
			case 2:
				return -20;
			case 1:
				return -1;
			default:
				break;
			}
		}
		return 0;
	};

	for (int k = 0; k < 4; k++) {
		int r1, c1, r2, c2;
		r1 = r2 = x, c1 = c2 = y;
		for (int i = 0; i <= LEN; i++) {
			if (board.CheckInside(x - to[k][0] * i, y - to[k][1] * i)) {
				r1 = x - to[k][0] * i, c1 = y - to[k][1] * i;
			}
			else {
				break;
			}
		}
		for (int i = 0; i <= LEN; i++) {
			if (board.CheckInside(x + to[k][0] * i, y + to[k][1] * i)) {
				r2 = x + to[k][0] * i, c2 = y + to[k][1] * i;
			}
			else {
				break;
			}
		}

		int sum = 0;

		board.TakeMove(x, y, color + 1);
		for (int u = r1, v = c1; u <= r2 && v <= c2; u += to[k][0], v += to[k][1]) {
			sum += getPoint(board.GetColor(u, v));
			if (abs(u - r1) + 1 == LEN || abs(v - c1) + 1 == LEN) {
				movValue += calc(sum);
			}
			if (abs(u - r1) + 1 >= LEN || abs(v - c1) + 1 >= LEN) {
				sum -= getPoint(board.GetColor(u - to[k][0] * 6, v - to[k][1] * 6));
				movValue += calc(sum);
			}
		}

		board.RestoreMove(x, y, color + 1);
		for (int u = r1, v = c1; u <= r2 && v <= c2; u += to[k][0], v += to[k][1]) {
			sum += getPoint(board.GetColor(u, v));
			if (abs(u - r1) + 1 == LEN || abs(v - c1) + 1 == LEN) {
				preValue += calc(sum);
			}
			if (abs(u - r1) + 1 >= LEN || abs(v - c1) + 1 >= LEN) {
				sum -= getPoint(board.GetColor(u - to[k][0] * 6, v - to[k][1] * 6));
				preValue += calc(sum);
			}
		}
	}

	return movValue - preValue;
}
int Reversi::CalcHeuristicValue2(pair <Position, Position> pos2, int color) {
	int value = CalcHeuristicValue(pos2.first, color);

	board.TakeMove(pos2.first, color + 1);
	value += CalcHeuristicValue(pos2.second, color);
	board.RestoreMove(pos2.first, color + 1);

	return value;
}
int Reversi::Minimax(pair <Position, Position> pos2, int depth, int alpha, int beta) {
	int color = ((SEARCH_DEPTH - depth) % 2) ^ ownColor;

	if (depth == 0) {
		return CalcHeuristicValue2(pos2, color);
	}

	vector <pair <pair <Position, Position>, int> > options2;
	MakeOption2Vec(options2, color);

	auto min = [](int a, int b)->int {
		return a < b ? a : b;
	};
	auto max = [](int a, int b)->int {
		return a > b ? a : b;
	};

	if ((SEARCH_DEPTH - depth) % 2) {
		// Opposite
		for (int i = 0; i < SEARCH_WIDTH; i++) {
			board.TakeMove(options2[i].first.first, (!ownColor) + 1);
			board.TakeMove(options2[i].first.second, (!ownColor) + 1);
			
			beta = min(beta, Minimax(options2[i].first, depth - 1, alpha, beta));

			board.RestoreMove(options2[i].first.first, (!ownColor) + 1);
			board.RestoreMove(options2[i].first.second, (!ownColor) + 1);

			if (beta <= alpha) {
				break;
			}
		}
		return beta;
	}
	else {
		// We
		for (int i = 0; i < SEARCH_WIDTH; i++) {
			board.TakeMove(options2[i].first.first, ownColor + 1);
			board.TakeMove(options2[i].first.second, ownColor + 1);

			alpha = max(alpha, Minimax(options2[i].first, depth - 1, alpha, beta));

			board.RestoreMove(options2[i].first.first, ownColor + 1);
			board.RestoreMove(options2[i].first.second, ownColor + 1);

			if (beta <= alpha) {
				break;
			}
		}
		return alpha;
	}
}
void Reversi::SearchForStep(int& r1, int& c1, int& r2, int& c2) {
	vector <pair <pair <Position, Position>, int> > options2;

	MakeOption2Vec(options2, ownColor);

	for (int i = 0; i < SEARCH_WIDTH; i++) {
		options2[i].second = Minimax(options2[i].first, SEARCH_DEPTH, INT_MIN, INT_MAX);
	}

	int maxValue = INT_MIN;
	for (int i = 0; i < SEARCH_WIDTH; i++) {
		if (options2[i].second > maxValue) {
			r1 = options2[i].first.first.first, c1 = options2[i].first.first.second;
			r2 = options2[i].first.second.first, c2 = options2[i].first.second.second;

			maxValue = options2[i].second;
		}
	}
}

pair<pair<int, int>, pair<int, int>> Reversi::step()
{
	//TODO:Generate a valid step here
	int r1 = -1, c1 = -1, r2 = -1, c2 = -1;

	int rmid = chessboard::BOARD_HEIGHT / 2, cmid = chessboard::BOARD_WIDTH / 2;

	for (int i = rmid - 1; i < chessboard::BOARD_HEIGHT; i++) {
		for (int j = cmid -1; j < chessboard::BOARD_WIDTH; j++) {
			if (board.GetColor(i, j) == 0) {
				if (r1 == -1) {
					r1 = i, c1 = j;
				}
				else {
					r2 = i, c2 = j;
					break;
				}
			}
		}
		if (r1 != -1 && r2 != -1) {
			break;
		}
	}

	auto make_ans = [&]() {
		pair<int, int> step1 = make_pair(r1, c1);
		pair<int, int> step2 = make_pair(r2, c2);
		return make_pair(step1, step2);
	};

	if (CertainStep(r1, c1, r2, c2)) {
		return make_ans();
	}

	// SearchForStep(r1, c1, r2, c2);

	// CheckCertainStep(r1, c1, r2, c2);

	return make_ans();


}

#include <fstream>

void Reversi::saveChessBoard()
{
	std::ofstream out(filename.data(), ios::app);
	board.Display(out);

	std::cout << "The chess board is already saved!" << std::endl;
}

void Reversi::handleMessage(int row1, int col1, int row2, int col2, int color) {
	board.TakeMove(row1, col1, color + 1);
	board.TakeMove(row2, col2, color + 1);

	saveChessBoard();
}
