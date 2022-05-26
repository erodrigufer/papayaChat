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

type results struct {
	// mean, mean value of all data points in a measurement.
	mean float64
	// variance, variance of all data points in a measurement.
	// variance float64
}

type application struct {
	// inputFile, file with input data.
	inputFile string
	// plotFile, name of file in which plot will be stored.
	plotFile string
	// outputFile, file in which mean value, variance and so on, will be stored
	// in text format.
	outputFile string
	// results, results struct of values that should be exported to outputFile.
	results results
}

func main() {

	app := new(application)

	flag.StringVar(&app.inputFile, "input", "", "File with input data to plot and analyze.")
	flag.StringVar(&app.plotFile, "plot", "", "File name to output plot.")
	flag.StringVar(&app.outputFile, "output", "", "File name to output results.")
	flag.Parse()

	if app.inputFile == "" || app.plotFile == "" || app.outputFile == "" {
		flag.Usage()
		os.Exit(-1)
	}

	// Open file with data.
	dataFile, err := os.Open(app.inputFile)
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
		// fmt.Printf("X: %f.\nY: %f.\n", pts[counter-1].X, pts[counter-1].Y)
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}

	// Finish calculation of mean value.
	app.results.mean = mean / float64(len(pts))

	//For debugging.
	fmt.Printf("Number of elements in slice: %d.\n", len(pts))

	fmt.Println("Mean: ", app.results.mean)

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
		log.Fatalln(err)
	}
	s.GlyphStyle.Color = color.RGBA{R: 255, B: 128, A: 255}

	var meanDataPoint plotter.XY
	meanDataPoint.X = pts[0].X
	meanDataPoint.Y = app.results.mean
	meanLine := make(plotter.XYs, 0, 2)
	meanLine = append(meanLine, meanDataPoint)
	meanDataPoint.X = pts[len(pts)-1].X
	meanDataPoint.Y = app.results.mean
	meanLine = append(meanLine, meanDataPoint)

	// Make a line plotter and set its style.
	l, err := plotter.NewLine(meanLine)
	if err != nil {
		log.Fatalln(err)
	}
	l.LineStyle.Width = vg.Points(1)
	l.LineStyle.Dashes = []vg.Length{vg.Points(5), vg.Points(5)}
	l.LineStyle.Color = color.RGBA{B: 255, A: 255}

	// Add the plotters to the plot.
	p.Add(s, l)
	//p.Legend.Add("Data", s)

	// Save the plot to an external file, use the first 2 parameters to
	// determine the font's width and height.
	if err := p.Save(4*vg.Inch, 4*vg.Inch, app.plotFile); err != nil {
		log.Fatalln(err)
	}

	if err := app.exportResults(); err != nil {
		log.Fatalln(err)
	}
}

func (app *application) exportResults() error {
	// Transform mean into string.
	meanString := strconv.FormatFloat(app.results.mean, 'f', 2, 64)
	err := os.WriteFile(app.outputFile, []byte(meanString), 0644)
	if err != nil {
		return err
	}
	return nil
}
