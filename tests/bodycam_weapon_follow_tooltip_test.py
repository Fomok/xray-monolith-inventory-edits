from pathlib import Path
import re
from lupa import LuaRuntime

root = Path(__file__).resolve().parents[1]
source = (root / "gamedata/scripts/bodycam_imgui.script").read_text()
def extract(text, start, end):
    return text[text.index(start):text.index(end, text.index(start))]

def harness(text):
    table = extract(text, "local binding_descriptions = {", "\n}") + "\n}"
    tooltip = extract(text, "local function tooltip_text(text)", "local function show_tooltip(text)")
    show = extract(text, "local function show_binding_tooltip(binding)", "local function float_format(binding)")
    lua = LuaRuntime()
    return lua.execute("""
        local captured
        ImGui = {
            BeginItemTooltip = function() return true end,
            EndTooltip = function() end,
            PushTextWrapPos = function() end,
            PopTextWrapPos = function() end,
            Separator = function() end,
            TextWrapped = function(text) assert(type(text) == 'string' and #text > 0, 'nil tooltip'); captured = text end,
            Text = function(text) assert(type(text) == 'string') end,
            TextDisabled = function(text) assert(type(text) == 'string') end,
        }
    """ + table + tooltip + show + """
        return function(name, kind)
            show_binding_tooltip({name=name, console_name='bodycam_'..name,
                type=kind, min=0, max=1, default=kind=='bool' and false or 0})
            return captured, binding_descriptions[name]
        end, function(value) tooltip_text(value); return captured end
    """)

LuaRuntime().compile(source)
show, tooltip = harness(source)
names = set(re.findall(r'"(weapon_follow_[a-z_]+)"', source))
assert len(names) == 9
for name in names:
    displayed, description = show(name, 'bool' if name.endswith('_enable') or name.endswith('_hold') else 'float')
    assert description and displayed == description
assert show('future_setting_without_description', 'bool')[0] == 'No description available.'
assert tooltip(None) == 'No description available.'
assert tooltip('') == 'No description available.'
assert tooltip('100% / %s') == '100%% / %%s'
print('PASS: all 9 weapon-follow hover paths, missing descriptions, nil/empty text and literal percent signs')
