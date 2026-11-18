#pragma once

#include <stdint.h>
#include <string.h>
#include <Packet.h>

namespace mesh {

// QwickMesh carriers are RAW_CUSTOM packets beginning with this fixed v1 envelope:
// "QM", version, flags, then the first eight bytes of the origin node id.
// The clear origin is only a local fairness key; it is not an identity assertion.
static constexpr uint8_t QWICKMESH_ENVELOPE_SIZE = 12;
static constexpr uint8_t QWICKMESH_VERSION = 1;
static constexpr uint8_t QWICKMESH_MAX_FLOOD_HOPS = 3;

inline bool isQwickMeshCarrier(const Packet* packet) {
  return packet->getPayloadType() == PAYLOAD_TYPE_RAW_CUSTOM
      && packet->payload_len >= QWICKMESH_ENVELOPE_SIZE
      && packet->payload[0] == 'Q'
      && packet->payload[1] == 'M'
      && packet->payload[2] == QWICKMESH_VERSION;
}

inline const uint8_t* qwickMeshSourceKey(const Packet* packet) {
  return &packet->payload[4];
}

// Fixed-size, allocation-free per-origin airtime limiter for repeaters. A source
// can burst a small number of packets, then consumes an airtime budget that resets
// every window. The key is intentionally best-effort: radio packets cannot prove
// an account identity, so this protects airtime fairness rather than authorization.
class QwickMeshForwardLimiter {
public:
  static constexpr uint8_t MAX_SOURCES = 32;
  static constexpr uint8_t SOURCE_KEY_SIZE = 8;

  QwickMeshForwardLimiter(uint32_t window_ms = 3600000UL,
                           uint32_t source_budget_ms = 9000UL,
                           uint8_t burst_packets = 4)
      : _window_ms(window_ms), _source_budget_ms(source_budget_ms),
        _burst_packets(burst_packets), _next_lru(0) {
    memset(_entries, 0, sizeof(_entries));
  }

  bool allow(const uint8_t source[SOURCE_KEY_SIZE], uint32_t now_ms,
             uint32_t airtime_ms) {
    Entry* entry = findOrAllocate(source, now_ms);
    if (elapsed(now_ms, entry->window_started_ms) >= _window_ms) {
      entry->window_started_ms = now_ms;
      entry->used_airtime_ms = 0;
      entry->packets = 0;
    }
    if (entry->packets >= _burst_packets) return false;
    if (airtime_ms > _source_budget_ms - entry->used_airtime_ms) return false;

    entry->used_airtime_ms += airtime_ms;
    entry->packets++;
    entry->last_used_ms = now_ms;
    return true;
  }

private:
  struct Entry {
    uint8_t source[SOURCE_KEY_SIZE];
    uint32_t window_started_ms;
    uint32_t last_used_ms;
    uint32_t used_airtime_ms;
    uint8_t packets;
    bool in_use;
  };

  static uint32_t elapsed(uint32_t now, uint32_t then) { return now - then; }

  Entry* findOrAllocate(const uint8_t source[SOURCE_KEY_SIZE], uint32_t now_ms) {
    for (uint8_t i = 0; i < MAX_SOURCES; i++) {
      Entry* entry = &_entries[i];
      if (entry->in_use && memcmp(entry->source, source, SOURCE_KEY_SIZE) == 0) return entry;
      if (!entry->in_use) {
        entry->in_use = true;
        memcpy(entry->source, source, SOURCE_KEY_SIZE);
        entry->window_started_ms = now_ms;
        entry->last_used_ms = now_ms;
        entry->used_airtime_ms = 0;
        entry->packets = 0;
        return entry;
      }
    }
    Entry* entry = &_entries[_next_lru++ % MAX_SOURCES];
    memcpy(entry->source, source, SOURCE_KEY_SIZE);
    entry->window_started_ms = now_ms;
    entry->last_used_ms = now_ms;
    entry->used_airtime_ms = 0;
    entry->packets = 0;
    return entry;
  }

  Entry _entries[MAX_SOURCES];
  uint32_t _window_ms;
  uint32_t _source_budget_ms;
  uint8_t _burst_packets;
  uint8_t _next_lru;
};

}  // namespace mesh
