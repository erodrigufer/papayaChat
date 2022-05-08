package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"net"
)

// configValues, parsed from the flags
type configValues struct {
	// addr, defines the port on which the server will be listening
	addr string
}

func main() {

	// Define default HOST and PORT, in case flag is not present
	DEFAULT_SERVICE := ":3333"

	cfg := new(configValues)
	flag.StringVar(&cfg.addr, "addr", DEFAULT_SERVICE, "Server's listening address")

	ln, err := net.Listen("tcp", cfg.addr)
	defer ln.Close()
	if err != nil {
		log.Println(err)
	}

	// infinite for-loop, accept clients and create goroutine to handle client
	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Println(err)
		}
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	// read the data up to the newline into netData
	netData, err := bufio.NewReader(conn).ReadString('\n')
	if err != nil {
		log.Println(err)
		return
	}
	fmt.Println(netData)
	conn.Close()
}
