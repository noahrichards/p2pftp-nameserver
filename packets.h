#ifndef PACKETS_H
#define PACKETS_H

#include <cstdlib>
#include <iostream>
#include <string>

const bool DEBUG = true;

// Packet types.
enum PacketTypes {
  REGISTER = 0,
  TICKET = 1,
  RENEW = 2,
  QUERY = 3,
  REPLY = 4,
  UPDATE = 5
};

void LogPacket(uint8_t* buffer, size_t size);

class PacketReader {
 public:
  static PacketReader* Create(uint8_t* buffer, size_t size);
  int type() const { return type_; }
  size_t remaining() const { return size_ - offset_; }
  // Reads a string. Returns true on success.
  bool ReadString(std::string* out_string);
  // Reads an int. Returns true on success.
  bool ReadInt(int* out_int);

 private:
  PacketReader(uint8_t* buffer, size_t size)
      : buffer_(buffer), size_(size), offset_(1), type_(buffer[0]) {}
  uint8_t* buffer_;
  size_t size_;
  size_t offset_;
  int type_;
};

class PacketWriter {
 public:
  PacketWriter(uint8_t* buffer, size_t size, int type);
  size_t remaining() const { return size_ - offset_; }
  size_t size() const { return offset_; }
  // Reads a string. Returns true on success.
  bool WriteString(const std::string& value);
  // Reads an int. Returns true on success.
  bool WriteInt(int value);

 private:
  uint8_t* buffer_;
  size_t size_;
  size_t offset_;
};

#endif  // PACKETS_H
