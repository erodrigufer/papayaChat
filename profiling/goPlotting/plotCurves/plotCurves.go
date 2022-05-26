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
	// Make a slice of structs with two floats (plotter.XYs), that contain the
	// whole scatter points to plot.
	pts := make(plotter.XYs, 0, 200)
	scanner := bufio.NewScanner(dataFile)
	// Scan one line of the file until EOF.
	for scanner.Scan() {
		// Transform data as string into floats 64. Return one line at the time,
		// until \n is encountered (scanner.Text()), of the text previously
		// scanned with scanner.Scan().
		CPUUsage, err := strconv.ParseFloat(scanner.Text(), 64)
		if err != nil {
			fmt.Fprintln(os.Stderr, "strconv to float64", err)
			// Scan next line after error.
			continue
		}
		var dataPoint plotter.XY
		dataPoint.X = float64(counter+1) * 0.5 // time in s
		dataPoint.Y = CPUUsage
		pts = append(pts, dataPoint)
		counter++
		mean += CPUUsage
		// For debugging.
		fmt.Printf("X: %f.\nY: %f.\n", pts[counter-1].X, pts[counter-1].Y)
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}

	// Finish calculation of mean value.
	mean = mean / float64(len(pts))
	//For debugging.
	fmt.Printf("Number of elements in slice: %d.\n", len(pts))

	fmt.Println("Mean: ", mean)

	// Create a new plot, set its title and
	// axis labels.
	p := plot.New()

	p.Title.Text = ""
	p.X.Label.Text = "t in s"
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
	//p.Legend.Add("Data", s)

	// Save the plot to a PNG file.
	if err := p.Save(4*vg.Inch, 4*vg.Inch, plotOutput); err != nil {
		panic(err)
	}
}
