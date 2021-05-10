#include "ChessBoard.h"
using namespace chessboard;

#include <cstring>
#include <iostream>

ChessBoard::ChessBoard() {
	std::memset(chessBoard, nullChess, sizeof(chessBoard));

	chessBoard[BOARD_HEIGHT / 2][BOARD_WIDTH / 2] = whiteChess;
}
bool chessboard::ChessBoard::CheckInside(int x, int y) const {
	if (x < 0 || x >= BOARD_HEIGHT || y < 0 || y >= BOARD_WIDTH) {
		return false;
	}
	return true;
}
int chessboard::ChessBoard::GetColor(int x, int y) const {
	return CheckInside(x, y) ? chessBoard[x][y] : -1;
}
bool ChessBoard::TakeMove(int x, int y, int color) {
	if (CheckInside(x, y) && GetColor(x, y) == 0) {
		chessBoard[x][y] = color;
		return true;
	}
	return false;
}
bool chessboard::ChessBoard::RestoreMove(int x, int y, int color) {
	if (CheckInside(x, y) && GetColor(x, y) == color) {
		chessBoard[x][y] = nullChess;
		return true;
	}
	return false;
}
void ChessBoard::Display(std::ostream& out) const {
	out << "===================" << std::endl;
	for (int i = 0; i < BOARD_HEIGHT; i++) {
		for (int j = 0; j < BOARD_WIDTH; j++) {
			if (chessBoard[i][j]) {
				out << chessBoard[i][j] << ' ';
			}
			else {
				out << "  ";
			}
		}
		out << std::endl;
	}
}