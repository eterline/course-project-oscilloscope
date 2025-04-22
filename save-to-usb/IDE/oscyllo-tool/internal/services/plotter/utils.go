package plotter

import "sync"

type Scaler struct {
	value int
	max   int
	mu    sync.Mutex
}

func InitPlotterScaler(max int) *Scaler {
	if max < 0 {
		max = 1
	}
	return &Scaler{
		value: 1,
		max:   max,
	}
}

func (s *Scaler) Up() {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.value++
}

func (s *Scaler) Down() {
	s.mu.Lock()
	defer s.mu.Unlock()

	s.validate()
	s.value--
}

func (s *Scaler) Reset() {
	s.mu.Lock()
	defer s.mu.Unlock()
	s.value = 1
}

func (s *Scaler) Value() int {
	s.mu.Lock()
	defer s.mu.Unlock()

	s.validate()
	return s.value
}

func (s *Scaler) validate() {
	if s.value < 0 || s.value > s.max {
		s.value = 1
	}
}
