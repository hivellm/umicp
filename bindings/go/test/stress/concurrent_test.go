package stress

import (
	"sync"
	"testing"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/hivellm/umicp-go/pkg/umicp/matrix"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// TestConcurrent_EnvelopeCreation tests concurrent envelope creation
func TestConcurrent_EnvelopeCreation(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping stress test in short mode")
	}

	const goroutines = 100
	const envelopesPerGoroutine = 100

	var wg sync.WaitGroup
	wg.Add(goroutines)

	errors := make(chan error, goroutines*envelopesPerGoroutine)

	for i := 0; i < goroutines; i++ {
		go func(id int) {
			defer wg.Done()

			for j := 0; j < envelopesPerGoroutine; j++ {
				_, err := umicp.NewEnvelope().
					From("sender").
					To("receiver").
					Operation(umicp.OperationData).
					Build()

				if err != nil {
					errors <- err
				}
			}
		}(i)
	}

	wg.Wait()
	close(errors)

	errorCount := 0
	for range errors {
		errorCount++
	}

	assert.Equal(t, 0, errorCount, "Should have no errors")
}

// TestConcurrent_Serialization tests concurrent serialization
func TestConcurrent_Serialization(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping stress test in short mode")
	}

	const goroutines = 50

	env, err := umicp.NewEnvelope().
		From("sender").
		To("receiver").
		Operation(umicp.OperationData).
		Build()
	require.NoError(t, err)

	var wg sync.WaitGroup
	wg.Add(goroutines)

	errors := make(chan error, goroutines)

	for i := 0; i < goroutines; i++ {
		go func() {
			defer wg.Done()

			for j := 0; j < 100; j++ {
				_, err := env.Serialize()
				if err != nil {
					errors <- err
					return
				}
			}
		}()
	}

	wg.Wait()
	close(errors)

	errorCount := 0
	for range errors {
		errorCount++
	}

	assert.Equal(t, 0, errorCount, "Should have no errors")
}

// TestStress_MatrixOperations tests concurrent matrix operations
func TestStress_MatrixOperations(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping stress test in short mode")
	}

	const goroutines = 100
	const iterations = 1000

	v1 := make([]float32, 512)
	v2 := make([]float32, 512)
	for i := range v1 {
		v1[i] = float32(i)
		v2[i] = float32(i)
	}

	var wg sync.WaitGroup
	wg.Add(goroutines)

	for i := 0; i < goroutines; i++ {
		go func() {
			defer wg.Done()

			for j := 0; j < iterations; j++ {
				_, _ = matrix.DotProduct(v1, v2)
				_, _ = matrix.CosineSimilarity(v1, v2)
			}
		}()
	}

	wg.Wait()
}
