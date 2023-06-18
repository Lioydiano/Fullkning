#include "include/sista/sista.hpp"


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
    SandBlock(sista::Coordinates coordinates_) : Block('#', coordinates_, sand_style, BlockType::Sand) {}
    SandBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block('#', coordinates_, sand_style, BlockType::Sand, true) {}
    virtual BlockType getType() {
        return BlockType::Sand;
    }
};
// A StoneBlock represents a block that cannot fall once it's been placed
class StoneBlock : public Block {
public:
    StoneBlock(sista::Coordinates coordinates_) : Block('#', coordinates_, stone_style, BlockType::Stone) {}
    StoneBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block('#', coordinates_, stone_style, BlockType::Stone, true) {}
    virtual BlockType getType() {
        return BlockType::Stone;
    }
};
// A VirtualBlock represents the place where the user will have to collocate the blocks
class VirtualBlock : public Block {
public:
    VirtualBlock(sista::Coordinates coordinates_) : Block(' ', coordinates_, virtual_style, BlockType::Virtual) {}
    VirtualBlock(sista::Coordinates& coordinates_, bool _by_reference) : Block(' ', coordinates_, virtual_style, BlockType::Virtual, true) {}
    virtual BlockType getType() {
        return BlockType::Virtual;
    }
};

// This class will be used to create the builder, that one which unhooks the blocks
class Builder : public sista::Pawn {
public:
    Builder(sista::Coordinates coordinates_) : Pawn('?', coordinates_, builder_style) {}
    Builder(sista::Coordinates& coordinates_, bool _by_reference) : Pawn('?', coordinates_, builder_style) {}

    void unhook() {}
};