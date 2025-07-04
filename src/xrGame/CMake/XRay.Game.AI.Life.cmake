add_module(XRay.Game.AI.Life
  CHILD_OF XRay.Game

  SOURCES
  ../xrServerEntities/alife_space.cpp
  ../xrServerEntities/alife_space.h

  alife_simulator.cpp
  alife_simulator.h
  alife_simulator_inline.h

  alife_simulator_script.cpp
)

add_module(XRay.Game.AI.Life.Interaction
  CHILD_OF XRay.Game

  SOURCES
  alife_interaction_manager.cpp
  alife_interaction_manager.h
  alife_interaction_manager_inline.h
)

add_module(XRay.Game.AI.Life.Interaction.Combat
  CHILD_OF XRay.Game

  SOURCES
  alife_combat_manager.cpp
  alife_combat_manager.h
  alife_combat_manager_inline.h
)

add_module(XRay.Game.AI.Life.Interaction.Communication
  CHILD_OF XRay.Game

  SOURCES
  alife_communication_manager.cpp
  alife_communication_manager.h
  alife_communication_manager_inline.h
  alife_communication_space.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade_manager.cpp
  inventory_upgrade_manager.h
  inventory_upgrade_manager_inline.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade.Property
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade_property.cpp
  inventory_upgrade_property.h
  inventory_upgrade_property_inline.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade.Upgrade
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade.h
  inventory_upgrade.cpp
  inventory_upgrade_inline.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade.Base
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade_base.cpp
  inventory_upgrade_base.h
  inventory_upgrade_base_inline.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade.Group
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade_group.cpp
  inventory_upgrade_group.h
  inventory_upgrade_group_inline.h
)

add_module(XRay.Game.AI.Life.InventoryUpgrade.Root
  CHILD_OF XRay.Game

  SOURCES
  inventory_upgrade_root.cpp
  inventory_upgrade_root.h
  inventory_upgrade_root_inline.h
)

add_module(XRay.Game.AI.Life.SavedGameWrapper
  CHILD_OF XRay.Game

  SOURCES
  saved_game_wrapper.cpp
  saved_game_wrapper_script.cpp
  saved_game_wrapper.h
  saved_game_wrapper_inline.h
)

add_module(XRay.Game.AI.Life.ServerEntities
  CHILD_OF XRay.Game

  SOURCES
  alife_anomalous_zone.cpp
  alife_creature_abstract.cpp
  alife_dynamic_object.cpp
  alife_group_abstract.cpp
  alife_object.cpp
  
  alife_online_offline_group.cpp
  alife_online_offline_group_brain.cpp
  alife_online_offline_group_brain.h
  alife_online_offline_group_brain_inline.h

  alife_smart_zone.cpp
  alife_trader.cpp
  alife_trader_abstract.cpp
)

add_module(XRay.Game.AI.Life.ServerEntities.Human
  CHILD_OF XRay.Game

  SOURCES
  ../xrServerEntities/alife_human_brain.cpp
  ../xrServerEntities/alife_human_brain.h
  ../xrServerEntities/alife_human_brain_inline.h

  alife_human_brain_script.cpp

  alife_human_brain_save.h

  alife_human_object_handler.cpp
  alife_human_object_handler.h
  alife_human_object_handler_inline.h
  alife_human_object_handler_save.h

  alife_human_abstract.cpp
)

add_module(XRay.Game.AI.Life.ServerEntities.Monster
  CHILD_OF XRay.Game

  SOURCES
  ../xrServerEntities/alife_monster_brain.cpp
  ../xrServerEntities/alife_monster_brain.h
  ../xrServerEntities/alife_monster_brain_inline.h
  
  alife_monster_brain_script.cpp

  alife_monster_detail_path_manager.cpp
  alife_monster_detail_path_manager_script.cpp
  alife_monster_detail_path_manager.h
  alife_monster_detail_path_manager_inline.h

  ../xrServerEntities/alife_movement_manager_holder.h
  
  alife_monster_patrol_path_manager.cpp
  alife_monster_patrol_path_manager_script.cpp
  alife_monster_patrol_path_manager.h
  alife_monster_patrol_path_manager_inline.h
  
  alife_monster_movement_manager.cpp
  alife_monster_movement_manager_script.cpp
  alife_monster_movement_manager.h
  alife_monster_movement_manager_inline.h

  alife_monster_abstract.cpp
  alife_monster_base.cpp
)

add_module(XRay.Game.AI.Life.Simulator
  CHILD_OF XRay.Game

  SOURCES
  alife_simulator_base.cpp
  alife_simulator_base2.cpp
  alife_simulator_base.h
  alife_simulator_base_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Header
  CHILD_OF XRay.Game

  SOURCES
  alife_simulator_header.cpp
  alife_simulator_header.h
  alife_simulator_header_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Abstract
  CHILD_OF XRay.Game

  SOURCES
  alife_abstract_registry.h
  alife_abstract_registry_inline.h

  alife_registry_wrapper.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Graph
  CHILD_OF XRay.Game

  SOURCES
  alife_graph_registry.cpp
  alife_graph_registry.h
  alife_graph_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Graph.Level
  CHILD_OF XRay.Game

  SOURCES
  alife_level_registry.h
  alife_level_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Group
  CHILD_OF XRay.Game

  SOURCES
  alife_group_registry.cpp
  alife_group_registry.h
  alife_group_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Object
  CHILD_OF XRay.Game

  SOURCES
  alife_object_registry.cpp
  alife_object_registry.h
  alife_object_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.SafeMapIterator
  CHILD_OF XRay.Game

  SOURCES
  safe_map_iterator.h
  safe_map_iterator_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Schedule
  CHILD_OF XRay.Game

  SOURCES
  alife_schedule_registry.cpp

  alife_schedule_registry.h
  alife_schedule_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.SmartTerrain
  CHILD_OF XRay.Game

  SOURCES
  alife_smart_terrain_registry.cpp
  alife_smart_terrain_registry.h
  alife_smart_terrain_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Spawn
  CHILD_OF XRay.Game

  SOURCES
  alife_spawn_registry.cpp
  alife_spawn_registry.h
  alife_spawn_registry_inline.h

  alife_spawn_registry_spawn.cpp
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Spawn.Header
  CHILD_OF XRay.Game

  SOURCES
  alife_spawn_registry_header.cpp
  alife_spawn_registry_header.h
  alife_spawn_registry_header_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Spawn.ServerEntityWrapper
  CHILD_OF XRay.Game

  SOURCES
  server_entity_wrapper.cpp
  server_entity_wrapper.h
  server_entity_wrapper_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.Registries.Story
  CHILD_OF XRay.Game

  SOURCES
  alife_story_registry.cpp
  alife_story_registry.h
  alife_story_registry_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.RegistryContainer
  CHILD_OF XRay.Game

  SOURCES
  alife_registry_container.cpp
  alife_registry_container.h
  alife_registry_container_composition.h
  alife_registry_container_inline.h
  alife_registry_container_space.h
  
  alife_registry_wrappers.h
)

add_module(XRay.Game.AI.Life.Simulator.SmartTerrainTask
  CHILD_OF XRay.Game

  SOURCES
  alife_smart_terrain_task.cpp
  alife_smart_terrain_task_script.cpp
  alife_smart_terrain_task.h
  alife_smart_terrain_task_inline.h
)

add_module(XRay.Game.AI.Life.Simulator.TimeManager
  CHILD_OF XRay.Game

  SOURCES
  alife_time_manager.cpp
  alife_time_manager.h
  alife_time_manager_inline.h
)

add_module(XRay.Game.AI.Life.Update
  CHILD_OF XRay.Game

  SOURCES
  alife_update_manager.cpp
  alife_update_manager.h
  alife_update_manager_inline.h
)

add_module(XRay.Game.AI.Life.Update.Storage
  CHILD_OF XRay.Game

  SOURCES
  alife_storage_manager.cpp
  alife_storage_manager.h
  alife_storage_manager_inline.h
)

add_module(XRay.Game.AI.Life.Update.Surge
  CHILD_OF XRay.Game

  SOURCES
  alife_surge_manager.cpp
  alife_surge_manager.h
  alife_surge_manager_inline.h
)

add_module(XRay.Game.AI.Life.Update.Switch
  CHILD_OF XRay.Game

  SOURCES
  alife_switch_manager.cpp
  alife_switch_manager.h
  alife_switch_manager_inline.h
)