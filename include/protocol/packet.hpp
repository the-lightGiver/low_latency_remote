#pragma once
#include <arpa/inet.h>  // htonl/ntohl
#include <cstdint>
#include <cstring>
#include <vector>

namespace llr {

// Fixed protocol identifier. Chosen so it's extremely unlikely to appear
// by coincidence in random/corrupted data, and so a receiver can reject
// non-protocol packets in a single comparison before parsing anything else.
constexpr uint32_t kMagic = 0x4C4C5231; // ASCII-ish "LLR1"

enum class PacketType : uint8_t {
    Unknown = 0,
    // Real types (Input, Output, Ack, ...) get added as later stages need
    // them. Deliberate simplification for this stage: only enough to prove
    // the field round-trips correctly.
    Test = 1,
};

// Fixed-size header. Every field is the same size on every packet, which
// is exactly why no "header length" field is needed - the receiver always
// knows this struct's size at compile time. Only the payload varies, which
// is why payload_length exists.
struct PacketHeader {
    uint32_t magic = kMagic;
    uint32_t sequence = 0;
    PacketType type = PacketType::Unknown;
    uint32_t payload_length = 0;
};

constexpr size_t kHeaderSize =
    sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t); // 13

struct Packet {
    PacketHeader header;
    std::vector<uint8_t> payload;

    // Turns this object into wire bytes, with every multi-byte field
    // explicitly converted to network byte order. This does NOT dump raw
    // struct memory - padding/alignment/endianness are irrelevant here
    // because every field is written out by hand, in a defined order.
    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> buf;
        buf.reserve(kHeaderSize + payload.size());

        auto push_u32 = [&](uint32_t v) {
            uint32_t be = htonl(v);
            const uint8_t* b = reinterpret_cast<const uint8_t*>(&be);
            buf.insert(buf.end(), b, b + sizeof(be));
        };

        push_u32(header.magic);
        push_u32(header.sequence);
        buf.push_back(static_cast<uint8_t>(header.type));
        push_u32(static_cast<uint32_t>(payload.size()));
        buf.insert(buf.end(), payload.begin(), payload.end());
        return buf;
    }

    // Returns false (and leaves `out` unspecified) on any malformed input:
    // too short to even contain a header, wrong magic, or a claimed
    // payload_length that doesn't match what actually arrived. The spec
    // requires malformed packets to be rejected, not misparsed.
    static bool deserialize(const uint8_t* data, size_t len, Packet& out) {
        if (len < kHeaderSize) {
            return false; // not even enough bytes for a header
        }

        auto read_u32 = [&](size_t offset) -> uint32_t {
            uint32_t be;
            std::memcpy(&be, data + offset, sizeof(be));
            return ntohl(be);
        };

        uint32_t magic = read_u32(0);
        if (magic != kMagic) {
            return false; // not our protocol, or corrupted
        }

        uint32_t sequence = read_u32(4);
        uint8_t type_raw = data[8];
        uint32_t payload_length = read_u32(9);

        if (kHeaderSize + payload_length != len) {
            return false; // claimed length doesn't match actual datagram size
        }

        out.header.magic = magic;
        out.header.sequence = sequence;
        out.header.type = static_cast<PacketType>(type_raw);
        out.header.payload_length = payload_length;
        out.payload.assign(data + kHeaderSize, data + len);
        return true;
    }
};

} // namespace llr
