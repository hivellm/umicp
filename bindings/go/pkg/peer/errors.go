package peer

// GenericError represents a generic error
type GenericError struct {
	Message string
}

func (e *GenericError) Error() string {
	return e.Message
}
