#include "include/sista/sista.hpp"
#include <iostream>
#include <fstream>

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
    sista::Pawn* builder = new sista::Pawn(
        sista::Coordinates(0, 0),
        builder_style, '$'
    );
    field.addPawn(builder);
    field.print('&');

    std::cout << "Use {W, A, S, D}+ENTER to move the cursor" << std::endl;
    std::cout << "Use {P, R}+ENTER to place and remove a block" << std::endl;
    std::cout << "Use {Q}+ENTER to quit" << std::endl;

    while (true) {
        char input = getchar();
        switch (input) {
            case 'w': case 'W':
                if (cursor->get_coordinates().y > 0) {
                    field.movePawnBy(cursor, up);
                }
                break;
            case 'a': case 'A':
                if (cursor->get_coordinates().x > 0) {
                    field.movePawnBy(cursor, left);
                }
                break;
            case 's': case 'S':
                if (cursor->get_coordinates().y < HEIGHT - 1) {
                    field.movePawnBy(cursor, down);
                }
                break;
            case 'd': case 'D':
                if (cursor->get_coordinates().x < WIDTH - 1) {
                    field.movePawnBy(cursor, right);
                }
                break;
            case 'p': case 'P':
                sista::Coordinates coordinates = cursor->get_coordinates();
                coordinates.y++;
                if (coordinates.y < HEIGHT) {
                    level_file << coordinates.y << " " << coordinates.x << std::endl;
                    field.addPawn(new sista::Pawn(coordinates, builder_style, '#'));
                }
                break;
            case 'r': case 'R':
                sista::Coordinates coordinates = cursor->get_coordinates();
                coordinates.y++;
                if (coordinates.y < HEIGHT) {
                    level_file << coordinates.y << " " << coordinates.x << std::endl;
                    field.removePawnAt(coordinates);
                }
                break;
            case 'q': case 'Q':
                level_file.close();
                return 0;
        }
    }
}