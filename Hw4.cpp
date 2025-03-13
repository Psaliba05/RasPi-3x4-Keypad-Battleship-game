/* ECEGRE-2020 - Seattle University
   Description: Homework#4 main for running a keypad and printing output
   Authors: Paolo Saliba and Brayton Alvarez - psaliba@seattleu.edu
*/
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>
#include "keypad.h"

using namespace std;

// Global flag to indicate when to exit
volatile sig_atomic_t running = 1;

// Signal handler for Ctrl+C
void sig_handler(int signal) {
    running = 0;
}

int main(){
    // Declare the pin arrays for columns and rows
    int colPins[3] = {21, 20, 16};   // pins C1 (GPIO21), C2 (GPIO20), C3 (GPIO16)
    int rowPins[4] = {19, 13, 6, 5};   // pins R4 (GPIO19), R5 (GPIO13), R6 (GPIO6), R7 (GPIO5)

    // Output helper message
    cout << "Press keys on your keypad." << endl;
    cout << " * = Backspace" << endl;
    cout << " # = Enter" << endl;
    cout << " Ctrl+C to quit" << endl << endl;

    // Set up signal handler for Ctrl+C
    signal(SIGINT, sig_handler);

    // Create the Keypad instance and start its thread
    Keypad kp(colPins, rowPins);
    kp.run();

    int counter = 0;
    while(running) {
        string digit = kp.get_digit();

        if(digit == "*") {        // Backspace: remove last character if any
            if(counter > 0) {
                cout << "\b \b";
                counter--;
            }
        }
        else if(digit == "#") {   // Enter: start new line and reset counter
            cout << endl;
            counter = 0;
        }
        else {                    // Otherwise, print the digit
            cout << digit;
            counter++;
        }
        cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    cout << "\nExiting" << endl;
    kp.stop();
    return 0;
}
