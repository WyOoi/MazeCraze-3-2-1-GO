# This program uses a library to generate the maze, the library is called pyamaze
# You can install this library using pip by 'pip install pyamaze' or you can run 'pip install -r requirements' in this directory
from pyamaze import maze, COLOR, agent
import sys
import re
from typing import Dict, Tuple, List, Set
from collections import deque # Needed for BFS
import time # Import time

# ---------- Constants ----------
DIRECTIONS = ['E', 'S', 'W', 'N']  # Order can influence exploration path
DIRECTION_OFFSETS = {'E': (0, 1), 'W': (0, -1), 'N': (-1, 0), 'S': (1, 0)}
MAZE_CONFIG_FILE = "maze_config.txt"
# -----------------------------

# --- Helper Functions (parse_coordinate, extract_variables, get_new_location) ---
def parse_coordinate(config_str: str, key: str) -> Tuple[int, int]:
    """Helper to parse (row, col) coordinates from config string using regex."""
    pattern = re.compile(rf"{key}\s*=\s*\((\d+)\s*,\s*(\d+)\)")
    match = pattern.search(config_str)
    if not match:
        raise ValueError(f"Cannot find or parse '{key}' in config. Expected format: {key} = (row, col)")
    # Convert captured groups (strings) to integers
    return (int(match.group(1)), int(match.group(2)))

def extract_variables(file_name: str) -> Dict:
    """
    Reads maze configuration from a file.
    Uses regex for more robust parsing.
    Requires rows, columns, start_location, and end_location.
    """
    try:
        with open(file_name, "r") as file:
            contents = file.read()

        # Use regex to find integer values for rows and columns
        def get_value(pattern, name):
            match = re.search(pattern, contents)
            if not match:
                raise ValueError(f"Missing '{name}' in config file '{file_name}'.")
            val = int(match.group(1))
            # Ensure dimensions are positive
            if val < 1:
                 raise ValueError(f"'{name}' must be a positive integer, got {val}.")
            return val

        maze_rows = get_value(r"number of rows\s*=\s*(\d+)", "number of rows")
        maze_cols = get_value(r"number of columns\s*=\s*(\d+)", "number of columns")

        # Parse start and end locations using the helper function
        start_loc = parse_coordinate(contents, "start_location")
        end_loc = parse_coordinate(contents, "end_location")

        # Validate coordinates are within bounds (1 to rows/cols inclusive)
        def is_valid_coord(coord, r, c, name):
            if not (1 <= coord[0] <= r and 1 <= coord[1] <= c):
                raise ValueError(f"{name} {coord} is outside the maze boundaries (1...{r}, 1...{c}).")

        is_valid_coord(start_loc, maze_rows, maze_cols, "Start location")
        is_valid_coord(end_loc, maze_rows, maze_cols, "End location")

        # Return the extracted configuration as a dictionary
        return {
            "rows": maze_rows,
            "columns": maze_cols,
            "start": start_loc,
            "end": end_loc
        }
    except FileNotFoundError:
        print(f"Error: Configuration file '{file_name}' not found.")
        sys.exit(1) # Exit if config file is missing
    except (ValueError, IndexError, Exception) as e:
        # Catch potential errors during parsing or validation
        print(f"Error processing configuration file '{file_name}': {e}")
        sys.exit(1) # Exit on error

def get_new_location(current: Tuple[int, int], direction: str) -> Tuple[int, int]:
    """Calculates the coordinates of the neighbor cell in a given direction."""
    # Get the coordinate change for the direction, default to (0,0) if direction is invalid
    offset = DIRECTION_OFFSETS.get(direction, (0, 0))
    # Calculate and return the new coordinates
    return (current[0] + offset[0], current[1] + offset[1])
# --- End Helper Functions ---


# --- Exploration Algorithm (DFS - Returns LIST, checks for end_node) ---
def explore_all_reachable_dfs(maze_obj, start_node: Tuple[int, int], target_end_node: Tuple[int, int]) -> List[Tuple[int, int]]:
    """
    Performs DFS exploration, notes when end_node is found, but explores everything.
    Returns the sequence list of visited cells.
    """
    print(f"\n--- Starting Full Exploration Simulation from {start_node} (DFS) ---")
    stack: List[Tuple[int, int]] = [start_node] # LIFO stack for DFS
    visited: Set[Tuple[int, int]] = {start_node} # Set to track visited cells
    exploration_order: List[Tuple[int, int]] = [] # List to store visit order
    found_end_node_flag = False # Flag to print message only once

    while stack: # Loop until stack is empty
        current_loc = stack.pop() # Get the last added cell
        exploration_order.append(current_loc) # Record visit order

        # --- Optional Debug Print ---
        # Check if this is the target end node (only print the first time)
        if current_loc == target_end_node and not found_end_node_flag:
            print(f"!!! Note: Target end_node {target_end_node} encountered during full exploration. Exploration continues... !!!")
            found_end_node_flag = True
        # --- End Optional Debug Print ---

        # Explore neighbors
        for direction in DIRECTIONS:
            # Check if a path exists in this direction from the current cell
            if maze_obj.maze_map[current_loc][direction]:
                new_loc = get_new_location(current_loc, direction)
                # If the neighbor hasn't been visited yet
                if new_loc not in visited:
                    visited.add(new_loc) # Mark as visited
                    stack.append(new_loc) # Add to stack to explore later

    print(f"--- DFS Exploration Simulation Complete ---")
    print(f"DFS Visited {len(visited)} cells.") # Print total unique cells visited
    # Check if the target node was ever found if it exists in the maze map
    if not found_end_node_flag:
         if target_end_node in maze_obj.maze_map:
             if target_end_node not in visited:
                 print(f"Note: Target end_node {target_end_node} was specified but not reachable from {start_node}.")
         else:
              print(f"Warning: Target end_node {target_end_node} seems invalid for this maze size.")

    return exploration_order # Return the full exploration sequence

# --- Shortest Path Algorithm (BFS - Returns DICT {parent: child}) ---
def find_shortest_path_bfs_dict(maze_obj, start_node: Tuple[int, int], end_node: Tuple[int, int]) -> Dict[Tuple[int, int], Tuple[int, int]]:
    """
    Finds the shortest path using BFS and returns a dictionary {parent: child}.
    """
    print(f"\n--- Finding Shortest Path from {start_node} to {end_node} (BFS to Dict) ---")
    queue = deque([start_node]) # FIFO queue for BFS
    visited = {start_node} # Track visited for this specific search
    parent_map = {} # Stores {child: parent} to reconstruct path
    path_found = False

    while queue:
        current_loc = queue.popleft() # Get the oldest cell in queue
        if current_loc == end_node:
            path_found = True
            break # Target found, stop BFS

        # Explore neighbors
        for direction in DIRECTIONS:
            # Check if path exists
            if maze_obj.maze_map[current_loc][direction]:
                new_loc = get_new_location(current_loc, direction)
                # If neighbor not visited yet in this search
                if new_loc not in visited:
                    visited.add(new_loc) # Mark visited
                    parent_map[new_loc] = current_loc # Record parent relationship
                    queue.append(new_loc) # Add to queue

    # Reconstruct path from end_node back to start_node using parent_map
    path_dict = {}
    if path_found:
        print(f"Shortest path found. Reconstructing path dictionary...")
        curr = end_node
        # Trace back until the start node is reached
        while curr != start_node:
            parent = parent_map.get(curr)
            if parent is None:
                 # This should ideally not happen if path_found is True
                 print(f"Error: Path reconstruction failed, parent not found for {curr}")
                 return {}
            # Store in the format tracePath uses: {parent: child}
            path_dict[parent] = curr
            curr = parent # Move to the parent for next iteration
        print(f"Shortest path dictionary created with {len(path_dict)} segments.")
    else:
        # If the loop finished without finding the end node
        print(f"Error: No path found from {start_node} to {end_node}.")

    return path_dict


# --- Main Execution Block ---
if __name__ == '__main__':
    # 1. Load Configuration
    print("Debug: Loading configuration...")
    config = extract_variables(MAZE_CONFIG_FILE)
    maze_rows = config["rows"]
    maze_columns = config["columns"]
    start_loc = config["start"]
    end_loc = config["end"] # The actual target end location
    print(f"Debug: Config loaded - Rows: {maze_rows}, Cols: {maze_columns}, Start: {start_loc}, End: {end_loc}")

    # 2. Initialize Maze Object
    print("Debug: Initializing maze object...")
    m = maze(rows=maze_rows, cols=maze_columns)
    print(f"Creating {maze_rows}x{maze_columns} maze...")
    # Create the maze structure, marking the end_loc
    # loopPercent=0 creates a "perfect" maze (no loops, fully connected)
    # loopPercent>0 adds loops, potentially making paths shorter/more complex
    m.CreateMaze(end_loc[0], end_loc[1], loopPercent=100, theme=COLOR.dark)
    print("Debug: Maze created.")

    # 3. Calculate Full Exploration Path (as List)
    print("Debug: Starting full exploration simulation...")
    # Pass end_loc for the optional "found end" message during DFS
    exploration_sequence = explore_all_reachable_dfs(m, start_loc, end_loc)
    print(f"Debug: Exploration sequence length: {len(exploration_sequence)}")

    # 4. Calculate Shortest Path (as Dict {parent: child})
    print("Debug: Finding shortest path (dict format)...")
    shortest_path_dict = find_shortest_path_bfs_dict(m, start_loc, end_loc)
    print(f"Debug: Shortest path dict segments: {len(shortest_path_dict)}")

    # --- Visualization (Separate tracePath calls, explicit agent position) ---

    # 5. Visualize Exploration Path (Blue Agent, List Path)
    if exploration_sequence:
        print("\nVisualizing Full Exploration (Blue)...")
        # Create the blue agent for exploration visualization
        agent_explore = agent(m, footprints=True, filled=True, color=COLOR.blue, shape='arrow')
        # Explicitly set the starting position of the agent
        agent_explore.position = start_loc
        print("Debug: Calling tracePath for exploration (list)...")
        # Trace the path using the list of visited cells
        m.tracePath({agent_explore: exploration_sequence}, delay=30) # Adjust delay for speed
        print("Debug: tracePath for exploration finished.")
        time.sleep(0.1) # Optional brief pause
    else:
        print("No exploration path to visualize.")

    # 6. Visualize Shortest Path (Red Agent, Dict Path)
    if shortest_path_dict:
        print("\nVisualizing Shortest Path (Red)...")
        # Create the red agent for shortest path visualization
        agent_shortest = agent(m, footprints=True, filled=True, color=COLOR.red, shape='square')
        # Explicitly set the starting position of the agent
        agent_shortest.position = start_loc
        print("Debug: Calling tracePath for shortest path (dict)...")
        # Trace the path using the parent-child dictionary
        m.tracePath({agent_shortest: shortest_path_dict}, delay=75) # Adjust delay for speed
        print("Debug: tracePath for shortest path finished.")
        time.sleep(0.1) # Optional brief pause
    else:
        print("No shortest path found to visualize.")

    # 7. Run the Maze Visualization Window
    print("\nLaunching maze visualization window...")
    m.run() # Starts the Tkinter event loop to display the window
    print("Debug: m.run() finished (window closed).")