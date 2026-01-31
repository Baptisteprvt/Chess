#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <chrono>

#include "Types.h"
#include "Board.h"

const int T_CASE = 100;
const int BOARD_SIZE = 8 * T_CASE;
const int SIDEBAR_WIDTH = 300;
const int WINDOW_WIDTH = BOARD_SIZE + SIDEBAR_WIDTH;
const int AI_DEPTH = 6;

std::map<int, sf::Texture> textures;
sf::Font font;

void LoadResources()
{
    std::string pieceNames[] = {"", "pion", "cavalier", "fou", "tour", "dame", "roi"};

    for (int i = 1; i <= 6; i++)
    {
        std::string filenameWhite = "pieces/blancs/" + pieceNames[i] + "_blanc.png";
        if (!textures[i].loadFromFile(filenameWhite))
            std::cout << "Erreur: " << filenameWhite << std::endl;

        std::string filenameBlack = "pieces/noirs/" + pieceNames[i] + "_noir.png";
        if (!textures[i + 6].loadFromFile(filenameBlack))
            std::cout << "Erreur: " << filenameBlack << std::endl;
    }

    if (!font.loadFromFile("ARIAL.ttf"))
    {
        std::cout << "ERREUR : Police arial.ttf introuvable" << std::endl;
    }
}

int GetTextureID(Piece p)
{
    if (p.type == PieceType::VIDE)
        return 0;
    int id = (int)p.type;
    if (p.color == Color::NOIR)
        id += 6;
    return id;
}

void DrawButton(sf::RenderWindow &window, std::string label, int yPos, bool active)
{
    sf::RectangleShape rect(sf::Vector2f(250, 60));
    rect.setPosition(BOARD_SIZE + 25, yPos);

    if (active)
        rect.setFillColor(sf::Color(70, 70, 70));
    else
        rect.setFillColor(sf::Color(30, 30, 30));

    rect.setOutlineThickness(2);
    rect.setOutlineColor(sf::Color::White);
    window.draw(rect);

    sf::Text text;
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(24);
    text.setFillColor(active ? sf::Color::White : sf::Color(150, 150, 150));

    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    text.setPosition(rect.getPosition().x + rect.getSize().x / 2.0f, rect.getPosition().y + rect.getSize().y / 2.0f);
    window.draw(text);
}

int main()
{
    std::srand(std::time(nullptr));
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, BOARD_SIZE), "Chess Engine (Negamax)");
    window.setFramerateLimit(60);

    Board echiquier;
    echiquier.Init();
    LoadResources();
    Color playerColor;
    Color aiColor;
    if (std::rand() % 2 == 0)
    {
        playerColor = Color::BLANC;
        aiColor = Color::NOIR;
        // std::cout << "Vous jouez les BLANCS" << std::endl;
    }
    else
    {
        playerColor = Color::NOIR;
        aiColor = Color::BLANC;
        // std::cout << "Vous jouez les NOIRS" << std::endl;
    }
    Color sideToMove = Color::BLANC;

    bool isPieceSelected = false;
    sf::Vector2i selectedPos(-1, -1);

    sf::Sprite pieceSprite;
    GameStatus gameStatus = GameStatus::EN_COURS;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if (mouseX < BOARD_SIZE && gameStatus == GameStatus::EN_COURS && sideToMove == playerColor)
                {
                    int x = mouseX / T_CASE;
                    int y = mouseY / T_CASE;
                    if (playerColor == Color::NOIR)
                    {
                        x = 7 - x;
                        y = 7 - y;
                    }

                    if (x >= 0 && x < 8 && y >= 0 && y < 8)
                    {
                        if (!isPieceSelected)
                        {
                            Piece p = echiquier.GetPiece(x, y);
                            if (p.type != PieceType::VIDE && p.color == sideToMove)
                            {
                                selectedPos = sf::Vector2i(x, y);
                                isPieceSelected = true;
                            }
                        }
                        else
                        {
                            if (x == selectedPos.x && y == selectedPos.y)
                            {
                                isPieceSelected = false;
                            }
                            else
                            {
                                Move move = {selectedPos.x, selectedPos.y, x, y};

                                if (echiquier.IsMoveLegal(move, sideToMove))
                                {
                                    echiquier.SaveSnapshot(move);
                                    echiquier.MovePiece(move);

                                    sideToMove = aiColor;

                                    gameStatus = echiquier.GetGameStatus(sideToMove);
                                }
                                isPieceSelected = false;
                            }
                        }
                    }
                }
                else if (mouseX > BOARD_SIZE)
                {

                    if (mouseY >= 150 && mouseY <= 210)
                    {
                        if (sideToMove == playerColor)
                        {
                            echiquier.Undo();
                            echiquier.Undo();
                        }

                        gameStatus = GameStatus::EN_COURS;
                        isPieceSelected = false;
                    }

                    if (mouseY >= 250 && mouseY <= 310)
                    {
                        echiquier.Init();
                        echiquier.ClearTT();

                        if (std::rand() % 2 == 0)
                        {
                            playerColor = Color::BLANC;
                            aiColor = Color::NOIR;
                        }
                        else
                        {
                            playerColor = Color::NOIR;
                            aiColor = Color::BLANC;
                        }

                        sideToMove = Color::BLANC;
                        gameStatus = GameStatus::EN_COURS;
                        isPieceSelected = false;
                    }
                }
            }
        }

        if (sideToMove == aiColor && gameStatus == GameStatus::EN_COURS)
        {

            window.clear(sf::Color(40, 40, 40));
            Move aiMove = echiquier.GetBestMoveNegamax(AI_DEPTH, aiColor);

            echiquier.SaveSnapshot(aiMove);
            echiquier.MovePiece(aiMove);

            sideToMove = playerColor;
            gameStatus = echiquier.GetGameStatus(sideToMove);
            std::cout << "--------------------------" << std::endl;
        }

        window.clear(sf::Color(40, 40, 40));

        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                int drawX = (playerColor == Color::BLANC) ? x : (7 - x);
                int drawY = (playerColor == Color::BLANC) ? y : (7 - y);

                sf::RectangleShape square(sf::Vector2f(T_CASE, T_CASE));
                square.setPosition(drawX * T_CASE, drawY * T_CASE);
                if ((x + y) % 2 == 0)
                    square.setFillColor(sf::Color(238, 238, 210));
                else
                    square.setFillColor(sf::Color(118, 150, 86));
                window.draw(square);
            }
        }

        // Surbrillance dernier coup
        if (!echiquier.history.empty())
        {
            GameHistory lastMove = echiquier.history.back();
            int startX = (playerColor == Color::BLANC) ? lastMove.move.startX : 7 - lastMove.move.startX;
            int startY = (playerColor == Color::BLANC) ? lastMove.move.startY : 7 - lastMove.move.startY;
            int endX = (playerColor == Color::BLANC) ? lastMove.move.endX : 7 - lastMove.move.endX;
            int endY = (playerColor == Color::BLANC) ? lastMove.move.endY : 7 - lastMove.move.endY;

            sf::RectangleShape highlightStart(sf::Vector2f(T_CASE, T_CASE));
            highlightStart.setPosition(startX * T_CASE, startY * T_CASE);
            highlightStart.setFillColor(sf::Color(0, 255, 0, 160));
            window.draw(highlightStart);

            sf::RectangleShape highlightEnd(sf::Vector2f(T_CASE, T_CASE));
            highlightEnd.setPosition(endX * T_CASE, endY * T_CASE);
            highlightEnd.setFillColor(sf::Color(0, 255, 0, 160));
            window.draw(highlightEnd);
        }

        if (isPieceSelected)
        {
            int drawX = (playerColor == Color::BLANC) ? selectedPos.x : (7 - selectedPos.x);
            int drawY = (playerColor == Color::BLANC) ? selectedPos.y : (7 - selectedPos.y);
            sf::RectangleShape highlight(sf::Vector2f(T_CASE, T_CASE));
            highlight.setPosition(drawX * T_CASE, drawY * T_CASE);
            highlight.setFillColor(sf::Color(255, 255, 0, 100));
            window.draw(highlight);
        }

        if (isPieceSelected)
        {
            std::vector<Move> legalMoves = echiquier.GenerateLegalMoves(sideToMove);
            for (const Move &move : legalMoves)
            {
                if (move.startX == selectedPos.x && move.startY == selectedPos.y)
                {
                    int drawX = (playerColor == Color::BLANC) ? move.endX : (7 - move.endX);
                    int drawY = (playerColor == Color::BLANC) ? move.endY : (7 - move.endY);
                    sf::CircleShape circle(T_CASE / 8);
                    circle.setFillColor(sf::Color(0, 0, 255, 200));
                    circle.setPosition(drawX * T_CASE + T_CASE / 2 - circle.getRadius(),
                                       drawY * T_CASE + T_CASE / 2 - circle.getRadius());
                    window.draw(circle);
                }
            }
        }

        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                Piece p = echiquier.GetPiece(x, y);
                if (p.type != PieceType::VIDE)
                {
                    int drawX = (playerColor == Color::BLANC) ? x : (7 - x);
                    int drawY = (playerColor == Color::BLANC) ? y : (7 - y);
                    int id = GetTextureID(p);
                    pieceSprite.setTexture(textures[id]);

                    float scaleX = (float)T_CASE / textures[id].getSize().x;
                    float scaleY = (float)T_CASE / textures[id].getSize().y;
                    pieceSprite.setScale(scaleX, scaleY);

                    pieceSprite.setPosition(drawX * T_CASE, drawY * T_CASE);
                    window.draw(pieceSprite);
                }
            }
        }

        sf::Text statusText;
        statusText.setFont(font);
        statusText.setCharacterSize(22);
        statusText.setFillColor(sf::Color::White);
        statusText.setPosition(BOARD_SIZE + 15, 30);

        if (gameStatus == GameStatus::ECHEC_ET_MAT)
        {
            statusText.setString("ECHEC ET MAT !");
            statusText.setFillColor(sf::Color::Red);
        }
        else if (gameStatus == GameStatus::PAT)
        {
            statusText.setString("PAT (Match Nul)");
            statusText.setFillColor(sf::Color::Yellow);
        }
        else
        {
            if (sideToMove == playerColor)
            {
                std::string c = (playerColor == Color::BLANC) ? "(Blancs)" : "(Noirs)";
                statusText.setString("Ton tour " + c);
            }
            else
            {
                statusText.setString("IA reflechit...");
            }
        }
        window.draw(statusText);

        bool canInteract = (sideToMove == playerColor);
        DrawButton(window, "Annuler (Undo)", 150, canInteract);
        DrawButton(window, "Nouvelle Partie", 250, true);

        sf::Text infoText;
        infoText.setFont(font);
        infoText.setCharacterSize(18);
        infoText.setFillColor(sf::Color(150, 150, 150));
        infoText.setPosition(BOARD_SIZE + 15, 400);
        infoText.setString("Depth: " + std::to_string(AI_DEPTH) + "\nEngine: Negamax");
        window.draw(infoText);

        window.display();
    }

    return 0;
}