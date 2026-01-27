#include <iostream>
#include <cmath> // Pour abs()

#include "Types.h"
#include "Board.h"

// --- INITIALISATION ---
void Board::Init(){
    ClearHistory(); 
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i < 2) {
                grid[i][j].color = Color::NOIR;
                grid[i][j].type = (i == 1) ? PieceType::PION : PieceType::VIDE; 
            } else if (i > 5) {
                grid[i][j].color = Color::BLANC;
                grid[i][j].type = (i == 6) ? PieceType::PION : PieceType::VIDE;
            } else {
                grid[i][j].type = PieceType::VIDE;
                grid[i][j].color = Color::AUCUNE;
            }
        }
    }

    // Placement Pièces
    grid[0][0].type = PieceType::TOUR; grid[0][7].type = PieceType::TOUR;
    grid[0][1].type = PieceType::CAVALIER; grid[0][6].type = PieceType::CAVALIER;
    grid[0][2].type = PieceType::FOU; grid[0][5].type = PieceType::FOU;
    grid[0][3].type = PieceType::DAME; grid[0][4].type = PieceType::ROI;

    grid[7][0].type = PieceType::TOUR; grid[7][7].type = PieceType::TOUR;
    grid[7][1].type = PieceType::CAVALIER; grid[7][6].type = PieceType::CAVALIER;
    grid[7][2].type = PieceType::FOU; grid[7][5].type = PieceType::FOU;
    grid[7][3].type = PieceType::DAME; grid[7][4].type = PieceType::ROI;

    sideToMove = Color::BLANC;
    whiteCanCastleKingside = true; whiteCanCastleQueenside = true;
    blackCanCastleKingside = true; blackCanCastleQueenside = true;
    enPassantCol = -1;
}

// --- DEPLACEMENT ---
void Board::MovePiece(Move move) {
    // 1. Sauvegardes et Détections
    PieceType type = grid[move.startY][move.startX].type;
    Color color = grid[move.startY][move.startX].color;
    
    bool isEnPassantCapture = false;
    if (type == PieceType::PION && abs(move.endX - move.startX) == 1 && grid[move.endY][move.endX].type == PieceType::VIDE) {
        isEnPassantCapture = true;
    }

    // 2. Mise à jour En Passant
    if(type == PieceType::PION && abs(move.endY - move.startY) == 2) enPassantCol = move.startX;
    else enPassantCol = -1;

    // --- CORRECTION MAJEURE : MISE A JOUR DES DROITS DE ROQUE ICI ---
    // On ne le fait PLUS dans MoveRoi/MoveTour, mais uniquement quand le coup est réellement joué.
    if (type == PieceType::ROI) {
        if (color == Color::BLANC) { whiteCanCastleKingside = false; whiteCanCastleQueenside = false; }
        else { blackCanCastleKingside = false; blackCanCastleQueenside = false; }
    }
    if (type == PieceType::TOUR) {
        if (color == Color::BLANC) {
            if (move.startX == 0 && move.startY == 7) whiteCanCastleQueenside = false;
            if (move.startX == 7 && move.startY == 7) whiteCanCastleKingside = false;
        } else {
            if (move.startX == 0 && move.startY == 0) blackCanCastleQueenside = false;
            if (move.startX == 7 && move.startY == 0) blackCanCastleKingside = false;
        }
    }

    // 3. Déplacement Physique
    grid[move.endY][move.endX] = grid[move.startY][move.startX];
    grid[move.startY][move.startX].type = PieceType::VIDE;
    grid[move.startY][move.startX].color = Color::AUCUNE;

    // 4. Prise En Passant
    if(isEnPassantCapture) {
        grid[move.startY][move.endX].type = PieceType::VIDE;
        grid[move.startY][move.endX].color = Color::AUCUNE;
    }

    // 5. Roque (Déplacement Tour)
    if (type == PieceType::ROI && abs(move.endX - move.startX) == 2) {
        int row = move.startY;
        if (move.endX > move.startX) { // Petit
            grid[row][5] = grid[row][7]; 
            grid[row][7].type = PieceType::VIDE; grid[row][7].color = Color::AUCUNE;
        } else { // Grand
            grid[row][3] = grid[row][0]; 
            grid[row][0].type = PieceType::VIDE; grid[row][0].color = Color::AUCUNE;
        }
    }

    // 6. Promotion
    if (type == PieceType::PION && (move.endY == 0 || move.endY == 7)) {
        grid[move.endY][move.endX].type = PieceType::DAME;
    }
}

// --- ANNULATION ---
void Board::UnMakeMove(Move move, Piece capturedPiece, PieceType originalType) {
    Piece movedPiece = grid[move.endY][move.endX];
    grid[move.startY][move.startX] = movedPiece;
    grid[move.startY][move.startX].type = originalType; 
    grid[move.endY][move.endX] = capturedPiece;

    // En Passant
    if(originalType == PieceType::PION && abs(move.endX - move.startX) == 1 && capturedPiece.type == PieceType::VIDE) {
        enPassantCol = move.endX; 
        grid[move.startY][move.endX].type = PieceType::PION;
        grid[move.startY][move.endX].color = (movedPiece.color == Color::BLANC) ? Color::NOIR : Color::BLANC;
    }

    // Roque (Remettre la Tour)
    if (originalType == PieceType::ROI && abs(move.endX - move.startX) == 2) {
        int row = move.startY;
        if (move.endX > move.startX) { // Petit
            grid[row][7] = grid[row][5]; 
            grid[row][5].type = PieceType::VIDE; grid[row][5].color = Color::AUCUNE;
        } else { // Grand
            grid[row][0] = grid[row][3];
            grid[row][3].type = PieceType::VIDE; grid[row][3].color = Color::AUCUNE;
        }
    }
    // Les flags de roque sont restaurés automatiquement par Undo() grâce à l'historique
}

// --- VALIDATION (SANS EFFETS DE BORD !) ---

bool Board::MovePion(Move move) {
    int dir = (grid[move.startY][move.startX].color == Color::BLANC) ? -1 : 1;
    int startRow = (grid[move.startY][move.startX].color == Color::BLANC) ? 6 : 1;

    // Avance 1
    if (move.endX == move.startX && move.endY == move.startY + dir && grid[move.endY][move.endX].type == PieceType::VIDE) return true;
    // Avance 2
    if (move.endX == move.startX && move.endY == move.startY + 2 * dir && move.startY == startRow &&
        grid[move.startY + dir][move.startX].type == PieceType::VIDE && grid[move.endY][move.endX].type == PieceType::VIDE) return true;
    // Capture
    if (abs(move.endX - move.startX) == 1 && move.endY == move.startY + dir && grid[move.endY][move.endX].type != PieceType::VIDE &&
        grid[move.endY][move.endX].color != grid[move.startY][move.startX].color) return true;
    // En Passant
    if(abs(move.endX - move.startX) == 1 && move.endY == move.startY + dir && grid[move.endY][move.endX].type == PieceType::VIDE &&
       move.endX == enPassantCol && grid[move.startY][move.endX].type == PieceType::PION &&
       grid[move.startY][move.endX].color != grid[move.startY][move.startX].color) return true;
    
    return false;
}

bool Board::MoveTour(Move move, Color sideToMove) {
    if (move.startX != move.endX && move.startY != move.endY) return false;
    int stepX = (move.endX - move.startX) == 0 ? 0 : (move.endX - move.startX) / abs(move.endX - move.startX);
    int stepY = (move.endY - move.startY) == 0 ? 0 : (move.endY - move.startY) / abs(move.endY - move.startY);
    int x = move.startX + stepX;
    int y = move.startY + stepY;
    while (x != move.endX || y != move.endY) {
        if (grid[y][x].type != PieceType::VIDE) return false;
        x += stepX; y += stepY;
    }
    // BUG FIX : Suppression de la modification des flags ici !
    return true;
}

bool Board::MoveCavalier(Move move) {
    return (abs(move.endX - move.startX) == 2 && abs(move.endY - move.startY) == 1) ||
           (abs(move.endX - move.startX) == 1 && abs(move.endY - move.startY) == 2);
}

bool Board::MoveFou(Move move) {
    // CORRECTION : Ajout de la parenthèse fermante après le deuxième abs(...)
    if (abs(move.endX - move.startX) != abs(move.endY - move.startY)) return false;

    int stepX = (move.endX - move.startX) / abs(move.endX - move.startX);
    int stepY = (move.endY - move.startY) / abs(move.endY - move.startY);
    int x = move.startX + stepX;
    int y = move.startY + stepY;
    
    while (x != move.endX && y != move.endY) {
        if (grid[y][x].type != PieceType::VIDE) return false;
        x += stepX; 
        y += stepY;
    }
    return true;
}

bool Board::MoveDame(Move move) {
    if (move.startX == move.endX || move.startY == move.endY) return MoveTour(move, sideToMove);
    return MoveFou(move);
}

bool Board::MoveRoi(Move move, Color sideToMove) {
    if (abs(move.endX - move.startX) <= 1 && abs(move.endY - move.startY) <= 1) {
        // BUG FIX : Suppression de la modification des flags ici !
        return true; 
    }
    if (abs(move.endX - move.startX) == 2 && move.endY == move.startY) return CanCastle(move, sideToMove);
    return false;
}

bool Board::CanCastle(Move move, Color sideToMove) {
    int row = (sideToMove == Color::BLANC) ? 7 : 0;
    if(move.endX > move.startX) { // Petit Roque
        if(sideToMove == Color::BLANC && !whiteCanCastleKingside) return false;
        if(sideToMove == Color::NOIR && !blackCanCastleKingside) return false;
        if(grid[row][5].type != PieceType::VIDE || grid[row][6].type != PieceType::VIDE) return false;
        Color enemy = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;
        if(IsSquareAttacked(4, row, enemy) || IsSquareAttacked(5, row, enemy) || IsSquareAttacked(6, row, enemy)) return false;
    } else { // Grand Roque
        if(sideToMove == Color::BLANC && !whiteCanCastleQueenside) return false;
        if(sideToMove == Color::NOIR && !blackCanCastleQueenside) return false;
        if(grid[row][1].type != PieceType::VIDE || grid[row][2].type != PieceType::VIDE || grid[row][3].type != PieceType::VIDE) return false;
        Color enemy = (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC;
        if(IsSquareAttacked(4, row, enemy) || IsSquareAttacked(3, row, enemy) || IsSquareAttacked(2, row, enemy)) return false;
    }
    return true;
}

bool Board::wasCastleInvolvedPiece(Move move, Color sideToMove) {
    // Helper conservé pour compatibilité, mais moins utile avec la nouvelle logique
    return false; 
}

// --- ATTAQUES ET LEGALITE ---

bool IsValid(int x, int y) { return x >= 0 && x < 8 && y >= 0 && y < 8; }

bool Board::IsSquareAttacked(int x, int y, Color attackerColor) {
    // 1. PIONS
    int pawnRow = (attackerColor == Color::BLANC) ? y + 1 : y - 1;
    if (IsValid(x - 1, pawnRow) && grid[pawnRow][x - 1].type == PieceType::PION && grid[pawnRow][x - 1].color == attackerColor) return true;
    if (IsValid(x + 1, pawnRow) && grid[pawnRow][x + 1].type == PieceType::PION && grid[pawnRow][x + 1].color == attackerColor) return true;

    // 2. CAVALIER
    int kx[] = { 1, 1, -1, -1, 2, 2, -2, -2 };
    int ky[] = { 2, -2, 2, -2, 1, -1, 1, -1 };
    for (int i = 0; i < 8; i++) if (IsValid(x+kx[i], y+ky[i]) && grid[y+ky[i]][x+kx[i]].type == PieceType::CAVALIER && grid[y+ky[i]][x+kx[i]].color == attackerColor) return true;

    // 3. LIGNES/DIAGONALES (Compacté)
    int dirs[8][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};
    for (int i = 0; i < 8; i++) {
        for (int dist = 1; dist < 8; dist++) {
            int nx = x + dirs[i][0]*dist, ny = y + dirs[i][1]*dist;
            if (!IsValid(nx, ny)) break;
            Piece p = grid[ny][nx];
            if (p.type != PieceType::VIDE) {
                if (p.color == attackerColor) {
                    if (i < 4 && (p.type == PieceType::TOUR || p.type == PieceType::DAME)) return true;
                    if (i >= 4 && (p.type == PieceType::FOU || p.type == PieceType::DAME)) return true;
                }
                break;
            }
        }
    }
    // 4. ROI
    for(int dx=-1;dx<=1;dx++) for(int dy=-1;dy<=1;dy++) if(IsValid(x+dx, y+dy) && grid[y+dy][x+dx].type == PieceType::ROI && grid[y+dy][x+dx].color == attackerColor) return true;

    return false;
}

bool Board::IsMoveLegal(Move move, Color sideToMove) {
    if(!IsValid(move.startX, move.startY) || !IsValid(move.endX, move.endY)) return false;
    if (grid[move.startY][move.startX].color != sideToMove || grid[move.endY][move.endX].color == sideToMove) return false;

    Piece p = grid[move.startY][move.startX];
    bool legal = false;
    switch (p.type) {
        case PieceType::PION: legal = MovePion(move); break;
        case PieceType::TOUR: legal = MoveTour(move, sideToMove); break;
        case PieceType::CAVALIER: legal = MoveCavalier(move); break;
        case PieceType::FOU: legal = MoveFou(move); break;
        case PieceType::DAME: legal = MoveDame(move); break;
        case PieceType::ROI: legal = MoveRoi(move, sideToMove); break;
        default: break;
    }
    if (!legal) return false;

    // Simulation
    Piece captured = grid[move.endY][move.endX];
    bool wK = whiteCanCastleKingside, wQ = whiteCanCastleQueenside;
    bool bK = blackCanCastleKingside, bQ = blackCanCastleQueenside;
    int ep = enPassantCol;

    MovePiece(move); 

    int kX = -1, kY = -1;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) if (grid[y][x].type == PieceType::ROI && grid[y][x].color == sideToMove) { kX = x; kY = y; }
    
    if (IsSquareAttacked(kX, kY, (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC)) legal = false;

    UnMakeMove(move, captured, p.type);
    
    // Restauration
    whiteCanCastleKingside = wK; whiteCanCastleQueenside = wQ;
    blackCanCastleKingside = bK; blackCanCastleQueenside = bQ;
    enPassantCol = ep;
    
    return legal;
}

std::vector<Move> Board::GenerateLegalMoves(Color side) {
    std::vector<Move> moves;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) if (grid[y][x].color == side) 
        for (int ey = 0; ey < 8; ey++) for (int ex = 0; ex < 8; ex++) {
            if (x==ex && y==ey) continue;
            Move m = {x, y, ex, ey};
            if (IsMoveLegal(m, side)) moves.push_back(m);
        }
    return moves;
}

bool Board::HasLegalMoves(Color sideToMove) { return !GenerateLegalMoves(sideToMove).empty(); }
GameStatus Board::GetGameStatus(Color sideToMove) {
    if (HasLegalMoves(sideToMove)) return GameStatus::EN_COURS;
    int kX = -1, kY = -1;
    for (int y = 0; y < 8; y++) for (int x = 0; x < 8; x++) if (grid[y][x].type == PieceType::ROI && grid[y][x].color == sideToMove) { kX = x; kY = y; }
    if (IsSquareAttacked(kX, kY, (sideToMove == Color::BLANC) ? Color::NOIR : Color::BLANC)) return GameStatus::ECHEC_ET_MAT;
    return GameStatus::PAT;
}
Piece Board::GetPiece(int x, int y) { if(IsValid(x,y)) return grid[y][x]; return {PieceType::VIDE, Color::AUCUNE}; }

void Board::SaveSnapshot(Move move) {
    GameHistory state;
    state.move = move;
    state.capturedPiece = grid[move.endY][move.endX];
    state.originalType = grid[move.startY][move.startX].type;
    state.enPassantCol = enPassantCol;
    state.wK = whiteCanCastleKingside; state.wQ = whiteCanCastleQueenside;
    state.bK = blackCanCastleKingside; state.bQ = blackCanCastleQueenside;
    history.push_back(state);
}
void Board::Undo() {
    if (history.empty()) return;
    GameHistory s = history.back(); history.pop_back();
    UnMakeMove(s.move, s.capturedPiece, s.originalType);
    enPassantCol = s.enPassantCol;
    whiteCanCastleKingside = s.wK; whiteCanCastleQueenside = s.wQ;
    blackCanCastleKingside = s.bK; blackCanCastleQueenside = s.bQ;
}
void Board::ClearHistory() { history.clear(); }