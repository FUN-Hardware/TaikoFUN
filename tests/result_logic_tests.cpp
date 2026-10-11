#include "Scene/Result/ResultLogic.h"
#include <climits>
#include <cstdlib>
#include <iostream>

using namespace ResultLogic;
void Check(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
    Check(Calculate(0,0,0,0,0,0).Hundredths() == 0, "empty accuracy");
    Check(Calculate(0,0,0,0,0,0).FinalRank() == Rank::C, "empty rank");
    Check(Calculate(1,1,0,0,20,7).Hundredths() == 7500, "OK half weight");
    Check(Calculate(1,1,0,0,20,7).roll == 27, "roll and balloon aggregate");
    Check(Calculate(2,0,3,5,0,0).bad == 8, "bad includes misses");
    Check(Calculate(59,0,0,41,0,0).FinalRank() == Rank::C, "below B");
    Check(Calculate(60,0,0,40,0,0).FinalRank() == Rank::B, "B boundary");
    Check(Calculate(80,0,0,20,0,0).FinalRank() == Rank::A, "A boundary");
    Check(Calculate(90,0,0,10,0,0).FinalRank() == Rank::S, "S boundary");
    Check(Calculate(100,0,0,0,0,0).FinalRank() == Rank::SS, "perfect boundary");
    Check(Calculate(5999,0,0,4001,0,0).FinalRank() == Rank::C, "59.99 C");
    auto almost = Calculate(99999,0,0,1,0,0);
    Check(almost.FinalRank() == Rank::S && almost.Hundredths() == 9999, "99.999 must not round to SS");
    Check(Calculate(INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX).Hundredths() == 3750, "64-bit arithmetic");
    Animation a;
    const auto perfect = Calculate(100,0,0,0,0,0);
    a.Reset(false);
    Check(a.Progress() == 0 && a.FadeOpacity() == 1, "initial animation");
    a.Update(.4,false,false);
    Check(a.Progress() == 0 && a.FadeOpacity() == 0, "fade precedes gauge");
    a.Update(1.5,false,false);
    Check(a.Progress() > .8749 && a.Progress() < .8751, "ease-out cubic midpoint");
    Check(a.DisplayRank(perfect) != Rank::SS, "no early SS");
    Check(!a.Update(1.5,true,true) && a.Complete(), "completion-frame Don only skips");
    Check(a.DisplayHundredths(perfect) == 10000 && a.DisplayRank(perfect) == Rank::SS, "final perfect");
    Check(!a.Update(0,true,true), "second key while holding cannot exit");
    a.Update(0,false,false);
    Check(a.Update(0,true,true), "released then new press exits");
    a.Reset(true);
    Check(!a.Update(.1,true,true) && !a.Complete(), "held on entry is ignored");
    a.Update(0,false,false);
    Check(!a.Update(0,true,true) && a.Complete(), "fade skip cannot exit");
    Check(a.DisplayHundredths(almost) == 9999 && a.DisplayRank(almost) == Rank::S, "skip uses exact final rules");
    a.Reset(false);
    a.Update(3.4,false,false);
    Check(a.Complete() && a.Update(0,true,true), "natural completion permits next press");
    a.Reset(false,false);
    Check(a.Update(0,true,true), "missing result can return");
    PlayEndTransition end;
    end.Update(4.6);
    Check(end.FadeOpacity() < .00001 && !end.Complete(), "five-second linger starts fade at 4.6");
    end.Update(.2);
    Check(end.FadeOpacity() > .4999 && end.FadeOpacity() < .5001, "fade midpoint");
    end.Update(.2);
    Check(end.Complete() && end.FadeOpacity() == 1, "five-second end is black");
    end.Reset();
    Check(!end.Complete() && end.FadeOpacity() == 0, "restart clears linger");
    std::cout << "Result logic checks passed\n";
}
