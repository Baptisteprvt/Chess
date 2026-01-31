#pragma once

enum class PieceType{
    VIDE = 0,
    PION,
    CAVALIER,
    FOU,
    TOUR,
    DAME,
    ROI
};

enum class Color{
    AUCUNE,
    BLANC,
    NOIR
};

enum class GameStatus {
    EN_COURS,
    ECHEC_ET_MAT,
    PAT,
    ECHEC
};

struct Piece
{
    PieceType type;
    Color color;
};

struct Move {
    int startX, startY;
    int endX, endY;
};

struct GameHistory {
    Move move;
    Piece capturedPiece;
    PieceType originalType;
    int enPassantCol;
    
    bool wK, wQ, bK, bQ;
    unsigned long long hash;
    int halfMoveClock;
};

enum class TTFlag {
    EXACT,
    LOWERBOUND,
    UPPERBOUND 
};

struct TTEntry {
    unsigned long long key;
    int depth;
    int value;
    TTFlag flag;
};

const int INF = 1000000;