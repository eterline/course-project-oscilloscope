package plotter

import (
	"context"
	"fmt"

	"github.com/eterline/oscyllo-tool/internal/services/transmit"
	"github.com/gizak/termui/v3"
)

type PlotReader interface {
	StartReading(ctx context.Context) <-chan transmit.SerialMessage
}

func RunPlotter(ctx context.Context, rd PlotReader, stop func()) error {

	if rd == nil || ctx.Err() != nil {
		return nil
	}

	rdChan := rd.StartReading(ctx)
	voltmatrix := NewPlotterMatrix(1, 256)

	voltGraph := GraphWidget(voltmatrix, NewWidgetFormat(0, 0, 100, 32))
	waveMetrics := NewWaveMetrics(NewWidgetFormat(0, 32, 40, 8))
	plotInfo := NewPlotInfo(NewWidgetFormat(40, 32, 60, 8))

	scale := InitPlotterScaler(10)

	// ----------------------

	eventCh := make(chan termui.Event)
	defer close(eventCh)

	if err := termui.Init(); err != nil {
		return fmt.Errorf("plotter error: %w", err)
	}
	defer termui.Close()

	termui.Render(voltGraph, waveMetrics.Widget, plotInfo.Widget)

	// ----------------------

	go func() {
		for e := range termui.PollEvents() {
			eventCh <- e
		}
	}()

	for {
		select {
		case e := <-eventCh:
			if e.Type == termui.KeyboardEvent {
				switch e.ID {
				case "q", "<C-c>":
					stop()
					return nil

				case "u":
					scale.Up()

				case "d":
					scale.Down()

				case "r":
					scale.Reset()

				}
			}

		case <-ctx.Done():
			stop()
			return nil

		case packet, ok := <-rdChan:
			if !ok {
				return fmt.Errorf("data channel closed")
			}

			zoom := scale.Value()

			if data, err := ParsePacket(packet, zoom); err == nil {
				voltmatrix.Push(0, data)

				waveMetrics.UpdateMetrics(data)
				plotInfo.UpdateMetrics(data, zoom)

				termui.Render(voltGraph, waveMetrics.Widget, plotInfo.Widget)
				continue
			}
		}
	}
}
