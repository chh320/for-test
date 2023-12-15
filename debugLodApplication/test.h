#pragma once
#include <vector>
#include <fstream>
#include <sstream>
#include <unordered_map>

using namespace std;
using real_t = float;

class Parser final {
public:
	class Material_new {
	public:
		float ka_r, ka_g, ka_b, a;
		float ks_r, ks_g, ks_b;
		int ns;
		Material_new() {}
		Material_new(float a_r, float a_g, float a_b, float s_r, float s_g, float s_b, float a, int ns) :ka_r(a_r), ka_g(a_g), ka_b(a_b), ks_r(s_r), ks_g(s_g), ks_b(s_b), a(a), ns(ns) {}
		int save_to_file(ofstream& os) {
			os.write((const char*)&ka_r, sizeof(float));
			os.write((const char*)&ka_g, sizeof(float));
			os.write((const char*)&ka_b, sizeof(float));
			os.write((const char*)&ks_r, sizeof(float));
			os.write((const char*)&ks_g, sizeof(float));
			os.write((const char*)&ks_b, sizeof(float));
			os.write((const char*)&a, sizeof(float));
			os.write((const char*)&ns, sizeof(int));
			return 0;
		}
	};


	struct Properties {
	public:
		unordered_map<string, string> propertySet;
	};

	struct Datas4Component {

		unsigned int count_id;

		//related to geometry datas
		unsigned int edge_index_start;
		unsigned int edge_index_end;

		unsigned int mat_index_start;
		unsigned int mat_index_end;

		//related to properties
		string name;
		string description;
		string guid;
		string type;

		string storey_name;
		int storey_index;

		vector<Properties> propertySet;

	};

	struct Datas2OpenGL {
		vector<unsigned int> vert_indices;
		vector<unsigned int> edge_indices;
		vector<vector<unsigned int>> search_m;
		vector<real_t> verts;
		vector<real_t> vert_normals2;
		vector<Material_new> face_mat;

		unordered_map<string, int> storey_map_string2int;
		unordered_map<int, string> storey_map_int2string;

		vector<vector<unsigned int>> storeys_component_id;

		vector<int> this_comp_belongs_to_which_storey;

		vector<Datas4Component> componentDatas;
	};

	template <typename T>
	vector<T> read_vector_from_binary(ifstream& is) {
		size_t s;
		is.read((char*)&s, sizeof(size_t));
		if (s == 0) return vector<T>();
		vector<T> ret(s);
		is.read((char*)&ret[0], sizeof(T) * s);
		return ret;
	}

	template <typename T>
	T read_meta_from_binary(ifstream& is) {
		T ret;
		is.read((char*)&ret, sizeof(T));
		return ret;
	}

	string read_string_from_binary(ifstream& is) {
		size_t s;
		char str[10000];
		is.read((char*)&s, sizeof(size_t));
		is.read(str, s);
		string ret(str, s / sizeof(char));
		return ret;
	}

	unordered_map<string, string> read_properties_from_binary(ifstream& is) {
		unordered_map<string, string> ret;
		size_t s = read_meta_from_binary<size_t>(is);
		for (size_t i = 0; i < s; i++) {
			string first = read_string_from_binary(is);
			ret[first] = read_string_from_binary(is);
		}
		return ret;
	}

	Datas4Component read_datas4Component_from_binary(ifstream& is) {
		Datas4Component ret;
		ret.count_id = read_meta_from_binary<unsigned int>(is);
		ret.edge_index_start = read_meta_from_binary<unsigned int>(is);
		ret.edge_index_end = read_meta_from_binary<unsigned int>(is);
		ret.mat_index_start = read_meta_from_binary<unsigned int>(is);
		ret.mat_index_end = read_meta_from_binary<unsigned int>(is);

		ret.name = read_string_from_binary(is);
		ret.description = read_string_from_binary(is);
		ret.guid = read_string_from_binary(is);
		ret.type = read_string_from_binary(is);

		ret.propertySet.resize(read_meta_from_binary<size_t>(is));
		for (size_t i = 0; i < ret.propertySet.size(); i++) {
			ret.propertySet[i].propertySet = read_properties_from_binary(is);
		}


		// 22.10.27 updated, add storey information
		ret.storey_name = read_string_from_binary(is);
		ret.storey_index = read_meta_from_binary<int>(is);

		return ret;
	}

	template <typename T> unordered_map<T, string> read_unordered_map_from_binary_1(ifstream& is) {
		unordered_map<T, string> ret;
		size_t s = read_meta_from_binary<size_t>(is);
		for (size_t i = 0; i < s; i++) {
			T first = read_meta_from_binary<T>(is);
			ret[first] = read_string_from_binary(is);
		}
		return ret;
	}

	template <typename T> unordered_map<string, T> read_unordered_map_from_binary_2(ifstream& is) {
		unordered_map<string, T> ret;
		size_t s = read_meta_from_binary<size_t>(is);
		for (size_t i = 0; i < s; i++) {
			string first = read_string_from_binary(is);
			ret[first] = read_meta_from_binary<T>(is);
		}
		return ret;
	}

	Datas2OpenGL read_datas2OpenGL_from_binary(ifstream& is, vector<vector<uint32_t>>& the_c_edge_indices, vector<real_t>& the_face_area, vector<vector<uint32_t>>& the_f_ver_indice, vector<uint32_t>& the_face_num_in_comp, vector<uint32_t>& the_collision_pairs) {
		Datas2OpenGL ret;
		ret.vert_indices = read_vector_from_binary<unsigned int>(is);
		ret.edge_indices = read_vector_from_binary<unsigned int>(is);
		ret.search_m.resize(read_meta_from_binary<size_t>(is));
		for (size_t i = 0; i < ret.search_m.size(); i++) {
			ret.search_m[i] = read_vector_from_binary<unsigned int>(is);
		}
		ret.verts = read_vector_from_binary<real_t>(is);
		ret.vert_normals2 = read_vector_from_binary<real_t>(is);
		ret.face_mat = read_vector_from_binary<Material_new>(is);

		ret.componentDatas.resize(read_meta_from_binary<size_t>(is));
		for (size_t i = 0; i < ret.componentDatas.size(); i++) {
			ret.componentDatas[i] = read_datas4Component_from_binary(is);
		}

		// 22.10.27 updated, add storey information
		ret.storeys_component_id.resize(read_meta_from_binary<size_t>(is));
		for (size_t i = 0; i < ret.storeys_component_id.size(); i++) {
			ret.storeys_component_id[i] = read_vector_from_binary<unsigned int>(is);
		}
		ret.storey_map_string2int = read_unordered_map_from_binary_2<int>(is);
		ret.storey_map_int2string = read_unordered_map_from_binary_1<int>(is);
		ret.this_comp_belongs_to_which_storey = read_vector_from_binary<int>(is);

		//22.11.21 updated, add components' edge index into a vector<vector<uint32_t>> into .midfile
		//the_c_edge_indices.resize(read_meta_from_binary<size_t>(is));
		//for (size_t i = 0; i < the_c_edge_indices.size(); i++) {
		//	the_c_edge_indices[i] = read_vector_from_binary<uint32_t>(is);
		//}

		////23.3.1 updated, add components' face information
		//the_face_area = read_vector_from_binary<real_t>(is);

		//the_f_ver_indice.resize(read_meta_from_binary<size_t>(is));
		//for (size_t i = 0; i < the_f_ver_indice.size(); i++) {
		//	the_f_ver_indice[i] = read_vector_from_binary<uint32_t>(is);
		//}

		//the_face_num_in_comp = read_vector_from_binary<uint32_t>(is);
		//the_collision_pairs = read_vector_from_binary<uint32_t>(is);
		return ret;
	}
};