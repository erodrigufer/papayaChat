package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
)

// configValues, parsed from the flags
type configValues struct {
	// addr, defines the port on which the server will be listening
	addr string
}

type application struct {
	errorLog *log.Logger // error log handler
	infoLog  *log.Logger // info log handler
}

func main() {

	// Define default HOST and PORT, in case flag is not present
	DEFAULT_SERVICE := ":3333"

	cfg := new(configValues)
	app := new(application) // Create a logger for INFO messages, the prefix "INFO" and a tab will be
	// displayed before each log message. The flags Ldate and Ltime provide the
	// local date and time
	app.infoLog = log.New(os.Stdout, "INFO\t", log.Ldate|log.Ltime)

	// Create an ERROR messages logger, addiotionally use the Lshortfile flag to
	// display the file's name and line number for the error
	app.errorLog = log.New(os.Stderr, "ERROR\t", log.Ldate|log.Ltime|log.Lshortfile)

	flag.StringVar(&cfg.addr, "addr", DEFAULT_SERVICE, "Server's listening address")

	app.infoLog.Printf("Server listening on %s", cfg.addr)
	ln, err := net.Listen("tcp", cfg.addr)
	defer ln.Close()
	if err != nil {
		app.errorLog.Println(err)
	}

	// infinite for-loop, accept clients and create goroutine to handle client
	for {
		conn, err := ln.Accept()
		if err != nil {
			app.errorLog.Println(err)
		}
		go app.handleConnection(conn)
	}
}

func (app *application) handleConnection(conn net.Conn) {
	// read the data up to the newline into netData
	for {
		netData, err := bufio.NewReader(conn).ReadString('\n')
		if err != nil {
			//app.errorLog.Println(err)
			continue
			//return
		}
		_, err = fmt.Println(netData)
		if err != nil {
			app.errorLog.Println(err)
		}
	}
	//conn.Close()
}
