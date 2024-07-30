#include "Assets.hpp"
#include "Crash.hpp"

void recursive_add_verts(const aiScene* scene, const aiNode *node, VertContainer<SimpleVertex> &verts) {
	// go over all meshes of the node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

		// go over all vertices of the mesh
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &pos = mesh->mVertices[i];
			verts.emplace_back(pos.x, pos.y, pos.z);
		}
	}

	// repeat for all children
	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		recursive_add_verts(scene,  node->mChildren[n], verts);
	}
}

std::unique_ptr<GameObject> Importer::load(const std::string &name) {
	// TODO let this be on the stack so it can be destroyed
	Assimp::Importer *importer = new Assimp::Importer;

	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer->ReadFile(ASSETS_FOLDER + name, aiProcessPreset_TargetRealtime_MaxQuality); // this is a preset that already does a lot of things
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

	recursive_add_verts(scene, node, obj->verts);

	return obj;
}
