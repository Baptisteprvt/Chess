#include <iostream>
#include <cmath>
#include <random>
#include "Types.h"
#include "Board.h"

unsigned long long RandomU64()
{
    static std::mt19937_64 gen(12345);
    static std::uniform_int_distribution<unsigned long long> dist;
    return dist(gen);
}

void Board::InitZobrist()
{
    for (int t = 0; t < 7; t++)
        for (int c = 0; c < 3; c++)
            for (int s = 0; s < 64; s++)
                zobristPiece[t][c][s] = RandomU64();

    zobristSide = RandomU64();

    for (int i = 0; i < 16; i++)
        zobristCastling[i] = RandomU64();
    for (int i = 0; i < 9; i++)
        zobristEnPassant[i] = RandomU64();
}

int Board::GetCastlingRights()
{
    int mask = 0;
    if (whiteCanCastleKingside)
        mask |= 1;
    if (whiteCanCastleQueenside)
        mask |= 2;
    if (blackCanCastleKingside)
        mask |= 4;
    if (blackCanCastleQueenside)
        mask |= 8;
    return mask;
}

void Board::Init()
{
    InitZobrist();
    ClearHistory();
    transpositionTable.resize(TT_SIZE);
    ClearTT();
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            grid[i][j] = {PieceType::VIDE, Color::AUCUNE};
            if (i == 1)
                grid[i][j] = {PieceType::PION, Color::NOIR};
            if (i == 6)
                grid[i][j] = {PieceType::PION, Color::BLANC};
        }
    }
    grid[0][0] = {PieceType::TOUR, Color::NOIR};
    grid[0][7] = {PieceType::TOUR, Color::NOIR};
    grid[0][1] = {PieceType::CAVALIER, Color::NOIR};
    grid[0][6] = {PieceType::CAVALIER, Color::NOIR};
    grid[0][2] = {PieceType::FOU, Color::NOIR};
    grid[0][5] = {PieceType::FOU, Color::NOIR};
    grid[0][3] = {PieceType::DAME, Color::NOIR};
    grid[0][4] = {PieceType::ROI, Color::NOIR};

    grid[7][0] = {PieceType::TOUR, Color::BLANC};
    grid[7][7] = {PieceType::TOUR, Color::BLANC};
    grid[7][1] = {PieceType::CAVALIER, Color::BLANC};
    grid[7][6] = {PieceType::CAVALIER, Color::BLANC};
    grid[7][2] = {PieceType::FOU, Color::BLANC};
    grid[7][5] = {PieceType::FOU, Color::BLANC};
    grid[7][3] = {PieceType::DAME, Color::BLANC};
    grid[7][4] = {PieceType::ROI, Color::BLANC};

    sideToMove = Color::BLANC;
    whiteCanCastleKingside = true;
    whiteCanCastleQueenside = true;
    blackCanCastleKingside = true;
    blackCanCastleQueenside = true;
    enPassantCol = -1;

    currentHash = 0;
    halfMoveClock = 0;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
        {
            Piece p = grid[y][x];
            if (p.type != PieceType::VIDE)
            {
                currentHash ^= zobristPiece[(int)p.type][(int)p.color][y * 8 + x];
            }
        }
    }
    if (sideToMove == Color::NOIR)
        currentHash ^= zobristSide;

    currentHash ^= zobristCastling[GetCastlingRights()];

    currentHash ^= zobristEnPassant[8];
}

void Board::MovePiece(Move move)
{
    currentHash ^= zobristSide;
    currentHash ^= zobristCastling[GetCastlingRights()];
    int epIndex = (enPassantCol == -1) ? 8 : enPassantCol;
    currentHash ^= zobristEnPassant[epIndex];

    Piece pMoved = grid[move.startY][move.startX];
    currentHash ^= zobristPiece[(int)pMoved.type][(int)pMoved.color][move.startY * 8 + move.startX];

    Piece pCaptured = grid[move.endY][move.endX];
    if (pCaptured.type != PieceType::VIDE)
    {
        currentHash ^= zobristPiece[(int)pCaptured.type][(int)pCaptured.color][move.endY * 8 + move.endX];
    }

    sideToMove = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;

    if (pMoved.type == PieceType::PION || pCaptured.type != PieceType::VIDE)
    {
        halfMoveClock = 0;
    }
    else
    {
        halfMoveClock++;
    }

    // Logique En Passant
    bool isEnPassantCapture = false;
    if (pMoved.type == PieceType::PION && abs(move.endX - move.startX) == 1 && grid[move.endY][move.endX].type == PieceType::VIDE)
    {
        isEnPassantCapture = true;
    }

    if (pMoved.type == PieceType::PION && abs(move.endY - move.startY) == 2)
        enPassantCol = move.startX;
    else
        enPassantCol = -1;

    // Logique Droits de Roque
    if (pMoved.type == PieceType::ROI)
    {
        if (pMoved.color == Color::BLANC)
        {
            whiteCanCastleKingside = false;
            whiteCanCastleQueenside = false;
        }
        else
        {
            blackCanCastleKingside = false;
            blackCanCastleQueenside = false;
        }
    }
    if (pMoved.type == PieceType::TOUR)
    {
        if (pMoved.color == Color::BLANC)
        {
            if (move.startX == 0 && move.startY == 7)
                whiteCanCastleQueenside = false;
            if (move.startX == 7 && move.startY == 7)
                whiteCanCastleKingside = false;
        }
        else
        {
            if (move.startX == 0 && move.startY == 0)
                blackCanCastleQueenside = false;
            if (move.startX == 7 && move.startY == 0)
                blackCanCastleKingside = false;
        }
    }
    // Si une tour est mangée, on perd le roque du coin correspondant
    if (pCaptured.type == PieceType::TOUR)
    {
        if (pCaptured.color == Color::BLANC)
        {
            if (move.endX == 0 && move.endY == 7)
                whiteCanCastleQueenside = false;
            if (move.endX == 7 && move.endY == 7)
                whiteCanCastleKingside = false;
        }
        else
        {
            if (move.endX == 0 && move.endY == 0)
                blackCanCastleQueenside = false;
            if (move.endX == 7 && move.endY == 0)
                blackCanCastleKingside = false;
        }
    }

    // Déplacement
    grid[move.endY][move.endX] = grid[move.startY][move.startX];
    grid[move.startY][move.startX] = {PieceType::VIDE, Color::AUCUNE};

    // Cas Spécial : Capture En Passant
    if (isEnPassantCapture)
    {
        Piece epPawn = grid[move.startY][move.endX];
        currentHash ^= zobristPiece[(int)epPawn.type][(int)epPawn.color][move.startY * 8 + move.endX];

        grid[move.startY][move.endX] = {PieceType::VIDE, Color::AUCUNE};
    }

    // Cas Spécial : Roque (Bouger la tour)
    if (pMoved.type == PieceType::ROI && abs(move.endX - move.startX) == 2)
    {
        int row = move.startY;
        int rookFromX, rookToX;
        if (move.endX > move.startX)
        { // Petit
            rookFromX = 7;
            rookToX = 5;
        }
        else
        { // Grand
            rookFromX = 0;
            rookToX = 3;
        }
        Piece rook = grid[row][rookFromX];
        currentHash ^= zobristPiece[(int)rook.type][(int)rook.color][row * 8 + rookFromX];

        grid[row][rookToX] = grid[row][rookFromX];
        grid[row][rookFromX] = {PieceType::VIDE, Color::AUCUNE};

        currentHash ^= zobristPiece[(int)rook.type][(int)rook.color][row * 8 + rookToX];
    }

    // Cas Spécial : Promotion
    PieceType finalType = pMoved.type;
    if (pMoved.type == PieceType::PION && (move.endY == 0 || move.endY == 7))
    {
        finalType = PieceType::DAME;
        grid[move.endY][move.endX].type = PieceType::DAME;
    }

    currentHash ^= zobristPiece[(int)finalType][(int)pMoved.color][move.endY * 8 + move.endX];

    currentHash ^= zobristCastling[GetCastlingRights()];

    int newEpIndex = (enPassantCol == -1) ? 8 : enPassantCol;
    currentHash ^= zobristEnPassant[newEpIndex];
}

void Board::UnMakeMove(Move move, Piece capturedPiece, PieceType originalType)
{

    sideToMove = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;

    Piece movedPiece = grid[move.endY][move.endX];
    // Undo promotion
    grid[move.startY][move.startX] = movedPiece;
    grid[move.startY][move.startX].type = originalType;

    grid[move.endY][move.endX] = capturedPiece;

    // Undo En Passant
    if (originalType == PieceType::PION && abs(move.endX - move.startX) == 1 && capturedPiece.type == PieceType::VIDE)
    {
        // Le pion capturé était à [startY][endX]
        grid[move.startY][move.endX].type = PieceType::PION;
        grid[move.startY][move.endX].color = (movedPiece.color == Color::BLANC) ? Color::NOIR : Color::BLANC;
    }

    // Undo Roque
    if (originalType == PieceType::ROI && abs(move.endX - move.startX) == 2)
    {
        int row = move.startY;
        if (move.endX > move.startX)
        { // Petit
            grid[row][7] = grid[row][5];
            grid[row][5] = {PieceType::VIDE, Color::AUCUNE};
        }
        else
        { // Grand
            grid[row][0] = grid[row][3];
            grid[row][3] = {PieceType::VIDE, Color::AUCUNE};
        }
    }
}

bool Board::MovePion(Move move)
{
    int dir = (grid[move.startY][move.startX].color == Color::BLANC) ? -1 : 1;
    int startRow = (grid[move.startY][move.startX].color == Color::BLANC) ? 6 : 1;

    // Avance de 1
    if (move.endX == move.startX && move.endY == move.startY + dir && grid[move.endY][move.endX].type == PieceType::VIDE)
        return true;
    // Avance de 2
    if (move.endX == move.startX && move.endY == move.startY + 2 * dir && move.startY == startRow &&
        grid[move.startY + dir][move.startX].type == PieceType::VIDE && grid[move.endY][move.endX].type == PieceType::VIDE)
        return true;
    // Capture
    if (abs(move.endX - move.startX) == 1 && move.endY == move.startY + dir && grid[move.endY][move.endX].type != PieceType::VIDE &&
        grid[move.endY][move.endX].color != grid[move.startY][move.startX].color)
        return true;
    // En Passant
    if (abs(move.endX - move.startX) == 1 && move.endY == move.startY + dir && grid[move.endY][move.endX].type == PieceType::VIDE &&
        move.endX == enPassantCol && grid[move.startY][move.endX].type == PieceType::PION &&
        grid[move.startY][move.endX].color != grid[move.startY][move.startX].color)
        return true;

    return false;
}

bool Board::MoveTour(Move move, Color sideToMove)
{
    if (move.startX != move.endX && move.startY != move.endY)
        return false;
    int stepX = (move.endX - move.startX) == 0 ? 0 : (move.endX - move.startX) / abs(move.endX - move.startX);
    int stepY = (move.endY - move.startY) == 0 ? 0 : (move.endY - move.startY) / abs(move.endY - move.startY);
    int x = move.startX + stepX;
    int y = move.startY + stepY;
    while (x != move.endX || y != move.endY)
    {
        if (grid[y][x].type != PieceType::VIDE)
            return false;
        x += stepX;
        y += stepY;
    }
    return true;
}

bool Board::MoveCavalier(Move move)
{
    return (abs(move.endX - move.startX) == 2 && abs(move.endY - move.startY) == 1) ||
           (abs(move.endX - move.startX) == 1 && abs(move.endY - move.startY) == 2);
}

bool Board::MoveFou(Move move)
{
    if (abs(move.endX - move.startX) != abs(move.endY - move.startY))
        return false;

    int stepX = (move.endX - move.startX) / abs(move.endX - move.startX);
    int stepY = (move.endY - move.startY) / abs(move.endY - move.startY);
    int x = move.startX + stepX;
    int y = move.startY + stepY;

    while (x != move.endX && y != move.endY)
    {
        if (grid[y][x].type != PieceType::VIDE)
            return false;
        x += stepX;
        y += stepY;
    }
    return true;
}

bool Board::MoveDame(Move move)
{
    if (move.startX == move.endX || move.startY == move.endY)
        return MoveTour(move, sideToMove);
    return MoveFou(move);
}

bool Board::MoveRoi(Move move, Color sideToMove)
{
    if (abs(move.endX - move.startX) <= 1 && abs(move.endY - move.startY) <= 1)
    {
        return true;
    }
    if (abs(move.endX - move.startX) == 2 && move.endY == move.startY)
        return CanCastle(move, sideToMove);
    return false;
}

bool Board::CanCastle(Move move, Color sideToMove)
{
    int row = (sideToMove == Color::BLANC) ? 7 : 0;
    if (move.endX > move.startX)
    { // Petit Roque
        if (sideToMove == Color::BLANC && !whiteCanCastleKingside)
            return false;
        if (sideToMove == Color::NOIR && !blackCanCastleKingside)
            return false;
        if (grid[row][5].type != PieceType::VIDE || grid[row][6].type != PieceType::VIDE)
            return false;
        Color enemy = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;
        if (IsSquareAttacked(4, row, enemy) || IsSquareAttacked(5, row, enemy) || IsSquareAttacked(6, row, enemy))
            return false;
    }
    else
    { // Grand Roque
        if (sideToMove == Color::BLANC && !whiteCanCastleQueenside)
            return false;
        if (sideToMove == Color::NOIR && !blackCanCastleQueenside)
            return false;
        if (grid[row][1].type != PieceType::VIDE || grid[row][2].type != PieceType::VIDE || grid[row][3].type != PieceType::VIDE)
            return false;
        Color enemy = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;
        if (IsSquareAttacked(4, row, enemy) || IsSquareAttacked(3, row, enemy) || IsSquareAttacked(2, row, enemy))
            return false;
    }
    return true;
}

bool IsValid(int x, int y) { return x >= 0 && x < 8 && y >= 0 && y < 8; }

bool Board::IsSquareAttacked(int x, int y, Color attackerColor)
{
    // PIONS
    int pawnRow = (attackerColor == Color::BLANC) ? y + 1 : y - 1;
    if (IsValid(x - 1, pawnRow) && grid[pawnRow][x - 1].type == PieceType::PION && grid[pawnRow][x - 1].color == attackerColor)
        return true;
    if (IsValid(x + 1, pawnRow) && grid[pawnRow][x + 1].type == PieceType::PION && grid[pawnRow][x + 1].color == attackerColor)
        return true;

    // CAVALIERS
    int kx[] = {1, 1, -1, -1, 2, 2, -2, -2};
    int ky[] = {2, -2, 2, -2, 1, -1, 1, -1};
    for (int i = 0; i < 8; i++)
        if (IsValid(x + kx[i], y + ky[i]) && grid[y + ky[i]][x + kx[i]].type == PieceType::CAVALIER && grid[y + ky[i]][x + kx[i]].color == attackerColor)
            return true;

    // FOUS, TOURS, DAMES
    int dirs[8][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
    for (int i = 0; i < 8; i++)
    {
        for (int dist = 1; dist < 8; dist++)
        {
            int nx = x + dirs[i][0] * dist, ny = y + dirs[i][1] * dist;
            if (!IsValid(nx, ny))
                break;
            Piece p = grid[ny][nx];
            if (p.type != PieceType::VIDE)
            {
                if (p.color == attackerColor)
                {
                    if (i < 4 && (p.type == PieceType::TOUR || p.type == PieceType::DAME))
                        return true;
                    if (i >= 4 && (p.type == PieceType::FOU || p.type == PieceType::DAME))
                        return true;
                }
                break;
            }
        }
    }
    // ROI
    for (int dx = -1; dx <= 1; dx++)
        for (int dy = -1; dy <= 1; dy++)
            if (IsValid(x + dx, y + dy) && grid[y + dy][x + dx].type == PieceType::ROI && grid[y + dy][x + dx].color == attackerColor)
                return true;

    return false;
}

bool Board::IsMoveLegal(Move move, Color sideToMove)
{
    if (!IsValid(move.startX, move.startY) || !IsValid(move.endX, move.endY))
        return false;
    if (grid[move.startY][move.startX].color != sideToMove || grid[move.endY][move.endX].color == sideToMove)
        return false;

    Piece p = grid[move.startY][move.startX];
    bool legal = false;
    switch (p.type)
    {
    case PieceType::PION:
        legal = MovePion(move);
        break;
    case PieceType::TOUR:
        legal = MoveTour(move, sideToMove);
        break;
    case PieceType::CAVALIER:
        legal = MoveCavalier(move);
        break;
    case PieceType::FOU:
        legal = MoveFou(move);
        break;
    case PieceType::DAME:
        legal = MoveDame(move);
        break;
    case PieceType::ROI:
        legal = MoveRoi(move, sideToMove);
        break;
    default:
        break;
    }
    if (!legal)
        return false;

    Piece captured = grid[move.endY][move.endX];
    bool wK = whiteCanCastleKingside, wQ = whiteCanCastleQueenside;
    bool bK = blackCanCastleKingside, bQ = blackCanCastleQueenside;
    int ep = enPassantCol;

    unsigned long long oldHash = currentHash;

    MovePiece(move);

    int kX = -1, kY = -1;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (grid[y][x].type == PieceType::ROI && grid[y][x].color == sideToMove)
            {
                kX = x;
                kY = y;
            }

    if (IsSquareAttacked(kX, kY, (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC))
        legal = false;

    UnMakeMove(move, captured, p.type);

    currentHash = oldHash;
    whiteCanCastleKingside = wK;
    whiteCanCastleQueenside = wQ;
    blackCanCastleKingside = bK;
    blackCanCastleQueenside = bQ;
    enPassantCol = ep;

    return legal;
}

std::vector<Move> Board::GenerateLegalMoves(Color side)
{
    std::vector<Move> moves;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (grid[y][x].color == side)
                for (int ey = 0; ey < 8; ey++)
                    for (int ex = 0; ex < 8; ex++)
                    {
                        if (x == ex && y == ey)
                            continue;
                        Move m = {x, y, ex, ey};
                        if (IsMoveLegal(m, side))
                            moves.push_back(m);
                    }
    return moves;
}

bool Board::HasLegalMoves(Color sideToMove)
{
    return !GenerateLegalMoves(sideToMove).empty();
}

GameStatus Board::GetGameStatus(Color sideToMove)
{
    if (HasLegalMoves(sideToMove))
        return GameStatus::EN_COURS;
    int kX = -1, kY = -1;
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (grid[y][x].type == PieceType::ROI && grid[y][x].color == sideToMove)
            {
                kX = x;
                kY = y;
            }
    if (IsSquareAttacked(kX, kY, (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC))
        return GameStatus::ECHEC_ET_MAT;
    return GameStatus::PAT;
}

Piece Board::GetPiece(int x, int y)
{
    if (IsValid(x, y))
        return grid[y][x];
    return {PieceType::VIDE, Color::AUCUNE};
}

void Board::SaveSnapshot(Move move)
{
    GameHistory state;
    state.move = move;
    state.capturedPiece = grid[move.endY][move.endX];
    state.originalType = grid[move.startY][move.startX].type;
    state.enPassantCol = enPassantCol;
    state.wK = whiteCanCastleKingside;
    state.wQ = whiteCanCastleQueenside;
    state.bK = blackCanCastleKingside;
    state.bQ = blackCanCastleQueenside;
    state.hash = currentHash;
    state.halfMoveClock = halfMoveClock;
    history.push_back(state);
}

void Board::Undo()
{
    if (history.empty())
        return;
    GameHistory s = history.back();
    history.pop_back();

    UnMakeMove(s.move, s.capturedPiece, s.originalType);

    // Restauration totale de l'état
    enPassantCol = s.enPassantCol;
    whiteCanCastleKingside = s.wK;
    whiteCanCastleQueenside = s.wQ;
    blackCanCastleKingside = s.bK;
    blackCanCastleQueenside = s.bQ;
    halfMoveClock = s.halfMoveClock;
    currentHash = s.hash;
}

void Board::ClearHistory() { history.clear(); }

int Board::Quiescence(int alpha, int beta)
{
    int stand_pat = Evaluate(sideToMove);

    if (stand_pat >= beta)
        return beta;
    if (alpha < stand_pat)
        alpha = stand_pat;

    std::vector<Move> moves = GenerateLegalMoves(sideToMove);
    moves = OrderMoves(moves);

    for (const auto &move : moves)
    {
        if (grid[move.endY][move.endX].type == PieceType::VIDE)
            continue;

        SaveSnapshot(move);
        MovePiece(move);
        int score = -Quiescence(-beta, -alpha);
        Undo();

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }
    return alpha;
}

bool Board::IsDraw()
{
    if (halfMoveClock >= 100)
        return true;

    int repetitionCount = 0;
    int n = history.size();

    for (int i = n - 1; i >= 0; i--)
    {
        GameHistory &h = history[i];

        if (h.hash == currentHash)
        {
            repetitionCount++;
        }

        if (h.halfMoveClock == 0)
            break;
    }

    return (repetitionCount >= 1);
}