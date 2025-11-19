// internal/routes/routes.go
package routes

import (
	"net/http"

	"axiom/internal/game"
	"axiom/internal/handlers"
)

func RegisterRoutes(app *game.Game) {
	handler := handlers.NewGameHandler(app)

	http.HandleFunc("/state", handler.GetState)
	http.HandleFunc("/map", handler.GetMap)
	http.HandleFunc("/unit/facing", handler.SetUnitFacing)
	http.HandleFunc("/unit/move", handler.MoveToPoint)
}
