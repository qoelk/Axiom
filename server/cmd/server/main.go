// cmd/server/main.go
package main

import (
	"log"
	"net/http"

	"axiom/internal/game"
	"axiom/internal/routes"
)

func main() {
	app := game.NewGame()
	routes.RegisterRoutes(app)

	log.Println("Starting server on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
