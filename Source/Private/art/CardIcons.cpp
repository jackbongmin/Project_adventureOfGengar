// 카드 아이콘 도트 데이터 (12x12, 팔레트 kUiIcon). 행을 직접 고치면 된다.
// 카드 -> 아이콘 연결은 맨 아래 CardIconRows() 의 표.
#include "art/SpriteData.h"

#include "data/CardId.h"

namespace gengar::art::data {
namespace {

const std::vector<std::string>& IconRange() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      ".......oo...",
      ".......owo..",
      "ooooooooccoo",
      "occcccccccco",
      "occcccccccco",
      "ooooooooccoo",
      ".......owo..",
      ".......oo...",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconShotSpeed() {
  static const std::vector<std::string> rows = {
      "............",
      "......oooo..",
      ".....opppmo.",
      "oooooppwpmmo",
      "......ppppmo",
      "ooooo.pppmmo",
      "......ppmmmo",
      "oooooompmmo.",
      ".....ommmmo.",
      "......oooo..",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconKnock() {
  static const std::vector<std::string> rows = {
      "............",
      "...oooo.....",
      "..owwwwo....",
      ".owsswwwoo..",
      ".owsswwwwwo.",
      ".owwwwwwwso.",
      ".osswwwwsso.",
      "..ossssso...",
      "...ooooo.oo.",
      "........ocoo",
      ".........occ",
      "..........oo",
  };
  return rows;
}

const std::vector<std::string>& IconStar() {
  static const std::vector<std::string> rows = {
      "............",
      ".....oo.....",
      ".....oyo....",
      "....oyyo....",
      "ooooyyyyoooo",
      "oyyyywyyyyyo",
      ".oyyyyyyyyo.",
      "..oyyyyyyo..",
      "..oyyddyyo..",
      ".oyyo..oyyo.",
      ".oyo....oyo.",
      ".oo......oo.",
  };
  return rows;
}

const std::vector<std::string>& IconPierce() {
  static const std::vector<std::string> rows = {
      "............",
      "......oo....",
      "..oo..oso...",
      "..oco.oso...",
      "oooccoossooo",
      "occcccccccco",
      "occcccccccco",
      "oooccoossooo",
      "..oco.oso...",
      "..oo..oso...",
      "......oo....",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconBackShot() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "..oo....oo..",
      ".opo....opo.",
      "oppoooooopo.",
      "opppppppppo.",
      "opppppppppo.",
      "oppoooooopo.",
      ".opo....opo.",
      "..oo....oo..",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconSkull() {
  static const std::vector<std::string> rows = {
      "............",
      "...oooooo...",
      "..owwwwwwo..",
      ".owwwwwwwwo.",
      ".owooswoowo.",
      ".owookwookwo",
      ".owwwoowwwo.",
      "..owwwwwwo..",
      "...owowow...",
      "...owowow...",
      "....ooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconBounce() {
  static const std::vector<std::string> rows = {
      "............",
      "........oo..",
      ".......occo.",
      "......occo..",
      ".oo..occo...",
      "occooco.....",
      ".occco......",
      "..oco.......",
      "...o........",
      "oooooooooooo",
      "osssssssssso",
      "oooooooooooo",
  };
  return rows;
}

const std::vector<std::string>& IconBurst() {
  static const std::vector<std::string> rows = {
      "............",
      "..o..oo..o..",
      "..oo.oyo.oo.",
      "...oooyooo..",
      ".ooyyywyyyoo",
      "oyyywwwwyyyo",
      ".ooyywwwyyo.",
      "...ooyyyoo..",
      "..oo.oyo.oo.",
      "..o..oyo..o.",
      ".....oo.....",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconEye() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "...oooooo...",
      "..owwwwwwo..",
      ".owwppppwwo.",
      "owwppmmppwwo",
      "owwppmmppwwo",
      ".owwppppwwo.",
      "..owwwwwwo..",
      "...oooooo...",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconTarget() {
  static const std::vector<std::string> rows = {
      "....oooo....",
      "..oorrrroo..",
      ".orrwwwwrro.",
      ".orwwrrwwro.",
      "orwwrrrrwwro",
      "orwrrwwrrwro",
      "orwrrwwrrwro",
      "orwwrrrrwwro",
      ".orwwrrwwro.",
      ".orrwwwwrro.",
      "..oorrrroo..",
      "....oooo....",
  };
  return rows;
}

const std::vector<std::string>& IconSplit() {
  static const std::vector<std::string> rows = {
      "oo.......oo.",
      "opo.....opo.",
      ".opo...opo..",
      "..opo.opo...",
      "...opppo....",
      "....opo.....",
      "....opo.....",
      "...opppo....",
      "...oppppo...",
      "...oppppo...",
      "....oooo....",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconCharge() {
  static const std::vector<std::string> rows = {
      "...oooooo...",
      "..oppppppo..",
      ".oppwwppmmo.",
      "oppwwppppmmo",
      "opppppppppmo",
      "opppppppppmo",
      "oppppppppmmo",
      "opppppppmmmo",
      ".opppppmmmo.",
      "..ommmmmmo..",
      "...oooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconHealHeart() {
  static const std::vector<std::string> rows = {
      "............",
      ".oo...oo....",
      "oRRo.oRRo...",
      "oRWRoRRRo...",
      "oRRRRRRRo...",
      "oRRRRRRoooo.",
      ".oRRRRRogo..",
      "..oRRRooggoo",
      "...oRRoggggo",
      "....ooogoooo",
      "......ogo...",
      "......ooo...",
  };
  return rows;
}

const std::vector<std::string>& IconClock() {
  static const std::vector<std::string> rows = {
      "............",
      "...oooooo...",
      "..owwwwwwo..",
      ".owwwoowwwo.",
      ".owwwoowwwo.",
      ".owwwoowwwo.",
      ".owwwooooow.",
      ".owwwwwwwwo.",
      ".owwwwwwwwo.",
      "..owwwwwwo..",
      "...oooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconSoul() {
  static const std::vector<std::string> rows = {
      "............",
      ".....o......",
      "....oKo.....",
      "...oKKKo....",
      "...oKwKo....",
      "....oKo.....",
      ".oo..o..oo..",
      "oRRo...oRRo.",
      "oRWRoooRRRo.",
      ".oRRRRRRRo..",
      "..oRRRRRo...",
      "....ooo.....",
  };
  return rows;
}

const std::vector<std::string>& IconThorns() {
  static const std::vector<std::string> rows = {
      "o....oo....o",
      "oo..oRRo..oo",
      ".ooooRRoooo.",
      "..oRRRRRRo..",
      "ooRRWRRRRRoo",
      "ooRRRRRRRRoo",
      "..oRRRRRRo..",
      ".oooRRRRooo.",
      "oo..oRRo..oo",
      "o....oo....o",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconShield() {
  static const std::vector<std::string> rows = {
      "............",
      "oooooooooooo",
      "occcccwwccco",
      "occccwwccco.",
      "occcccccccco",
      "occcccccccco",
      ".occcccccco.",
      ".occcccccco.",
      "..occcccco..",
      "...occcco...",
      "....oooo....",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconFeather() {
  static const std::vector<std::string> rows = {
      "..........oo",
      "........oowo",
      "......oowwo.",
      ".....owwsso.",
      "....owwsso..",
      "...owwsso...",
      "..owwsso....",
      "..owsso.....",
      ".owsoo......",
      ".oso........",
      "oso.........",
      "oo..........",
  };
  return rows;
}

const std::vector<std::string>& IconDashLines() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "oooo...oooo.",
      "......oppppo",
      "ooooooppwppo",
      "......oppppo",
      "oooo..oppppo",
      "......oppppo",
      "oooooopoppoo",
      ".......o.o..",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconDashSlots() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "oooo.oooo.oo",
      "occo.occo.oc",
      "occo.occo.oc",
      "occo.occo.oc",
      "occo.occo.oc",
      "occo.occo.oc",
      "oooo.oooo.oo",
      "............",
      "oooooooooooo",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconWing() {
  static const std::vector<std::string> rows = {
      "............",
      "o...........",
      "oo..........",
      "owo.........",
      "owwoo.......",
      "owwwwoo.....",
      "owswwwwoooo.",
      ".owsswwwwwwo",
      "..ossswwwwo.",
      "...oossswo..",
      ".....oooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconGhostFade() {
  static const std::vector<std::string> rows = {
      "....oooo....",
      "..oopppoo...",
      ".opppppppo..",
      ".oppwppwpo..",
      ".oppwppwpo..",
      ".opppppppo..",
      ".opppppppo..",
      ".opppppppo..",
      ".opopopopo..",
      ".o.o.o.o.o..",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconBlade() {
  static const std::vector<std::string> rows = {
      "..........oo",
      ".........owo",
      "........owso",
      ".......owso.",
      "......owso..",
      ".....owso...",
      "oooooowso...",
      "....owso....",
      "...owso.....",
      "..owso......",
      ".owso.......",
      "ooo.........",
  };
  return rows;
}

const std::vector<std::string>& IconGhostBurst() {
  static const std::vector<std::string> rows = {
      "y....o.....y",
      ".y..opoo..y.",
      "...opppppo..",
      "..oppwppwpo.",
      "y.oppwppwpo.",
      "..opppppppo.",
      "..opppppppo.",
      "..opopopopo.",
      ".y.........y",
      "y...y..y....",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconGauge() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "oooooooooooo",
      "oppppppmmmmo",
      "opwwppmmmmmo",
      "oppppppmmmmo",
      "oooooooooooo",
      "............",
      "....oo......",
      "...oyyo.....",
      "..oyyyyo....",
      "...oooo.....",
  };
  return rows;
}

const std::vector<std::string>& IconRain() {
  static const std::vector<std::string> rows = {
      "..oooooooo..",
      ".ommmmmmmmo.",
      "ommmmmmmmmmo",
      ".oooooooooo.",
      "..o...o...o.",
      ".opo.opo.opo",
      "..o...o...o.",
      "....o...o...",
      "...opo.opo..",
      "....o...o...",
      "..o...o.....",
      ".opo.opo....",
  };
  return rows;
}

const std::vector<std::string>& IconVortex() {
  static const std::vector<std::string> rows = {
      "...oooooo...",
      "..ommmmmmo..",
      ".ommoooommo.",
      "omooppppomo.",
      "omopoooopomo",
      "omopomoopomo",
      "omopooopomo.",
      "omoppppoomo.",
      ".ommoooommo.",
      "..ommmmmmo..",
      "...oooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconNightmare() {
  static const std::vector<std::string> rows = {
      "............",
      "...oooooo...",
      "..orrrrrro..",
      ".orrwwwwrro.",
      "orrwwkkwwrro",
      "orwwkoowwwro",
      "orwwkookwwro",
      "orrwwkkwwrro",
      ".orrwwwwrro.",
      "..orrrrrro..",
      "...oooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconMagnet() {
  static const std::vector<std::string> rows = {
      "............",
      ".oooo..oooo.",
      ".orro..oswo.",
      ".orro..oswo.",
      ".orro..oswo.",
      ".orroooosso.",
      ".orrrrsssso.",
      "..orrrssso..",
      "...oooooo...",
      "............",
      "..y..y..y...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconXp() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "oo..oo.oooo.",
      "ogo.ogoggggo",
      ".ogogo.ogoog",
      "..ogo..oggg.",
      ".ogogo.ogo..",
      "ogo.ogoogo..",
      "oo..oo..oo..",
      "............",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconCombo() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "oo..oo.oooo.",
      "oyo.oyoyyyyo",
      ".oyoyooooyyo",
      "..oyo..oyyo.",
      ".oyoyooyyo..",
      "oyo.oyoyyyyo",
      "oo..oo.oooo.",
      "............",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconCoins() {
  static const std::vector<std::string> rows = {
      "............",
      "....oooo....",
      "...oyyyyo...",
      "..oyhyyydo..",
      "...oyyyyo...",
      ".oooooooooo.",
      "oyyyyoyyyyyo",
      "oyhyyyoyyydo",
      "oyyyyoyyyydo",
      ".oooo.ooooo.",
      "............",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconClover() {
  static const std::vector<std::string> rows = {
      "............",
      "...oo..oo...",
      "..oggooggo..",
      "..ogggoggo..",
      "...oggggo...",
      ".oogggggggo.",
      "ogggooggoggo",
      ".ogo..oggo..",
      "..o...oo.o..",
      "......o.....",
      ".....o......",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconReroll() {
  static const std::vector<std::string> rows = {
      "............",
      "...ooooo....",
      "..occcccoo..",
      ".oco...occo.",
      ".oo.....oco.",
      "........oo..",
      "..oo........",
      ".oco.....oo.",
      ".occo...oco.",
      "..oocccccoo.",
      "....ooooo...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconCards() {
  static const std::vector<std::string> rows = {
      "............",
      "..oooooo....",
      "..owwwwoooo.",
      "oooowwowwwo.",
      "owwowwowwwo.",
      "owwowwowwwo.",
      "owwowwowwwo.",
      "owwooooowwo.",
      "owwwwwwowwo.",
      "owwwwwwoooo.",
      "oooooooo....",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconMiniGhost() {
  static const std::vector<std::string> rows = {
      "............",
      "............",
      "....oooo....",
      "...oppppo...",
      "..oppppppo..",
      "..opwppwpo..",
      "..opwppwpo..",
      "..oppppppo..",
      "..oppppppo..",
      "..opopopo...",
      "..o.o.o.o...",
      "............",
  };
  return rows;
}

const std::vector<std::string>& IconCrackedHeart() {
  static const std::vector<std::string> rows = {
      "............",
      ".oo....oo...",
      "oRRo..oRRo..",
      "oRWRooRRRo..",
      "oRRRoRRRRo..",
      "oRRRRoRRRo..",
      ".oRRoRRRo...",
      "..oRRoRo....",
      "...oRoo.....",
      "....oo......",
      "..r.r.r.....",
      "............",
  };
  return rows;
}

}  // namespace

// 카드별 아이콘. 비슷한 카드는 같은 아이콘을 함께 쓴다 (* = 예전 상점 아이콘 재사용).
const std::vector<std::string>& CardIconRows(CardId id) {
  switch (id) {
    case CardId::AttackUp: return IconPower();
    case CardId::FireRate: return IconRate();
    case CardId::RangeUp: return IconRange();
    case CardId::ShotSpeed: return IconShotSpeed();
    case CardId::Knockback: return IconKnock();
    case CardId::Critical: return IconStar();
    case CardId::Pierce: return IconPierce();
    case CardId::BackShot: return IconBackShot();
    case CardId::Execute: return IconSkull();
    case CardId::MultiShot: return IconMulti();
    case CardId::Bounce: return IconBounce();
    case CardId::BurstShot: return IconBurst();
    case CardId::Curse: return IconEye();
    case CardId::Homing: return IconTarget();
    case CardId::Split: return IconSplit();
    case CardId::ChargeShot: return IconCharge();
    case CardId::MaxHeart: return IconMaxHp();
    case CardId::Heal: return IconHealHeart();
    case CardId::LongGuard: return IconClock();
    case CardId::SoulDrain: return IconSoul();
    case CardId::Thorns: return IconThorns();
    case CardId::Shield: return IconShield();
    case CardId::ExtraLife: return IconLife();
    case CardId::MoveSpeed: return IconSpeed();
    case CardId::JumpUp: return IconDoubleJump();
    case CardId::AirJump: return IconDoubleJump();
    case CardId::Float: return IconFeather();
    case CardId::DashRange: return IconDashLines();
    case CardId::DashRecharge: return IconClock();
    case CardId::DashCharge: return IconDashSlots();
    case CardId::AirDash: return IconWing();
    case CardId::PhaseDash: return IconGhostFade();
    case CardId::ShadowRush: return IconBlade();
    case CardId::Afterimage: return IconGhostBurst();
    case CardId::GaugeUp: return IconGauge();
    case CardId::UltPower: return IconUlt();
    case CardId::HeadStart: return IconGauge();
    case CardId::UltRain: return IconRain();
    case CardId::UltBlackHole: return IconVortex();
    case CardId::UltNightmare: return IconNightmare();
    case CardId::Magnet: return IconMagnet();
    case CardId::XpBoost: return IconXp();
    case CardId::ComboKeep: return IconCombo();
    case CardId::Greed: return IconCoins();
    case CardId::Luck: return IconClover();
    case CardId::Reroll: return IconReroll();
    case CardId::LastStand: return IconCrackedHeart();
    case CardId::MoreChoice: return IconCards();
    case CardId::Familiar: return IconMiniGhost();
    case CardId::Count: break;
  }
  return IconPower();
}

}  // namespace gengar::art::data
