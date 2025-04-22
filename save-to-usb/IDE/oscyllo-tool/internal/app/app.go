package app

import (
	"fmt"

	"github.com/eterline/oscyllo-tool/internal/services/plotter"
	"github.com/eterline/oscyllo-tool/internal/services/transmit"
	"github.com/eterline/oscyllo-tool/pkg/toolkit"
)

func Execute(ctx *toolkit.AppStarter) {

	port, err := transmit.SelectPort(ctx.Context)
	if err != nil {
		fmt.Printf("\nError: %v\n", err)
	}

	baud, err := transmit.SelectBaud(ctx.Context)
	if err != nil {
		fmt.Printf("\nError: %v\n", err)
	}

	conn, err := transmit.NewSerialConnector(baud, port, '\n')
	if err != nil {
		fmt.Printf("\nError: %v\n", err)
		ctx.StopApp()
	}

	if err := plotter.RunPlotter(ctx.Context, conn, ctx.StopApp); err != nil {
		fmt.Printf("\nError: %v\n", err)
	}

	ctx.Wait()
}
