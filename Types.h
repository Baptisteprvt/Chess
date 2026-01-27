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
    PAT
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
};

enum class TTFlag {
    EXACT,
    LOWERBOUND, // Alpha
    UPPERBOUND  // Beta
};

struct TTEntry {
    int depth;
    int value;
    TTFlag flag;
};

// Valeurs infinies pour l'algo
const int INF = 1000000;