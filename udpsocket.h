#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <string>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

std::string IpAddressName(const in_addr& address);

// Quick helper socket class for receiving UDP datagrams.
class UdpServerSocket {
 public:
  // Creates a UdpServerSocket bound optionally to the given port.
  // On failure, returns nullptr.
  static UdpServerSocket* Create(int port = 0);
  ~UdpServerSocket();

  // Returns the bound port.
  int port() const { return port_; }

  // Receives a datagram with a timeout of 10ms. Returns true on success.
  bool ReceiveDatagram(uint8_t* buffer, size_t buffer_size,
      sockaddr_in* out_address, socklen_t* out_address_size,
      size_t* out_bytes_written);

  // Sends a datagram to another client.
  bool SendDatagram(
      const sockaddr_in* address, socklen_t address_size, uint8_t* buffer,
      size_t buffer_size);

 private:
  UdpServerSocket(int port) : port_(port), fd_(-1) {}
  bool Bind();
  int port_;
  int fd_;
};

#endif  // UDPSOCKET_H
