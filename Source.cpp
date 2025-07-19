#include "raylib.h"
#include "rcamera.h"
#include <time.h>
#include <vector>
#include <string>

//vector for tasks. 12 months, 31 days
std::vector<std::string> tasks[12][31];

//Possible screens
enum Screen { MONTH_VIEW, TASK_ENTRY };
Screen currentScreen = MONTH_VIEW;

struct PlannerState {
    int currentYear = 2025;
    int currentMonth = 0;
    int selectedDay = -1;
    int selectedTaskIndex = -1;
    enum Screen { MONTH_VIEW, TASK_ENTRY } currentScreen = MONTH_VIEW;
};
int selectedTaskIndex = -1;

// Declare the instance globally
PlannerState planner;

//Year, day, week, and month values
int currentYear = 2025;
int selectedDay = -1;
int currentMonth = 0;
int daysInWeek = 7;

//array of months
const char* monthNames[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September",
"October", "November", "December"};


//formula for differing days in months
int GetDaysInMonth(int year, int month) {
    if (month == 2) return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28;
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

bool isValidDay(int year, int month, int day) {
    if (month < 0 || month > 11) return false;
    if (day < 1 || day > GetDaysInMonth(year, month)) return false;
}

void LoadTasks() {
    FILE* file = nullptr;
    if (fopen_s(&file, "tasks.txt", "r") == 0) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            int year, month, day;
            char task[200];
            task[199] = '\0';  // Safer terminator
            if (sscanf_s(line, "%d,%d,%d,%[^\n]", &year, &month, &day, task, (unsigned)_countof(task)) == 4) {
                tasks[month][day - 1].push_back(std::string(task));
            }
        }
        fclose(file);
    }
}


void SaveTasks() {
    FILE* file = nullptr;
    if (fopen_s(&file, "tasks.txt", "w") == 0) {
        for (int m = 0; m < 12; m++) {
            for (int d = 0; d < 31; d++) {
                for (auto& task : tasks[m][d]) {
                    fprintf(file, "%d,%d,%d,%s\n", currentYear, m, d + 1, task.c_str());
                }
            }
        }
        fclose(file);
    }
}




int main(void)
{
    LoadTasks();
    //screen resolution
    const int screenWidth = 800;
    const int screenHeight = 450;

    //window name
    InitWindow(screenWidth, screenHeight, "Raylib Planner");

    //fps
    SetTargetFPS(60);     

    //task input with 128 bytes of memory allocated
    char taskInput[128] = "";
    bool enterPressed = false;

    //loop performed while window is open
    while (!WindowShouldClose())    
    {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        //Month selection arrow boxes position
        Rectangle leftArrow = { 350, 10, 30, 30 };
        Rectangle rightArrow = { 400, 10, 30, 30 };

        //Draw Month selection arrows within boxes
        DrawRectangleRec(leftArrow, RED);
        DrawText("<", leftArrow.x + 8, leftArrow.y + 5, 20, WHITE);
        DrawRectangleRec(rightArrow, RED);
        DrawText(">", rightArrow.x + 8, rightArrow.y + 5, 20, WHITE);

        int key;
        while ((key = GetCharPressed()) > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(taskInput) < 127)) {
                int len = strlen(taskInput);
                taskInput[len] = (char)key;
                taskInput[len + 1] = '\0';
            }
        }
       //Month Screen
        if (currentScreen == MONTH_VIEW) {
         DrawText(monthNames[currentMonth], 350, 40, 20, DARKGRAY); 
         
        if (CheckCollisionPointRec(GetMousePosition(), leftArrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentMonth = (currentMonth - 1 + 12) % 12;
        }

        if (CheckCollisionPointRec(GetMousePosition(), rightArrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            currentMonth = (currentMonth + 1) % 12;
        }

        int daysInMonth = GetDaysInMonth(currentYear, currentMonth);
            for (int i = 0; i < daysInMonth; i++) {
                int row = i / 7;
                int col = i % 7;
                Rectangle dayBox = { 50 + col * 100, 80 + row * 60, 80, 50 };
                DrawRectangleRec(dayBox, BLUE);
                DrawText(TextFormat("%d", i + 1), dayBox.x + 30, dayBox.y + 15, 20, BLACK);

                if (!tasks[currentMonth][i].empty()) {
                    DrawCircle(dayBox.x + 40, dayBox.y + 10, 5, RED);
            }

        if (CheckCollisionPointRec(GetMousePosition(), dayBox) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selectedDay = i + 1;
                    currentScreen = TASK_ENTRY;
                }
            }
            DrawText("Select a day to add tasks", 50, 30, 20, DARKGRAY);
        }
        //Tesk Entry Screen
        else if (currentScreen == TASK_ENTRY) {
            DrawText(TextFormat("Tasks for Day %d", selectedDay), 50, 50, 20, BLACK);

            DrawText(taskInput, 50, 100, 30, BLACK);
            DrawText("Type Task and press Enter", 50, 130, 20, DARKGRAY);

           
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(taskInput);
                if (len > 0) taskInput[len - 1] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && strlen(taskInput) > 0) {
            tasks[currentMonth][selectedDay-1].push_back(std::string(taskInput));
            taskInput[0] = '\0';
            }

            Rectangle backButton = { 50, 400, 100, 30 };
            DrawRectangleRec(backButton, GRAY);
            DrawText("Back", backButton.x + 30, backButton.y + 5, 20, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentScreen = MONTH_VIEW;
            }


            int y = 150;
            Vector2 mousePos = GetMousePosition();
            auto& dayTasks = tasks[currentMonth][selectedDay - 1];

            for (int i = 0; i < dayTasks.size(); /* nothing here */) {
                Rectangle taskBox = { 45, y - 5, 400, 25 };
                bool isHovered = CheckCollisionPointRec(mousePos, taskBox);

                if (isHovered) {
                    DrawRectangleRec(taskBox, LIGHTGRAY);
                }

                DrawText(dayTasks[i].c_str(), 50, y, 20, DARKGRAY);

                if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    dayTasks.erase(dayTasks.begin() + i);  // Don't increment i if erased
                    continue;  // Skip increment and rendering for removed task
                }

                y += 30;
                i++;  // Only increment if no deletion occurred

            }

        }

        EndDrawing();

    }
    SaveTasks();
    CloseWindow();


    return 0;
}