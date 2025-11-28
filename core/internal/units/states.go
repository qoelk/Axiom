package units

type StateKey string

type UnitState struct {
	Current   StateKey
	Next      StateKey
	TicksLeft int64
}

type State struct {
	Key           StateKey
	TicksDuration int64
}

const IDLE StateKey = "IDLE"

var StateBook = map[StateKey]State{
	IDLE: {
		Key:           IDLE,
		TicksDuration: 3,
	},
}
