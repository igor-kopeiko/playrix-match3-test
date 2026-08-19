#pragma once
#include "app/Game.h"
#include <filesystem>

namespace match3 {

enum class AppState {
	LevelSelect,
	Playing
};






class App {
public:
	App(int screenWidth, int screenHeight);



	void tick();
	
	//void load_congigs();
	void level_select_tick();

	void try_load_levels();

	void update_level_select();
	

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
	Rectangle get_level_button_rect(std::size_t i) const;
	void start_level(std::size_t i);

	const int screenWidth;
	const int screenHeight;


};


} // namespace match3
