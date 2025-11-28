# Algorithmic Strategy Game Collection

## 🎮 Overview
This repository hosts a suite of classic logic and strategy games implemented from scratch in **C** and **C++**. 

The primary goal of these projects is to demonstrate proficiency in:
* **Low-Level Logic:** Implementing complex game rules without game engines.
* **Memory Management:** Efficient use of pointers and arrays for board states.
* **Recursion:** Utilizing recursive function calls for game loops and state updates.
* **Algorithmic Thinking:** Implementing flood-fill, pattern matching, and state evaluation algorithms.

## 📂 Included Games

### 1. Minesweeper (C)
A terminal-based implementation of the classic puzzle game.
* **Key Logic:** Uses a **recursive flood-fill algorithm** to automatically reveal empty cells when a safe zone is clicked.
* **Features:** Dynamic board generation, proximity calculation for mines.

### 2. Reversi / Othello (C)
A implementation of the strategic board game involving disc flipping.
* **Key Logic:** Algorithms to traverse the board in 8 directions to validate legal moves and flip opponent pieces.
* **State Management:** Tracks player turns and board saturation.

### 3. Battleship (C)
A naval strategy game simulation.
* **Key Logic:** Coordinate mapping and 2D array manipulation to track hit/miss states.
* **Hidden Information:** Manages two separate board states (visible vs. hidden) for the fog-of-war mechanic.

### 4. Connect-Four (C++)
Two variations of the vertical checker game.
* **Key Logic:** Pattern matching algorithms to detect horizontal, vertical, and diagonal win conditions efficiently.
* **Language Features:** Demonstrates C++ standard I/O and flow control.

### 5. Vault Breaker (C)
A logic-based code-breaking puzzle game.
* **Key Logic:** Comparisons between user guesses and a generated secret code to provide feedback (similar to Mastermind logic).

### 6. Wizard Duel RPG (C)
A turn-based battle simulation using recursive state transitions.
* **Recursion vs. Iteration:** Unlike traditional loops, the game loop and score calculation logic are implemented using **recursive function calls**.
* **Data Handling:** Parses external configuration files (`spellbook.txt`) to load dynamic game assets (spells/stats) into structs.
* **Mechanics:** Manages mana cost, damage RNG, and elemental class matching logic.

## 🛠️ Build & Run
Each game is self-contained in its respective directory. To play, navigate to the game folder and compile using GCC or G++.

**Example (Minesweeper):**
```bash
cd minesweeper
gcc main.c -o minesweeper
./minesweeper
