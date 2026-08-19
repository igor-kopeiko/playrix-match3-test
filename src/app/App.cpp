#include "app/App.h"



namespace match3 {
App::App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y) 
{
	game_ = std::make_unique<Game>(kBoardOffsetX, kBoardOffsetY, kCellSize, space_near_cell, cell_amount_x, cell_amount_y);
}

void App::tick() {
	switch (state_) {
	case AppState::LevelSelect: {
		break;
	}
		
	case AppState::Playing: {
		game_->tick();
		break;
	}
	}
	
}

void App::level_select_tick() {
	//проверить наличие файлов с уровнями
	
	//отрисовка меню
}








} // namespace match3