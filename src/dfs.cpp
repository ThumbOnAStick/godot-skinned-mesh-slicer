#include <dfs.h>

namespace godot {
#pragma region Adjacency

/// @brief Get all vertices
/// @return Returns a string
String Triangle::get_all(){
	String string_builder = "(";
	for (size_t i = 0; i < this->vertices.size(); i++)
	{
		string_builder += String(vertices[i]) + ",";
	}
	string_builder += ")";
	return string_builder;
}

/// @brief Get all adjacencies
/// @return Returns a string
String Adjacency::get_all(){
	String string_builder = String();
	for (size_t i = 0; i < this->adjacencies.size(); i++)
	{
		string_builder += adjacencies[i][0].get_all() + "-->" + adjacencies[i][1].get_all() + "\n";
	}
	return string_builder;
}


/// @brief Directly push the tirangle pair to the adjacency list
/// @param triangle1 
/// @param triangle2 
void Adjacency::add_adjacency(Triangle &triangle1, Triangle &triangle2){
	this->adjacencies.push_back(std::array<Triangle, 2>{triangle1, triangle2});
}

/// @brief Try to add a triangle pair to the adjacency list
/// @param triangle1 
/// @param triangle2 
/// @return 
bool Adjacency::try_add_adjacency(Triangle &triangle1, Triangle &triangle2){
	for (size_t t1_v_idx = 0; t1_v_idx < 3; t1_v_idx++)
	{
		Vector3 t1_v = triangle1.vertices[t1_v_idx];
		for (size_t t2_v_idx = 0; t2_v_idx < 3; t2_v_idx++)
		{
			Vector3 t2_v = triangle2.vertices[t2_v_idx];
			if (t1_v == t2_v){
				add_adjacency(triangle1, triangle2);
				return true;
			}
		}
	}
	
	return false;
} 



#pragma endregion
} 