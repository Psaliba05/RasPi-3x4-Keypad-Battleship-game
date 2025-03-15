Include a README.md file to the GitLab project that:
Describe the hardware infrastructure,
Describe the game,
Describe how to use the keypad,
Describe the game message exchange protocol (what comes after the 'PLAY' command),
Describe how to compile and run your code.


Hardware infrastructure:\
something something


Description of our game:\
We wanted to create a replica of the famous game "Battleship". Where two players with a 9x9 grid with 7 ships play to shoot down eachothers ships with a randomly generated grid. The point of the game is to blindly choose an X and Y coordinate and see if you hit your opponents ship. If not, you will switch turns until all ships from one player has been sunk.

How to use the keypad:\
We used a 4x3 keypad connected to the Raspberry Pi, this keypad functions like any ordinary keypad where you can click any number you want. First be prompt to enter a number from 0-9 for your X coordinate. Next, it will ask for a 0-9 number for your Y coordinate. The game will then register your input as a "shot" at the opponents corresponding grid.

Game message exchange protocol:\
After both players connect, the server will start to display each players play coordinate followed by the result of the shot (either hit a ship or missed a ship)

How to compile and run our code:
To compile our code you will need to make sure you have all correct files for the game (mygame.cpp, keypad.cpp, keypad.h, Hw4.cpp, genFleet.h, and genFleet.cpp) in a folder and make sure you are in that directory. To compile, use this command in a Raspberry Pi PuTTy session :\
`g++ -std=c++20 -o mygame  mygame.cpp genFleet.cpp keypad.cpp -lwiringPi -lpthread`\
To run the game, you can type :\
`./mygame` 

