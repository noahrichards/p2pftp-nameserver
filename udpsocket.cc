#include "udpsocket.h"

#include <iostream>
#include <memory>

#include <arpa/inet.h>

std::string IpAddressName(const in_addr& address) {
  char buffer[32];
  const char* name = inet_ntop(AF_INET, &address, buffer, 32);
  return std::string(name);
}

UdpServerSocket* UdpServerSocket::Create(int port) {
  std::unique_ptr<UdpServerSocket> s(new UdpServerSocket(port));
  if (!s->Bind()) {
    return nullptr;
  }
  return s.release();
}

UdpServerSocket::~UdpServerSocket() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

bool UdpServerSocket::Bind() {
  fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_ < 0) {
    std::cerr << "socket create failed: " << strerror(errno) << std::endl;
    return false;
  }

  bool port_known = port_ != 0;

  struct sockaddr_in local_address;
  bzero(&local_address, sizeof(local_address));
  local_address.sin_family = AF_INET;
  local_address.sin_port = htons(port_);

  std::cout << "Attempting bind to port: " << port_ << std::endl;
  if (bind(fd_, reinterpret_cast<struct sockaddr*>(&local_address),
           sizeof(local_address)) < 0) {
    std::cerr << "socket bind failed: " << errno << ", " << strerror(errno)
              << std::endl;
    return false;
  }

  // If we used a random port, fetch the on we used.
  if (!port_known) {
    socklen_t local_address_length = sizeof(local_address);
    if (getsockname(fd_, reinterpret_cast<struct sockaddr*>(&local_address),
                    &local_address_length) < 0) {
      std::cerr << "getsockname failed: " << strerror(errno) << std::endl;
      return false;
    }
    port_ = ntohs(local_address.sin_port);
  }

  // Set the timeout to 10ms.
  timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 10 * 1000; // 10ms
  if (setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    std::cerr << "setsockopt timeout failed: " << strerror(errno) << std::endl;
    return false;
  }

  return true;
}

bool UdpServerSocket::ReceiveDatagram(
    uint8_t* buffer, size_t buffer_size,
    sockaddr_in* out_address, socklen_t* out_address_size,
    size_t* out_bytes_received) {
  ssize_t receive_length = recvfrom(fd_, buffer, buffer_size, 0,
      reinterpret_cast<sockaddr*>(out_address), out_address_size);
  if (receive_length <= 0) {
    return false;
  }
  *out_bytes_received = receive_length;
  return true;
}

bool UdpServerSocket::SendDatagram(
    const sockaddr_in* address, socklen_t address_size, uint8_t* buffer,
    size_t buffer_size) {
  ssize_t send_length = sendto(fd_, buffer, buffer_size, 0,
      reinterpret_cast<const sockaddr*>(address), address_size);
  return send_length == buffer_size;
}
