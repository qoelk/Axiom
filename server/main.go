package main

import (
	"encoding/json"
	"log"
	"math"
	"math/rand/v2"
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
	Type   string  `json:"type"`
}

type GameState struct {
	Map      TileMap        `json:"map"`
	Entities map[int]Entity `json:"entities"`
	Ticks    int            `json:"tick"`
}

func (gs *GameState) Tick() {
	gs.Ticks++
	for id, entity := range gs.Entities {
		entity.Facing += 1
		gs.Entities[id] = entity // update the map with the modified copy
	}
}

var (
	globalState GameState
	stateMutex  sync.RWMutex
)

func init() {
	width, height := 128, 128
	tiles := make([]int, width*height)
	noise := opensimplex.New(time.Now().UnixNano())

	scale := 0.03
	threshold := 0.0

	// Generate base land/water map
	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			nx := float64(x) * scale
			ny := float64(y) * scale
			value := noise.Eval2(nx, ny)
			if value > threshold {
				tiles[y*width+x] = 1 // land
			} else {
				tiles[y*width+x] = 0 // water
			}
		}
	}

	entities := make(map[int]Entity)
	entities[123] = Entity{ID: 123, X: float64(width / 2), Y: float64(height / 2), Facing: 0, Type: "player"}

	// --- Natural Tree Placement ---
	treeNoise1 := opensimplex.New(time.Now().UnixNano() + 100) // low freq: big clumps
	treeNoise2 := opensimplex.New(time.Now().UnixNano() + 200) // high freq: detail
	treeID := 1000

	// Tune these!
	forestScale := 0.015 // very low frequency → huge forest zones
	detailScale := 0.08  // medium detail
	forestPower := 2.0   // exaggerate forest zones (makes them punchier)

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			if tiles[y*width+x] != 1 {
				continue // only on land
			}

			// Big forest zones
			f1 := treeNoise1.Eval2(float64(x)*forestScale, float64(y)*forestScale)
			// Add detail
			f2 := treeNoise2.Eval2(float64(x)*detailScale, float64(y)*detailScale)

			// Combine: boost clumps by squaring or exponentiating the base
			forestStrength := math.Pow(math.Max(0, f1), forestPower) // 0.0 to 1.0, clumpy!
			combined := forestStrength * (0.7 + 0.3*f2)              // modulate with detail

			// Now, instead of hard threshold, use probabilistic placement
			// e.g., 80% chance in dense forest, 5% in sparse
			placeChance := combined * 0.8 // max 80% chance even in best spots

			if rand.Float64() < placeChance {
				treeID++
				entities[treeID] = Entity{
					ID:     treeID,
					X:      float64(x) + rand.Float64()*0.8, // slight sub-tile offset!
					Y:      float64(y) + rand.Float64()*0.8,
					Facing: 0,
					Type:   "tree",
				}
			}
		}
	}
	globalState = GameState{
		Map: TileMap{
			Width:  width,
			Height: height,
			Tiles:  tiles,
		},
		Entities: entities,
		Ticks:    0,
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
		ticker := time.NewTicker(5000 * time.Millisecond) // 1000ms / 20 = 50ms
		defer ticker.Stop()
		for range ticker.C {
			tickHandler()
		}
	}()
	http.HandleFunc("/", getStateHandler)
	log.Println("Server starting on :8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
