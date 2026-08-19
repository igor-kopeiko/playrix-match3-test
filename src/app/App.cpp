#include "app/App.h"



namespace match3 {
App::App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y) :
	game_{ kBoardOffsetX, kBoardOffsetY, kCellSize, space_near_cell, cell_amount_x, cell_amount_y } {

}

void App::tick() {
	game_.tick();
}








} // namespace match3