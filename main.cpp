#include <cstdio>
#include <raylib.h>
#include <vector>
#include <random>
#include <string>

int cell_size = 40;
int grid_width = 9;
int grid_height = 9;
int num_mines = 3;
int num_flags = num_mines;

struct Cell {
    int x;
    int y;
    bool is_mine;
    bool is_revealed;
    bool is_flagged;
    int num_adjacent_mines;
};

using Grid = std::vector<Cell>;

int get_index(int x, int y, int width) {
    return y * width + x;
}

bool get_is_mine() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 1);
    return dis(gen);
}

Grid initialize_grid(int width, int height, int num_mines) {
    Grid grid;
    grid.resize(width * height);
    for (int i = 0; i < width * height; i++) {
        auto& cell = grid[get_index(i % width, i / width, width)];
        cell.x = i % width;
        cell.y = i / width;
        cell.num_adjacent_mines = 0;
        if (num_mines > 0) {
            auto is_mine = get_is_mine();
            cell.is_mine = is_mine;
            cell.is_revealed = false;
            if (is_mine) {
                num_mines--;
            }
        }
    }

    return grid;
}



void draw_grid(const Grid& grid, int offset_x = 0, int offset_y = 0, int gap = 0) {
    for (const auto& cell : grid) {
        if (cell.is_revealed) {
            DrawRectangle(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, DARKGRAY);
            DrawRectangleLines(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, LIGHTGRAY);
            if (!cell.is_mine && cell.num_adjacent_mines > 0) {
                DrawText(std::to_string(cell.num_adjacent_mines).c_str(), (offset_x + cell.x * cell_size) + cell_size / 2, (offset_y + cell.y * cell_size) + cell_size / 2, 14, BLUE);
            }
        } else if (cell.is_flagged) {
            DrawRectangle(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, BLACK);
            DrawRectangleLines(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, LIGHTGRAY);
            DrawText("F", (offset_x + cell.x * cell_size) + cell_size / 2, (offset_y + cell.y * cell_size) + cell_size / 2, 24, GREEN);
        } else {
            DrawRectangle(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, BLACK);
            DrawRectangleLines(offset_x + cell.x * cell_size + gap, offset_y + cell.y * cell_size + gap, cell_size - gap, cell_size - gap, LIGHTGRAY);
        }

        if (cell.is_mine && cell.is_revealed) {
            DrawCircle(offset_x + cell.x * cell_size + cell_size / 2, offset_y + cell.y * cell_size + cell_size / 2, cell_size / 4, RED);
        }
    }
}


void print_grid(const Grid& grid) {
    printf("Grid size: %zu\n", grid.size());
    printf("Grid first element: (%d, %d), isMine: %d\n", grid[0].x, grid[0].y, grid[0].is_mine);
    for (const auto& cell : grid) {
        printf("Cell (%d, %d), isMine: %d\n", cell.x, cell.y, cell.is_mine);
    }
}


Cell & get_cell(Grid& grid, int mouse_x, int mouse_y, int offset_x = 0, int offset_y = 0) {
    int x = (mouse_x - offset_x) / cell_size;
    int y = (mouse_y - offset_y) / cell_size;

    //printf("Mouse: (%d, %d), Cell: (%d, %d)\n", mouse_x, mouse_y, x, y);
    return grid[get_index(x, y, grid_width)];
}

int get_num_adjacent_mines(const Grid& grid, int x, int y) {
    int num_adjacent_mines = 0;
    for (int i = -1; i <= 1; i++) {
        if (x + i < 0 || x + i >= grid_width) {
            continue;
        }
        for (int j = -1; j <= 1; j++) {
            if (y + j < 0 || y + j >= grid_height) {
                continue;
            }
            if (grid[get_index(x + i, y + j, grid_width)].is_mine) {
                num_adjacent_mines++;
            }
        }
    }
    return num_adjacent_mines;
}

void update_cells(Grid& grid) {
    for (auto& cell : grid) {
        cell.num_adjacent_mines = get_num_adjacent_mines(grid, cell.x, cell.y);
    }
}

void reveal_adjacent_non_bombs_cells(Grid& grid, int x, int y) {
    for (int i = -1; i <= 1; i++) {
        if (x + i < 0 || x + i >= grid_width) {
            continue;
        }
        for (int j = -1; j <= 1; j++) {
            if (y + j < 0 || y + j >= grid_height) {
                continue;
            }
            auto& cell = grid[get_index(x + i, y + j, grid_width)];
            if (!cell.is_mine && !cell.is_revealed) {
                cell.is_revealed = true;
                if (cell.num_adjacent_mines == 0) {
                    reveal_adjacent_non_bombs_cells(grid, cell.x, cell.y);
                }
            }
        }
    }
}

bool check_if_won(const Grid& grid, bool is_game_won) {
    if (is_game_won) {
        return true;
    }
    for (const auto& cell : grid) {
        if (!cell.is_revealed && !cell.is_mine && !cell.is_flagged) {
            return false;
        }
    }

    return true;
}

int calculate_cell_size(int width, int height, int grid_width, int grid_height) {
    int cell_size_x = width / grid_width;
    int cell_size_y = height / grid_height;
    return cell_size_x < cell_size_y ? cell_size_x : cell_size_y;
}

int main(int argc, char** argv) {
    int width = 450;
    int height = 450;
    bool is_game_over = false;
    bool is_game_won = false;
    int flagged_mines = 0;

    if (argc == 4) {
        grid_width = std::stoi(argv[1]);
        grid_height = std::stoi(argv[2]);
        num_mines = std::stoi(argv[3]);
        num_flags = num_mines;
    }

    if (num_mines > grid_width * grid_height) {
        num_mines = (grid_width * grid_height) - 1;
        num_flags = num_mines;
    }

    auto grid = initialize_grid(grid_width, grid_height, num_mines);
    update_cells(grid);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "Minesweeper");
    SetTargetFPS(60);

    int center_x = (width / 2) - (cell_size * grid_width / 2);
    int center_y = (height / 2) - (cell_size * grid_height / 2);

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
            cell_size = calculate_cell_size(width, height, grid_width, grid_height);
            center_x = (width / 2) - (cell_size * grid_width / 2);
            center_y = (height / 2) - (cell_size * grid_height / 2);
        }

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !is_game_over) {
            auto& cell = get_cell(grid, mouse_x, mouse_y, center_x, center_y);
            cell.is_revealed = true;
            if (cell.num_adjacent_mines == 0) {
                reveal_adjacent_non_bombs_cells(grid, cell.x, cell.y);
            }
            if (cell.is_mine && !cell.is_flagged) {
                is_game_over = true;
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !is_game_over) {
            auto& cell = get_cell(grid, mouse_x, mouse_y, center_x, center_y);
            if (num_flags > 0 && !cell.is_revealed && !cell.is_flagged) {
                cell.is_flagged = true;
                num_flags--;
                if (cell.is_mine) {
                    flagged_mines++;
                    if (flagged_mines == num_mines) {
                        is_game_won = true;
                    }
                }
            } else if (cell.is_flagged) {
                cell.is_flagged = false;
                num_flags++;
                if (cell.is_mine) {
                    flagged_mines--;
                }
            }
        }

        is_game_won = check_if_won(grid, is_game_won);

        if ((is_game_over || is_game_won) && IsKeyPressed(KEY_R)) {
            grid = initialize_grid(grid_width, grid_height, num_mines);
            update_cells(grid);
            is_game_over = false;
            is_game_won = false;
            num_flags = num_mines;
            flagged_mines = 0;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_grid(grid, center_x, center_y, 2);
        if (is_game_over) {
            DrawText("Game Over! To restart press 'R'", 10, 50, 20, PINK);
        } else if (is_game_won) {
            DrawText("You Won! To restart press 'R'", 10, 50, 20, PINK);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}