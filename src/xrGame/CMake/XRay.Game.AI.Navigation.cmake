add_module(XRay.Game.AI.Navigation
  TYPE STATIC

  LINKS
  ode
  luabind
  LuaJIT
  LZO
  TinyXML

  XRay.Core.Defines

  XRay.Includes
  XRay.Collision.Includes
  XRay.Core.Includes
  XRay.Engine.Includes
  XRay.Game.Includes
  XRay.NetServer.Includes
  XRay.Physics.Includes
  XRay.Render.Common.Includes
  XRay.Render.API.Includes
  XRay.ServerEntities.Includes

  XRay.Game.Precompiles
)

target_compile_options(XRay.Game.AI.Navigation
  PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/wd4244>
)

add_module(XRay.Game.AI.Navigation.GameGraph
  SOURCES
  ../xrServerEntities/game_graph_space.h
  game_graph_script.cpp
  game_graph.h
  game_graph_inline.h
)

add_module(XRay.Game.AI.Navigation.GameLevelCrossTable
  SOURCES
  game_level_cross_table.h
  game_level_cross_table_inline.h
)

add_module(XRay.Game.AI.Navigation.GraphAbstract.GraphEdge
  SOURCES
  graph_edge.h
  graph_edge_inline.h
)

add_module(XRay.Game.AI.Navigation.GraphAbstract.GraphVertex
  SOURCES
  graph_vertex.h
  graph_vertex_inline.h
)

add_module(XRay.Game.AI.Navigation.LevelGraph
  SOURCES
  level_graph.cpp
  level_graph.h
  level_graph_inline.h

  level_graph_vertex.cpp
  level_graph_vertex_inline.h

  level_graph_debug.cpp
  level_graph_debug2.cpp
  level_graph_space.h
)

add_module(XRay.Game.AI.Navigation.ObjectLocation
  SOURCES
  ai_object_location.h
  ai_object_location_impl.h
  ai_object_location_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.Algorithms.AStar
  SOURCES
  a_star.h
  a_star_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.Algorithms.Dijkstra
  SOURCES
  dijkstra.h
  dijkstra_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Constructors
  SOURCES
  data_storage_constructor.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Constructors.Builder
  SOURCES
  builder_allocator_constructor.h
  builder_allocator_constructor_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Constructors.ManagerBuilder
  SOURCES
  manager_builder_allocator_constructor.h
  manager_builder_allocator_constructor_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.PathBuilders.EdgePath
  SOURCES
  edge_path.h
  edge_path_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.PathBuilders.VertexPath
  SOURCES
  vertex_path.h
  vertex_path_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.PriorityQueues
  SOURCES
  data_storage_double_linked_list.h
  data_storage_double_linked_list_inline.h
  data_storage_single_linked_list.h
  data_storage_single_linked_list_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.PriorityQueues.BinaryHeap
  SOURCES
  data_storage_binary_heap.h
  data_storage_binary_heap_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.PriorityQueues.BucketList
  SOURCES
  data_storage_bucket_list.h
  data_storage_bucket_list_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Vertex.Allocators.Fixed
  SOURCES
  vertex_allocator_fixed.h
  vertex_allocator_fixed_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Vertex.Managers.Fixed
  SOURCES
  vertex_manager_fixed.h
  vertex_manager_fixed_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.DataStorages.Vertex.Managers.HashFixed
  SOURCES
  vertex_manager_hash_fixed.h
  vertex_manager_hash_fixed_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.GraphEngine
  SOURCES
  graph_engine.h
  graph_engine_inline.h
  graph_engine_space.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager
  SOURCES
  path_manager.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager.Game
  SOURCES
  path_manager_game.h
  path_manager_game_inline.h

  path_manager_game_level.h
  path_manager_game_level_inline.h

  path_manager_game_vertex.h
  path_manager_game_vertex_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager.Generic
  SOURCES
  path_manager_generic.h
  path_manager_generic_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager.Level
  SOURCES
  path_manager_level.h
  path_manager_level_flooder.h
  path_manager_level_flooder_inline.h
  path_manager_level_inline.h
  path_manager_level_nearest_vertex.h
  path_manager_level_nearest_vertex_inline.h
  path_manager_level_straight_line.h
  path_manager_level_straight_line_inline.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager.Params
  SOURCES
  path_manager_params.h
  path_manager_params_flooder.h
  path_manager_params_game_level.h
  path_manager_params_game_vertex.h
  path_manager_params_nearest_vertex.h
  path_manager_params_straight_line.h
)

add_module(XRay.Game.AI.Navigation.Pathfinding.PathManager.Solver
  SOURCES
  path_manager_solver.h
  path_manager_solver_inline.h
)

add_module(XRay.Game.AI.Navigation.PatrolPaths.Path
  SOURCES
  patrol_path.cpp
  patrol_path.h
  patrol_path_inline.h
)

add_module(XRay.Game.AI.Navigation.PatrolPaths.Storage
  SOURCES
  patrol_path_storage.cpp
  patrol_path_storage.h
  patrol_path_storage_inline.h
)

add_module(XRay.Game.AI.Navigation.PatrolPaths.Point
  # Needs to end up in XRay.Game.AI for correct linkage against XRay.Engine
  NO_LINK_PARENT

  SOURCES
  patrol_point.cpp
  patrol_point.h
  patrol_point_inline.h
)
