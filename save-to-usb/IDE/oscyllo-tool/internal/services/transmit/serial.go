package transmit

import (
	"bufio"
	"context"
	"encoding/gob"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
	"sync"

	"go.bug.st/serial"
)

// ==============================================

// ==============================================

func SelectPort(ctx context.Context) (string, error) {

	if ctx.Err() != nil {
		return "", nil
	}

	portArr, err := serial.GetPortsList()
	if err != nil {
		return "", fmt.Errorf("serial port list getting error: %w", err)
	}

	if len(portArr) < 1 {
		return "", fmt.Errorf("serial port list is empty")
	}

	if len(portArr) == 1 {
		return portArr[0], nil
	}

	portCh := make(chan int)
	errCh := make(chan error)

	go func() {
		reader := bufio.NewReader(os.Stdin)
		for {
			fmt.Println("================================")
			fmt.Println("Select serial port number:")
			for i, port := range portArr {
				fmt.Printf("[%d] - '%s'\n", i, port)
			}
			fmt.Print("Enter port number (default = 0): ")

			text, _ := reader.ReadString('\n')
			text = strings.TrimSpace(text)
			if text == "" {
				portCh <- 0
				return
			}

			num, err := strconv.Atoi(text)
			if err != nil || num < 0 || num >= len(portArr) {
				fmt.Println("Incorrect port number")
				continue
			}

			portCh <- num
			return
		}
	}()

	select {
	case <-ctx.Done():
		return "", fmt.Errorf("operation cancelled by context")
	case err := <-errCh:
		return "", err
	case portNum := <-portCh:
		return portArr[portNum], nil
	}
}

type baudRange [6]SerialBaudSpeed

func (r baudRange) selectBaud(pos int) (SerialBaudSpeed, bool) {
	if pos < 0 || len(r)-1 < pos {
		return 0, false
	}
	return r[pos], true
}

func baudList() baudRange {

	list := baudRange{
		STANDART_BAUD_1,
		STANDART_BAUD_2,
		STANDART_BAUD_3,

		HIGHT_BAUD_1,
		HIGHT_BAUD_2,
		HIGHT_BAUD_3,
	}

	return list
}

func SelectBaud(ctx context.Context) (SerialBaudSpeed, error) {

	if ctx.Err() != nil {
		return 0, nil
	}

	rangeBauds := baudList()
	baudCh := make(chan SerialBaudSpeed)
	errCh := make(chan error)

	go func() {
		reader := bufio.NewReader(os.Stdin)
		for {
			fmt.Println("================================")
			fmt.Println("Select baud rate:")
			for i, baud := range rangeBauds {
				fmt.Printf("[%d] - '%d'\n", i, baud)
			}
			fmt.Print("Enter baud (default = 0): ")

			text, _ := reader.ReadString('\n')
			text = strings.TrimSpace(text)
			if text == "" {
				baudCh <- rangeBauds[0]
				return
			}

			num, _ := strconv.Atoi(text)
			selected, ok := rangeBauds.selectBaud(num)

			if !ok {
				fmt.Println("Incorrect baud number")
				continue
			}

			baudCh <- selected
			return
		}
	}()

	select {
	case <-ctx.Done():
		return 0, fmt.Errorf("operation cancelled by context")
	case err := <-errCh:
		return 0, fmt.Errorf("baud selection error: %w", err)
	case b := <-baudCh:
		return b, nil
	}
}

// ===================

func NewSerialConnector(rate SerialBaudSpeed, name string, det byte) (*SerialConnector, error) {

	if rate == 0 || name == "" {
		return nil, nil
	}

	port, err := serial.Open(name, rate.Mode())
	if err != nil {
		return nil, fmt.Errorf("micro service couldn't open port: %s - %w", name, err)
	}

	if det == 0 {
		det = '\n'
	}

	connector := &SerialConnector{
		portName:      name,
		speed:         rate,
		port:          port,
		determination: det,
	}

	return connector, nil
}

type SerialConnector struct {
	portName      string
	determination byte
	speed         SerialBaudSpeed

	port serial.Port
	mu   sync.RWMutex
}

func (sc *SerialConnector) StartReading(ctx context.Context) <-chan SerialMessage {
	messageChannel := make(chan SerialMessage, 10)

	go func() {
		defer sc.port.Close()
		defer close(messageChannel)

		var (
			buffer = make([]byte, 0, 1024)
			tmp    [1]byte
		)

		for {
			select {
			case <-ctx.Done():
				return

			default:
				if ctx.Err() != nil {
					return
				}

				sc.mu.RLock()
				_, err := sc.port.Read(tmp[:])
				sc.mu.RUnlock()

				if err != nil {
					messageChannel <- SerialMessage{
						Data:   nil,
						Length: 0,
						Error:  err,
					}
					continue
				}

				b := tmp[0]

				if b == sc.determination {

					data := make([]byte, len(buffer))
					copy(data, buffer)

					messageChannel <- SerialMessage{
						Data:   data,
						Length: len(data),
						Error:  nil,
					}

					buffer = buffer[:0]
				} else {
					buffer = append(buffer, b)

					if cap(buffer) > 4096 && len(buffer) < 512 {
						buffer = make([]byte, 0, 1024)
					}
				}
			}
		}
	}()

	return messageChannel
}

func (sc *SerialConnector) Write(data []byte) (n int, err error) {
	sc.mu.Lock()
	defer sc.mu.Unlock()

	return sc.port.Write(data)
}

func (sc *SerialConnector) SendString(text string) (n int, err error) {
	return sc.Write([]byte(text))
}

func (sc *SerialConnector) SendJSON(structure interface{}) (n int, err error) {
	data, err := json.Marshal(structure)
	if err != nil {
		return 0, err
	}

	return sc.Write(data)
}

func (sc *SerialConnector) SendGOB(structure interface{}) error {
	return gob.NewEncoder(sc).Encode(structure)
}
