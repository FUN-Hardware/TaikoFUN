#include "Scene/Demo/DemoLogic.h"
#include <cstdlib>
#include <iostream>
#include <limits>

void Check(bool value, const char* message) {
    if (!value) { std::cerr << message << '\n'; std::exit(1); }
}
int main() {
    using namespace DemoLogic;
    IdleTimer timer;
    Check(!timer.Update(1,true,false,false,true), "first eligible frame does not count old time");
    for (int i=0;i<29;++i) Check(!timer.Update(1,true,false,false,true), "does not start before 30 seconds");
    Check(!timer.Update(1,true,true,true,true), "input wins at idle boundary");
    timer.Update(0,true,false,false,true);
    for (int i=0;i<29;++i) timer.Update(1,true,false,false,true);
    Check(timer.Update(1,true,false,false,true), "starts after 30 seconds");
    Check(!timer.Update(1,true,true,false,true), "holding a key resets idle even without a new press");
    Check(!timer.Update(1,false,false,false,true), "inactive window resets idle");
    Check(!timer.Update(60,true,false,false,true), "refocus cannot count suspended time");
    Check(!timer.Update(1,true,false,false,false), "course choice or unavailable song blocks demo");
    Check(!timer.Update(std::numeric_limits<double>::quiet_NaN(),true,false,false,true), "invalid delta is ignored");
    ReleaseGate gate;
    gate.Reset(true);
    Check(!gate.Update(true), "cancel key is consumed");
    Check(!gate.Update(true), "overlapping second key is consumed");
    Check(!gate.Update(false), "release frame is consumed");
    Check(gate.Update(true), "fresh input after complete release is accepted");
    gate.Reset(false);
    Check(gate.Update(true), "ordinary menu input remains immediate");
    const std::array<bool,5> courses{true,false,true,true,false};
    Check(ChooseCourse(courses,2) == 2, "available preferred course is kept");
    Check(ChooseCourse(courses,1) == 0, "fallback selects first available course");
    Check(ChooseCourse(courses,99) == 0, "invalid preferred index is safe");
    Check(ChooseCourse({false,false,false,true,false},0) == 3, "Oni-only song can demo");
    Check(!ChooseCourse({false,false,false,false,false},0), "empty courses cannot start");
    Session session;
    Check(session.FadeOpacity() == 1, "entry begins black");
    Check(session.Update(.2,false,false) == Exit::None, "entry animation keeps playing");
    Check(session.FadeOpacity() > .4999 && session.FadeOpacity() < .5001, "entry fade midpoint");
    session.Update(.2,false,false);
    Check(session.FadeOpacity() == 0, "entry becomes visible after .4 seconds");
    Check(session.Update(.1,true,false) == Exit::None, "finish starts a separate linger");
    session.Update(.6,true,false);
    Check(session.FadeOpacity() < .00001, "exit fade starts in final .4 seconds");
    session.Update(.2,true,false);
    Check(session.FadeOpacity() > .4999 && session.FadeOpacity() < .5001, "exit fade midpoint");
    Check(session.Update(.2,true,false) == Exit::Finished && session.FadeOpacity() == 1, "one-second natural exit is black");
    session.Reset();
    Check(session.Update(1,true,true) == Exit::Cancelled, "cancel takes priority over natural finish");
    session.Reset();
    Check(session.Update(.1,false,true) == Exit::Cancelled, "cancel during entry is immediate");
    std::cout << "Demo timing and input checks passed\n";
}
