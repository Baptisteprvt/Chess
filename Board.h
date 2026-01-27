#pragma once
#include "Types.h"
#include <vector>
#include <string>
#include <map>

class Board{
    private: 
        Piece grid[8][8] {};
        Color sideToMove;
        bool whiteCanCastleKingside;
        bool whiteCanCastleQueenside;
        bool blackCanCastleKingside;
        bool blackCanCastleQueenside;
        int enPassantCol;
        bool enpassant = false;
        std::vector<GameHistory> history;
        std::map<std::string, TTEntry> transpositionTable;
        std::vector<Move> GenerateLegalMoves(Color side);

    public:
        void Init();
        void Print();
        void MovePiece(Move move);
        bool IsMoveLegal(Move move, Color sideToMove);
        void UnMakeMove(Move move, Piece capturedPiece, PieceType originalType);
        bool IsSquareAttacked(int x, int y, Color attackerColor);
        bool HasLegalMoves(Color sideToMove);
        GameStatus GetGameStatus(Color sideToMove);
        bool MovePion(Move move);
        bool MoveTour(Move move, Color sideToMove);
        bool MoveCavalier(Move move);
        bool MoveFou(Move move);
        bool MoveDame(Move move);
        bool MoveRoi(Move move, Color sideToMove);
        bool CanCastle(Move move,Color sideToMove);
        bool wasCastleInvolvedPiece(Move move, Color sideToMove);
        Piece GetPiece(int x, int y);
        void SaveSnapshot(Move move);
        void Undo();
        void ClearHistory();

        int Evaluate(Color side); // Évalue la position
        int Negamax(int depth, int alpha, int beta, int color);
        std::string GenerateBoardKey(); // Crée une clé unique pour la TT
        std::vector<Move> OrderMoves(std::vector<Move>& moves); // Tri des coups
        Move GetBestMoveNegamax(int depth, Color aiColor); // La fonction à appeler dans le main
        void ClearTT(); // Vider la table
};