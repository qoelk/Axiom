// internal/handlers/game.go
package handlers

import (
	"encoding/json"
	"log"
	"net/http"

	"axiom/internal/game"
	"axiom/internal/services"

	"github.com/google/uuid"
)

type SetFacingRequest struct {
	UnitID uuid.UUID `json:"unit_id"`
	Facing float64   `json:"facing"`
}

type MoveToPointRequest struct {
	UnitID uuid.UUID `json:"unit_id"`
	X float64 `json:"x"`
	Y float64 `json:"y"`
}
type GameHandler struct {
	App *game.Game
}

func NewGameHandler(app *game.Game) *GameHandler {
	return &GameHandler{App: app}
}

func (h *GameHandler) GetState(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	gameState := services.BuildFullGameState(h.App)

	if err := json.NewEncoder(w).Encode(gameState); err != nil {
		log.Printf("Error encoding full game state: %v", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func (h *GameHandler) GetMap(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	gameState := services.BuildMapOnly(h.App)

	if err := json.NewEncoder(w).Encode(gameState); err != nil {
		log.Printf("Error encoding map state: %v", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func (h *GameHandler) SetUnitFacing(w http.ResponseWriter, r *http.Request) {
	var req SetFacingRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	if err := services.UpdateUnitFacing(h.App, req.UnitID, req.Facing); err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		return
	}

	w.WriteHeader(http.StatusNoContent) // 204 No Content
}
func (h *GameHandler) MoveToPoint(w http.ResponseWriter, r *http.Request) {
	var req MoveToPointRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON", http.StatusBadRequest)
		return
	}

	if err := services.MoveToPoint(h.App, req.UnitID, req.X, req.Y); err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		return
	}

	w.WriteHeader(http.StatusNoContent) // 204 No Content
}
