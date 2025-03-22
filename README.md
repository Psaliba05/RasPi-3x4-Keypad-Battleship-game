# Battleship

## Hardware infrastructure:
Hardware we used were 2 Raspberry Pi's connected to a 4x3 keypad. The keypad is to allow user input for our Battleship game. You are able to type any number while also using ' * ' as a backspace and ' # ' to enter your input.


## Description of our game:
We wanted to create a replica of the famous game "Battleship". Where two players with a 9x9 grid with 7 ships play to shoot down eachothers ships with a randomly generated grid. The point of the game is to blindly choose an X and Y coordinate and see if you hit your opponents ship. If not, you will switch turns until all ships from one player has been sunk.

## How to use the keypad:
We used a 4x3 keypad connected to the Raspberry Pi, this keypad functions like any ordinary keypad where you can click any number you want. First be prompt to enter a number from 0-9 for your X coordinate. Next, it will ask for a 0-9 number for your Y coordinate. The game will then register your input as a "shot" at the opponents corresponding grid.

## Game message exchange protocol:
After both players connect, the server will start to display each players play coordinate followed by the result of the shot (either hit a ship or missed a ship)\
\
**Example of a game message after a play:**\
\
```
Your turn. Enter shot coordinates.(# to enter shot, * to delete)\
Enter X coordinate (0-9): 4\
Enter Y coordinate (0-9): 3\
Shot sent at (4, 3). Waiting for result...\
Your shot hit the enemy ship!\
\
Your Grid:\
   0 1 2 3 4 5 6 7 8 9\
   --------------------\
0|     o         o\
1|     X   X\
2|   o X o X     o ■\
3|     X   X     o ■\
4|     o   X o     ■ o\
5| o ■ o   X o o   o\
6|   ■         X o\
7|   o         X\
8| ■ ■ o   o   o\
9|     o ■ ■ ■ ■ o\
\
Opponent Grid:\
   0 1 2 3 4 5 6 7 8 9\
   --------------------\
0| ? ? ? ? ? ? ? ? ? ?\
1| ? ? ? ? ?   ? ?   ?\
2| ? ?   ? ?   ? ? ? ?\
3|   ■ ? ■ ■ ■     ? ?\
4| ? ?   ? ? ? ? ■ ? ?\
5| ? ? ? ?   ? ? ■ ? ?\
6| ? ?   ? ? ? ?   ? ?\
7|   ■ ■     ■ ■ ■\
8| ?   ? ? ? ? ? ?   ?\
9| ? ?   ■ ■ ■ ■   ? ?
```
## How to compile and run our code:
To compile our code you will need to make sure you have all correct files for the game (mygame.cpp, keypad.cpp, keypad.h, Hw4.cpp, genFleet.h, and genFleet.cpp) in a folder and make sure you are in that directory.\
\
To compile, use this command in a Raspberry Pi PuTTY session :\
`g++ -std=c++20 -o mygame  mygame.cpp genFleet.cpp keypad.cpp -lwiringPi -lpthread`\
\
To run the game, you can type :\
`./mygame` 

