"""Offline Lua entry-point contracts; never substitute for native stash acceptance."""
import argparse,json,sys
from pathlib import Path
ap=argparse.ArgumentParser();ap.add_argument('--lupa-path',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
sys.path.insert(0,str(a.lupa_path))
from lupa.luajit21 import LuaRuntime
lua=LuaRuntime(unpack_returned_tuples=True)
lua.execute(r'''
active=true; identity="108"; requests={}; vanilla=0; closed=0; callbacks={}; cancel_remove=false
CUIScriptWnd={}
function class(name) return function() _G[name]={} end end
function strformat(...) return string.format(...) end
function Unregister_UI() end
function alife_create() vanilla=vanilla+1;return nil end
actor_menu={set_msg=function() end}
game={translate_string=function(key) return key=="st_itm_stash_of_character" and "Stash %s" or key end}
db={actor={id=function() return 0 end,character_name=function() return "Player" end}}
bag={id=function() return 50 end,section=function() return "itm_backpack" end,parent=function() return db.actor end}
db.actor.item_in_slot=function() return bag end
db.actor.position=function() return {} end;db.actor.level_vertex_id=function() return 1 end;db.actor.game_vertex_id=function() return 1 end
proxy={section=function() return "sqa3_ground_backpack" end}
level={inventory_next_stash_enabled=function() return active end,
 inventory_next_stash_identity=function() return identity end,
 inventory_next_stash_create=function(id,expected,name)
  if expected~=identity then return false end
  requests[#requests+1]={id=id,expected=expected,name=name};return true
 end,
 object_by_id=function(id) if id==50 then return bag elseif id==900 then return proxy end end}
function SendScriptCallback(name,data)
 callbacks[#callbacks+1]={name=name,data=data}
 if name=="actor_on_stash_remove" then data.cancel=cancel_remove end
end
function module(code)
 local env=setmetatable({},{__index=_G});local chunk=assert(loadstring(code));setfenv(chunk,env);chunk();return env
end
''')
base=Path(__file__).parent/'gamedata/scripts'
lua.globals().sqa3_stash=lua.globals().module((base/'sqa3_stash.script').read_bytes())
lua.globals().item_backpack=lua.globals().module((base/'item_backpack.script').read_bytes())
lua.execute(r'''
UICreateStash.Close=function() closed=closed+1 end
input={SetText=function() end,GetText=function() return "Named stash" end}
dialog=setmetatable({input=input},{__index=UICreateStash})
dialog:Reset(bag);dialog:OnAccept()
assert(#requests==1 and requests[1].name=="Named stash" and requests[1].expected=="108" and closed==1 and vanilla==0)
item_backpack.actor_on_item_use({section=function() return "itm_actor_backpack" end})
assert(#requests==2 and requests[2].name=="Stash Player" and vanilla==0)
dialog:Reset(bag);identity="109";dialog:OnAccept()
assert(#requests==2 and closed==1 and vanilla==0,"stale dialog recreated or submitted a replacement bag")
assert(not sqa3_stash.create(nil,"Name",identity))
assert(not sqa3_stash.create(bag,string.rep("x",241),identity))
assert(not sqa3_stash.create(bag,"bad\nname",identity));assert(#requests==2)
sqa3_stash.on_created(900,"Named stash","itm_backpack")
assert(#callbacks==1 and callbacks[1].name=="actor_on_stash_create" and callbacks[1].data.stash_id==900)
assert(sqa3_stash.can_remove(900));cancel_remove=true;assert(not sqa3_stash.can_remove(900));assert(not sqa3_stash.can_remove(999))
active=false;dialog:OnAccept();item_backpack.actor_on_item_use({section=function() return "itm_actor_backpack" end})
assert(vanilla==2 and #requests==2,"disabled mode changed vanilla creation entry points")
''')
report={'status':'passed','checks':['dialog entry','equipped-bag entry','stale dialog identity','invalid input retains source','creation callback','cancellable removal','disabled vanilla fallback'],'limits':'Mocked Lua boundary contracts only; engine compilation and gameplay remain required.'}
a.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report))
