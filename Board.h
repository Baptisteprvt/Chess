#pragma once
#include "Types.h"
#include <vector>
#include <string>

class Board{
    private: 
        Piece grid[8][8] {};
        
        bool whiteCanCastleKingside;
        bool whiteCanCastleQueenside;
        bool blackCanCastleKingside;
        bool blackCanCastleQueenside;

        int halfMoveClock;
        int enPassantCol;
        
        // --- ZOBRIST HASHING ---
        unsigned long long zobristPiece[7][3][64]; // [Type][Color][Square]
        unsigned long long zobristSide;            // Si c'est aux noirs
        unsigned long long zobristCastling[16];    // 4 bits pour les roques
        unsigned long long zobristEnPassant[9];    // 8 colonnes + 1 "aucun"
        unsigned long long currentHash;            // Hash actuel du plateau

        // --- TRANSPOSITION TABLE (Optimisée) ---
        std::vector<TTEntry> transpositionTable;
        const int TT_SIZE = 1048576; // 2^20 entrées (~1 million)

    public:
        Color sideToMove;

        void Init();
        void InitZobrist(); // Initialise les nombres aléatoires
        void MovePiece(Move move);
        bool IsMoveLegal(Move move, Color sideToMove);
        void UnMakeMove(Move move, Piece capturedPiece, PieceType originalType);
        bool IsSquareAttacked(int x, int y, Color attackerColor);
        bool HasLegalMoves(Color sideToMove);
        GameStatus GetGameStatus(Color sideToMove);
        bool IsDraw();
        // Mouvements spécifiques
        bool MovePion(Move move);
        bool MoveTour(Move move, Color sideToMove);
        bool MoveCavalier(Move move);
        bool MoveFou(Move move);
        bool MoveDame(Move move);
        bool MoveRoi(Move move, Color sideToMove);
        bool CanCastle(Move move, Color sideToMove);
        
        Piece GetPiece(int x, int y);
        
        void SaveSnapshot(Move move);
        void Undo();
        void ClearHistory();
        std::vector<GameHistory> history;
        std::vector<Move> GenerateLegalMoves(Color side);

        // IA & Eval
        int Evaluate(Color side);
        int Quiescence(int alpha, int beta);
        int Negamax(int depth, int alpha, int beta, int color);
        
        std::vector<Move> OrderMoves(std::vector<Move>& moves);
        Move GetBestMoveNegamax(int depth, Color aiColor);
        
        void ClearTT();
        
        // Helper interne
        int GetCastlingRights(); 
};