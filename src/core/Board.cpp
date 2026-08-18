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

bool Board::has_possible_moves(){
    for (std::size_t y = 0; y < height_; y++) {
        for (std::size_t x = 0; x < width_; x++) {
            Position current{ x,y };

            //горизонтально
            if (x > 0) {
                Position left{ x - 1,y };
                try_swap(current, left);
                if (has_matches_in_area_of(current, left)) {
                    try_swap(current, left);//обратно
                    //std::cout << "solution = [" << current.x << " "  << current.y <<  "] [" << left.x<< " " << left.y << "]" << std::endl;
                    return true;
                }
                else {
                    try_swap(current, left); //обратно
                }
            }
            

            //вертикально
            if (y > 0) {
                Position up{ x,y - 1};
                try_swap(current, up);
                if (has_matches_in_area_of(current, up)) {
                    try_swap(current, up); //обратно
                    //std::cout << "solution = [" << current.x << " " << current.y << "] [" << up.x << " " << up.y << "]" << std::endl;
                    return true;
                }
                else {
                    try_swap(current, up); //обратно
                }
            }
        }
    }
    //значит решений нет
    return false;
}

bool Board::has_matches_in_area_of(Position first, Position second) const {
    //метод требует, но не проверяет чтобы ячейки были рядом
    std::vector<std::size_t> vec_x;
    std::vector<std::size_t> vec_y;
    //определим какая координата совпадает
    if (first.x == second.x) {
        vec_x.push_back(first.x);
        vec_y.push_back(first.y);
        vec_y.push_back(second.y);
    }
    else { //значит совпадает y
        vec_x.push_back(first.x);
        vec_x.push_back(second.x);
        vec_y.push_back(first.y);
    }

    //ищем совпадение в районе сначала по вертикали
    for (auto& x : vec_x) {
        int counter = 1;
        Tile last_tile = Tile::Default;
        for (int y = 0; y < height_; y++) { //проходим по всей оси
            Tile curr_tile = at(x, y);
            if (curr_tile == last_tile) {
                counter++;
                if (counter == 3) {
                    return true;
                }
            }
            else {
                counter = 1;
                last_tile = curr_tile;
            }
        }
    }

    //теперь по горизонтали
    for (auto& y : vec_y) {
        int counter = 1;
        Tile last_tile = Tile::Default;
        for (int x = 0; x < width_; x++) { //проходим по всей оси
            Tile curr_tile = at(x, y);
            if (curr_tile == last_tile) {
                counter++;
                if (counter == 3) {
                    return true;
                }
            }
            else {
                counter = 1;
                last_tile = curr_tile;
            }
        }
    }
    return false; //значит ничего не нашли
}

bool Board::try_match_swap(Position first, Position second) {
    if (!try_swap(first, second)) {
        return false;
    }
    if (has_matches_in_area_of(first, second)) {
        //find_matches();
        return true;
    }
    else {
        try_swap(first, second);//обратно
        return false;
    }
}

void Board::delete_cells(std::vector<Position>& cells_to_delete) {
    for (auto& cell_pos : cells_to_delete) {
        set(cell_pos, Tile::Default);
    }
}

void Board::collapse_cells() {
    for (int x = 0; x < width_; x++) {
        std::size_t current_empty_y = 0;
        bool found_empty = false;

        for (int y = (int)height_ - 1; y >= 0; y--) {
            Tile curr_cell = at(x, y);
            if (curr_cell == Tile::Default && !found_empty) {
                current_empty_y = y;
                found_empty = true;
            }
            else {
                if (found_empty) {
                    set(x, current_empty_y, curr_cell);
                    set(x, y, Tile::Default);
                    //ищем следующую пустую клетку
                    for (int empty_y = current_empty_y; empty_y >= 0;  empty_y--) {
                        if (at(x, empty_y) == Tile::Default) {
                            current_empty_y = empty_y;
                            break;
                        }
                    }
                }
            }
        }

    }
}

void Board::fill_empty_cells() {
    for (std::size_t x = 0; x < width_; x++) {
        for (std::size_t y = 0; y < height_; y++) {
            if (at(x, y) == Tile::Default) {
                set(x, y, randomTile());
            }
        }
    }
}



} // namespace match3
