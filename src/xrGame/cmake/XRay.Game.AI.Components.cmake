add_xray_game_static(XRay.Game.AI.Components.Damage
  DamageSource.h
  damage_manager.cpp
  damage_manager.h
  damage_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Action.Base
  action_base_script.cpp
  action_script_base.h
  action_script_base_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Action.Planner
  action_planner_script.cpp
  action_planner_script.h
  action_planner_script_inline.h
  action_planner.h
  action_planner_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Action.Planner.Action
  action_planner_action_script.cpp
  action_planner_action.h
  action_planner_action_inline.h
  action_planner_action_script.h
  action_planner_action_script_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.PropertyStorage
  property_storage_script.cpp
  property_storage.h
  property_storage_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.WorldPropertyEvaluator
  property_evaluator_script.cpp
  property_evaluator.h
  property_evaluator_const.h
  property_evaluator_const_inline.h
  property_evaluator_inline.h
  property_evaluator_member.h
  property_evaluator_member_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Danger
  danger_manager.cpp
  danger_object.cpp

  danger_manager.h
  danger_manager_inline.h
  danger_object.h
  danger_object_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Enemy
  enemy_manager.cpp
  enemy_manager.h
  enemy_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Item
  item_manager.cpp
  item_manager.h
)

add_xray_game_static(XRay.Game.AI.Components.Doors
  doors_actor.cpp
  doors_door.cpp
  doors_manager.cpp
  doors_actor.h
  doors_door.h
  doors_manager.h
)

add_xray_game_static(XRay.Game.AI.Components.Material
  material_manager.cpp
  material_manager.h
  material_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Memory
  memory_manager.cpp
  memory_manager.h
  memory_manager_inline.h

  memory_space_script.cpp
  memory_space.h
  memory_space_impl.h
)

add_xray_game_static(XRay.Game.AI.Components.Memory.Hit
  hit_memory_manager.cpp
  hit_memory_manager.h
  hit_memory_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Memory.Sound
  sound_memory_manager.cpp
  sound_memory_manager.h
  sound_memory_manager_inline.h
  sound_user_data_visitor.h
)

add_xray_game_static(XRay.Game.AI.Components.Memory.Visual
  vision_client.cpp
  vision_client.h
  vision_client_inline.h

  visual_memory_params.cpp
  visual_memory_params.h

  visual_memory_manager.cpp
  visual_memory_manager.h
  visual_memory_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Movement
  location_manager.cpp
  location_manager.h
  location_manager_inline.h

  abstract_location_selector.h
  abstract_location_selector_inline.h

  game_location_selector.h
  game_location_selector_inline.h

  abstract_path_manager.h
  abstract_path_manager_inline.h

  detail_path_manager.cpp
  detail_path_manager_smooth.cpp

  detail_path_builder.h
  detail_path_manager.h
  detail_path_manager_inline.h
  detail_path_manager_space.h

  game_path_manager.h
  game_path_manager_inline.h

  level_path_builder.h
  level_path_manager.h
  level_path_manager_inline.h

  patrol_path_manager.h
  patrol_path_manager_inline.h
  patrol_path_manager_space.h
  
  restricted_object.cpp
  restricted_object.h
  restricted_object_inline.h
  
  movement_manager.cpp
  movement_manager_game.cpp
  movement_manager_level.cpp
  movement_manager_patrol.cpp
  movement_manager_physic.cpp
  movement_manager.h
  movement_manager_impl.h
  movement_manager_inline.h
  movement_manager_space.h
)

add_xray_game_static(XRay.Game.AI.Components.MovingObjects
  moving_object.cpp
  moving_objects.cpp
  moving_objects_dynamic.cpp
  moving_objects_dynamic_collision.cpp
  moving_objects_static.cpp

  moving_object.h
  moving_objects.h
  moving_objects_impl.h
  moving_objects_inline.h
  moving_object_inline.h
)

add_xray_game_interface(XRay.Game.AI.Components.ProblemSolver
  condition_state.h
  condition_state_inline.h

  operator_abstract.h
  operator_abstract_inline.h

  operator_condition.h
  operator_condition_inline.h

  problem_solver.h
  problem_solver_inline.h
)

add_xray_game_interface(XRay.Game.AI.Components.SetupManager
  setup_manager.h
  setup_manager_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.ObjectHandler
  object_actions.cpp
  object_actions.h
  object_actions_inline.h

  object_property_evaluators.cpp
  object_property_evaluators.h
  object_property_evaluators_inline.h

  object_handler_planner.cpp
  object_handler_planner_missile.cpp
  object_handler_planner_weapon.cpp
  object_handler_planner.h
  object_handler_planner_impl.h
  object_handler_planner_inline.h

  object_handler.cpp
  object_handler.h
  object_handler_inline.h
  object_handler_space.h
)

add_xray_game_static(XRay.Game.AI.Components.Sight
  aimers_base.cpp
  aimers_base.h
  aimers_base_inline.h

  aimers_bone.h
  aimers_bone_inline.h

  aimers_weapon.cpp
  aimers_weapon.h
  aimers_weapon_inline.h

  control_action.h
  control_action_inline.h

  sight_action.cpp
  sight_action.h
  sight_action_inline.h

  sight_control_action.h
  sight_control_action_inline.h

  sight_manager.cpp
  sight_manager_target.cpp
  sight_manager.h
  sight_manager_inline.h
  sight_manager_space.h
)

add_xray_game_static(XRay.Game.AI.Components.Sound
  sound_collection_storage.cpp
  sound_collection_storage.h
  sound_collection_storage_inline.h

  sound_player.cpp
  sound_player.h
  sound_player_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.SpaceRestriction
  ../xrServerEntities/restriction_space.h

  space_restriction.cpp
  space_restriction_base.cpp
  space_restriction_bridge.cpp
  space_restriction_composition.cpp
  space_restriction_holder.cpp
  space_restriction_manager.cpp
  space_restriction_shape.cpp

  space_restriction.h
  space_restriction_abstract.h
  space_restriction_abstract_inline.h
  space_restriction_base.h
  space_restriction_base_inline.h
  space_restriction_bridge.h
  space_restriction_bridge_inline.h
  space_restriction_composition.h
  space_restriction_composition_inline.h
  space_restriction_holder.h
  space_restriction_holder_inline.h
  space_restriction_inline.h
  space_restriction_manager.h
  space_restriction_manager_inline.h
  space_restriction_shape.h
  space_restriction_shape_inline.h
)

add_xray_game_static(XRay.Game.AI.Components.Step
  step_manager.cpp
  step_manager.h
  step_manager_defs.h
)