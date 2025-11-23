package tilemap

import "core.axiom/internal/objects"

const RegionSize = 16

type TileKey int64

type Region struct {
	tile    TileKey
	objects []*objects.Object
}

type TileMap struct {
	Width  int64
	Height int64
	Tiles  []Region
}

