/*
    ECEGR-2020 - Final Project
    Revised mygame.cpp: Integrates fleet generation (from genFleet.h/genFleet.cpp)
    and keypad input (from keypad.h/keypad.cpp) for the PLAY command.
*/

#include "genFleet.h"    // Fleet generation functions
#include "keypad.h"      // Keypad interface
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>   // Added to provide inet_addr()
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <csignal>
#include <exception>
#include <chrono>
#include <thread>

using namespace std;

// Helper to parse comma-delimited server responses
string getFromBuffer(string& s_buff, string del) {
    string ret_str;
    size_t del_pos = s_buff.find(del);
    if (del_pos != string::npos) {
        ret_str = s_buff.substr(0, del_pos);
        s_buff.erase(0, del_pos + del.length());
    } else {
        ret_str = s_buff;
        s_buff = "";
    }
    return ret_str;
}

// Global flag and signal handler for graceful exit
volatile sig_atomic_t running = 1;
void signalHandler(int signum) {
    cout << "\nExiting... " << flush;
    running = 0;
}

// Helper function to read a PLAY command from the keypad
// '*' acts as backspace and '#' terminates the input.
string readPlayCommand(Keypad &kp) {
    string command;
    cout << "Enter PLAY command using keypad (press '#' to finish, '*' for backspace): " << flush;
    while (true) {
        string digit = kp.get_digit();
        if (digit == "*") {
            if (!command.empty()) {
                command.pop_back();
                cout << "\b \b" << flush;
            }
        } else if (digit == "#") {
            cout << endl;
            break;
        } else {
            command += digit;
            cout << digit << flush;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return command;
}

int main()
{
    // Setup signal handler for Ctrl+C
    signal(SIGINT, signalHandler);
    
    string server_ip;
    string user_name;
    string their_command;
    string my_sub_command;
    string their_sub_command;
    string opponent = "";
    string player_pos;
    string their_game_id;

    // TODO: Change to your actual game ID
    string my_game_id = "SampleGame";

    // Get user input for name and server IP
    cout << "Enter your name > ";
    getline(cin, user_name);
    cout << "Server IP > ";
    getline(cin, server_ip);

    // Create and initialize the fleet board
    string myMap[GRID_SIZE][GRID_SIZE];
    const unsigned short fleet[FLEET_COUNT] = {5, 4, 3, 3, 2, 2, 2};
    resetFleet(myMap);
    autogenFleet(myMap, fleet);
    
    // Optionally display the generated fleet board
    cout << "\nGenerated Fleet Board:" << endl;
    printFleet(myMap);

    // Setup keypad pins (adjust as per your wiring)
    int colPins[3] = {21, 20, 16};
    int rowPins[4] = {19, 13, 6, 5};

    // Create and start the keypad thread
    Keypad kp(colPins, rowPins);
    kp.run();
    
    try {
        // Create a socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            cerr << "Socket creation error" << endl;
            return 1;
        }

        // Specify server address
        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(10000);
        serverAddress.sin_addr.s_addr = inet_addr(server_ip.c_str());

        cout << "Connecting ... " << flush;
        while (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) {
            if (!running) break;
            this_thread::sleep_for(chrono::seconds(5));
        }
        cout << "Connected" << endl;

        // Main loop for communication with the server
        while (running) {
            // Send READY command
            string message = "READY," + user_name + "," + my_game_id + "\r\n";
            send(sock, message.c_str(), message.length(), 0);
            cout << "Waiting to be paired..." << endl;

            while (running) {
                char buffer[1024] = {0};
                int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
                if (bytesReceived <= 0) {
                    cerr << "Server stopped or connection error" << endl;
                    throw "Server stopped";
                }
                cout << "Message from server: " << buffer << endl;
                string buff(buffer);
                their_command = getFromBuffer(buff, ",");
                
                if (their_command == "START") {
                    player_pos = getFromBuffer(buff, ",");
                    opponent = getFromBuffer(buff, ",");
                    their_game_id = getFromBuffer(buff, "\r");
                    if (their_game_id == my_game_id) {
                        cout << "Let's play a game" << endl;
                    } else {
                        cout << "I don't know how to play " << their_game_id << endl;
                        break;
                    }
                    if (player_pos == "1") {  // Opponent's turn first
                        cout << "Their turn..." << endl;
                        continue;
                    }
                }
                else if (their_command == "PLAY") {
                    their_sub_command = getFromBuffer(buff, "\r");
                    // TODO: Parse opponent's play command as needed.
                }
                else if (their_command == "ERROR") {
                    their_sub_command = getFromBuffer(buff, "\r");
                    if (their_sub_command == "Opponent " + opponent + " left") {
                        opponent = "";
                        break;
                    }
                }
                
                // It's this player's turn: collect PLAY command from the keypad.
                my_sub_command = readPlayCommand(kp);
                message = "PLAY," + my_sub_command + "\r\n";
                send(sock, message.c_str(), message.length(), 0);
                cout << "Waiting for opponent's move..." << endl;
            }
        }
    }
    catch (const char* msg) {
        cerr << "\nExiting: " << msg << endl;
    }
    catch (...) {
        cerr << "\nExiting due to an unexpected error." << endl;
    }
    
    // Clean up: stop the keypad thread before exiting.
    kp.stop();
    cout << "Done" << endl;
    return 0; 
}
