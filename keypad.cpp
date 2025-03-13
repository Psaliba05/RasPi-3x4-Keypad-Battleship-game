/* ECEGRE-2020 - Seattle University
   Description: Homework#4 Keypad class file
   Authors:
*/

#include <string>
#include <wiringPi.h>
#include "keypad.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono_literals;  // Allow use of 10ms and 0.1s literals

Keypad::Keypad(int columnInPins[MAXCOL], int rowInPins[MAXROW]){
    // Setup wiringPi
    wiringPiSetupGpio();
    
    // Check for duplicate pin numbers
    for (int i = 0; i < MAXCOL; i++) {
        for (int j = 0; j < MAXCOL; j++) {
            if (i != j && columnInPins[i] == columnInPins[j]) throw "Invalid Custom Pinout: There are Duplicate Pins";
        }
        for (int j = 0; j < MAXROW; j++) {
            if (columnInPins[i] == rowInPins[j]) throw "Invalid Custom Pinout: There are Duplicate Pins";
        }
    }
    
    for (int i = 0; i < MAXROW; i++) {
        for (int j = 0; j < MAXROW; j++) {
            if (i != j && rowInPins[i] == rowInPins[j]) throw "Invalid Custom Pinout: There are Duplicate Pins";
        }
    }
    
    // Set custom pinouts if valid
    for (int i = 0; i < MAXCOL; i++) {
        if (columnInPins[i] <= 27) column[i] = columnInPins[i];
        else throw "Invalid Custom Column Pinout";
    }

    for (int i = 0; i < MAXROW; i++) {
        if (rowInPins[i] <= 27) row[i] = rowInPins[i];
        else throw "Invalid Custom Row Pinout";
    }

    // Initialize booleans
    digit_ready = false;
    is_stopped = true;
}

void Keypad::run(){
    // Start thread if not already running
    if (is_stopped) {
        try {
            get_key_thread = new std::jthread(&Keypad::get_key, this);
            is_stopped = false;
        } catch(...) {
            cerr << "Couldn't start thread";
        }
    }
}

void Keypad::get_key() {
    while(true) {
        if (is_stopped) break;
        
        // Short delay at each loop iteration
        this_thread::sleep_for(10ms);
        
        // Set column pins to output low
        for (int i = 0; i < MAXCOL; i++) {
            pinMode(this->column[i], OUTPUT);
            digitalWrite(this->column[i], 0);
        }
        
        // Set row pins to input with pull-up resistor
        for (int i = 0; i < MAXROW; i++) {
            pinMode(this->row[i], INPUT);
            pullUpDnControl(this->row[i], PUD_UP);
        }
        
        // Check rows for a key press
        int row_val = -1;
        for (int i = 0; i < MAXROW; i++) {
            if (digitalRead(this->row[i]) == 0) row_val = i;
        }
        
        // If no key was pressed, reset and continue
        if (row_val < 0 || row_val >= MAXROW) {
            this->last_digit = "";
            this->digit_ready = false;
            continue;
        }
        
        // Set columns to input with pull-down resistor
        for (int i = 0; i < MAXCOL; i++) {
            pinMode(this->column[i], INPUT);
            pullUpDnControl(this->column[i], PUD_DOWN);
        }
        
        // Set the pressed row to output high
        pinMode(this->row[row_val], OUTPUT);
        digitalWrite(this->row[row_val], 1);
        
        // Determine which column was pressed
        int col_val = -1;
        for(int i = 0; i < MAXCOL; i++) {
            if (digitalRead(this->column[i]) == 1) col_val = i;
        }

        // If no valid column was found, reset and continue
        if (col_val < 0 || col_val >= MAXCOL) {
            this->last_digit = "";
            this->digit_ready = false;
            continue;
        }
        
        // Register the key press if one isn’t already pending
        if (!this->digit_ready) {
            this->digit_ready = true;
            this->last_digit = KEYPAD[row_val][col_val];
        }
    }
}

string Keypad::get_digit(){
    // Loop until a digit becomes available, then return it
    while(true){
        if (this->digit_ready){
            this->digit_ready = false;
            string temp = last_digit;
            last_digit = "";
            return temp;
        }
        this_thread::sleep_for(0.1s);
    }
}

// Safely stop the keypad thread
void Keypad::stop() {
    is_stopped = true;
    if (get_key_thread) {
        get_key_thread->join();
        delete get_key_thread;
        get_key_thread = nullptr;
    }
}
