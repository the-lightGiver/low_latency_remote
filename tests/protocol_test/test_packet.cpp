// Stage 2 proof: object -> serialize -> bytes -> deserialize -> object,
// with original fields recovered exactly. Also tests malformed packets,
// as required before this stage can be considered done.
#include "../../include/protocol/packet.hpp"
#include <cassert>
#include <cstdio>

int main() {
    // --- Test 1: round trip recovers exact fields ---
    llr::Packet original;
    original.header.sequence = 42;
    original.header.type = llr::PacketType::Test;
    std::vector<uint8_t> payload_data = {'h', 'e', 'l', 'l', 'o'};
    original.payload = payload_data;

    auto wire_bytes = original.serialize();
    printf("[test] serialized to %zu bytes (expected header %zu + payload %zu)\n",
           wire_bytes.size(), llr::kHeaderSize, payload_data.size());
    assert(wire_bytes.size() == llr::kHeaderSize + payload_data.size());

    llr::Packet recovered;
    bool ok = llr::Packet::deserialize(wire_bytes.data(), wire_bytes.size(), recovered);
    assert(ok);
    assert(recovered.header.magic == llr::kMagic);
    assert(recovered.header.sequence == 42);
    assert(recovered.header.type == llr::PacketType::Test);
    assert(recovered.payload == payload_data);
    printf("[test] PASS: round trip recovered all fields exactly\n");

    // --- Test 2: empty payload round trips correctly ---
    llr::Packet empty_payload;
    empty_payload.header.sequence = 7;
    auto empty_wire = empty_payload.serialize();
    llr::Packet empty_recovered;
    ok = llr::Packet::deserialize(empty_wire.data(), empty_wire.size(), empty_recovered);
    assert(ok);
    assert(empty_recovered.payload.empty());
    printf("[test] PASS: empty payload round trips correctly\n");

    // --- Test 3: too-short data is rejected ---
    std::vector<uint8_t> too_short = {1, 2, 3};
    llr::Packet bogus;
    ok = llr::Packet::deserialize(too_short.data(), too_short.size(), bogus);
    assert(!ok);
    printf("[test] PASS: too-short data correctly rejected\n");

    // --- Test 4: wrong magic number is rejected ---
    auto corrupted = wire_bytes;
    corrupted[0] = 0xFF; // corrupt the first magic byte
    ok = llr::Packet::deserialize(corrupted.data(), corrupted.size(), bogus);
    assert(!ok);
    printf("[test] PASS: wrong magic number correctly rejected\n");

    // --- Test 5: payload_length mismatch (claims more than actually present) ---
    auto truncated = wire_bytes;
    truncated.resize(truncated.size() - 2); // chop off some payload bytes
    ok = llr::Packet::deserialize(truncated.data(), truncated.size(), bogus);
    assert(!ok);
    printf("[test] PASS: truncated packet (length mismatch) correctly rejected\n");

    printf("\nAll Stage 2 tests passed.\n");
    return 0;
}
