#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_SIZE 100

// Maze characters
#define WALL '#'
#define PATH ' '
#define START 'S'
#define END 'E'
#define VISITED '.'
#define SOLUTION '*'

// Direction arrays
int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

char maze[MAX_SIZE][MAX_SIZE];
char solution[MAX_SIZE][MAX_SIZE];
int rows, cols;

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point* items;
    int front, rear, size;
} Queue;

bool is_valid(int x, int y) {
    return x >= 0 && x < rows && y >= 0 && y < cols;
}

void print_maze() {
    printf("\033[H\033[J");  // Clear screen
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (solution[i][j] == SOLUTION) {
                printf("\033[1;32m*\033[0m");  // Green * for solution path
            } else if (maze[i][j] == WALL) {
                printf("\033[1;37m#\033[0m");  // White # for walls
            } else if (maze[i][j] == START) {
                printf("\033[1;34mS\033[0m");  // Blue S for start
            } else if (maze[i][j] == END) {
                printf("\033[1;31mE\033[0m");  // Red E for end
            } else if (maze[i][j] == VISITED) {
                printf("\033[1;33m.\033[0m");  // Yellow . for visited
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    fflush(stdout);
    usleep(100000);  // Sleep for 100ms
}

bool dfs(int x, int y) {
    if (!is_valid(x, y) || maze[x][y] == WALL || maze[x][y] == VISITED) {
        return false;
    }

    if (maze[x][y] == END) {
        solution[x][y] = SOLUTION;
        return true;
    }

    maze[x][y] = VISITED;
    solution[x][y] = SOLUTION;
    print_maze();

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (dfs(nx, ny)) {
            return true;
        }
    }

    solution[x][y] = PATH;
    print_maze();
    return false;
}

Queue* create_queue(int capacity) {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    queue->items = (Point*)malloc(capacity * sizeof(Point));
    queue->front = queue->rear = -1;
    queue->size = 0;
    return queue;
}

void enqueue(Queue* queue, Point item) {
    queue->rear = (queue->rear + 1) % (rows * cols);
    queue->items[queue->rear] = item;
    if (queue->front == -1) queue->front = 0;
    queue->size++;
}

Point dequeue(Queue* queue) {
    Point item = queue->items[queue->front];
    queue->front = (queue->front + 1) % (rows * cols);
    queue->size--;
    return item;
}

bool bfs() {
    Queue* queue = create_queue(rows * cols);
    Point start = {-1, -1};

    // Find the starting position
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (maze[i][j] == START) {
                start.x = i;
                start.y = j;
                break;
            }
        }
        if (start.x != -1) break;
    }

    if (start.x == -1) {
        printf("No starting position found.\n");
        return false;
    }

    enqueue(queue, start);
    Point parent[MAX_SIZE][MAX_SIZE] = {{{-1, -1}}};

    while (queue->size > 0) {
        Point current = dequeue(queue);
        
        if (maze[current.x][current.y] == END) {
            // Reconstruct path
            while (current.x != start.x || current.y != start.y) {
                solution[current.x][current.y] = SOLUTION;
                current = parent[current.x][current.y];
            }
            solution[start.x][start.y] = SOLUTION;
            free(queue->items);
            free(queue);
            return true;
        }

        maze[current.x][current.y] = VISITED;
        print_maze();

        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];
            if (is_valid(nx, ny) && (maze[nx][ny] == PATH || maze[nx][ny] == END)) {
                enqueue(queue, (Point){nx, ny});
                parent[nx][ny] = current;
            }
        }
    }

    free(queue->items);
    free(queue);
    return false;
}

void generate_maze() {
    srand(time(NULL));

    // Initialize maze with walls
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            maze[i][j] = WALL;
        }
    }

    // Start from the top-left corner
    int x = 1, y = 1;
    maze[x][y] = PATH;

    // Array to store unvisited neighbors
    Point stack[MAX_SIZE * MAX_SIZE];
    int top = 0;

    while (true) {
        Point neighbors[4];
        int count = 0;

        // Check unvisited neighbors
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i] * 2;
            int ny = y + dy[i] * 2;
            if (is_valid(nx, ny) && maze[nx][ny] == WALL) {
                neighbors[count++] = (Point){nx, ny};
            }
        }

        if (count > 0) {
            // Choose a random unvisited neighbor
            int index = rand() % count;
            int nx = neighbors[index].x;
            int ny = neighbors[index].y;

            // Mark the path
            maze[(x + nx) / 2][(y + ny) / 2] = PATH;
            maze[nx][ny] = PATH;

            // Push current cell to stack
            stack[top++] = (Point){x, y};

            // Move to the chosen cell
            x = nx;
            y = ny;
        } else if (top > 0) {
            // Backtrack
            Point p = stack[--top];
            x = p.x;
            y = p.y;
        } else {
            break;
        }
    }

    // Place start and end points
    maze[1][1] = START;
    maze[rows - 2][cols - 2] = END;
}

bool validate_input() {
    int start_count = 0, end_count = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            char c = maze[i][j];
            if (c != WALL && c != PATH && c != START && c != END) {                           //validates input
                printf("Invalid character '%c' at position (%d, %d)\n", c, i, j);
                return false;
            }
            if (c == START) start_count++;
            if (c == END) end_count++;
        }
    }

    if (start_count != 1) {
        printf("There must be exactly one start point. Found: %d\n", start_count);
        return false;
    }
    if (end_count != 1) {
        printf("There must be exactly one end point. Found: %d\n", end_count);
        return false;
    }

    return true;
}

void solve_maze(bool use_dfs) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            solution[i][j] = maze[i][j];
        }
    }

    bool solved;
    if (use_dfs) {
        int start_x = -1, start_y = -1;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (maze[i][j] == START) {
                    start_x = i;
                    start_y = j;
                    break;
                }
            }
            if (start_x != -1) break;
        }
        solved = dfs(start_x, start_y);
    } else {
        solved = bfs();
    }

    if (solved) {
        printf("Maze solved! Path found:\n");
        print_maze();
    } else {
        printf("No path found.\n");
    }
}

int main() {
    int choice;
    printf("1. Input maze manually\n");
    printf("2. Generate random maze\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        printf("Enter the number of rows and columns: ");
        scanf("%d %d", &rows, &cols);

        printf("Enter the maze (use '#' for walls, ' ' for paths, 'S' for start, and 'E' for end):\n");
        for (int i = 0; i < rows; i++) {
            scanf("%s", maze[i]);
        }

        if (!validate_input()) {
            printf("Invalid maze input. Exiting.\n");
            return 1;
        }
    } else if (choice == 2) {
        printf("Enter the number of rows and columns for the generated maze: ");
        scanf("%d %d", &rows, &cols);
        generate_maze();
    } else {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    printf("Choose solving algorithm:\n");
    printf("1. Depth-First Search (DFS)\n");
    printf("2. Breadth-First Search (BFS)\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice != 1 && choice != 2) {
        printf("Invalid choice. Exiting.\n");
        return 1;
    }

    solve_maze(choice == 1);

    return 0;
}

