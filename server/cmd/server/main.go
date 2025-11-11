package main

import (
	"encoding/json"
	"log"
	"net/http"

	"axiom/internal/game"

	"github.com/google/uuid"
	"github.com/samber/lo"
)

var app game.Game

type TileMap struct {
	Width  int   `json:"width"`
	Height int   `json:"height"`
	Tiles  []int `json:"tiles"`
}

type Object struct {
	ID   uuid.UUID `json:"id"`
	X    float64   `json:"x"`
	Y    float64   `json:"y"`
	Size float64   `json:"size"`
	Key  string    `json:"key"`
}

type GameState struct {
	Map         TileMap              `json:"map"`
	Objects     map[uuid.UUID]Object `json:"objects"`
	Projectiles map[uuid.UUID]Object `json:"projectiles"`
	Units       map[uuid.UUID]Object `json:"units"`
	Ticks       int                  `json:"tick"`
}

func init() {
	app = *game.NewGame()
}

func getStateHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	gameState := GameState{
		Map: TileMap{
			Width:  app.State.Map.Width,
			Height: app.State.Map.Height,
			Tiles: lo.Map(app.State.Map.Tiles, func(item game.TileKey, _ int) int {
				if item == game.Land {
					return 1
				}

				if item == game.Dirt {
					return 2
				}

				if item == game.Rock {
					return 3
				}

				return 0
			}),
		},
		Objects:     make(map[uuid.UUID]Object),
		Projectiles: make(map[uuid.UUID]Object),
		Units:       make(map[uuid.UUID]Object),
	}

	for _, o := range app.State.Objects {
		gameState.Objects[o.ID] = Object{
			ID:   o.ID,
			X:    o.X,
			Y:    o.Y,
			Size: o.Size,
			Key:  "unknown",
		}
	}

	if err := json.NewEncoder(w).Encode(gameState); err != nil {
		log.Printf("Error encoding state: %v", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
	}
}

func main() {
	http.HandleFunc("/", getStateHandler)
	log.Fatal(http.ListenAndServe(":8080", nil))
}
