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

	// Create a new plot, set its title and
	// axis labels.
	p := plot.New()

	// p.Title.Text = "Title"
	p.X.Label.Text = "# of concurrent connections"
	p.Y.Label.Text = "Mean CPU load in %"
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

	// Make a line plotter with points for the meanC measurement.
	lpLineC, lpPointsC, err := plotter.NewLinePoints(meanC)
	if err != nil {
		panic(err)
	}
	// Set the style.
	lpLineC.Color = color.RGBA{G: 255, A: 255}
	lpPointsC.Shape = draw.PyramidGlyph{}
	lpPointsC.Color = color.RGBA{R: 255, A: 255}

	// Make a line plotter with points for the meanGo measurement.
	lpLineGo, lpPointsGo, err := plotter.NewLinePoints(meanGo)
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
	if err := p.Save(4*vg.Inch, 4*vg.Inch, "meanCPU.pdf"); err != nil {
		panic(err)
	}
}
