package transmit

import (
	"encoding/json"

	"go.bug.st/serial"
)

type SerialBaudSpeed int

func (s SerialBaudSpeed) Mode() *serial.Mode {
	return &serial.Mode{
		BaudRate: int(s),
		DataBits: 7,
		Parity:   serial.EvenParity,
		StopBits: serial.OneStopBit,
	}
}

const (

	// Debug/slow-safe baud rates
	DEBUG_BAUD_1 SerialBaudSpeed = 4800
	DEBUG_BAUD_2 SerialBaudSpeed = 2400
	DEBUG_BAUD_3 SerialBaudSpeed = 1200

	// Standard baud rates
	STANDART_BAUD_1 SerialBaudSpeed = 9600
	STANDART_BAUD_2 SerialBaudSpeed = 35000
	STANDART_BAUD_3 SerialBaudSpeed = 115200

	// High-speed baud rates
	HIGHT_BAUD_1 SerialBaudSpeed = 230400
	HIGHT_BAUD_2 SerialBaudSpeed = 460800
	HIGHT_BAUD_3 SerialBaudSpeed = 921600

	// Experimental/unstable baud rates
	UNSTABLE_BAUD_1 SerialBaudSpeed = 1000000
	UNSTABLE_BAUD_2 SerialBaudSpeed = 2000000
)

// =========================

type SerialMessage struct {
	Data   []byte
	Length int
	Error  error
}

func (sm SerialMessage) Decode(v any) error {
	if sm.Error != nil {
		return sm.Error
	}
	return json.Unmarshal(sm.Data, v)
}

func (sm SerialMessage) MessageLen(v any) int {
	return sm.Length
}
