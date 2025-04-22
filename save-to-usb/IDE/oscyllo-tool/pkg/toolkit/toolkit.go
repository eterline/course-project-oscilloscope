package toolkit

import (
	"context"
	"crypto/sha1"
	"encoding/json"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"

	"github.com/google/uuid"
)

type AppStarter struct {
	Context  context.Context
	stopFunc context.CancelFunc
	wg       sync.WaitGroup
}

// StopApp - cancel root app context and stopping app
func (s *AppStarter) StopApp() {
	if s.Context.Err() == nil {
		s.stopFunc()
	}
}

// MustStopApp - cancel root app context and extremely stops app with exit code
func (s *AppStarter) MustStopApp(exitCode int) {
	s.StopApp()
	os.Exit(exitCode)
}

// Wait - waiting for app root context done
func (s *AppStarter) Wait() {
	<-s.Context.Done()
}

// NewThread - appends thread
func (s *AppStarter) NewThread() {
	s.wg.Add(1)
}

// DoneThread - final thread
func (s *AppStarter) DoneThread() {
	s.wg.Done()
}

// FinalThreads - wait for thread final or timeout exit
func (s *AppStarter) FinalThreads(timeout time.Duration) {
	done := make(chan struct{})

	go func() {
		s.wg.Wait()
		close(done)
	}()

	timer := time.NewTimer(timeout)
	defer timer.Stop()

	select {
	case <-timer.C:
		panic("app threads stop timeout")

	case <-done:
		return
	}
}

// AddValue - appends to context values with key
func (s *AppStarter) AddValue(key, value any) {
	s.Context = context.WithValue(s.Context, key, value)
}

// UseContextAdders - uses func list that returns new context
func (s *AppStarter) UseContextAdders(
	addFunc ...func(context.Context) context.Context,
) {
	for _, add := range addFunc {
		s.Context = add(s.Context)
	}
}

// InitAppStart - create app root context and stop function object
func InitAppStart(preInitFunc func() error) *AppStarter {
	return InitAppStartWithContext(
		context.Background(),
		preInitFunc,
	)
}

// InitAppStartWithContext - create app root context and stop function object form external context.
// Must be used with pre init function. If their init will be errored - panic closes app
func InitAppStartWithContext(ctx context.Context, preInitFunc func() error) *AppStarter {

	if err := preInitFunc(); err != nil {
		panic("app starting fatal error: " + err.Error())
	}

	rootContext, stopFunc := signal.NotifyContext(
		ctx, syscall.SIGINT, syscall.SIGTERM,
	)

	return &AppStarter{
		Context:  rootContext,
		stopFunc: stopFunc,
	}
}

// =================================

// BytesUUID - generates uuid (v5) from bytes array
func BytesUUID(input []byte) (uuid.UUID, bool) {
	hash := sha1.New()

	_, err := hash.Write(input)
	if err != nil {
		return uuid.Nil, false
	}

	id, err := uuid.FromBytes(hash.Sum(nil)[:16])
	if err != nil {
		return uuid.Nil, false
	}

	return id, true
}

// StringUUID - generates uuid (v5) from string value
func StringUUID(input string) (uuid.UUID, bool) {
	return BytesUUID([]byte(input))
}

// BytesUUID - generates uuid (v5) from object
func ObjectUUID(object any) (uuid.UUID, bool) {
	data, err := json.Marshal(object)
	if err != nil {
		return uuid.Nil, false
	}

	return BytesUUID(data)
}

// =================================
