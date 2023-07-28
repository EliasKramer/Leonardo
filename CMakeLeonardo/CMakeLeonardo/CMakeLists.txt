# CMakeList.txt : CMake project for CMakeLeonardo, include source and define
# project specific logic here.
#
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
enable_language(CUDA)
# Check if the CMake version is greater than or equal to 3.12
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.12)
    # Set C++ standard to 20 for the whole project
    set(CMAKE_CXX_STANDARD 20)
endif()
include_directories($ENV{CUDA_PATH}/include)
# Add source to this project's executable.
add_executable (CMakeLeonardo "LeonardoAi/NeuroFox/gpu_math.cu" "LeonardoAi/NeuroFox/gpu_math.cuh" "MockChessEngine/AlphaBetaPruningBot.cpp" "MockChessEngine/AlphaBetaPruningBot.h" "MockChessEngine/BitBoard.cpp" "MockChessEngine/BitBoard.h" "MockChessEngine/BoardRepresentation.cpp" "MockChessEngine/BoardRepresentation.h" "MockChessEngine/ChessBoard.cpp" "MockChessEngine/ChessBoard.h" "MockChessEngine/ChessPiece.cpp" "MockChessEngine/ChessPiece.h" "MockChessEngine/Constants.cpp" "MockChessEngine/Constants.h" "MockChessEngine/DataAndTypes.cpp" "MockChessEngine/DataAndTypes.h" "MockChessEngine/Game.cpp" "MockChessEngine/Game.h" "MockChessEngine/HelpMethods.cpp" "MockChessEngine/HelpMethods.h" "MockChessEngine/HumanPlayer.cpp" "MockChessEngine/HumanPlayer.h"  "MockChessEngine/Medicrius.cpp" "MockChessEngine/Medicrius.h" "MockChessEngine/Move.cpp" "MockChessEngine/Move.h" "MockChessEngine/MoveCastle.cpp" "MockChessEngine/MoveCastle.h" "MockChessEngine/MoveDoublePawn.cpp" "MockChessEngine/MoveDoublePawn.h" "MockChessEngine/MoveEnPassant.cpp" "MockChessEngine/MoveEnPassant.h" "MockChessEngine/MovePromote.cpp" "MockChessEngine/MovePromote.h" "MockChessEngine/Player.cpp" "MockChessEngine/Player.h" "MockChessEngine/RandomPlayer.cpp" "MockChessEngine/RandomPlayer.h" "LeonardoAi/chess_arena.cpp" "LeonardoAi/leonardo_bot.cpp" "LeonardoAi/leonardo_overlord.cpp" "LeonardoAi/leonardo_util.cpp" "LeonardoAi/main.cpp" "LeonardoAi/NeuroFox/convolutional_layer.cpp" "LeonardoAi/NeuroFox/data_space.cpp" "LeonardoAi/NeuroFox/fully_connected_layer.cpp" "LeonardoAi/NeuroFox/layer.cpp" "LeonardoAi/NeuroFox/math_functions.cpp" "LeonardoAi/NeuroFox/matrix.cpp" "LeonardoAi/NeuroFox/neural_network.cpp" "LeonardoAi/NeuroFox/pooling_layer.cpp" "LeonardoAi/NeuroFox/test_result.cpp" "LeonardoAi/NeuroFox/util.cpp" "LeonardoAi/NeuroFox/vector3.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeLeonardo PROPERTY CXX_STANDARD 20)
endif()

#add your cuda architecture here (6.1 is 61 - and supports the 1070)
#https://gist.github.com/standaloneSA/99788f30466516dbcc00338b36ad5acf
set_property(TARGET CMakeLeonardo PROPERTY CUDA_ARCHITECTURES 61)