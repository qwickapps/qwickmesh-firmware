#include <gtest/gtest.h>
#include "helpers/QwickMeshForwardLimiter.h"
#include "helpers/RoutingPolicy.h"

using namespace mesh;

static Packet qwickMeshFlood(const uint8_t source[8], uint8_t hops = 0) {
  Packet packet;
  packet.header = ROUTE_TYPE_FLOOD | (PAYLOAD_TYPE_RAW_CUSTOM << PH_TYPE_SHIFT);
  packet.setPathHashSizeAndCount(1, hops);
  packet.payload_len = QWICKMESH_ENVELOPE_SIZE;
  packet.payload[0] = 'Q';
  packet.payload[1] = 'M';
  packet.payload[2] = QWICKMESH_VERSION;
  packet.payload[3] = 0;
  memcpy(&packet.payload[4], source, 8);
  return packet;
}

TEST(QwickMeshCarrier, IdentifiesOnlyVersionedQwickMeshCustomPackets) {
  const uint8_t source[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  auto packet = qwickMeshFlood(source);
  EXPECT_TRUE(isQwickMeshCarrier(&packet));
  EXPECT_EQ(0, memcmp(source, qwickMeshSourceKey(&packet), 8));

  packet.payload[2] = QWICKMESH_VERSION + 1;
  EXPECT_FALSE(isQwickMeshCarrier(&packet));
}

TEST(QwickMeshForwardLimiter, FloodPathIsCappedAtThreeHops) {
  const uint8_t source[8] = {0};
  auto two_hops = qwickMeshFlood(source, 2);
  auto three_hops = qwickMeshFlood(source, 3);
  EXPECT_FALSE(isFloodHopLimitExceeded(&two_hops, QWICKMESH_MAX_FLOOD_HOPS, 64, 64));
  EXPECT_TRUE(isFloodHopLimitExceeded(&three_hops, QWICKMESH_MAX_FLOOD_HOPS, 64, 64));
}

TEST(QwickMeshForwardLimiter, OneSourceCannotExhaustAnotherSourcesBudget) {
  const uint8_t first[8] = {1};
  const uint8_t second[8] = {2};
  QwickMeshForwardLimiter limiter(1000, 100, 4);

  EXPECT_TRUE(limiter.allow(first, 0, 25));
  EXPECT_TRUE(limiter.allow(first, 1, 25));
  EXPECT_TRUE(limiter.allow(first, 2, 25));
  EXPECT_TRUE(limiter.allow(first, 3, 25));
  EXPECT_FALSE(limiter.allow(first, 4, 1));
  EXPECT_TRUE(limiter.allow(second, 5, 100));
}

TEST(QwickMeshForwardLimiter, BudgetRefillsAtTheNextWindow) {
  const uint8_t source[8] = {9};
  QwickMeshForwardLimiter limiter(1000, 100, 4);

  EXPECT_TRUE(limiter.allow(source, 0, 100));
  EXPECT_FALSE(limiter.allow(source, 999, 1));
  EXPECT_TRUE(limiter.allow(source, 1000, 100));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
