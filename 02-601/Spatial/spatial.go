package main

import (
	"bufio"
	"fmt"
	//"image"
	"os"
	"strconv"
	"strings"
)

// The data stored in a single cell of a field
type Cell struct {
	strategy string  //represents "C" or "D" corresponding to the type of prisoner in the cell
	score    float64 //represents the score of the cell based on the prisoner's relationship with neighboring cells
}

// The game board is a 2D slice of Cell objects
type GameBoard [][]Cell

// we will create a gameboard of size rows by columns
func CreateGameBoard(rows, columns int) [][]Cell {
	//Gameboard is [][]Cell
	gameboard := make([][]Cell, rows)
	for i := range gameboard {
		gameboard[i] = make([]Cell, columns)
	}
	return gameboard
}

/* ReadFile() functions should correctly open files or return error messages
the first line of targeted file contains rows and colums
each line except first line will contain strategies which are Cs or Ds
CCCCDCCCDDCCCCC
*/

func ReadFile(filename string) GameBoard {

	//open the file and make sure all went well
	file, err := os.Open("filename")
	if err != nil {
		fmt.Println("Error: something went wrong opening the file")
		fmt.Println("Probably you gave the wrong filename")
		os.Exit(1)
	}

	//create the variable to hold the lines
	var lines []string = make([]string, 0)
	//for every line in the file
	scanner := bufio.NewScanner(file)

	for scanner.Scan() {
		//append it to the lines slice
		lines = append(lines, scanner.Text())
	}

	//check that all went ok
	if scanner.Err() != nil {
		fmt.Println("Sorry: there was some kind of error during the file reading")
		os.Exit(1)
	}

	//we will parse first line that contains the number of rows and columns
	var rows, columns int
	fmt.Sscanf(lines[0], "%v,%v", &rows, &columns)

	//now we can create a gameboard
	var myboard GameBoard = CreateGameBoard(rows, columns)

	//fill the gameboard with strategis which are Cs or Ds
	//each line except first line contains a string of Cs and Ds
	for i := 0; i < rows; i++ {
		//split into single line, and the seperation mark would be: ""
		var items []string = strings.Split(lines[i+1], "") //except first line
		//now fill the gameboard
		//use column-loop to obtain all elements in a single line
		for j := 0; j < columns; j++ {
			//board-->type GameBoard-->[][]Cell-->strategy
			myboard[i][j].strategy = items[j]
		}
	}

	//close the file and return lines
	file.Close()
	//after reading files, we shall have a new gameboard filled with Cs or Ds
	return myboard
}

//Create a new canvas to visualize.
func DrawGameBoard(board GameBoard) {
	//width is the length of column,set the square of length 5
	w := len(board[0]) * 5
	//height is the length of rows
	h := len(board) * 5
	pic := CreateNewCanvas(w, h)
	//set color
	blue := MakeColor(0, 0, 255)
	red := MakeColor(255, 0, 0)

	//now we draw the board with different color
	for i := 0; i < len(board); i++ {
		for j := 0; j < len(board[0]); j++ {
			//C prisoner is blue
			if board[i][j].strategy == "C" {
				pic.SetFillColor(blue)
				pic.ClearRect(i*5, j*5, (i+1)*5, (j+1)*5)
			} else { //for D prisoner
				pic.SetFillColor(red)
				pic.ClearRect(i*5, j*5, (i+1)*5, (j+1)*5)
			}
		}
	}
	pic.SaveToPNG("Prisoners.png")
	//for animation
	//return pic
}

//Here we implement a prisoner's dilemma table based on Assignment 4.1
func GameRules(Prisoner1, Prisoner2 string, b float64) float64 {
	switch {
	case Prisoner1 == "C" && Prisoner2 == "C":
		return 1
	case Prisoner1 == "C" && Prisoner2 == "D":
		return 0
	case Prisoner1 == "D" && Prisoner2 == "C":
		return b
	default:
		return 0
	}
}

//i,j can serve as indices.To make sure playing games inside our gameboard.
func InField(board GameBoard, i, j int) bool {
	switch {
	case i < 0 || i > len(board):
		return false
	case j < 0 || j > len(board[0]):
		return false
	default:
		return true
	}
}

//based on GameRules we can calculate each prisoner's score
func Scores(board GameBoard, i, j int, b float64) float64 {
	//here each prisoner will play with neighbors
	//for corner/edge prisoners, some of their neighbors are ourside
	var score float64
	//For a common cell, the indices of its neighbors range from [i-1,i+1] and [j-1,j+1]
	for x := i - 1; x < i+2; x++ {
		for y := j - 1; y < j+2; j++ {
			//ignore the "outsider"
			if InField(board, x, y) {
				//i,j is current prisoner,while x,y indicates neighbors
				score += GameRules(board[i][j].strategy, board[x][y].strategy, b)
			}
		}
	}
	return score
}

//now We can loop over all prisoners and simulate playing games
func PlayGames(board GameBoard, b float64) {
	//now we loop over all i and j which served as indexes
	for i := 0; i < len(board); i++ {
		for j := 0; j < len(board); j++ {
			//all prisoners should play their games
			board[i][j].score = Scores(board, i, j, b)
		}
	}
}

//prisoner will change strategy based on which strategy got the highest scores
func BestStrategy(board GameBoard, i, j int) string {
	//find the maximun value of a 2D slice,assume itself is the best at first
	max_score := board[i][j].score
	best := board[i][j].strategy
	//now we compare to its neighbors until the highest scores happened
	for x := i - 1; x < i+2; x++ {
		for y := j - 1; y < j+2; j++ {
			//ignore the "outsider" and update to better strategy/score
			if InField(board, x, y) && (board[x][y].score > max_score) {
				max_score = board[x][y].score
				best = board[x][y].strategy
			}
		}
	}
	return best
}

//now each prisoners can adopt the best Strategy for themselves
//update new strategy in place and have a new gameboard
func ChangeStrategy(board GameBoard) GameBoard {
	newboard := CreateGameBoard(len(board), len(board[0]))
	for i := 0; i < len(board); i++ {
		for j := 0; j < len(board[0]); j++ {
			newboard[i][j].strategy = BestStrategy(board, i, j)
		}
	}
	return newboard
}

//The Evolve() function will simply just play multiple games. We used STEPS to control it.
func Evolve(board GameBoard, STEPS int, b float64) GameBoard {
	for step := 0; step < STEPS; step++ {
		PlayGames(board, b)
		board = ChangeStrategy(board)
	}
	return board
}

/* we implement a AnimeSpatial() function to make animation GIF

func AnimeSpatial(boards []GameBoard) []image.Image {
	gifImages := make([]image.Image, len(boards))
	for i, board := range boards {
		pic := DrawGameBoard(board)
		// gifImages[0] is the original board
		gifImages[i] = pic.img
	}
	return gifImages
}

*/

func main() {

	//check argument numbers
	if len(os.Args) != 4 {
		fmt.Println("Error: FILENAME,b,STEPS are needed")
		os.Exit(1)
	}

	//read arguments
	boardfilename := os.Args[1]

	//b is float64
	b, err := strconv.ParseFloat(os.Args[2], 64)
	if err != nil {
		fmt.Println("Error: b must be a number")
		os.Exit(1)
	} else if b <= 0 {
		fmt.Println("Error: b must be positive")
		os.Exit(1)
	}

	STEPS, err := strconv.Atoi(os.Args[3])
	if err != nil {
		fmt.Println("Error: STEPS must be a number")
		os.Exit(1)
	} else if STEPS < 0 {
		fmt.Println("Error: STEPS must be an integer")
		os.Exit(1)
	}

	//now we create a new gameboard based on command line argument
	board := ReadFile(boardfilename)
	//then we evolve the board
	board = Evolve(board, STEPS, b)
	//finially we visualize the evolved board
	DrawGameBoard(board)

	/* the following codes will animate spatial game

	//in order to animate spatial games, we first have a slice of all evolved boards
	//Steps represent evolve times,need to include original board,so steps+1

	boards := make([]GameBoard, STEPS+1)

	//use index to represent evolving steps, so boards[0] is the original board
	for i := range boards {
		//each evolves i step/steps
		boards[i] = Evolve(board, i, b)
	}
	gifImages := AnimeSpatial(boards)
	//now draw the GIF
	process(gifImages, "spatial")

	*/
}
