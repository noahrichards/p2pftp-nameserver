// Implements the server side of the RIT CS P2PFTP project:
// https://www.cs.rit.edu/~ptt/courses/4003-236/projects/P2pFTP/specifications.html

#include "packets.h"
#include "udpsocket.h"

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#define CHECKED(op)                                 \
  if (!op) {                                        \
    std::cerr << "OP failed: " << #op << std::endl; \
  }

struct Address {
  Address(const std::string& name, const std::string& ip_address,
      int port) : name(name), ip_address(ip_address), port(port) {}
  std::string name;
  std::string ip_address;
  unsigned int port;
};

int main(int argc, char** argv) {
  if (argc > 2) {
    std::cerr << "Usage: " << std::endl;
    std::cerr << "  " << argv[0]
              << " [port]";
  }
  int port = 0;
  if (argc == 2) {
    port = std::stoi(argv[1]);
  }

  std::unique_ptr<UdpServerSocket> socket(UdpServerSocket::Create(port));
  if (!socket) {
    std::cerr << "Failed to create socket." << std::endl;
    return 1;
  }
  std::cout << "Listening on port " << socket->port() << std::endl;

  std::vector<Address> servers;
  servers.push_back(Address("Bob Marley", "127.0.0.1", 12345));
  servers.push_back(Address("Tom Barley", "127.0.0.1", 54321));

  uint8_t buffer[1024];
  uint8_t send_buffer[2048];
  while(true) {
    sockaddr_in address = {0};
    socklen_t address_size = sizeof(address);
    size_t size;
    if (socket->ReceiveDatagram(buffer, sizeof(buffer), &address, &address_size,
                                &size)) {
      std::unique_ptr<PacketReader> reader(
          PacketReader::Create(buffer, size));
      if (!reader) {
        continue;
      }
      if (reader->type() == QUERY) {
        std::string matcher;
        std::vector<Address> matching_servers;
        if (reader->ReadString(&matcher) && !matcher.empty()) {
          for (const auto& address : servers) {
            if (address.name.compare(0, matcher.length(), matcher) == 0) {
              matching_servers.push_back(address);
            }
          }
        } else {
          matching_servers = servers;
        }
        // Send back matching clients.
        PacketWriter writer(send_buffer, sizeof(send_buffer), REPLY);
        CHECKED(writer.WriteInt(matching_servers.size()));
        for (const auto& address : matching_servers) {
          CHECKED(writer.WriteString(address.name));
          CHECKED(writer.WriteString(address.ip_address));
          CHECKED(writer.WriteInt(address.port));
        }
        CHECKED(socket->SendDatagram(&address, address_size, send_buffer,
              writer.size()));
      } else if (reader->type() == REGISTER) {
        LogPacket(buffer, size);
        std::string name;
        int send_port;
        CHECKED(reader->ReadString(&name));
        CHECKED(reader->ReadInt(&send_port));
        std::cout << "Got register request: " << std::endl;
        std::cout << "name: " << name << ", port: " << send_port << std::endl;
        servers.push_back(Address(name, IpAddressName(address.sin_addr),
                                  send_port));

        // Send back a ticket.
        // TODO(noahric): Keep more ticket information around.
        PacketWriter writer(send_buffer, sizeof(send_buffer), TICKET);
        CHECKED(writer.WriteInt(1111));
        CHECKED(writer.WriteInt(100 * 1000));
        CHECKED(socket->SendDatagram(&address, address_size, send_buffer,
              writer.size()));
        std::cout << "Sent TICKET response." << std::endl;
        LogPacket(send_buffer, writer.size());
      }
    }
  }
}
