#include <iostream>
#include <Windows.h>
#include <array>
#include <io.h>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include "Block.h"

using namespace std::literals::chrono_literals;

class Timer {
public:
	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;
	float ms;

	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer()
	{
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;
		ms = duration.count() * 1000.0f;

		std::wcout << ms << "\n";
	}
};

static std::array<std::array<Block, x>, y> blocks;
//std::array<Block*, x*y> snake;

void Iterate(_Inout_ std::array<std::array<Block, x>, y>& arr, _In_ std::function<void(Block&, int, int)> iterate) {
	for (int Y = 0; Y < y; Y++)
	{
		for (short X = 0; X < x; X++)
		{
			iterate(arr[Y][X], Y, X);
		}
	}
}

void ClearConsole()
{
	//Get the handle to the current output buffer...
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//This is used to reset the carat/cursor to the top left.
	COORD coord = { 0, 0 };
	//A return value... indicating how many chars were written
	//   not used but we need to capture this since it will be
	//   written anyway (passing NULL causes an access violation).
	DWORD count;
	//This is a structure containing all of the console info
	// it is used here to find the size of the console.
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	//Here we will set the current color
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		//This fills the buffer with a given character (in this case 32=space).
		FillConsoleOutputCharacter(hStdOut, (TCHAR)32, csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
		FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
		//This will set our cursor position for the next print statement.
		SetConsoleCursorPosition(hStdOut, coord);
	}
	return;
}

template<typename T>
void WriteAt(T contents, COORD coords) {
	//Timer timer;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coords);
	std::wcout << contents;
	//SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});
}

void FillConsoleAt(_Inout_ std::array<std::array<Block, x>, y>& arr, _In_ COORD coords = {0, 0})	 {
	//Timer t;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	static bool initial = false;

	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coords);
	else return;
	if (!initial) {
		Iterate(blocks, [](Block& block, int Y, int X) {
			if (block.changed)
			{
				std::wcout << block.GetAbsoluteType();
				if (X == x - 1)
					std::wcout << "\n";
				//block.changed = false;
			}
			});

		initial = true;
	}
	else
		Iterate(blocks, [](Block& block, int Y, int X) {
		if(block.changed)
			WriteAt(block.GetAbsoluteType(), { (SHORT)X, (SHORT)Y });
		});
}

void FillArray() {
	for (int Y = 0; Y < y; Y++)
	{
		for (int X = 0; X < x; X++)
		{
			blocks[Y][X] = Block(BlockType::Floor, true);
			if (Y == 0 || Y == y - 1)
			{
				blocks[Y][X].SetAbsoluteType(BlockType::Wall, -1, WallType::Horizontal);
			}
			else if (X == 0 || X == x - 1)
			{
				blocks[Y][X].SetAbsoluteType(BlockType::Wall, -1, WallType::Vertical);
			}
			else
			{
				blocks[Y][X].SetAbsoluteType(BlockType::Floor, -1, WallType::None);
			}
		}
	}

	blocks[0][0].SetAbsoluteType(BlockType::Wall, -1, WallType::CornerTL);
	blocks[0][x - 1].SetAbsoluteType(BlockType::Wall, -1, WallType::CornerTR);
	blocks[y - 1][0].SetAbsoluteType(BlockType::Wall, -1, WallType::CornerBL);
	blocks[y - 1][x - 1].SetAbsoluteType(BlockType::Wall, -1, WallType::CornerBR);
	blocks[(y / 2) - 1][(x / 2) - 1].SetAbsoluteType(BlockType::Tail, 1);
	blocks[(y / 2) - 1][(x / 2) - 2].SetAbsoluteType(BlockType::Head, 2);
	blocks[(y / 2) - 1][(x / 2) - 3].SetAbsoluteType(BlockType::Food);
}

void Swap(std::vector<COORD>& coords, int index1, int index2) {
	COORD prevIndex1 = coords[index1];
	coords[index1] = coords[index2];
	coords[index2] = prevIndex1;
}

std::vector<COORD> GetImportantBlocks() {
	std::vector<COORD> coords;
	Iterate(blocks, [&coords](Block& block, int Y, int X) {
		if (blocks[Y][X].GetBlockType() == BlockType::Head || blocks[Y][X].GetBlockType() == BlockType::Tail || blocks[Y][X].GetBlockType() == BlockType::Food)
			coords.push_back({ (SHORT)X, (SHORT)Y });
		});
	
	std::sort(coords.begin(), coords.end(), [](COORD a, COORD b) {
		return (blocks[a.Y][a.X].GetTailNumber() > blocks[b.Y][b.X].GetTailNumber());
		});
	for (int i = coords.size() - 1; i > 0; i--){
		Swap(coords, i - 1, i);
		}

	return coords;
}

void Controls() {
	
	while (true)
		GetAsyncKeyState(VK_UP);
		GetAsyncKeyState(VK_DOWN);
		GetAsyncKeyState(VK_LEFT);
		GetAsyncKeyState(VK_RIGHT);

	/*while (true)
	{

		if (GetAsyncKeyState(VK_UP) == -32767 && Block::GetDirection().first != VK_DOWN && Block::GetDirection().first != VK_UP) {
			Block::SetDirection(Direction::Up);
			continue;
		}
		else if (GetAsyncKeyState(VK_DOWN) == -32767 && Block::GetDirection().first != VK_UP && Block::GetDirection().first != VK_DOWN) {
			Block::SetDirection(Direction::Down);
			continue;
		}
		else if (GetAsyncKeyState(VK_LEFT) == -32767 && Block::GetDirection().first != VK_RIGHT && Block::GetDirection().first != VK_LEFT) {
			Block::SetDirection(Direction::Left);
			continue;
		}
		else if (GetAsyncKeyState(VK_RIGHT) == -32767 && Block::GetDirection().first != VK_LEFT && Block::GetDirection().first != VK_RIGHT) {
			Block::SetDirection(Direction::Right);
			continue;
		}

	}*/
}

void Tick() {
	auto start = std::chrono::high_resolution_clock::now();
	
	{
		std::vector<COORD> coords = GetImportantBlocks();
			
		for (COORD c : coords)
			blocks[c.Y][c.X].Move(blocks, c);

		coords = GetImportantBlocks();

		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

		bool foodIs;

		FillConsoleAt(blocks, { 0, 0 });
		for (COORD c : coords) {
			if (blocks[c.Y][c.X].GetBlockType() == BlockType::Food) {
				foodIs = true;
				break;
			}
			else foodIs = false;
		}
		while (true) {
			Block& b = blocks[rand() % y][rand() % x];
			if (b.GetBlockType() == BlockType::Floor && !foodIs) {
				b.SetAbsoluteType(BlockType::Food);
				break;
			}
			else if (foodIs)
				break;
		}


		FillConsoleAt(blocks, { 0, 0 });
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
	Sleep(500.0f - (duration.count() * 1000.0f));
}

int main() {
	_setmode(_fileno(stdout), _O_U8TEXT);

	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);

	DWORD prev_mode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &prev_mode);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), prev_mode & ~ENABLE_ECHO_INPUT);

	//std::thread controls(Controls);

	FillArray();

	//FillConsoleAt(blocks, { 0, 0 });

	while (true)
		Tick();

	std::array<int, 5> arr = {1, 2, 3, 4, 5};
	std::sort(arr.begin(), arr.end(), [](int a, int b) {
		if (a == 2)
			return true;
		if (b == 2)
			return false;

		return a > b;
		});
	for (int i : arr)
		std::wcout << i << "\n";

	std::cin.get();
	return 0;
}