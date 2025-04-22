package plotter

import (
	"fmt"

	"github.com/gizak/termui/v3"
	"github.com/gizak/termui/v3/widgets"
)

// ==================================================

type WidgetFormat struct {
	X0, X1 int
	Y0, Y1 int
}

func NewWidgetFormat(startX, startY int, w, h int) WidgetFormat {
	return WidgetFormat{
		X0: startX,
		Y0: startY,
		X1: startX + w,
		Y1: startY + h,
	}
}

func listParameter(name, fmt string) string {
	return name + ": " + fmt
}

// ==================================================

type PlotterMatrix struct {
	matrix [][]float64
	tmpBuf []float64
	depth  int
}

func NewPlotterMatrix(depth, len int) *PlotterMatrix {
	matrix := make([][]float64, depth)
	for i := 0; i < depth; i++ {
		matrix[i] = make([]float64, len)
	}
	return &PlotterMatrix{
		tmpBuf: make([]float64, len, len),
		matrix: matrix,
		depth:  depth,
	}
}

type VoltageEmmiter interface {
	Voltages() []float64
}

func (pm *PlotterMatrix) Push(pos int, data VoltageEmmiter) (ok bool) {
	if pm.depth < pos+1 {
		return false
	}

	destLen := len(pm.matrix[pos])
	src := data.Voltages()

	var x0, x1 int
	var x, weight, scale float64

	if len(src) == 0 || destLen == 0 {
		return false
	}

	scaled := make([]float64, destLen)
	scale = float64(len(src)-1) / float64(destLen-1)

	for i := 0; i < destLen; i++ {

		x = float64(i) * scale
		x0 = int(x)
		x1 = x0 + 1

		if x1 >= len(src) {
			x1 = x0
		}

		weight = x - float64(x0)
		scaled[i] = src[x0]*(1-weight) + src[x1]*weight

	}

	copy(pm.matrix[pos], scaled)
	return true
}

func (pm *PlotterMatrix) Data() [][]float64 {
	return pm.matrix
}

func GraphWidget(mat *PlotterMatrix, format WidgetFormat) *widgets.Plot {
	p0 := widgets.NewPlot()
	p0.Title = "Graphic probes (Voltage)"
	p0.SetRect(format.X0, format.Y0, format.X1, format.Y1)
	p0.AxesColor = termui.ColorWhite
	p0.Data = mat.Data()
	return p0
}

// =========================================

type WaveMetrics struct {
	Widget *widgets.List
}

func NewWaveMetrics(format WidgetFormat) WaveMetrics {

	mt := widgets.NewList()
	mt.Title = "Wave Metrics"
	mt.SetRect(format.X0, format.Y0, format.X1, format.Y1)
	mt.Rows = make([]string, 6)
	mt.TextStyle = termui.NewStyle(termui.ColorGreen)

	mt.Rows[0] = listParameter("Name", "Value")

	return WaveMetrics{Widget: mt}
}

func (wm WaveMetrics) UpdateMetrics(pk PlotterPacket) {
	rws := wm.Widget.Rows

	rws[1] = listParameter(" - RMS", fmt.Sprintf("%.2fV", pk.ToVolts(pk.RMS())))
	rws[2] = listParameter(" - Duty cycle", fmt.Sprintf("%d%%", pk.AutoDutyCycle()))
	rws[3] = listParameter(" - Average", fmt.Sprintf("%.2fV", pk.ToVolts(pk.Average())))
	rws[4] = listParameter(" - Maximum", fmt.Sprintf("%.2fV", pk.ToVolts(pk.Maximum())))
	rws[5] = listParameter(" - Minimum", fmt.Sprintf("%.2fV", pk.ToVolts(pk.Minimum())))
}

// =========================================

type PlotInfo struct {
	Widget *widgets.List
}

func NewPlotInfo(format WidgetFormat) PlotInfo {

	mt := widgets.NewList()
	mt.Title = "Plotter info"
	mt.SetRect(format.X0, format.Y0, format.X1, format.Y1)
	mt.Rows = make([]string, 6)
	mt.TextStyle = termui.NewStyle(termui.ColorGreen)

	mt.Rows[0] = listParameter("Name", "Value")

	return PlotInfo{Widget: mt}
}

func (wm PlotInfo) UpdateMetrics(pk PlotterPacket, zoom int) {
	rws := wm.Widget.Rows

	rws[1] = listParameter(" - Sample rate", fmt.Sprintf("%dKSmpl/s", pk.SampleRate/1000))
	rws[2] = listParameter(" - Probes count", fmt.Sprint(pk.Length))
	rws[3] = listParameter(" - Unwrap duration", pk.UnwrapDuration().String())
	rws[4] = listParameter(" - Unwrap frequency", fmt.Sprintf("%dHz", pk.UnwrapFreq()))
	rws[5] = listParameter(" - Plotter zoom", fmt.Sprintf("%dX", zoom))
}
