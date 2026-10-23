#pragma once
// 도트 스프라이트 격자 데이터 (SpriteData.cpp). 팔레트는 Art.cpp.
#include <string>
#include <vector>

#include "data/CardId.h"

namespace gengar::art::data {

const std::vector<std::string>& Phantom0();
const std::vector<std::string>& Phantom1();
const std::vector<std::string>& Slime0();
const std::vector<std::string>& Slime1();
const std::vector<std::string>& Bat0();
const std::vector<std::string>& Bat1();
const std::vector<std::string>& Hound0();
const std::vector<std::string>& Hound1();
const std::vector<std::string>& Caster0();
const std::vector<std::string>& Caster1();
const std::vector<std::string>& Coin0();
const std::vector<std::string>& Coin1();
const std::vector<std::string>& Coin2();
const std::vector<std::string>& Coin3();
const std::vector<std::string>& HeartFull();
const std::vector<std::string>& HeartEmpty();
const std::vector<std::string>& Ball0();
const std::vector<std::string>& Ball1();
const std::vector<std::string>& Orb0();
const std::vector<std::string>& Wave0();
const std::vector<std::string>& Wave1();
const std::vector<std::string>& Pennant0();
const std::vector<std::string>& Pennant1();
const std::vector<std::string>& Pennant2();
const std::vector<std::string>& Moon();
const std::vector<std::string>& TileGroundTop();
const std::vector<std::string>& TileDirt();
const std::vector<std::string>& TileBrick();
const std::vector<std::string>& TileCrumble();
const std::vector<std::string>& TileSpike();
const std::vector<std::string>& Boss();
const std::vector<std::string>& BossHand();
const std::vector<std::string>& PhantomHappy0();
const std::vector<std::string>& PhantomHappy1();
const std::vector<std::string>& TreasureHat();
// 아이콘 (12x12, 팔레트 kUiIcon). 카드 아이콘으로도 쓴다 (CardIcons.cpp).
const std::vector<std::string>& IconPower();
const std::vector<std::string>& IconRate();
const std::vector<std::string>& IconMulti();
const std::vector<std::string>& IconMaxHp();
const std::vector<std::string>& IconDoubleJump();
const std::vector<std::string>& IconSpeed();
const std::vector<std::string>& IconUlt();
const std::vector<std::string>& IconLife();
const std::vector<std::string>& IconDepart();
// 카드 아이콘 (12x12, 팔레트 kUiIcon). CardIcons.cpp
const std::vector<std::string>& CardIconRows(CardId id);

}  // namespace gengar::art::data
