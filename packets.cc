#include "packets.h"

#include <iostream>

void LogPacket(uint8_t* buffer, size_t size) {
  std::cout << "Packet: ";
  for (size_t i = 0; i < size; ++i) {
    std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
  }
  std::cout << std::endl;
}

PacketReader* PacketReader::Create(uint8_t* buffer, size_t size) {
  if (size == 0) {
    return nullptr;
  }
  int type = buffer[0];
  if (type != REGISTER && type != RENEW && type != QUERY) {
    std::cerr << "Invalid client packet type: " << type;
    if (DEBUG) {
      LogPacket(buffer, size);
    }
    return nullptr;
  }
  return new PacketReader(buffer, size);
}

bool PacketReader::ReadString(std::string* out_string) {
  if (remaining() < 2) {
    return false;
  }
  uint8_t* buffer = buffer_ + offset_;
  // First two bytes are length, little endian.
  size_t string_length = buffer[0] << 8 | buffer[1];
  if (remaining() < 2 + string_length) {
    return false;
  }
  *out_string = std::string(
      reinterpret_cast<char*>(buffer) + 2, string_length);
  offset_ += string_length + 2;
  return true;
}

bool PacketReader::ReadInt(int* out_int) {
  if (remaining() < 4) {
    return false;
  }
  uint8_t* buffer = buffer_ + offset_;
  *out_int = (buffer[0] << 24) |
             (buffer[1] << 16) |
             (buffer[2] << 8) |
             buffer[3];
  offset_ += 4;
  return true;
}

PacketWriter::PacketWriter(uint8_t* buffer, size_t size, int type)
    : buffer_(buffer), size_(size), offset_(1) {
  buffer[0] = type;
}

bool PacketWriter::WriteString(const std::string& value) {
  if (remaining() < 2 + value.length()) {
    return false;
  }
  uint8_t* buffer = buffer_ + offset_;
  buffer[0] = value.length() >> 8;
  buffer[1] = value.length() & 0xFF;
  memcpy(buffer + 2, value.c_str(), value.length());
  offset_ += 2 + value.length();
  return true;
}

bool PacketWriter::WriteInt(int value) {
  if (remaining() < 4) {
    return 0;
  }
  uint8_t* buffer = buffer_ + offset_;
  buffer[0] = (value >> 24) & 0xFF;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >> 8) & 0xFF;
  buffer[3] = value & 0xFF;
  offset_ += 4;
  return true;
}
