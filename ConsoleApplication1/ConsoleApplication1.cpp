#include <iostream>
#include <vector>
#include <stack>
#include <random>
#include <ctime>
#include <algorithm>

using namespace std;

// Class to represent a cell's coordinates
struct Point {
    int row, col;
    Point(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const Point& p) const {
        return row == p.row && col == p.col;
    }
};

// Class to generate and solve mazes
class MazeSolver {
private:
    vector<vector<char>> maze;
    int rows, cols;
    Point start, end;

    // Possible movements: up, right, down, left
    vector<int> dr = { -1, 0, 1, 0 };
    vector<int> dc = { 0, 1, 0, -1 };

    // To keep track of visited cells and the path
    vector<vector<bool>> visited;
    vector<vector<Point>> parent;

    // Random number generation
    mt19937 rng;

public:
    MazeSolver(int r, int c) {
        // Initialize with custom size
        rows = r;
        cols = c;

        // Initialize random number generator
        rng.seed(static_cast<unsigned int>(time(nullptr)));

        // Create empty maze filled with walls
        maze.resize(rows, vector<char>(cols, '#'));

        // Initialize arrays
        visited.resize(rows, vector<bool>(cols, false));
        parent.resize(rows, vector<Point>(cols));
    }

    MazeSolver(const vector<vector<char>>& m, Point s, Point e) : maze(m), start(s), end(e) {
        rows = maze.size();
        cols = maze[0].size();

        // Initialize visited array and parent array
        visited.resize(rows, vector<bool>(cols, false));
        parent.resize(rows, vector<Point>(cols));

        // Initialize random number generator
        rng.seed(static_cast<unsigned int>(time(nullptr)));
    }

    void generateMaze() {
        // Reset the maze to all walls
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                maze[i][j] = '#';
            }
        }

        // Create a grid of cells for maze generation
        // We'll work with odd coordinates to have walls between cells
        vector<vector<bool>> visited_gen((rows - 1) / 2 + 1, vector<bool>((cols - 1) / 2 + 1, false));

        // Start at a random cell (using odd coordinates)
        int start_row = 1;
        int start_col = 1;

        // Mark as visited
        visited_gen[start_row / 2][start_col / 2] = true;
        maze[start_row][start_col] = '.';

        // Stack for backtracking
        stack<pair<int, int>> stack_gen;
        stack_gen.push(make_pair(start_row, start_col));

        // Continue until all cells have been visited
        while (!stack_gen.empty()) {
            // Get current cell
            int curr_row = stack_gen.top().first;
            int curr_col = stack_gen.top().second;

            // Check for unvisited neighbors
            vector<int> directions = { 0, 1, 2, 3 }; // Up, Right, Down, Left
            shuffle(directions.begin(), directions.end(), rng);

            bool found_next = false;
            for (int dir : directions) {
                int new_row = curr_row + 2 * dr[dir];
                int new_col = curr_col + 2 * dc[dir];

                // Check if the new cell is within bounds and not visited
                if (new_row > 0 && new_row < rows && new_col > 0 && new_col < cols &&
                    !visited_gen[new_row / 2][new_col / 2]) {

                    // Mark the wall and the cell as path
                    maze[curr_row + dr[dir]][curr_col + dc[dir]] = '.';
                    maze[new_row][new_col] = '.';

                    // Mark as visited
                    visited_gen[new_row / 2][new_col / 2] = true;

                    // Push to stack
                    stack_gen.push(make_pair(new_row, new_col));
                    found_next = true;
                    break;
                }
            }

            // If no unvisited neighbors, backtrack
            if (!found_next) {
                stack_gen.pop();
            }
        }

        // Set start and end points at opposite corners
        start = Point(1, 1);
        end = Point(rows - 2, cols - 2);

        // Make sure start and end are open paths
        maze[start.row][start.col] = 'S';
        maze[end.row][end.col] = 'E';

        // Ensure there's a path to the entrance and exit
        if (start.row - 1 >= 0) maze[start.row - 1][start.col] = '.';
        if (end.row + 1 < rows) maze[end.row + 1][end.col] = '.';
    }

    bool isValid(int row, int col) {
        // Check if cell is within the maze boundaries and is not a wall
        return (row >= 0 && row < rows && col >= 0 && col < cols && maze[row][col] != '#');
    }

    void resetVisited() {
        // Reset visited and parent arrays
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                visited[i][j] = false;
            }
        }
    }

    bool solveDFS() {
        // Reset visited array
        resetVisited();

        stack<Point> s;
        s.push(start);
        visited[start.row][start.col] = true;

        while (!s.empty()) {
            Point current = s.top();
            s.pop();

            // If we've reached the end
            if (current.row == end.row && current.col == end.col) {
                return true;
            }

            // Try all four directions
            for (int i = 0; i < 4; i++) {
                int newRow = current.row + dr[i];
                int newCol = current.col + dc[i];

                if (isValid(newRow, newCol) && !visited[newRow][newCol]) {
                    // Mark as visited
                    visited[newRow][newCol] = true;
                    // Remember where we came from
                    parent[newRow][newCol] = current;
                    // Add to stack
                    s.push(Point(newRow, newCol));
                }
            }
        }

        // No path found
        return false;
    }

    void printMaze() {
        cout << "Generated Maze:" << endl;
        for (const auto& row : maze) {
            for (char cell : row) {
                cout << cell << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    void printPath() {
        if (!solveDFS()) {
            cout << "No solution exists!" << endl;
            return;
        }

        // Create a copy of the maze to mark the path
        vector<vector<char>> solution = maze;

        // Reconstruct the path
        Point current = end;
        while (!(current == start)) {
            solution[current.row][current.col] = '*'; // Mark path
            current = parent[current.row][current.col];
        }
        solution[start.row][start.col] = 'S'; // Mark start
        solution[end.row][end.col] = 'E';     // Mark end

        // Print the solution
        cout << "Maze Solution:" << endl;
        for (const auto& row : solution) {
            for (char cell : row) {
                cout << cell << " ";
            }
            cout << endl;
        }
    }
};

int main() {
    // Random maze size (odd numbers work best)
    int rows = 21;
    int cols = 21;

    // Create a maze solver with the desired size
    MazeSolver randomMaze(rows, cols);

    // Generate a random maze
    randomMaze.generateMaze();

    // Print the generated maze
    randomMaze.printMaze();

    // Solve and print the solution
    randomMaze.printPath();

    return 0;
}