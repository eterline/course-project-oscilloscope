package main

import (
	"github.com/eterline/oscyllo-tool/internal/app"
	"github.com/eterline/oscyllo-tool/pkg/toolkit"
)

func main() {
	root := toolkit.InitAppStart(
		func() error {
			return nil
		},
	)

	app.Execute(root)
}
