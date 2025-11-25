package physics

import "github.com/google/uuid"

// CollisionsMesh represents a uniform grid for spatial partitioning.
// It divides the world into cells of size CellSize × CellSize.
type CollisionsMesh struct {
	CellSize    int64                    // Width/height of each cell in world units
	WidthCells  int                      // Number of cells along X axis
	HeightCells int                      // Number of cells along Y axis
	regions     []map[uuid.UUID]struct{} // 1D array of cell hash sets
}

// NewCollisionsMesh creates a new collision mesh.
// worldWidth and worldHeight are in world units (e.g., pixels).
// CellSize determines granularity.
func NewCollisionsMesh(worldWidth, worldHeight, cellSize int64) *CollisionsMesh {
	widthCells := int((worldWidth + cellSize - 1) / cellSize)   // ceiling division
	heightCells := int((worldHeight + cellSize - 1) / cellSize) // ceiling division
	totalCells := widthCells * heightCells

	regions := make([]map[uuid.UUID]struct{}, totalCells)

	return &CollisionsMesh{
		CellSize:    cellSize,
		WidthCells:  widthCells,
		HeightCells: heightCells,
		regions:     regions,
	}
}

// IndexByPosition converts world coordinates (x, y) to a 1D grid cell index.
// Returns -1 if (x, y) is outside the grid bounds.
func (cm *CollisionsMesh) IndexByPosition(x, y int64) int {
	// Compute cell coordinates
	cellX := x / cm.CellSize
	cellY := y / cm.CellSize

	// Handle negative positions (optional: you could offset instead)
	if cellX < 0 || cellY < 0 {
		return -1
	}

	// Check bounds
	if cellX >= int64(cm.WidthCells) || cellY >= int64(cm.HeightCells) {
		return -1
	}

	// Convert to 1D index
	return int(cellY)*cm.WidthCells + int(cellX)
}

// Add inserts an object ID into the cell corresponding to (x, y).
// If (x, y) is outside the grid, it does nothing.
func (cm *CollisionsMesh) Add(id uuid.UUID, x, y int64) {
	idx := cm.IndexByPosition(x, y)
	if idx < 0 {
		return // out of bounds
	}

	// Lazy-initialize the map for this cell
	if cm.regions[idx] == nil {
		cm.regions[idx] = make(map[uuid.UUID]struct{})
	}

	cm.regions[idx][id] = struct{}{}
}

// ObjectsByIndex returns a slice of object IDs in the cell at 1D index i.
// Returns nil if the index is invalid or the cell is empty.
func (cm *CollisionsMesh) ObjectsByIndex(i int64) []uuid.UUID {
	if i < 0 || i >= int64(len(cm.regions)) {
		return nil
	}

	cellMap := cm.regions[i]
	if len(cellMap) == 0 {
		return nil
	}

	result := make([]uuid.UUID, 0, len(cellMap))
	for id := range cellMap {
		result = append(result, id)
	}
	return result
}

func (cm *CollisionsMesh) ObjectsByPosition(x int64, y int64) []uuid.UUID {
	return nil
}
