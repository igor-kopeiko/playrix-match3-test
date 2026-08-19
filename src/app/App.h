#pragma once
#include "app/Game.h"

namespace match3 {

enum class AppState {
	LevelSelect,
	Playing
};




class App {
public:
	App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y);

	void tick();
	
	//void load_congigs();
	//void level_select_tick();

	
private:
	std::unique_ptr<Game> game_;
	AppState state_ = AppState::LevelSelect;





};


} // namespace match3
