package main

import (
	"fmt"
	"net"
)

func main() {

	// Define default HOST and PORT, in case flag is not present
	DEFAULT_SERVICE := ":50000"

	fmt.Printf("Server listening on %s\n", DEFAULT_SERVICE)
	ln, err := net.Listen("tcp", DEFAULT_SERVICE)
	defer ln.Close()
	if err != nil {
		fmt.Println(err)
	}

	// infinite for-loop, accept clients and create goroutine to handle client
	for {
		conn, err := ln.Accept()
		if err != nil {
			fmt.Println(err)
		}
		go handleConnection(conn)
	}
}

func handleConnection(conn net.Conn) {
	defer conn.Close()
	buf := make([]byte, 1024)
	// read the data up to the newline into netData
	for {
		//		netData, err := bufio.NewReader(conn).ReadString('\n')
		//		if err != nil {
		//			//app.errorLog.Println(err)
		//			continue
		//			//return
		//		}
		//		_, err = fmt.Println(netData)
		//		if err != nil {
		//			app.errorLog.Println(err)
		//		}

		n, err := conn.Read(buf)
		if err != nil {
			//fmt.Printf("Error occured: %q\n", err)
			// This break is necessary to return from the goroutine after a
			// client terminates a session with an io.EOF error.
			// Otherwise, the for-loops runs again, but the conn.Read method
			// will not read anything and infinite empty strings will be printed
			// to stdout.
			break
		}
		//fmt.Printf("Bytes received: %d\n", n)
		fmt.Printf("%q\n", buf[:n]) // use q to print also non-readable
		// characters being sent

	}
}
