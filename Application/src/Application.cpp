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
#include <iomanip>
#include "Block.h"

using namespace std::literals::chrono_literals;

static std::array<std::array<Block, x>, y> blocks;
static bool control = false;
static bool bar = false;
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
	std::wcout << std::fixed << std::setprecision(2) << contents;
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
	//Block::timer = new Timer;
}

void Swap(std::vector<COORD>& coords, int index1, int index2) {
	COORD prevIndex1 = coords[index1];
	coords[index1] = coords[index2];
	coords[index2] = prevIndex1;
}

std::vector<COORD> GetImportantBlocks() {
	std::vector<COORD> coords;
	Iterate(blocks, [&coords](Block& block, int Y, int X) {
		if (blocks[Y][X].GetBlockType() == BlockType::Head || blocks[Y][X].GetBlockType() == BlockType::Tail || blocks[Y][X].GetBlockType() == BlockType::Food || blocks[Y][X].GetBlockType() == BlockType::FoodSpecial)
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


	while (control)
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
	}

	return;
}

void Bar() {
	int timerPercent = 0;

	while (bar)
	{
		if (Block::timer != NULL)
			timerPercent = round(((Block::timer->Stop() + 1) / 2000.0f) * 100.0f);

		for (int i = 0; i < 100; i += 5)
		{
			if (Block::timer == NULL) {
				WriteAt("                      ", { 15, y });
				break;
			}
			else if (timerPercent == 0)
			{
				WriteAt("[                    ]", { 15, y });
				break;
			}

			if (timerPercent <= i) {
				WriteAt("[", { 15, y });

				for (int j = i; j > 0; j -= 5)
				{
					WriteAt('=', { 15 + (SHORT)j / 5, y });
					if(j / 5 == 34 - 15)
						WriteAt('=', { 35, y });
				}

				for (int k = i; k < 10; k += 5)
				{
					WriteAt(' ', { 16 + (SHORT)k / 5, y });
				}

				WriteAt("]", { 36, y });

				break;
			}
		}
		Sleep(70);
	}
	WriteAt("                      ", { 15, y });
}

void Tick() {
	auto start = std::chrono::high_resolution_clock::now();
	///////////////////////////////////////////////////////

	std::vector<COORD> coords = GetImportantBlocks();

	for (COORD c : coords)
		blocks[c.Y][c.X].Move(blocks, c);

	coords = GetImportantBlocks();

	srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	bool foodIs;

	for (COORD c : coords) {
		if (blocks[c.Y][c.X].GetBlockType() == BlockType::Food || blocks[c.Y][c.X].GetBlockType() == BlockType::FoodSpecial) {
			foodIs = true;
			break;
		}
		else foodIs = false;
	}
	while (true) {
		Block& b = blocks[rand() % y][rand() % x];
		if (b.GetBlockType() == BlockType::Floor && !foodIs) {
			if ((rand() % 20) == 0) {
				b.SetAbsoluteType(BlockType::FoodSpecial);
				Block::timer = new Timer;
			}
			else
				b.SetAbsoluteType(BlockType::Food);
			break;
		}
		else if (foodIs)
			break;

	}


	FillConsoleAt(blocks, { 0, 0 });

	int n = Block::score;
	int count = 0;

	while (n != 0) {
		n = n / 10;
		count++;
	}



	for (int i = 0; i < 7 - count; i++)
	{
		WriteAt('0', {(SHORT)i, y});
	}
	
	WriteAt(Block::score, { (SHORT)(7 - count), y });
	
	bar = true;
	std::thread work(Bar);

	/*int timerPercent = 0;

	if (Block::timer != NULL)
		timerPercent = round(((Block::timer->Stop() + 1) / 2000.0f) * 100.0f);

	for (int i = 0; i < 100; i += 5)
	{
		if (Block::timer == NULL) {
			break;
		}
		else if (timerPercent == 0)
		{
			WriteAt("[                    ]", { 15, y });
			break;
		}

		if (timerPercent <= i) {
			WriteAt("[", { 15, y });

			for (int j = i; j > 0; j -= 5)
			{
				WriteAt('=', { 15 + (SHORT)j / 5, y });
			}

			for (int k = i; k < 10; k += 5)
			{
				WriteAt(' ', { 16 + (SHORT)k / 5, y });
			}

			WriteAt("]", { 36, y });

			break;
		}
	}*/

	control = true;
	std::thread worker(Controls);
	

	///////////////////////////////////////////////////////
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> duration = end - start;
	Sleep(300.0f - (duration.count() * 1000.0f));

	control = false;
	worker.join();

	bar = false;
	work.join();

}

int main() {
	_setmode(_fileno(stdout), _O_U8TEXT);

	CONSOLE_SCREEN_BUFFER_INFO conInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);

	DWORD prev_mode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &prev_mode);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), prev_mode & ~ENABLE_ECHO_INPUT);

	SMALL_RECT windowSize = { 0, 0, x, y + 1 };
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), true, &windowSize);


	FillArray();

	//std::thread work(Bar);
	//std::thread worker(Controls);

	FillConsoleAt(blocks, { 0, 0 });

	while (true)
		Tick();

	std::cin.get();
	return 0;
}