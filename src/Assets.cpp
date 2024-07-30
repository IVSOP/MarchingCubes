#include "Assets.hpp"
#include "Crash.hpp"

																														// TODO doing this cpu side is prob bad
#define POST_PROCESS aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices
// aiProcessPreset_TargetRealtime_MaxQuality

void process_mesh(const aiMesh *mesh, GameObject *obj) {
	GLuint index_offset = obj->verts.size();

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
			obj->indices.emplace_back(face.mIndices[j] + index_offset);
		}
	}
}

void recursive_add_verts(const aiScene* scene, const aiNode *node, GameObject *obj) {
	// since N meshes, need to offset index
	GLuint index_offset;

	// go over all meshes of the node
	for (unsigned int m = 0; m < node->mNumMeshes; m++) {
		index_offset = obj->verts.size();
		const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

		process_mesh(mesh, obj);
	}

	// repeat for all children
	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		recursive_add_verts(scene,  node->mChildren[n], obj);
	}
}

std::unique_ptr<GameObject> Importer::load(const std::string &name) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

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

	recursive_add_verts(scene, node, obj.get());

	return obj;
}

void Importer::load(const std::string &name, std::vector<GameObject> &objs) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

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

	recursive_add_verts(scene, node, &objs.back());

	// objs.push_back(obj);
}

