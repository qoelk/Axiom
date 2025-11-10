package main

import (
	"encoding/json"
	"log"
	"net/http"

	"axiom/internal/game"

	"github.com/samber/lo"
)

var app game.Game

type TileMap struct {
	Width  int   `json:"width"`
	Height int   `json:"height"`
	Tiles  []int `json:"tiles"`
}

type Entity struct {
	ID     int     `json:"id"`
	X      float64 `json:"x"`
	Y      float64 `json:"y"`
	Facing float64 `json:"facing"`
	Type   string  `json:"type"`
}

type GameState struct {
	Map     TileMap        `json:"map"`
	Objects map[int]Entity `json:"entities"`
	Ticks   int            `json:"tick"`
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
		Objects: make(map[int]Entity),
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
