#pragma once

#include <vector>
#include <random>
#include "Defines.h"
#include "./objects/Vk_Structures.hpp"

namespace VK4{
	class Vk_SampleObjects {
	public:
// ############################################################################################################
//       ██████     █    █     █ ██████  ███████ █     █         █          █    ██████   █████  ███████       
//       █     █   █ █   ██    █ █     █ █     █ ██   ██         █         █ █   █     █ █     █ █             
//       █     █  █   █  █ █   █ █     █ █     █ █ █ █ █         █        █   █  █     █ █       █             
//       ██████  █     █ █  █  █ █     █ █     █ █  █  █         █       █     █ ██████  █  ████ █████         
//       █   █   ███████ █   █ █ █     █ █     █ █     █         █       ███████ █   █   █     █ █             
//       █    █  █     █ █    ██ █     █ █     █ █     █         █       █     █ █    █  █     █ █             
//       █     █ █     █ █     █ ██████  ███████ █     █         ███████ █     █ █     █  █████  ███████       
// ############################################################################################################
		template<class T_DataType>
		static std::vector<T_DataType> ConstLarge_Data(size_t size, VK4::point_type value) {
			if constexpr(std::is_same_v<T_DataType,VK4::Vk_Vertex_PC>){
				std::vector<VK4::Vk_Vertex_PC> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(VK4::Vk_Vertex_PC{ 
						glm::tvec3<VK4::point_type>(value, value, value),
						glm::tvec3<VK4::point_type>(value, value, value)
					});
				}
				return data;
			}

			if constexpr(std::is_same_v<T_DataType,VK4::Vk_Vertex_PCN>){
				std::vector<VK4::Vk_Vertex_PCN> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(VK4::Vk_Vertex_PCN{ 
						glm::tvec3<VK4::point_type>(value, value, value),
						glm::tvec3<VK4::point_type>(value, value, value),
						glm::tvec3<VK4::point_type>(value, value, value)
					});
				}
				return data;
			}

			if constexpr(std::is_same_v<T_DataType,VK4::Vk_Vertex_PCNT>){
				std::vector<VK4::Vk_Vertex_PCNT> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(VK4::Vk_Vertex_PCNT{ 
						glm::tvec3<VK4::point_type>(value, value, value), 
						glm::tvec3<VK4::point_type>(value, value, value),
						glm::tvec3<VK4::point_type>(value, value, value),
						glm::tvec2<VK4::point_type>(value, value)
					});
				}
				return data;
			}

			if constexpr(std::is_same_v<T_DataType,VK4::index_type>){
				std::vector<VK4::index_type> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(static_cast<VK4::index_type>(value));
				}
				return data;
			}

			return std::vector<T_DataType>();
		}

		template<class T_DataType>
		static std::vector<T_DataType> UniformRandom_PointObjData(size_t size, VK4::point_type minValue, VK4::point_type maxValue) {
			std::random_device rd;  // Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

			if constexpr(std::is_same_v<T_DataType,VK4::Vk_Vertex_P>){
				std::uniform_real_distribution<> randP(minValue, maxValue);
				std::vector<VK4::Vk_Vertex_P> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(VK4::Vk_Vertex_P{ 
						glm::tvec3<VK4::point_type>(randP(gen), randP(gen), randP(gen))
					});
				}
				return data;
			}

			if constexpr(std::is_same_v<T_DataType,VK4::Vk_Vertex_C>){
				std::uniform_real_distribution<> randC(0, 1.0);
				std::vector<VK4::Vk_Vertex_C> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(VK4::Vk_Vertex_C{ 
						glm::tvec3<VK4::point_type>(randC(gen), randC(gen), randC(gen))
					});
				}
				return data;
			}

			// no randomness here because we may want to display the data
			if constexpr(std::is_same_v<T_DataType,VK4::index_type>){
				std::vector<VK4::index_type> data;
				for (size_t i = 0; i < size; ++i) {
					data.push_back(i);
				}
				return data;
			}

			Vk_Logger::RuntimeError(typeid(NoneObj), "Requested unsuported data type for random data generator!");
			return std::vector<T_DataType>();
		}

// ############################################################################################################
//                                     ██████  ███████ ███ █     █ ███████                                     
//                                     █     █ █     █  █  ██    █    █                                        
//                                     █     █ █     █  █  █ █   █    █                                        
//                                     ██████  █     █  █  █  █  █    █                                        
//                                     █       █     █  █  █   █ █    █                                        
//                                     █       █     █  █  █    ██    █                                        
//                                     █       ███████ ███ █     █    █                                        
// ############################################################################################################
		static std::vector<VK4::Vk_Vertex_P> Point_P(float angle=0) {
			std::vector<VK4::Vk_Vertex_P> geometry =
			{
				
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_C> Point_C()
		{
			std::vector<VK4::Vk_Vertex_C> colors =
			{
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   1.0,   1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0    ) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5    ) }
			};
			return colors;
		}

		static std::vector<VK4::index_type> Point_P_C_Indices() {
			std::vector<VK4::index_type> indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
			return indices;
		}


// ############################################################################################################
//                                         █       ███ █     █ ███████                                         
//                                         █        █  ██    █ █                                               
//                                         █        █  █ █   █ █                                               
//                                         █        █  █  █  █ █████                                           
//                                         █        █  █   █ █ █                                               
//                                         █        █  █    ██ █                                               
//                                         ███████ ███ █     █ ███████                                         
// ############################################################################################################
		static std::vector<VK4::Vk_Vertex_P> Line_P(float angle=0.0f) {
			return std::vector<VK4::Vk_Vertex_P> {
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) }
			};
		}

		static std::vector<VK4::Vk_Vertex_C> Line_C() {
			return std::vector<VK4::Vk_Vertex_C> {
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5) }
			};
		}

		static std::vector<VK4::index_type> Line_P_C_Indices() { 
			return std::vector<VK4::index_type> {
				0,1,  1,2,  2,3,  3,0,
				4,5,  5,6,  6,7,  7,4,
				0,4,  1,5,  2,6,  3,7
			};
		}


// ############################################################################################################
//                █████  ███████ ███████ ██████  ██████           █████  █     █  █████  ███████               
//               █     █ █     █ █     █ █     █ █     █         █     █  █   █  █     █    █                  
//               █       █     █ █     █ █     █ █     █         █         █ █   █          █                  
//               █       █     █ █     █ ██████  █     █          █████     █     █████     █                  
//               █       █     █ █     █ █   █   █     █               █    █          █    █                  
//               █     █ █     █ █     █ █    █  █     █         █     █    █    █     █    █                  
//                █████  ███████ ███████ █     █ ██████           █████     █     █████     █                  
// ############################################################################################################
		static std::vector<VK4::Vk_Vertex_P> Coords_P(float fromX, float toX, float tipLenX, float fromY, float toY, float tipLenY, float fromZ, float toZ, float tipLenZ)
		{
			std::vector<VK4::Vk_Vertex_P> geometry =
			{
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>( fromX,     0,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(   toX-tipLenX,     0,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(   toX,     0,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0, fromY,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0,   toY-tipLenY,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0,   toY,     0) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0,     0, fromZ) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0,     0,   toZ-tipLenZ) },
				VK4::Vk_Vertex_P { glm::tvec3<VK4::point_type>(     0,     0,   toZ) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_C> Coords_C(float tipR, float tipG, float tipB)
		{
			std::vector<VK4::Vk_Vertex_C> colors =
			{
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(tipR,tipG,tipB)},
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(tipR,tipG,tipB)},
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(tipR,tipG,tipB)}
			};
			return colors;
		}

		static std::vector<VK4::index_type> Coords_P_C_Indices() { 
			return std::vector<VK4::index_type> {
				0,1,  1,2,
				3,4,  4,5,
				6,7,  7,8
			};
		}


// ############################################################################################################
//    █████  █     █ ██████  ███████         █     █ ███████         █     █ ███████ ██████  █     █  █████    
//   █     █ █     █ █     █ █               █  █  █ █     █         ██    █ █     █ █     █ ██   ██ █     █   
//   █       █     █ █     █ █               █  █  █ █     █         █ █   █ █     █ █     █ █ █ █ █ █         
//   █       █     █ ██████  █████           █  █  █ █     █         █  █  █ █     █ ██████  █  █  █  █████    
//   █       █     █ █     █ █               █  █  █ █     █         █   █ █ █     █ █   █   █     █       █   
//   █     █ █     █ █     █ █               █  █  █ █     █         █    ██ █     █ █    █  █     █ █     █   
//    █████   █████  ██████  ███████          ██ ██  ███████         █     █ ███████ █     █ █     █  █████    
// ############################################################################################################
		static std::vector<VK4::Vk_Vertex_P> Cube1_P(float angle=0.0f)
		{
			std::vector<VK4::Vk_Vertex_P> geometry =
			{
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_C> Cube1_C()
		{
			std::vector<VK4::Vk_Vertex_C> colors =
			{
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5) }
			};
			return colors;
		}

		static std::vector<VK4::Vk_Vertex_N> Cube1_N()
		{
			std::vector<VK4::Vk_Vertex_N> normals =
			{
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>( 1, 1, 1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>(-1, 1, 1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>(-1,-1, 1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>( 1,-1, 1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>( 1, 1,-1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>(-1, 1,-1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>(-1,-1,-1) },
				VK4::Vk_Vertex_N { glm::tvec3<VK4::point_type>( 1,-1,-1) }
			};
			return normals;
		}

		static std::vector<VK4::Vk_Vertex_PC> Cube1_PC()
		{
			std::vector<VK4::Vk_Vertex_PC> geometry =
			{
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>( 1, 1, 1),  glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>(-1, 1, 1),  glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>(-1,-1, 1),  glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>( 1,-1, 1),  glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>( 1, 1,-1),  glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>(-1, 1,-1),  glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>(-1,-1,-1),  glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },
				VK4::Vk_Vertex_PC { glm::tvec3<VK4::point_type>( 1,-1,-1),  glm::tvec3<VK4::point_type>(0.5,   0, 0.5) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_PCN> Cube1_PCN()
		{
			std::vector<VK4::Vk_Vertex_PCN> geometry =
			{
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>( 1, 1, 1),  glm::tvec3<VK4::point_type>(1.0,   0,   0),  glm::tvec3<VK4::point_type>( 1, 1, 1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>(-1, 1, 1),  glm::tvec3<VK4::point_type>(0,   1.0,   0),  glm::tvec3<VK4::point_type>(-1, 1, 1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>(-1,-1, 1),  glm::tvec3<VK4::point_type>(0,     0, 1.0),  glm::tvec3<VK4::point_type>(-1,-1, 1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>( 1,-1, 1),  glm::tvec3<VK4::point_type>(1.0, 1.0,   0),  glm::tvec3<VK4::point_type>( 1,-1, 1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>( 1, 1,-1),  glm::tvec3<VK4::point_type>(0,   1.0, 1.0),  glm::tvec3<VK4::point_type>( 1, 1,-1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>(-1, 1,-1),  glm::tvec3<VK4::point_type>(1.0,   0, 1.0),  glm::tvec3<VK4::point_type>(-1, 1,-1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>(-1,-1,-1),  glm::tvec3<VK4::point_type>(1.0, 0.5,   0),  glm::tvec3<VK4::point_type>(-1,-1,-1) },
				VK4::Vk_Vertex_PCN { glm::tvec3<VK4::point_type>( 1,-1,-1),  glm::tvec3<VK4::point_type>(0.5,   0, 0.5),  glm::tvec3<VK4::point_type>( 1,-1,-1) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_PCNT> Cube1_PCNT()
		{
			std::vector<VK4::Vk_Vertex_PCNT> normals = {
			/*00*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1, 1, 1),  glm::tvec3<VK4::point_type>(1.0,   0,   0),  glm::tvec3<VK4::point_type>( 1, 1, 1), glm::tvec2<VK4::point_type>(0.25f, 0.50f) },
			/*01*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1, 1, 1),  glm::tvec3<VK4::point_type>(0,   1.0,   0),  glm::tvec3<VK4::point_type>(-1, 1, 1), glm::tvec2<VK4::point_type>(0.25f, 0.25f) },
			/*02*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1,-1, 1),  glm::tvec3<VK4::point_type>(0,     0, 1.0),  glm::tvec3<VK4::point_type>(-1,-1, 1), glm::tvec2<VK4::point_type>(0.50f, 0.25f) },
			/*03*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1,-1, 1),  glm::tvec3<VK4::point_type>(1.0, 1.0,   0),  glm::tvec3<VK4::point_type>( 1,-1, 1), glm::tvec2<VK4::point_type>(0.50f, 0.50f) },
			/*04*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1, 1,-1),  glm::tvec3<VK4::point_type>(0,   1.0, 1.0),  glm::tvec3<VK4::point_type>( 1, 1,-1), glm::tvec2<VK4::point_type>(0.00f, 0.50f) },
			/*05*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1, 1,-1),  glm::tvec3<VK4::point_type>(1.0,   0, 1.0),  glm::tvec3<VK4::point_type>(-1, 1,-1), glm::tvec2<VK4::point_type>(0.00f, 0.25f) },
			/*06*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1,-1,-1),  glm::tvec3<VK4::point_type>(1.0, 0.5,   0),  glm::tvec3<VK4::point_type>(-1,-1,-1), glm::tvec2<VK4::point_type>(0.75f, 0.25f) },
			/*07*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1,-1,-1),  glm::tvec3<VK4::point_type>(0.5,   0, 0.5),  glm::tvec3<VK4::point_type>( 1,-1,-1), glm::tvec2<VK4::point_type>(0.75f, 0.50f) },

			/*08*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1, 1,-1),  glm::tvec3<VK4::point_type>(1.0,   0, 1.0),  glm::tvec3<VK4::point_type>(-1, 1,-1), glm::tvec2<VK4::point_type>(1.00f, 0.25f) },
			/*09*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1, 1,-1),  glm::tvec3<VK4::point_type>(0,   1.0, 1.0),  glm::tvec3<VK4::point_type>( 1, 1,-1), glm::tvec2<VK4::point_type>(1.00f, 0.50f) },
			/*10*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1, 1,-1),  glm::tvec3<VK4::point_type>(1.0,   0, 1.0),  glm::tvec3<VK4::point_type>(-1, 1,-1), glm::tvec2<VK4::point_type>(0.75f, 0.00f) },
			/*11*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>(-1, 1, 1),  glm::tvec3<VK4::point_type>(0,   1.0,   0),  glm::tvec3<VK4::point_type>(-1, 1, 1), glm::tvec2<VK4::point_type>(0.50f, 0.00f) },
			/*12*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1, 1, 1),  glm::tvec3<VK4::point_type>(1.0,   0,   0),  glm::tvec3<VK4::point_type>( 1, 1, 1), glm::tvec2<VK4::point_type>(0.50f, 0.75f) },
			/*13*/	VK4::Vk_Vertex_PCNT { glm::tvec3<VK4::point_type>( 1, 1,-1),  glm::tvec3<VK4::point_type>(0,   1.0, 1.0),  glm::tvec3<VK4::point_type>( 1, 1,-1), glm::tvec2<VK4::point_type>(0.75f, 0.75f) },
			};
			return normals;
		}

		static std::vector<VK4::index_type> Cube1_PCNT_Indices() {
			std::vector<VK4::index_type> indices = { 
				/*top   */ 0,1,2,  2,3,0,
				/*back  */ 0,5,1,  0,4,5,  
				/*right */ 3,7,0,  7,4,0,  
				/*left  */ 1,5,2,  5,6,2,  
				/*front */ 7,3,2,  2,6,7,  
				/*bottom*/ 7,6,5,  5,4,7 
			};
			return indices;
		}

		static std::vector<VK4::index_type> Cube1_P_C_Indices() {
			return Cube1_PCNT_Indices();
		}

		static std::vector<VK4::index_type> Cube1_PC_Indices() {
			return Cube1_PCNT_Indices();
		}

		static std::vector<VK4::index_type> Cube1_PCN_Indices() {
			return Cube1_PCNT_Indices();
		}


// ############################################################################################################
//        █████  █     █ ██████  ███████         █     █         █     █ ███████ ██████  █     █  █████        
//       █     █ █     █ █     █ █               █  █  █         ██    █ █     █ █     █ ██   ██ █     █       
//       █       █     █ █     █ █               █  █  █         █ █   █ █     █ █     █ █ █ █ █ █             
//       █       █     █ ██████  █████           █  █  █         █  █  █ █     █ ██████  █  █  █  █████        
//       █       █     █ █     █ █               █  █  █         █   █ █ █     █ █   █   █     █       █       
//       █     █ █     █ █     █ █               █  █  █         █    ██ █     █ █    █  █     █ █     █       
//        █████   █████  ██████  ███████          ██ ██          █     █ ███████ █     █ █     █  █████        
// ############################################################################################################
		static std::vector<VK4::Vk_Vertex_P> Cube2_P(float angle=0.0f)
		{
			std::vector<VK4::Vk_Vertex_P> geometry =
			{
				/* 00t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 01b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 02r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 03t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 04b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 05l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 06t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 07l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 08f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 09t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 10f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 11r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 12u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 13b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 14r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 15u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 16b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 17l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 18u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 19l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 20f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 21t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 22f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 23r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1,-1,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}

		static std::vector<VK4::Vk_Vertex_P> Cube2_N(float angle=0.0f)
		{
			/*t=top, b=back, r=right, l=left, f=front, u=under(==bottom)*/
			std::vector<VK4::Vk_Vertex_P> geometry =
			{
				/* 00t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 01b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 02r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 03t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 04b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 05l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 06t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 07l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 08f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0,-1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 09t */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0, 1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 10r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0,-1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 11f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 12u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 13b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 14r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 15u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 16b */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 17l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 18u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 19l */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>(-1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 20f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0,-1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },

				/* 21u */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0, 0,-1), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 22r */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 0,-1, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) },
				/* 23f */ VK4::Vk_Vertex_P { glm::rotate(glm::tvec3<VK4::point_type>( 1, 0, 0), angle, glm::tvec3<VK4::point_type>( 0, 0, 1)) }
			};
			return geometry;
		}		

		static std::vector<VK4::Vk_Vertex_C> Cube2_C()
		{
			std::vector<VK4::Vk_Vertex_C> colors =
			{
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0,   0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0,   0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,     0, 1.0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 1.0,   0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0,   1.0, 1.0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0,   0, 1.0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(1.0, 0.5,   0) },

				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5) },
				VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.5,   0, 0.5) }
			};
			return colors;
		}

		static std::vector<VK4::index_type> Cube2_P_C_N_Indices() {
			std::vector<VK4::index_type> indices = { 
				/*top   */  0, 3, 6,   6, 9, 0,
				/*back  */  1,16, 4,   1,13,16,  
				/*right */ 10,22, 2,  22,14, 2,  
				/*left  */  5,17, 7,  17,19, 7,  
				/*front */ 23,11, 8,   8,20,23,  
				/*bottom*/ 21,18,15,  15,12,21 
			};
			return indices;
		}

		static std::vector<VK4::Vk_Vertex_P> Cube2_NormalLines_P(float len, float angle=0.0f)
		{
			std::vector<VK4::Vk_Vertex_P> res;
			auto p = Cube2_P(angle);
			auto n = Cube2_N(angle);
			size_t s = p.size();
			for(size_t i=0; i<s; ++i){
				res.push_back(p.at(i));
				auto pPos = p.at(i).pos;
				auto nPos = n.at(i).pos;
				auto f = pPos + len*nPos;
				res.push_back(VK4::Vk_Vertex_P{ .pos=f });
			};

			return res;
		}

		static std::vector<VK4::index_type> Cube2_NormalLines_Indices() {
			std::vector<VK4::index_type> indices;
			for(size_t i=0; i<48; ++i){
				indices.push_back(i);
			}
			return indices;
		}

		static std::vector<VK4::Vk_Vertex_C> Cube2_NormalLines_C()
		{
			return std::vector<VK4::Vk_Vertex_C>(48, VK4::Vk_Vertex_C { glm::tvec3<VK4::point_type>(0.0, 1.0, 0.0) });
		}
	};
}
