#include "include/sista/sista.hpp"
#include <algorithm>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#ifdef _WIN32
    #include <windows.h>
#endif
#ifdef _WIN32
    #include <conio.h>
#elif __APPLE__
    #include <termios.h>

    struct termios orig_termios;
    void term_echooff() {
        struct termios noecho;

        tcgetattr(0, &orig_termios);

        noecho = orig_termios;
        noecho.c_lflag &= ~ECHO;

        tcsetattr(0, TCSANOW, &noecho);
    }
#elif __linux__    
    #include <unistd.h>
    #include <termios.h>

    char getch(void) {
        char buf = 0;
        struct termios old = {0};
        fflush(stdout);
        if(tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if(tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");
        if(read(0, &buf, 1) < 0)
            perror("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if(tcsetattr(0, TCSADRAIN, &old) < 0)
            perror("tcsetattr ~ICANON");
        // printf("%c\n", buf);
        return buf;
    }
#endif

#define WIDTH 10
#define HEIGHT 20
#define COOLDOWN 3

sista::Coordinates one_down(1, 0);

ANSI::Settings description_style(
    ANSI::ForegroundColor::F_WHITE,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
);
ANSI::Settings sand_style(
    ANSI::ForegroundColor::F_WHITE,
    ANSI::BackgroundColor::B_YELLOW,
    ANSI::Attribute::BRIGHT
);
ANSI::Settings stone_style(
    ANSI::ForegroundColor::F_WHITE,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::BRIGHT
);
ANSI::Settings virtual_style(
    ANSI::ForegroundColor::F_BLACK,
    ANSI::BackgroundColor::B_CYAN,
    ANSI::Attribute::BLINK
);
ANSI::Settings builder_style(
    ANSI::ForegroundColor::F_MAGENTA,
    ANSI::BackgroundColor::B_BLACK,
    ANSI::Attribute::REVERSE
);

enum class BlockType {
    Virtual,
    Sand,
    Stone
};

// Block will be a layer between the Pawn class and the actual blocks, just to differentiate them
class Block : public sista::Pawn {
protected:
    BlockType type;
public:
    Block(char symbol_, sista::Coordinates coordinates_, ANSI::Settings settings_, BlockType type_): sista::Pawn(symbol_, coordinates_, settings_), type(type_) {}
    Block(char symbol_, sista::Coordinates& coordinates_, ANSI::Settings& settings_, BlockType type_, bool _by_reference): sista::Pawn(symbol_, coordinates_, settings_), type(type_) {}
    virtual BlockType getType() {
        return type;
    }
};

// A SandBlock represents a block that falls until it finds another block or the ground
class SandBlock : public Block {
public:
    bool shadowing_virtual = false; // This will tell if the Block is over a VirtualBlock

    SandBlock(sista::Coordinates coordinates_) : Block('#', coordinates_, sand_style, BlockType::Sand) {}
    SandBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block('#', coordinates_, sand_style, BlockType::Sand, true) {}
    virtual BlockType getType() {
        return BlockType::Sand;
    }
};
// A StoneBlock represents a block that cannot fall once it's been placed
class StoneBlock : public Block {
public:
    bool shadowing_virtual = false; // This will tell if the Block is over a VirtualBlock

    StoneBlock(sista::Coordinates coordinates_) : Block('#', coordinates_, stone_style, BlockType::Stone) {}
    StoneBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block('#', coordinates_, stone_style, BlockType::Stone, true) {}
    virtual BlockType getType() {
        return BlockType::Stone;
    }
};
// A VirtualBlock represents the place where the user will have to collocate the blocks
class VirtualBlock : public Block {
public:
    VirtualBlock(sista::Coordinates coordinates_) : Block((char)('0'+coordinates_.x), coordinates_, virtual_style, BlockType::Virtual) {}
    VirtualBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block((char)('0'+coordinates_.x), coordinates_, virtual_style, BlockType::Virtual, true) {}
    virtual BlockType getType() {
        return BlockType::Virtual;
    }
};

// This class will be used to create the builder, that one which unhooks the blocks
class Builder : public sista::Pawn {
public:
    Builder(sista::Coordinates coordinates_) : Pawn('$', coordinates_, builder_style) {}
    Builder(sista::Coordinates& coordinates_, bool _by_reference) : Pawn('$', coordinates_, builder_style) {}

    void unhook();
};

// This namespace will contain all that ugly global variables
namespace game {
    Builder* builder; // Global variable which will be used as a pointer to the builder
    sista::Field* field; // Global variable which will be used as a pointer to the field
    short int score = 0; // Global variable which will be used to store the score
    bool stone_enabled = true; // This will tell if there's no more stone falling, so you can unhook another
    short int frame_countdown = 0; // This will tell when the builder will be able to unhook another block
    BlockType hooked_block = BlockType::Sand; // This will tell which block the user has selected
    StoneBlock* stone_falling = nullptr; // This will point to the stone which is currently falling
    std::vector<SandBlock*> sand_falling; // This will contain all the sand blocks which are currently falling
    std::vector<sista::Coordinates*> targets; // This will contain all the coordinates of the virtual blocks
}

// This function will be called when the builder will unhook a block
void Builder::unhook() {
    // As the first thing, we must check if the frame_countdown is 0 or less, otherwise we can't unhook
    if (game::frame_countdown <= 0) {
        sista::Coordinates coordinates = this->getCoordinates(); // We get the coordinates of the builder
        coordinates.y++; // We move the coordinates one block down
        if (game::hooked_block == BlockType::Sand) { // If we have to unhook Sand...
            game::sand_falling.push_back(new SandBlock(coordinates)); // We create a new SandBlock
            game::field->addPrintPawn(game::sand_falling.back()); // We add the SandBlock to the field
        } else if (game::hooked_block == BlockType::Stone) { // If we have to unhook Stone...
            if (game::stone_enabled) { // If we can unhook Stone...
                game::stone_falling = new StoneBlock(coordinates); // We create a new StoneBlock
                game::field->addPrintPawn(game::stone_falling); // We add the StoneBlock to the field
                game::stone_enabled = false; // We disable the stone unhooking
            } else {
                // If we can't unhook Stone, we just return
                return;
            }
        }
        game::score--; // We decrement the score
        game::frame_countdown = COOLDOWN; // We reset the frame_countdown
    }
}

// This function will move all the sand blocks which are currently falling
void moveAllSandBlocks() {
    std::queue<SandBlock*> to_remove; // This will contain all the sand blocks which have reached the ground
    for (SandBlock*& sand_block : game::sand_falling) {
        try {
            game::field->movePawnBy(sand_block, one_down);
            if (sand_block->shadowing_virtual) {
                // If the sand block is shadowing a VirtualBlock
                sista::Coordinates coordinates = sand_block->getCoordinates();
                coordinates.y--; // We move the coordinates one block up
                game::field->addPrintPawn(new VirtualBlock(coordinates)); // We add a new VirtualBlock to the field
                sand_block->shadowing_virtual = false; // We disable the shadowing
            }
        } catch (std::out_of_range& e) {
            // If the sand block has reached the ground, we add it to the queue
            to_remove.push(sand_block);
        } catch (std::invalid_argument& e) {
            // In this case we actually have to check if the sand block has reached another concrete Block or if it has reached a VirtualBlock
            sista::Coordinates coordinates = sand_block->getCoordinates();
            coordinates.y++;
            BlockType type = ((Block*)game::field->getPawn(coordinates))->getType();
            if (type == BlockType::Sand || type == BlockType::Stone) {
                // If the sand block has reached a SandBlock or a StoneBlock, we add it to the queue
                to_remove.push(sand_block);
            } else if (type == BlockType::Virtual) {
                // If the sand block has reached a VirtualBlock... well, here it comes the hard part...
                if (sand_block->shadowing_virtual) {
                    game::field->removePawn(sand_block); // We remove the sand block from the field
                    game::field->movePawnFromTo(coordinates.y, coordinates.x, coordinates.y - 1, coordinates.x); // We move the VirtualBlock one block up (smart swap)
                    game::field->addPrintPawn(game::field->getPawn(coordinates.y - 1, coordinates.x)); // We add the VirtualBlock to the field
                    sand_block->setCoordinates(coordinates); // We set the coordinates of the sand block to the new ones
                    game::field->addPrintPawn(sand_block); // We add the sand block to the field
                } else {
                    sand_block->shadowing_virtual = true; // We set the shadowing_virtual to true
                    // Now we remove the VirtualBlock from the field and we replace it with the SandBlock
                    game::field->removePawn(game::field->getPawn(coordinates));
                    game::field->movePawnBy(sand_block, one_down);
                    // game::field->addPrintPawn(sand_block);
                }
            }
        }
    }
    // Now we remove all the sand blocks which have reached the ground
    while (!to_remove.empty()) {
        SandBlock* block = to_remove.front();
        game::sand_falling.erase(std::find(game::sand_falling.begin(), game::sand_falling.end(), block));
        to_remove.pop();
    }
}
void moveStoneBlock() {
    if (game::stone_falling == nullptr) {
        // If there is no stone block falling, we just return
        return;
    }
    try {
        game::field->movePawnBy(game::stone_falling, one_down);
        if (game::stone_falling->shadowing_virtual) {
            // If the stone block is shadowing a VirtualBlock
            sista::Coordinates coordinates = game::stone_falling->getCoordinates();
            coordinates.y--; // We move the coordinates one block up
            game::field->addPrintPawn(new VirtualBlock(coordinates)); // We add a new VirtualBlock to the field
            game::stone_falling->shadowing_virtual = false; // We disable the shadowing
        }
    } catch (std::out_of_range& e) {
        // If the stone block has reached the ground, it's no more falling
        game::stone_falling = nullptr;
        game::stone_enabled = true;
    } catch (std::invalid_argument& e) {
        // In this case we actually have to check if the stone block has reached another concrete Block or if it has reached a VirtualBlock
        sista::Coordinates coordinates = game::stone_falling->getCoordinates();
        coordinates.y++;
        BlockType type = ((Block*)game::field->getPawn(coordinates))->getType();
        if (type == BlockType::Sand || type == BlockType::Stone) {
            // If the stone block has reached a SandBlock or a StoneBlock, we add it to the queue
            game::stone_falling = nullptr;
            game::stone_enabled = true;
        } else if (type == BlockType::Virtual) {
            // If the stone block has reached a VirtualBlock... well, here it comes the hard part...
            if (game::stone_falling->shadowing_virtual) {
                game::field->removePawn(game::stone_falling); // We remove the stone block from the field
                game::field->movePawnFromTo(coordinates.y, coordinates.x, coordinates.y - 1, coordinates.x); // We move the VirtualBlock one block up (smart swap)
                game::field->addPrintPawn(game::field->getPawn(coordinates.y - 1, coordinates.x)); // We add the VirtualBlock to the field
                game::stone_falling->setCoordinates(coordinates); // We set the coordinates of the stone block to the new ones
                game::field->addPrintPawn(game::stone_falling); // We add the stone block to the field
            } else {
                game::stone_falling->shadowing_virtual = true; // We set the shadowing_virtual to true
                // Now we remove the VirtualBlock from the field and we replace it with the SandBlock
                game::field->removePawn(game::field->getPawn(coordinates));
                game::field->movePawnBy(game::stone_falling, one_down);
                // game::field->addPrintPawn(game::stone_falling);
            }
        }
    }
}
void stopStoneFalling() {
    game::stone_falling = nullptr;
    game::stone_enabled = true;
}

void fillFromLevelFile(std::string path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Error while opening the file " << path << std::endl;
        #if defined(_WIN32) or defined(__linux__)
            getch();
        #elif __APPLE__
            getchar();
        #endif
        exit(1);
    }
    unsigned int y, x;
    while (file >> y >> x) {
        game::targets.push_back(new sista::Coordinates(y, x));
        game::field->addPawn(new VirtualBlock(*game::targets.back()));
    }
    file.close();
    game::score = game::targets.size() * 3;
}

bool victory() {
    for (sista::Coordinates* coordinates : game::targets) {
        if (game::field->getPawn(*coordinates) == nullptr)
            continue;
        if (((Block*)game::field->getPawn(*coordinates))->getType() == BlockType::Virtual)
            return false;
    }
    return true;
}

inline void changeBlockType() {
    game::hooked_block = game::hooked_block == BlockType::Sand ? BlockType::Stone : BlockType::Sand;
}

int main(int argc, char* argv[]) {
    #ifdef _WIN32
        CONSOLE_FONT_INFOEX font_info;
        font_info.cbSize = sizeof(font_info);
        font_info.dwFontSize.X = 31;
        font_info.dwFontSize.Y = 11;
        font_info.FontFamily = FF_DONTCARE;
        font_info.FontWeight = FW_NORMAL;
        SetCurrentConsoleFontEx(
            GetStdHandle(STD_OUTPUT_HANDLE),
            false, &font_info
        );
    #endif
    #ifdef __APPLE__
        term_echooff();
    #endif
    sista::Cursor cursor;
    sista::Field field_(WIDTH, HEIGHT); // [normally the scheme is [y][x], this is an exception in Sista]
    game::field = &field_;
    game::builder = new Builder(sista::Coordinates(1, 5));
    field_.addPawn(game::builder);
    field_.print('&');
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    sista::clearScreen();
    std::string path = "levels/";
    path += argc > 1 ? argv[1] : "1";
    path += ".level";
    fillFromLevelFile(path);
    field_.print('&');
    ANSI::Settings(
        ANSI::ForegroundColor::F_WHITE,
        ANSI::BackgroundColor::B_BLACK,
        ANSI::Attribute::REVERSE
    ).apply();
    cursor.set(2, 2);
    std::cout << "0123456789" << std::flush;
    cursor.set(2 + HEIGHT + 1, 2);
    std::cout << "0123456789" << std::flush;
    ANSI::reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    bool finished = false;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    while (!victory() && !finished) {
        std::future<int> future = std::async(std::launch::async, []() {
            #ifdef _WIN32
                return getch();
            #elif __APPLE__
                return getchar();
            #elif __linux__
                return (int)getch();
            #endif
        });
        while (future.wait_for(std::chrono::milliseconds(300)) != std::future_status::ready) {
            game::frame_countdown--;
            moveAllSandBlocks();
            moveStoneBlock(); // There's only one stone block, so we don't need to iterate through a vector

            description_style.apply();
            cursor.set(6, 15);
            std::cout << "Time: " << std::chrono::duration_cast<std::chrono::duration<int, std::milli>>(std::chrono::steady_clock::now() - start).count() << "ms      ";
            cursor.set(8, 15);
            std::cout << "Score: " << game::score << "      ";
            cursor.set(10, 15);
            std::cout << "Targets: " << game::targets.size() << "      ";
            cursor.set(12, 15);
            std::cout << "Cooldown: " << std::max(game::frame_countdown, (short)0) << "      ";
            cursor.set(14, 15);
            std::cout << "Selected: " << (game::hooked_block == BlockType::Sand ? "Sand" : "Stone") << "      ";
            #if __linux__ // Ubuntu 22.04 has a low refresh rate...
                std::cout << std::flush;
            #endif
        }
        if (victory())
            break;

        char input = future.get();
        switch (input) {
            case 'w': case 'W':
                changeBlockType();
                break;
            case 'a': case 'A':
                field_.movePawnBy(game::builder, 0, -1, PACMAN_EFFECT);
                break;
            case 'd': case 'D':
                field_.movePawnBy(game::builder, 0, 1, PACMAN_EFFECT);
                break;
            case 's': case 'S':
                game::builder->unhook();
                break;
            case ' ':
                stopStoneFalling();
                break;
            case 'q': case 'Q':
                finished = true;
        }
    }
    #ifdef __APPLE__
        // noecho.c_lflag &= ~ECHO;, noecho.c_lflag |= ECHO;
        tcsetattr(0, TCSAFLUSH, &orig_termios);
    #endif
    stone_style.apply();
    cursor.set(HEIGHT + 4, 0);
    if (finished) {
        std::cout << "Game terminated by the user." << std::endl;
    } else {
        std::cout << "You won with " << game::score << " points!" << std::endl;
    }
    #if defined(_WIN32) or defined(__linux__)
        getch();
    #elif __APPLE__
        getchar();
    #endif
    return 0;
}