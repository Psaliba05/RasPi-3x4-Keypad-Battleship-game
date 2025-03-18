//Provided by Eddy Ferre
#ifndef GENFLEET_H
#define GENFLEET_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <random>

#define FLEET_COUNT   7
#define GRID_SIZE    10

using namespace std;

// Returns a random unsigned short within the range [0, range]
inline unsigned short bounded_rand(unsigned short range) {
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> dist6(0, range);
    return static_cast<unsigned short>(dist6(rng));
}

// Resets the fleet map by setting every cell to a blank space.
inline void resetFleet(string(& map)[GRID_SIZE][GRID_SIZE]){
    for(unsigned short row = 0; row < GRID_SIZE; row++){
        for(unsigned short col = 0; col < GRID_SIZE; col++){
            map[row][col] = " ";
        }
    }
}

// Prints a simplified view of the fleet map using a solid block for non-empty cells.
inline void printFleet(const string(& map)[GRID_SIZE][GRID_SIZE]){
    cout << "   ";
    for(unsigned short i = 0; i < GRID_SIZE; i++){
        cout << i << " ";
    }
    cout << endl << "___";
    for(unsigned short i = 0; i < GRID_SIZE; i++){
        cout << "__";
    }
    cout << endl;
    for(unsigned short row = 0; row < GRID_SIZE; row++){
        cout << row << "| ";
        for(unsigned short col = 0; col < GRID_SIZE; col++){
            cout << (map[row][col] != " " ? "\u25A0" : " ") << " ";
        }
        cout << endl;
    }
}

// Prints a detailed view of the fleet map, displaying each cell's actual content.
inline void printFleetDetailed(const string(& map)[GRID_SIZE][GRID_SIZE]){
    cout << "   ";
    for(unsigned short i = 0; i < GRID_SIZE; i++){
        cout << i << " ";
    }
    cout << endl << "___";
    for(unsigned short i = 0; i < GRID_SIZE; i++){
        cout << "__";
    }
    cout << endl;
    for(unsigned short row = 0; row < GRID_SIZE; row++){
        cout << row << "| ";
        for(unsigned short col = 0; col < GRID_SIZE; col++){
            cout << map[row][col] << " ";
        }
        cout << endl;
    }
}

// Automatically generates a fleet on the map based on the fleet sizes provided.
inline void autogenFleet(string(& map)[GRID_SIZE][GRID_SIZE], const unsigned short(& fleet)[FLEET_COUNT]){
    for(unsigned short ship_idx = 0; ship_idx < FLEET_COUNT; ship_idx++){
        unsigned short ship_size = fleet[ship_idx];
        unsigned short isHorizontal;
        unsigned short max_col, lo_col, hi_col;
        unsigned short max_row, lo_row, hi_row;
        unsigned short start_col;
        unsigned short start_row;

        while(true){
            isHorizontal = bounded_rand(1);
            if(isHorizontal){
                max_row = GRID_SIZE - 1;
                max_col = GRID_SIZE - ship_size - 1;
            }
            else{
                max_row = GRID_SIZE - ship_size - 1;
                max_col = GRID_SIZE - 1;
            }
            start_row = bounded_rand(max_row);
            start_col = bounded_rand(max_col);
            
            // Replace std::max with a ternary expression to ensure matching types.
            lo_col = (start_col > 0 ? start_col - 1 : 0);
            lo_row = (start_row > 0 ? start_row - 1 : 0);
            
            if(isHorizontal){
                hi_col = min(static_cast<unsigned short>(start_col + ship_size), static_cast<unsigned short>(GRID_SIZE - 1));
                hi_row = min(static_cast<unsigned short>(start_row + 1), static_cast<unsigned short>(GRID_SIZE - 1));
            }
            else{
                hi_col = min(static_cast<unsigned short>(start_col + 1), static_cast<unsigned short>(GRID_SIZE - 1));
                hi_row = min(static_cast<unsigned short>(start_row + ship_size), static_cast<unsigned short>(GRID_SIZE - 1));
            }
            
            bool occupied = false;
            if(isHorizontal){
                for(unsigned short col = start_col; col < start_col + ship_size; col++){
                    if(map[start_row][col] != " "){
                        occupied = true;
                        break;
                    }
                }
            }
            else{
                for(unsigned short row = start_row; row < start_row + ship_size; row++){
                    if(map[row][start_col] != " "){
                        occupied = true;
                        break;
                    }
                }
            }
            if(occupied){
                continue;
            }
            for(unsigned short col = lo_col; col <= hi_col; col++){
                for(unsigned short row = lo_row; row <= hi_row; row++){
                    map[row][col] = ".";
                }
            }
            if(isHorizontal){
                for(unsigned short col = start_col; col < start_col + ship_size; col++){
                    map[start_row][col] = to_string(ship_idx);
                }
            }
            else{
                for(unsigned short row = start_row; row < start_row + ship_size; row++){
                    map[row][start_col] = to_string(ship_idx);
                }
            }
            break;
        }
    }
    // Remove temporary markers.
    for(unsigned short row = 0; row < GRID_SIZE; row++){
        for(unsigned short col = 0; col < GRID_SIZE; col++){
            if(map[row][col] == ".")
                map[row][col] = " ";
        }
    }
}

#endif // GENFLEET_H
