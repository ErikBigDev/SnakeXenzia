#include "Block.h"

Direction Block::direction = Direction::Left;
Direction Block::prevDirection = Direction::Left;
int Block::maxSize = 2;
bool Block::maxSizeChanged = false;
int Block::score = 0;
Timer* Block::timer = NULL;

Block::Block(BlockType bt, bool b, int tn) : changed(b), tailNumber(tn = 0), blockType(bt), wallType(WallType::None)
{
	
}

Block::Block(BlockType bt, WallType wt, bool b, int tn) : changed(b), tailNumber(tn = 0), blockType(bt), wallType(wt)
{

}

Block::Block() : changed(false), tailNumber(0), blockType(BlockType::Floor), wallType(WallType::None) {}

void Block::SetDirection(Direction d)
{
	prevDirection = direction;
	direction = d;
}

std::pair<Direction, Direction> Block::GetDirection()
{
	return {direction, prevDirection};
}

void Block::SetAbsoluteType(_In_ BlockType bt, _In_opt_ int tn, _In_opt_ WallType wt)
{
	if (bt == BlockType::Wall && wt == WallType::None)
	{
		throw std::invalid_argument("WallType must be specified when \"BlockType\" is \"Wall\"");
	}

	if ((bt == BlockType::Head || bt == BlockType::Tail) && wt != WallType::None && tn == -1)
	{
		throw std::invalid_argument("WallType must not be specified and TailNumber must be specified  when \"BlockType\" is \"Head\" or \"Tail\"");
	}

	blockType = bt;
	wallType = wt;
	tailNumber = tn;

	changed = true;
}

wchar_t Block::GetAbsoluteType()
{
	changed = false;
	return (wchar_t)(blockType == BlockType::Wall ? wallType : blockType);
}

BlockType Block::GetBlockType()
{
	return blockType;
}

WallType Block::GetWallType()
{
	return wallType;
}

int Block::GetTailNumber() {
	return tailNumber;
}

void Block::Move(std::array<std::array<Block, x>, y>& blocks, COORD coords) {

	if (GetAsyncKeyState(VK_UP) == -32768 && Block::GetDirection().first != VK_DOWN && Block::GetDirection().first != VK_UP) {
		Block::SetDirection(Direction::Up);
	}
	else if (GetAsyncKeyState(VK_DOWN) == -32768 && Block::GetDirection().first != VK_UP && Block::GetDirection().first != VK_DOWN) {
		Block::SetDirection(Direction::Down);
	}
	else if (GetAsyncKeyState(VK_LEFT) == -32768 && Block::GetDirection().first != VK_RIGHT && Block::GetDirection().first != VK_LEFT) {
		Block::SetDirection(Direction::Left);
	}
	else if (GetAsyncKeyState(VK_RIGHT) == -32768 && Block::GetDirection().first != VK_LEFT && Block::GetDirection().first != VK_RIGHT) {
		Block::SetDirection(Direction::Right);
	}

	if (blockType == BlockType::FoodSpecial && (timer->Stop()) >= 2000) {
		SetAbsoluteType(BlockType::Floor);
		delete timer;
		timer = NULL;
		return;
	}

	if (Directify(blocks, coords).blockType == BlockType::Tail && blockType == BlockType::Head) {	
		direction = prevDirection;
		//return;
	}

	if (maxSizeChanged && tailNumber > 1)
	{
		return;
	}
	else if(maxSizeChanged)
	{
		maxSizeChanged = false;
		return;
	}

	if ((Directify(blocks, coords).blockType == BlockType::Food || Directify(blocks, coords).blockType == BlockType::FoodSpecial) && blockType == BlockType::Head)
	{
		if (Directify(blocks, coords).blockType == BlockType::Food)
			score += 8;
		else if (Directify(blocks, coords).blockType == BlockType::FoodSpecial) {
			int ms = timer->Stop();
			if (ms < 2000)
				score =+ round(ms / 7.0f);
			delete timer;
			timer = NULL;
		}
		maxSizeChanged = true;
		maxSize++;
		Directify(blocks, coords).SetAbsoluteType(BlockType::Head, maxSize);
		SetAbsoluteType(BlockType::Tail, tailNumber);
		return;
	}

	if (tailNumber == maxSize)
	{
		Directify(blocks, coords).SetAbsoluteType(BlockType::Head, maxSize);
	}

	if (blockType == BlockType::Head || blockType == BlockType::Tail)
	{
		tailNumber--;

		if (blockType == BlockType::Head)
			SetAbsoluteType(BlockType::Tail, tailNumber);
	}

	if (tailNumber < 1 && !(blockType == BlockType::Food || blockType == BlockType::FoodSpecial))
	{
		SetAbsoluteType(BlockType::Floor);
	}

}

Block& Block::Directify(std::array<std::array<Block, x>, y>& blocks, COORD coords) {
	switch (Block::direction)
	{
	case Direction::Up: //return blocks[coords.Y - 1][coords.X];
		if (blocks[coords.Y - 1][coords.X].blockType == BlockType::Wall && coords.Y == 1) {
			return blocks[y - 2][coords.X];
		}
		else
			return blocks[coords.Y - 1][coords.X];

	case Direction::Down: //return blocks[coords.Y + 1][coords.X];
		if (blocks[coords.Y + 1][coords.X].blockType == BlockType::Wall && coords.Y == y - 2) {
			return blocks[1][coords.X];
		}
		else
			return blocks[coords.Y + 1][coords.X];

	case Direction::Left: //return blocks[coords.Y][coords.X - 1];
		if (blocks[coords.Y][coords.X - 1].blockType == BlockType::Wall && coords.X == 1) {
			return blocks[coords.Y][x - 2];
		}
		else
			return blocks[coords.Y][coords.X - 1];

	case Direction::Right: //return blocks[coords.Y][coords.X + 1];
		if (blocks[coords.Y][coords.X + 1].blockType == BlockType::Wall && coords.X == x - 2) {
			return blocks[coords.Y][1];
		}
		else
			return blocks[coords.Y][coords.X + 1];
	}
}

COORD Block::DirectifyCoords(COORD coords) {
	switch (Block::direction)
	{
	case Direction::Up: return { coords.X, coords.Y - 1 };
	case Direction::Down: return { coords.X, coords.Y + 1};
	case Direction::Left: return { coords.X - 1, coords.Y, };
	case Direction::Right: return { coords.X + 1, coords.Y, };
	}
}