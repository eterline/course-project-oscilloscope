package plotter

import (
	"math"
	"math/big"
	"time"
)

var millivoltDivider *big.Float = big.NewFloat(1000)

type PlotterPacket struct {
	Duration   int64   `json:"duration"`
	Length     int     `json:"length"`
	SampleRate int     `json:"sample_rate"`
	StepSize   float64 `json:"step_size"`
	Probes     []int   `json:"probes"`
	zoom       int
}

type Decoder interface {
	Decode(v any) error
}

func ParsePacket(dc Decoder, divideByProbes int) (PlotterPacket, error) {
	pack := PlotterPacket{}
	pack.zoom = divideByProbes

	if err := dc.Decode(&pack); err != nil {
		return pack, err
	}

	if divideByProbes <= 0 {
		divideByProbes = 1
	}

	newLen := pack.Length / divideByProbes
	if newLen > len(pack.Probes) {
		newLen = len(pack.Probes)
	}

	pack.Length = newLen
	pack.Probes = pack.Probes[:newLen]

	return pack, nil
}

func (pk PlotterPacket) Voltages() []float64 {
	voltageList := make([]float64, pk.Length)

	for i := 0; i < pk.Length; i++ {
		voltageList[i] = pk.ToVolts(pk.Probes[i])
	}

	return voltageList
}

func (pk PlotterPacket) Maximum() int {
	if len(pk.Probes) == 0 {
		return 0
	}

	max := pk.Probes[0]
	for _, val := range pk.Probes[1:] {
		if val > max {
			max = val
		}
	}
	return max
}

func (pk PlotterPacket) Minimum() int {
	if pk.Length == 0 {
		return 0
	}

	min := pk.Probes[0]
	for _, val := range pk.Probes[1:] {
		if val < min {
			min = val
		}
	}
	return min
}

func (pk PlotterPacket) Average() int {
	if pk.Length == 0 {
		return 0
	}

	var sum int

	for _, val := range pk.Probes {
		sum += val
	}

	return sum / pk.Length
}

func (pk PlotterPacket) RMS() int {
	if pk.Length == 0 {
		return 0
	}

	var sumSquares int

	for _, val := range pk.Probes {
		sumSquares += val * val
	}

	meanSquares := float64(sumSquares) / float64(pk.Length)

	return int(math.Sqrt(meanSquares))
}

func (pk PlotterPacket) CalculateDutyCycle(threshold int) int {
	count := 0
	for _, v := range pk.Probes {
		if v >= threshold {
			count++
		}
	}
	if pk.Length == 0 {
		return 0.0
	}
	return count * 100 / pk.Length
}

func (pk PlotterPacket) AutoDutyCycle() int {
	if pk.Length == 0 {
		return 0
	}

	var max int
	for _, val := range pk.Probes {
		if val > max {
			max = val
		}
	}

	threshold := max / 2.0
	return pk.CalculateDutyCycle(threshold)
}

func (pk PlotterPacket) ToVolts(value int) float64 {

	multiplier := big.NewFloat(float64(value))
	multiplier.SetPrec(128)

	step := big.NewFloat(pk.StepSize)
	step.SetPrec(128)

	voltage := new(big.Float).Mul(step, multiplier)
	voltage.Quo(voltage, millivoltDivider)
	v, _ := voltage.Float64()
	return v
}

func (pk PlotterPacket) UnwrapDuration() time.Duration {
	return time.Duration(pk.Duration/int64(pk.zoom)) * time.Microsecond
}

func (pk PlotterPacket) UnwrapFreq() int64 {
	duration := pk.UnwrapDuration()
	if duration <= 0 {
		return 0
	}
	return 1e9 / int64(duration)
}
