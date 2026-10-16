#include "../src/xrGame/bodycam_weapon_follow.h"
#include <cassert>
#include <limits>
#include <iostream>
using Bodycam::WeaponFollowState;
float run(int fps) {
 WeaponFollowState s; s.Update(0,0,1.f/fps,0,1,true,.16f,1,1);
 for(int i=1;i<=fps;++i) s.Update(float(i)/fps,0,1.f/fps,i,1,true,.16f,1,1);
 assert(s.yaw<0 && s.yaw>-.17f); return s.yaw;
}
int main() {
 assert(std::abs(run(30)-run(144))<.00001f);
 assert(std::abs(run(60)-run(144))<.00001f);
 WeaponFollowState s;
 s.Update(3.13f,0,.01f,0,1,true,.16f,.1f,.1f);
 s.Update(-3.13f,.03f,.01f,1,1,true,.16f,.1f,.1f);
 assert(s.yaw<0 && s.yaw>-.03f && s.pitch<0);
 auto before=s.yaw; s.Update(-3.f,0,.01f,1,1,true,.16f,.1f,.1f); assert(s.yaw==before);
 for(int i=2;i<100;++i) s.Update(-3.13f,.03f,.01f,i,1,true,.16f,.1f,.1f);
 assert(std::abs(s.yaw)<.0001f);
 s.Update(-2.5f,0,.01f,100,1,true,.16f,.1f,.1f);assert(s.yaw==-.1f);
 s.Update(-2.9f,0,.01f,101,1,true,.16f,.1f,.1f);assert(s.yaw>0 && s.yaw<=.1f);
 s.Update(-2.9f,0,.01f,102,1,true,.16f,0,0);assert(s.yaw==0 && s.pitch==0);
 s.Update(-2.6f,0,.01f,103,1,true,.16f,.1f,.1f);assert(s.yaw<0);
 s.Update(-2.5f,0,.01f,104,2,true,.16f,.1f,.1f);assert(s.yaw==0);
 s.Update(-2.3f,0,.01f,107,2,true,.16f,.1f,.1f);assert(s.yaw==0);
 s.Update(-2.2f,0,.4f,108,2,true,.16f,.1f,.1f);assert(s.yaw==0);
 s.Update(-2.1f,0,.01f,109,2,false,.16f,.1f,.1f);assert(s.yaw==0 && !s.initialized);
 s.Update(std::numeric_limits<float>::quiet_NaN(),0,.01f,110,2,true,.16f,.1f,.1f);assert(!s.initialized);
 std::cout<<"Weapon follow tests passed: FPS independence, direction, wrap, caps, reversal, ADS, reset, duplicate frames, invalid inputs\n";
}
