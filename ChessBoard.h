#pragma once
#include <iostream>
#include <utility>

namespace chessboard {

	const int BOARD_WIDTH = 19;
	const int BOARD_HEIGHT = 19;

	const int nullChess = 0;
	const int blackChess = 1;
	const int whiteChess = 2;

	class ChessBoard
	{
	private:
		int chessBoard[BOARD_HEIGHT][BOARD_WIDTH];
	public:
		ChessBoard();
		bool CheckInside(int x, int y) const;
		int GetColor(int x, int y) const;
		bool TakeMove(int x, int y, int color);
		bool TakeMove(std::pair<int, int> pos, int color);
		bool RestoreMove(int x, int y, int color);
		bool RestoreMove(std::pair<int, int> pos, int color);
		void Display(std::ostream& out) const;
	};

};

