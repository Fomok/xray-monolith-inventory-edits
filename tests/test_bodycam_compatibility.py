"""Compile unmodified Bodycam simulation and PiP adapter outside X-Ray."""
from pathlib import Path
import argparse,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);a=p.parse_args()
r=Path(__file__).resolve().parents[1]
source=r'''
#include "bodycam_simulation.h"
#include "bodycam_pip_adapter.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 using namespace Bodycam;
 SimulationSettings settings; SimulationState state; SimulationInput input; SimulationOutput out;
 ResetSimulation(state,0,0,0,0);
 for(int i=0;i<3600;++i){
  input.dt=1.f/120.f;input.target_yaw=std::sin(i*0.01f)*0.5f;input.target_pitch=std::cos(i*0.02f)*0.2f;
  input.move_flags=(i/120)%2?smfForward|smfSprint:0;input.ads=(i/240)%2;input.ads_blend=input.ads?1.f:0.f;
  if(i%30==0)AddFireImpulse(settings,state,1.f,input.ads);
  UpdateSimulation(settings,state,input,out);
  assert(std::isfinite(out.yaw)&&std::isfinite(out.pitch)&&std::isfinite(out.roll));
  assert(std::isfinite(out.camera_pos.x)&&std::isfinite(out.viewmodel_rot.x));
 }
 ResetSimulation(state,0,0,0,0);input=SimulationInput{};input.dt=1.f/60.f;
 UpdateSimulation(settings,state,input,out);assert(std::isfinite(out.yaw));
 PipAdapter adapter; PipInput pi;pi.requested_fov=75.f; PipRuntimeState rt;
 assert(!adapter.Update(pi,rt).active);
 pi.weapon_zoomed=true;pi.primary_sight=true;rt.true_pip_enabled=true;rt.main_fov=90.f;
 assert(!adapter.Update(pi,rt).active);rt.geometry_ready=true;
 auto v=adapter.Update(pi,rt);assert(v.active&&v.freeze_world_pose&&v.main_fov==90.f);
 rt.geometry_ready=false;assert(adapter.Update(pi,rt).active);
 rt.world_camera_effects=true;assert(!adapter.Update(pi,rt).freeze_world_pose);
 pi.weapon_zoomed=false;v=adapter.Update(pi,rt);assert(!v.active&&v.main_fov==75.f);
 pi.weapon_zoomed=true;rt.viewport_active=true;assert(adapter.Update(pi,rt).active);
 adapter.Reset();rt.viewport_active=false;assert(!adapter.Update(pi,rt).active);
 rt.geometry_ready=true;rt.true_pip_enabled=false;assert(!adapter.Update(pi,rt).active);
 puts("PASS: 3600 simulation frames with movement, ADS and firing; reset and PiP entry/exit/fallback");
}
'''
with tempfile.TemporaryDirectory(prefix='bodycam-test-') as d:
 cpp=Path(d)/'test.cpp';exe=Path(d)/'test.exe';cpp.write_text(source)
 subprocess.run([a.compiler,'c++','-std=c++17','-DBODYCAM_STANDALONE','-I'+str(r/'src/xrGame'),str(cpp),str(r/'src/xrGame/bodycam_simulation.cpp'),str(r/'src/xrGame/bodycam_pip_adapter.cpp'),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
