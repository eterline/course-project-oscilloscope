.PHONY: build run

# ========= Vars definitions =========

app = oscyllo-tool

# ========= Prepare commands =========

tidy:
	go mod tidy
	go clean

del:
	rm ./$(app)* || echo "file didn't exists"
	rm ./trace*  || echo "file didn't exists"

# ========= Compile commands =========

build:
	go build -o ./$(app) -v ./cmd/$(app)/main.go

run: del build
	./$(app)

compile:
	GOOS=linux GOARCH=amd64 go build -o build/$(app)-linux -v ./cmd/$(app)/main.go
	GOOS=windows GOARCH=amd64 go build -o build/$(app)-windows.exe -v ./cmd/$(app)/main.go
	GOOS=freebsd GOARCH=amd64 go build -o build/$(app)-freebsd -v ./cmd/$(app)/main.go


.DEFAULT_GOAL := run
