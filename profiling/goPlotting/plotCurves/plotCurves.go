package main

import (
	"bufio"
	"fmt"
	"image/color"
	"log"
	"os"
	"strconv"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

func main() {
	dataFile, err := os.Open("./1cpu1c1Mb.gom.cl")
	if err != nil {
		log.Fatalln(err)
	}
	defer dataFile.Close()
	var mean float64 = 0.0
	var counter int = 0
	pts := make(plotter.XYs, 200)
	scanner := bufio.NewScanner(dataFile)
	for scanner.Scan() {
		//fmt.Println(scanner.Text())
		dataPoint, err := strconv.ParseFloat(scanner.Text(), 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "strconv to float64", err)
		}
		pts[counter].X = float64(counter)
		pts[counter].Y = dataPoint
		counter++
		mean += dataPoint
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}

	fmt.Println("Mean: ", mean/float64(counter))

	// Create a new plot, set its title and
	// axis labels.
	p := plot.New()

	p.Title.Text = "CPU load for C program with 2 connections"
	p.X.Label.Text = "t"
	p.Y.Label.Text = "CPU load in %"
	// Draw a grid behind the data.
	p.Add(plotter.NewGrid())

	// Make a scatter plotter and set its style.
	s, err := plotter.NewScatter(pts)
	if err != nil {
		panic(err)
	}
	s.GlyphStyle.Color = color.RGBA{R: 255, B: 128, A: 255}

	// Add the plotters to the plot, with a legend
	// entry for each.
	p.Add(s)
	//p.Add(s, l, lpLine, lpPoints)
	p.Legend.Add("Data", s)

	// Save the plot to a PNG file.
	if err := p.Save(4*vg.Inch, 4*vg.Inch, "data.pdf"); err != nil {
		panic(err)
	}
}

// randomPoints returns some random x, y points.
// func randomPoints(n int) plotter.XYs {
// 	pts := make(plotter.XYs, n)
// 	for i := range pts {
// 		if i == 0 {
// 			pts[i].X = rand.Float64()
// 		} else {
// 			pts[i].X = pts[i-1].X + rand.Float64()
// 		}
// 		pts[i].Y = pts[i].X + 10*rand.Float64()
// 	}
// 	return pts
// }
