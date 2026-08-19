#include "app/App.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


namespace match3 {
App::App(int kBoardOffsetX, int kBoardOffsetY, int kCellSize, int space_near_cell, std::size_t cell_amount_x, std::size_t cell_amount_y) 
{
	//game_ = std::make_unique<Game>(kBoardOffsetX, kBoardOffsetY, kCellSize, space_near_cell, cell_amount_x, cell_amount_y);
    create_level_filenames_vec();
    std::cout << "App created " << std::endl;
}

void App::create_level_filenames_vec() {
	for (int lvl = 1; lvl <= levels_amount; lvl++) {
		std::string path = "assets/levels/level_";
		std::string lvl_str;
		if (lvl < 10) {
			lvl_str = "0" + std::to_string(lvl);
		}
		else {
			lvl_str = std::to_string(lvl);
		}
		path += lvl_str;
        path += ".json";

		filenames.push_back(path);

	}
}


void App::tick() {
	switch (state_) {
	case AppState::LevelSelect: {
		level_select_tick();
		break;
	}
		
	case AppState::Playing: {
		game_->tick();
		break;
	}
	}
	
}

void App::level_select_tick() {
    std::cout << "try load levels" << std::endl;
	try_load_levels();

    std::cout << "draw level select" << std::endl;
	draw_level_select();

}

void App::try_load_levels() {
	//проверяем наличие уровня
	for (int i = 0; i < filenames.size(); i++) {
        std::cout << "will get time " << filenames[i] << std::endl;
		auto time = std::filesystem::last_write_time(filenames[i]);
        std::cout << "got time " << filenames[i] << std::endl;
		if (time != level_write_times_[i]) {
			//значит надо перезаписать уровень
            std::cout << "will load " << filenames[i] << std::endl;
			levels_data[i] = load_level_cfg_file(filenames[i]);
            std::cout << "loaded " << filenames[i]  << std::endl;
        }
        else {
            std::cout << "not loaded " << filenames[i] << std::endl;
        }
	}
}

//void draw_level_select() {
//	//отрисовка меню
//	Color background = Color{ 24, 27, 36, 255 };
//}

void App::draw_level_select() const
{
    BeginDrawing();

    ClearBackground(Color{ 24, 27, 36, 255 });

    DrawText(
        "Select level",
        40,
        40,
        32,
        RAYWHITE
    );

    constexpr int columns = 5;

    constexpr int button_width = 90;
    constexpr int button_height = 70;

    constexpr int gap_x = 15;
    constexpr int gap_y = 20;

    constexpr int start_x = 40;
    constexpr int start_y = 110;

    const Vector2 mouse = GetMousePosition();

    for (std::size_t i = 0; i < levels_data.size(); ++i) {
        const int row = static_cast<int>(i) / columns;
        const int column = static_cast<int>(i) % columns;

        const int x = start_x + column * (button_width + gap_x);

        const int y = start_y + row * (button_height + gap_y);

        const Rectangle button{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(button_width),
            static_cast<float>(button_height)
        };

        const bool hovered =
            CheckCollisionPointRec(mouse, button);

        Color button_color = Color{ 55, 62, 80, 255 };

        if (hovered) {
            button_color = Color{ 75, 85, 110, 255 };
        }

        DrawRectangleRounded(
            button,
            0.2f,
            8,
            button_color
        );

        //DrawRectangleRoundedLines(
        //    button,
        //    0.2f,
        //    8,
        //    2.0f,
        //    RAYWHITE
        //);
        DrawRectangleRoundedLines(
            button,
            0.2f,
            8,
            RAYWHITE
        );

        const std::string level_text =
            "Level " + std::to_string(i + 1);

        const int text_width =
            MeasureText(level_text.c_str(), 20);

        DrawText(
            level_text.c_str(),
            x + (button_width - text_width) / 2,
            y + 12,
            20,
            RAYWHITE
        );

        //if (i < levels_data.size()) {
        //    const std::string name =
        //        levels_data[i].name;

        //    const int name_width =
        //        MeasureText(name.c_str(), 14);

        //    DrawText(
        //        name.c_str(),
        //        x + (button_width - name_width) / 2,
        //        y + 42,
        //        14,
        //        LIGHTGRAY
        //    );
        //}
    }

    EndDrawing();
}

LevelConfig App::load_level_cfg_file(std::string path) {

	std::ifstream file(path);

	LevelConfig level;

	if (!file.is_open()) {
		// ошибка
		level.name = "error";
		level.id = 0;
        return level;
	}

	json data;
	file >> data;

	level.id = data["id"].get<int>();
	level.name = data["name"].get<std::string>();
	level.width = data["width"].get<int>();
	level.height = data["height"].get<int>();
	level.colors_count = data["colors"].get<int>();
	level.moves = data["moves"].get<int>();

	//запишем время открытия
	auto time = std::filesystem::last_write_time(path);
	level_write_times_[level.id - 1] = time;

	return level;
}








} // namespace match3