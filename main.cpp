#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <string>
#include <thread> // Pour le petit temps de pause (optionnel)
#include <chrono>

#include "Types.h"
#include "Board.h"

// --- CONSTANTES ---
const int T_CASE = 100;
const int BOARD_SIZE = 8 * T_CASE;
const int SIDEBAR_WIDTH = 300; // Un peu plus large pour afficher les infos
const int WINDOW_WIDTH = BOARD_SIZE + SIDEBAR_WIDTH;

// --- GLOBALES ---
std::map<int, sf::Texture> textures;
sf::Font font;

// --- CHARGEMENT DES RESSOURCES ---
void LoadResources() {
    std::string pieceNames[] = {"", "pion", "cavalier", "fou", "tour", "dame", "roi"};
    
    for (int i = 1; i <= 6; i++) {
        // Chemins : assure-toi que tes dossiers "pieces/blancs" et "pieces/noirs" existent
        std::string filenameWhite = "pieces/blancs/" + pieceNames[i] + "_blanc.png";
        if (!textures[i].loadFromFile(filenameWhite)) std::cout << "Erreur: " << filenameWhite << std::endl;

        std::string filenameBlack = "pieces/noirs/" + pieceNames[i] + "_noir.png";
        if (!textures[i + 6].loadFromFile(filenameBlack)) std::cout << "Erreur: " << filenameBlack << std::endl;
    }

    // CHARGEMENT POLICE (Copie un fichier .ttf dans ton dossier projet)
    if (!font.loadFromFile("ARIAL.ttf")) {
        std::cout << "ERREUR CRITIQUE: Police arial.ttf introuvable !" << std::endl;
    }
}

int GetTextureID(Piece p) {
    if (p.type == PieceType::VIDE) return 0;
    int id = (int)p.type;
    if (p.color == Color::NOIR) id += 6;
    return id;
}

// --- DESSIN UI ---
void DrawButton(sf::RenderWindow& window, std::string label, int yPos, bool active) {
    sf::RectangleShape rect(sf::Vector2f(250, 60));
    rect.setPosition(BOARD_SIZE + 25, yPos);
    
    if(active) rect.setFillColor(sf::Color(70, 70, 70));
    else rect.setFillColor(sf::Color(30, 30, 30)); // Grisé si inactif

    rect.setOutlineThickness(2);
    rect.setOutlineColor(sf::Color::White);
    window.draw(rect);

    sf::Text text;
    text.setFont(font);
    text.setString(label);
    text.setCharacterSize(24);
    text.setFillColor(active ? sf::Color::White : sf::Color(150, 150, 150));
    
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(rect.getPosition().x + rect.getSize().x/2.0f, rect.getPosition().y + rect.getSize().y/2.0f);
    window.draw(text);
}

// --- MAIN ---
int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, BOARD_SIZE), "Chess Engine (Negamax)");
    window.setFramerateLimit(60); // Evite de surchauffer le GPU pour rien

    Board echiquier;
    echiquier.Init();
    LoadResources();

    Color sideToMove = Color::BLANC; // Les Blancs (Humain) commencent
    
    // Variables Souris
    bool isPieceSelected = false;
    sf::Vector2i selectedPos(-1, -1);
    
    sf::Sprite pieceSprite;
    GameStatus gameStatus = GameStatus::EN_COURS;
    
    // Profondeur de l'IA (3 est un bon début, 4 peut être lent sans optimisation poussée)
    const int AI_DEPTH = 4; 

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // --- GESTION CLIC SOURIS (Uniquement si c'est au tour de l'HUMAIN) ---
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                // A. Clic sur le Plateau
                if (mouseX < BOARD_SIZE && gameStatus == GameStatus::EN_COURS && sideToMove == Color::BLANC) {
                    int x = mouseX / T_CASE;
                    int y = mouseY / T_CASE;

                    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                        if (!isPieceSelected) {
                            // Sélection
                            Piece p = echiquier.GetPiece(x, y);
                            if (p.type != PieceType::VIDE && p.color == sideToMove) {
                                selectedPos = sf::Vector2i(x, y);
                                isPieceSelected = true;
                            }
                        } else {
                            // Déplacement
                            if (x == selectedPos.x && y == selectedPos.y) {
                                isPieceSelected = false; // Désélection
                            } else {
                                Move move = {selectedPos.x, selectedPos.y, x, y};

                                if (echiquier.IsMoveLegal(move, sideToMove)) {
                                    // 1. Sauvegarde & Mouvement
                                    echiquier.SaveSnapshot(move);
                                    echiquier.MovePiece(move);
                                    
                                    // 2. Changement de tour (IA va jouer)
                                    sideToMove = Color::NOIR;
                                    
                                    // 3. Vérif Fin de partie
                                    gameStatus = echiquier.GetGameStatus(sideToMove);
                                }
                                isPieceSelected = false;
                            }
                        }
                    }
                }
                // B. Clic Barre Latérale
                else if (mouseX > BOARD_SIZE) {
                    
                    // BOUTON UNDO (Y: 150-210)
                    if (mouseY >= 150 && mouseY <= 210) {
                        // Undo Spécial Humain vs IA : On annule 2 coups
                        // (Le coup de l'IA + Le coup du joueur) pour redonner la main au joueur
                        if (sideToMove == Color::BLANC) {
                            echiquier.Undo(); // Annule coup IA
                            echiquier.Undo(); // Annule coup Humain
                            // On reste aux Blancs
                        } 
                        // Cas rare : Si on clique Undo pendant que l'IA réfléchit (difficile) ou fin de partie
                        else {
                            echiquier.Undo();
                            sideToMove = Color::BLANC;
                        }
                        
                        gameStatus = GameStatus::EN_COURS; // On débloque si c'était Mat
                        isPieceSelected = false;
                    }

                    // BOUTON RESTART (Y: 250-310)
                    if (mouseY >= 250 && mouseY <= 310) {
                        echiquier.Init();
                        echiquier.ClearTT(); // Vider la mémoire de l'IA
                        sideToMove = Color::BLANC;
                        gameStatus = GameStatus::EN_COURS;
                        isPieceSelected = false;
                    }
                }
            }
        }

        // --- TOUR DE L'IA (NOIRS) ---
        if (sideToMove == Color::NOIR && gameStatus == GameStatus::EN_COURS) {
            
            // 1. On force le dessin pour voir le coup du joueur AVANT que l'IA réfléchisse
            // (Astuce simple : on dessine, display, puis on calcule)
            window.clear(sf::Color(40, 40, 40));
            // ... (Code de dessin copié plus bas, ou on laisse faire une frame) ...
            // Pour simplifier, on laisse l'IA geler la fenêtre pendant le calcul (normal au début)
            
            // 2. Calcul du meilleur coup
            // std::cout << "IA reflechit..." << std::endl;
            Move aiMove = echiquier.GetBestMoveNegamax(AI_DEPTH, Color::NOIR);
            
            // 3. Application
            echiquier.SaveSnapshot(aiMove);
            echiquier.MovePiece(aiMove);
            
            // 4. Changement de tour
            sideToMove = Color::BLANC;
            gameStatus = echiquier.GetGameStatus(sideToMove);
            std::cout << "--------------------------" << std::endl;

        }


        // --- DESSIN (RENDERING) ---
        window.clear(sf::Color(40, 40, 40));

        // 1. Plateau
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                sf::RectangleShape square(sf::Vector2f(T_CASE, T_CASE));
                square.setPosition(x * T_CASE, y * T_CASE);
                // Couleurs style "Chess.com" vert
                if ((x + y) % 2 == 0) square.setFillColor(sf::Color(238, 238, 210));
                else square.setFillColor(sf::Color(118, 150, 86));
                window.draw(square);
            }
        }

        // 2. Surbrillance dernier coup (optionnel, à faire plus tard)
        // ...

        // 3. Surbrillance Sélection
        if (isPieceSelected) {
            sf::RectangleShape highlight(sf::Vector2f(T_CASE, T_CASE));
            highlight.setPosition(selectedPos.x * T_CASE, selectedPos.y * T_CASE);
            highlight.setFillColor(sf::Color(255, 255, 0, 100)); // Jaune transparent
            window.draw(highlight);
        }

        // 4. Pièces
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                Piece p = echiquier.GetPiece(x, y);
                if (p.type != PieceType::VIDE) {
                    int id = GetTextureID(p);
                    pieceSprite.setTexture(textures[id]);
                    
                    // Auto-scale
                    float scaleX = (float)T_CASE / textures[id].getSize().x;
                    float scaleY = (float)T_CASE / textures[id].getSize().y;
                    pieceSprite.setScale(scaleX, scaleY);
                    
                    pieceSprite.setPosition(x * T_CASE, y * T_CASE);
                    window.draw(pieceSprite);
                }
            }
        }

        // 5. Interface Latérale
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setCharacterSize(22);
        statusText.setFillColor(sf::Color::White);
        statusText.setPosition(BOARD_SIZE + 15, 30);

        if (gameStatus == GameStatus::ECHEC_ET_MAT) {
            statusText.setString("ECHEC ET MAT !");
            statusText.setFillColor(sf::Color::Red);
        }
        else if (gameStatus == GameStatus::PAT) {
            statusText.setString("PAT (Match Nul)");
            statusText.setFillColor(sf::Color::Yellow);
        }
        else {
            std::string t = (sideToMove == Color::BLANC) ? "Ton tour (Blancs)" : "IA reflechit...";
            statusText.setString(t);
        }
        window.draw(statusText);

        // Boutons
        bool canInteract = (sideToMove == Color::BLANC); // Désactiver boutons quand IA réfléchit
        DrawButton(window, "Annuler (Undo)", 150, canInteract);
        DrawButton(window, "Nouvelle Partie", 250, true);

        // Info Debug
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