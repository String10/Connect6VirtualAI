#ifndef Reversi_h
#define Reversi_h
#include <stdio.h>
#include "ClientSocket.h"

#include "ChessBoard.h"

#include <vector>
#include <utility>

#define Position pair <int, int>

using namespace std;

class Reversi{
private:
    ClientSocket client_socket;
    int ownColor;
    int oppositeColor;

    std::string filename;
    chessboard::ChessBoard board;

	//function 

	 // according to chessman position (row , col) , generate one step message in order to send to server
    void generateOneStepMessage(int row1 , int col1, int row2, int col2);

    bool CertainStep(int& r1, int& c1, int& r2, int& c2);
    void CheckCertainStep(int& r1, int& c1, int& r2, int& c2);

    void MakeOptionVec(vector <pair <Position, int> >& options, int color);
    void MakeOption2Vec(vector <pair <pair <Position, Position>, int> >& options2, int color);
    int CalcHeuristicValue(Position pos, int color);
    int CalcHeuristicValue2(pair <Position, Position> pos2, int color);
    int Minimax(pair <Position, Position> pos2, int depth, int alpha, int beta);
    void SearchForStep(int& r1, int& c1, int& r2, int& c2);

public:
	pair<pair<int, int>, pair<int, int>> step();
    
    void saveChessBoard();

	void handleMessage(int row1, int col1, int row2, int col2, int color);
public:
    Reversi();
    ~Reversi();
	void setOwnColor(int color);

    void authorize(const char *id , const char *pass);
    
    void gameStart();
    
    void gameOver();
    
    void roundStart(int round);
    
    void oneRound();
    
    void roundOver(int round);
    
    int observe();
    
};

#endif /* Reversi_h */
