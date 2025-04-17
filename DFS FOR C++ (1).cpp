#include <SFML/Graphics.hpp>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <map>
#include <string>
#include <fstream>
#include <regex>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>

// Constants
const std::vector<char> DIRECTIONS = {'E', 'S', 'W', 'N'};
const std::map<char, std::pair<int, int>> DIRECTION_OFFSETS = {
    {'E', {0, 1}}, {'W', {0, -1}}, {'N', {-1, 0}}, {'S', {1, 0}}
};
const std::string MAZE_CONFIG_FILE = "maze_config.txt";
const float CELL_SIZE = 40.0f; // Pixels per cell for visualization
const float AGENT_SIZE = 20.0f; // Agent size in pixels

// Configuration struct
struct Config {
    int rows, cols;
    std::pair<int, int> start, end;
};

// Maze cell structure
struct Cell {
    bool walls[4] = {true, true, true, true}; // E, S, W, N
    bool visited = false;
};

// Parse coordinate from config string
std::pair<int, int> parseCoordinate(const std::string& content, const std::string& key) {
    std::regex pattern(key + "\\s*=\\s*\\((\\d+)\\s*,\\s*(\\d+)\\)");
    std::smatch match;
    if (!std::regex_search(content, match, pattern)) {
        throw std::runtime_error("Cannot parse '" + key + "'. Expected format: " + key + " = (row, col)");
    }
    return {std::stoi(match[1]), std::stoi(match[2])};
}

// Read configuration file
Config extractVariables(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Configuration file '" + filename + "' not found.");
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::regex rowsRegex("number of rows\\s*=\\s*(\\d+)");
    std::regex colsRegex("number of columns\\s*=\\s*(\\d+)");
    std::smatch match;

    Config config;
    if (std::regex_search(content, match, rowsRegex)) {
        config.rows = std::stoi(match[1]);
        if (config.rows < 1) throw std::runtime_error("Rows must be positive.");
    } else {
        throw std::runtime_error("Missing 'number of rows' in config.");
    }

    if (std::regex_search(content, match, colsRegex)) {
        config.cols = std::stoi(match[1]);
        if (config.cols < 1) throw std::runtime_error("Columns must be positive.");
    } else {
        throw std::runtime_error("Missing 'number of columns' in config.");
    }

    config.start = parseCoordinate(content, "start_location");
    config.end = parseCoordinate(content, "end_location");

    // Validate coordinates (1-based to 0-based conversion for internal use)
    auto validateCoord = [&](const std::pair<int, int>& coord, const std::string& name) {
        if (coord.first < 1 || coord.first > config.rows || coord.second < 1 || coord.second > config.cols) {
            throw std::runtime_error(name + " (" + std::to_string(coord.first) + "," + std::to_string(coord.second) +
                                     ") is outside maze boundaries (1..." + std::to_string(config.rows) + ", 1..." +
                                     std::to_string(config.cols) + ").");
        }
    };
    validateCoord(config.start, "Start location");
    validateCoord(config.end, "End location");

    // Convert to 0-based indexing
    config.start = {config.start.first - 1, config.start.second - 1};
    config.end = {config.end.first - 1, config.end.second - 1};
    return config;
}

// Generate maze using recursive backtracking
void generateMaze(std::vector<std::vector<Cell>>& maze, int rows, int cols, std::mt19937& rng) {
    std::stack<std::pair<int, int>> stack;
    maze[0][0].visited = true;
    stack.push({0, 0});

    while (!stack.empty()) {
        auto [r, c] = stack.top();
        stack.pop();

        // Get unvisited neighbors
        std::vector<std::pair<int, char>> neighbors;
        for (char dir : DIRECTIONS) {
            auto [dr, dc] = DIRECTION_OFFSETS.at(dir);
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !maze[nr][nc].visited) {
                neighbors.emplace_back(std::pair<int, int>{nr, nc}, dir);
            }
        }

        if (!neighbors.empty()) {
            stack.push({r, c});
            // Shuffle neighbors for randomness
            std::shuffle(neighbors.begin(), neighbors.end(), rng);
            auto [pos, dir] = neighbors[0];
            auto [nr, nc] = pos;

            // Remove walls between current and neighbor
            int dirIdx;
            if (dir == 'E') dirIdx = 0;
            else if (dir == 'S') dirIdx = 1;
            else if (dir == 'W') dirIdx = 2;
            else dirIdx = 3;

            maze[r][c].walls[dirIdx] = false;
            char oppDir = (dir == 'E') ? 'W' : (dir == 'W') ? 'E' : (dir == 'S') ? 'N' : 'S';
            int oppDirIdx = (oppDir == 'E') ? 0 : (oppDir == 'S') ? 1 : (oppDir == 'W') ? 2 : 3;
            maze[nr][nc].walls[oppDirIdx] = false;

            maze[nr][nc].visited = true;
            stack.push({nr, nc});
        }
    }
}

// DFS for full exploration
std::vector<std::pair<int, int>> exploreAllReachableDFS(const std::vector<std::vector<Cell>>& maze,
                                                       std::pair<int, int> start,
                                                       std::pair<int, int> target) {
    std::cout << "\n--- Starting Full Exploration Simulation from (" << start.first + 1 << "," << start.second + 1
              << ") (DFS) ---\n";
    std::stack<std::pair<int, int>> stack;
    std::set<std::pair<int, int>> visited;
    std::vector<std::pair<int, int>> explorationOrder;
    bool foundEnd = false;

    stack.push(start);
    visited.insert(start);

    while (!stack.empty()) {
        auto current = stack.top();
        stack.pop();
        explorationOrder.push_back(current);

        if (current == target && !foundEnd) {
            std::cout << "!!! Note: Target end_node (" << target.first + 1 << "," << target.second + 1
                      << ") encountered during full exploration. Exploration continues... !!!\n";
            foundEnd = true;
        }

        for (int i = 0; i < 4; ++i) {
            char dir = DIRECTIONS[i];
            if (!maze[current.first][current.second].walls[i]) {
                auto [dr, dc] = DIRECTION_OFFSETS.at(dir);
                std::pair<int, int> next = {current.first + dr, current.second + dc};
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    stack.push(next);
                }
            }
        }
    }

    std::cout << "--- DFS Exploration Simulation Complete ---\n";
    std::cout << "DFS Visited " << visited.size() << " cells.\n";
    if (!foundEnd && (target.first < maze.size() && target.second < maze[0].size())) {
        if (visited.find(target) == visited.end()) {
            std::cout << "Note: Target end_node (" << target.first + 1 << "," << target.second + 1
                      << ") was specified but not reachable from (" << start.first + 1 << "," << start.second + 1
                      << ").\n";
        }
    }
    return explorationOrder;
}

// BFS for shortest path
std::map<std::pair<int, int>, std::pair<int, int>> findShortestPathBFS(const std::vector<std::vector<Cell>>& maze,
                                                                      std::pair<int, int> start,
                                                                      std::pair<int, int> end) {
    std::cout << "\n--- Finding Shortest Path from (" << start.first + 1 << "," << start.second + 1 << ") to ("
              << end.first + 1 << "," << end.second + 1 << ") (BFS to Dict) ---\n";
    std::queue<std::pair<int, int>> queue;
    std::set<std::pair<int, int>> visited;
    std::map<std::pair<int, int>, std::pair<int, int>> parentMap;
    bool pathFound = false;

    queue.push(start);
    visited.insert(start);

    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        if (current == end) {
            pathFound = true;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            char dir = DIRECTIONS[i];
            if (!maze[current.first][current.second].walls[i]) {
                auto [dr, dc] = DIRECTION_OFFSETS.at(dir);
                std::pair<int, int> next = {current.first + dr, current.second + dc};
                if (visited.find(next) == visited.end()) {
                    visited.insert(next);
                    parentMap[next] = current;
                    queue.push(next);
                }
            }
        }
    }

    std::map<std::pair<int, int>, std::pair<int, int>> pathDict;
    if (pathFound) {
        std::cout << "Shortest path found. Reconstructing path dictionary...\n";
        auto curr = end;
        while (curr != start) {
            auto parent = parentMap[curr];
            pathDict[parent] = curr;
            curr = parent;
        }
        std::cout << "Shortest path dictionary created with " << pathDict.size() << " segments.\n";
    } else {
        std::cout << "Error: No path found from (" << start.first + 1 << "," << start.second + 1 << ") to ("
                  << end.first + 1 << "," << end.second + 1 << ").\n";
    }
    return pathDict;
}

// Main function
int main() {
    try {
        // Load configuration
        std::cout << "Debug: Loading configuration...\n";
        Config config = extractVariables(MAZE_CONFIG_FILE);
        std::cout << "Debug: Config loaded - Rows: " << config.rows << ", Cols: " << config.cols
                  << ", Start: (" << config.start.first + 1 << "," << config.start.second + 1
                  << "), End: (" << config.end.first + 1 << "," << config.end.second + 1 << ")\n";

        // Initialize maze
        std::vector<std::vector<Cell>> maze(config.rows, std::vector<Cell>(config.cols));
        std::random_device rd;
        std::mt19937 rng(rd());
        std::cout << "Debug: Generating maze...\n";
        generateMaze(maze, config.rows, config.cols, rng);
        std::cout << "Debug: Maze created.\n";

        // Run DFS exploration
        std::cout << "Debug: Starting full exploration simulation...\n";
        auto explorationSequence = exploreAllReachableDFS(maze, config.start, config.end);
        std::cout << "Debug: Exploration sequence length: " << explorationSequence.size() << "\n";

        // Run BFS for shortest path
        std::cout << "Debug: Finding shortest path (dict format)...\n";
        auto shortestPathDict = findShortestPathBFS(maze, config.start, config.end);
        std::cout << "Debug: Shortest path dict segments: " << shortestPathDict.size() << "\n";

        // SFML Visualization
        sf::RenderWindow window(sf::VideoMode(config.cols * CELL_SIZE, config.rows * CELL_SIZE), "Maze Visualization");
        window.setFramerateLimit(60);

        // Agents
        sf::RectangleShape agentExplore(sf::Vector2f(AGENT_SIZE, AGENT_SIZE));
        agentExplore.setFillColor(sf::Color::Blue);
        agentExplore.setOrigin(AGENT_SIZE / 2, AGENT_SIZE / 2);

        sf::RectangleShape agentShortest(sf::Vector2f(AGENT_SIZE, AGENT_SIZE));
        agentShortest.setFillColor(sf::Color::Red);
        agentShortest.setOrigin(AGENT_SIZE / 2, AGENT_SIZE / 2);

        // Animation variables
        size_t exploreIdx = 0;
        size_t shortestIdx = 0;
        bool showExplore = !explorationSequence.empty();
        bool showShortest = !shortestPathDict.empty();
        std::vector<std::pair<int, int>> shortestPath;
        if (showShortest) {
            auto curr = config.end;
            shortestPath.push_back(curr);
            while (shortestPathDict.find(curr) != shortestPathDict.end()) {
                curr = shortestPathDict[curr];
                shortestPath.push_back(curr);
            }
            std::reverse(shortestPath.begin(), shortestPath.end());
        }

        sf::Clock clock;
        float exploreDelay = 0.03f; // 30ms per step
        float shortestDelay = 0.075f; // 75ms per step
        float exploreTimer = 0.0f;
        float shortestTimer = 0.0f;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }

            float deltaTime = clock.restart().asSeconds();
            window.clear(sf::Color::White);

            // Draw maze
            for (int r = 0; r < config.rows; ++r) {
                for (int c = 0; c < config.cols; ++c) {
                    float x = c * CELL_SIZE;
                    float y = r * CELL_SIZE;
                    sf::VertexArray lines(sf::Lines, 8);
                    if (maze[r][c].walls[0]) { // East
                        lines[0].position = sf::Vector2f(x + CELL_SIZE, y);
                        lines[1].position = sf::Vector2f(x + CELL_SIZE, y + CELL_SIZE);
                    }
                    if (maze[r][c].walls[1]) { // South
                        lines[2].position = sf::Vector2f(x, y + CELL_SIZE);
                        lines[3].position = sf::Vector2f(x + CELL_SIZE, y + CELL_SIZE);
                    }
                    if (maze[r][c].walls[2]) { // West
                        lines[4].position = sf::Vector2f(x, y);
                        lines[5].position = sf::Vector2f(x, y + CELL_SIZE);
                    }
                    if (maze[r][c].walls[3]) { // North
                        lines[6].position = sf::Vector2f(x, y);
                        lines[7].position = sf::Vector2f(x + CELL_SIZE, y);
                    }
                    for (int i = 0; i < 8; ++i) lines[i].color = sf::Color::Black;
                    window.draw(lines);

                    // Draw start and end points
                    if (r == config.start.first && c == config.start.second) {
                        sf::RectangleShape startRect(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
                        startRect.setPosition(x + 5, y + 5);
                        startRect.setFillColor(sf::Color::Green);
                        window.draw(startRect);
                    }
                    if (r == config.end.first && c == config.end.second) {
                        sf::RectangleShape endRect(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
                        endRect.setPosition(x + 5, y + 5);
                        endRect.setFillColor(sf::Color::Magenta);
                        window.draw(endRect);
                    }
                }
            }

            // Update and draw exploration agent
            if (showExplore && exploreIdx < explorationSequence.size()) {
                exploreTimer += deltaTime;
                if (exploreTimer >= exploreDelay) {
                    exploreIdx++;
                    exploreTimer = 0.0f;
                }
                if (exploreIdx < explorationSequence.size()) {
                    auto [r, c] = explorationSequence[exploreIdx];
                    agentExplore.setPosition(c * CELL_SIZE + CELL_SIZE / 2, r * CELL_SIZE + CELL_SIZE / 2);
                    window.draw(agentExplore);
                } else {
                    showExplore = false;
                    std::cout << "Debug: Exploration visualization finished.\n";
                }
            }

            // Update and draw shortest path agent
            if (!showExplore && showShortest && shortestIdx < shortestPath.size()) {
                shortestTimer += deltaTime;
                if (shortestTimer >= shortestDelay) {
                    shortestIdx++;
                    shortestTimer = 0.0f;
                }
                if (shortestIdx < shortestPath.size()) {
                    auto [r, c] = shortestPath[shortestIdx];
                    agentShortest.setPosition(c * CELL_SIZE + CELL_SIZE / 2, r * CELL_SIZE + CELL_SIZE / 2);
                    window.draw(agentShortest);
                } else {
                    showShortest = false;
                    std::cout << "Debug: Shortest path visualization finished.\n";
                }
            }

            window.display();

            // Close window after both animations
            if (!showExplore && !showShortest) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                window.close();
            }
        }

        std::cout << "Debug: Visualization window closed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

/*
 * To compile and run this program, you need to install SFML (Simple and Fast Multimedia Library).
 * 
 * On Ubuntu/Debian:
 *   sudo apt-get install libsfml-dev
 *   g++ -c maze.cpp
 *   g++ maze.o -o maze -lsfml-graphics -lsfml-window -lsfml-system
 *   ./maze
 * 
 * On macOS (using Homebrew):
 *   brew install sfml
 *   g++ -c maze.cpp -I/usr/local/include
 *   g++ maze.o -o maze -L/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system
 *   ./maze
 * 
 * On Windows:
 *   - Download SFML from https://www.sfml-dev.org/download.php
 *   - Set up your compiler (e.g., MinGW, MSVC) with SFML libraries
 *   - Link against sfml-graphics, sfml-window, sfml-system
 * 
 * Ensure maze_config.txt is in the same directory with format:
 *   number of rows = 10
 *   number of columns = 10
 *   start_location = (1, 1)
 *   end_location = (10, 10)
 */
