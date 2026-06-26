package main

import (
	"log"
	"os"

	"axiom/internal/app"
)

func main() {
	app, err := app.NewApp(os.Args[1], os.Args[2])
	if err != nil {
		log.Fatalf("Failed to create app: %v", err)
	}
	_ = app.Run(":8000")
}
