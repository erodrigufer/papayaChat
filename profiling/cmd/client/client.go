package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"time"
)

func main() {

	host := "localhost:3333"
	conn, err := connectToHost(host)
	defer conn.Close()
	if err != nil {
		log.Println(err)
		os.Exit(1)
	}
	if _, err := fmt.Fprintf(conn, "Test\n"); err != nil {
		log.Println(err)
		os.Exit(1)
	}
	time.Sleep(3 * time.Second)
	if _, err := fmt.Fprintf(conn, "Test2\n"); err != nil {
		log.Println(err)
		os.Exit(1)
	}
}

func connectToHost(host string) (net.Conn, error) {
	conn, err := net.Dial("tcp", host)
	if err != nil {
		return nil, err
	}

	return conn, nil

}
