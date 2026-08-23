#include "app/App.h"
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


namespace match3 {

namespace {

std::optional<Tile> tile_from_string(const std::string& value)
{
    if (value == "Red") return Tile::Red;
    if (value == "Green") return Tile::Green;
    if (value == "Blue") return Tile::Blue;
    if (value == "Yellow") return Tile::Yellow;
    if (value == "Purple") return Tile::Purple;
    if (value == "Orange") return Tile::Orange;
    if (value == "Cyan") return Tile::Cyan;
    if (value == "GrayLight") return Tile::GrayLight;
    if (value == "GrayMedium") return Tile::GrayMedium;
    if (value == "GrayDark") return Tile::GrayDark;

    return std::nullopt;
}

}// namespace







App::App(int screenWidth, int screenHeight)
    : screenWidth{ screenWidth },
    screenHeight{ screenHeight }
{

    create_level_filenames_vec();
    std::cout << "App created " << std::endl;
}

void App::create_level_filenames_vec() {
    filenames.clear();

    for (std::size_t lvl = 1; lvl <= max_levels_amount; ++lvl) {

        std::string lvl_str;

        if (lvl < 10) {
            lvl_str = "0" + std::to_string(lvl);
        }
        else {
            lvl_str = std::to_string(lvl);
        }

        std::string path = MATCH3_ASSETS_DIR;
        path += "/levels/level_";
        path += lvl_str;
        path += ".json";

        // Добавляем только реально существующие уровни
        if (std::filesystem::exists(path)) {
            filenames.push_back(path);
        }
    }

    level_write_times_.resize(filenames.size());
    levels_data.resize(filenames.size());
}


void App::tick() {
	switch (state_) {
	case AppState::LevelSelect: {
		level_select_tick();
		break;
	}
		
	case AppState::Playing: {
		game_->tick();
        if (game_->requested_action() == match3::GameAction::ExitToMenu) {
            state_ = AppState::LevelSelect;
            game_.reset();
            return;
            
        }
		break;
	}
	}
	
}

void App::level_select_tick() {
    //загружает уровни если обновились
	try_load_levels();

    update_level_select();


    //отрисовка
	draw_level_select();

}

void App::try_load_levels() {
#ifdef __EMSCRIPTEN__
    // Browser builds use Emscripten's preloaded virtual filesystem.
    // There is no useful file hot reload there, and filesystem timestamps
    // are not reliable/required. Load the packaged level configs once.
    static bool levels_loaded = false;
    if (levels_loaded) {
        return;
    }

    for (std::size_t i = 0; i < filenames.size(); ++i) {
        levels_data[i] = load_level_cfg_file(filenames[i]);
    }

    levels_loaded = true;
#else
	// Desktop: check timestamps so edited JSON level files can be hot-reloaded.
	for (std::size_t i = 0; i < filenames.size(); ++i) {
		auto time = std::filesystem::last_write_time(filenames[i]);
		if (time != level_write_times_[i]) {
			levels_data[i] = load_level_cfg_file(filenames[i]);
        }
	}
#endif
}

void App::update_level_select() {
    //обработка нажатий мыши
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return;
    }

    Vector2 mouse = GetMousePosition();

    for (std::size_t i = 0; i < levels_data.size(); ++i) {
        Rectangle button = get_level_button_rect(i);

        if (CheckCollisionPointRec(mouse, button)) {
            start_level(i);
            return;
        }
    }
}

void App::start_level(std::size_t i) {


    game_ = std::make_unique<Game>(levels_data[i], screenWidth, screenHeight);
    state_ = AppState::Playing;
}

void App::draw_level_select() const
{
    BeginDrawing();

    ClearBackground(Color{ 24, 27, 36, 255 });

    constexpr int title_font_size = 40;
    const char* title = "Select level";

    const int title_width =
        MeasureText(title, title_font_size);

    DrawText(
        title,
        (screenWidth - title_width) / 2,
        45,
        title_font_size,
        RAYWHITE
    );

    constexpr int columns = 4;

    constexpr int button_width = 150;
    constexpr int button_height = 90;

    constexpr int gap_x = 20;
    constexpr int gap_y = 20;

    constexpr int start_y = 130;

    const int grid_width =
        columns * button_width +
        (columns - 1) * gap_x;

    const int start_x =
        (screenWidth - grid_width) / 2;

    const Vector2 mouse = GetMousePosition();

    for (std::size_t i = 0; i < levels_data.size(); ++i) {
        const Rectangle button = get_level_button_rect(i);

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

        DrawRectangleRoundedLines(
            button,
            0.2f,
            8,
            RAYWHITE
        );

        const std::string level_text =
            "Level " + std::to_string(i + 1);

        constexpr int font_size = 24;

        const int text_width =
            MeasureText(level_text.c_str(), font_size);

        DrawText(
            level_text.c_str(),
            static_cast<int>(
                button.x +
                (button.width - text_width) / 2.0f
                ),
            static_cast<int>(
                button.y +
                (button.height - font_size) / 2.0f
                ),
            font_size,
            RAYWHITE
        );
    }

    EndDrawing();
}

Rectangle App::get_level_button_rect(std::size_t i) const
{
    constexpr int columns = 4;

    constexpr int button_width = 150;
    constexpr int button_height = 90;

    constexpr int gap_x = 20;
    constexpr int gap_y = 20;

    constexpr int start_y = 130;

    const int grid_width =
        columns * button_width +
        (columns - 1) * gap_x;

    const int start_x =
        (screenWidth - grid_width) / 2;

    const int row =
        static_cast<int>(i) / columns;

    const int column =
        static_cast<int>(i) % columns;

    const int x =
        start_x +
        column * (button_width + gap_x);

    const int y =
        start_y +
        row * (button_height + gap_y);

    return Rectangle{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(button_width),
        static_cast<float>(button_height)
    };
}

LevelConfig App::load_level_cfg_file(std::string path) {
    LevelConfig level;
    level.name = "error";
    level.id = 0;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file: " << path << std::endl;
        return level;
    }

    try {
        json data;
        file >> data;

        std::cout << "Loading level: " << path
                  << ", root JSON type: " << data.type_name() << std::endl;

        // Be tolerant of accidentally double-encoded JSON files.
        if (data.is_string()) {
            std::cout << "Level JSON is encoded as string, parsing inner JSON: "
                      << path << std::endl;
            data = json::parse(data.get<std::string>());
        }

        if (!data.is_object()) {
            std::cerr << "Invalid level JSON root in " << path
                      << ": expected object, got " << data.type_name() << std::endl;
            return level;
        }

        if (!data.contains("id") || !data.contains("name") ||
            !data.contains("width") || !data.contains("height") ||
            !data.contains("colors") || !data.contains("moves") ||
            !data.contains("goals")) {
            std::cerr << "Level JSON is missing required fields: " << path << std::endl;
            return level;
        }

        if (!data["goals"].is_array()) {
            std::cerr << "Invalid goals in " << path
                      << ": expected array, got " << data["goals"].type_name() << std::endl;
            return level;
        }

        level.id = data["id"].get<int>();
        level.name = data["name"].get<std::string>();
        level.width = data["width"].get<int>();
        level.height = data["height"].get<int>();
        level.colors_count = data["colors"].get<int>();
        level.moves = data["moves"].get<int>();
        //--------------------------

        if (data.contains("map")) {
            const auto& map_data = data["map"];

            if (!map_data.is_array()) {
                throw std::runtime_error("Level map must be an array");
            }

            if (map_data.size() != static_cast<std::size_t>(level.height)) {
                throw std::runtime_error(
                    "Level map height does not match level height"
                );
            }

            level.map.resize(level.height);

            for (int y = 0; y < level.height; ++y) {
                const auto& row = map_data[y];

                if (!row.is_array()) {
                    throw std::runtime_error(
                        "Level map row must be an array"
                    );
                }

                if (row.size() != static_cast<std::size_t>(level.width)) {
                    throw std::runtime_error(
                        "Level map width does not match level width"
                    );
                }

                level.map[y].resize(level.width);

                for (int x = 0; x < level.width; ++x) {
                    const std::string value =
                        row[x].get<std::string>();

                    if (value == "Random") {
                        level.map[y][x] = std::nullopt;
                        continue;
                    }

                    const auto tile =
                        tile_from_string(value);

                    if (!tile) {
                        throw std::runtime_error(
                            "Unknown tile in level map: " + value
                        );
                    }

                    level.map[y][x] = tile.value();
                }
            }
        }



        //---------------------------

        for (const auto& goal_data : data["goals"]) {
            if (!goal_data.is_object()) {
                std::cerr << "Invalid goal entry in " << path
                          << ": expected object, got " << goal_data.type_name() << std::endl;
                continue;
            }

            if (!goal_data.contains("color") || !goal_data.contains("amount")) {
                std::cerr << "Goal entry is missing color or amount in " << path << std::endl;
                continue;
            }

            LevelGoal goal;

            const std::string color =
                goal_data["color"].get<std::string>();

            goal.amount =
                goal_data["amount"].get<int>();

            const std::optional<Tile> tile =
                tile_from_string(color);

            if (!tile) {
                std::cerr
                    << "Unknown goal color '"
                    << color
                    << "' in "
                    << path
                    << std::endl;

                continue;
            }

            goal.color = tile.value();

            level.goals.push_back(goal);
        }

#ifndef __EMSCRIPTEN__
        // Desktop only: remember the timestamp for hot reload.
        auto time = std::filesystem::last_write_time(path);
        level_write_times_[level.id - 1] = time;
#endif
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse level " << path << ": " << e.what() << std::endl;
        level.name = "error";
        level.id = 0;
    }

    return level;
}

} // namespace match3
