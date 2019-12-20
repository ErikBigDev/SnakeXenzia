﻿#pragma once
#include <iostream>
#include <array>
#include <tuple>
#include <Windows.h>


enum BlockType : wchar_t { Head = '@', Tail = '#', Food = L'▪', FoodSpecial = L'♦', Floor = '.', Wall};
enum WallType : wchar_t { None, Vertical = L'║', Horizontal = L'═', CornerTL = L'╔', CornerTR = L'╗', CornerBL = L'╚', CornerBR = L'╝' };

enum Direction : BYTE {Up = VK_UP, Down = VK_DOWN, Left = VK_LEFT, Right = VK_RIGHT};


#define x 70
#define y 20

class Block
{
public:
	Block(BlockType bt, bool b, int tn = 0);
	Block(BlockType bt, WallType wt, bool b, int tn = 0);
	Block();
	wchar_t GetAbsoluteType();
	BlockType GetBlockType();
	WallType GetWallType();
	int GetTailNumber();
	void Move(std::array<std::array<Block, x>, y>& blocks, COORD coords);

	static void SetDirection(Direction d);
	static std::pair<Direction, Direction> GetDirection();
	void SetAbsoluteType(_In_ BlockType bt, _In_opt_ int tn = 0, _In_opt_ WallType wt = WallType::None);
	Block& Directify(std::array<std::array<Block, x>, y>& blocks, COORD coords);

	static int maxSize;
	static bool maxSizeChanged;

	bool changed;

private:
	int tailNumber;
	BlockType blockType;
	WallType wallType;

	static Direction direction;
	static Direction prevDirection;
};

