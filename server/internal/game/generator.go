package game

import (
	"time"

	"github.com/ojrac/opensimplex-go"
)

func GenerateMap(config ConfigProperty) *TileMap {
	width, height := config.Width, config.Height
	tiles := make([]TileKey, width*height)
	noise := opensimplex.New(time.Now().UnixNano())
	scale := 0.03

	// Define thresholds for terrain classification
	// Tune these values to control biome distribution
	waterThreshold := -0.2 // below this → Water
	dirtThreshold := 0.0   // between waterThreshold and dirtThreshold → Dirt
	rockThreshold := 0.4   // above rockThreshold → Rock
	// between dirtThreshold and rockThreshold → Land

	for y := 0; y < height; y++ {
		for x := 0; x < width; x++ {
			nx := float64(x) * scale
			ny := float64(y) * scale
			value := noise.Eval2(nx, ny)

			idx := y*width + x
			switch {
			case value > rockThreshold:
				tiles[idx] = Rock
			case value > dirtThreshold:
				tiles[idx] = Land
			case value > waterThreshold:
				tiles[idx] = Dirt
			default:
				tiles[idx] = Water
			}
		}
	}

	return &TileMap{
		Width:  width,
		Height: height,
		Tiles:  tiles,
	}
}

func GenerateObjects(state *GameState) *GameState {
	return state
}
