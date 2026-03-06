#include<Wire.h>

 // Define constants
  #define MAZE_MAX_SIZE 16
  #define UNEXPLORED -1
  #define WALL 1
  #define NO_WALL 0
  #define UNVISITED 0
  #define VISITED 1

  // Direction definitions
  #define NORTH 0
  #define EAST 1
  #define SOUTH 2
  #define WEST 3

int IRpinFront = PA7;
int IRpinLeft = PA6;
int IRpinRight = PA5; 

int IRreadFront = digitalRead(IRpinFront);
int IRreadRight = digitalRead(IRpinRight);
int IRreadLeft = digitalRead(IRpinLeft);



// Motors
int motor1pin1 = PA10;
int motor1pin2 = PA11;

int motor2pin1 = PB3;
int motor2pin2 = PB4;

int ENA = PA9;
int ENB = PA8;

// Maze representations
int floodArray[MAZE_MAX_SIZE][MAZE_MAX_SIZE]; // Distance from target
int wallMap[MAZE_MAX_SIZE][MAZE_MAX_SIZE][4]; // Wall information (N,E,S,W)
int visited[MAZE_MAX_SIZE][MAZE_MAX_SIZE];    // Visited cells
int explorationScore[MAZE_MAX_SIZE][MAZE_MAX_SIZE]; // Exploration priority

// Current position and orientation
int x = 0;
int y = 0;
int direction = NORTH;

// Target position (center of maze)
int targetX, targetY;
int mazeWidth, mazeHeight;

// Number of cells visited
int cellsVisited = 0;
int totalCells = 0;

// Queue for flood fill
typedef struct {
    int data[MAZE_MAX_SIZE * MAZE_MAX_SIZE][2];
    int front, rear;
} Queue;

Queue queue;

// void log(char* text) {
//     fprintf(stderr, "%s\n", text);
//     fflush(stderr);
// }

void initQueue() {
    queue.front = 0;
    queue.rear = 0;
}

bool isQueueEmpty() {
    return queue.front == queue.rear;
}

void enqueue(int x, int y) {
    queue.data[queue.rear][0] = x;
    queue.data[queue.rear][1] = y;
    queue.rear++;
}

void dequeue(int* x, int* y) {
    *x = queue.data[queue.front][0];
    *y = queue.data[queue.front][1];
    queue.front++;
}

// Convert direction to character for API_setWall
char directionToChar(int dir) {
    switch (dir) {
        case NORTH: return 'n';
        case EAST:  return 'e';
        case SOUTH: return 's';
        case WEST:  return 'w';
        default:    return '?';
    }
}


// Move Forward
int API_moveForward() {
  analogWrite(ENA, 100); 
  analogWrite(ENB, 100); 

  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, HIGH);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);

  delay(1500);

  }

// Turn Right
void API_turnRight() {
  analogWrite(ENA, 100); 
  analogWrite(ENB, 100); 

  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, HIGH);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, LOW);

  delay(1000);
  }

// Turn Left
void API_turnLeft() {
  analogWrite(ENA, 100); 
  analogWrite(ENB, 100); 

  digitalWrite(motor1pin1, LOW);
  digitalWrite(motor1pin2, LOW);
  digitalWrite(motor2pin1, LOW);
  digitalWrite(motor2pin2, HIGH);

  delay(1200);
  }

// Front Sense
int API_wallFront() {
    if(IRreadFront == 0){
        return 1;
    }
  }

// Right Sense
int API_wallRight() {
    if(IRreadRight == 0){
        return 1;
    }
  }

// Left Sense
int API_wallLeft() {
    if(IRreadLeft == 0){
        return 1;
    }
  }


void setup() {
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdbool.h>
  // #include "API.h"

  Wire.begin();  

  //Start the motor shield
  pinMode(motor1pin1, OUTPUT);
  pinMode(motor1pin2, OUTPUT);
  pinMode(motor2pin1, OUTPUT);
  pinMode(motor2pin2, OUTPUT);
  pinMode(ENA, OUTPUT); 
  pinMode(ENB, OUTPUT);


  pinMode(IRpinFront, INPUT);
  pinMode(IRpinLeft, INPUT);
  pinMode(IRpinRight, INPUT);

}


// Initialize the maze with default values
void initializeMaze() {
    int mazeWidth = 16;
    int mazeHeight = 16;
    totalCells = mazeWidth * mazeHeight;
    
    // Calculate center of maze as target
    targetX = mazeWidth / 2;
    targetY = mazeHeight / 2;
    
    // If even dimensions, consider multiple center cells
    if (mazeWidth % 2 == 0) targetX--;
    if (mazeHeight % 2 == 0) targetY--;
    
    // Initialize all cells as unexplored
    for (int i = 0; i < mazeWidth; i++) {
        for (int j = 0; j < mazeHeight; j++) {
            floodArray[i][j] = UNEXPLORED;
            visited[i][j] = UNVISITED;
            explorationScore[i][j] = 0;
            
            // Initialize all walls as unknown (-1)
            for (int k = 0; k < 4; k++) {
                wallMap[i][j][k] = UNEXPLORED;
            }
        }
    }
    
    // Set outer maze boundaries as walls
    for (int i = 0; i < mazeWidth; i++) {
        wallMap[i][0][SOUTH] = WALL;
        wallMap[i][mazeHeight-1][NORTH] = WALL;
    }
    
    for (int j = 0; j < mazeHeight; j++) {
        wallMap[0][j][WEST] = WALL;
        wallMap[mazeWidth-1][j][EAST] = WALL;
    }
    
    // Mark start position as visited
    visited[0][0] = VISITED;
    cellsVisited = 1;
}

// Update wall information based on current position and sensor readings
void updateWalls() {
    // Mark current cell as visited if not already
    if (visited[x][y] == UNVISITED) {
        visited[x][y] = VISITED;
        cellsVisited++;
        
        // Visual feedback - color visited cells
        // API_setColor(x, y, 'G');
    }
    
    // Check for walls around the current position
    wallMap[x][y][direction] = API_wallFront() ? WALL : NO_WALL;
    wallMap[x][y][(direction + 1) % 4] = API_wallRight() ? WALL : NO_WALL;
    wallMap[x][y][(direction + 3) % 4] = API_wallLeft() ? WALL : NO_WALL;
    
    // Set walls in visualization
    // if (wallMap[x][y][direction] == WALL) {
    //     API_setWall(x, y, directionToChar(direction));
    // }
    // if (wallMap[x][y][(direction + 1) % 4] == WALL) {
    //     API_setWall(x, y, directionToChar((direction + 1) % 4));
    // }
    // if (wallMap[x][y][(direction + 3) % 4] == WALL) {
    //     API_setWall(x, y, directionToChar((direction + 3) % 4));
    // }
    
    // Update the opposite wall information for adjacent cells
    int nx, ny, opposite;
    
    // Front
    nx = x; ny = y;
    switch (direction) {
        case NORTH: ny++; break;
        case EAST:  nx++; break;
        case SOUTH: ny--; break;
        case WEST:  nx--; break;
    }
    opposite = (direction + 2) % 4;
    if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight) {
        wallMap[nx][ny][opposite] = wallMap[x][y][direction];
        if (wallMap[nx][ny][opposite] == WALL) {
            // API_setWall(nx, ny, directionToChar(opposite));
        }
    }
    
    // Right
    nx = x; ny = y;
    switch ((direction + 1) % 4) {
        case NORTH: ny++; break;
        case EAST:  nx++; break;
        case SOUTH: ny--; break;
        case WEST:  nx--; break;
    }
    opposite = (direction + 3) % 4;
    if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight) {
        wallMap[nx][ny][opposite] = wallMap[x][y][(direction + 1) % 4];
        if (wallMap[nx][ny][opposite] == WALL) {
            // API_setWall(nx, ny, directionToChar(opposite));
        }
    }
    
    // Left
    nx = x; ny = y;
    switch ((direction + 3) % 4) {
        case NORTH: ny++; break;
        case EAST:  nx++; break;
        case SOUTH: ny--; break;
        case WEST:  nx--; break;
    }
    opposite = (direction + 1) % 4;
    if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight) {
        wallMap[nx][ny][opposite] = wallMap[x][y][(direction + 3) % 4];
        if (wallMap[nx][ny][opposite] == WALL) {
            // API_setWall(nx, ny, directionToChar(opposite));
        }
    }
}

// Calculate exploration scores for unvisited cells
void updateExplorationScores() {
    for (int i = 0; i < mazeWidth; i++) {
        for (int j = 0; j < mazeHeight; j++) {
            if (visited[i][j] == UNVISITED) {
                // Count number of adjacent cells that have been visited
                int adjacentVisited = 0;
                int unknownWalls = 0;
                
                for (int d = 0; d < 4; d++) {
                    int ni = i, nj = j;
                    
                    switch (d) {
                        case NORTH: nj++; break;
                        case EAST:  ni++; break;
                        case SOUTH: nj--; break;
                        case WEST:  ni--; break;
                    }
                    
                    if (ni >= 0 && ni < mazeWidth && nj >= 0 && nj < mazeHeight) {
                        if (visited[ni][nj] == VISITED) {
                            adjacentVisited++;
                        }
                        
                        if (wallMap[i][j][d] == UNEXPLORED) {
                            unknownWalls++;
                        }
                    }
                }
                
                // Prioritize cells that are adjacent to visited cells but have unknown walls
                explorationScore[i][j]=adjacentVisited * 10 + unknownWalls;
            } else {
                explorationScore[i][j] = 0;
            }
        }
    }
}

// Flood fill algorithm to update distances to a target cell
void floodFill(int targetX, int targetY) {
    // Reset all cells to UNEXPLORED
    for (int i = 0; i < mazeWidth; i++) {
        for (int j = 0; j < mazeHeight; j++) {
            floodArray[i][j] = UNEXPLORED;
        }
    }
    
    // Initialize queue with target cell
    initQueue();
    enqueue(targetX, targetY);
    floodArray[targetX][targetY] = 0;
    
    // BFS flood fill
    int cx, cy;
    while (!isQueueEmpty()) {
        dequeue(&cx, &cy);
        
        // Check all four directions
        for (int d = 0; d < 4; d++) {
            int nx = cx, ny = cy;
            
            // Calculate neighbor coordinates
            switch (d) {
                case NORTH: ny++; break;
                case EAST:  nx++; break;
                case SOUTH: ny--; break;
                case WEST:  nx--; break;
            }
            
            // Check if neighbor is valid
            if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight) {
                // Check if there's no wall between current cell and neighbor
                if (wallMap[cx][cy][d] != WALL && floodArray[nx][ny] == UNEXPLORED) {
                    floodArray[nx][ny] = floodArray[cx][cy] + 1;
                    enqueue(nx, ny);
                }
            }
        }
    }
}

// Find the nearest unvisited cell
void findNearestUnvisited(int* tx, int* ty) {
    updateExplorationScores();
    
    int bestScore = -1;
    *tx = -1;
    *ty = -1;
    
    for (int i = 0; i < mazeWidth; i++) {
        for (int j = 0; j < mazeHeight; j++) {
            if (visited[i][j] == UNVISITED && explorationScore[i][j] > bestScore) {
                bestScore = explorationScore[i][j];
                *tx = i;
                *ty = j;
            }
        }
    }
    
    // If no unvisited cells with good scores, find any unvisited cell
    if (*tx == -1) {
        for (int i = 0; i < mazeWidth; i++) {
            for (int j = 0; j < mazeHeight; j++) {
                if (visited[i][j] == UNVISITED) {
                    *tx = i;
                    *ty = j;
                    return;
                }
            }
        }
    }
}

// Move to the neighbor with lowest flood value
bool moveToLowestNeighbor() {
    int minValue = 1000;
    int bestDirection = -1;
    
    // Check all four directions
    for (int d = 0; d < 4; d++) {
        int nx = x, ny = y;
        
        // Calculate neighbor coordinates
        switch (d) {
            case NORTH: ny++; break;
            case EAST:  nx++; break;
            case SOUTH: ny--; break;
            case WEST:  nx--; break;
        }
        
        // Check if neighbor is valid and accessible
        if (nx >= 0 && nx < mazeWidth && ny >= 0 && ny < mazeHeight &&
            wallMap[x][y][d] == NO_WALL && floodArray[nx][ny] != UNEXPLORED) {
            
            if (floodArray[nx][ny] < minValue) {
                minValue = floodArray[nx][ny];
                bestDirection = d;
            }
        }
    }
    
    // Turn to face the best direction
    if (bestDirection != -1) {
        // Calculate how many turns needed
        int turns = (bestDirection - direction + 4) % 4;
        
        // Execute turns
        switch (turns) {
            case 0: // Already facing the right direction
                break;
            case 1: // Turn right
                API_turnRight();
                break;
            case 2: // Turn around (two rights)
                API_turnRight();
                API_turnRight();
                break;
            case 3: // Turn left
                API_turnLeft();
                break;
        }
        
        // Update direction
        direction = bestDirection;
        
        // Move forward
        if (API_moveForward()) {
            // Update position
            switch (direction) {
                case NORTH: y++; break;
                case EAST:  x++; break;
                case SOUTH: y--; break;
                case WEST:  x--; break;
            }
            return true;
        } else {
            // If move failed, update wall information
            wallMap[x][y][direction] = WALL;
            // API_setWall(x, y, directionToChar(direction));
            return false;
        }
    }
    return false;
}

// Move to the specified target
void moveToTarget(int tx, int ty) {
    floodFill(tx, ty);
    
    // Keep moving until we reach the target or can't move anymore
    while (!(x == tx && y == ty)) {
        updateWalls();
        floodFill(tx, ty);
        
        if (!moveToLowestNeighbor()) {
            // If stuck, find another path
            break;
        }
        
        // Visual feedback - update cell color for the current position
        if (visited[x][y] == UNVISITED) {
            // API_setColor(x, y, 'G');
            visited[x][y] = VISITED;
            cellsVisited++;
        } else {
            // API_setColor(x, y, 'B'); // Different color for revisited cells
        }
    }
}

// Explore the entire maze
void exploreMaze() {
    int targetX, targetY;
    char buffer[50];
    
    while (cellsVisited < totalCells) {
        // Find the nearest unvisited cell
        findNearestUnvisited(&targetX, &targetY);
        
        // If all cells visited or no path available
        if (targetX == -1 || targetY == -1) {
            break;
        }
        
        // sprintf(buffer, "Moving to unexplored cell (%d,%d)", targetX, targetY);
        // log(buffer);
        
        // Highlight target
        // API_setColor(targetX, targetY, 'R');
        
        // Move to the target
        moveToTarget(targetX, targetY);
        
        // sprintf(buffer, "Cells visited: %d/%d", cellsVisited, totalCells);
        // log(buffer);
    }
}

// Return to starting position
void returnToStart() {
    // log("Returning to start position...");
    moveToTarget(0, 0);
}

// Move to the center of the maze
void moveToCenter() {
    // log("Moving to center of the maze...");
    
    // Calculate center coordinates
    int centerX = mazeWidth / 2;
    int centerY = mazeHeight / 2;
    
    // If even dimensions, adjust center
    if (mazeWidth % 2 == 0) centerX--;
    if (mazeHeight % 2 == 0) centerY--;
    
    // Clear previous colors
    // API_clearAllColor();
    
    // Mark start and center
    // API_setColor(0, 0, 'G');
    // API_setColor(centerX, centerY, 'R');
    
    // Move to center
    moveToTarget(centerX, centerY);
    
    // log("Center reached!");
}

// Display current state of flood array as text
// void displayFloodArray() {
//     char text[4];
//     for (int i = 0; i < mazeWidth; i++) {
//         for (int j = 0; j < mazeHeight; j++) {
//             if (floodArray[i][j] != UNEXPLORED) {
//                 sprintf(text, "%d", floodArray[i][j]);
//                 API_setText(i, j, text);
//             }
//         }
//     }
// }

void loop() {
    // log("Starting Micromouse Maze Mapping Algorithm");
    
    // Initialize maze data structures
    initializeMaze();
    
    // First phase: Full exploration of the maze
    // log("Phase 1: Maze Exploration");
    exploreMaze();
    
    // Second phase: Return to start
    // log("Phase 2: Returning to Start");
    returnToStart();
    
    // Third phase: Optimal path to center
    // log("Phase 3: Optimal Path to Center");
    moveToCenter();
    
    // log("Maze mapping and solving complete!");
    
}
