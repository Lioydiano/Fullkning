# Fullkning

## Download

Download the latest version from [here](https://github.com/Lioydiano/Fullkning/releases).

## Compilation

After unzipping the source code, you can compile it with the following commands...

```batch
cd Fullkning
g++ fullkning.cpp -o fullkning.exe -std=c++17
```

## Usage

### Windows Usage

```batch
fullkning.exe <level-number>
```

### Others Usage

```bash
./fullkning <level-number>
```

## Create your own level

### Manually

You can create your own level by creating a `<level-number>.level` file with the following format...

```txt
{y-coordinate} {x-coordinate}
{y-coordinate} {x-coordinate}
```

For each coordinate you will have a `VirtualBlock` which is your cyan `O` target.

```txt
13 1
13 8

15 2
15 3
15 6
15 7

16 2
16 3
16 6
16 7

17 4
17 5

18 3
18 6

19 2
19 7
```

For example this is the level 4.

### With level editor

#### Windows

Since `v0.2.0`, you can create your own level with the level editor by running the following command...

```batch
levelmaker.exe <level-name>
```

...you will see a window like this...

```batch
&&&&&&&&&&&&
&          &
&     $    &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&          &
&&&&&&&&&&&&
Use {W, A, S, D}+ENTER to move the cursor
Use {P, R}+ENTER to place and remove a block
Use {Q}+ENTER to quit
```

...and you can create your own level by moving the cursor with `W`, `A`, `S`, `D` and placing or removing a block with `P` and `R`.

ℹ️ - The blocks are placed under the `$` character if possible

Then you can save your level by pressing `Q` and the level will be saved in the `levels` folder.

#### Others

You can create your own level with the level editor by running the following command...

```bash
g++ levelmaker.cpp -o levelmaker -std=c++17
./levelmaker <level-name>
```

...you will see a window like the one above, now you can create your own level by moving the cursor with `W`, `A`, `S`, `D` and placing or removing a block with `P` and `R`.

ℹ️ - The blocks are placed under the `$` character if possible
⚠️ - Due to problems with raw input, you will have to press `ENTER` after each key press, since `v0.5` this is no longer the case for Linux users.

Then you can save your level by pressing `Q` and the level will be saved in the `levels` folder.
