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
