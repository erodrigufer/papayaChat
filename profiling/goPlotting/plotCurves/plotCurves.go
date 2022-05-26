package main

import (
	"bufio"
	"flag"
	"fmt"
	"image/color"
	"log"
	"math"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
)

type results struct {
	// mean, mean value of all data points in a measurement.
	mean float64
	// variance, variance of all data points in a measurement.
	variance float64
	// standardDeviation
	standardDeviation float64
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
	var sumOfCPUUsage float64 = 0.0
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
		sumOfCPUUsage += CPUUsage
	}

	if err := scanner.Err(); err != nil {
		fmt.Fprintln(os.Stderr, "reading standard input:", err)
	}

	// Calculate mean value.
	app.results.mean = sumOfCPUUsage / float64(len(pts))
	// Calculate variance.
	app.results.variance = calculateVariance(app.results.mean, pts)
	// Calculate standard deviation.
	app.results.standardDeviation = calculateStandardDeviation(app.results.variance)

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

	// Create a datapoint for the mean line at the first X value with the mean
	// value as Y.
	var meanDataPoint plotter.XY
	meanDataPoint.X = pts[0].X
	meanDataPoint.Y = app.results.mean
	// Create a slice to store two mean values and create a line.
	meanLine := make(plotter.XYs, 0, 2)
	// Append first mean value.
	meanLine = append(meanLine, meanDataPoint)
	// Create the second point at the last value in the X axis and append it.
	meanDataPoint.X = pts[len(pts)-1].X
	meanDataPoint.Y = app.results.mean
	meanLine = append(meanLine, meanDataPoint)

	// Make a line plotter and set its style.
	lineMean, err := plotter.NewLine(meanLine)
	if err != nil {
		log.Fatalln(err)
	}
	lineMean.LineStyle.Width = vg.Points(1)
	lineMean.LineStyle.Dashes = []vg.Length{vg.Points(5), vg.Points(5)}
	lineMean.LineStyle.Color = color.RGBA{B: 255, A: 255}

	sdPos := createLine(app.results.standardDeviation+app.results.mean, pts)

	// Make a line plotter and set its style.
	sdPosLine, err := plotter.NewLine(sdPos)
	if err != nil {
		log.Fatalln(err)
	}
	sdPosLine.LineStyle.Width = vg.Points(1)
	sdPosLine.LineStyle.Dashes = []vg.Length{vg.Points(5), vg.Points(5)}
	sdPosLine.LineStyle.Color = color.RGBA{R: 255, G: 153, B: 0, A: 255}

	// Add the plotters to the plot.
	p.Add(s, lineMean, sdPosLine)
	//p.Legend.Add("Data", s)

	// Save the plot to an external file, use the first 2 parameters to
	// determine the font's width and height.
	if err := p.Save(4*vg.Inch, 4*vg.Inch, app.plotFile); err != nil {
		log.Fatalln(err)
	}

	// Export results to text file.
	if err := app.exportResults(); err != nil {
		log.Fatalln(err)
	}
}

// exportResults, stores the results of processing each measurement into an
// output file (app.outputFile).
func (app *application) exportResults() error {
	// Transform mean into string with two numbers after the decimal point.
	precision := 2
	meanString := strconv.FormatFloat(app.results.mean, 'f', precision, 64)
	// Get basename from measurement and remove file extension (e.g. .pdf).
	measurementName := strings.TrimSuffix(filepath.Base(app.plotFile), filepath.Ext(filepath.Base(app.plotFile)))
	// Transform variance into string with four numbers after the decimal point.
	varianceString := strconv.FormatFloat(app.results.variance, 'f', 4, 64)

	// Transform standard deviation into string with four numbers after the
	// decimal point.
	sdString := strconv.FormatFloat(app.results.standardDeviation, 'f', 4, 64)

	// Create output string for results output file.
	outputString := fmt.Sprintf("%s\t\t%s\t%s\t%s\n", measurementName, meanString, varianceString, sdString)

	// If the file doesn't exist, create it, or append to the file.
	f, err := os.OpenFile(app.outputFile, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		return err
	}
	// Write to the file.
	if _, err := f.Write([]byte(outputString)); err != nil {
		f.Close()
		return err
	}
	// Close file after writing.
	if err := f.Close(); err != nil {
		return err
	}
	return nil
}

// calculateVariance, calculates the variance of a plotter.XYs object.
// The variance is defined as the addition of all deviations from the mean,
// divided by the total number of data points in plotter.XYs.
func calculateVariance(mean float64, pts plotter.XYs) float64 {
	// Variable to add up all deviations from the mean for all datapoints.
	var deviations float64 = 0.0

	for _, dataPoint := range pts {
		// Add up all the deviations from the mean for all datapoints.
		// A deviation from the mean is the difference of a datapoint from
		// the mean to the power of two.
		deviations += (dataPoint.Y - mean) * (dataPoint.Y - mean)

		// For debugging.
		// fmt.Printf("------------------\ndataPoint.Y: %f.\nmean: %f.\ndeviations: %f.\n", dataPoint.Y, mean, deviations)

	}

	// The variance equals the division of the sum of the deviations by
	// the total number of data points.
	variance := deviations / float64(len(pts))

	return variance

}

// calculateStandardDeviation, given a variance as a parameter, calculate its
// standard deviation.
func calculateStandardDeviation(variance float64) float64 {
	return math.Sqrt(variance)
}

// createLine, creates a straight line through out the
func createLine(value float64, pts plotter.XYs) plotter.XYs {
	var points plotter.XY
	points.X = pts[0].X
	points.Y = value
	// Create a slice where the two points for the line will be stored.
	line := make(plotter.XYs, 0, 2)
	// Append first data points.
	line = append(line, points)
	// Create the second point at the last value in the X axis and append it.
	points.X = pts[len(pts)-1].X
	points.Y = value
	line = append(line, points)

	return line

}
