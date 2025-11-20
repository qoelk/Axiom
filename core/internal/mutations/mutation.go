package mutations

type Mutation struct {
	ActorID uuid.UUID
	D int64
	StateID uuid.UUID
	MutationType uuid.UUID
}