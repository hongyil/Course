package main

import (
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"
)

func main() {
	// check argument numbers
	if len(os.Args) != 6 {
		fmt.Println("Error: Please enter values of WIDTH HEIGHT STEPSIZE NUMBER-OF-STEPS SEED")
		os.Exit(1)
	}

	//check arguments boundary
	WIDTH, err := strconv.ParseFloat(os.Args[1], 64)
	if WIDTH <= 0 {
		fmt.Println("Error: WIDTH must be positive number")
		os.Exit(1)
	}

	HEIGHT, err := strconv.ParseFloat(os.Args[2], 64)
	if HEIGHT <= 0 {
		fmt.Println("Error: HEIGHT must be positive number")
		os.Exit(1)
	}

	STEPSIZE, err := strconv.ParseFloat(os.Args[3], 64)
	if STEPSIZE <= 0 {
		fmt.Println("Error: STEPSIZE must be positive number")
		os.Exit(1)
	}

	NUMBER_OF_STEPS, err := strconv.Atoi(os.Args[4])
	if err != nil {
		fmt.Println("Error: NUMBER-OF-STEPS must be integer number")
		os.Exit(1)
	} else if NUMBER_OF_STEPS <= 0 {
		fmt.Println("Error: NUMBER-OF-STEPS must be positive number")
		os.Exit(1)
	}

	SEED, err := strconv.ParseInt(os.Args[5], 10, 64)
	if err != nil {
		fmt.Println("Error: SEED must be integer number")
		os.Exit(1)
	} else if SEED <= 0 {
		fmt.Println("Error: SEED must be positive number")
		os.Exit(1)
	}

	randomwalk(WIDTH, HEIGHT, STEPSIZE, NUMBER_OF_STEPS, SEED)

}

//ensure in width*height field
func InField(a, b, width, height float64) bool {
	switch {
	case a <= 0 || a > width:
		return false
	case b <= 0 || b > height:
		return false
	default:
		return true
	}
}

//perform one step when can move
func RandomStep(x, y, WIDTH, HEIGHT, STEPSIZE float64) (float64, float64) {
	var a, b float64 = x, y
	for (a == x && b == y) || !InField(a, b, WIDTH, HEIGHT) {
		RandomDelta := rand.Float64() * 2 * math.Pi
		a = x + STEPSIZE*math.Cos(RandomDelta)
		b = y + STEPSIZE*math.Sin(RandomDelta)
	}
	return a, b
}

//a function will return the distance between two points
func distance(endX, endY, startX, startY float64) float64 {
	DistanceX := math.Pow((endX - startX), 2)
	DistanceY := math.Pow((endY - startY), 2)
	return math.Sqrt(DistanceX + DistanceY)
}

func randomwalk(WIDTH, HEIGHT, STEPSIZE float64, NUMBER_OF_STEPS int, SEED int64) {

	rand.Seed(SEED)
	var x, y float64 = WIDTH / 2, HEIGHT / 2
	fmt.Println(x, y)
	for i := 0; i < NUMBER_OF_STEPS; i++ {
		x, y = RandomStep(x, y, WIDTH, HEIGHT, STEPSIZE)
		fmt.Println(x, y)
	}
	fmt.Println("Distance = ", distance(x, y, WIDTH/2, HEIGHT/2))
}
