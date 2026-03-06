#include <Wire.h>    // For I2C communication (if sensors require it)
#include <queue>     // For BFS-based flood-fill
#include <utility>   // For std::pair in queue

// **Constants**
const int MAZE_SIZE = 16;
const int GOAL_X = 8;    // Center of 16x16 maze
const int GOAL_Y = 8;
const int START_X = 0;   // Starting position
const int START_Y = 0;

// **Pin Definitions (Adjust based on hardware)**
const int LEFT_IR_PIN = A0;   // Left IR sensor
const int FRONT_IR_PIN = A1;  // Front IR sensor
const int RIGHT_IR_PIN = A2;  // Right IR sensor
const int LEFT_MOTOR_PIN1 = 3; // PWM pins for left motor
const int LEFT_MOTOR_PIN2 = 4;
const int RIGHT_MOTOR_PIN1 = 5; // PWM pins for right motor
const int RIGHT_MOTOR_PIN2 = 6;

// **Calibration Constants (Tune these values experimentally)**
const int WALL_THRESHOLD = 500; // IR sensor threshold for wall detection
const int MOVE_DELAY = 200;     // Delay for moving forward (ms)
const int TURN_DELAY = 300;     // Delay for 90-degree turns (ms)

// **Global Variables**
int currentX = START_X;         // Current X position
int currentY = START_Y;         // Current Y position
int direction = 0;              // 0: North, 1: East, 2: South, 3: West
int maze[MAZE_SIZE][MAZE_SIZE]; // Flood-fill values (distance to target)
bool visited[MAZE_SIZE][MAZE_SIZE] = {false}; // Visited cells
bool eastWalls[MAZE_SIZE][MAZE_SIZE] = {false};  // Wall to the east of (x,y)
bool northWalls[MAZE_SIZE][MAZE_SIZE] = {false}; // Wall to the north of (x,y)
bool explorationComplete = false; // Flag for exploration phase

// **Setup Function**
void setup() {
    Serial.begin(9600);
    
    // Initialize pins
    pinMode(LEFT_IR_PIN, INPUT);
    pinMode(FRONT_IR_PIN, INPUT);
    pinMode(RIGHT_IR_PIN, INPUT);
    pinMode(LEFT_MOTOR_PIN1, OUTPUT);
    pinMode(LEFT_MOTOR_PIN2, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN1, OUTPUT);
    pinMode(RIGHT_MOTOR_PIN2, OUTPUT);

    // Initialize perimeter walls
    for (int y = 0; y < MAZE_SIZE; y++) {
        eastWalls[MAZE_SIZE - 1][y] = true;  // Right boundary
    }
    for (int x = 0; x < MAZE_SIZE; x++) {
        northWalls[x][MAZE_SIZE - 1] = true; // Top boundary
    }
    // Left (x=0) and bottom (y=0) walls are implied by boundary checks

    // Mark starting position as visited
    visited[START_X][START_Y] = true;
}

// **Motor Control Functions**
void moveForward() {
    // Left motor forward
    analogWrite(LEFT_MOTOR_PIN1, 255);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);
    // Right motor forward
    analogWrite(RIGHT_MOTOR_PIN1, 255);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
    delay(MOVE_DELAY); // Adjust delay based on speed and distance
    stopMotors();
    updatePosition();
    visited[currentX][currentY] = true;
}

void turnLeft() {
    // Left motor backward
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    analogWrite(LEFT_MOTOR_PIN2, 255);
    // Right motor forward
    analogWrite(RIGHT_MOTOR_PIN1, 255);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
    delay(TURN_DELAY); // Adjust for 90-degree turn
    stopMotors();
    direction = (direction + 3) % 4; // Update direction counterclockwise
}

void turnRight() {
    // Left motor forward
    analogWrite(LEFT_MOTOR_PIN1, 255);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);
    // Right motor backward
    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    analogWrite(RIGHT_MOTOR_PIN2, 255);
    delay(TURN_DELAY); // Adjust for 90-degree turn
    stopMotors();
    direction = (direction + 1) % 4; // Update direction clockwise
}

void stopMotors() {
    digitalWrite(LEFT_MOTOR_PIN1, LOW);
    digitalWrite(LEFT_MOTOR_PIN2, LOW);
    digitalWrite(RIGHT_MOTOR_PIN1, LOW);
    digitalWrite(RIGHT_MOTOR_PIN2, LOW);
}

// **Position Update**
void updatePosition() {
    if (direction == 0) currentY++;      // North
    else if (direction == 1) currentX++; // East
    else if (direction == 2) currentY--; // South
    else if (direction == 3) currentX--; // West
}

// **Sensor Reading**
bool isWallDetected(int pin) {
    return analogRead(pin) > WALL_THRESHOLD; // Adjust threshold
}

// **Wall Updating**
void updateWalls() {
    // Update walls based on sensor readings and current orientation
    if (isWallDetected(FRONT_IR_PIN)) {
        if (direction == 0 && currentY < MAZE_SIZE - 1) northWalls[currentX][currentY] = true;
        else if (direction == 1 && currentX < MAZE_SIZE - 1) eastWalls[currentX][currentY] = true;
        else if (direction == 2 && currentY > 0) northWalls[currentX][currentY - 1] = true;
        else if (direction == 3 && currentX > 0) eastWalls[currentX - 1][currentY] = true;
    }
    if (isWallDetected(LEFT_IR_PIN)) {
        if (direction == 0 && currentX > 0) eastWalls[currentX - 1][currentY] = true;
        else if (direction == 1 && currentY < MAZE_SIZE - 1) northWalls[currentX][currentY] = true;
        else if (direction == 2 && currentX < MAZE_SIZE - 1) eastWalls[currentX][currentY] = true;
        else if (direction == 3 && currentY > 0) northWalls[currentX][currentY - 1] = true;
    }
    if (isWallDetected(RIGHT_IR_PIN)) {
        if (direction == 0 && currentX < MAZE_SIZE - 1) eastWalls[currentX][currentY] = true;
        else if (direction == 1 && currentY > 0) northWalls[currentX][currentY - 1] = true;
        else if (direction == 2 && currentX > 0) eastWalls[currentX - 1][currentY] = true;
        else if (direction == 3 && currentY < MAZE_SIZE - 1) northWalls[currentX][currentY] = true;
    }
}

// **Check if Movement is Possible**
bool canMove(int dir) {
    if (dir == 0) { // North
        return currentY < MAZE_SIZE - 1 && !northWalls[currentX][currentY];
    } else if (dir == 1) { // East
        return currentX < MAZE_SIZE - 1 && !eastWalls[currentX][currentY];
    } else if (dir == 2) { // South
        return currentY > 0 && !northWalls[currentX][currentY - 1];
    } else if (dir == 3) { // West
        return currentX > 0 && !eastWalls[currentX - 1][currentY];
    }
    return false;
}

// **Flood-Fill Algorithm**
void floodFillAlgorithm(int targetX, int targetY) {
    // Initialize maze with large values
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            maze[i][j] = MAZE_SIZE * MAZE_SIZE;
        }
    }
    maze[targetX][targetY] = 0;

    // BFS with queue
    std::queue<std::pair<int, int>> q;
    q.push({targetX, targetY});

    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();
        int dist = maze[x][y];

        // Check all four neighbors
        if (y < MAZE_SIZE - 1 && !northWalls[x][y] && maze[x][y + 1] > dist + 1) {
            maze[x][y + 1] = dist + 1;
            q.push({x, y + 1});
        }
        if (x < MAZE_SIZE - 1 && !eastWalls[x][y] && maze[x + 1][y] > dist + 1) {
            maze[x + 1][y] = dist + 1;
            q.push({x + 1, y});
        }
        if (y > 0 && !northWalls[x][y - 1] && maze[x][y - 1] > dist + 1) {
            maze[x][y - 1] = dist + 1;
            q.push({x, y - 1});
        }
        if (x > 0 && !eastWalls[x - 1][y] && maze[x - 1][y] > dist + 1) {
            maze[x - 1][y] = dist + 1;
            q.push({x - 1, y});
        }
    }
}

// **Exploration Phase**
void exploreMaze() {
    while (!(currentX == GOAL_X && currentY == GOAL_Y)) {
        // Update wall information
        updateWalls();

        // Update flood-fill values
        floodFillAlgorithm(GOAL_X, GOAL_Y);

        // Determine possible moves: forward, left, right
        int possibleDirs[3] = {direction, (direction + 3) % 4, (direction + 1) % 4};
        int bestDir = -1;
        int minValue = MAZE_SIZE * MAZE_SIZE;
        bool preferUnvisited = false;

        for (int i = 0; i < 3; i++) {
            int dir = possibleDirs[i];
            int nextX = currentX;
            int nextY = currentY;
            if (dir == 0) nextY++;
            else if (dir == 1) nextX++;
            else if (dir == 2) nextY--;
            else if (dir == 3) nextX--;
            if (nextX < 0 || nextX >= MAZE_SIZE || nextY < 0 || nextY >= MAZE_SIZE) continue;

            if (canMove(dir)) {
                int value = maze[nextX][nextY];
                if (!visited[nextX][nextY]) {
                    if (value <= minValue) {
                        minValue = value;
                        bestDir = dir;
                        preferUnvisited = true;
                    }
                } else if (!preferUnvisited && value < minValue) {
                    minValue = value;
                    bestDir = dir;
                }
            }
        }

        if (bestDir == -1) {
            Serial.println("Stuck during exploration!");
            // Attempt to backtrack or turn
            turnRight(); // Simple recovery: turn and try again
            continue;
        }

        // Turn to face bestDir
        while (direction != bestDir) {
            if ((direction + 1) % 4 == bestDir) turnRight();
            else turnLeft();
        }
        moveForward();
    }
    Serial.println("Reached goal during exploration!");
}

// **Return to Start**
void returnToStart() {
    floodFillAlgorithm(START_X, START_Y);
    while (!(currentX == START_X && currentY == START_Y)) {
        int nextX = currentX;
        int nextY = currentY;
        int minValue = maze[currentX][currentY];
        int nextDir = direction;

        // Check all four directions
        if (currentY < MAZE_SIZE - 1 && !northWalls[currentX][currentY] && maze[currentX][currentY + 1] < minValue) {
            minValue = maze[currentX][currentY + 1];
            nextX = currentX;
            nextY = currentY + 1;
            nextDir = 0;
        }
        if (currentX < MAZE_SIZE - 1 && !eastWalls[currentX][currentY] && maze[currentX + 1][currentY] < minValue) {
            minValue = maze[currentX + 1][currentY];
            nextX = currentX + 1;
            nextY = currentY;
            nextDir = 1;
        }
        if (currentY > 0 && !northWalls[currentX][currentY - 1] && maze[currentX][currentY - 1] < minValue) {
            minValue = maze[currentX][currentY - 1];
            nextX = currentX;
            nextY = currentY - 1;
            nextDir = 2;
        }
        if (currentX > 0 && !eastWalls[currentX - 1][currentY] && maze[currentX - 1][currentY] < minValue) {
            minValue = maze[currentX - 1][currentY];
            nextX = currentX - 1;
            nextY = currentY;
            nextDir = 3;
        }

        if (minValue >= maze[currentX][currentY]) {
            Serial.println("Stuck while returning to start!");
            break;
        }

        // Turn to face nextDir
        while (direction != nextDir) {
            if ((direction + 1) % 4 == nextDir) turnRight();
            else turnLeft();
        }
        moveForward();
    }
    Serial.println("Returned to start!");
}

// **Traverse Shortest Path**
void traverseShortestPath() {
    floodFillAlgorithm(GOAL_X, GOAL_Y);
    while (!(currentX == GOAL_X && currentY == GOAL_Y)) {
        int nextX = currentX;
        int nextY = currentY;
        int minValue = maze[currentX][currentY];
        int nextDir = direction;

        // Check all four directions
        if (currentY < MAZE_SIZE - 1 && !northWalls[currentX][currentY] && maze[currentX][currentY + 1] < minValue) {
            minValue = maze[currentX][currentY + 1];
            nextX = currentX;
            nextY = currentY + 1;
            nextDir = 0;
        }
        if (currentX < MAZE_SIZE - 1 && !eastWalls[currentX][currentY] && maze[currentX + 1][currentY] < minValue) {
            minValue = maze[currentX + 1][currentY];
            nextX = currentX + 1;
            nextY = currentY;
            nextDir = 1;
        }
        if (currentY > 0 && !northWalls[currentX][currentY - 1] && maze[currentX][currentY - 1] < minValue) {
            minValue = maze[currentX][currentY - 1];
            nextX = currentX;
            nextY = currentY - 1;
            nextDir = 2;
        }
        if (currentX > 0 && !eastWalls[currentX - 1][currentY] && maze[currentX - 1][currentY] < minValue) {
            minValue = maze[currentX - 1][currentY];
            nextX = currentX - 1;
            nextY = currentY;
            nextDir = 3;
        }

        if (minValue >= maze[currentX][currentY]) {
            Serial.println("Stuck on shortest path!");
            break;
        }

        // Turn to face nextDir
        while (direction != nextDir) {
            if ((direction + 1) % 4 == nextDir) turnRight();
            else turnLeft();
        }
        moveForward();
    }
    Serial.println("Goal reached via shortest path!");
    while (true) delay(1000); // Stop after reaching goal
}

// **Main Loop**
void loop() {
    if (!explorationComplete) {
        exploreMaze();
        returnToStart();
        explorationComplete = true;
    } else {
        traverseShortestPath();
    }
}
