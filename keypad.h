/* ECEGRE-2020 - Seattle University
   Description: Homework#4 Header file for keypad class
   Authors:Brayton Alvarez and Paolo Saliba
*/

#include <string>
#include <iostream>  // cerr
#include <thread>    // jthread, this_thread::sleep_for, get_key_thread

#pragma once  // include only once

// Size of column and row for keypad
#define MAXCOL 3
#define MAXROW 4

using namespace std;

class Keypad
{
    private:
        bool digit_ready;
        bool is_stopped;
        std::jthread* get_key_thread;
        const string KEYPAD[MAXROW][MAXCOL] = {
            {"1", "2", "3"},
            {"4", "5", "6"},
            {"7", "8", "9"},
            {"*", "0", "#"}
        };
        int column[MAXCOL];
        int row[MAXROW];
        string last_digit;

    public:
        // Constructor
        Keypad(int columnInPins[MAXCOL], int rowInPins[MAXROW]);
        
        // Start the keypad thread
        void run(void);
        
        // Thread function to continuously check for key presses
        void get_key();
        
        // Return the latest key digit
        string get_digit(void);
        
        // Stop the keypad thread
        void stop();
};
