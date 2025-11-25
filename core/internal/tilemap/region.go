package tilemap

const RegionSize = 16

type TileKey int64

type Region struct {
	tile TileKey
}

type TileMap struct {
	Width  int64
	Height int64
	Tiles  []Region
}
