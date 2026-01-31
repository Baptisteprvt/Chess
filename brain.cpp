#include "Board.h"
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

const int pawnTable[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},
    {5, -5, -10, 0, 0, -10, -5, 5},
    {5, 10, 10, -20, -20, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0}};

const int knightTable[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20, 0, 0, 0, 0, -20, -40},
    {-30, 0, 10, 15, 15, 10, 0, -30},
    {-30, 5, 15, 20, 20, 15, 5, -30},
    {-30, 0, 15, 20, 20, 15, 0, -30},
    {-30, 5, 10, 15, 15, 10, 5, -30},
    {-40, -20, 0, 5, 5, 0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}};

const int bishopTable[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-10, 0, 5, 10, 10, 5, 0, -10},
    {-10, 5, 5, 10, 10, 5, 5, -10},
    {-10, 0, 10, 10, 10, 10, 0, -10},
    {-10, 10, 10, 10, 10, 10, 10, -10},
    {-10, 5, 0, 0, 0, 0, 5, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}};

const int rookTable[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {5, 10, 10, 10, 10, 10, 10, 5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {-5, 0, 0, 0, 0, 0, 0, -5},
    {0, 0, 0, 5, 5, 0, 0, 0}};

const int queenTable[8][8] = {
    {-20, -10, -10, -5, -5, -10, -10, -20},
    {-10, 0, 0, 0, 0, 0, 0, -10},
    {-10, 0, 5, 5, 5, 5, 0, -10},
    {-5, 0, 5, 5, 5, 5, 0, -5},
    {0, 0, 5, 5, 5, 5, 0, -5},
    {-10, 5, 5, 5, 5, 5, 0, -10},
    {-10, 0, 5, 0, 0, 0, 0, -10},
    {-20, -10, -10, -5, -5, -10, -10, -20}};

const int kingTable[8][8] = {
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    {20, 20, 0, 0, 0, 0, 20, 20},
    {20, 30, 10, 0, 0, 10, 30, 20}};

const int kingEndgameTable[8][8] = {
    {-50, -40, -30, -20, -20, -30, -40, -50},
    {-30, -20, -10, 0, 0, -10, -20, -30},
    {-30, -10, 20, 30, 30, 20, -10, -30},
    {-30, -10, 30, 40, 40, 30, -10, -30},
    {-30, -10, 30, 40, 40, 30, -10, -30},
    {-30, -10, 20, 30, 30, 20, -10, -30},
    {-30, -30, 0, 0, 0, 0, -30, -30},
    {-50, -30, -30, -30, -30, -30, -30, -50}};

const int DOUBLED_PAWN_PENALTY = 20;
const int ISOLATED_PAWN_PENALTY = 20;
const int BISHOP_PAIR_BONUS = 40;
const int ROOK_OPEN_FILE_BONUS = 25;
const int ROOK_SEMI_OPEN_FILE_BONUS = 10;

int Board::Evaluate(Color side)
{
    int score = 0;
    int totalMaterial = 0;

    int pawnsOnCol[2][8] = {{0}};

    int bishopsCount[2] = {0};

    int wKingX = -1, wKingY = -1;
    int bKingX = -1, bKingY = -1;

    const int passedPawnBonus[8] = {0, 5, 10, 20, 40, 80, 150, 0};

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            Piece p = grid[y][x];
            if (p.type == PieceType::VIDE)
                continue;

            int value = 0;
            int positionBonus = 0;

            int cIdx = (p.color == Color::BLANC) ? 0 : 1;

            int tableY = (p.color == Color::BLANC) ? y : (7 - y);
            int tableX = x;

            switch (p.type)
            {
            case PieceType::PION:
                value = 100;
                totalMaterial += 100;
                positionBonus = pawnTable[tableY][tableX];

                pawnsOnCol[cIdx][x]++;

                {
                    bool isPassed = true;
                    int dir = (p.color == Color::BLANC) ? -1 : 1;
                    for (int checkX = x - 1; checkX <= x + 1; checkX++)
                    {
                        if (checkX < 0 || checkX > 7)
                            continue;
                        int checkY = y + dir;
                        while (checkY >= 0 && checkY < 8)
                        {
                            Piece obstacle = grid[checkY][checkX];
                            if (obstacle.type == PieceType::PION && obstacle.color != p.color)
                            {
                                isPassed = false;
                                break;
                            }
                            checkY += dir;
                        }
                        if (!isPassed)
                            break;
                    }
                    if (isPassed)
                    {
                        int rankProgress = (p.color == Color::BLANC) ? (7 - y) : y;
                        if (p.color == Color::BLANC)
                            score += passedPawnBonus[rankProgress];
                        else
                            score -= passedPawnBonus[rankProgress];
                    }
                }
                break;

            case PieceType::CAVALIER:
                value = 320;
                totalMaterial += 320;
                positionBonus = knightTable[tableY][tableX];
                break;

            case PieceType::FOU:
                value = 330;
                totalMaterial += 330;
                positionBonus = bishopTable[tableY][tableX];

                bishopsCount[cIdx]++;
                break;

            case PieceType::TOUR:
                value = 500;
                totalMaterial += 500;
                positionBonus = rookTable[tableY][tableX];

                break;

            case PieceType::DAME:
                value = 900;
                totalMaterial += 900;
                positionBonus = queenTable[tableY][tableX];
                break;

            case PieceType::ROI:
                value = 20000;
                if (p.color == Color::BLANC)
                {
                    wKingX = x;
                    wKingY = y;
                }
                else
                {
                    bKingX = x;
                    bKingY = y;
                }
                break;

            default:
                break;
            }

            if (p.color == Color::BLANC)
                score += value + positionBonus;
            else
                score -= (value + positionBonus);
        }
    }

    for (int x = 0; x < 8; x++)
    {
        if (pawnsOnCol[0][x] > 0)
        {
            if (pawnsOnCol[0][x] > 1)
                score -= DOUBLED_PAWN_PENALTY;

            bool left = (x > 0) ? (pawnsOnCol[0][x - 1] > 0) : false;
            bool right = (x < 7) ? (pawnsOnCol[0][x + 1] > 0) : false;
            if (!left && !right)
                score -= ISOLATED_PAWN_PENALTY;
        }

        if (pawnsOnCol[1][x] > 0)
        {
            if (pawnsOnCol[1][x] > 1)
                score += DOUBLED_PAWN_PENALTY;

            bool left = (x > 0) ? (pawnsOnCol[1][x - 1] > 0) : false;
            bool right = (x < 7) ? (pawnsOnCol[1][x + 1] > 0) : false;
            if (!left && !right)
                score += ISOLATED_PAWN_PENALTY;
        }
    }

    if (bishopsCount[0] >= 2)
        score += BISHOP_PAIR_BONUS;
    if (bishopsCount[1] >= 2)
        score -= BISHOP_PAIR_BONUS;

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            if (grid[y][x].type == PieceType::TOUR)
            {
                int cIdx = (grid[y][x].color == Color::BLANC) ? 0 : 1;
                int oppIdx = 1 - cIdx;

                if (pawnsOnCol[cIdx][x] == 0 && pawnsOnCol[oppIdx][x] == 0)
                {
                    if (cIdx == 0)
                        score += ROOK_OPEN_FILE_BONUS;
                    else
                        score -= ROOK_OPEN_FILE_BONUS;
                }
                else if (pawnsOnCol[cIdx][x] == 0)
                {
                    if (cIdx == 0)
                        score += ROOK_SEMI_OPEN_FILE_BONUS;
                    else
                        score -= ROOK_SEMI_OPEN_FILE_BONUS;
                }
            }
        }
    }

    bool isEndgame = (totalMaterial < 1500);

    if (wKingX != -1)
    {
        int bonus = isEndgame ? kingEndgameTable[wKingY][wKingX] : kingTable[wKingY][wKingX];
        score += bonus;
    }
    if (bKingX != -1)
    {
        int bonus = isEndgame ? kingEndgameTable[7 - bKingY][bKingX] : kingTable[7 - bKingY][bKingX];
        score -= bonus;
    }

    return (side == Color::BLANC) ? score : -score;
}

const int mvvLvaValues[] = {0, 100, 300, 300, 500, 900, 10000};

std::vector<Move> Board::OrderMoves(std::vector<Move> &moves)
{
    struct MoveScore
    {
        Move move;
        int score;
    };

    std::vector<MoveScore> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const auto &move : moves)
    {
        int score = 0;
        Piece captured = grid[move.endY][move.endX];
        Piece attacker = grid[move.startY][move.startX];

        if (captured.type != PieceType::VIDE)
        {
            int victimVal = mvvLvaValues[(int)captured.type];
            int attackerVal = mvvLvaValues[(int)attacker.type];

            score = (victimVal * 10) - attackerVal;

            score += 10000;
        }

        scoredMoves.push_back({move, score});
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore &a, const MoveScore &b)
              { return a.score > b.score; });

    std::vector<Move> result;
    result.reserve(moves.size());
    for (const auto &s : scoredMoves)
    {
        result.push_back(s.move);
    }

    return result;
}

void Board::ClearTT()
{
    std::fill(transpositionTable.begin(), transpositionTable.end(), TTEntry{0, 0, 0, TTFlag::EXACT});
}

int Board::Negamax(int depth, int alpha, int beta, int color)
{
    int alphaOrig = alpha;

    unsigned long long key = currentHash;
    int ttIndex = key % TT_SIZE;
    TTEntry &entry = transpositionTable[ttIndex];

    if (entry.key == key && entry.depth >= depth)
    {
        if (entry.flag == TTFlag::EXACT)
            return entry.value;
        else if (entry.flag == TTFlag::LOWERBOUND)
            alpha = std::max(alpha, entry.value);
        else if (entry.flag == TTFlag::UPPERBOUND)
            beta = std::min(beta, entry.value);
        if (alpha >= beta)
            return entry.value;
    }
    if (IsDraw())
    {
        return 0;
    }

    GameStatus status = GetGameStatus(sideToMove);
    if (depth == 0 || status != GameStatus::EN_COURS)
    {
        if (status == GameStatus::ECHEC_ET_MAT)
            return -INF + (100 - depth);
        if (status == GameStatus::PAT)
            return 0;
        if (depth == 0)
            return Quiescence(alpha, beta);
        return Evaluate(sideToMove);
    }

    std::vector<Move> childNodes = GenerateLegalMoves(sideToMove);
    childNodes = OrderMoves(childNodes);

    int value = -INF;
    for (const auto &child : childNodes)
    {
        SaveSnapshot(child);
        MovePiece(child);

        int eval = -Negamax(depth - 1, -beta, -alpha, -color);

        Undo();

        value = std::max(value, eval);
        alpha = std::max(alpha, value);
        if (alpha >= beta)
            break;
    }

    // Sauvegarde dans la TT
    entry.value = value;
    entry.depth = depth;
    entry.key = key;
    if (value <= alphaOrig)
        entry.flag = TTFlag::UPPERBOUND;
    else if (value >= beta)
        entry.flag = TTFlag::LOWERBOUND;
    else
        entry.flag = TTFlag::EXACT;

    return value;
}

Move Board::GetBestMoveNegamax(int depth, Color aiColor)
{
    int pieceCount = 0;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (grid[y][x].type != PieceType::VIDE)
                pieceCount++;

    // Profondeur dynamique
    int realDepth = depth;

    if (pieceCount <= 8)
        realDepth = depth + 4; // Finale très épurée
    else if (pieceCount <= 14)
        realDepth = depth + 2; // Finale standard
    else if (pieceCount <= 20)
        realDepth = depth + 1; // Milieu de jeu

    std::cout << "--- RECHERCHE (Pieces: " << pieceCount << " | Depth: " << realDepth << ") ---" << std::endl;

    // ClearTT();

    std::vector<Move> moves = GenerateLegalMoves(aiColor);
    moves = OrderMoves(moves);

    Move bestMove = moves[0];
    int bestValue = -INF;
    int alpha = -INF;
    int beta = INF;

    for (const auto &move : moves)
    {
        SaveSnapshot(move);
        MovePiece(move);

        int val = -Negamax(realDepth - 1, -beta, -alpha, 1);

        Undo();

        std::cout << "Coup " << (char)('a' + move.startX) << 8 - move.startY
                  << (char)('a' + move.endX) << 8 - move.endY
                  << " Score: " << val << std::endl;

        if (val > bestValue)
        {
            bestValue = val;
            bestMove = move;
        }
        alpha = std::max(alpha, bestValue);
    }
    std::cout << ">>> CHOIX FINAL: " << bestValue << std::endl;
    return bestMove;
}