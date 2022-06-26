package main

import (
	"image/color"

	"gonum.org/v1/plot"
	"gonum.org/v1/plot/plotter"
	"gonum.org/v1/plot/vg"
	"gonum.org/v1/plot/vg/draw"
)

func main() {
	meanC := plotter.XYs{{1, 0.34}, {2, 0.45}, {4, 0.79}, {8, 1.37}, {16, 2.17}, {32, 2.53}, {64, 2.53}}
	meanGo := plotter.XYs{{1, 1.94}, {2, 2.61}, {4, 4.35}, {8, 6.15}, {16, 11.71}, {32, 22.71}, {64, 36.08}}

	plotValues(meanC, meanGo, "# of concurrent connections", "Mean CPU load in %", "mean.pdf")

	cvC := plotter.XYs{{1, 0.159}, {2, 0.221}, {4, 0.228}, {8, 0.137}, {16, 0.147}, {32, 0.172}, {64, 0.180}}
	cvGo := plotter.XYs{{1, 0.169}, {2, 0.205}, {4, 0.172}, {8, 0.276}, {16, 0.229}, {32, 0.322}, {64, 0.243}}

	plotValues(cvC, cvGo, "# of concurrent connections", "Coefficient of variation", "cv.pdf")
}

func plotValues(cValues, goValues plotter.XYs, titleX, titleY, outputFile string) {
	// Create a new plot, set its title and
	// axis labels.
	p := plot.New()

	// p.Title.Text = "Title"
	p.X.Label.Text = titleX
	p.Y.Label.Text = titleY
	// Draw a grid behind the data.
	p.Add(plotter.NewGrid())

	// Make a scatter plotter and set its style.
	// s, err := plotter.NewScatter(scatterData)
	// if err != nil {
	// 	panic(err)
	// }
	// s.GlyphStyle.Color = color.RGBA{R: 255, B: 128, A: 255}

	// 	// Make a line plotter and set its style.
	// 	l, err := plotter.NewLine(meanC)
	// 	if err != nil {
	// 		panic(err)
	// 	}
	// 	l.LineStyle.Width = vg.Points(1)
	// 	l.LineStyle.Dashes = []vg.Length{vg.Points(5), vg.Points(5)}
	// 	l.LineStyle.Color = color.RGBA{B: 255, A: 255}

	// Make a line plotter with points for the cValues measurement.
	lpLineC, lpPointsC, err := plotter.NewLinePoints(cValues)
	if err != nil {
		panic(err)
	}
	// Set the style.
	lpLineC.Color = color.RGBA{G: 255, A: 255}
	lpPointsC.Shape = draw.PyramidGlyph{}
	lpPointsC.Color = color.RGBA{R: 255, A: 255}

	// Make a line plotter with points for the goValues measurement.
	lpLineGo, lpPointsGo, err := plotter.NewLinePoints(goValues)
	if err != nil {
		panic(err)
	}
	// Set the style.
	lpLineGo.Color = color.RGBA{B: 255, A: 255}
	lpPointsGo.Shape = draw.CircleGlyph{}
	lpPointsGo.Color = color.RGBA{R: 255, A: 255}

	// Add the plotters to the plot, with a legend entry for each.
	p.Add(lpLineC, lpPointsC, lpLineGo, lpPointsGo)
	//p.Add(s, l, lpLine, lpPoints)
	p.Legend.Add("C", lpPointsC, lpLineC)
	p.Legend.Add("Go", lpPointsGo, lpLineGo)
	//p.Legend.Add("line", l)
	// p.Legend.Add("line points", lpLine, lpPoints)

	// Save the plot to a PDF file.
	if err := p.Save(4*vg.Inch, 4*vg.Inch, outputFile); err != nil {
		panic(err)
	}
}
