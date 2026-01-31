#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "Board.h"
#include "Types.h"

// Convertit "e2e4" en objet Move
Move ParseMove(std::string moveStr, Board &board)
{
    if (moveStr.length() < 4)
        return {-1, -1, -1, -1};

    int srcX = moveStr[0] - 'a';
    int srcY = 8 - (moveStr[1] - '0');
    int dstX = moveStr[2] - 'a';
    int dstY = 8 - (moveStr[3] - '0');

    std::vector<Move> moves = board.GenerateLegalMoves(board.sideToMove);
    for (auto &m : moves)
    {
        if (m.startX == srcX && m.startY == srcY && m.endX == dstX && m.endY == dstY)
        {
            return m;
        }
    }
    return {-1, -1, -1, -1};
}

std::string MoveToString(Move m)
{
    std::string s = "";
    s += (char)('a' + m.startX);
    s += std::to_string(8 - m.startY);
    s += (char)('a' + m.endX);
    s += std::to_string(8 - m.endY);
    return s;
}

void UciLoop()
{
    Board board;
    board.Init();

    std::string line, command;

    std::cout << "info string MOTEUR DEMARRE" << std::endl;

    while (std::getline(std::cin, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (line.empty())
            continue;

        std::istringstream iss(line);
        iss >> command;

        std::cout << "info string RECU: [" << command << "]" << std::endl;

        if (command == "uci")
        {
            std::cout << "id name MonMoteur_BLINDE" << std::endl;
            std::cout << "id author Toi" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (command == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "ucinewgame")
        {
            board.Init();
            board.ClearTT();
        }
        else if (command == "position")
        {
            std::string token;
            iss >> token;
            if (token == "startpos")
            {
                board.Init();
                iss >> token;
            }
            while (iss >> token)
            {
                if (token == "moves")
                    continue;
                Move m = ParseMove(token, board);
                if (m.startX != -1)
                {
                    board.MovePiece(m);
                }
            }
        }
        else if (command == "go")
        {
            Color us = board.sideToMove;
            Move best = board.GetBestMoveNegamax(6, us);
            std::cout << "bestmove " << MoveToString(best) << std::endl;
        }
        else if (command == "quit")
        {
            break;
        }
    }
}

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);
    std::cout.setf(std::ios::unitbuf);
    UciLoop();
    return 0;
}