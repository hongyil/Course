/**********************************

Project Name: Diffusion Simulator
Name: Hongyi Liang
AndrewID: hongyil

***********************************/

package main

import (
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"
)

/**********************************************************
Random Walkers: type and related functions
***********************************************************/

type Walker struct {
	numParticles int   //the number of random-walkers, which indicate concentration
	randomSteps  []int //possible random steps,assume integer for simplicity
	numSteps     int   //all random walkers will perfom random walk for "numSteps" times
}

//to initialize walkers, based on command line arguments
func (walker *Walker) InitializeWalker(numParticles, numSteps int, randomSteps []int) {
	walker.numParticles = numParticles
	walker.numSteps = numSteps
	walker.randomSteps = randomSteps
}

//reset the parameters of the initialized random-walkers
func (walker *Walker) NewWalker(newNumParticles, newNumSteps int, newRandomSteps []int) {
	walker.numParticles = newNumParticles
	walker.numSteps = newNumSteps
	walker.randomSteps = newRandomSteps
}

//get the maximum steps(absolute value) that a random-walker can travel
func (walker *Walker) GetMaxSteps() int {
	max := 0
	for _, i := range walker.randomSteps {
		if math.Abs(float64(i)) >= float64(max) {
			max = int(math.Abs(float64(i)))
		}
	}
	return max
}

//the maximum distance that certain particles can travel
func (walker *Walker) GetMaxDist() int {
	maxTime := walker.numSteps
	maxStep := walker.GetMaxSteps()
	return maxTime * maxStep
}

//sometimes a walker may perform no movement, the frequency has some effects on mean square displacement
func (walker *Walker) GetJumpFreq() float64 {
	//if it's not a "0" step, then it's active
	movement := 0
	for _, step := range walker.randomSteps {
		if step != 0 {
			movement += 1
		}
	}
	return float64(movement) / float64(len(walker.randomSteps))
}

//RandomStep() Method can produce a random step for random-walkers
func (walker *Walker) RandomStep(randomSteps []int) int {

	//use time.Now() as seed which can keep changing
	seed := time.Now().UTC().UnixNano()
	//use rand.Seed() method to break the current state
	rand.Seed(seed)
	//now produce a random index number
	index := rand.Int() % len(randomSteps)
	//retrieve the random number from randomSteps list
	return randomSteps[index]
}

//Move() method will return a new position after moving one random step
func (walker *Walker) Move(position, step int) int {
	//step comes from RandomStep() Method
	position += step
	return position
}

//ChooseDirection() will return a flag either -1 or 1 randomly to indicate the direction
func (walker *Walker) ChooseDirection() int {

	//use time.Now() as seed which can keep changing
	seed := time.Now().UTC().UnixNano()
	//break current state
	rand.Seed(seed)
	//then we produce two random number(like 0 and 1)
	flag := rand.Int() % 2
	//parse flag into direction
	if flag == 1 {
		return 1
	} else {
		return -1
	}
}

/**********************************************************
Diffusion board: type and related functions
**********************************************************/

//the walker diffuse within the scope of the board
type DiffusionBoard struct {
	square [][]Walker //each square in our board holds multiple random-walkers
	size   int        //record the size of our diffusion board
}

func InitializeBoard(maxDist int) *DiffusionBoard {
	//the depth of diffusion board is maxDist
	board := make([][]Walker, 10)
	//next initialize the width of our board
	for i := 0; i < len(board); i++ {
		board[i] = make([]Walker, 101)
	}
	return &DiffusionBoard{square: board, size: 100}
}

//assume that diffusion will only occur inside certain region at given time
func (board *DiffusionBoard) Infield(r, c int) bool {
	switch {
	case r < 0 || r > 10:
		return false
	case c < 0 || c > board.size:
		return false
	default:
		return true
	}
}

/*****************************************************************************
     thin         thick       surface       slab
   00011000     0001111000    1000001     11110000   -----       1=random-Walkers
   00011000     0001111000    1000001     11110000     |         0=blank
   00011000     0001111000    1000001     11110000     | 10
   00011000     0001111000    1000001     11110000     |
   00011000     0001111000    1000001     11110000   -----
   |<-100->|
******************************************************************************/

//InitializeWalkers() will place the random-walkers to the proper locations of the diffusion board
func (board *DiffusionBoard) InitializeWalkers(mode string, numParticles, numSteps int, randomSteps []int) {

	switch mode {
	case "thin":
		for x := 0; x < 10; x++ {
			for y := -1; y < 1; y++ {
				board.square[x][y+board.size/2].InitializeWalker(numParticles, numSteps, randomSteps)
			}
		}

	case "thick":
		for x := 0; x < 10; x++ {
			for y := -2; y < 2; y++ {
				board.square[x][y+board.size/2].InitializeWalker(numParticles, numSteps, randomSteps)
			}
		}

	case "surface":
		for x := 0; x < 10; x++ {
			board.square[x][0].InitializeWalker(numParticles, numSteps, randomSteps)
			board.square[x][board.size].InitializeWalker(numParticles/2, numSteps, randomSteps)
		}

	case "slab":
		for x := 0; x < 10; x++ {
			for y := 0; y < board.size/2; y++ {
				board.square[x][y].InitializeWalker(numParticles, numSteps, randomSteps)
			}
		}

	default:
		fmt.Println("Error: no such diffusion mode")
		fmt.Println("Please enter a correct mode")
		os.Exit(1)
	}
}

//DiffuseMethod() can indicate the way how the random walkers move with different mode
func (board *DiffusionBoard) DiffuseMethod(mode string, yStart, yLimit int, randomSteps []int) {

	switch mode {
	case "surface":
		//loop over given range
		for x := 0; x < 10; x++ {
			for y := yStart; y < yLimit; y++ {
				//first we check if there has particle to move
				if board.square[x][y].numParticles > 0 {
					//flag will be "-" or "+"
					//randomly choose all parameters
					seed := time.Now().UTC().UnixNano()
					rand.Seed(seed)
					flag := board.square[x][y].ChooseDirection()
					dist := board.square[x][y].RandomStep(randomSteps)
					move := flag * dist

					//if move=0,deem it as "had moved"
					//if move !=0,then followed the concentration gredient
					if move == 0 {
						continue
					} else {
						if board.Infield(x, y+move) && board.AlongGradient(x, y, x, y+move) {
							//active a new square
							board.square[x][y+move].numParticles += 1
							board.square[x][y+move].randomSteps = randomSteps
							//the surface concentration remain constant
							if y != 0 && (y != board.size) {
								board.square[x][y].numParticles -= 1
							}
						}
					}
				}
			}
		}
		//for a semi-infinite slab, we will start our diffusion from "interface"
		//basically the same as "surface" mode
	case "slab":

		for x := 0; x < 10; x++ {
			//here is the difference between "surface" mode and "slab" mode
			for y := yLimit - 1; y >= 0; y-- {
				//first we check if there has particle to move
				if board.square[x][y].numParticles > 0 {
					//randomly choose all parameters
					//no longer to indicate directions
					seed := time.Now().UTC().UnixNano()
					rand.Seed(seed)
					flag := board.square[x][y].ChooseDirection()
					dist := board.square[x][y].RandomStep(randomSteps)
					move := flag * dist

					//if move=0,deem it as "had moved"
					//if move !=0,then followed the concentration gredient
					if move == 0 {
						continue
					} else {
						if board.Infield(x, y+move) && board.AlongGradient(x, y, x, y+move) {
							//active a new square
							//in this case, all random walkers will move right
							board.square[x][y+move].numParticles += 1
							board.square[x][y+move].randomSteps = randomSteps
							//no need to remain constant concentration
							board.square[x][y].numParticles -= 1

						}
					}
				}
			}
		}
	default:
		//for "thin" and "thick" mode
		//basically the same as above,except that the concentrations of origins will not be constant any more
		//no need to choose direction,it diffuses from the center to both sides
		for x := 0; x < 10; x++ {
			for y := yStart; y < yLimit; y++ {

				if board.square[x][y].numParticles > 0 {
					seed := time.Now().UTC().UnixNano()
					rand.Seed(seed)
					dist := board.square[x][y].RandomStep(randomSteps)
					move := dist

					//if move=0,deem it as "had moved"
					//if move !=0,then followed the concentration gredient
					if move == 0 {
						continue
					} else {
						if board.Infield(x, y+move) && board.AlongGradient(x, y, x, y+move) {
							//active a new square
							board.square[x][y+move].numParticles += 1
							board.square[x][y+move].randomSteps = randomSteps
							//here is the difference between thin/thick and surface mode
							board.square[x][y].numParticles -= 1
						}
					}
				}
			}
		}
	}
}

//after initialization, the random-walker can start moving in the board
func (board *DiffusionBoard) Diffuse(mode string, numSteps int, randomSteps []int) {

	timeLimit := numSteps
	//at each time step, all walkers present random walks
	for t := 1; t < timeLimit; t++ {
		board.DiffuseMethod(mode, 0, board.size/2, randomSteps)
		//use concurrency for random diffusion
		go board.DiffuseMethod(mode, board.size/2, board.size+1, randomSteps)
	}
}

//ParseConcentration() Method will parse the number of random walkers into concentration
func (board *DiffusionBoard) ParseConcentration(numParticles int) []float64 {

	total := numParticles * 10
	c := make([]float64, board.size+1)

	for y := 0; y < board.size+1; y++ {
		var sum int
		for x := 0; x < 10; x++ {
			sum += board.square[x][y].numParticles
		}
		c[y] = float64(sum) / float64(total)
	}

	return c
}

//output the concentration file
func (board *DiffusionBoard) OutputConcentration(numParticles, numSteps int, mode string, c []float64) {
	//first assign unique name
	filename := strconv.Itoa(numParticles) + "-" + strconv.Itoa(numSteps) + "-steps " + mode + " concentration-profile" + ".txt"
	//create a new file for storing data
	outfile, err := os.Create(filename)
	if err != nil {
		fmt.Println("Error: couldn't create the file")
		os.Exit(1)
	}
	//description of the file
	fmt.Fprintln(outfile, numParticles, "particles", numSteps, "steps random walk")
	fmt.Fprintln(outfile, "using", mode, "mode")
	fmt.Fprintln(outfile)
	for place, conc := range c {
		fmt.Fprintln(outfile, place, conc)
	}
	fmt.Fprintln(outfile, "have a nice day")
}

//GetNumParticles() Method can return the number of particles at certain square[r][c]
func (board *DiffusionBoard) GetNumParticles(r, c int) int {

	if !board.Infield(r, c) {
		fmt.Println("Error: not valid square")
		os.Exit(1)
	}
	return board.square[r][c].numParticles
}

//SetNumParticles() Method can be used to adjust the number of particles in certain square
func (board *DiffusionBoard) SetNumParticles(r, c, num int) {

	if !board.Infield(r, c) {
		fmt.Println("Error: not valid square")
		os.Exit(1)
	}
	board.square[r][c].numParticles = num
}

//diffusion will occure along with concentration gradient
//diffusing from high concentration to low concentration means "follow the gradient"
func (board *DiffusionBoard) AlongGradient(x1, y1, x2, y2 int) bool {
	switch {
	case board.square[x1][y1].numParticles > board.square[x2][y2].numParticles:
		return true
	//local equilibrium,diffusion did not occur
	default:
		return false
	}
}

/*************************************
functions for numerical analysis
**************************************/

//return the max value for a given slice
func MaxSlice(target []float64) float64 {

	if len(target) < 1 {
		fmt.Println("Error: empty slice")
		return -1
	}

	maxValue := target[0]
	for _, val := range target[1:] {
		if val >= maxValue {
			maxValue = val
		}
	}
	return maxValue
}

//return the min value for a given slice
func MinSlice(target []float64) float64 {

	if len(target) < 1 {
		fmt.Println("Error: empty slice")
		return -1
	}

	minValue := target[0]
	for _, val := range target[1:] {
		if val <= minValue {
			minValue = val
		}
	}
	return minValue
}

//occurance versus positions
func PlotPosition(histSize int, maxOccurance float64, finalPosition []float64) {

	//x limit is histSize
	w := histSize
	//y limit is maxOccurance, which is a relatively small number(around 0.1)
	h := int(maxOccurance*1000 + 1)
	//set necessary parameters
	pic := CreateNewCanvas(w, h)
	black := MakeColor(0, 0, 0)
	pic.SetStrokeColor(black)
	pic.SetLineWidth(1)

	for posit, occurance := range finalPosition {
		pic.LineTo(float64(posit), occurance*1000)
	}
	pic.Stroke()
	pic.SaveToPNG("occurance.png")
}

//mean square position versus timesteps
func PlotMeanSquare(meanSquareRecord []float64, maxMeanSquare float64, numSteps int) {

	//x axis is numSteps
	w := numSteps
	//in order to cover all mean square
	h := int(maxMeanSquare + 1.0)
	//set necessary parameters
	pic := CreateNewCanvas(w, h)
	black := MakeColor(0, 0, 0)
	pic.SetStrokeColor(black)
	pic.SetLineWidth(1)

	for step, position := range meanSquareRecord {
		pic.LineTo(float64(step), position)
	}
	pic.Stroke()
	pic.SaveToPNG("meanSquare-timeSteps.png")

}

//CompareValue() will return the number with greater abs value
func CompareValue(minPosition, maxPosition int) int {
	//first obtain absolute value
	min := math.Abs(float64(minPosition))
	max := math.Abs(float64(maxPosition))
	switch {
	case min >= max:
		return int(min)
	default:
		return int(max)
	}
}

//Divide() Method accept slice and divide by numParticles
func Divide(target []float64, numParticles int) []float64 {
	//first range over target value
	for idx, val := range target {
		val = val / float64(numParticles)
		//replace the original value
		target[idx] = val
	}
	return target
}

//use for regression anaylsis of mean square, assume linear-related(y=a*x+b)
func AnalyzeData(profile []float64) (float64, float64) {

	//initialize necessary parameters
	var sumX, sumY, sumX2, sumXY float64
	var meanX, meanY float64
	var slope, intersection float64
	n := float64(len(profile))

	//first step is calculating the necessary sum values
	for x, y := range profile {
		sumX += float64(x)
		sumY += y
		sumX2 += Square(float64(x))
		sumXY += float64(x) * y
	}

	meanX, meanY = sumX/n, sumY/n

	//based on regression analysis to get the value of slope,assume linear
	slope = (sumXY - n*meanX*meanY) / (sumX2 - n*Square(meanX))
	//then b=(y-a*x)
	intersection = meanY - slope*meanX

	return slope, intersection

}

//CalculateSlope() method will return the calculated slope of the mean square record
func CalculateSlope(randomSteps []int) float64 {
	//we need to consider frequency here
	var sum float64
	n := len(randomSteps)

	for _, val := range randomSteps {
		sum += Square(float64(val))
	}

	return (float64(sum) / float64(n))
}

//out put the position profile
func OutputPositionOccurance(histSize int, maxOccurance float64, finalPosition []float64) {

	filename := "occurance-position.txt"
	//create a new file for storing data
	outfile, err := os.Create(filename)
	if err != nil {
		fmt.Println("Error: couldn't create the file")
		os.Exit(1)
	}
	//description of the file
	fmt.Fprintln(outfile, "occurance versus positions")
	fmt.Fprintln(outfile, "maxOccurance:", maxOccurance, "max displacement :", histSize)
	fmt.Fprintln(outfile)
	for posit, freq := range finalPosition {
		fmt.Fprintln(outfile, posit, freq)
	}
	fmt.Fprintln(outfile, "have a nice day")
}

//here we output the random walk data (like positions),including calculated slope and numerical slope
func OutputRandomData(slopeN, slopeC float64, numSteps int, meanSquareRecord []float64) {
	//first assign unique name
	filename := strconv.Itoa(numSteps) + "-steps-RandomWalk" + ".txt"
	//create a new file for storing data
	outfile, err := os.Create(filename)
	if err != nil {
		fmt.Println("Error: couldn't create the file")
		os.Exit(1)
	}
	//description of the file
	fmt.Fprintln(outfile, "results of", numSteps, "steps random walk")
	fmt.Fprintln(outfile, "numerical slope:", slopeN, "theoretical slope:", slopeC)
	fmt.Fprintln(outfile)
	for timeStep, val := range meanSquareRecord {
		fmt.Fprintln(outfile, timeStep, val)
	}
	fmt.Fprintln(outfile, "have a nice day")
}

/*********************************
FTCS functions
**********************************/

//InitializeFTCS()will return a initialized concentration profile
func InitializeFTCS(c float64, numNodes int, dx float64) []float64 {
	profile := make([]float64, numNodes)
	//set initial concentraions
	for i := 1; i < numNodes; i++ {
		profile[i-1] = Concentration(i-1, dx)
	}
	//set boundary conditions
	profile[0] = c
	profile[numNodes-1] = c
	return profile
}

// y=-0.5*x^2+x+0.5, 0<=x<=2
func Concentration(i int, dx float64) float64 {
	if i < 0 {
		fmt.Println("Error: not valid i")
		os.Exit(1)
	}
	return -0.5*Square(float64(i-1)*dx) + float64((i-1))*dx + 0.5
}

//EvolveFTCS() Method will evolve the concentration profile as time passing
func EvolveFTCS(profile []float64, numSteps, numNodes int, c, r, r2 float64) []float64 {

	result := make([]float64, len(profile))
	//loop over time,each time step,except initial time
	for i := 1; i < numSteps; i++ {
		//update all positions except boundary
		for j := 1; j < numNodes-1; j++ {
			//forward time,central space method
			result[j] = r*profile[j-1] + r2*profile[j] + r*profile[j+1]
		}
		result[0] = c
		result[numNodes-1] = c
		for k := 0; k < len(profile); k++ {
			profile[k] = result[k]
		}

	}
	return result
}

//check if it's stable diffusion
func CheckStable(r float64) bool {
	switch {
	case r > 0.5:
		return false
	default:
		return true
	}
}

//check if concentration profile has reached steady state
func CheckSteady(profile []float64, err float64) bool {
	//if the difference between 0 and maxValue in profile is less than err
	max := MaxSlice(profile)
	min := MinSlice(profile)
	if math.Abs(max-min) <= err {
		return true
	} else {
		return false
	}
}

//to plot the curve of concentration profile
func VisualizeFTCS(profile []float64, dx float64, filename string) {
	//create new canvas to accommodate the FTCS profile after evolving
	//w for x coordinationsh, for y coordinations, the concentration
	w := 400
	h := 200
	//draw concentration profile
	pic := CreateNewCanvas(w, h)
	//set color for lines
	black := MakeColor(0, 0, 0)
	//draw the concentration profile with black lines
	pic.SetStrokeColor(black)
	//line width is 1
	pic.SetLineWidth(1)
	for i := 0; i < len(profile); i++ {
		pic.LineTo(float64(i)*dx*200, profile[i]*200)
	}
	pic.Stroke()
	pic.SaveToPNG("filename")

}

//output txt files for FTCS data,with different names
func OutputFTCS(numStep int, profile []float64) {
	//a unique name which distinguish files from each other
	filename := "FTCS-" + strconv.Itoa(numStep) + ".txt"
	//then we can output each file seperately
	outfile, err := os.Create(filename)
	//ensure that we create a valid file
	if err != nil {
		fmt.Println("Error: couldn't create the file")
		os.Exit(1)
	}
	//description for each file
	fmt.Fprintln(outfile, "the FTCS data after", numStep, "steps")
	//now writing the data to the file
	for idx, val := range profile {
		fmt.Fprintln(outfile, idx, val)
	}
	fmt.Fprintln(outfile, "have a nice day")
}

/*************************************
Other necessary functions
**************************************/

//check string containment
func Contains(mode string, list []string) bool {

	for _, target := range list {
		//when found,stop searching
		if mode == target {
			return true
		}
	}
	return false
}

//implement x^2
func Square(x float64) float64 {
	return x * x
}

func main() {

	//only accept limited optional modes
	modeList := []string{"ftcs", "thin", "thick", "surface", "slab"}
	//in case it has some upper-case letter
	mode := strings.ToLower(os.Args[1])
	//if mode not in modeList, which means incorrect mode
	if !Contains(mode, modeList) {
		fmt.Println("Error: no such mode")
		fmt.Println("Please select one mode : ftcs,thin,thick,surface and slab")
		os.Exit(1)
	}

	//numerical methods: ftcs,numNodes,numSteps,c
	//else: mode numParticles,numSteps,randomSteps1,randomSteps2..."
	if mode == "ftcs" {

		if len(os.Args) != 5 {
			fmt.Println("Error: numNodes,numSteps and c are needed")
			os.Exit(1)
		}

		//parse command line arguments,number of Nodes in x direction
		numNodes, err := strconv.Atoi(os.Args[2])
		if err != nil {
			fmt.Println("Error: numNodes must be integer")
			os.Exit(1)
		} else if numNodes <= 1 {
			fmt.Println("Error: numNodes must be greater than 1")
			os.Exit(1)
		}

		//number of time steps
		numSteps, err := strconv.Atoi(os.Args[3])
		if err != nil {
			fmt.Println("Error: numSteps must be integer")
			os.Exit(1)
		} else if numNodes <= 1 {
			fmt.Println("Error: numSteps must be greater than 1") //a bit different from random-walk
			os.Exit(1)
		}

		//boundary conditions
		c, err := strconv.ParseFloat(os.Args[4], 64)
		if err != nil {
			fmt.Println("concentration must be real number")
			os.Exit(1)
		} else if c < 0 {
			fmt.Println("concentrations must be nature number")
			os.Exit(1)
		}

		const (
			l     = 2    //length of domain in x direction
			tmax  = 50   //end time
			alpha = 0.05 //diffusion coefficient
		)

		dx := float64(l) / float64(numNodes-1)    //size of x increment
		dt := float64(tmax) / float64(numSteps-1) //size of t increment
		r := alpha * dt / (dx * dx)               //stable conditions
		r2 := 1 - 2*r

		//if not stable, calculations may go wrong
		if !CheckStable(r) {
			fmt.Println("Error: it's not a stable FTCS method, probably wrong!")
			fmt.Println("Please select a proper combination of numNodes and numSteps")
			os.Exit(3)
		}

		//obtain the initial concentration profile
		composition := InitializeFTCS(c, numNodes, dx)
		OutputFTCS(0, composition)

		//then evolve the concentration, with different stages
		for numStep := 5; numStep <= numSteps; numStep *= 10 {
			composition = EvolveFTCS(composition, numStep, numNodes, c, r, r2)
			OutputFTCS(numStep, composition)
		}

		//check if steady after "numSteps" steps, the standard is err=0.01
		if CheckSteady(composition, 0.001) {
			fmt.Println("After", numSteps, "steps steady state is reached")
		} else {
			fmt.Println("not yet in steady state, you may try more numSteps")
		}
		fmt.Println("Job is done, have a nice day!")

	} else {
		//if not "FTCS" mode
		if len(os.Args) < 5 {
			fmt.Println("Error: numParticles numSteps and at least two random steps are needed")
			os.Exit(1)
		}

		//parse numParticles,which should be positive
		numParticles, err := strconv.Atoi(os.Args[2])
		if err != nil {
			fmt.Println("Error: numParticles must be integer")
			os.Exit(1)
		} else if numParticles <= 0 {
			fmt.Println("Error: numParticles must be positive number")
			os.Exit(1)
		}

		//parse numSteps,a random walker can move numSteps times,which should be >=0
		numSteps, err := strconv.Atoi(os.Args[3])
		if err != nil {
			fmt.Println("Error: numSteps must be integer")
			os.Exit(1)
		} else if numSteps < 0 {
			fmt.Println("Error: numSteps must be positive number")
			os.Exit(1)
		}

		length := len(os.Args) - 4
		randomSteps := make([]int, length)

		//parse randomSteps, which are all integers that a random walker can move
		//random steps can be positive or negative or even zero
		for i := 4; i < len(os.Args); i++ {
			step, err := strconv.Atoi(os.Args[i])
			if err != nil {
				fmt.Println("Error: step must be integer")
				os.Exit(1)
			}
			//if correctly input random steps,then store it
			randomSteps[i-4] = step
		}

		/**************************************
		first part: changing positions
		**************************************/

		//initialize parameters for visulization
		var minPosition, maxPosition int
		var maxOccurance, maxMeanSquare float64
		//we will proceed numSteps,for mean square position
		//the record can be seen as time:position
		var meanSquareRecord = make([]float64, numSteps)
		//no matter moving in which way, still starts from 0
		meanSquareRecord[0] = 0

		//use comman-line argument to initialize the random walkers
		walker := new(Walker)
		walker.InitializeWalker(numParticles, numSteps, randomSteps)
		//maxDistance will be used for plotting
		maxDist := walker.GetMaxDist()
		//we also record the final position and its occurance
		//len([-maxDist...maxDist])=2*maxDist+1
		finalPosition := make([]float64, 2*maxDist+1)

		//delve into the relations between position and steps
		//we use walker.properties here for the sake that we may reset the original data
		for i := 1; i < walker.numParticles; i++ {

			//study particles seperately, position starts from 0
			var position int

			//except initial step, so j starts from 1
			for j := 1; j < walker.numSteps; j++ {
				//randomSteps is already initialized
				step := walker.RandomStep(walker.randomSteps)
				//increment the position by the random step
				position = walker.Move(position, step)
				//check global minimum or maximum position
				if position <= minPosition {
					minPosition = position
				} else if position >= maxPosition {
					maxPosition = position
				}
				meanSquareRecord[j] = meanSquareRecord[j] + Square(float64(position))
			}
			//certain position occurance+=1
			finalPosition[position+maxDist] = finalPosition[position+maxDist] + 1
		}
		//divide by numParticles to get mean square position
		meanSquareRecord = Divide(meanSquareRecord, walker.numParticles)
		//divide by numParticles to get probability of position
		finalPosition = Divide(finalPosition, walker.numParticles)

		histSize := CompareValue(minPosition, maxPosition) //set x axis length limit for position histogram
		maxOccurance = MaxSlice(finalPosition)             //set y axis length limit for position histogram
		maxMeanSquare = MaxSlice(meanSquareRecord)         //set y limit for mean square position plot
		//PlotMeanSquare(meanSquareRecord, maxMeanSquare, walker.numSteps)

		slopeN, _ := AnalyzeData(meanSquareRecord)                          //numerical slope, we don't care about the intersection
		slopeC := CalculateSlope(walker.randomSteps)                        // theoretical slope, calculated with randomSteps
		OutputRandomData(slopeN, slopeC, walker.numSteps, meanSquareRecord) //output the results for plotting
		//OutputPositionOccurance(histSize, maxOccurance, finalPosition)      //output the distance profile

		/**************************************
		seconde part: changing concentrations
		**************************************/

		board := new(DiffusionBoard)
		board = InitializeBoard(maxDist)
		board.InitializeWalkers(mode, walker.numParticles, walker.numSteps, walker.randomSteps)
		board.Diffuse(mode, numSteps, randomSteps)
		c := board.ParseConcentration(numParticles)
		board.OutputConcentration(numParticles, numSteps, mode, c)
		//for _, col := range board.square {
		//	fmt.Println(col)
		//}
		fmt.Println(c)

		/**************************************
		printing final conclusions
		**************************************/

		fmt.Println("numerical slope:", slopeN, "theoretical slope:", slopeC)
		fmt.Println("max moving distance:", maxMeanSquare)
		fmt.Println("maxOccurance:", maxOccurance, "max displacement:", histSize)
		fmt.Println("Job is done, have a nice day!")

	}
}
