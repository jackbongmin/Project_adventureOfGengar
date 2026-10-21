// 이 파일은 도트 스프라이트 격자 데이터다. 문자 하나 = 픽셀 하나, '.' = 투명.
// 색은 Art.cpp 의 팔레트 표에서 문자별로 지정한다. 손으로 직접 고쳐도 된다.
#include "art/SpriteData.h"

namespace gengar::art::data {

const std::vector<std::string>& Phantom0() {
  static const std::vector<std::string> rows = {
      ".....o...........oo.....",
      "....olo.........oppo....",
      "....ollo.......opppo....",
      "....olllo.....oppppo....",
      "...oollllooooopppppo....",
      ".ooddllllppppppppppo....",
      "oddddlllpppppppppppo....",
      ".odddllppppppppppppo....",
      "..oollrrpppppppppprro...",
      "..ollprrrrpppppprrrrpo..",
      "..olppprrwrrpprrwrrppo..",
      "..oppppprrrpppprrrpppo..",
      ".odppppppppppppppppppo..",
      "oddppkpppppppppppppkdo..",
      ".oppppkkkkkkkkkkkkkdddo.",
      ".oppppkwwkwwkwwkwwkdddo.",
      ".opppppkwkwwkwwkwkddddo.",
      ".oppppppkwwwwwwwkdddddo.",
      "..oppppppkkkkkkkdddddo..",
      ".oddddppppppdddddddoo...",
      "..oddddppppdddddddo.....",
      "...ooooppooooooodo......",
      ".......oo......odo......",
      "................o.......",
  };
  return rows;
}

const std::vector<std::string>& Phantom1() {
  static const std::vector<std::string> rows = {
      ".....o...........oo.....",
      "....olo.........oppo....",
      "....ollo.......opppo....",
      "....olllo.....oppppo....",
      "...oollllooooopppppo....",
      ".ooddllllppppppppppo....",
      "oddddlllpppppppppppo....",
      ".odddllppppppppppppo....",
      "..oollrrpppppppppprro...",
      "..ollprrrrpppppprrrrpo..",
      "..olppprrwrrpprrwrrppo..",
      "..oppppprrrpppprrrpppo..",
      ".odppppppppppppppppppo..",
      "oddppkpppppppppppppkdo..",
      ".oppppkkkkkkkkkkkkkdddo.",
      ".oppppkwwkwwkwwkwwkdddo.",
      ".opppppkwkwwkwwkwkddddo.",
      ".oppppppkwwwwwwwkdddddo.",
      "..oppppppkkkkkkkdddddo..",
      ".oddddppppppddddddooo...",
      "..oddddppppddddddo......",
      "...oooodoooddoooo.......",
      ".......o...oo...........",
      "........................",
  };
  return rows;
}

const std::vector<std::string>& Slime0() {
  static const std::vector<std::string> rows = {
      "................",
      "................",
      "....ooooooo.....",
      "...oWWgggggo....",
      "...oggggggggo...",
      "..ogwwwggwwwgo..",
      ".oggwkkggwkkggo.",
      "ogggwkkggwkkgddo",
      "oggggggggggddddo",
      "ogggggggggdddddo",
      "oggggggggddddddo",
      ".oooooooooooooo.",
  };
  return rows;
}

const std::vector<std::string>& Slime1() {
  static const std::vector<std::string> rows = {
      "................",
      "................",
      "................",
      "....ooooooo.....",
      "...oWWgggggoo...",
      "..oggggggggggo..",
      ".oggwwwggwwwggo.",
      "ogggwkkggwkkgddo",
      "ogggwkkggwkkdddo",
      "ggggggggggdddddd",
      "gggggggggddddddd",
      "oooooooooooooooo",
  };
  return rows;
}

const std::vector<std::string>& Bat0() {
  static const std::vector<std::string> rows = {
      "....................",
      "..o..............o..",
      ".oWo...o....o...owo.",
      "oWWWo.oboooobo.owwo.",
      "oWWWWoobbbbbboowwwwo",
      "oWowwwobbbbbbowwwovo",
      ".oowwwwbbbbbbwwwvoo.",
      "...owwwbybbybwwvo...",
      "...owowbbbbbbvovo...",
      "....o.obbbbbbo.o....",
      "......obbfbfbo......",
      ".......obbbbo.......",
      "........oooo........",
      "....................",
  };
  return rows;
}

const std::vector<std::string>& Bat1() {
  static const std::vector<std::string> rows = {
      "....................",
      "....................",
      ".......o....o.......",
      "......oboooobo......",
      "......obbbbbbo......",
      "......obbbbbbo......",
      ".....oobbbbbbo......",
      "..ooowwbybbybwooo...",
      ".owwwwwbbbbbbvvvvoo.",
      "owwwwwwbbbbbbvvvvvvo",
      "owwwwwwbbfbfbvvvvvvo",
      ".owowwwobbbbovvvovo.",
      ".owoowo.oooo.ovoovo.",
      "..o..o........o..o..",
  };
  return rows;
}

const std::vector<std::string>& Hound0() {
  static const std::vector<std::string> rows = {
      "................opo.....",
      "o..............opppoo...",
      "lo....o...o..ooppppppo..",
      "olo..ovooovoovppppppppo.",
      "ollooovvpvvppvvppppeeppo",
      ".ollpppppppppppppppppppp",
      "..oppppppppppppppppppppd",
      ".opppppppppppppppppppddd",
      ".opppppppppppppppppooooo",
      ".oppppppppppppppppooowwo",
      "..opppppppppppppdooddoo.",
      "...oppppppppppddo.oddo..",
      "...oppopppppdddo..oddo..",
      "...oppoooppooddo..oddo..",
      "...oppo..oo.oddo...oo...",
      "....oo.......oo.........",
  };
  return rows;
}

const std::vector<std::string>& Hound1() {
  static const std::vector<std::string> rows = {
      "................opo.....",
      "o..............opppoo...",
      "lo....o...o..ooppppppo..",
      "olo..ovooovoovppppppppo.",
      "ollooovvpvvppvvppppeeppo",
      ".ollpppppppppppppppppppp",
      "..oppppppppppppppppppppd",
      ".opppppppppppppppppppddd",
      ".opppppppppppppppppooooo",
      ".oppppppppppppppppooowwo",
      "..opppppppppppppdoddooo.",
      "...oppppppppppdddoddo...",
      "....opppppppdooddoddo...",
      "....oppppoooo.oddoddo...",
      ".....ooppo.....oooddo...",
      ".......oo.........oo....",
  };
  return rows;
}

const std::vector<std::string>& Caster0() {
  static const std::vector<std::string> rows = {
      "...........ommmo",
      "..........ommmmm",
      "......ooooommmmm",
      ".....olpppoommmo",
      "....olpppppoobo.",
      "...olppkkkkpobo.",
      "...oppkkkkkkobo.",
      "..olppkkkkkkpbo.",
      "...oppkkrkkrobo.",
      "...oppkkkkkkobo.",
      "....oppkkkkoobo.",
      ".....oppppo.obo.",
      "....oppppppoobo.",
      "....oppppppoobo.",
      "....opppppdoobo.",
      "...opppppdddobo.",
      "...opppppdddobo.",
      "..opppppddddobo.",
      ".odddddddddddbo.",
      ".odddddddddddbo.",
  };
  return rows;
}

const std::vector<std::string>& Caster1() {
  static const std::vector<std::string> rows = {
      "...........ommmo",
      "..........ommwwm",
      "......ooooommmmm",
      ".....olpppoommmo",
      "....olpppppoobo.",
      "...olppkkkkpobo.",
      "...oppkkkkkkobo.",
      "..olppkkkkkkpbo.",
      "...oppkkrkkrobo.",
      "...oppkkkkkkobo.",
      "....oppkkkkoobo.",
      ".....oppppo.obo.",
      "....oppppppoobo.",
      "....oppppppoobo.",
      "....opppppdoobo.",
      "...opppppdddobo.",
      "...opppppdddobo.",
      "..opppppddddobo.",
      ".odddddddddddbo.",
      ".odddddddddddbo.",
  };
  return rows;
}

const std::vector<std::string>& Coin0() {
  static const std::vector<std::string> rows = {
      ".ohhhyo.",
      "ohhhyyyo",
      "ohhyyyyo",
      "ohyyyyyo",
      "oyyyyydo",
      "oyyyyddo",
      "oyyydddo",
      ".oydddo.",
  };
  return rows;
}

const std::vector<std::string>& Coin1() {
  static const std::vector<std::string> rows = {
      "..ohho..",
      ".ohhyyo.",
      ".ohyyyo.",
      ".oyyyyo.",
      ".oyyyyo.",
      ".oyyydo.",
      ".oyyddo.",
      "..oddo..",
  };
  return rows;
}

const std::vector<std::string>& Coin2() {
  static const std::vector<std::string> rows = {
      "..ohho..",
      "..ohyo..",
      "..oyyo..",
      "..oyyo..",
      "..oyyo..",
      "..oyyo..",
      "..oydo..",
      "..oddo..",
  };
  return rows;
}

const std::vector<std::string>& Coin3() {
  static const std::vector<std::string> rows = {
      "..ohho..",
      ".ohhyyo.",
      ".ohyyyo.",
      ".oyyyyo.",
      ".oyyyyo.",
      ".oyyydo.",
      ".oyyddo.",
      "..oddo..",
  };
  return rows;
}

const std::vector<std::string>& HeartFull() {
  static const std::vector<std::string> rows = {
      ".oo...oo.",
      "oRRo.oRRo",
      "oRWRoRRRo",
      "oRRRRRRRo",
      ".oRRRRRo.",
      "..oRRRo..",
      "...oRo...",
      "....o....",
  };
  return rows;
}

const std::vector<std::string>& HeartEmpty() {
  static const std::vector<std::string> rows = {
      ".oo...oo.",
      "oEEo.oEEo",
      "oEEEoEEEo",
      "oEEEEEEEo",
      ".oEEEEEo.",
      "..oEEEo..",
      "...oEo...",
      "....o....",
  };
  return rows;
}

const std::vector<std::string>& Ball0() {
  static const std::vector<std::string> rows = {
      "..rrrr..",
      ".rmmmmr.",
      "rmhhdmmr",
      "rmdddlmr",
      "rmdddlmr",
      "rmmdlmmr",
      ".rmmmmr.",
      "..rrrr..",
  };
  return rows;
}

const std::vector<std::string>& Ball1() {
  static const std::vector<std::string> rows = {
      "..rrrr..",
      ".rmmmmr.",
      "rmhhdmmr",
      "rmddddmr",
      "rmdddlmr",
      "rmmllmmr",
      ".rmmmmr.",
      "..rrrr..",
  };
  return rows;
}

const std::vector<std::string>& Orb0() {
  static const std::vector<std::string> rows = {
      "..oooo..",
      ".oppppo.",
      "oppwwppo",
      "opwwwwpo",
      "opwwwwpo",
      "oppwwppo",
      ".oppppo.",
      "..oooo..",
  };
  return rows;
}

const std::vector<std::string>& Wave0() {
  static const std::vector<std::string> rows = {
      ".....oo.....",
      "....oppo....",
      "....oppo....",
      "....oppo....",
      "...oppppo...",
      "...oppppo...",
      "..opppwppo..",
      "..oppwwppo..",
      ".opppwwwppo.",
      "opppwwwwpppo",
      "opppwwwwpppo",
      "pppwwwwwwppp",
  };
  return rows;
}

const std::vector<std::string>& Wave1() {
  static const std::vector<std::string> rows = {
      "............",
      ".....oo.....",
      "....oppo....",
      "....oppo....",
      "...oppppo...",
      "...oppppo...",
      "..opppwppo..",
      "..oppwwppo..",
      ".opppwwwppo.",
      "opppwwwwpppo",
      "opppwwwwpppo",
      "pppwwwwwwppp",
  };
  return rows;
}

const std::vector<std::string>& Pennant0() {
  static const std::vector<std::string> rows = {
      "o.....olllloo....oll",
      "looooollllllloooolll",
      "lllllllpppplllllllpp",
      "plllllpppppppllllppo",
      "ppppppppppppppppppo.",
      "pppppppppppppppppo..",
      "ppppppppppppppppo...",
      "ppppppppoooooooo....",
      "pppppppo............",
      "ppppppo.............",
      "pppppo..............",
      "opooo...............",
  };
  return rows;
}

const std::vector<std::string>& Pennant1() {
  static const std::vector<std::string> rows = {
      "..olllloo....olllllo",
      "oollllllloooolllllll",
      "lllpppplllllllpppppl",
      "llpppppppllllppppppp",
      "ppppppppppppppppoooo",
      "ppppppppppppppoo....",
      "pppppppppppppo......",
      "pppppppppppppo......",
      "pppppoopoppoo.......",
      "pppoo..o.oo.........",
      "ppo.................",
      "ppo.................",
  };
  return rows;
}

const std::vector<std::string>& Pennant2() {
  static const std::vector<std::string> rows = {
      "llllo....oollllo....",
      "llllloooollllllloooo",
      "pppplllllllpppplllll",
      "pppppllllpppppppllll",
      "pppppppppppppppppppp",
      "pppppppppppppooppppo",
      "pppppppppppoo..oooo.",
      "pppppppppppo........",
      "pppppppppoo.........",
      "ppooppppo...........",
      "oo..oooo............",
      "....................",
  };
  return rows;
}

const std::vector<std::string>& Moon() {
  static const std::vector<std::string> rows = {
      "........MMmmmm........",
      ".....MMMMmmmmmmmm.....",
      "....MMMMmmmmmmmmmm....",
      "...MMMMmmmmmmmmmmmm...",
      "..MMMMmmmmmmmmmmmmmm..",
      ".MMMMcccmmmmmcccmmmmm.",
      ".MMmmcccmmmmcccccmmmm.",
      ".Mmmmmmmmmmmcccccmmmm.",
      "Mmmmmmmmmmmmmcccmmmmmm",
      "mmmmmmmmmmmmmmmmmmmmmm",
      "mmmmmmmmmmmmmmmmmmmmmm",
      "mmmmmccccmmmmmmmmmmmmd",
      "mmmmccccccmmmmmmmmmddd",
      "mmmmccccccmmmmccmmdddd",
      ".mmmccccccmmmccccdddd.",
      ".mmmccccccmmmccccdddd.",
      ".mmmmccccmmmmmccddddd.",
      "..mmmmmmmmmmmmdddddd..",
      "...mmmmmmmmmddddddd...",
      "....mmmmmmmddddddd....",
      ".....mmmmmddddddd.....",
      "........mddddd........",
  };
  return rows;
}

const std::vector<std::string>& TileGroundTop() {
  static const std::vector<std::string> rows = {
      "TTTTTTTTTTTTTTTT",
      "TTTTTTTTTTTTTTTT",
      "tttttttttttttttt",
      "ttDtDDDttDDDDDDt",
      "DDDDDDDDDtDDtDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDLDDDD",
      "DDDDDDSDDDDDDDDD",
      "DDDDDDDDDDDDLDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDLDSDDDSDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDSDDDDDDDDDD",
      "DDDDDDDDDSDDDSDD",
  };
  return rows;
}

const std::vector<std::string>& TileDirt() {
  static const std::vector<std::string> rows = {
      "DDSSDDSDLLDDDDDD",
      "DDDDDDDDDLLDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDSDDDDSDDD",
      "DSDDDSDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDSDDDDDLLDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDSDDDDDDDDDDDD",
      "DDDDLLDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
      "DDDDDDDDDDDDDDDD",
  };
  return rows;
}

const std::vector<std::string>& TileBrick() {
  static const std::vector<std::string> rows = {
      "HHHHHHHHHHHHHHHS",
      "HBBBBMBBBBBBBBBS",
      "HBBBBMBBBBBBBBBS",
      "HBBBBMBBBBBBBBBS",
      "HBBBBMBBBBBBBBBS",
      "HBBBBMBBBBBBBBBS",
      "HSSSSSSSSSSSSSSS",
      "HMMMMMMMMMMMMMMS",
      "HHHHHHHHHHHHHHHS",
      "HBBBBMBBBBBBBMBS",
      "HBBBBMBBBBBBBMBS",
      "HBBBBMBBBBBBBMBS",
      "HBBBBMBBBBBBBMBS",
      "HBBBBMBBBBBBBMBS",
      "HSSSSSSSSSSSSSSS",
      "HMMMMMMMMMMMMMMS",
  };
  return rows;
}

const std::vector<std::string>& TileCrumble() {
  static const std::vector<std::string> rows = {
      "HHHHHHHHHHHHHHHH",
      "HHHHHHHHHHHHHHHH",
      "BBBKBBBBBBBKBBBB",
      "BBBSKBBBBBKBBBBB",
      "BBBBKBBBSBKBBBBB",
      "BBBBBKBBBKBBBBBB",
      "BBBBBKBBBKBBBBBB",
      "BBBBKBBBBBKBBBBS",
      "BBBBKBBBBBKBSBBB",
      "BBBKBSBBBBBKBBBB",
      "BBBKBBBBBBBBKBBB",
      "BBKBBBBBBBBSKBBB",
      "BBKSBBBBBBBBBBBB",
      "BSSBBBBSBBBBBBBB",
      "SSSSSSSSSSSSSSSS",
      "SSSSSSSSSSSSSSSS",
  };
  return rows;
}

const std::vector<std::string>& TileSpike() {
  static const std::vector<std::string> rows = {
      "................",
      "................",
      "................",
      "................",
      "................",
      "................",
      "...oo......oo...",
      "..oLSo....oLSo..",
      "..oLSo....oLSo..",
      "..oLSo....oLSo..",
      ".oLLSSo..oLLSSo.",
      ".oLLSSo..oLLSSo.",
      "oLLLSSSooLLLSSSo",
      "oLLLSSSooLLLSSSo",
      "oLLLSSSooLLLSSSo",
      "LLLLSSSSLLLLSSSS",
  };
  return rows;
}

const std::vector<std::string>& Boss() {
  static const std::vector<std::string> rows = {
      "...............................x................................",
      "..............................xax...............................",
      "..............................xaax..............................",
      "..............................xaax..............................",
      "......................xx.....xaaax......xx......................",
      ".....................xaax....xaaaax....xaax.....................",
      ".....................xaax....xaaaax....xaax.....................",
      ".....................xaax...xaaaaax....xaax.....................",
      "....................xaaaaxxxxaaaaaaxxxxaaaax....................",
      ".............xx.....xaaaaxcccaaaaaacccxaaaax.....xx.............",
      "............xaax...xaaaaaaccaaaaaaacccaaaaaax...xaax............",
      "...........xaaaaxxxcaaaaaaccaaaaaaaaccaaaaaacxxxaaaax...........",
      "...........xaaaaxcccaaaaaaccaaaaaaaaccaaaaaacccxaaaax...........",
      "..........xaaaaaaccaaaaaaaaaaaaaaaaacaaaaaaaaccaaaaaax..........",
      ".........xaaaaaaaacaaaaaaaaaaaaaaaaaaaaaaaaaacaaaaaaaax.........",
      "..........xxmcccccccccmmmmmmmmmmmmmmmmmmmmccccccccccxx..........",
      "..........xcccccccccmmmmmmmmmmmmmmmmmmmmmmmmcccccccccx..........",
      ".........xccccccccmmmmmmmmmvvvvvvvvvvmmmmmmmmmccccccccx.........",
      "........xcccccccmmmmmmmmvvvvvvvvvvvvvvvvmmmmmmmmcccccccx........",
      ".......xcccccccmmmmmmmvvvvvvvvvvvvvvvvvvvvmmmmmmmcccccccx.......",
      ".......xccccccmmmmmmvvvvvvvvvxxxxxxvvvvvvvvvmmmmmmccccccx.......",
      "......xccccccmmmmmmvvvvvvvxxxxxxxxxxxxvvvvvvvmmmmmmccccccx......",
      ".....xccccccmmmmmmvvvvvvvxxxeeeeeeeexxxvvvvvvvmmmmmmccccccx.....",
      ".....xccccccmmmmmvvvvvvvxxeeeeeeeeeeeexxvvvvvvvmmmmmccccccx.....",
      "....xccccccmmmmmvvvvvvvxxeeeeffffffeeeexxvvvvvvvmmmmmccccccx....",
      "....xccccccmmmmvvvvvvvxxeeeeffffffffeeeexxvvvvvvvmmmmccccccx....",
      "...xcccccccmmmmvvvvvvvxxeeeffwwffffffeeexxvvvvvvvmmmmcccccccx...",
      "...xcccccccmmmvvvvvvvvxeeeffwwwwffffffeeexvvvvvvvvmmmcccccccx...",
      "...xcccccccmmmvvvvvvvxxeefffwwwwwffffffeexxvvvvvvvmmmcccccccx...",
      "..xccccccccmmvvvvvvvvxxeefffwwwwwffffffeexxvvvvvvvvmmccccccccx..",
      "..xcccccccccmvvvvvvvvxxeefffwwwwfffffffeexxvvvvvvvvmcccccccccx..",
      "..xccccccccccvvvvvvvvxxeeffffwwffffffffeexxvvvvvvvvccccccccccx..",
      "..xccccccccccvvvvvvvvxxeeffffffffffffffeexxvvvvvvvvccccccccccx..",
      "..xccccccccccvvvvvvvvxxeeffffffffffffffeexxvvvvvvvvccccccccccx..",
      "..xccccccccccvvvvvvvvvxeeeffffffffffffeeexvvvvvvvvvccccccccccx..",
      "..xccccccccccvvvvvvvvvxxeeeffffffffffeeexxvvvvvvvvvccccccccccx..",
      "..xccccccccccvvvvvvvvvxxeeeeffffffffeeeexxvvvvvvvvvccccccccccx..",
      "...xccccccccccvvvvvvvvvxxeeeeffffffeeeexxvvvvvvvvvccccccccccx...",
      "...xccccccccccvvvvvvvvvvxxeeeeeeeeeeeexxvvvvvvvvvvccccccccccx...",
      "...xcccccccccccvvvvvvvvvvxxxeeeeeeeexxxvvvvvvvvvvcccccccccccx...",
      "...xcccccccccccvvvvvvvvvvvxxxxxxxxxxxxvvvvvvvvvvvcccccccccccx...",
      "...xccccccccccccvvvvvvvvvvvvvxxxxxxvvvvvvvvvvvvvcccccccccccx....",
      "....xccccccccccccvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvccccccccccccx....",
      "....xcccccccccccccvvvvvvvvvvvvvvvvvvvvvvvvvvvvcccccccccccccx....",
      "....xccccccccccccccvvvvvvvvvvvvvvvvvvvvvvvvvvccccccccccccccx....",
      "....xcccccccccccccccvvvvvvvvvvvvvvvvvvvvvvvvcccccccccccccccx....",
      "....xcccccccccccccckkwwwwwwkwwwkwwkwwwkwwwwwwcccccccccccccx.....",
      ".....xccccccccccccckkwwwkwwkwwwkwwkwwwkwwkwwwcccccccccccccx.....",
      ".....xccccccccccccckkwwkkwkkwwkkwwkkwwkkwkkwwcccccccccccccx.....",
      ".....xcccccccccccccccwwccccccwccccccwccccccwwcccccccccccccx.....",
      ".....xccccccccccccccccwccccccwccccccwccccccwccccccccccccccx.....",
      ".....xccccccccccccccccwccccccwccccccwccccccwcccccccccccccx......",
      "......xccccccccccccccccccccccwccccccwccccccccccccccccccccx......",
      ".....xccccccccccccccccccccccccccccccccccccccccccccccccccccx.....",
      "......xccccccxccccccxxcccccxxccccccxxcccccxxccccccxccccccx......",
      ".......xccccxxccccccxxcccccxxccccccxxcccccxxccccccxxccccx.......",
      ".......xcccx..xcccccx.xcccx..xccccx..xcccx.xcccccx..xcccx.......",
      "........xccx..xccccx...xccx..xccccx..xccx...xccccx..xccx........",
      "........xcx...xccccx...xcx...xccccx...xcx...xccccx...xcx........",
      ".........x.....xcccx....x.....xccx.....x....xcccx.....x.........",
      "...............xccx...........xccx...........xccx...............",
      "................xcx...........xccx...........xcx................",
      ".................x.............xx.............x.................",
      "................................................................",
  };
  return rows;
}

const std::vector<std::string>& BossHand() {
  static const std::vector<std::string> rows = {
      "..............",
      "...x...x...x..",
      "..xax.xax.xax.",
      "..xax.xax.xax.",
      ".xaaxxaaxxaax.",
      ".xaaxxaaxxaax.",
      ".xaaxmaamxaax.",
      ".xaaamaaacaaax",
      ".xmmmmccccccx.",
      ".xmmmcccccccx.",
      "xmmmcccccccccx",
      "xmmccccccccccx",
      "xmcccccccccccx",
      "xccccccccccccx",
      ".xccccccccccx.",
      ".xccccccccccx.",
      "..xccccccccx..",
      "...xxccccxx...",
  };
  return rows;
}

}  // namespace gengar::art::data
