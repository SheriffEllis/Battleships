/*
ACS130, Individual C Assignment 3
Written by: Sherif El Sawwaf
Completed:
Purpose:
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <dirent.h>

// Struct used to keep track of positions on the board.
struct Coord{
    int x;
    int y;
};

// Struct that acts as a modified linked list to keep track of each piece of a boat on the board.
struct BoatSegment{
    struct BoatSegment *head; // Point to BoatSegment representing head of the ship
    struct BoatSegment *next; // Point to next BoatSegment

    char ship_type;
    struct Coord position;
    int is_hit; // Represents which BoatSegment has been struck by enemy player
    int is_sunk; // Represents if all BoatSegments in the the ship have been struck
    int is_null; // Represents if BoatSegment is part of a ship or an empty space
};

/*
Struct that contains 2 2d arrays. One with a visual of the hit ships for the opposing player
and the other with the BoatSegment structs that represent the battleships.

hits/boats[y][x]
 --->  x
|
V
y

*/
struct Board{
    char hits[10][10];
    struct BoatSegment boats[10][10];
};

//
struct AiData{
    enum game_difficulty {easy, normal, hard} difficulty;
    enum search_mode {search, destroy} mode; // AI is in search mode by default
    struct Coord lastSucHit; // Position of last successful hit
};

//
enum Direction{
    up, right, down, left
};

int randRange(int, int);

int shipCharToSize(char);
char * shipCharToName(char);
void initialiseBoard(struct Board *, int);
void placeShip(struct Board *, struct Coord, enum Direction, char);
int checkCollision(struct Board, struct Coord, enum Direction, int);

struct Coord userInputShipPosition(struct Board, int);
struct Coord userInputStrikePosition(struct Board);
enum Direction userInputDirection(struct Board, struct Coord, int);
void AIChooseShipPosAndDir(struct Board, int, struct Coord *, enum Direction *);

void displayBoard(struct Board, int);
void displayEntireBoard(struct Board, struct Board);

char strike(struct Board *, struct Coord, int *);
void aiMove(struct Board *, struct AiData *);
void playerMove(struct Board *);
int checkWin(struct Board);

void displaySaves();
int savesAvailable();
int loadGame(const char *, struct Board *, struct Board *, struct AiData *);
void saveGame(const char *, struct Board, struct Board, struct AiData);

void main() {
    srand(time(0)); // Seed pseudorandom number generator with current time
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
                char *str;
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
        while(winner == 0){ // Game loop continues until there is a winner
            displayEntireBoard(player_board, ai_board);
            playerMove(&ai_board);
            if(checkWin(ai_board)){ // If player has sunk all ships on AI board...
                winner = 1; // Player wins
            }else{
                //displayEntireBoard(player_board, ai_board);
                aiMove(&player_board, &ai_data);
                if(checkWin(player_board)){ // If AI has sunk all ships on player board...
                    winner = 2; // AI wins
                }
                // TODO
                // Ask to save
            }            
        }

        // Ask if player wishes to play again
        printf("Would you like to play again? (y/n): ");
        fflush(stdin);
        scanf("%c", &response);
        // 1 if user input y or Y, 0 otherwise
        repeat = (tolower(response) == 'y');
    }
}



// Output a random number from min to max
int randRange(int min, int max){
    return (rand() % (max - min + 1)) + min;
}



// Simple function to output the size of a ship based on it's type character.
int shipCharToSize(char ship_type){
    switch(toupper(ship_type)){
        case 'A': // Aircraft Carrier: AAAAA
            return 5;
        case 'B': // Battleship: BBBB
            return 4;
        case 'C': // Cruiser: CCC
            return 3;
        case 'S': // Submarine: SSS
            return 3;
        case 'D': // Destroyer: DD
            return 2;
    }
    return 0;
}

// Simple function to output the name of a ship (as a pointer to a string) based on it's type character.
char * shipCharToName(char ship_type){
    switch(toupper(ship_type)){
        case 'A': // Aircraft Carrier: AAAAA
            return "Aircraft Carrier";
        case 'B': // Battleship: BBBB
            return "Battleship";
        case 'C': // Cruiser: CCC
            return "Cruiser";
        case 'S': // Submarine: SSS
            return "Submarine";
        case 'D': // Destroyer: DD
            return "Destroyer";
    }
    return "";
}

// Set up board with ships before game begins. When setting AI's board, player_input = 0.
void initialiseBoard(struct Board *board_ptr, int player_input){
    // Set all points on the board to display unknown '?' visually and all BoatSegments to is_null initially
    for(int i=0; i<10; i++){
        for(int j=0; j<10; j++){
            board_ptr->hits[i][j] = '?';
            board_ptr->boats[i][j].is_null = 1;
            board_ptr->boats[i][j].is_hit = 0;
        }
    }

    // Placing ships on the board
    char ships[] = {'A', 'B', 'C', 'S', 'D'}; // Array of each ship type to reduce redundancy of placement routine
    if(player_input){ // Player manual placement of ships
        printf("\nPlacing your ships...\n\n");
        for(int ship_index = 0; ship_index < 5; ship_index++){
            char ship_type = ships[ship_index]; // Retrieve current ship type to place
            displayBoard(*board_ptr, 0);
            printf("\nShip to place: %s\n", shipCharToName(ship_type));
            printf("Size: %d (", shipCharToSize(ship_type));
            for(int i=0; i<shipCharToSize(ship_type); i++){ // Print appearance of ship using character and size
                printf("%c", ship_type);
            }
            printf(")\n");
            printf("Choose a position to place the ship's head\n");
            struct Coord position = userInputShipPosition(*board_ptr, shipCharToSize(ship_type)); // Retrieve ship position from user
            enum Direction direction = userInputDirection(*board_ptr, position, shipCharToSize(ship_type)); // Retrieve ship direction from user
            placeShip(board_ptr, position, direction, ship_type); // Place ship on game board
            printf("\n\n");
        }
    }else{ // AI random placement of ships
        for(int ship_index = 0; ship_index < 5; ship_index++){
            char ship_type = ships[ship_index]; // Retrieve current ship type to place
            struct Coord position;
            enum Direction direction;
            AIChooseShipPosAndDir(*board_ptr, shipCharToSize(ship_type), &position, &direction); // Choose a random position and direction
            placeShip(board_ptr, position, direction, ship_type); // Place ship on game board
        }
    }
}

// Place all BoatSegments of a ship using a starting position and direction for the ship to point
void placeShip(struct Board *board_ptr, struct Coord position, enum Direction direction, char ship_type){
    struct BoatSegment *boat_head_ptr = &(board_ptr->boats[position.y][position.x]);
    boat_head_ptr->position = position;
    boat_head_ptr->ship_type = ship_type;
    boat_head_ptr->head = boat_head_ptr;
    boat_head_ptr->next = NULL;
    boat_head_ptr->is_hit = 0;
    boat_head_ptr->is_sunk = 0;
    boat_head_ptr->is_null = 0;

    int ship_size = shipCharToSize(ship_type);

    
    for(int i = 1; i < ship_size; i++){
        int x = position.x +i*((direction==right) -(direction==left)); // Non-branching boolean maths to decide what direction to increment the coordinates
        int y = position.y +i*((direction==down) -(direction==up));

        struct BoatSegment *boat_segment_ptr = &(board_ptr->boats[y][x]);
        struct Coord segment_pos;
        segment_pos.x = x;
        segment_pos.y = y;
        boat_segment_ptr->position = segment_pos;
        boat_segment_ptr->ship_type = ship_type;
        boat_segment_ptr->head = boat_head_ptr;
        boat_segment_ptr->next = NULL;
        boat_segment_ptr->is_hit = 0;
        boat_segment_ptr->is_sunk = 0;
        boat_segment_ptr->is_null = 0;
        
        struct BoatSegment *prev_boat_segment_ptr; // Pointer to previous boat segment
        prev_boat_segment_ptr = &(board_ptr->boats[y+((direction==up) -(direction==down))][x+((direction==left) -(direction==right))]); // Boolean maths to decide direction of previous BoatSegment
        prev_boat_segment_ptr->next = boat_segment_ptr; // Give previous boat segment a pointer to current segment
    }
}

// Check if a ship placed in this location and direction collides with game wall or other ship.
int checkCollision(struct Board board, struct Coord position, enum Direction direction, int ship_size){
    switch(direction){
        case up:
            if(position.y - ship_size < 0){return 1;} // Ship goes off board
            for(int i = position.y; i >= position.y-ship_size; i--){
                if(!board.boats[i][position.x].is_null){ // Collides with other ship
                    return 1;
                }
            }
            break;
        case down:
            if(position.y + ship_size > 10){return 1;} // Ship goes off board
            for(int i = position.y; i <= position.y+ship_size; i++){
                if(!board.boats[i][position.x].is_null){ // Collides with other ship
                    return 1;
                }
            }
            break;
        case right:
            if(position.x + ship_size > 10){return 1;} // Ship goes off board
            for(int i = position.x; i <= position.x+ship_size; i++){
                if(!board.boats[position.y][i].is_null){ // Collides with other ship
                    return 1;
                }
            }
            break;
        case left:
            if(position.x - ship_size < 0){return 1;} // Ship goes off board
            for(int i = position.x; i >= position.x-ship_size; i--){
                if(!board.boats[position.y][i].is_null){ // Collides with other ship
                    return 1;
                }
            }
            break;
    }
    return 0; // If no collision detected, return false
}



// Take user input (with validation) of position to place a ship.
// Makes sure there is at least 1 direction to face with no collisions with walls or other ships.
struct Coord userInputShipPosition(struct Board board, int ship_size){
    printf("Select a position on the board by typing a letter and number like \"B4\"\n");
    printf("(The letter must be from A to J and the number from 1 to 10):\n");
    int valid; // Check for valid input
    struct Coord position;
    do{
        char letterY;
        int numX;
        fflush(stdin);
        scanf("%c%d", &letterY, &numX);
        letterY = toupper(letterY); // Board position letter must be in uppercase for consistency
        
        if(!((numX >= 1 && numX <= 10) && (letterY >= 'A' && letterY <= 'J'))){ // Number must be from 1 to 10 and letter from A to J
            printf("Error: Please type a letter from A to J and a number from 1 to 10 in the form \"A 10\":\n");
            valid = 0;
        }else{ // if the input is valid attempt to create a coordinate at that position
            int numY = (int)letterY - (int)'A'; // Convert from char to equivalent numeric board coordinate
            numX = numX -1; // 0 index numX

            position.x = numX;
            position.y = numY;

            // Check for if all directions collide with wall or ship
            int collides = 1; // true by default, check for at least 1 direction that does not collide
            for(int i=up; i <= left && collides; i++){ // Iterate through all 4 directions unless lack of collision found
                if(!checkCollision(board, position, i, ship_size)){collides = 0;} // If a direction that does not collide is found, collides = 0
            }
            if(collides){
                printf("Error: You cannot place a ship here as all directions will result in a collision, please type a different position\n");
            }
            valid = !collides;
        }
    }while(!valid);
    return position;
}

// Take user input (with validation) of position to strike on the board.
// Makes sure there is not already a strike in the given position.
struct Coord userInputStrikePosition(struct Board board){
    printf("Select a position on the board by typing a letter and number like \"B4\"\n");
    printf("(The letter must be from A to J and the number from 1 to 10):\n");
    int valid; // Check for valid input
    struct Coord position;
    do{
        char letterY;
        int numX;
        fflush(stdin);
        scanf("%c%d", &letterY, &numX);
        letterY = toupper(letterY); // Board position letter must be in uppercase for consistency
        
        if(!((numX >= 1 && numX <= 10) && (letterY >= 'A' && letterY <= 'J'))){ // Number must be from 1 to 10 and letter from A to J
            printf("Error: Please type a letter from A to J and a number from 1 to 10 in the form \"A 10\":\n");
            valid = 0;
        }else{ // if the input is valid attempt to create a coordinate at that position
            int numY = (int)letterY - (int)'A'; // Convert from char to equivalent numeric board coordinate
            numX = numX -1; // 0 index numX

            // Check if strike has already been made in this position
            if(valid = !board.boats[numY][numX].is_hit){ // valid if not already hit
                position.x = numX;
                position.y = numY;
            }else{
                printf("Error: You have already struck this position, please try another\n");
            }
        }
    }while(!valid);
    return position;
}

// Take user input (with validation) of direction to place a ship.
// Checks for collisions with walls or other ships.
enum Direction userInputDirection(struct Board board, struct Coord position, int ship_size){
    int valid;
    enum Direction direction;
    do{
        printf("Choose a direction for the ship to face (it cannot go through a wall or other ship)\n");
        printf("Input the direction as a number from 0 to 3\n");
        printf("0: up\n1: right\n2: down\n3: left\n");
        int dirNum; // Number representing direction
        fflush(stdin);
        scanf("%d", &dirNum);

        if(dirNum < 0 || dirNum > 3){ // Direction number must be from 0 to 3
            printf("Error: Please type a number/direction from 0 to 3\n");
            valid = 0;
        }else{
            direction = (enum Direction)dirNum; // Convert input number into direction
            if(checkCollision(board, position, direction, ship_size)){ // Make sure there is no collision with walls or other ships in chosen direction
                printf("Error: Cannot place ship in this direction, there is a collision\n");
                valid = 0;
            }else{
                valid = 1;
            }
        }
    }while(!valid);

    return direction;
}

// Chooses a random (valid) ship position and direction
void AIChooseShipPosAndDir(struct Board board, int ship_size, struct Coord *position_ptr, enum Direction *direction){
    int collides = 1; // true by default, check for at least 1 direction that does not collide
    while(collides){
        position_ptr->x = randRange(0, 9); // Choose a random position on the board
        position_ptr->y = randRange(0, 9);

        // Check for if all directions collide with wall or ship
        for(int i=up; i <= left && collides; i++){ // Iterate through all 4 directions unless lack of collision found
            if(!checkCollision(board, *position_ptr, i, ship_size)){
                collides = 0; // If a direction that does not collide is found, collides = 0
                *direction = i; // Output first valid direction
            }
        }
    }
}



// Displays player or AI board to console. When obfuscate = 1, (on AI's board) the positions of ships are hidden.
void displayBoard(struct Board board, int obfuscate){
    printf("    1  2  3  4  5  6  7  8  9  10\n");
    printf("    -----------------------------\n");
    for(int i=0; i<10; i++){
        printf("%c | ",(char)i + 65);
        for(int j=0; j<10; j++){
            char board_char;
            if(obfuscate){
                board_char = board.hits[i][j];
            }else if(board.boats[i][j].is_null){
                board_char = '-';
            }else if(board.boats[i][j].is_hit){
                board_char = tolower(board.boats[i][j].ship_type); // Indicate to player when part of their ship is hit
            }else{
                board_char = board.boats[i][j].ship_type;
            }
            printf("%c  ", board_char);
        }
        printf("\n");
    }
}

// Displays both boards to console making use of displayBoard() function twice.
void displayEntireBoard(struct Board player_board, struct Board ai_board){
    printf("AI board:\n");
    displayBoard(ai_board, 1);
    printf("\n\n--------------------------------------------\n\n");
    printf("Your board:\n");
    displayBoard(player_board, 0);
    printf("\n");
}



// Strikes ship at position on board, marking BoatSegment as hit. Check if all BoatSegments have been hit to decide if ship sunk.
// Returns '-' if no ship was hit at the location. is_sunk flag is set to true if ship was sunk on current hit.
char strike(struct Board *board_ptr, struct Coord position, int *is_sunk_ptr){
    board_ptr->boats[position.y][position.x].is_hit = 1;
    if(board_ptr->boats[position.y][position.x].is_null){ // If a blank space was hit, reveal 'X' and return no ship hit character
        board_ptr->hits[position.y][position.x] = '-';
        return '-';
    }// Since condition ends in return, no else needed
    board_ptr->hits[position.y][position.x] = 'X'; // Reveal that (anonymous) BoatSegment has been hit
    
    *is_sunk_ptr = 1; // Start with assumption that ship is sunk, if unhit segment found is sunk is set to false
    struct BoatSegment *boat_segment_ptr = board_ptr->boats[position.y][position.x].head; // Start from head of boat to check if sunk
    do{
        if(!boat_segment_ptr->is_hit){
            *is_sunk_ptr = 0;
        }
        boat_segment_ptr = boat_segment_ptr->next; // Iterate through linked list
    }while(boat_segment_ptr != NULL); // End when end of boat reached

    if(*is_sunk_ptr){ // If ship is sunk, update on board
        boat_segment_ptr = board_ptr->boats[position.y][position.x].head; // Start from head again
        do{
            boat_segment_ptr->is_sunk = 1; // Mark as sunk in BoatSegment array

            int x = boat_segment_ptr->position.x;
            int y = boat_segment_ptr->position.y;
            board_ptr->hits[y][x] = tolower(boat_segment_ptr->ship_type); // Mark as sunk in hit array by setting as lowercase version of ship character
            
            boat_segment_ptr = boat_segment_ptr->next; // Iterate through linked list
        }while(boat_segment_ptr != NULL); // End when end of boat reached
    }

    return board_ptr->boats[position.y][position.x].ship_type;
}

//
// TODO
void aiMove(struct Board *player_board_ptr, struct AiData *ai_data_ptr){
}

//
// TODO
void playerMove(struct Board *ai_board_ptr){
    printf("Choose a position on the AI board to strike\n");
    struct Coord position = userInputStrikePosition(*ai_board_ptr);
    int is_sunk;
    char struck_ship_type = strike(ai_board_ptr, position, &is_sunk);
    if(struck_ship_type == '-'){
        printf("\nYou MISSED!\n\n");
    }else if(is_sunk){
        printf("\nYou SUNK the AI's %s!\n\n", shipCharToName(struck_ship_type));
    }else{
        printf("\nYou HIT the AI's ship!\n\n");
    }
}

//
// TODO
int checkWin(struct Board board){
    return 0;
}



// Displays a list of all the .txt files in the program folder which can be used to retrieve save data
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
                *dot = '\0'; // Remove .txt from end of string by setting . to NULL which ends string
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
}

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
}

//
// TODO
int loadGame(const char *filename, struct Board *player_board_ptr, struct Board *ai_board_ptr, struct AiData *ai_data_ptr){
    return 0;
}

//
// TODO
void saveGame(const char *filename, struct Board player_board, struct Board ai_board, struct AiData ai_data){
}