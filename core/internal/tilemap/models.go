package tilemap

const RegionSize = 16

type TileKey int64

const (
	TileWater TileKey = 0
	TileLand  TileKey = 1
)

type Tile struct {
	key       TileKey
	elevation int64
}

type TileMap struct {
	Width  int64
	Height int64
	Tiles  []Tile
}
