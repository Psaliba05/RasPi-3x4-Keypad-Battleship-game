/*
    Battleship Game Client - mygame.cpp
    ------------------------------------
    This program implements a full two-player Battleship game.
    
    Each player generates a fleet on a 10x10 grid (using autogenFleet from genFleet.h)
    and maintains a view of the opponent’s grid (initially unknown).
    After the READY/START handshake with the server, the players alternate turns:
    
      - On your turn, you enter your shot’s X and Y coordinates.
        You may choose to use the keypad or standard keyboard.
        The shot is sent as: "PLAY,x,y\r\n".
      - The opponent processes the shot on their grid and replies with:
            "PLAY,RESULT,HIT\r\n"  if the shot hit a ship,
            "PLAY,RESULT,WIN\r\n"  if that shot sunk their final ship,
         or "PLAY,RESULT,MISS\r\n" if the shot missed.
      - After each shot, the current player grid and the opponent’s grid view are displayed.
      - In the display, on your own grid ships are shown as solid squares,
        hits as "X" and misses as "o".  
        On the opponent grid, unknown cells show as "?", misses appear as blank,
        and hits are shown as a white square (□, Unicode U+25A1).
      - The game ends when all ships of one player are sunk.
      
    Compilation:
      g++ -std=c++20 -o mygame mygame.cpp genFleet.cpp keypad.cpp -lwiringPi -lpthread
*/

#include "genFleet.h"    // Fleet generation functions and print routines
#include "keypad.h"      // Keypad interface (runs in the background)
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>   // for inet_addr()
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <csignal>
#include <exception>
#include <chrono>
#include <thread>
#include <sstream>
#include <limits>
#include <cctype>

using namespace std;

// Check if all ships in the grid are sunk.
// A cell is considered part of a ship if it is not " " and not marked as hit ("X") or miss ("o").
bool allShipsSunk(const string (&grid)[GRID_SIZE][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (grid[i][j] != " " && grid[i][j] != "X" && grid[i][j] != "o")
                return false;
        }
    }
    return true;
}

// Helper to parse a comma-delimited message.
string getFromBuffer(string& s_buff, string del) {
    string ret_str;
    size_t pos = s_buff.find(del);
    if (pos != string::npos) {
        ret_str = s_buff.substr(0, pos);
        s_buff.erase(0, pos + del.length());
    } else {
        ret_str = s_buff;
        s_buff = "";
    }
    return ret_str;
}

// Global flag and signal handler to gracefully exit.
volatile sig_atomic_t running = 1;
void signalHandler(int signum) {
    cout << "\nExiting... " << flush;
    running = 0;
}

// Function to get a coordinate value using either keypad or keyboard.
// The user is prompted to choose the input method.

/*
// New helper: Read a single digit from the keypad and validate it is between 0 and 9.
int getCoordinateFromKeypad(Keypad &kp, const string &prompt) {
    cout << prompt << flush;
    string digit = kp.get_digit();  // This call blocks until a key is pressed
    // Validate that digit is in the range "0" to "9"
    while (digit < "0" || digit > "9") {
        cout << "\nInvalid digit. Please press a key 0-9." << endl;
        cout << prompt << flush;
        digit = kp.get_digit();
    }
    cout << digit << endl;
    return stoi(digit);
}
*/


int getCoordinateFromKeypad(const string &prompt) {
 int coord;
        while (true) {
            cout << "Enter a number between 0 and 9: ";
            if (!(cin >> coord)) {
                cout << "Invalid input. Please enter a number between 0 and 9." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            if (coord < 0 || coord > 9) {
                cout << "Number must be between 0 and 9. Try again." << endl;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return coord;
        }
}


// Print the player's own grid.
// Ships (cells that are not " " and not hit/miss) are shown as a solid square (■),
// hits as "X", and misses as "o".
void printPlayerGrid(const string (&grid)[GRID_SIZE][GRID_SIZE]) {
    cout << "   ";
    for (int i = 0; i < GRID_SIZE; i++){
         cout << i << " ";
    }
    cout << "\n   ";
    for (int i = 0; i < GRID_SIZE; i++){
         cout << "--";
    }
    cout << "\n";
    for (int r = 0; r < GRID_SIZE; r++) {
       cout << r << "| ";
       for (int c = 0; c < GRID_SIZE; c++){
          string cell = grid[r][c];
          if(cell == "X") {
              cout << "X ";
          } else if(cell == "o") {
              cout << "o ";
          } else if(cell != " ") {
              cout << "\u25A0" << " ";  // Solid square for ship parts
          } else {
              cout << "  ";
          }
       }
       cout << "\n";
    }
}

// Print the opponent's grid.
// Unknown cells are shown as "?", misses as blank, and hits as a white square (□, Unicode U+25A1).
void printOpponentGrid(const string (&grid)[GRID_SIZE][GRID_SIZE]) {
    cout << "   ";
    for (int i = 0; i < GRID_SIZE; i++){
         cout << i << " ";
    }
    cout << "\n   ";
    for (int i = 0; i < GRID_SIZE; i++){
         cout << "--";
    }
    cout << "\n";
    for (int r = 0; r < GRID_SIZE; r++) {
       cout << r << "| ";
       for (int c = 0; c < GRID_SIZE; c++){
          string cell = grid[r][c];
          if(cell == "X") {
              cout << "\u25A0" << " ";  // White square for hits
          } else if(cell == "o") {
              cout << "  "; // Misses are blank
          } else {
              cout << "? ";
          }
       }
       cout << "\n";
    }
}

// Display both grids.
void displayGrids(const string (&myGrid)[GRID_SIZE][GRID_SIZE], const string (&oppGrid)[GRID_SIZE][GRID_SIZE]) {
    cout << "\nYour Grid:" << endl;
    printPlayerGrid(myGrid);
    cout << "\nOpponent Grid:" << endl;
    printOpponentGrid(oppGrid);
    cout << endl;
}

int main()
{
    // Setup Ctrl+C signal handler.
    signal(SIGINT, signalHandler);
    
    string server_ip;
    string user_name;
    string opponent = "";
    string player_pos;
    string game_id;
    
    // Set your game ID (must match between players).
    string my_game_id = "BattleshipGame";
    
    // Get player name and server IP.
    cout << "Enter your name > ";
    getline(cin, user_name);
    cout << "Enter server IP > ";
    getline(cin, server_ip);
    
    // Generate your own fleet on a 10x10 grid.
    string myMap[GRID_SIZE][GRID_SIZE];
    resetFleet(myMap);
    const unsigned short fleet[FLEET_COUNT] = {5,4,3,3,2,2,2};
    autogenFleet(myMap, fleet);
    
    // Create an opponent grid view (initially unknown).
    string oppMap[GRID_SIZE][GRID_SIZE];
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            oppMap[i][j] = " ";
        }
    }
    
    // Display your initial fleet.
    cout << "\nYour Fleet:" << endl;
    printPlayerGrid(myMap);
    
    // Setup the keypad (if connected).
    int colPins[3] = {21, 20, 16};
    int rowPins[4] = {19, 13, 6, 5};
    Keypad kp(colPins, rowPins);
    kp.run();
    
    // Create a TCP socket and connect to the server.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation error" << endl;
        return 1;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(10000);
    serverAddress.sin_addr.s_addr = inet_addr(server_ip.c_str());
    
    cout << "Connecting to server..." << flush;
    while (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) {
        if (!running) break;
        this_thread::sleep_for(chrono::seconds(5));
    }
    cout << "Connected!" << endl;
    
    // Send the READY command to the server.
    string readyMsg = "READY," + user_name + "," + my_game_id + "\r\n";
    send(sock, readyMsg.c_str(), readyMsg.length(), 0);
    cout << "Waiting to be paired..." << endl;
    
    // Wait for the START command.
    char buffer[1024] = {0};
    int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesReceived <= 0) {
        cerr << "Connection error while waiting for pairing." << endl;
        return 1;
    }
    string startMsg(buffer);
    // Expected format: "START,<position>,<opponent name>,<game id>\r\n"
    string token = getFromBuffer(startMsg, ","); // "START"
    player_pos = getFromBuffer(startMsg, ",");     // "1" or "2"
    opponent = getFromBuffer(startMsg, ",");         // opponent name
    game_id = getFromBuffer(startMsg, "\r");           // game id
    
    cout << "Paired with opponent: " << opponent << endl;
    // According to the protocol, player in position "1" starts first.
    bool myTurn = (player_pos == "1");
    if (myTurn) {
        cout << "You start first." << endl;
    } else {
        cout << "Opponent starts first. Wait for their move." << endl;
    }
    
    bool gameOver = false;
    while (!gameOver && running) {
        if (myTurn) {

            
            // Prompt for shot coordinates 
            cout << "Your turn. Enter shot coordinates:" << endl;
            int shotX, shotY;
            // Loop until a coordinate that hasn't been shot at is chosen.
            while (true) {
                shotX = getCoordinateFromKeypad(/*kp,*/ "Enter X coordinate (0-9): ");
                shotY = getCoordinateFromKeypad(/*kp,*/ "Enter Y coordinate (0-9): ");
                if (oppMap[shotY][shotX] != " ") {
                    cout << "You've already shot at (" << shotX << ", " << shotY << "). Please choose different coordinates." << endl;
                } else {
                    break;
                }
            }
            
            // Send the shot command: "PLAY,x,y\r\n"
            string shotCmd = "PLAY," + to_string(shotX) + "," + to_string(shotY) + "\r\n";
            send(sock, shotCmd.c_str(), shotCmd.length(), 0);
            cout << "Shot sent at (" << shotX << ", " << shotY << "). Waiting for result..." << endl;
            
            // Wait for the result from the opponent.
            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                cerr << "Connection error during shot result." << endl;
                break;
            }
            string resultMsg(buffer);
            // Expected response: "PLAY,RESULT,<result>\r\n"
            token = getFromBuffer(resultMsg, ","); // "PLAY"
            string secondToken = getFromBuffer(resultMsg, ","); // should be "RESULT"
            string result = "";
            if(secondToken == "RESULT"){
                result = getFromBuffer(resultMsg, "\r");
            }
            
            if (result == "HIT" || result == "WIN") {
                oppMap[shotY][shotX] = "X";
                cout << "Your shot hit the enemy ship!" << endl;
                if (result == "WIN") {
                    cout << "All enemy ships sunk. You win!" << endl;
                    gameOver = true;
                    displayGrids(myMap, oppMap);
                    break;
                }
                // A hit gives you another turn.
                myTurn = true;
            } else if (result == "MISS") {
                oppMap[shotY][shotX] = "o";
                cout << "Your shot missed." << endl;
                myTurn = false;
            }
            // Display grids after processing the shot.
            displayGrids(myMap, oppMap);
        } else {
            // Opponent's turn: wait for a shot command.
            cout << "Waiting for opponent's shot..." << endl;
            memset(buffer, 0, sizeof(buffer));
            bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                cerr << "Connection error while waiting for opponent's shot." << endl;
                break;
            }
            string shotMsg(buffer);
            // Expected format: "PLAY,x,y\r\n"
            token = getFromBuffer(shotMsg, ","); // "PLAY"
            string secondToken = getFromBuffer(shotMsg, ",");
            int shotX = 0, shotY = 0;
            if(secondToken == "RESULT"){
                continue;
            } else {
                shotX = stoi(secondToken);
                string sy = getFromBuffer(shotMsg, "\r");
                shotY = stoi(sy);
            }
            
            cout << "Opponent shot at (" << shotX << ", " << shotY << ")." << endl;
            // Process the shot on your grid.
            string cell = myMap[shotY][shotX];
            string response;
            if (cell != " " && cell != "X" && cell != "o") {
                // It's a hit.
                myMap[shotY][shotX] = "X";
                cout << "Your ship was hit!" << endl;
                if (allShipsSunk(myMap)) {
                    response = "PLAY,RESULT,WIN\r\n";
                    send(sock, response.c_str(), response.length(), 0);
                    cout << "All your ships have been sunk. You lose." << endl;
                    gameOver = true;
                    displayGrids(myMap, oppMap);
                    break;
                } else {
                    response = "PLAY,RESULT,HIT\r\n";
                    send(sock, response.c_str(), response.length(), 0);
                }
                myTurn = false;
            } else {
                // It's a miss.
                if (myMap[shotY][shotX] == " ")
                    myMap[shotY][shotX] = "o";
                cout << "Opponent missed." << endl;
                response = "PLAY,RESULT,MISS\r\n";
                send(sock, response.c_str(), response.length(), 0);
                myTurn = true;
            }
            // Display grids after processing the shot.
            displayGrids(myMap, oppMap);
        }
    }
    
    // Display final grids.
    cout << "\nFinal Your Grid:" << endl;
    printPlayerGrid(myMap);
    cout << "\nFinal Opponent Grid:" << endl;
    printOpponentGrid(oppMap);
    cout << "\nGame Over." << endl;
    
    close(sock);
    kp.stop();
    return 0;
}
