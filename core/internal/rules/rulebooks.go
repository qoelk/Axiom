package rules

type Mutation interface {
   Perform(o *Object, m *mutations.Mutation) {} 
}
var MutationsBook map[uuid.UUID] 