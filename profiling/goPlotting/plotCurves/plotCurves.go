package main

import (
	"bufio"
	"flag"
	"fmt"
	"image/color"
	"log"
	"os"
	"strconv"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

// type results struct {
// 	mean float64
// 	variance float64
// 	// Name of measurement.
// 	name string
// 	// Number of data-points for measurement.
// 	numberOfPoints int
// }

func main() {
	// Name of file from where data is extracted.
	var fileName string
	// Name of file where plot is stored (the file extension determines the plot
	// file type, e.g. .svg, .pdf, .png, .jpeg, etc.).
	var plotOutput string
	flag.StringVar(&fileName, "file", "", "File to plot and analyze.")
	flag.StringVar(&plotOutput, "o", "", "File name to output plot.")
	flag.Parse()

	if fileName == "" {
		log.Fatalln("-file flag missing.")
	}

	if plotOutput == "" {
		log.Fatalln("-o flag missing.")
	}

	// Open file with data.
	dataFile, err := os.Open(fileName)
	if err != nil {
		log.Fatalln(err)
	}
	defer dataFile.Close()

	// Scan data from file into 'plotter.XYs' data structure, and calculate mean
	// value of measurement.
	var mean float64 = 0.0
	var counter int = 0
	// Make slice of structs with two floats, that contain the whole scatter
	// points to plot.
	pts := make(plotter.XYs, 200)
	scanner := bufio.NewScanner(dataFile)
	// Scan one line of the file until EOF.
	for scanner.Scan() {
		//fmt.Println(scanner.Text())
		// Transform data as string into floats 64.
		dataPoint, err := strconv.ParseFloat(scanner.Text(), 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "strconv to float64", err)
			// Scan next line after error.
			continue
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
	if err := p.Save(4*vg.Inch, 4*vg.Inch, plotOutput); err != nil {
		panic(err)
	}
}
