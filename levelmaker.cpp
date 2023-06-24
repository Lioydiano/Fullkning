#include "include/sista/sista.hpp"
#include <iostream>
#include <fstream>
#ifdef _WIN32
    #include <conio.h>
#endif

#define WIDTH 10
#define HEIGHT 20


ANSI::Settings builder_style(
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::REVERSE
);
sista::Coordinates up(-1, 0), down(1, 0), left(0, -1), right(0, 1);


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <level_name>" << std::endl;
        return 1;
    }
    std::string level_name = argv[1];
    std::string level_path = "levels/" + level_name + ".level"; // .level files are pairs of {y, x} coordinates
    if (std::ifstream(level_path)) {
        std::cout << "Level already exists" << std::endl;
        return 1;
    }
    std::ofstream level_file(level_path);
    if (!level_file) {
        std::cout << "Could not create level file" << std::endl;
        return 1;
    }
    std::cout << "Level file created" << std::endl;

    sista::Cursor cursor_handler;
    sista::Field field(WIDTH, HEIGHT);
    sista::Pawn* builder = new sista::Pawn('$', sista::Coordinates(1, 5), builder_style);
    field.addPawn(builder);
    field.print('&');

    std::cout << "\nUse {W, A, S, D}+ENTER to move the cursor" << std::endl;
    std::cout << "Use {P, R}+ENTER to place and remove a block" << std::endl;
    std::cout << "Use {Q}+ENTER to quit" << std::endl;

    while (true) {
        #ifdef _WIN32
            char input = getch();
        #elif __APPLE__
            cursor_handler.set({20, 30});
            char input = std::cin.get();
        #else
            cursor_handler.set({20, 30});
            char input = std::cin.get();
        #endif
        switch (input) {
            case 'w': case 'W':
                if (builder->getCoordinates().y > 0) {
                    try {
                        field.movePawnBy(builder, up);
                    } catch (std::invalid_argument& e) {}
                }
                break;
            case 'a': case 'A':
                if (builder->getCoordinates().x > 0) {
                    try {
                        field.movePawnBy(builder, left);
                    } catch (std::invalid_argument& e) {}
                }
                break;
            case 's': case 'S':
                if (builder->getCoordinates().y < HEIGHT - 1) {
                    try {
                        field.movePawnBy(builder, down);
                    } catch (std::invalid_argument& e) {}
                }
                break;
            case 'd': case 'D':
                if (builder->getCoordinates().x < WIDTH - 1) {
                    try {
                        field.movePawnBy(builder, right);
                    } catch (std::invalid_argument& e) {}
                }
                break;
            case 'p': case 'P': {
                sista::Coordinates coordinates = builder->getCoordinates();
                if (coordinates.y < 1)
                    continue; // Do not place blocks on the first/second row
                coordinates.y++;
                if (coordinates.y < HEIGHT) {
                    field.addPrintPawn(new sista::Pawn('#', coordinates, builder_style));
                }
                break;
            }
            case 'r': case 'R': {
                sista::Coordinates coordinates = builder->getCoordinates();
                coordinates.y++;
                if (coordinates.y < HEIGHT) {
                    sista::Pawn* deleted = field.getPawn(coordinates);
                    if (deleted != nullptr) {
                        if (deleted->getSymbol() == '#') {
                            field.removePawn(deleted);
                            delete deleted;
                            // Graphically remove the pawn
                            cursor_handler.set(coordinates);
                            ANSI::reset();
                            std::cout << ' ';
                        }
                    }
                }
                break;
            }
            case 'q': case 'Q':
                for (int row = 0; row < HEIGHT; row++) {
                    for (int col = 0; col < WIDTH; col++) {
                        sista::Coordinates coordinates(row, col);
                        if (field.getPawn(coordinates) != nullptr) {
                            if (field.getPawn(coordinates)->getSymbol() == '#')
                                level_file << coordinates.y << " " << coordinates.x << '\n';
                        }
                    }
                }
                level_file.close();
                std::cout << "Level saved" << std::endl;
                field.reset();
                return 0;
        }
    }
}