#include "core/Board.h"

#include <stdexcept>


namespace match3 {

Board::Board(const std::size_t width, const std::size_t height, const std::uint32_t seed)
    : width_(width), height_(height), tiles_(width * height), rng_(seed) {
    if (width == 0 || height == 0) {
        throw std::invalid_argument("Board dimensions must be greater than zero");
    }

    for (auto& tile : tiles_) {
        tile = Tile::Default;//randomTile(); //Tile::Default;
    }

    create_board();
}

void Board::create_board() {
    //создаем доску без совпадений
    for (std::size_t y = 0; y < height_; y++) {
        int counter = 1;
        Tile last_tile = Tile::Default; //значение по умолчанию
        for (std::size_t x = 0; x < width_; x++) {
            while(1){
                //генерируем
                Tile new_tile = randomTile();
                //проверяем соседей слева
                if (x > 1) {
                    if (at(x - 1, y) == new_tile) { 
                        if (at(x - 2, y) == new_tile) {
                            continue;
                        }
                    }
                }
                //проверяем соседей сверху
                if (y > 1) {
                    if (at(x, y - 1) == new_tile) {
                        if (at(x, y - 2) == new_tile) {
                            continue;
                        }
                    }
                }
                set(x, y, new_tile);
                break;
            }

            
        }
    }
}

Tile Board::at(const std::size_t x, const std::size_t y) const {
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Board coordinates are out of range");
    }
    return tiles_[index(x, y)];
}

Tile Board::at(Position pos) const {
    return at(pos.x, pos.y);
}

std::size_t Board::index(const std::size_t x, const std::size_t y) const {
    return y * width_ + x;
}

Tile Board::randomTile() {
    std::uniform_int_distribution<int> distribution(1, static_cast<int>(Tile::Count) - 1);
    return static_cast<Tile>(distribution(rng_));
}

//===============================

void Board::set(std::size_t x, std::size_t y, Tile tile) {
    //выставляем цвет по клетке
    if (x >= width_ || y >= height_) {
        throw std::out_of_range("Board coordinates are out of range");
    }
    tiles_[index(x, y)] = tile;
}

void Board::set(Position pos, Tile tile) {
    //выставляем цвет по клетке
    set(pos.x, pos.y, tile);
}

bool Board::contains(std::size_t x, std::size_t y) const {
    //проверяем принадлежность
    if (x >= width_ || y >= height_) {
        return false;
    }
    return true;
}

bool Board::contains(Position pos) const {
    return contains(pos.x, pos.y);
}





bool Board::are_neighbours(Position first, Position second) const noexcept {
    //проверяем что это соседние клетки
    if (!contains(first)) {
        return false;
    }
    if (!contains(second)) {
        return false;
    }

    //по горизонтали
    if (first.x == second.x + 1 || first.x + 1 == second.x) {
        if (first.y == second.y) {
            return true;
        }
    }

    //по вертикали
    if (first.y == second.y + 1 || first.y + 1 == second.y) {
        if (first.x == second.x) {
            return true;
        }
    }

    return false;
}

bool Board::try_swap(Position first, Position second) {


    if (!are_neighbours(first, second)) {
        return false;
    }
    else {
        Tile temp = at(first);
        set(first, at(second));
        set(second, temp);
        return true;
    }
}

std::vector<Position> Board::find_matches() const {
    std::vector<std::vector<Position>> groups_of_cells_to_delete;
    
    //Сначала ищем 3 в ряд в вертикально, то есть в столбце
    for (std::size_t x = 0; x < width_; x++) {
        int counter = 1;
        Tile last_tile = Tile::Default; //значение по умолчанию
        for (std::size_t y = 0; y < height_; y++) {
            Tile tile = at(x, y);

            if (tile == last_tile) {
                counter++;
                if (counter == 3) {
                    //делаем поиск в ширину
                    groups_of_cells_to_delete.push_back(breadth_first_search({ x,y }, tile));

                }
            }
            else {
                //сброс
                last_tile = tile;
                counter = 1;
            }
        }
    }
    //====================================================
    //теперь ищем 3 в ряд в горизонтально, по строкам
    
    for (std::size_t y = 0; y < height_; y++) {
        int counter = 1;
        Tile last_tile = Tile::Default; //значение по умолчанию
        for (std::size_t x = 0; x < width_; x++) {
            Tile tile = at(x, y);
            if (tile == last_tile) {
                counter++;
                if (counter == 3) {
                    //делаем поиск в ширину
                    groups_of_cells_to_delete.push_back(breadth_first_search({ x,y }, tile));
                }
            }
            else {
                //сброс
                last_tile = tile;
                counter = 1;
            }
        }
    }
    
    //получаем уникальные клетки
    std::unordered_set<Position, PositionHash> unic_cells_to_delete;
    for (auto& group : groups_of_cells_to_delete) {
        for (auto& pos : group) {
            unic_cells_to_delete.insert(pos);
        }
    }
    
    std::vector<Position>cells_to_delete;
    //записываем уникальные клетки
    for (auto it = unic_cells_to_delete.begin(); it != unic_cells_to_delete.end(); ++it) {
        cells_to_delete.push_back(*it);
    }
    return cells_to_delete;
}

//поиск в ширину
std::vector<Position> Board::breadth_first_search(Position start, Tile wish_tile) const {
    std::unordered_set<Position, PositionHash> already_checked;
    std::vector<Position> need_to_check;
    std::vector<Position> result;
    need_to_check.push_back(start);
    
    while (!need_to_check.empty()) {
        //извлекаем
        Position current_pos = need_to_check.back();
        need_to_check.pop_back();


        Position pos_to_check = current_pos;
        pos_to_check.x += 1;
        bf_search_check_cell(already_checked, need_to_check, result, pos_to_check, wish_tile);

        if (current_pos.x > 0) {
            pos_to_check = current_pos;
            pos_to_check.x -= 1;
            bf_search_check_cell(already_checked, need_to_check, result, pos_to_check, wish_tile);
        }


        pos_to_check = current_pos;
        pos_to_check.y += 1;
        bf_search_check_cell(already_checked, need_to_check, result, pos_to_check, wish_tile);

        if (current_pos.y > 0) {
            pos_to_check = current_pos;
            pos_to_check.y -= 1;
            bf_search_check_cell(already_checked, need_to_check, result, pos_to_check, wish_tile);
        }
    }
    return result;
}

void Board::bf_search_check_cell(
    std::unordered_set<Position, PositionHash>& already_checked,
    std::vector<Position>& need_to_check,
    std::vector<Position>& result,
    Position& pos_to_check,
    Tile& wish_tile
) const {
    if (contains(pos_to_check)) {
        //проверим, вдруг проверяли
        if (already_checked.count(pos_to_check) == 0) {
            Tile tile = at(pos_to_check);
            if (tile == wish_tile) {
                result.push_back(pos_to_check);
                need_to_check.push_back(pos_to_check);
            }
            already_checked.insert(pos_to_check);
        }
    }
}

//std::vector<Position> Board::find_matches(std::string side) const {
//
//    std::size_t first_coord_max = 0;
//    std::size_t second_coord_max = 0;
//    bool is_horizontal = false;
//    if (side == "horizontal") {
// //        first_coord_max = width_;
//        second_coord_max = height_;

//
//        is_horizontal = true;
//    }
//    else if (side == "vertical") {
        //        first_coord_max = width_;
//        second_coord_max = height_;
//    }
//    else {
//        return {};
//    }
//
//    for (std::size_t first_coord = 0; first_coord < first_coord_max; first_coord++) {
//        int counter = 1;
//        Tile last_tile = Tile::Default; //значение по умолчанию
//        for (std::size_t second_coord = 0; second_coord < second_coord_max; second_coord++) {
//            Tile tile = Tile::Default;
//            if (is_horizontal) {
//                tile = at(second_coord, first_coord);
//                
//            }
//            else {
//                tile = at(first_coord, second_coord); //x, y
//            }
//            if (tile == last_tile) {
//                counter++;
//                if (counter == 3) {
//                    //делаем поиск в ширину
//                }
//            }
//            else {
//                //сброс
//                last_tile = tile;
//                counter = 1;
//            }
//        }
//    }
//}



} // namespace match3
