#include "Board.h"
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

// ... (Garde tes tables PST pawnTable, knightTable... inchangées ici) ...
// Pour gagner de la place, je ne les recolle pas, mais elles DOIVENT être là !

const int pawnTable[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0}, 
    {50, 50, 50, 50, 50, 50, 50, 50}, 
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0}, 
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10, 5}, 
    { 0,  0,  0,  0,  0,  0,  0,  0} 
};

const int knightTable[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30}, 
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50} 
};

const int bishopTable[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

const int rookTable[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0} 
};

const int queenTable[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

const int kingTable[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20} 
};

// 1. EVALUATION (CORRIGÉE : Même logique que précédemment)
int Board::Evaluate(Color side) {
    int score = 0;
    
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            Piece p = grid[y][x];
            if (p.type == PieceType::VIDE) continue;

            int value = 0;
            int positionBonus = 0;
            
            int tableY = (p.color == Color::BLANC) ? y : (7 - y); 
            int tableX = x; // (Symétrie gauche-droite automatique pour les tableaux fournis)

            switch (p.type) {
                case PieceType::PION: value = 100; positionBonus = pawnTable[tableY][tableX]; break;
                case PieceType::CAVALIER: value = 320; positionBonus = knightTable[tableY][tableX]; break;
                case PieceType::FOU: value = 330; positionBonus = bishopTable[tableY][tableX]; break;
                case PieceType::TOUR: value = 500; positionBonus = rookTable[tableY][tableX]; break;
                case PieceType::DAME: value = 900; positionBonus = queenTable[tableY][tableX]; break;
                case PieceType::ROI: value = 20000; positionBonus = kingTable[tableY][tableX]; break;
                default: break;
            }

            if (p.color == Color::BLANC) score += value + positionBonus;
            else score -= (value + positionBonus);
        }
    }
    return (side == Color::BLANC) ? score : -score;
}

// 2. GÉNÉRATION DE CLÉ (CORRIGÉE : Ajout des flags de roque !)
std::string Board::GenerateBoardKey() {
    std::string key = "";
    for(int y=0; y<8; y++) {
        for(int x=0; x<8; x++) {
            if(grid[y][x].type == PieceType::VIDE) key += ".";
            else {
                char c = '?';
                switch(grid[y][x].type) {
                    case PieceType::PION: c='p'; break;
                    case PieceType::CAVALIER: c='n'; break;
                    case PieceType::FOU: c='b'; break;
                    case PieceType::TOUR: c='r'; break;
                    case PieceType::DAME: c='q'; break;
                    case PieceType::ROI: c='k'; break;
                    default: break;
                }
                if(grid[y][x].color == Color::BLANC) c = toupper(c);
                key += c;
            }
        }
    }
    key += (sideToMove == Color::BLANC ? "w" : "b");
    
    // --- CORRECTION CRUCIALE ---
    // Si on ne met pas ça, l'IA pense qu'une position "sans roque" est la même
    // qu'une position "avec roque", et elle pioche le mauvais score dans la mémoire.
    if(whiteCanCastleKingside) key+="K";
    if(whiteCanCastleQueenside) key+="Q";
    if(blackCanCastleKingside) key+="k";
    if(blackCanCastleQueenside) key+="q";
    if (enPassantCol != -1) {
        key += "ep" + std::to_string(enPassantCol);
    }

    return key;
}

// 3. TRI DES COUPS
std::vector<Move> Board::OrderMoves(std::vector<Move>& moves) {
    std::vector<Move> orderedMoves;
    std::vector<Move> normalMoves;
    for (const auto& move : moves) {
        if (grid[move.endY][move.endX].type != PieceType::VIDE) orderedMoves.push_back(move);
        else normalMoves.push_back(move);
    }
    orderedMoves.insert(orderedMoves.end(), normalMoves.begin(), normalMoves.end());
    return orderedMoves;
}

void Board::ClearTT() { transpositionTable.clear(); }

// 4. NEGAMAX
int Board::Negamax(int depth, int alpha, int beta, int color) {
    int alphaOrig = alpha;

    std::string nodeKey = GenerateBoardKey();
    if (transpositionTable.count(nodeKey)) {
        TTEntry entry = transpositionTable[nodeKey];
        if (entry.depth >= depth) {
            if (entry.flag == TTFlag::EXACT) return entry.value;
            else if (entry.flag == TTFlag::LOWERBOUND) alpha = std::max(alpha, entry.value);
            else if (entry.flag == TTFlag::UPPERBOUND) beta = std::min(beta, entry.value);
            if (alpha >= beta) return entry.value;
        }
    }

    GameStatus status = GetGameStatus(sideToMove);
    if (depth == 0 || status != GameStatus::EN_COURS) {
        if (status == GameStatus::ECHEC_ET_MAT) return -INF + (100 - depth); 
        if (status == GameStatus::PAT) return 0;
        return Evaluate(sideToMove); 
    }

    std::vector<Move> childNodes = GenerateLegalMoves(sideToMove);
    childNodes = OrderMoves(childNodes);

    int value = -INF;
    for (const auto& child : childNodes) {
        SaveSnapshot(child);
        MovePiece(child);
        Color oldSide = sideToMove;
        sideToMove = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;

        int eval = -Negamax(depth - 1, -beta, -alpha, -color);
        
        sideToMove = oldSide;
        Undo();

        value = std::max(value, eval);
        alpha = std::max(alpha, value);
        if (alpha >= beta) break; 
    }

    TTEntry newEntry;
    newEntry.value = value;
    newEntry.depth = depth;
    if (value <= alphaOrig) newEntry.flag = TTFlag::UPPERBOUND;
    else if (value >= beta) newEntry.flag = TTFlag::LOWERBOUND;
    else newEntry.flag = TTFlag::EXACT;
    transpositionTable[nodeKey] = newEntry;

    return value;
}

// 5. LANCEMENT IA
Move Board::GetBestMoveNegamax(int depth, Color aiColor) {
    ClearTT(); // Sécurité pour éviter les corruptions entre tours

    std::vector<Move> moves = GenerateLegalMoves(aiColor);
    moves = OrderMoves(moves); 
    
    Move bestMove = moves[0];
    int bestValue = -INF;
    int alpha = -INF;
    int beta = INF;

    for (const auto& move : moves) {
        SaveSnapshot(move);
        MovePiece(move);
        Color oldSide = sideToMove;
        sideToMove = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;

        int val = -Negamax(depth - 1, -beta, -alpha, 1);
        
        sideToMove = oldSide;
        Undo();

        std::cout << "Coup " << (char)('a'+move.startX) << 8-move.startY 
                  << (char)('a'+move.endX) << 8-move.endY 
                  << " Score: " << val << std::endl;

        if (val > bestValue) {
            bestValue = val;
            bestMove = move;
        }
        alpha = std::max(alpha, bestValue);
    }
    std::cout << ">>> CHOIX FINAL: " << bestValue << std::endl;
    return bestMove;
}