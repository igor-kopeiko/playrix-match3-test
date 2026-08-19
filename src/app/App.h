#pragma once
#include "app/Game.h"

namespace match3 {
class App {
public:
	App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y);


	

	void tick();
private:
	Game game_;






};


} // namespace match3
