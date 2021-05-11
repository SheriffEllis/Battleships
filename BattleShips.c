/*
ACS130, Individual C Assignment 3
Written by: Sherif El Sawwaf
Completed:
Purpose:
*/

#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

// Struct used to keep track of positions on the board
struct Coord{
    int x;
    int y;
};

// Struct that acts as a modified linked list to keep track of each piece of a boat on the board
struct BoatSegment{
    struct BoatSegment *head;
    struct BoatSegment *next;

    char boat_type;
    struct Coord position;
    int isHit;
};

//
struct Board{
    char hits[10][10];
    struct BoatSegment *boats[10][10];
};

//
struct AiData{
    enum game_difficulty {easy, normal, hard} difficulty;
    enum search_mode {search, destroy} mode; // AI is in search mode by default
    struct Coord lastSucHit; // Position of last successful hit
};

//
enum battleships{
    /*
    Aircraft Carrier:
    AAAAA

    Battleship:
    BBBB

    Cruiser:
    CCC

    Submarine:
    SSS

    Destroyer:
    DD
    */
    aircraft_carrier = 5,
    battleship = 4,
    cruiser = 3,
    submarine = 3,
    destroyer = 2
};

void initialiseBoard(struct Board *board, int playerInput);
void displayBoard(struct Board board, int obfuscate);
void displayEntireBoard(struct Board player_board, struct Board ai_board);
char strike(struct Board *board, struct Coord pos);
void aiMove(struct Board *player_board, struct AiData *ai_data);
void playerMove(struct Board *ai_board);
int checkWin(struct Board *board);

void displaySaves();
int savesAvailable();
int loadGame(const char *filename, struct Board *player_board, struct Board *ai_board, struct AiData *ai_data);
void saveGame(const char *filename, struct Board *player_board, struct Board *ai_board, struct AiData *ai_data);


void main() {
    int repeat = 1;
    while(repeat){
        printf("Battleships!\n\n");
        struct AiData ai_data;
        struct Board player_board;
        struct Board ai_board;

        char response;
        // Only ask to load saves if saves available otherwise default to no
        if(savesAvailable()){
            printf("Would you like to load a saved game? (y/n): ");
            fflush(stdin);
            scanf("%c", &response);
        }else{
            response = 'n'; // No
        }

        int valid = 0; // Used to check user input in while loop
        if(tolower(response) == 'y'){ // 1 if response y or Y, 0 otherwise
            do{
                printf("Choose a save to load (type its name into the console):\n");
                displaySaves();
                printf("\n");
                char str[50];
                fflush(stdin);
                gets(str);
                // TODO: add .txt to entered string
                valid = loadGame(str, &player_board, &ai_board, &ai_data); // Returns 1 on successful load
                if(!valid){
                    printf("Please type a valid save name\n");
                }
            }while(!valid);
        }else{
            ai_data.mode = search; // AI initially set to search mode

            do{
                printf("Choose a game difficulty from 0 to 2:\n0: Easy\n1: Normal\n2: Hard\n");
                scanf("%d", &ai_data.difficulty);
                valid = ai_data.difficulty >= 0 && ai_data.difficulty <= 2;
                if(!valid){
                    printf("\nPlease choose a number from 0 to 2\n");
                }
            }while(!valid);

            initialiseBoard(&player_board, 1);
            initialiseBoard(&ai_board, 0);
        }

        int winner = 0; // 0: No winner, 1: Player wins, 2: AI wins
        while(!winner){ // Game loop continues until there is a winner
            displayEntireBoard(player_board, ai_board);
            playerMove(&ai_board);
            if(checkWin(&ai_board)){ // If player has sunk all ships on AI board...
                winner = 1; // Player wins
            }

            displayEntireBoard(player_board, ai_board);
            aiMove(&player_board, &ai_data);
            if(checkWin(&player_board)){ // If AI has sunk all ships on player board...
                winner = 2; // AI wins
            }
            // TODO
            // Ask to save
        }

        // Ask if player wishes to play again
        printf("Would you like to play again? (y/n): ");
        fflush(stdin);
        scanf("%c", &response);
        // 1 if user input y or Y, 0 otherwise
        repeat = (tolower(response) == 'y');
    }
}

// Set up board with ships before game begins. When setting AI's board, playerInput = 0
// TODO
void initialiseBoard(struct Board *board, int playerInput){
};

// Displays player or AI board to console. When obfuscate = 1, (on AI's board) the positions of ships are hidden
// TODO
void displayBoard(struct Board board, int obfuscate){
};

// Displays both boards to console making use of displayBoard() function twice
void displayEntireBoard(struct Board player_board, struct Board ai_board){
    printf("AI board:\n");
    displayBoard(ai_board, 1);
    printf("\n\n--------------------------------------------\n\n");
    printf("Your board:\n");
    displayBoard(player_board, 0);
};

//
// TODO
char strike(struct Board *board, struct Coord pos){
};

//
// TODO
void aiMove(struct Board *player_board, struct AiData *ai_data){
};

//
// TODO
void playerMove(struct Board *aiBoard){
};

//
// TODO
int checkWin(struct Board *board){
};

//
// TODO: remove .txt from displayed name
void displaySaves(){
    DIR *d;
    struct dirent *dir; // Imported directory struct for finding all files in a folder
    d = opendir("."); // Open the current folder
    if (d) // If there is anything in the current folder
    {
        while ((dir = readdir(d)) != NULL) // Read out files until end of list reached
        {
            char *dot = strrchr(dir->d_name, '.'); // Find rightmost . in filename and create substring from there using pointer
            if (dot && !strcmp(dot, ".txt")){ // If file type is .txt, display to screen
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
};

// Similar to displaySaves() except it returns 1 on finding a .txt file and does not display the files
int savesAvailable(){
    DIR *d;
    struct dirent *dir; // Imported directory struct for finding all files in a folder
    d = opendir("."); // Open the current folder

    int isSave = 0; // flag to check for any .txt files
    if (d) // If there is anything in the current folder
    {
        while ((dir = readdir(d)) != NULL && !isSave) // Read out files until end of list reached or .txt file found
        {
            char *dot = strrchr(dir->d_name, '.'); // Find rightmost . in filename and create substring from there using pointer
            if (dot && !strcmp(dot, ".txt")){ // If file type is .txt, return 1
                isSave = 1;
            }
        }
        closedir(d);
    }
    return isSave;
};

//
// TODO
int loadGame(const char *filename, struct Board *player_board, struct Board *ai_board, struct AiData *ai_data){
    return 0;
};

//
// TODO
void saveGame(const char *filename, struct Board *player_board, struct Board *ai_board, struct AiData *ai_data){
};
