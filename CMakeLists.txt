cmake_minimum_required(VERSION 3.15)


project(RayTracing)



set(CMAKE_CXX_STANDARD 14)



add_executable(RayTracing main.cpp functions.cpp functions.hpp Figure.cpp Figure.hpp Sphere.cpp Sphere.hpp Box.cpp Box.hpp Tetra.cpp Tetra.hpp Factory.hpp SphereFactory.cpp SphereFactory.hpp BoxFactory.cpp BoxFactory.hpp TetraFactory.cpp TetraFactory.hpp CImg.h)