"""Boundary tests only; engine rendering and ownership are not emulated here."""
from pathlib import Path
import unittest
from lupa.luajit21 import LuaRuntime
SOURCE = (Path(__file__).parent / "gamedata/scripts/inventory_lab.script").read_text()

class GuardTests(unittest.TestCase):
    def setUp(self):
        self.lua = LuaRuntime(unpack_returned_tuples=True)
        self.lua.execute("""
        logs={}; rebuilds=0; shown=false; pda_shown=false; present=true; alive=true
        function printf(fmt, text) logs[#logs+1]=text end
        level={present=function() return present end, reload_ui_xml=function() rebuilds=rebuilds+1 end}
        db={actor={alive=function() return alive end, iterate_inventory=function(self, cb, owner)
          for _,n in ipairs({9,2,7}) do
            cb(owner,{id=function() return n end,section=function() return 'fixture_'..n end})
          end
        end}}
        ActorMenu={get_actor_menu=function() return {IsShown=function() return shown end} end,
          get_pda_menu=function() return {IsShown=function() return pda_shown end} end}
        """)
        self.lua.execute(SOURCE)
    def test_no_auto_mutation(self):
        self.assertEqual(self.lua.globals().rebuilds,0)
        self.assertIsNone(self.lua.globals().on_game_start)
    def test_snapshot_is_sorted_and_does_not_rebuild(self):
        result=self.lua.globals().snapshot()
        self.assertEqual([result[i]['id'] for i in range(1,4)], [2,7,9])
        self.assertEqual(self.lua.globals().rebuilds,0)
    def test_requires_explicit_test_session(self):
        self.assertFalse(self.lua.globals().reload_native_ui(None))
        self.assertEqual(self.lua.globals().rebuilds,0)
    def test_rejects_missing_level_or_actor(self):
        for statement in ['present=false','present=true;alive=false','alive=true;db.actor=nil']:
            self.lua.execute(statement)
            self.assertFalse(self.lua.globals().reload_native_ui('isolated-native-session'))
        self.assertEqual(self.lua.globals().rebuilds,0)
    def test_rejects_open_dialogs(self):
        for statement in ['shown=true','shown=false;pda_shown=true']:
            self.lua.execute(statement)
            self.assertFalse(self.lua.globals().reload_native_ui('isolated-native-session'))
        self.assertEqual(self.lua.globals().rebuilds,0)
    def test_rejects_missing_bindings(self):
        self.lua.execute('ActorMenu=nil')
        self.assertFalse(self.lua.globals().reload_native_ui('isolated-native-session'))
    def test_closed_native_session_calls_real_entry_once(self):
        self.assertTrue(self.lua.globals().reload_native_ui('isolated-native-session'))
        self.assertEqual(self.lua.globals().rebuilds,1)

if __name__ == '__main__': unittest.main()
