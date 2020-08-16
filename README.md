# 15-puzzle-solver

15-puzzle is a sliding puzzle that consists of a frame of numbered square tiles in random order with one tile missing. The solver takes the file containing initial state of puzzle as input and outputs the number of moves of the optimal solution for the puzzle.

The solver is built by implementing the Iterative Deepening *A** (IDA*) algorithm. The heuristic used to prune the search space is the Sum of Manhattan Distances. 

<p align="center">
  <img width="300" height="300" src="https://upload.wikimedia.org/wikipedia/commons/f/ff/15-puzzle_magical.svg">
  <p align="center">Image from <a href="https://en.wikipedia.org/wiki/15_puzzle">wikipedia</a></p>
</p>

## Prerequisite

To run C program, you need to download a C compiler. You can download [GCC](https://gcc.gnu.org/).

## Installation

Download the source code of the project and compile it with the command. 
```
make
```
## Usage

![](15ps.gif)

To use the program, run the program with the following
```
./15puzzle <.puzzle file>
```
stdout has the following information:
- initial state of the puzzle
- *h*(*s*<sub>0</sub>) heuristic estimate for the initial state
- thresholds the search have used to solve the problem
- number of moves of the optimal solution
- number of generated nodes
- number of expanded nodes
- total search time, in seconds
- number of expanded nodes per second


## License

The project uses [MIT License](<LICENSE>).
