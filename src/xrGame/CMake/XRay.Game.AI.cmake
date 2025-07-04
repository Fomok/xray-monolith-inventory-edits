add_module(XRay.Game.AI
  TYPE STATIC
  
  INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/xrServerEntities
  
  SOURCES
  ai/monsters/ai_monster_effector.cpp
  ai/trader/trader_animation.cpp
  ai/weighted_random.cpp
  ai_obstacle.cpp
  AI_PhraseDialogManager.cpp
  ai_sounds.cpp
  ai_space.cpp

  ai/monsters/ai_monster_effector.h
  ai/trader/trader_animation.h
  ai/weighted_random.h
  ai_obstacle.h
  ai_obstacle_inline.h
  AI_PhraseDialogManager.h
  ai_space.h
  ai_space_inline.h
)

include(XRay.Game.AI.Components)
include(XRay.Game.AI.Cover)
include(XRay.Game.AI.Debug)
include(XRay.Game.AI.Eval)
include(XRay.Game.AI.Group)
include(XRay.Game.AI.Life)
include(XRay.Game.AI.Monsters)
include(XRay.Game.AI.Navigation)