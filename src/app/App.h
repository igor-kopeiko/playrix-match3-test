#pragma once
#include "app/Game.h"
#include <filesystem>

namespace match3 {

enum class AppState {
	LevelSelect,
	Playing
};

struct LevelGoal {
	Tile color;
	int amount;
};

struct LevelConfig {
	int id;
	std::string name;

	int width;
	int height;
	int colors_count;
	int moves;

	std::vector<LevelGoal> goals;
};




class App {
public:
	App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y);



	void tick();
	
	//void load_congigs();
	void level_select_tick();

	void try_load_levels();
	

	LevelConfig load_level_cfg_file(std::string path);
	
private:
	std::unique_ptr<Game> game_;
	AppState state_ = AppState::LevelSelect;

	static constexpr std::size_t levels_amount = 10;

	std::vector<std::string> filenames;
	std::vector<std::filesystem::file_time_type> level_write_times_{ levels_amount };
	std::vector<LevelConfig> levels_data{ levels_amount };

	void create_level_filenames_vec();
	void draw_level_select() const;




};


} // namespace match3
