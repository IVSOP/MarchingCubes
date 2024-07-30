#include "Assets.hpp"
#include "Crash.hpp"

// since N meshes, need to offset index
void recursive_add_verts(const aiScene* scene, const aiNode *node, GameObject *obj, GLuint index_offset) {
	// go over all meshes of the node
	for (unsigned int m = 0; m < node->mNumMeshes; m++) {
		const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

		// go over all vertices of the mesh
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &pos = mesh->mVertices[v];
			obj->verts.emplace_back(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(0.0f), glm::vec2(0.0f));
		}

		// go over all faces of the mesh
		for(unsigned int f = 0; f < mesh->mNumFaces; f++)
		{
			const aiFace face = mesh->mFaces[f];
			for(unsigned int j = 0; j < face.mNumIndices; j++) {
				obj->indices.push_back(face.mIndices[j] + index_offset);
			}
		}
	}

	// repeat for all children
	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		recursive_add_verts(scene,  node->mChildren[n], obj, obj->verts.size());
	}
}

std::unique_ptr<GameObject> Importer::load(const std::string &name) {
	Assimp::Importer importer;

	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, aiProcessPreset_TargetRealtime_MaxQuality); // this is a preset that already does a lot of things
		// aiProcess_CalcTangentSpace       |
		// aiProcess_Triangulate            |
		// aiProcess_JoinIdenticalVertices  |
		// aiProcess_SortByPType);

	 // If the import failed, report it
	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	// importer destructor will clean everything up!!!!!!

	// scene has nodes
	// nodes have num meshes, and an array of indices to meshes
	// these indices are for an array in the scene itself

	// need to iterate over the nodes recursively and add up the vertices that make up their meshes
	// TODO test this vs iterating over things already in scene

	std::unique_ptr<GameObject> obj = std::make_unique<GameObject>(1); // getNextPowerOfTwo(mesh->mNumVertices));

	aiNode *node = scene->mRootNode;

	recursive_add_verts(scene, node, obj.get(), 0);

	return obj;
}

void Importer::load(const std::string &name, std::vector<GameObject> &objs) {
	Assimp::Importer importer;

	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, aiProcessPreset_TargetRealtime_MaxQuality); // this is a preset that already does a lot of things
		// aiProcess_CalcTangentSpace       |
		// aiProcess_Triangulate            |
		// aiProcess_JoinIdenticalVertices  |
		// aiProcess_SortByPType);

	 // If the import failed, report it
	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	// importer destructor will clean everything up!!!!!!

	// scene has nodes
	// nodes have num meshes, and an array of indices to meshes
	// these indices are for an array in the scene itself

	// need to iterate over the nodes recursively and add up the vertices that make up their meshes
	// TODO test this vs iterating over things already in scene

	objs.emplace_back(1); // getNextPowerOfTwo(mesh->mNumVertices));

	aiNode *node = scene->mRootNode;

	recursive_add_verts(scene, node, &objs.back(), 0);

	// objs.push_back(obj);
}

