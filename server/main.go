package main

import (
	"encoding/json"
	"log"
	"net/http"
	"sync"
	"time"

	"github.com/ojrac/opensimplex-go"
)

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
}

type GameState struct {
	Map      TileMap        `json:"map"`
	Entities map[int]Entity `json:"entities"`
	Ticks    int            `json:"tick"`
}

func (gs *GameState) Tick() {
	gs.Ticks++
	for id, entity := range gs.Entities {
		entity.Facing += 0.01
		gs.Entities[id] = entity // update the map with the modified copy
	}
}

var (
	globalState GameState
	stateMutex  sync.RWMutex
)

func init() {
	// Initialize once at startup
	width, height := 128, 128
	tiles := make([]int, width*height)
	noise := opensimplex.New(time.Now().UnixNano()) // true randomness

	scale := 0.03
	threshold := 0.0

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			nx := float64(x) * scale
			ny := float64(y) * scale
			value := noise.Eval2(nx, ny)
			tiles[y*width+x] = 0
			if value > threshold {
				tiles[y*width+x] = 1
			}
		}
	}

	globalState = GameState{
		Map: TileMap{
			Width:  width,
			Height: height,
			Tiles:  tiles,
		},
		Entities: map[int]Entity{
			123: {ID: 123, X: float64(width / 2), Y: float64(height / 2), Facing: 0},
		},
		Ticks: 0,
	}
}

func getStateHandler(w http.ResponseWriter, r *http.Request) {
	stateMutex.RLock()
	defer stateMutex.RUnlock()

	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(globalState); err != nil {
		log.Printf("Error encoding state: %v", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
	}
	log.Println("state requested")
}

func tickHandler() {
	stateMutex.Lock()
	globalState.Tick()
	stateMutex.Unlock()
}

func main() {
	go func() {
		ticker := time.NewTicker(50 * time.Millisecond) // 1000ms / 20 = 50ms
		defer ticker.Stop()
		for range ticker.C {
			tickHandler()
		}
	}()
	http.HandleFunc("/", getStateHandler)
	log.Println("Server starting on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
